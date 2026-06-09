"""PySide6/pyqtgraph GUI for the live camera viewer."""

from __future__ import annotations

import csv
import time
from pathlib import Path

import numpy as np
import pyqtgraph as pg
from PySide6 import QtCore, QtWidgets

from .camera_frame import CameraFrame, ReaderStats, counters_as_text
from .config import (
    DEFAULT_DISPLAY_HZ,
    DEFAULT_HISTORY,
    SAMPLE_COUNT,
    VISION_INVALID_IDX,
    VISION_TRIM_LEFT_PX,
)
from .serial_reader import LatestFrameStore


PIXELS = np.arange(SAMPLE_COUNT)


class CameraViewerWindow(QtWidgets.QMainWindow):
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
        self.is_paused = False
        self.latest_frame: CameraFrame | None = None
        self._display_times: list[float] = []

        self.setWindowTitle(f"Teensy Camera Live Viewer - {port_label}")
        self.resize(1180, 760)

        central = QtWidgets.QWidget()
        root = QtWidgets.QVBoxLayout(central)
        self.setCentralWidget(central)

        controls = QtWidgets.QHBoxLayout()
        root.addLayout(controls)

        self.pause_button = QtWidgets.QPushButton("Pause")
        self.pause_button.setCheckable(True)
        self.pause_button.clicked.connect(self._toggle_pause)
        controls.addWidget(self.pause_button)

        autoscale_button = QtWidgets.QPushButton("Autoscale")
        autoscale_button.clicked.connect(self._autoscale)
        controls.addWidget(autoscale_button)

        save_button = QtWidgets.QPushButton("Save CSV")
        save_button.clicked.connect(self._save_csv)
        controls.addWidget(save_button)

        controls.addStretch(1)

        body = QtWidgets.QHBoxLayout()
        root.addLayout(body, stretch=1)

        plots = QtWidgets.QVBoxLayout()
        body.addLayout(plots, stretch=3)

        self.signal_plot = pg.PlotWidget()
        self.signal_plot.setLabel("bottom", "Pixel")
        self.signal_plot.setLabel("left", "raw camera", units="ADC counts")
        self.signal_plot.setXRange(0, SAMPLE_COUNT - 1)
        self.signal_plot.setYRange(0, 4095)
        self.signal_plot.showGrid(x=True, y=True, alpha=0.25)
        self.signal_curve = self.signal_plot.plot(
            PIXELS,
            np.zeros(SAMPLE_COUNT),
            pen=pg.mkPen("#1976d2", width=2),
            name="raw ADC",
        )
        self.left_line = pg.InfiniteLine(
            angle=90, movable=False, pen=pg.mkPen("#2e7d32", width=2)
        )
        self.right_line = pg.InfiniteLine(
            angle=90, movable=False, pen=pg.mkPen("#c62828", width=2)
        )
        self.left_line.hide()
        self.right_line.hide()
        self.signal_plot.addItem(self.left_line)
        self.signal_plot.addItem(self.right_line)
        plots.addWidget(self.signal_plot, stretch=3)

        self.history_plot = pg.PlotWidget()
        self.history_plot.setLabel("bottom", "Frame sequence")
        self.history_plot.setLabel("left", "Stats", units="ADC counts")
        self.history_plot.showGrid(x=True, y=True, alpha=0.25)
        self.history_min_curve = self.history_plot.plot(
            [], [], pen=pg.mkPen("#607d8b", width=1), name="min"
        )
        self.history_avg_curve = self.history_plot.plot(
            [], [], pen=pg.mkPen("#f57c00", width=2), name="avg"
        )
        self.history_max_curve = self.history_plot.plot(
            [], [], pen=pg.mkPen("#d32f2f", width=1), name="max"
        )
        self.history_plot.addLegend()
        plots.addWidget(self.history_plot, stretch=1)

        side = QtWidgets.QVBoxLayout()
        body.addLayout(side, stretch=1)

        self.stats_text = QtWidgets.QPlainTextEdit()
        self.stats_text.setReadOnly(True)
        self.stats_text.setLineWrapMode(QtWidgets.QPlainTextEdit.NoWrap)
        font = self.stats_text.font()
        font.setFamily("Consolas")
        self.stats_text.setFont(font)
        side.addWidget(self.stats_text, stretch=1)

        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self._refresh)
        interval_ms = max(1, int(round(1000.0 / max(1.0, display_hz))))
        self.timer.start(interval_ms)

    def _toggle_pause(self, checked: bool) -> None:
        self.is_paused = checked
        self.pause_button.setText("Resume" if checked else "Pause")

    def _autoscale(self) -> None:
        self.signal_plot.setXRange(0, SAMPLE_COUNT - 1)
        self.signal_plot.setYRange(0, 4095)
        self.history_plot.enableAutoRange()

    def _save_csv(self) -> None:
        if self.latest_frame is None:
            return

        default_name = f"camera_frame_{self.latest_frame.sequence}.csv"
        path_text, _ = QtWidgets.QFileDialog.getSaveFileName(
            self,
            "Save current frame",
            str(Path.cwd() / default_name),
            "CSV files (*.csv);;All files (*.*)",
        )
        if not path_text:
            return

        with open(path_text, "w", newline="", encoding="utf-8") as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(["pixel", "raw_adc"])
            values = self._frame_plot_values(self.latest_frame)
            for pixel, value in enumerate(values.tolist()):
                writer.writerow([pixel, int(value)])

    def _refresh(self) -> None:
        latest, history, reader_stats = self.store.snapshot()
        if latest is None:
            self._set_waiting_text(reader_stats)
            return

        self.latest_frame = latest
        self._record_display_time()

        if not self.is_paused:
            self.signal_curve.setData(PIXELS, self._frame_plot_values(latest).astype(float))
            self._update_detector_overlay(latest)
            self._update_history(history)

        self.stats_text.setPlainText(self._build_stats_text(latest, reader_stats))

    def _set_waiting_text(self, reader_stats: ReaderStats) -> None:
        self.stats_text.setPlainText(
            "Waiting for camera frames\n\n"
            f"port: {self.port_label}\n"
            f"lines: {reader_stats.lines_read}\n"
            f"parse errors: {reader_stats.parse_errors}\n"
            f"last error: {reader_stats.last_error}"
        )

    def _update_history(self, history: list[CameraFrame]) -> None:
        if not history:
            return

        visible_history = history[-self.history_size :]
        x = np.asarray([frame.sequence for frame in visible_history], dtype=float)
        mins = np.asarray([int(np.min(self._frame_plot_values(frame))) for frame in visible_history], dtype=float)
        avgs = np.asarray([float(np.mean(self._frame_plot_values(frame))) for frame in visible_history], dtype=float)
        maxs = np.asarray([int(np.max(self._frame_plot_values(frame))) for frame in visible_history], dtype=float)

        self.history_min_curve.setData(x, mins)
        self.history_avg_curve.setData(x, avgs)
        self.history_max_curve.setData(x, maxs)

    def _record_display_time(self) -> None:
        now_s = time.monotonic()
        self._display_times.append(now_s)
        if len(self._display_times) > 120:
            self._display_times = self._display_times[-120:]

    def _display_fps(self) -> float:
        if len(self._display_times) < 2:
            return 0.0
        elapsed = self._display_times[-1] - self._display_times[0]
        if elapsed <= 0.0:
            return 0.0
        return (len(self._display_times) - 1) / elapsed

    def _frame_plot_values(self, frame: CameraFrame) -> np.ndarray:
        return frame.values_raw if frame.values_raw is not None else frame.values8

    def _update_detector_overlay(self, frame: CameraFrame) -> None:
        counters = frame.counters
        valid = bool(counters.get("visionFlags", 0) & 0x01)
        if not valid:
            self.left_line.hide()
            self.right_line.hide()
            return

        self._set_detector_line(self.left_line, counters.get("visionLeftLineIdx"))
        self._set_detector_line(self.right_line, counters.get("visionRightLineIdx"))

    def _set_detector_line(self, line: pg.InfiniteLine, detector_idx: int | None) -> None:
        if detector_idx is None or detector_idx == VISION_INVALID_IDX:
            line.hide()
            return

        raw_idx = detector_idx + VISION_TRIM_LEFT_PX
        if raw_idx < 0 or raw_idx >= SAMPLE_COUNT:
            line.hide()
            return

        line.setPos(raw_idx)
        line.show()

    def _build_stats_text(self, frame: CameraFrame, reader_stats: ReaderStats) -> str:
        counters = frame.counters
        detector_text = self._build_detector_text(counters)
        return "\n".join(
            [
                "Signal",
                "  raw LinearCameraFrame, 128 ADC samples, 0..4095",
                "",
                "Frame",
                f"  seq:        {frame.sequence}",
                f"  teensy ms:  {frame.teensy_time_ms}",
                f"  age ms:     {frame.frame_age_s * 1000.0:7.1f}",
                f"  seq delta:  {frame.sequence_delta}",
                "",
                "Camera Counters",
                f"  {counters_as_text(counters)}",
                "",
                "Detector",
                f"  {detector_text}",
                "",
                "Timing",
                f"  serial fps:  {reader_stats.receive_fps:7.2f}",
                f"  display fps: {self._display_fps():7.2f}",
                f"  lines:       {reader_stats.lines_read}",
                f"  frames:      {reader_stats.parsed_frames}",
                "",
                "Reader Health",
                f"  ignored lines:    {reader_stats.ignored_lines}",
                f"  malformed lines:  {reader_stats.malformed_lines}",
                f"  parse errors:     {reader_stats.parse_errors}",
                f"  duplicates:       {reader_stats.duplicate_frames}",
                f"  estimated drops:  {reader_stats.estimated_dropped_frames}",
                f"  last error:       {reader_stats.last_error}",
            ]
        )

    def _build_detector_text(self, counters: dict[str, int]) -> str:
        if "visionFlags" not in counters:
            return "not in stream"

        valid = bool(counters.get("visionFlags", 0) & 0x01)
        status = _vision_status_name(counters.get("visionStatus", 0))
        feature = _vision_feature_name(counters.get("visionFeature", 0))
        error = counters.get("visionErrorMilli", 0) / 1000.0
        confidence = counters.get("visionConfidence", 0)
        left_idx = counters.get("visionLeftLineIdx", VISION_INVALID_IDX)
        right_idx = counters.get("visionRightLineIdx", VISION_INVALID_IDX)
        left_text = _idx_text(left_idx)
        right_text = _idx_text(right_idx)
        return (
            f"valid={int(valid)} status={status} feature={feature} "
            f"conf={confidence} err={error:+.3f} left={left_text} right={right_text}"
        )


def _idx_text(idx: int) -> str:
    if idx == VISION_INVALID_IDX:
        return "none"
    return f"{idx}+{VISION_TRIM_LEFT_PX}"


def _vision_status_name(status: int) -> str:
    return {
        0: "lost",
        1: "both",
        2: "left",
        3: "right",
    }.get(status, str(status))


def _vision_feature_name(feature: int) -> str:
    return {
        0: "none",
        1: "finish",
    }.get(feature, str(feature))


def run_gui(
    *,
    store: LatestFrameStore,
    port_label: str,
    display_hz: float,
    history_size: int,
) -> int:
    pg.setConfigOptions(antialias=True)
    app = QtWidgets.QApplication.instance() or QtWidgets.QApplication([])
    window = CameraViewerWindow(
        store=store,
        port_label=port_label,
        display_hz=display_hz,
        history_size=history_size,
    )
    window.show()
    return app.exec()
