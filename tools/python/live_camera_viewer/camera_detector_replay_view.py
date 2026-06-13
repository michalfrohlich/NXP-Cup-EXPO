"""Detector replay view for the live camera viewer.

This viewer records a 30 second camera run, then lets you replay it frame-by-frame.

It stores:
- raw 12-bit camera samples when frame.values_raw is available
- fallback 8-bit samples only when raw samples are not present
- detector metadata from frame.vision
- timestamps and sequence numbers

It displays:
- current/replay camera line with left/right/center detector markers
- processed stacked scan image with detector traces drawn over it
- replay slider, play/pause, step controls
- CSV, NPZ and PNG export
"""

from __future__ import annotations

import csv
import time
from dataclasses import dataclass
from pathlib import Path

import numpy as np
import pyqtgraph as pg
from PySide6 import QtCore, QtGui, QtWidgets

from .camera_frame import CameraFrame, ReaderStats
from .config import DEFAULT_DISPLAY_HZ, DEFAULT_HISTORY, SAMPLE_COUNT
from .serial_reader import LatestFrameStore


PIXELS = np.arange(SAMPLE_COUNT)
ADC_12BIT_MAX = 4095
ADC_8BIT_MAX = 255
DEFAULT_RECORD_SECONDS = 30.0
RECORD_LENGTH_OPTIONS_S = (5, 10, 30, 60)

DISPLAY_X_SCALE = 6
DISPLAY_Y_SCALE = 1

INVALID_IDX = -1

LEFT_RIGHT_COLOR = QtGui.QColor("#2e7d32")
CENTER_COLOR = QtGui.QColor("#fb8c00")
CURRENT_ROW_COLOR = QtGui.QColor("#03a9f4")
TEXT_COLOR = QtGui.QColor("#eeeeee")


@dataclass
class ReplayFrame:
    values: np.ndarray
    sequence: int
    teensy_time_ms: int
    host_time_s: float
    source: str
    valid: bool
    status: int
    status_name: str
    feature: int
    feature_name: str
    confidence: int
    error: float
    error_pct: float
    left_idx: int
    right_idx: int
    center_idx: int
    flags: int


