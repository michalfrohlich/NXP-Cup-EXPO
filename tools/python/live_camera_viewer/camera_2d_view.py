"""Camera 2D scan view for the live camera viewer.

This GUI ignores vision/line-detection output. It records camera lines for a
fixed 3 second window and renders them into a static QImage/QPixmap.
The live line plot is shown on the left and the 2D scan image on the right.
The line plot is intentionally wider than the first side-by-side version.

Data convention:
- Prefer frame.values_raw when available. That preserves the original 12-bit ADC
  samples from the binary TCM1 stream.
- Fall back to frame.values8 only if the incoming stream does not carry raw ADC
  values.
- CSV export saves the recorded numeric values, not display-scaled pixels.

Display convention:
- Each received camera frame is one 1-pixel-high horizontal line.
- No blank spacer rows are inserted.
- The first recorded line is at the bottom.
- The last recorded line is at the top.
- The screen image is auto-contrasted to 8-bit grayscale for visibility only.
"""

from __future__ import annotations

import csv
import time
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
RECORD_SECONDS = 3.0

# Display-only nearest-neighbor enlargement. The saved CSV is unaffected.
DISPLAY_X_SCALE = 6
DISPLAY_Y_SCALE = 2


class Camera2DViewWindow(QtWidgets.QMainWindow):
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
        self._record_start_s = 0.0
        self._recorded_rows: list[np.ndarray] = []
        self._recorded_sources: list[str] = []
        self._recorded_sequences: set[int] = set()
        self._last_image_u8: np.ndarray | None = None
        self._last_contrast_limits: tuple[float, float] = (0.0, 255.0)
        self._last_source_name = "none"

        self.setWindowTitle(f"Camera 2D scan view - {port_label}")
        self.resize(1500, 760)

        central = QtWidgets.QWidget()
        root = QtWidgets.QVBoxLayout(central)
        root.setContentsMargins(6, 6, 6, 6)
        root.setSpacing(6)
        self.setCentralWidget(central)

        controls = QtWidgets.QHBoxLayout()
        root.addLayout(controls)

        self.record_button = QtWidgets.QPushButton("Record 3 s scan")
        self.record_button.clicked.connect(self._start_recording)
        controls.addWidget(self.record_button)

        self.clear_button = QtWidgets.QPushButton("Clear")
        self.clear_button.clicked.connect(self._clear_recording)
        controls.addWidget(self.clear_button)

        self.save_csv_button = QtWidgets.QPushButton("Save CSV")
        self.save_csv_button.clicked.connect(self._save_csv)
        controls.addWidget(self.save_csv_button)

        self.save_png_button = QtWidgets.QPushButton("Save PNG")
        self.save_png_button.clicked.connect(self._save_png)
        controls.addWidget(self.save_png_button)

        self.status_label = QtWidgets.QLabel(f"Idle | {port_label}")
        controls.addWidget(self.status_label, stretch=1)

        views = QtWidgets.QHBoxLayout()
        views.setSpacing(8)
        root.addLayout(views, stretch=1)

        self.current_plot = pg.PlotWidget()
        self.current_plot.setTitle("Current camera line")
        self.current_plot.setLabel("bottom", "Pixel")
        self.current_plot.setLabel("left", "Camera value")
        self.current_plot.setXRange(-0.5, SAMPLE_COUNT - 0.5)
        self.current_plot.setYRange(0, ADC_12BIT_MAX)
        self.current_plot.showGrid(x=True, y=True, alpha=0.25)
        self.current_plot.setMinimumWidth(650)
        self.current_plot.setMaximumWidth(850)
        self.current_curve = self.current_plot.plot(
            PIXELS,
            np.zeros(SAMPLE_COUNT),
            pen=pg.mkPen(width=2),
        )
        views.addWidget(self.current_plot, stretch=3)

        self.image_label = QtWidgets.QLabel()
        self.image_label.setAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
        self.image_label.setMinimumSize(SAMPLE_COUNT * DISPLAY_X_SCALE, 560)
        self.image_label.setStyleSheet("QLabel { background: #111; border: 1px solid #555; }")
        self.image_label.setText("Press 'Record 3 s scan' to build a 2D camera image")

        self.image_scroll = QtWidgets.QScrollArea()
        self.image_scroll.setWidgetResizable(True)
        self.image_scroll.setWidget(self.image_label)
        views.addWidget(self.image_scroll, stretch=5)

        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self._refresh)
        interval_ms = max(1, int(round(1000.0 / max(1.0, display_hz))))
        self.timer.start(interval_ms)

    def _start_recording(self) -> None:
        self._recorded_rows.clear()
        self._recorded_sources.clear()
        self._recorded_sequences.clear()
        self._last_image_u8 = None
        self._last_source_name = "none"
        self._record_start_s = time.monotonic()
        self._recording = True
        self.record_button.setEnabled(False)
        self.record_button.setText("Recording...")
        self._render_static_scan_image()

    def _stop_recording(self) -> None:
        self._recording = False
        self.record_button.setEnabled(True)
        self.record_button.setText("Record 3 s scan")

    def _clear_recording(self) -> None:
        if self._recording:
            self._stop_recording()
        self._recorded_rows.clear()
        self._recorded_sources.clear()
        self._recorded_sequences.clear()
        self._last_image_u8 = None
        self._last_source_name = "none"
        self.image_label.clear()
        self.image_label.setText("Press 'Record 3 s scan' to build a 2D camera image")
        self._set_idle_status(None)

    def _refresh(self) -> None:
        latest, history, reader_stats = self.store.snapshot()
        now_s = time.monotonic()

        if latest is None:
            self.status_label.setText(f"Waiting for camera frames | {self.port_label}")
            return

        self.latest_frame = latest
        self._record_display_time()

        latest_values, source_name = self._frame_values(latest)
        self._last_source_name = source_name
        self.current_curve.setData(PIXELS, latest_values.astype(float))
        self._update_current_plot_range(latest_values, source_name)

        if self._recording:
            elapsed_s = now_s - self._record_start_s
            new_rows = self._record_new_history_rows(history)
            if new_rows > 0:
                self._render_static_scan_image()

            if elapsed_s >= RECORD_SECONDS:
                self._stop_recording()
                elapsed_s = RECORD_SECONDS
                self._render_static_scan_image()
        else:
            elapsed_s = 0.0

        self._set_status_text(latest, reader_stats, elapsed_s)

    def _record_new_history_rows(self, history: list[CameraFrame]) -> int:
        added = 0
        for frame in history:
            if frame.sequence in self._recorded_sequences:
                continue
            if frame.host_time_s < self._record_start_s:
                continue
            if (frame.host_time_s - self._record_start_s) > RECORD_SECONDS:
                continue

            values, source_name = self._frame_values(frame)
            self._recorded_rows.append(values.astype(np.uint16, copy=True))
            self._recorded_sources.append(source_name)
            self._recorded_sequences.add(frame.sequence)
            self._last_source_name = source_name
            added += 1
        return added

    def _render_static_scan_image(self) -> None:
        if not self._recorded_rows:
            self.image_label.clear()
            self.image_label.setText("Recording... waiting for rows")
            return

        image_u8 = self._build_display_image_u8()
        self._last_image_u8 = image_u8

        qimage = _gray_qimage_from_array(image_u8)
        pixmap = QtGui.QPixmap.fromImage(qimage)

        scaled = pixmap.scaled(
            image_u8.shape[1] * DISPLAY_X_SCALE,
            max(1, image_u8.shape[0] * DISPLAY_Y_SCALE),
            QtCore.Qt.AspectRatioMode.IgnoreAspectRatio,
            QtCore.Qt.TransformationMode.FastTransformation,
        )

        self.image_label.setPixmap(scaled)
        self.image_label.setMinimumSize(scaled.size())

    def _build_display_image_u8(self) -> np.ndarray:
        rows = np.vstack(self._recorded_rows).astype(np.float32)

        # Robust auto-contrast for display only. The stored rows and CSV remain
        # unscaled uint16 values.
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

        # QLabel/QImage row 0 is the top of the image. We want the newest row on
        # top and the oldest row at the bottom, so reverse chronological order.
        return normalized[::-1].copy()

    def _save_csv(self) -> None:
        if not self._recorded_rows:
            return

        default_name = "camera_2d_scan.csv"
        path_text, _ = QtWidgets.QFileDialog.getSaveFileName(
            self,
            "Save recorded 2D scan",
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
                    "source",
                    *[f"px{pixel}" for pixel in range(SAMPLE_COUNT)],
                ]
            )
            for row_index, row in enumerate(self._recorded_rows):
                source = self._recorded_sources[row_index]
                writer.writerow(
                    [
                        row_index,
                        source,
                        *[int(value) for value in row.tolist()],
                    ]
                )

    def _save_png(self) -> None:
        if self._last_image_u8 is None:
            if not self._recorded_rows:
                return
            self._last_image_u8 = self._build_display_image_u8()

        default_name = "camera_2d_scan.png"
        path_text, _ = QtWidgets.QFileDialog.getSaveFileName(
            self,
            "Save recorded 2D scan image",
            str(Path.cwd() / default_name),
            "PNG files (*.png);;All files (*.*)",
        )
        if not path_text:
            return

        qimage = _gray_qimage_from_array(self._last_image_u8)
        qimage.save(path_text)

    def _frame_values(self, frame: CameraFrame) -> tuple[np.ndarray, str]:
        if frame.values_raw is not None:
            return frame.values_raw, "values_raw_12bit"

        # Fallback only. This happens if the stream/parser produced only values8,
        # for example from a text frame8 stream instead of binary raw TCM1.
        return frame.values8.astype(np.uint16), "values8_fallback"

    def _update_current_plot_range(self, values: np.ndarray, source_name: str) -> None:
        if source_name == "values_raw_12bit":
            self.current_plot.setYRange(0, ADC_12BIT_MAX)
            return

        maximum = int(np.max(values)) if values.size else ADC_8BIT_MAX
        self.current_plot.setYRange(0, max(ADC_8BIT_MAX, maximum))

    def _set_status_text(
        self,
        frame: CameraFrame,
        reader_stats: ReaderStats,
        elapsed_s: float,
    ) -> None:
        if self._recording:
            remaining_s = max(0.0, RECORD_SECONDS - elapsed_s)
            state = f"Recording {elapsed_s:0.1f}/{RECORD_SECONDS:0.1f}s, {remaining_s:0.1f}s left"
        else:
            state = "Idle"

        lo, hi = self._last_contrast_limits
        self.status_label.setText(
            f"{state} | "
            f"rows {len(self._recorded_rows)} | "
            f"source {self._last_source_name} | "
            f"contrast {lo:0.0f}..{hi:0.0f} | "
            f"seq {frame.sequence} | "
            f"RX {reader_stats.receive_fps:0.1f} fps | "
            f"drops {reader_stats.estimated_dropped_frames} | "
            f"{self.port_label}"
        )

    def _set_idle_status(self, frame: CameraFrame | None) -> None:
        seq_text = "none" if frame is None else str(frame.sequence)
        self.status_label.setText(
            f"Idle | rows {len(self._recorded_rows)} | seq {seq_text} | {self.port_label}"
        )

    def _record_display_time(self) -> None:
        now_s = time.monotonic()
        self._display_times.append(now_s)
        if len(self._display_times) > 120:
            self._display_times = self._display_times[-120:]


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
    # copy() detaches the QImage from the temporary numpy buffer.
    return qimage.copy()


def run_gui(
    *,
    store: LatestFrameStore,
    port_label: str,
    display_hz: float,
    history_size: int,
) -> int:
    pg.setConfigOptions(antialias=False)
    app = QtWidgets.QApplication.instance() or QtWidgets.QApplication([])
    window = Camera2DViewWindow(
        store=store,
        port_label=port_label,
        display_hz=display_hz,
        history_size=history_size,
    )
    window.show()
    return app.exec()
