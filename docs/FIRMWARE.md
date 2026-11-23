# Firmware

ESP32 DevKit V1 (WROOM-32), Arduino framework, PlatformIO. Source: [`firmware/`](../firmware).
It is a refactor of the draft `poker_dealer.ino` and keeps the draft's pins, button scheme and
mechanism behaviour.

```
firmware/
  platformio.ini        envs: esp32dev (firmware), esp32dev_nooled (same, no OLED code), native (host tests)
  include/config.h      ALL pins and calibration constants
  include/geometry.h    AUTO-GENERATED from cad/params.scad (tools/gen_geometry.py). Do not edit.
  lib/dealer_core/      hardware-independent logic, unit-tested on the host
    rng.*               randBelow() rejection sampling, injectable random source, SplitMix64
    rack.*              52-shelf occupancy bitmap (also the NVS format), load allocation, random pick
    motion.*            rack eject/load positions (mm, steps), seat/board/burn angles (0..360)
    game.*              hand state machine, deal plans (abstract "deal card to angle X, profile P")
    battery.*           divider maths, ok / warn / refuse classification
    button.*            non-blocking debounce, short / long press
    status.*            fault codes + texts, LED patterns
    loader.*            loader beam timing verdict (empty / single / double / jam)
  src/                  ESP32 hardware layer
    main.cpp            buttons -> dealer::Game -> Machine, fault acknowledgement
    console.*           serial calibration / test console
    machine.*           steppers, homing, loader, pusher, launcher, sensors, power up / down
    ui.*                buttons, status LED, optional SSD1306 OLED
    storage.*           NVS (Preferences): player count, button seat, rack mask
  test/                 Unity tests for lib/dealer_core (pio test -e native)
```

The game logic never touches hardware. `dealer::Game` turns button events into commands and
**plans**: lists of `DealAction {deg, launch = Seat | Board | Burn, seat, boardSlot}`.
`Machine::executePlan()` runs a plan: for each action it picks a uniformly random occupied
shelf (`Rack::pickRandom` with `esp_random()`), moves the rack to the eject station and the
turret to `deg`, sets the launcher speed and pushes the card. The tests check the plans, so
the deal order, burns and button rotation are verified without hardware.

## Build, flash, test

PlatformIO Core 6.x (`pip install platformio`, or the VS Code extension). Here it is used as
`~/.platformio/penv/bin/pio`; `pio` below means that binary.

```sh
cd firmware
pio run -e esp32dev                 # build (zero errors, zero warnings in project code)
pio run -e esp32dev -t upload       # flash over USB (hold BOOT if the upload does not start)
pio device monitor -b 115200        # serial log
pio test -e native                  # host unit tests (needs a C++17 compiler: gcc or clang)
pio run -e esp32dev_nooled          # optional: the build without the OLED code
```

Project code (`src/`, `lib/dealer_core/`) builds with `-Wall -Wextra -Werror`. The ESP32 Arduino
core adds `-Wno-error=unused-*`, so a CI check should also grep the build log for `warning:`.
Third-party libraries (AccelStepper, ESP32Servo, Adafruit SSD1306/GFX/BusIO) keep their own
flags.

Before building after a CAD change, regenerate the geometry: `python3 tools/gen_geometry.py`
(the Makefile does this). Shelf pitch, station offset, steps/mm, steps/deg and the board angle
are only defined in `cad/params.scad`.

### First power-up

1. Flash with the battery off (USB only). The serial log prints `FAULT: TURRET/RACK HOMING
   FAILED` or `NO BATTERY` because the motors have no power; that is expected.
2. Switch the battery on, press GO to acknowledge, press the EN/RST button on the ESP32 (or
   power-cycle). The rack homes down to the microswitch, then the turret homes to the hall
   sensor, then it parks at 0° and powers down.
3. If an axis runs the wrong way, flip `TURN_DIR_INVERT` / `RACK_DIR_INVERT` in `config.h`.
   The turret must turn **clockwise seen from above** for positive angles (seats are numbered
   clockwise), the rack must move **up** for positive steps.
4. Run the calibration below.

## Using it

Seats are numbered 1..N clockwise starting from the first seat after the board (seat 1 is
just clockwise of the board). The board lands next to the machine at `BOARD_DEG` (8°, fanned
0..16°), in the gap between seat N and seat 1. The hall home / park position is 0°. The dealer button is shown as a seat number on the OLED and by LED blinks.