class DetectorReplayWindow(QtWidgets.QMainWindow):
    def __init__(
        self,
        *,
        store: LatestFrameStore,
        port_label: str,
        display_hz: float = DEFAULT_DISPLAY_HZ,
        history_size: int = DEFAULT_HISTORY,
    ) -> None:
        super().__init__()
        self.store = store
        self.port_label = port_label
        self.history_size = history_size

        self.latest_frame: CameraFrame | None = None
        self._display_times: list[float] = []

        self._recording = False
        self._record_seconds = DEFAULT_RECORD_SECONDS
        self._record_start_s = 0.0
        self._recorded_sequences: set[int] = set()
        self._frames: list[ReplayFrame] = []

        self._current_index = -1
        self._last_image_u8: np.ndarray | None = None
        self._last_render_pixmap: QtGui.QPixmap | None = None
        self._last_contrast_limits: tuple[float, float] = (0.0, 255.0)
        self._last_source_name = "none"

        self.setWindowTitle(f"Camera detector replay - {port_label}")
        self.resize(1600, 860)

        central = QtWidgets.QWidget()
        root = QtWidgets.QVBoxLayout(central)
        root.setContentsMargins(6, 6, 6, 6)
        root.setSpacing(6)
        self.setCentralWidget(central)

        controls = QtWidgets.QHBoxLayout()
        controls.setSpacing(6)
        root.addLayout(controls)

        self.record_button = QtWidgets.QPushButton("Record")
        self.record_button.clicked.connect(self._start_recording)
        controls.addWidget(self.record_button)

        controls.addWidget(QtWidgets.QLabel("Length:"))
        self.record_length_combo = QtWidgets.QComboBox()
        for seconds in RECORD_LENGTH_OPTIONS_S:
            self.record_length_combo.addItem(f"{seconds} s", seconds)
        default_index = list(RECORD_LENGTH_OPTIONS_S).index(int(DEFAULT_RECORD_SECONDS))
        self.record_length_combo.setCurrentIndex(default_index)
        self.record_length_combo.currentIndexChanged.connect(self._record_length_changed)
        controls.addWidget(self.record_length_combo)

        self.stop_button = QtWidgets.QPushButton("Stop")
        self.stop_button.clicked.connect(self._stop_recording)
        controls.addWidget(self.stop_button)

        self.play_button = QtWidgets.QPushButton("Play")
        self.play_button.setCheckable(True)
        self.play_button.clicked.connect(self._toggle_playback)
        controls.addWidget(self.play_button)

        self.step_back_button = QtWidgets.QPushButton("◀ Step")
        self.step_back_button.clicked.connect(lambda: self._step_replay(-1))
        controls.addWidget(self.step_back_button)

        self.step_forward_button = QtWidgets.QPushButton("Step ▶")
        self.step_forward_button.clicked.connect(lambda: self._step_replay(1))
        controls.addWidget(self.step_forward_button)

        self.save_npz_button = QtWidgets.QPushButton("Save NPZ")
        self.save_npz_button.clicked.connect(self._save_npz)
        controls.addWidget(self.save_npz_button)

        self.load_npz_button = QtWidgets.QPushButton("Load NPZ")
        self.load_npz_button.clicked.connect(self._load_npz)
        controls.addWidget(self.load_npz_button)

        self.save_csv_button = QtWidgets.QPushButton("Export CSV")
        self.save_csv_button.clicked.connect(self._save_csv)
        controls.addWidget(self.save_csv_button)

        self.save_png_button = QtWidgets.QPushButton("Save PNG")
        self.save_png_button.clicked.connect(self._save_png)
        controls.addWidget(self.save_png_button)

        self.status_label = QtWidgets.QLabel(f"Idle | {port_label}")
        controls.addWidget(self.status_label, stretch=1)

        self.frame_slider = QtWidgets.QSlider(QtCore.Qt.Orientation.Horizontal)
        self.frame_slider.setMinimum(0)
        self.frame_slider.setMaximum(0)
        self.frame_slider.setEnabled(False)
        self.frame_slider.valueChanged.connect(self._slider_changed)
        root.addWidget(self.frame_slider)

        views = QtWidgets.QHBoxLayout()
        views.setSpacing(8)
        root.addLayout(views, stretch=1)

        left_panel = QtWidgets.QVBoxLayout()
        left_panel.setSpacing(6)
        views.addLayout(left_panel, stretch=3)

        self.current_plot = pg.PlotWidget()
        self.current_plot.setTitle("Replay frame camera line")
        self.current_plot.setLabel("bottom", "Pixel")
        self.current_plot.setLabel("left", "Camera value")
        self.current_plot.setXRange(-0.5, SAMPLE_COUNT - 0.5)
        self.current_plot.setYRange(0, ADC_12BIT_MAX)
        self.current_plot.showGrid(x=True, y=True, alpha=0.25)
        self.current_plot.setMinimumWidth(520)

        self.signal_curve = self.current_plot.plot(
            PIXELS,
            np.zeros(SAMPLE_COUNT),
            pen=pg.mkPen(width=2),
            name="camera",
        )
        self.left_line = pg.InfiniteLine(
            angle=90,
            movable=False,
            pen=pg.mkPen("#2e7d32", width=2),
        )
        self.right_line = pg.InfiniteLine(
            angle=90,
            movable=False,
            pen=pg.mkPen("#2e7d32", width=2),
        )
        self.center_line = pg.InfiniteLine(
            angle=90,
            movable=False,
            pen=_center_pen("#fb8c00"),
        )
        for line in (self.left_line, self.right_line, self.center_line):
            line.hide()
            self.current_plot.addItem(line)

        left_panel.addWidget(self.current_plot, stretch=1)

        self.frame_info = QtWidgets.QTextEdit()
        self.frame_info.setReadOnly(True)
        self.frame_info.setMaximumHeight(175)
        self.frame_info.setLineWrapMode(QtWidgets.QTextEdit.LineWrapMode.NoWrap)
        font = self.frame_info.font()
        font.setFamily("Consolas")
        self.frame_info.setFont(font)
        left_panel.addWidget(self.frame_info)

        self.image_label = QtWidgets.QLabel()
        self.image_label.setAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
        self.image_label.setMinimumSize(SAMPLE_COUNT * DISPLAY_X_SCALE, 600)
        self.image_label.setStyleSheet("QLabel { background: #111; border: 1px solid #555; }")
        self.image_label.setText("Record a run to build the detector replay image")

        self.image_scroll = QtWidgets.QScrollArea()
        self.image_scroll.setWidgetResizable(True)
        self.image_scroll.setWidget(self.image_label)
        views.addWidget(self.image_scroll, stretch=6)

        self.live_timer = QtCore.QTimer(self)
        self.live_timer.timeout.connect(self._refresh_live)
        interval_ms = max(1, int(round(1000.0 / max(1.0, display_hz))))
        self.live_timer.start(interval_ms)

        self.play_timer = QtCore.QTimer(self)
        self.play_timer.timeout.connect(lambda: self._step_replay(1))
        self.play_timer.setInterval(33)

        self.record_button.setText(f"Record {int(self._record_seconds)} s")
        self._update_button_states()

    def _record_length_changed(self) -> None:
        self._record_seconds = float(self.record_length_combo.currentData())
        if not self._recording:
            self.record_button.setText(f"Record {int(self._record_seconds)} s")

    def _start_recording(self) -> None:
        self._stop_playback()
        self._record_seconds = float(self.record_length_combo.currentData())
        self._frames.clear()
        self._recorded_sequences.clear()
        self._current_index = -1
        self._last_image_u8 = None
        self._last_render_pixmap = None
        self._last_source_name = "none"
        self._record_start_s = time.monotonic()
        self._recording = True

        self.image_label.clear()
        self.image_label.setText("Recording... waiting for frames")
        self.frame_slider.blockSignals(True)
        self.frame_slider.setMaximum(0)
        self.frame_slider.setValue(0)
        self.frame_slider.setEnabled(False)
        self.frame_slider.blockSignals(False)

        self.record_button.setEnabled(False)
        self.record_length_combo.setEnabled(False)
        self.record_button.setText("Recording...")
        self._update_button_states()

    def _stop_recording(self) -> None:
        if not self._recording:
            return

        self._recording = False
        self.record_button.setEnabled(True)
        self.record_length_combo.setEnabled(True)
        self.record_button.setText(f"Record {int(self._record_seconds)} s")

        self._finalize_recording()

    def _finalize_recording(self) -> None:
        if not self._frames:
            self.image_label.clear()
            self.image_label.setText("No frames recorded")
            self._current_index = -1
            self._update_button_states()
            return

        self._current_index = 0
        self._update_slider_range()
        self._render_replay_image()
        self._show_frame(self._current_index)
        self._update_button_states()

    def _refresh_live(self) -> None:
        latest, history, reader_stats = self.store.snapshot()
        now_s = time.monotonic()

        if latest is None:
            self.status_label.setText(f"Waiting for camera frames | {self.port_label}")
            return

        self.latest_frame = latest
        self._record_display_time()

        if self._recording:
            elapsed_s = now_s - self._record_start_s
            added = self._record_new_history_frames(history)
            if added > 0:
                self._current_index = len(self._frames) - 1
                self._update_slider_range()
                self._show_frame(self._current_index, render_image=False)

            if elapsed_s >= self._record_seconds:
                self._stop_recording()
                elapsed_s = self._record_seconds

            remaining_s = max(0.0, self._record_seconds - elapsed_s)
            self.status_label.setText(
                f"Recording {elapsed_s:0.1f}/{self._record_seconds:0.1f}s, "
                f"{remaining_s:0.1f}s left | "
                f"frames {len(self._frames)} | "
                f"RX {reader_stats.receive_fps:0.1f} fps | "
                f"drops {reader_stats.estimated_dropped_frames} | "
                f"{self.port_label}"
            )
            return

        if not self._frames:
            values, source_name = _frame_values(latest)
            replay_frame = _replay_frame_from_camera_frame(latest, values, source_name)
            self._draw_line_frame(replay_frame)
            self._set_frame_info(replay_frame, reader_stats)
            self.status_label.setText(
                f"Idle | live seq {latest.sequence} | "
                f"source {source_name} | "
                f"RX {reader_stats.receive_fps:0.1f} fps | "
                f"{self.port_label}"
            )

    def _record_new_history_frames(self, history: list[CameraFrame]) -> int:
        added = 0
        for frame in history:
            if frame.sequence in self._recorded_sequences:
                continue
            if frame.host_time_s < self._record_start_s:
                continue
            if (frame.host_time_s - self._record_start_s) > self._record_seconds:
                continue

            values, source_name = _frame_values(frame)
            replay_frame = _replay_frame_from_camera_frame(frame, values, source_name)

            self._frames.append(replay_frame)
            self._recorded_sequences.add(frame.sequence)
            self._last_source_name = source_name
            added += 1
        return added

    def _update_slider_range(self) -> None:
        self.frame_slider.blockSignals(True)
        self.frame_slider.setEnabled(bool(self._frames))
        self.frame_slider.setMinimum(0)
        self.frame_slider.setMaximum(max(0, len(self._frames) - 1))
        if self._current_index >= 0:
            self.frame_slider.setValue(self._current_index)
        self.frame_slider.blockSignals(False)

    def _slider_changed(self, value: int) -> None:
        if not self._frames:
            return
        self._stop_playback()
        self._current_index = int(value)
        self._show_frame(self._current_index)

    def _toggle_playback(self, checked: bool) -> None:
        if checked:
            if not self._frames:
                self.play_button.setChecked(False)
                return
            if self._current_index < 0:
                self._current_index = 0
            if self._current_index >= len(self._frames) - 1:
                self._current_index = 0
            self.play_button.setText("Pause")
            self.play_timer.start()
        else:
            self._stop_playback()

    def _stop_playback(self) -> None:
        self.play_timer.stop()
        self.play_button.blockSignals(True)
        self.play_button.setChecked(False)
        self.play_button.setText("Play")
        self.play_button.blockSignals(False)

    def _step_replay(self, delta: int) -> None:
        if not self._frames:
            return

        next_index = self._current_index + delta
        if next_index >= len(self._frames):
            next_index = len(self._frames) - 1
            self._stop_playback()
        elif next_index < 0:
            next_index = 0
            self._stop_playback()

        self._current_index = next_index
        self.frame_slider.blockSignals(True)
        self.frame_slider.setValue(self._current_index)
        self.frame_slider.blockSignals(False)
        self._show_frame(self._current_index)

    def _show_frame(self, index: int, *, render_image: bool = True) -> None:
        if index < 0 or index >= len(self._frames):
            return

        frame = self._frames[index]
        self._draw_line_frame(frame)
        self._set_frame_info(frame, None)

        if render_image:
            self._render_replay_image(current_index=index)

        self.status_label.setText(
            f"Replay frame {index + 1}/{len(self._frames)} | "
            f"seq {frame.sequence} | "
            f"status {frame.status_name} | "
            f"confidence {frame.confidence} | "
            f"error {frame.error:+0.3f} | "
            f"{self.port_label}"
        )

    def _draw_line_frame(self, frame: ReplayFrame) -> None:
        self.signal_curve.setData(PIXELS, frame.values.astype(float))
        self._update_current_plot_range(frame.values, frame.source)
        self._set_detector_line(self.left_line, frame.left_idx)
        self._set_detector_line(self.right_line, frame.right_idx)
        self._set_detector_line(self.center_line, frame.center_idx)

    def _update_current_plot_range(self, values: np.ndarray, source_name: str) -> None:
        if source_name == "values_raw_12bit":
            self.current_plot.setYRange(0, ADC_12BIT_MAX)
            return
        maximum = int(np.max(values)) if values.size else ADC_8BIT_MAX
        self.current_plot.setYRange(0, max(ADC_8BIT_MAX, maximum))

    def _set_detector_line(self, line: pg.InfiniteLine, raw_idx: int | float | None) -> None:
        if raw_idx is None or raw_idx < 0 or raw_idx >= SAMPLE_COUNT:
            line.hide()
            return
        line.setPos(raw_idx)
        line.show()

    def _render_replay_image(self, current_index: int | None = None) -> None:
        if not self._frames:
            self.image_label.clear()
            self.image_label.setText("No replay frames")
            return

        image_u8 = self._build_display_image_u8()
        self._last_image_u8 = image_u8

        overlay_pixmap = self._build_overlay_pixmap(image_u8, current_index=current_index)
        self._last_render_pixmap = overlay_pixmap
        self.image_label.setPixmap(overlay_pixmap)
        self.image_label.setMinimumSize(overlay_pixmap.size())

        if current_index is not None:
            self._scroll_to_frame(current_index)

    def _build_display_image_u8(self) -> np.ndarray:
        rows = np.vstack([frame.values for frame in self._frames]).astype(np.float32)

        lo = float(np.percentile(rows, 1.0))
        hi = float(np.percentile(rows, 99.0))
        if hi <= lo:
            lo = float(np.min(rows))
            hi = float(np.max(rows))
        if hi <= lo:
            hi = lo + 1.0
        self._last_contrast_limits = (lo, hi)

        normalized = ((rows - lo) * 255.0) / (hi - lo)
        normalized = np.clip(normalized, 0.0, 255.0).astype(np.uint8)

        # QImage row 0 is top. Put newest frame on top and oldest frame bottom.
        return normalized[::-1].copy()

    def _build_overlay_pixmap(
        self,
        image_u8: np.ndarray,
        *,
        current_index: int | None,
    ) -> QtGui.QPixmap:
        qimage = _gray_qimage_from_array(image_u8)
        base = QtGui.QPixmap.fromImage(qimage)
        scaled = base.scaled(
            image_u8.shape[1] * DISPLAY_X_SCALE,
            max(1, image_u8.shape[0] * DISPLAY_Y_SCALE),
            QtCore.Qt.AspectRatioMode.IgnoreAspectRatio,
            QtCore.Qt.TransformationMode.FastTransformation,
        )

        painter = QtGui.QPainter(scaled)
        painter.setRenderHint(QtGui.QPainter.RenderHint.Antialiasing, False)

        self._draw_detector_trace(
            painter,
            [frame.left_idx for frame in self._frames],
            LEFT_RIGHT_COLOR,
            dashed=False,
        )
        self._draw_detector_trace(
            painter,
            [frame.right_idx for frame in self._frames],
            LEFT_RIGHT_COLOR,
            dashed=False,
        )
        self._draw_detector_trace(
            painter,
            [frame.center_idx for frame in self._frames],
            CENTER_COLOR,
            dashed=True,
        )

        if current_index is not None and 0 <= current_index < len(self._frames):
            y = self._frame_index_to_display_y(current_index)
            pen = QtGui.QPen(CURRENT_ROW_COLOR)
            pen.setWidth(2)
            painter.setPen(pen)
            painter.drawLine(0, y, scaled.width(), y)

        self._draw_overlay_text(painter, scaled)
        painter.end()
        return scaled

    def _draw_detector_trace(
        self,
        painter: QtGui.QPainter,
        indices: list[int],
        color: QtGui.QColor,
        *,
        dashed: bool,
    ) -> None:
        pen = QtGui.QPen(color)
        pen.setWidth(2)
        if dashed:
            pen.setStyle(QtCore.Qt.PenStyle.CustomDashLine)
            pen.setDashPattern([5, 8])
        painter.setPen(pen)

        previous_point: QtCore.QPoint | None = None
        for chronological_index, raw_idx in enumerate(indices):
            if raw_idx is None or raw_idx < 0 or raw_idx >= SAMPLE_COUNT:
                previous_point = None
                continue

            point = QtCore.QPoint(
                int(round((float(raw_idx) + 0.5) * DISPLAY_X_SCALE)),
                self._frame_index_to_display_y(chronological_index),
            )
            if previous_point is not None:
                painter.drawLine(previous_point, point)
            previous_point = point

    def _draw_overlay_text(self, painter: QtGui.QPainter, pixmap: QtGui.QPixmap) -> None:
        if not self._frames:
            return
        lo, hi = self._last_contrast_limits
        text = (
            f"{len(self._frames)} frames | "
            f"oldest bottom, newest top | "
            f"contrast {lo:0.0f}..{hi:0.0f} | "
            "green=L/R, orange=center"
        )
        font = painter.font()
        font.setPointSize(9)
        painter.setFont(font)
        metrics = painter.fontMetrics()
        rect = metrics.boundingRect(text).adjusted(-5, -3, 5, 3)
        rect.moveTopLeft(QtCore.QPoint(8, 8))

        painter.fillRect(rect, QtGui.QColor(0, 0, 0, 165))
        painter.setPen(TEXT_COLOR)
        painter.drawText(rect, QtCore.Qt.AlignmentFlag.AlignCenter, text)

    def _frame_index_to_display_y(self, chronological_index: int) -> int:
        # index 0 is oldest and must be at the bottom.
        display_row = (len(self._frames) - 1) - chronological_index
        return int(round((display_row + 0.5) * DISPLAY_Y_SCALE))

    def _scroll_to_frame(self, index: int) -> None:
        if not self._frames:
            return
        y = self._frame_index_to_display_y(index)
        scroll_bar = self.image_scroll.verticalScrollBar()
        target = max(0, y - self.image_scroll.viewport().height() // 2)
        scroll_bar.setValue(target)

    def _set_frame_info(
        self,
        frame: ReplayFrame,
        reader_stats: ReaderStats | None,
    ) -> None:
        scroll_bar = self.frame_info.verticalScrollBar()
        previous_scroll_value = scroll_bar.value()
        previous_scroll_max = scroll_bar.maximum()
        was_at_bottom = previous_scroll_value >= max(0, previous_scroll_max - 2)

        reader_lines = []
        if reader_stats is not None:
            reader_lines = [
                "",
                "Reader",
                f"  rx fps:        {reader_stats.receive_fps:7.2f}",
                f"  frames:        {reader_stats.parsed_frames}",
                f"  dropped est:   {reader_stats.estimated_dropped_frames}",
                f"  malformed:     {reader_stats.malformed_lines}",
                f"  parse errors:  {reader_stats.parse_errors}",
                f"  last error:    {reader_stats.last_error}",
            ]

        text = "\n".join(
            [
                "Frame",
                f"  replay index:  {self._current_index + 1 if self._current_index >= 0 else 0}/{len(self._frames)}",
                f"  seq:           {frame.sequence}",
                f"  teensy ms:     {frame.teensy_time_ms}",
                f"  source:        {frame.source}",
                "",
                "Detector",
                f"  valid:         {int(frame.valid)}  flags=0x{frame.flags:02X}",
                f"  status:        {frame.status_name} ({frame.status})",
                f"  feature:       {frame.feature_name} ({frame.feature})",
                f"  confidence:    {frame.confidence}",
                f"  error:         {frame.error:+0.4f} ({frame.error_pct:+0.1f}%)",
                f"  left idx:      {_idx_text(frame.left_idx)}",
                f"  right idx:     {_idx_text(frame.right_idx)}",
                f"  center idx:    {_idx_text(frame.center_idx)}",
                *reader_lines,
            ]
        )

        self.frame_info.setUpdatesEnabled(False)
        self.frame_info.setPlainText(text)
        self.frame_info.setUpdatesEnabled(True)

        QtCore.QTimer.singleShot(
            0,
            lambda: self._restore_frame_info_scroll(previous_scroll_value, was_at_bottom),
        )

    def _restore_frame_info_scroll(self, previous_scroll_value: int, was_at_bottom: bool) -> None:
        scroll_bar = self.frame_info.verticalScrollBar()
        if was_at_bottom:
            scroll_bar.setValue(scroll_bar.maximum())
        else:
            scroll_bar.setValue(min(previous_scroll_value, scroll_bar.maximum()))

    def _save_npz(self) -> None:
        if not self._frames:
            return

        default_name = "camera_detector_replay.npz"
        path_text, _ = QtWidgets.QFileDialog.getSaveFileName(
            self,
            "Save replay data",
            str(Path.cwd() / default_name),
            "NumPy compressed (*.npz);;All files (*.*)",
        )
        if not path_text:
            return

        data = self._frames_to_arrays()
        np.savez_compressed(path_text, **data)

    def _load_npz(self) -> None:
        path_text, _ = QtWidgets.QFileDialog.getOpenFileName(
            self,
            "Load replay data",
            str(Path.cwd()),
            "NumPy compressed (*.npz);;All files (*.*)",
        )
        if not path_text:
            return

        loaded = np.load(path_text, allow_pickle=False)
        self._frames = self._arrays_to_frames(loaded)
        self._recorded_sequences = {frame.sequence for frame in self._frames}
        self._recording = False
        self.record_button.setEnabled(True)
        self.record_length_combo.setEnabled(True)
        self.record_button.setText(f"Record {int(self._record_seconds)} s")
        self._finalize_recording()

    def _save_csv(self) -> None:
        if not self._frames:
            return

        default_name = "camera_detector_replay.csv"
        path_text, _ = QtWidgets.QFileDialog.getSaveFileName(
            self,
            "Export replay CSV",
            str(Path.cwd() / default_name),
            "CSV files (*.csv);;All files (*.*)",
        )
        if not path_text:
            return

        with open(path_text, "w", newline="", encoding="utf-8") as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(
                [
                    "row",
                    "sequence",
                    "teensy_time_ms",
                    "host_time_s",
                    "source",
                    "valid",
                    "status",
                    "status_name",
                    "feature",
                    "feature_name",
                    "confidence",
                    "error",
                    "error_pct",
                    "left_idx",
                    "right_idx",
                    "center_idx",
                    "flags",
                    *[f"px{pixel}" for pixel in range(SAMPLE_COUNT)],
                ]
            )
            for row_index, frame in enumerate(self._frames):
                writer.writerow(
                    [
                        row_index,
                        frame.sequence,
                        frame.teensy_time_ms,
                        f"{frame.host_time_s:.9f}",
                        frame.source,
                        int(frame.valid),
                        frame.status,
                        frame.status_name,
                        frame.feature,
                        frame.feature_name,
                        frame.confidence,
                        f"{frame.error:.9f}",
                        f"{frame.error_pct:.6f}",
                        frame.left_idx,
                        frame.right_idx,
                        frame.center_idx,
                        frame.flags,
                        *[int(value) for value in frame.values.tolist()],
                    ]
                )

    def _save_png(self) -> None:
        if not self._frames:
            return

        if self._last_image_u8 is None or self._last_render_pixmap is None:
            self._render_replay_image(current_index=self._current_index)

        default_name = "camera_detector_replay.png"
        path_text, _ = QtWidgets.QFileDialog.getSaveFileName(
            self,
            "Save replay image",
            str(Path.cwd() / default_name),
            "PNG files (*.png);;All files (*.*)",
        )
        if not path_text:
            return

        pixmap = self._build_overlay_pixmap(
            self._last_image_u8,
            current_index=self._current_index if self._current_index >= 0 else None,
        )
        pixmap.save(path_text)

    def _frames_to_arrays(self) -> dict[str, np.ndarray]:
        values = np.vstack([frame.values for frame in self._frames]).astype(np.uint16)
        return {
            "values": values,
            "sequence": np.asarray([frame.sequence for frame in self._frames], dtype=np.uint32),
            "teensy_time_ms": np.asarray([frame.teensy_time_ms for frame in self._frames], dtype=np.uint32),
            "host_time_s": np.asarray([frame.host_time_s for frame in self._frames], dtype=np.float64),
            "source": np.asarray([frame.source for frame in self._frames], dtype="U32"),
            "valid": np.asarray([frame.valid for frame in self._frames], dtype=np.bool_),
            "status": np.asarray([frame.status for frame in self._frames], dtype=np.int16),
            "status_name": np.asarray([frame.status_name for frame in self._frames], dtype="U32"),
            "feature": np.asarray([frame.feature for frame in self._frames], dtype=np.int16),
            "feature_name": np.asarray([frame.feature_name for frame in self._frames], dtype="U32"),
            "confidence": np.asarray([frame.confidence for frame in self._frames], dtype=np.uint8),
            "error": np.asarray([frame.error for frame in self._frames], dtype=np.float32),
            "error_pct": np.asarray([frame.error_pct for frame in self._frames], dtype=np.float32),
            "left_idx": np.asarray([frame.left_idx for frame in self._frames], dtype=np.int16),
            "right_idx": np.asarray([frame.right_idx for frame in self._frames], dtype=np.int16),
            "center_idx": np.asarray([frame.center_idx for frame in self._frames], dtype=np.int16),
            "flags": np.asarray([frame.flags for frame in self._frames], dtype=np.uint8),
        }

    def _arrays_to_frames(self, loaded) -> list[ReplayFrame]:
        values = loaded["values"]
        count = values.shape[0]
        frames: list[ReplayFrame] = []
        for i in range(count):
            frames.append(
                ReplayFrame(
                    values=values[i].astype(np.uint16, copy=True),
                    sequence=int(loaded["sequence"][i]),
                    teensy_time_ms=int(loaded["teensy_time_ms"][i]),
                    host_time_s=float(loaded["host_time_s"][i]),
                    source=str(loaded["source"][i]),
                    valid=bool(loaded["valid"][i]),
                    status=int(loaded["status"][i]),
                    status_name=str(loaded["status_name"][i]),
                    feature=int(loaded["feature"][i]),
                    feature_name=str(loaded["feature_name"][i]),
                    confidence=int(loaded["confidence"][i]),
                    error=float(loaded["error"][i]),
                    error_pct=float(loaded["error_pct"][i]),
                    left_idx=int(loaded["left_idx"][i]),
                    right_idx=int(loaded["right_idx"][i]),
                    center_idx=int(loaded["center_idx"][i]),
                    flags=int(loaded["flags"][i]),
                )
            )
        return frames

    def _update_button_states(self) -> None:
        has_frames = bool(self._frames)
        self.play_button.setEnabled(has_frames)
        self.step_back_button.setEnabled(has_frames)
        self.step_forward_button.setEnabled(has_frames)
        self.save_npz_button.setEnabled(has_frames)
        self.save_csv_button.setEnabled(has_frames)
        self.save_png_button.setEnabled(has_frames)

    def _record_display_time(self) -> None:
        now_s = time.monotonic()
        self._display_times.append(now_s)
        if len(self._display_times) > 120:
            self._display_times = self._display_times[-120:]


def _frame_values(frame: CameraFrame) -> tuple[np.ndarray, str]:
    if frame.values_raw is not None:
        return frame.values_raw, "values_raw_12bit"
    return frame.values8.astype(np.uint16), "values8_fallback"


def _replay_frame_from_camera_frame(
    frame: CameraFrame,
    values: np.ndarray,
    source_name: str,
) -> ReplayFrame:
    vision = getattr(frame, "vision", None)
    return ReplayFrame(
        values=values.astype(np.uint16, copy=True),
        sequence=int(getattr(frame, "sequence", 0)),
        teensy_time_ms=int(getattr(frame, "teensy_time_ms", 0)),
        host_time_s=float(getattr(frame, "host_time_s", 0.0)),
        source=source_name,
        valid=bool(getattr(vision, "valid", False)),
        status=int(getattr(vision, "status", 0)),
        status_name=str(getattr(vision, "status_name", "UNKNOWN")),
        feature=int(getattr(vision, "feature", 0)),
        feature_name=str(getattr(vision, "feature_name", "NONE")),
        confidence=int(getattr(vision, "confidence", 0)),
        error=float(getattr(vision, "error", 0.0)),
        error_pct=float(getattr(vision, "error_pct", 0.0)),
        left_idx=_safe_idx(getattr(vision, "left_raw_idx", INVALID_IDX)),
        right_idx=_safe_idx(getattr(vision, "right_raw_idx", INVALID_IDX)),
        center_idx=_safe_idx(getattr(vision, "center_raw_idx", INVALID_IDX)),
        flags=int(getattr(vision, "flags", 0)),
    )


def _safe_idx(value) -> int:
    try:
        idx = int(value)
    except (TypeError, ValueError):
        return INVALID_IDX
    if idx < 0 or idx >= SAMPLE_COUNT:
        return INVALID_IDX
    return idx


def _idx_text(idx: int) -> str:
    if idx < 0 or idx >= SAMPLE_COUNT:
        return "none"
    return str(idx)


def _gray_qimage_from_array(image_u8: np.ndarray) -> QtGui.QImage:
    contiguous = np.ascontiguousarray(image_u8)
    height, width = contiguous.shape
    bytes_per_line = width
    qimage = QtGui.QImage(
        contiguous.data,
        width,
        height,
        bytes_per_line,
        QtGui.QImage.Format.Format_Grayscale8,
    )
    return qimage.copy()


def _center_pen(color: str):
    pen = pg.mkPen(color, width=2, style=QtCore.Qt.PenStyle.CustomDashLine)
    pen.setDashPattern([5, 12])
    return pen


def run_gui(
    *,
    store: LatestFrameStore,
    port_label: str,
    display_hz: float,
    history_size: int,
) -> int:
    pg.setConfigOptions(antialias=False)
    app = QtWidgets.QApplication.instance() or QtWidgets.QApplication([])
    window = DetectorReplayWindow(
        store=store,
        port_label=port_label,
        display_hz=display_hz,
        history_size=history_size,
    )
    window.show()
    return app.exec()
