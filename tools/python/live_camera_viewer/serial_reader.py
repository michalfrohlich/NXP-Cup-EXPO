"""Background serial reader for the Teensy camera live viewer."""

from __future__ import annotations

import threading
import time
from collections import deque
from dataclasses import replace
from typing import Deque

from .camera_frame import CameraFrame, ReaderStats
from .config import (
    DEFAULT_BAUD,
    DEFAULT_HISTORY,
    DEFAULT_SERIAL_TIMEOUT_S,
)
from .packet_format import (
    MAGIC_BYTES,
    PACKET_HEADER_SIZE,
    expected_camera_packet_size,
    parse_camera_packet,
)


class LatestFrameStore:
    """Thread-safe latest-frame mailbox plus a bounded history buffer."""

    def __init__(self, history_size: int = DEFAULT_HISTORY) -> None:
        self._lock = threading.Lock()
        self._latest: CameraFrame | None = None
        self._history: Deque[CameraFrame] = deque(maxlen=max(1, history_size))
        self._rx_times: Deque[float] = deque(maxlen=120)

        self._lines_read = 0
        self._parsed_frames = 0
        self._ignored_lines = 0
        self._malformed_lines = 0
        self._parse_errors = 0
        self._duplicate_frames = 0
        self._estimated_dropped_frames = 0
        self._last_error = ""

    def record_line(self) -> None:
        with self._lock:
            self._lines_read += 1

    def record_ignored_line(self) -> None:
        with self._lock:
            self._ignored_lines += 1

    def record_malformed_line(self) -> None:
        with self._lock:
            self._malformed_lines += 1

    def record_parse_error(self, error: Exception) -> None:
        with self._lock:
            self._parse_errors += 1
            self._last_error = str(error)

    def put_frame(self, frame: CameraFrame) -> None:
        now_s = time.monotonic()
        with self._lock:
            previous = self._latest
            sequence_delta = 0
            if previous is not None:
                sequence_delta = frame.sequence - previous.sequence
                if sequence_delta == 0:
                    self._duplicate_frames += 1
                elif sequence_delta > 1:
                    self._estimated_dropped_frames += sequence_delta - 1

            self._rx_times.append(now_s)
            receive_fps = _rate_from_times(self._rx_times)

            frame.with_reader_stats(
                receive_fps=receive_fps,
                sequence_delta=sequence_delta,
                malformed_count=self._malformed_lines,
                parse_error_count=self._parse_errors,
                duplicate_count=self._duplicate_frames,
                estimated_drop_count=self._estimated_dropped_frames,
            )

            self._latest = frame
            self._history.append(frame)
            self._parsed_frames += 1

    def snapshot(self) -> tuple[CameraFrame | None, list[CameraFrame], ReaderStats]:
        with self._lock:
            latest = self._latest
            if latest is not None:
                latest = replace(latest, frame_age_s=time.monotonic() - latest.host_time_s)

            stats = ReaderStats(
                lines_read=self._lines_read,
                parsed_frames=self._parsed_frames,
                ignored_lines=self._ignored_lines,
                malformed_lines=self._malformed_lines,
                parse_errors=self._parse_errors,
                duplicate_frames=self._duplicate_frames,
                estimated_dropped_frames=self._estimated_dropped_frames,
                receive_fps=_rate_from_times(self._rx_times),
                last_error=self._last_error,
            )
            return latest, list(self._history), stats


class SerialCameraReader:
    """Owns a serial port and feeds parsed frames into a LatestFrameStore."""

    def __init__(
        self,
        *,
        port: str,
        baud: int = DEFAULT_BAUD,
        store: LatestFrameStore | None = None,
        timeout_s: float = DEFAULT_SERIAL_TIMEOUT_S,
    ) -> None:
        self.port = port
        self.baud = baud
        self.timeout_s = timeout_s
        self.store = store if store is not None else LatestFrameStore()
        self._stop_event = threading.Event()
        self._thread = threading.Thread(target=self._run, name="camera-serial-reader", daemon=True)

    def start(self) -> None:
        self._thread.start()

    def stop(self) -> None:
        self._stop_event.set()

    def join(self, timeout_s: float = 2.0) -> None:
        self._thread.join(timeout=timeout_s)

    def _run(self) -> None:
        try:
            import serial
        except ImportError as error:
            self.store.record_parse_error(error)
            return

        try:
            with serial.Serial(self.port, self.baud, timeout=self.timeout_s) as serial_port:
                serial_port.reset_input_buffer()
                rx_buffer = bytearray()
                while not self._stop_event.is_set():
                    read_size = max(1, min(4096, serial_port.in_waiting or 1))
                    chunk = serial_port.read(read_size)
                    if not chunk:
                        continue

                    rx_buffer.extend(chunk)

                    while True:
                        magic_index = rx_buffer.find(MAGIC_BYTES)
                        if magic_index < 0:
                            if len(rx_buffer) > len(MAGIC_BYTES):
                                del rx_buffer[: -(len(MAGIC_BYTES) - 1)]
                                self.store.record_ignored_line()
                            break

                        if magic_index > 0:
                            del rx_buffer[:magic_index]
                            self.store.record_ignored_line()

                        if len(rx_buffer) < PACKET_HEADER_SIZE:
                            break

                        packet_size = expected_camera_packet_size(rx_buffer)
                        if packet_size is None:
                            del rx_buffer[0]
                            self.store.record_malformed_line()
                            continue

                        if len(rx_buffer) < packet_size:
                            break

                        packet = bytes(rx_buffer[:packet_size])
                        frame = parse_camera_packet(packet)
                        if frame is None:
                            del rx_buffer[0]
                            self.store.record_malformed_line()
                            continue

                        del rx_buffer[:packet_size]
                        self.store.record_line()
                        self.store.put_frame(frame)
        except Exception as error:  # Serial unplugged, busy port, permission error, etc.
            self.store.record_parse_error(error)


def _rate_from_times(times: Deque[float]) -> float:
    if len(times) < 2:
        return 0.0
    elapsed = times[-1] - times[0]
    if elapsed <= 0.0:
        return 0.0
    return (len(times) - 1) / elapsed