### Buttons

| When | Press | Action |
|---|---|---|
| Between hands | SET short | Cycle player count 2..10 (LED blinks the count) |
| Between hands | SET long (≥0.8 s) | Move the dealer button +1 seat (LED blinks the seat number) |
| Between hands | GO short | Battery check, load the hopper into empty shelves, verify 52 cards, deal 2 hole cards each |
| In a hand | GO short | Next street: burn + flop (3), burn + turn, burn + river |
| In a hand | GO long | Hand over early (everyone folded): the button advances |
| In a hand | SET long | Misdeal: the hand ends, the button stays |
| Fault | GO | Acknowledge after fixing the cause |
| Power-on | hold SET | Purge: push every shelf the rack map marks as full into the burn tray, clear the map |

After the river the hand ends and the button advances automatically. Hole cards go one at a
time, two rounds, clockwise, starting with the seat left of the button (the button gets the
last card of each round). Player count, button seat and the rack map are saved in NVS and
survive power-off.

Long presses fire as soon as 0.8 s is reached (no need to release). Presses are ignored while
the machine is moving cards. A button held at boot is ignored until released (except the SET
purge check).

### Status LED (GPIO 2)

| Pattern | Meaning |
|---|---|
| 60 ms blip every 2 s | Ready, between hands, battery OK |
| Two blips every 2 s | Ready, battery low (< 10.9 V): charge soon |
| 1 Hz, 50 % | In a hand, waiting for GO (next street) |
| Solid on | Busy: homing, loading or dealing |
| Fast blink, 5 Hz | Fault: fix it and press GO |
| N slow blinks (150/150 ms) | After SET: player count, or button seat number; also the player count at boot |

### OLED (optional SSD1306 128×64, I²C 0x3C, SDA 21 / SCL 22)

Compiled in with `HAS_OLED=1` (default, env `esp32dev`). At boot the firmware probes 0x3C; if
nothing answers it runs without a screen (the serial log says `OLED not found`).

```
POKER DEALER 12.31V      battery voltage (resting, read every 5 s when idle)
Batt: OK                 OK / LOW / EMPTY / NONE
State: WAIT FLOP         READY / WAIT FLOP / WAIT TURN / WAIT RIVER / BUSY
Players: 6  Btn: 3       player count, dealer button seat (1..N)
Rack: 52/52              cards the rack map holds
DEALING FLOP             status message, or the fault text in inverse video
GO: flop                 what GO does next / "Fix, then press GO"
```

## Faults

On any fault the machine stops the loader and launcher, retracts the pusher, **disables the
steppers and detaches the servo** (so the rack and turret can be handled to clear the jam),
saves the rack map, blinks the LED fast and shows the fault on the OLED and serial log. Fix
the cause and press GO. Both axes re-home on the next action. A fault during a hand aborts
the hand like a misdeal: the button stays; collect all cards and start the hand again.

| Fault (OLED text) | Detected by | Meaning | Recovery |
|---|---|---|---|
| `LOADER JAM` | loader beam blocked > `JAM_MS` | A card stuck between the roller and the rack | Open the top cap, pull the card out, put it back in the hopper, press GO, then GO to start again. That card was not counted. |
| `DOUBLE FEED` | loader beam blocked > `DOUBLE_MS` (but < `JAM_MS`) | Two cards went into one shelf | Press GO: the machine pushes that shelf into the burn tray. Put the cards from the burn tray back in the hopper and press GO. |
| `CARD COUNT != 52` | rack map count after loading | The hopper ran empty before 52 cards were in the rack (card missing, or misfeeds) | Find the missing card(s), put them in the hopper, press GO to acknowledge, press GO again: only the empty shelves are filled. |
| `NO CARD IN SHELF` | launcher beam not blocked within `EXIT_WAIT_MS` after pushing | The map said the shelf was full but nothing came out (map wrong, or pusher/blade problem) | The shelf is marked empty. Press GO; the hand is aborted. Check the pusher. Re-collect cards and start a new hand (the loader refills the shelf). If it repeats, purge (hold SET at power-on) and reload. |
| `LAUNCHER JAM` | launcher beam blocked > `EXIT_JAM_MS` | The card stopped in the launcher nip | Remove the card from the launcher, press GO. The shelf is already marked empty. Hand aborted. |
| `RACK EMPTY` | rack map count = 0 during a deal | Should not happen after a verified load (map corrupted) | Press GO, start a new hand (it reloads). |
| `TURRET HOMING FAILED` | hall sensor not seen within `TURN_HOME_TIMEOUT_MS` (boot / after fault) or `TURN_REHOME_TIMEOUT_MS` (re-home from park), or stuck active | Hall sensor, magnet, belt or turret motor problem; no 12 V | Check the battery switch, magnet gap (≤ 3 mm), A3144 wiring, belt tension. Press GO and retry. |
| `RACK HOMING FAILED` | bottom microswitch not seen within `RACK_HOME_TIMEOUT_MS`, or stuck pressed | Rack stuck, switch broken, motor wiring; no 12 V | Check rods/leadscrew for binding, switch wiring. Press GO and retry. |
| `BATTERY LOW-CHARGE` | battery < `VBAT_MIN` (10.5 V) at GO | Battery too low to finish a hand reliably | Charge the TalentCell (its own charger via the panel DC jack). Hand refused. |
| `NO BATTERY (USB)` | battery < `VBAT_ABSENT` (3 V) | Running from USB only, the motors have no power | Switch the battery on. |

