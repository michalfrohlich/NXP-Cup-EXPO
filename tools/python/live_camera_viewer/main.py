"""Command-line entry point for the Teensy camera live viewer."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

if __package__ in (None, ""):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
    from live_camera_viewer.config import DEFAULT_BAUD, DEFAULT_DISPLAY_HZ, DEFAULT_HISTORY
    from live_camera_viewer.serial_ports import (
        find_default_port,
        format_available_ports,
        list_available_ports,
    )
else:
    from .config import DEFAULT_BAUD, DEFAULT_DISPLAY_HZ, DEFAULT_HISTORY
    from .serial_ports import (
        find_default_port,
        format_available_ports,
        list_available_ports,
    )


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Live plotter for Teensy TSL1401CL binary raw-frame stream."
    )
    parser.add_argument(
        "--port",
        default=None,
        help="Serial port, for example COM11. Auto-detected when omitted.",
    )
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUD)
    parser.add_argument("--display-hz", type=float, default=DEFAULT_DISPLAY_HZ)
    parser.add_argument("--history", type=int, default=DEFAULT_HISTORY)
    parser.add_argument(
        "--list-ports",
        action="store_true",
        help="List detected serial ports and exit.",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)

    ports = list_available_ports()
    if args.list_ports:
        print(format_available_ports(ports))
        return 0

    port = args.port or find_default_port()
    if port is None:
        print("No serial port found. Available ports:")
        print(format_available_ports(ports))
        return 2

    try:
        if __package__ in (None, ""):
            from live_camera_viewer.serial_reader import LatestFrameStore, SerialCameraReader
        else:
            from .serial_reader import LatestFrameStore, SerialCameraReader
    except ModuleNotFoundError as error:
        print(f"Missing Python dependency: {error.name}")
        print("Install dependencies with:")
        print("  python -m pip install -r tools/python/live_camera_viewer/requirements.txt")
        return 3

    store = LatestFrameStore(history_size=args.history)
    reader = SerialCameraReader(port=port, baud=args.baud, store=store)
    reader.start()
    try:
        try:
            if __package__ in (None, ""):
                from live_camera_viewer.plots import run_gui
            else:
                from .plots import run_gui
        except ModuleNotFoundError as error:
            print(f"Missing Python dependency: {error.name}")
            print("Install dependencies with:")
            print("  python -m pip install -r tools/python/live_camera_viewer/requirements.txt")
            return 3

        return run_gui(
            store=store,
            port_label=f"{port} @ {args.baud}",
            display_hz=args.display_hz,
            history_size=args.history,
        )
    finally:
        reader.stop()
        reader.join()


if __name__ == "__main__":
    raise SystemExit(main())
