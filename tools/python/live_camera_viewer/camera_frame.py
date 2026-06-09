"""Typed frame data shared between the serial reader and GUI."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Mapping

import numpy as np


@dataclass
class CameraFrame:
    """One parsed camera frame from the Teensy binary stream.

    values_raw contains the original 12-bit ADC samples from LinearCameraFrame.
    values8 is derived in the viewer for plotting on a compact 0..255 axis.
    """

    sequence: int
    teensy_time_ms: int
    host_time_s: float
    values8: np.ndarray
    values_raw: np.ndarray | None = None
    status: str = ""
    counters: dict[str, int] = field(default_factory=dict)

    receive_fps: float = 0.0
    sequence_delta: int = 0
    frame_age_s: float = 0.0
    malformed_count: int = 0
    parse_error_count: int = 0
    duplicate_count: int = 0
    estimated_drop_count: int = 0

    @property
    def minimum(self) -> int:
        values = self.values_raw if self.values_raw is not None else self.values8
        return int(self.counters.get("min", int(np.min(values))))

    @property
    def maximum(self) -> int:
        values = self.values_raw if self.values_raw is not None else self.values8
        return int(self.counters.get("max", int(np.max(values))))

    @property
    def average(self) -> int:
        values = self.values_raw if self.values_raw is not None else self.values8
        return int(self.counters.get("avg", int(np.mean(values))))

    def with_reader_stats(
        self,
        *,
        receive_fps: float,
        sequence_delta: int,
        malformed_count: int,
        parse_error_count: int,
        duplicate_count: int,
        estimated_drop_count: int,
    ) -> "CameraFrame":
        self.receive_fps = receive_fps
        self.sequence_delta = sequence_delta
        self.malformed_count = malformed_count
        self.parse_error_count = parse_error_count
        self.duplicate_count = duplicate_count
        self.estimated_drop_count = estimated_drop_count
        return self


@dataclass(frozen=True)
class ReaderStats:
    lines_read: int = 0
    parsed_frames: int = 0
    ignored_lines: int = 0
    malformed_lines: int = 0
    parse_errors: int = 0
    duplicate_frames: int = 0
    estimated_dropped_frames: int = 0
    receive_fps: float = 0.0
    last_error: str = ""


def counters_as_text(counters: Mapping[str, int]) -> str:
    """Return a compact, stable text representation for debug panels."""

    preferred_keys = (
        "req",
        "ready",
        "dma",
        "drop",
        "overrun",
        "adcErr",
        "dmaErr",
        "capUs",
        "capUsMax",
        "capClk",
        "min",
        "max",
        "avg",
        "visionErrorMilli",
        "visionStatus",
        "visionFeature",
        "visionConfidence",
        "visionLeftLineIdx",
        "visionRightLineIdx",
        "visionFlags",
    )
    parts = []
    for key in preferred_keys:
        if key in counters:
            parts.append(f"{key}={counters[key]}")
    return " ".join(parts)