## Idle power-down

The steppers (TMC2209 EN high, shared on GPIO 13) and the servo (detached, no pulses) are
only powered while the machine is homing, loading or dealing. Before each action
(`Machine::powerUp`) the drivers are enabled, the servo attached and retracted, and the
turret **re-homes** (it may have been bumped while unpowered). After each action
(`Machine::powerDown`) the turret **parks at 0°**, so the re-home is a short move. The rack
leadscrew is self-locking, so the rack only re-homes at boot and after a fault. Between
hands and between streets only the ESP32 (and the OLED) draws current.

## Battery check

GPIO 34 reads the battery through a 100 k / 22 k divider (12.6 V full → 2.27 V at the pin,
11 dB attenuation, `analogReadMilliVolts`, 16 samples). It is read at every GO between hands
and every 5 s while idle (motors off, so it is the resting voltage).

* ≥ 10.9 V: OK.
* 10.5–10.9 V: warning (LED double blip, OLED `LOW`, serial message); play is allowed.
* < 10.5 V: the hand is refused (`BATTERY LOW-CHARGE`). A hand already running is finished.
* < 3 V: no battery / USB only (`NO BATTERY (USB)`).

## Calibration constants (`firmware/include/config.h`)

Geometry is **not** in config.h: it comes from `geometry.h` (`geom::SHELF_PITCH`,
`geom::STATION_OFFSET`, `geom::RACK_STEPS_PER_MM`, `geom::TURRET_STEPS_PER_DEG`,
`geom::BOARD_DEG`, `geom::BOARD_SPREAD_DEG`), generated from `cad/params.scad`. Change those
in params.scad and regenerate.

**Angle frame:** hall home = **0°** (turret homes and parks there; the magnet marks the
launcher +Y direction). The board centre is at **`BOARD_DEG`** (8°, params.scad), fanned
±2 × `BOARD_SPREAD_DEG` = 0..16°. Seats sit at `BOARD_DEG + (seat + 0.5) × 360 / N`, the highest
is 350° (N = 10). So every target is inside [0, 360) without wrapping and the turret never
makes a near-full-turn move for the board (`test_all_angles_in_range_no_wrap`). `normDeg()`
is only a safety net.

Rack positions (from the homed bottom position, mm): `eject(i) = (51 − i) × pitch`,
`load(i) = eject(i) + STATION_OFFSET`. Shelf 0 is the bottom shelf; homed = shelf 51 at the
eject station. Seats: `BOARD_DEG + (seat + 0.5) × 360 / N`; board slot k:
`BOARD_DEG + (k − 2) × BOARD_SPREAD_DEG`; burns at `BOARD_DEG`. All wrapped into 0..360°.

