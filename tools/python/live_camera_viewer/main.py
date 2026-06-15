"""Command-line entry point for the Teensy camera live viewer."""

from __future__ import annotations

import argparse
import importlib
import sys
from pathlib import Path
from typing import Callable

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


GUI_MODULES = {
    "classic": "plots",
    "camera_2d": "camera_2d_view",
    "detector_replay": "camera_detector_replay_view",
}

RunGuiFn = Callable[..., int]


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
        "--gui",
        choices=tuple(GUI_MODULES.keys()),
        default="classic",
        help=(
            "GUI frontend to launch: 'classic' uses plots.py, "
            "'camera_2d' uses camera_2d_view.py."
            "'detector_replay' uses camera_detector_replay_view.py."
        ),
    )
    parser.add_argument(
        "--list-ports",
        action="store_true",
        help="List detected serial ports and exit.",
    )
    return parser.parse_args(argv)


def _load_gui_runner(gui_name: str) -> RunGuiFn:
    """Import the selected GUI module and return its run_gui function."""

    module_basename = GUI_MODULES[gui_name]
    if __package__ in (None, ""):
        module_name = f"live_camera_viewer.{module_basename}"
    else:
        module_name = f"{__package__}.{module_basename}"

    module = importlib.import_module(module_name)
    run_gui = getattr(module, "run_gui", None)
    if not callable(run_gui):
        raise AttributeError(
            f"Selected GUI module '{module_basename}.py' does not define callable run_gui(...)."
        )
    return run_gui


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

    try:
        run_gui = _load_gui_runner(args.gui)
    except ModuleNotFoundError as error:
        selected_module = GUI_MODULES[args.gui]
        if error.name in {selected_module, f"live_camera_viewer.{selected_module}"}:
            print(f"Selected GUI module not found: {selected_module}.py")
            print("Expected it in:")
            print(f"  tools/python/live_camera_viewer/{selected_module}.py")
            return 4

        print(f"Missing Python dependency: {error.name}")
        print("Install dependencies with:")
        print("  python -m pip install -r tools/python/live_camera_viewer/requirements.txt")
        return 3
    except AttributeError as error:
        print(error)
        return 4

    store = LatestFrameStore(history_size=args.history)
    reader = SerialCameraReader(port=port, baud=args.baud, store=store)
    reader.start()
    try:
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
