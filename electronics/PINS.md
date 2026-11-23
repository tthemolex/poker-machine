# ESP32 pin table

Source of truth: `firmware/include/config.h` (`#define PIN_<SIGNAL> <gpio>`). This table is
mirrored in [pins.csv](pins.csv) and in the ESP32 connector of [wiring.yml](wiring.yml).
`python3 tools/check_pins.py` asserts that all three agree.

Board: ESP32 DevKit V1 (WROOM-32, 30-pin). All logic is 3.3 V. Never feed 5 V into a GPIO
(the A3144 runs at 5 V but its output is open-collector and only pulled up to 3.3 V).

| Signal (PIN_...) | GPIO | Dir | Connects to | Notes |
|---|---|---|---|---|
| TURN_STEP | 25 | out | TMC2209 turret STEP | 16 usteps (MS1=MS2=VIO) |
| TURN_DIR | 26 | out | TMC2209 turret DIR | swap one coil pair if the direction is wrong |
| RACK_STEP | 27 | out | TMC2209 rack STEP | 8 usteps (MS1=MS2=GND) |
| RACK_DIR | 14 | out | TMC2209 rack DIR | |
| STEP_EN | 13 | out | EN of both TMC2209 | active LOW; HIGH = motors off when idle |
| FEED_IN1 | 32 | out | DRV8833 AIN1 | loader N20 PWM |
| FEED_IN2 | 33 | out | DRV8833 AIN2 | loader N20 direction (LOW = feed) |
| LAUNCH | 19 | out | DRV8833 BIN1 (IN3) | launcher 130 motor PWM, BIN2 (IN4) tied to GND |
| PUSHER | 23 | out | MG90S signal | detached when idle |
| IR_LOAD | 15 | in | IR receiver OUT, loader | INPUT_PULLUP, LOW = blocked; strapping pin |
| IR_EXIT | 4 | in | IR receiver OUT, launcher entry | INPUT_PULLUP, LOW = blocked |
| HOME_TURN | 16 | in | A3144 OUT | INPUT_PULLUP + 10k to 3V3, LOW at 0 deg |
| HOME_RACK | 17 | in | microswitch NO, COM to GND | INPUT_PULLUP, LOW at rack bottom |
| BTN_GO | 18 | in | GO button to GND | INPUT_PULLUP |
| BTN_SET | 5 | in | SET button to GND | INPUT_PULLUP; strapping pin |
| LED | 2 | out | LED anode via 330R, cathode to GND | strapping pin, also the on-board LED |
| VBAT | 34 | in | divider mid-point 100k/22k | input-only ADC1, no pull-up |
| OLED_SDA | 21 | io | SSD1306 SDA | I2C 0x3C (optional OLED) |
| OLED_SCL | 22 | out | SSD1306 SCL | I2C |

## Rules the pin map follows (checked by `tools/check_pins.py`)

* No flash pins (GPIO 6-11) are used. GPIO 1/3 (UART0, USB serial) are left free.
* GPIO 34-39 are input-only and have no internal pull-ups. Only VBAT (34) uses one, as an input.
* Strapping pins: GPIO 2 must be low/floating to enter the bootloader. The LED to GND keeps it low.
  GPIO 5 and 15 like to be high at boot. Their pull-ups (button / IR receiver idle) keep them high.
  GPIO 12 (flash voltage strap) is not used.
* ADC: VBAT is on ADC1 (works with WiFi on). No signal uses ADC2.

## Power pins

| ESP32 pin | Connects to |
|---|---|
| VIN | 5 V rail (Mini560 OUT+) |
| GND | common ground |
| 3V3 | 3.3 V logic bus: IR receivers, OLED, TMC2209 VIO, MS1/MS2 of the turret driver |

Other fixed wiring: TMC2209 VM = switched 12 V with 100 uF at each driver. DRV8833 VM = 5 V
(not 12 V: its absolute max is 10.8 V), nSLEEP to 5 V, BIN2 to GND. Divider top = switched 12 V.