| Constant | Default | What it does | How to calibrate |
|---|---|---|---|
| `RACK_ZERO_TRIM_MM` | 0.0 | Added to every rack target (eject and load). + = rack higher | Home, `rack eject 51`: shelf 51 must sit exactly at the eject station (pusher blade / launcher nip). Measure the error, set the trim, rebuild. Check with `rack eject 0` / `rack load 0`. |
| `TURRET_ZERO_TRIM_DEG` | 0.0 | Added to every turret target angle. + = more clockwise | `turret 0`: the launcher must point exactly along the machine's +Y mark. If not, set the trim to the error (degrees). |
| `TURRET_PARK_DEG` | 0.0 | Idle park angle (on the hall sensor) | Leave at 0 (spec). |
| `TURN_MAX_SPEED`, `TURN_ACCEL` | 6000 st/s, 12000 st/s² | Turret motion | Raise until it skips steps, then back off 30 %. |
| `RACK_MAX_SPEED`, `RACK_ACCEL` | 16000 st/s, 40000 st/s² | Rack motion (80 mm/s) | Same; check for lost steps by dealing a full deck and confirming the last shelves still line up. |
| `TURN_HOMING_SPEED`, `RACK_HOMING_SPEED` | 1500, 3000 st/s | Homing seek speed | Slower = more repeatable zero. |
| `TURN_BACKOFF_DEG`, `RACK_BACKOFF_MM` | 10°, 3 mm | Back-off when the sensor is already active | Must exceed the sensor's active zone. |
| `TURN_DIR_INVERT`, `RACK_DIR_INVERT` | false | Flip motor direction in software | See first power-up. |
| `*_HOME_TIMEOUT_MS`, `BACKOFF_TIMEOUT_MS` | 25000 / 5000 / 8000 / 3000 | Homing fault timeouts | Longer than a full sweep at homing speed. |
| `LAUNCH_SEAT` | 255 | Launcher PWM for a throw to a seat | Deal hole cards to 10 seats; lower it if cards fly off the table, the farthest seat must still be reached. |
| `LAUNCH_BOARD` | 110 | Launcher PWM for the board (short throw next to the machine) | Board cards should land 10–15 cm from the base and not overlap seat 1 / seat N. |
| `LAUNCH_BURN` | 55 | Launcher PWM for a burn (drops into the turret tray) | Lowest PWM at which the card still reliably leaves the nip; the card must fall into the tray, not onto the table. |
| `SPINUP_MS` | 250 | Wait after a launcher speed change | Increase if the first card after a speed change flies differently. |
| `FEED_PWM` | 170 | Loader roller PWM | Lowest reliable feed; higher speeds make double feeds more likely. |
| `NO_CARD_MS` | 1500 | No card at the loader beam within this = hopper empty | Must exceed the time for one card to reach the beam. |
| `DOUBLE_MS` | 350 | Beam blocked longer = double feed | Console `loadone` prints the single-card block time; over a full deck set DOUBLE_MS ≈ 1.5 × the typical time and well below 2 ×. |
| `JAM_MS` | 1000 | Beam blocked longer = loader jam | > 2 × single-card time. |
| `SEAT_MS` | 150 | Extra roller run after the card clears the beam | Card must sit flush against the stop fingers; too long scuffs it. |
| `PUSH_HOME`, `PUSH_OUT` | 10°, 160° | Pusher servo angles | HOME: blade fully behind the rack (rack can move freely). OUT: card fully in the launcher nip. |
| `PUSH_RETRACT_MS`, `SERVO_SETTLE_MS` | 200, 50 | Waits for the blade | Blade must be clear of the rack before the rack moves. |
| `EXIT_WAIT_MS` | 1500 | Card must reach the launcher beam within this after pushing | Otherwise `NO CARD IN SHELF`. |
| `EXIT_JAM_MS` | 1000 | Launcher beam blocked longer = jam | > time for a card to pass at `LAUNCH_BURN`. |
| `VBAT_R_TOP_K`, `VBAT_R_BOT_K` | 100, 22 | Battery divider | Match the fitted resistors. |
| `VBAT_WARN`, `VBAT_MIN`, `VBAT_ABSENT` | 10.9, 10.5, 3.0 V | Battery thresholds | Spec values; do not lower VBAT_MIN (3S Li-ion cut-off margin). |
| `VBAT_CAL` | 1.0 | Scale the reading | Measure the pack with a multimeter, set `VBAT_CAL = V_meter / V_shown`. |
| `DEBOUNCE_MS`, `LONG_PRESS_MS` | 30, 800 | Buttons | |
| `MIN_PLAYERS`, `MAX_PLAYERS`, `DEFAULT_PLAYERS` | 2, 10, 6 | Table size | |

