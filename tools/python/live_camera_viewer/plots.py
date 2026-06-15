"""PySide6/pyqtgraph GUI for the live camera viewer."""

from __future__ import annotations

import csv
import html
import time
from pathlib import Path

import numpy as np
import pyqtgraph as pg
from PySide6 import QtCore, QtWidgets

from .camera_frame import CameraFrame, ReaderStats, VisionResult, counters_as_text
from .config import (
    DEFAULT_DISPLAY_HZ,
    DEFAULT_HISTORY,
    SAMPLE_COUNT,
    VISION_INVALID_IDX,
    VISION_PROCESS_COUNT,
    VISION_TRIM_LEFT_PX,
    VISION_TRIM_RIGHT_PX,
)
from .serial_reader import LatestFrameStore


PIXELS = np.arange(SAMPLE_COUNT)
ADC_MAX = 4095

STATUS_COLORS = {
    0: "#757575",
    1: "#2e7d32",
    2: "#f9a825",
    3: "#f9a825",
}


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
        self._last_history_update_s = 0.0
        self._last_stats_update_s = 0.0
        self._last_waiting_update_s = 0.0

        self.setWindowTitle(f"Teensy Camera Vision Viewer - {port_label}")
        self.resize(1280, 840)

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

        self.mode_label = QtWidgets.QLabel("TCM1 compact vision stream")
        controls.addWidget(self.mode_label)
        controls.addStretch(1)

        body = QtWidgets.QHBoxLayout()
        root.addLayout(body, stretch=1)

        plots = QtWidgets.QVBoxLayout()
        body.addLayout(plots, stretch=3)

        self.signal_plot = pg.PlotWidget()
        self.signal_plot.setTitle("Raw Camera + Vision Output")
        self.signal_plot.setLabel("bottom", "Pixel")
        self.signal_plot.setLabel("left", "raw camera", units="ADC counts")
        self.signal_plot.setXRange(-0.5, SAMPLE_COUNT - 0.5)
        self.signal_plot.setYRange(0, ADC_MAX)
        self.signal_plot.showGrid(x=True, y=True, alpha=0.25)
        self.signal_plot.addLegend(offset=(10, 10))
        self._add_trim_regions()
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
            angle=90, movable=False, pen=pg.mkPen("#2e7d32", width=2)
        )
        self.center_line = pg.InfiniteLine(
            angle=90,
            movable=False,
            pen=_center_pen("#fb8c00"),
        )
        self.left_line.hide()
        self.right_line.hide()
        self.center_line.hide()
        self.signal_plot.addItem(self.left_line)
        self.signal_plot.addItem(self.right_line)
        self.signal_plot.addItem(self.center_line)
        plots.addWidget(self.signal_plot, stretch=4)

        self.vision_history_plot = pg.PlotWidget()
        self.vision_history_plot.setLabel("bottom", "Frame sequence")
        self.vision_history_plot.setLabel("left", "Vision", units="%")
        self.vision_history_plot.setYRange(-105, 105)
        self.vision_history_plot.showGrid(x=True, y=True, alpha=0.25)
        self.vision_history_plot.addLegend()
        self.vision_zero_line = pg.InfiniteLine(
            angle=0, movable=False, pen=pg.mkPen("#9e9e9e", width=1)
        )
        self.vision_history_plot.addItem(self.vision_zero_line)
        self.vision_error_curve = self.vision_history_plot.plot(
            [], [], pen=pg.mkPen("#5e35b1", width=2), name="error %"
        )
        self.vision_confidence_curve = self.vision_history_plot.plot(
            [], [], pen=pg.mkPen("#00897b", width=2), name="confidence"
        )
        self.finish_scatter = pg.ScatterPlotItem(
            [],
            [],
            pen=pg.mkPen("#8e24aa"),
            brush=pg.mkBrush("#8e24aa"),
            size=8,
            name="finish line",
        )
        self.vision_history_plot.addItem(self.finish_scatter)
        plots.addWidget(self.vision_history_plot, stretch=1)

        self.history_plot = pg.PlotWidget()
        self.history_plot.setLabel("bottom", "Frame sequence")
        self.history_plot.setLabel("left", "Raw stats", units="ADC counts")
        self.history_plot.setYRange(0, ADC_MAX)
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

        self.stats_text = QtWidgets.QTextEdit()
        self.stats_text.setReadOnly(True)
        self.stats_text.setLineWrapMode(QtWidgets.QTextEdit.LineWrapMode.NoWrap)
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
        self.signal_plot.setXRange(-0.5, SAMPLE_COUNT - 0.5)
        self.signal_plot.setYRange(0, ADC_MAX)
        self.vision_history_plot.enableAutoRange()
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
            vision = self.latest_frame.vision
            writer.writerow(["sequence", self.latest_frame.sequence])
            writer.writerow(["teensy_time_ms", self.latest_frame.teensy_time_ms])
            if vision.in_stream:
                writer.writerow(["vision_valid", int(vision.valid)])
                writer.writerow(["vision_status", vision.status_name])
                writer.writerow(["vision_feature", vision.feature_name])
                writer.writerow(["vision_confidence", vision.confidence])
                writer.writerow(["vision_error", f"{vision.error:.6f}"])
                writer.writerow(["vision_left_position", _idx_or_empty(vision.left_line_idx)])
                writer.writerow(["vision_right_position", _idx_or_empty(vision.right_line_idx)])
            writer.writerow([])
            writer.writerow(["pixel", "raw_adc", "scaled8"])
            values = self._frame_plot_values(self.latest_frame)
            values8 = self.latest_frame.values8
            for pixel, value in enumerate(values.tolist()):
                writer.writerow([pixel, int(value), int(values8[pixel])])

    def _refresh(self) -> None:
        now_s = time.monotonic()
        latest, history, reader_stats = self.store.snapshot()
        if latest is None:
            if (now_s - self._last_waiting_update_s) >= 0.25:
                self._last_waiting_update_s = now_s
                self._set_waiting_text(reader_stats)
            return

        self.latest_frame = latest
        self._record_display_time()

        if not self.is_paused:
            self.signal_curve.setData(PIXELS, self._frame_plot_values(latest).astype(float))
            self._update_detector_overlay(latest)
            if (now_s - self._last_history_update_s) >= 0.10:
                self._last_history_update_s = now_s
                self._update_history(history)

        if (now_s - self._last_stats_update_s) >= 0.20:
            self._last_stats_update_s = now_s
            self.stats_text.setHtml(self._build_stats_html(latest, reader_stats))

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
        mins = np.asarray(
            [int(np.min(self._frame_plot_values(frame))) for frame in visible_history],
            dtype=float,
        )
        avgs = np.asarray(
            [float(np.mean(self._frame_plot_values(frame))) for frame in visible_history],
            dtype=float,
        )
        maxs = np.asarray(
            [int(np.max(self._frame_plot_values(frame))) for frame in visible_history],
            dtype=float,
        )

        self.history_min_curve.setData(x, mins)
        self.history_avg_curve.setData(x, avgs)
        self.history_max_curve.setData(x, maxs)
        self._update_vision_history(visible_history)

    def _update_vision_history(self, history: list[CameraFrame]) -> None:
        vision_history = [frame for frame in history if frame.vision.in_stream]
        if not vision_history:
            self.vision_error_curve.setData([], [])
            self.vision_confidence_curve.setData([], [])
            self.finish_scatter.setData([], [])
            return

        x = np.asarray([frame.sequence for frame in vision_history], dtype=float)
        errors = np.asarray([frame.vision.error_pct for frame in vision_history], dtype=float)
        confidence = np.asarray(
            [float(frame.vision.confidence) if frame.vision.valid else np.nan
             for frame in vision_history],
            dtype=float,
        )
        finish_x = [
            float(frame.sequence)
            for frame in vision_history
            if frame.vision.valid and frame.vision.feature == 1
        ]
        finish_y = [
            float(frame.vision.confidence)
            for frame in vision_history
            if frame.vision.valid and frame.vision.feature == 1
        ]

        self.vision_error_curve.setData(x, errors)
        self.vision_confidence_curve.setData(x, confidence)
        self.finish_scatter.setData(finish_x, finish_y)

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
        vision = frame.vision
        if not vision.in_stream or not vision.valid:
            self.left_line.hide()
            self.right_line.hide()
            self.center_line.hide()
            return

        center_color = "#fb8c00" if vision.status == 1 else "#9e9e9e"
        self.center_line.setPen(_center_pen(center_color))
        self._set_detector_line(self.left_line, vision.left_raw_idx)
        self._set_detector_line(self.right_line, vision.right_raw_idx)
        self._set_detector_line(self.center_line, vision.center_raw_idx)

    def _set_detector_line(self, line: pg.InfiniteLine, raw_idx: int | float | None) -> None:
        if raw_idx is None:
            line.hide()
            return

        if raw_idx < 0 or raw_idx >= SAMPLE_COUNT:
            line.hide()
            return

        line.setPos(raw_idx)
        line.show()

    def _build_stats_text(self, frame: CameraFrame, reader_stats: ReaderStats) -> str:
        counters = frame.counters
        vision = frame.vision
        detector_text = self._build_detector_text(vision)
        return "\n".join(
            [
                "Signal",
                "  raw LinearCameraFrame, 128 ADC samples, 0..4095",
                f"  detector input: raw [{VISION_TRIM_LEFT_PX}..{SAMPLE_COUNT - VISION_TRIM_RIGHT_PX - 1}] -> {VISION_PROCESS_COUNT} px",
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
                "Vision Output",
                detector_text,
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

    def _build_stats_html(self, frame: CameraFrame, reader_stats: ReaderStats) -> str:
        counters = frame.counters
        vision = frame.vision
        text_before = "\n".join(
            [
                "Signal",
                "  raw LinearCameraFrame, 128 ADC samples, 0..4095",
                f"  detector input: raw [{VISION_TRIM_LEFT_PX}..{SAMPLE_COUNT - VISION_TRIM_RIGHT_PX - 1}] -> {VISION_PROCESS_COUNT} px",
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
                "Vision Output",
            ]
        )
        text_after = "\n".join(
            [
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
        return (
            '<pre style="font-family: Consolas, monospace; font-size: 10pt;">'
            f"{html.escape(text_before)}\n"
            f"{self._build_detector_html(vision)}\n"
            f"{html.escape(text_after)}"
            "</pre>"
        )

    def _build_detector_text(self, vision: VisionResult) -> str:
        if not vision.in_stream:
            return "  not in stream"

        return "\n".join(
            [
                f"  valid:      {int(vision.valid)}  flags=0x{vision.flags:02X}",
                f"  status:     {vision.status_name} ({vision.status})",
                f"  feature:    {vision.feature_name} ({vision.feature})",
                f"  confidence: {vision.confidence:3d}",
                f"  error:      {vision.error:+.3f} ({vision.error_pct:+.1f}%)",
                f"  left pos:   {_position_text(vision.left_line_idx)}",
                f"  right pos:  {_position_text(vision.right_line_idx)}",
                f"  center pos: {_center_position_text(vision)}",
            ]
        )

    def _build_detector_html(self, vision: VisionResult) -> str:
        if not vision.in_stream:
            return "  not in stream"

        status_color = _status_color(vision.status)
        confidence_color = _confidence_color(vision.confidence)
        return "\n".join(
            [
                f"  valid:      {int(vision.valid)}  flags=0x{vision.flags:02X}",
                f'  status:     <span style="color:{status_color}; font-weight:700;">{html.escape(vision.status_name)} ({vision.status})</span>',
                f"  feature:    {html.escape(vision.feature_name)} ({vision.feature})",
                f'  confidence: <span style="color:{confidence_color}; font-weight:700;">{vision.confidence:3d}</span>',
                f"  error:      {vision.error:+.3f} ({vision.error_pct:+.1f}%)",
                f"  left pos:   {html.escape(_position_text(vision.left_line_idx))}",
                f"  right pos:  {html.escape(_position_text(vision.right_line_idx))}",
                f"  center pos: {html.escape(_center_position_text(vision))}",
            ]
        )

    def _add_trim_regions(self) -> None:
        left_region = _trim_region(-0.5, VISION_TRIM_LEFT_PX - 0.5)
        right_region = _trim_region(
            SAMPLE_COUNT - VISION_TRIM_RIGHT_PX - 0.5,
            SAMPLE_COUNT - 0.5,
        )
        left_region.setZValue(-10)
        right_region.setZValue(-10)
        self.signal_plot.addItem(left_region)
        self.signal_plot.addItem(right_region)


def _idx_or_empty(idx: int) -> str:
    if idx == VISION_INVALID_IDX:
        return ""
    return str(idx)


def _position_text(position: int) -> str:
    if position == VISION_INVALID_IDX:
        return "none"
    return f"{position:3d}"


def _center_position_text(vision: VisionResult) -> str:
    if not vision.valid or vision.status == 0:
        return "none"
    detector_mid = (float(VISION_PROCESS_COUNT) - 1.0) / 2.0
    center_position = (vision.error * detector_mid) + detector_mid
    if center_position < 0.0 or center_position > float(VISION_PROCESS_COUNT - 1):
        return "none"
    return f"{center_position:5.1f}"


def _status_color(status: int) -> str:
    if status == 1:
        return "#2e7d32"
    if status in (2, 3):
        return "#ef6c00"
    return "#c62828"


def _confidence_color(confidence: int) -> str:
    value = max(0, min(100, int(confidence)))
    red = int(round(198 * (100 - value) / 100))
    green = int(round(180 * value / 100))
    blue = 40
    return f"#{red:02x}{green:02x}{blue:02x}"


def _center_pen(color: str):
    pen = pg.mkPen(color, width=2, style=QtCore.Qt.PenStyle.CustomDashLine)
    pen.setDashPattern([5, 12])
    return pen


def _trim_region(left: float, right: float) -> pg.LinearRegionItem:
    region = pg.LinearRegionItem(
        values=(left, right),
        movable=False,
        brush=pg.mkBrush(120, 120, 120, 24),
    )
    pen = pg.mkPen(120, 120, 120, 45)
    try:
        region.setHoverBrush(pg.mkBrush(120, 120, 120, 24))
    except AttributeError:
        pass
    for line in getattr(region, "lines", []):
        line.setPen(pen)
        try:
            line.setHoverPen(pen)
        except AttributeError:
            pass
    return region


def run_gui(
    *,
    store: LatestFrameStore,
    port_label: str,
    display_hz: float,
    history_size: int,
) -> int:
    pg.setConfigOptions(antialias=False)
    app = QtWidgets.QApplication.instance() or QtWidgets.QApplication([])
    window = CameraViewerWindow(
        store=store,
        port_label=port_label,
        display_hz=display_hz,
        history_size=history_size,
    )
    window.show()
    return app.exec()
