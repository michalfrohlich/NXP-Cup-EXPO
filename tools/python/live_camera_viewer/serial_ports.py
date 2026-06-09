"""Serial port discovery helpers with no GUI or NumPy dependency."""

from __future__ import annotations

from .config import PORT_DESCRIPTION_HINTS


def find_default_port() -> str | None:
    ports = list_available_ports()
    if not ports:
        return None

    for port_name, description in ports:
        description_lc = description.lower()
        if any(hint in description_lc for hint in PORT_DESCRIPTION_HINTS):
            return port_name

    return ports[0][0]


def list_available_ports() -> list[tuple[str, str]]:
    try:
        from serial.tools import list_ports
    except ImportError:
        return []

    return [(port.device, port.description or "") for port in list_ports.comports()]


def format_available_ports(ports: list[tuple[str, str]]) -> str:
    lines = []
    for port_name, description in ports:
        lines.append(f"  {port_name}: {description}")
    return "\n".join(lines) if lines else "  no serial ports found"