### Calibration procedure

1. **Rack zero and shelf pitch.** Home (console `home`), then `rack eject 51`. With the top cap
   off, shelf 51 (top shelf) must line up with the eject station (pusher blade / launcher nip).
   Correct a constant error with `RACK_ZERO_TRIM_MM` in config.h (or shim the microswitch if it
   is large). Check shelf 0 at the eject
   station and at the load station: an error that grows with the shelf number means
   `SHELF_PITCH` (as printed) or `RACK_STEPS_PER_MM` (lead / microsteps) is off; correct
   `SHELF_PITCH` in params.scad to the measured value (measure 51 pitches with calipers and
   divide) and regenerate.
2. **Stations.** `STATION_OFFSET` is the vertical distance between the load and eject
   stations (params.scad). A card loaded at shelf i must come out of shelf i at the eject
   station.
3. **DOUBLE_MS**: put a deck in the hopper, `clear` (only if the rack is empty), then repeat
   `loadone` and note the printed beam-blocked times; set `DOUBLE_MS` ≈ 1.5 × the typical
   value. Then **pusher angles** (`servo <angle>`), **launcher PWMs** (`deal seat <n>`,
   `deal board <k>`, `deal burn` with a loaded rack; `launch <pwm> [ms]` to try a speed).
4. **Seat angles / board.** Seats are evenly spaced; `BOARD_DEG` (params.scad) is the turret
   angle of the board relative to the hall home. If the machine sits rotated relative to the
   table, rotate the machine, not the firmware. If the hall magnet is not exactly at the
   launcher +Y direction, correct it with `TURRET_ZERO_TRIM_DEG` (config.h). `BOARD_DEG`
   (params.scad, 8°) puts the board centre so the ±2 × `BOARD_SPREAD_DEG` fan (0..16°) never
   wraps past 0/360°. Check with `seat <n>` and `board <k>` on the console.

## Serial console (calibration / test)

USB serial, 115200 baud, line based (`pio device monitor -b 115200`), accepted only between
hands. Motion commands power up and home first and leave the motors on; `off` parks the
turret at 0° and releases everything (a GO press also works normally). Faults are printed and
release the motors; no GO acknowledgement is needed on the console.

| Command | Does |
|---|---|
| `help` | list commands |
| `status` | players, button, rack count + mask, battery, motor state, rack/turret position, trims |
| `vbat` | battery voltage and class |
| `sensors` | raw IR_LOAD, IR_EXIT, HOME_TURN, HOME_RACK, GO, SET |
| `on` / `off` / `home` | power up + home / park + power down / forget zeros and re-home both axes |
| `rack eject <shelf>` / `rack load <shelf>` | move shelf 0..51 to the eject / load station |
| `rack mm <mm>` / `rack steps <n>` | absolute rack position from home (clamped to travel) |
| `turret <deg>` / `seat <n>` / `board <k>` | aim at an angle / seat 1..N / board slot 0..4 |
| `launch <pwm> [ms]` | run the launcher (default 2000 ms, 0 = leave on; `launch 0` stops) |
| `feed <pwm> [ms]` | run the loader roller (default 1000 ms) |
| `servo <angle>` | pusher servo angle |
| `deal seat <n>` / `deal board <k>` / `deal burn` | deal one random card with that aim and launch profile |
| `loadone` | feed one card into the next empty shelf, print the beam-blocked ms |
| `load` | fill empty shelves from the hopper |
| `purge` | push every full shelf into the burn tray |
| `clear` | forget the rack map (after emptying the rack by hand) |

## Notes

* Rack and turret positions are always computed as geometry (params.scad) + trim (config.h);
  there is no other override of pitch, station offset, steps/mm or steps/deg.
* Randomness: `esp_random()` is a true RNG while Wi-Fi/BT run; with radios off it is still
  seeded from hardware noise. `randBelow()` uses rejection sampling, so every occupied shelf is
  equally likely (tested with chi-square over 520 000 draws).
* The rack map is saved to NVS after each load, each street, at power-down and on every fault
  (only when it changed). If the machine loses power mid-deal, the map can be wrong: that
  shows up as `NO CARD IN SHELF` or `CARD COUNT != 52`; purge (hold SET at power-on) to reset.
* A loader jam leaves the jammed card out of the map: remove it and feed it again.
