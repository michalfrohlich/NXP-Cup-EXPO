"""Typed frame data shared between the serial reader and GUI."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Mapping

import numpy as np

from .config import (
    SAMPLE_COUNT,
    VISION_INVALID_IDX,
    VISION_PROCESS_COUNT,
    VISION_TRIM_LEFT_PX,
)


VISION_TRACK_STATUS = {
    0: "LOST",
    1: "BOTH",
    2: "LEFT",
    3: "RIGHT",
}

VISION_FEATURE = {
    0: "NONE",
    1: "FINISH_LINE",
}


@dataclass(frozen=True)
class VisionResult:
    """Compact detector output carried by the current TCM1 v2 stream."""

    in_stream: bool
    valid: bool
    error: float = 0.0
    status: int = 0
    feature: int = 0
    confidence: int = 0
    left_line_idx: int = VISION_INVALID_IDX
    right_line_idx: int = VISION_INVALID_IDX
    flags: int = 0

    @classmethod
    def from_counters(cls, counters: Mapping[str, int]) -> "VisionResult":
        if "visionFlags" not in counters:
            return cls(in_stream=False, valid=False)

        flags = int(counters.get("visionFlags", 0))
        return cls(
            in_stream=True,
            valid=bool(flags & 0x01),
            error=float(counters.get("visionErrorMilli", 0)) / 1000.0,
            status=int(counters.get("visionStatus", 0)),
            feature=int(counters.get("visionFeature", 0)),
            confidence=int(counters.get("visionConfidence", 0)),
            left_line_idx=int(counters.get("visionLeftLineIdx", VISION_INVALID_IDX)),
            right_line_idx=int(counters.get("visionRightLineIdx", VISION_INVALID_IDX)),
            flags=flags,
        )

    @property
    def status_name(self) -> str:
        return VISION_TRACK_STATUS.get(self.status, str(self.status))

    @property
    def feature_name(self) -> str:
        return VISION_FEATURE.get(self.feature, str(self.feature))

    @property
    def error_pct(self) -> float:
        return self.error * 100.0

    @property
    def left_raw_idx(self) -> int | None:
        return _detector_idx_to_raw_idx(self.left_line_idx)

    @property
    def right_raw_idx(self) -> int | None:
        return _detector_idx_to_raw_idx(self.right_line_idx)

    @property
    def center_raw_idx(self) -> float | None:
        if not self.valid or self.status == 0:
            return None

        detector_mid = (float(VISION_PROCESS_COUNT) - 1.0) / 2.0
        raw_idx = (self.error * detector_mid) + detector_mid + VISION_TRIM_LEFT_PX
        if raw_idx < 0.0 or raw_idx > float(SAMPLE_COUNT - 1):
            return None
        return raw_idx


def _detector_idx_to_raw_idx(detector_idx: int) -> int | None:
    if detector_idx == VISION_INVALID_IDX:
        return None
    raw_idx = detector_idx + VISION_TRIM_LEFT_PX
    if raw_idx < 0 or raw_idx >= SAMPLE_COUNT:
        return None
    return raw_idx


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

    @property
    def vision(self) -> VisionResult:
        return VisionResult.from_counters(self.counters)

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
