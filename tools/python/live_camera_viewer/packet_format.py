"""Parsers for Teensy camera transport formats."""

from __future__ import annotations

import re
import struct
import time

import numpy as np

from .camera_frame import CameraFrame
from .config import SAMPLE_COUNT


MAGIC_BYTES = b"TCM1"
PACKET_HEADER_FORMAT = "<IHHII"
PACKET_HEADER_SIZE = struct.calcsize(PACKET_HEADER_FORMAT)
PACKET_FORMAT_V1 = "<IHHII128HH"
PACKET_FORMAT_V2 = "<IHHII128HhBBBBBBH"
PACKET_SIZE_V1 = struct.calcsize(PACKET_FORMAT_V1)
PACKET_SIZE_V2 = struct.calcsize(PACKET_FORMAT_V2)
PACKET_FORMAT = PACKET_FORMAT_V2
PACKET_SIZE = PACKET_SIZE_V2
PACKET_VERSION = 2
ADC_12BIT_MAX = 4095
CAMERA_MARKERS = ("mode=cam-bench", "mode=cam-bringup")
FRAME8_PATTERN = re.compile(r"\bframe8=\[([^\]]*)\]")
KEY_VALUE_PATTERN = re.compile(r"\b([A-Za-z][A-Za-z0-9]*)=([^\s\[]+)")


def is_camera_line(line: str) -> bool:
    return any(marker in line for marker in CAMERA_MARKERS)


def parse_camera_line(line: str, host_time_s: float | None = None) -> CameraFrame | None:
    """Parse one Teensy camera status line.

    Returns None for non-camera lines, incomplete lines, malformed frame8 arrays,
    or values outside the expected 0..255 scaled frame range.
    """

    if not is_camera_line(line):
        return None

    frame_match = FRAME8_PATTERN.search(line)
    if frame_match is None:
        return None

    values = _parse_frame8_values(frame_match.group(1))
    if values is None:
        return None

    counters, status = _parse_key_values(line)
    if "seq" not in counters or "t" not in counters:
        return None

    values8 = np.asarray(values, dtype=np.uint8)
    return CameraFrame(
        sequence=counters["seq"],
        teensy_time_ms=counters["t"],
        host_time_s=time.monotonic() if host_time_s is None else host_time_s,
        values8=values8,
        status=status,
        counters=counters,
    )


def expected_camera_packet_size(buffer: bytes | bytearray) -> int | None:
    """Return the expected packet size for a buffer starting with a TCM1 header."""

    if len(buffer) < PACKET_HEADER_SIZE:
        return None
    if bytes(buffer[: len(MAGIC_BYTES)]) != MAGIC_BYTES:
        return None

    _magic, version, _sample_count, _sequence, _timestamp_us = struct.unpack_from(
        PACKET_HEADER_FORMAT, buffer
    )
    if version == 1:
        return PACKET_SIZE_V1
    if version == 2:
        return PACKET_SIZE_V2
    return None


def parse_camera_packet(packet: bytes, host_time_s: float | None = None) -> CameraFrame | None:
    """Parse one binary TCM1 camera packet."""

    packet_size = expected_camera_packet_size(packet)
    if packet_size is None or len(packet) != packet_size:
        return None

    crc_expected = struct.unpack_from("<H", packet, packet_size - 2)[0]
    crc_actual = crc16_ccitt_false(packet[4:-2])
    if crc_actual != crc_expected:
        return None

    version = struct.unpack_from("<H", packet, 4)[0]
    packet_format = PACKET_FORMAT_V2 if version == 2 else PACKET_FORMAT_V1
    unpacked = struct.unpack(packet_format, packet)
    _magic, version, sample_count, sequence, timestamp_us, *tail = unpacked
    values = tail[:SAMPLE_COUNT]

    if sample_count != SAMPLE_COUNT:
        return None

    values_raw = np.asarray(values, dtype=np.uint16)
    values8 = (
        ((values_raw.astype(np.uint32) * 255) + (ADC_12BIT_MAX // 2)) //
        ADC_12BIT_MAX
    ).astype(np.uint8)
    counters = {
        "min": int(np.min(values_raw)),
        "max": int(np.max(values_raw)),
        "avg": int(np.mean(values_raw)),
    }
    if version == 2:
        (
            vision_error_milli,
            vision_status,
            vision_feature,
            vision_confidence,
            vision_left_idx,
            vision_right_idx,
            vision_flags,
            _crc16,
        ) = tail[SAMPLE_COUNT:]
        counters.update(
            {
                "visionErrorMilli": int(vision_error_milli),
                "visionStatus": int(vision_status),
                "visionFeature": int(vision_feature),
                "visionConfidence": int(vision_confidence),
                "visionLeftLineIdx": int(vision_left_idx),
                "visionRightLineIdx": int(vision_right_idx),
                "visionFlags": int(vision_flags),
            }
        )

    return CameraFrame(
        sequence=sequence,
        teensy_time_ms=timestamp_us // 1000,
        host_time_s=time.monotonic() if host_time_s is None else host_time_s,
        values8=values8,
        values_raw=values_raw,
        counters=counters,
    )


def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for value in data:
        crc ^= value << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc


def _parse_frame8_values(frame_text: str) -> list[int] | None:
    if frame_text.strip() == "":
        return None

    parts = [part.strip() for part in frame_text.split(",")]
    if len(parts) != SAMPLE_COUNT:
        return None

    values: list[int] = []
    for part in parts:
        if not part:
            return None
        try:
            value = int(part, 10)
        except ValueError:
            return None
        if value < 0 or value > 255:
            return None
        values.append(value)

    return values


def _parse_key_values(line: str) -> tuple[dict[str, int], str]:
    counters: dict[str, int] = {}
    status = ""

    # Remove frame8 first so its comma-separated payload is not scanned as text.
    line_without_frame = FRAME8_PATTERN.sub("", line)
    for key, value_text in KEY_VALUE_PATTERN.findall(line_without_frame):
        if key == "status":
            status = value_text
            continue

        if key == "capUs" and "/" in value_text:
            latest_text, max_text = value_text.split("/", 1)
            _store_int(counters, "capUs", latest_text)
            _store_int(counters, "capUsMax", max_text)
            continue

        _store_int(counters, key, value_text)

    return counters, status


def _store_int(counters: dict[str, int], key: str, value_text: str) -> None:
    try:
        counters[key] = int(value_text, 10)
    except ValueError:
        return
