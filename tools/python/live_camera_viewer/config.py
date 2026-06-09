"""Default settings for the Teensy camera live viewer."""

SAMPLE_COUNT = 128
VISION_TRIM_LEFT_PX = 2
VISION_INVALID_IDX = 255
DEFAULT_BAUD = 921600
DEFAULT_DISPLAY_HZ = 60.0
DEFAULT_HISTORY = 300
DEFAULT_SERIAL_TIMEOUT_S = 0.2

PORT_DESCRIPTION_HINTS = (
    "teensy",
    "usb serial",
    "usb-serial",
    "arduino",
    "serial device",
)
