# Ultrasonic Tests And Victory Lap

## Where To Run Them

Build with the runtime test menu:

```c
#define APP_TEST_NXP_CUP_TESTS            1
```

Then use the pot to select:

- `Ultrasonic`
- `Ultra+ESC`
- `Victory Lap`

Use `swPcb` to enter or leave the selected test.

These are separate runtime tests. `Ultrasonic` and `Ultra+ESC` are obstacle
safety tests. `Victory Lap` is its own pole-triggered celebration test. They
share the OLED and ultrasonic driver, but they do not depend on each other.

## Ultrasonic Timing

The ultrasonic trigger period is:

```c
ULTRA_TRIGGER_PERIOD_MS = 60 ms
```

HC-SR04-style ultrasonic modules use a short trigger pulse and then need enough
time for the echo path to finish before the next trigger. A 60 ms period is a
safe default for the current 2 cm to 400 cm sensor range.

## Ultra+ESC Test

Purpose:

- prove the ultrasonic sensor can control both ESC outputs,
- prove the servo is locked straight during obstacle handling,
- show clear RGB LED states.

Behavior:

| Distance | LED | Servo | ESC command |
|---:|---|---|---:|
| no valid distance | yellow | straight | 0% |
| above 45 cm | green | straight | 50% |
| 45 cm to above 8 cm | yellow | straight | `HONOR_SLOW1_SPEED_PCT` |
| 8 cm or less | red | straight | 0% |

The ESC command is sent to both motor outputs equally.

## Honor/Race Ultrasonic Behavior

The shared honor-lap distance policy now also uses the 45 cm first slow zone and
the 8 cm stop zone.

When the honor or race honor phase sees an object at 45 cm or closer, the servo
is held straight so the car does not steer away from the pole while slowing.

## Victory Lap Test

Purpose:

- prepare a separate victory behavior test,
- approach a pole slowly with ultrasonic,
- stop at 8 cm,
- run a Mario-style victory fanfare note sequence.

Current behavior:

1. Arm both ESC outputs.
2. Keep the servo straight.
3. Drive slowly until the ultrasonic distance is 8 cm or less.
4. Stop both ESC outputs.
5. Step through the victory note table and show the note frequency on the OLED.

## BLDC Music Note

Real BLDC motor music needs control of the motor phase switching frequency, or
ESC firmware that accepts beep/music commands.

The current S32K ESC API only sends normal RC-style throttle commands through a
50 Hz PWM signal. That is enough for speed control, but it is not enough to play
accurate notes like 523 Hz or 1047 Hz through the motor.

The current `Victory Lap` test keeps the note table and timing in place so a
future low-level motor-tone driver can use it, while the car remains stopped
and safe now.
