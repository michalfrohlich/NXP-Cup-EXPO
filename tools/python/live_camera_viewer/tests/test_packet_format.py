from __future__ import annotations

import sys
import unittest
from pathlib import Path

try:
    import numpy  # noqa: F401
except ModuleNotFoundError:
    numpy = None

if numpy is not None:
    sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
    from live_camera_viewer.packet_format import (
        MAGIC_BYTES,
        PACKET_FORMAT,
        PACKET_FORMAT_V1,
        crc16_ccitt_false,
        expected_camera_packet_size,
        parse_camera_line,
        parse_camera_packet,
    )
    import struct


def _valid_line(frame_values: list[int] | None = None) -> str:
    values = frame_values if frame_values is not None else list(range(128))
    frame_text = ",".join(str(value) for value in values)
    return (
        "t=51301 mode=cam-bench status=RUN clkHz=200000 frameHz=200 "
        "frameUs=5000 readoutUs=645 req=4888 si=4888 win=4888 ready=4888 "
        "drop=0 samples=625664 overrun=0 capUs=651/651 capClk=130 "
        "adcErr=0 dma=4888 dmaErr=0 seq=4887 min=626 max=4074 avg=1724 "
        f"p0=661 p64=4072 p127=915 frame8=[{frame_text}]"
    )


@unittest.skipIf(numpy is None, "numpy is required for parser tests")
class PacketFormatTests(unittest.TestCase):
    def test_valid_frame8_line_parses(self) -> None:
        frame = parse_camera_line(_valid_line(), host_time_s=1.25)

        self.assertIsNotNone(frame)
        assert frame is not None
        self.assertEqual(frame.sequence, 4887)
        self.assertEqual(frame.teensy_time_ms, 51301)
        self.assertEqual(frame.values8.shape, (128,))
        self.assertEqual(int(frame.values8[0]), 0)
        self.assertEqual(int(frame.values8[127]), 127)
        self.assertEqual(frame.counters["capUs"], 651)
        self.assertEqual(frame.counters["capUsMax"], 651)
        self.assertEqual(frame.counters["dmaErr"], 0)

    def test_malformed_frame_length_is_rejected(self) -> None:
        frame = parse_camera_line(_valid_line([1, 2, 3]), host_time_s=1.25)
        self.assertIsNone(frame)

    def test_out_of_range_frame_value_is_rejected(self) -> None:
        values = list(range(128))
        values[5] = 256
        frame = parse_camera_line(_valid_line(values), host_time_s=1.25)
        self.assertIsNone(frame)

    def test_non_camera_line_is_ignored(self) -> None:
        frame = parse_camera_line("Teensy TSL1401CL camera bench", host_time_s=1.25)
        self.assertIsNone(frame)

    def test_valid_binary_packet_parses_raw_samples(self) -> None:
        values = list(range(128))
        body = struct.pack(
            PACKET_FORMAT[:-1],
            int.from_bytes(MAGIC_BYTES, "little"),
            2,
            128,
            42,
            123456,
            *values,
            -125,
            1,
            0,
            87,
            12,
            104,
            1,
        )
        crc = crc16_ccitt_false(body[4:])
        packet = body + struct.pack("<H", crc)

        self.assertEqual(expected_camera_packet_size(packet), len(packet))

        frame = parse_camera_packet(packet, host_time_s=1.25)

        self.assertIsNotNone(frame)
        assert frame is not None
        self.assertEqual(frame.sequence, 42)
        self.assertEqual(frame.teensy_time_ms, 123)
        self.assertIsNotNone(frame.values_raw)
        assert frame.values_raw is not None
        self.assertEqual(int(frame.values_raw[127]), 127)
        self.assertEqual(int(frame.values8[0]), 0)
        self.assertEqual(frame.counters["visionErrorMilli"], -125)
        self.assertEqual(frame.counters["visionStatus"], 1)
        self.assertEqual(frame.counters["visionConfidence"], 87)
        self.assertEqual(frame.counters["visionLeftLineIdx"], 12)
        self.assertEqual(frame.counters["visionRightLineIdx"], 104)
        self.assertEqual(frame.counters["visionFlags"], 1)

    def test_legacy_v1_binary_packet_still_parses_raw_samples(self) -> None:
        values = list(range(128))
        body = struct.pack(
            PACKET_FORMAT_V1[:-1],
            int.from_bytes(MAGIC_BYTES, "little"),
            1,
            128,
            42,
            123456,
            *values,
        )
        crc = crc16_ccitt_false(body[4:])
        packet = body + struct.pack("<H", crc)

        self.assertEqual(expected_camera_packet_size(packet), len(packet))

        frame = parse_camera_packet(packet, host_time_s=1.25)

        self.assertIsNotNone(frame)
        assert frame is not None
        self.assertEqual(frame.sequence, 42)
        self.assertIsNotNone(frame.values_raw)
        self.assertNotIn("visionFlags", frame.counters)


if __name__ == "__main__":
    unittest.main()
