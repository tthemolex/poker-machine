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
    homing.*            bounded turret homing search (window around the believed angle)
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

1. Flash with the battery off (USB only). The firmware reads the battery first: below
   `VBAT_ABSENT` (3 V) it shows `NO BATTERY (USB)` (below 10.5 V: `BATTERY LOW-CHARGE`), blinks
   fast and **does not drive any motor**. The serial console is not available in this state.
2. Before switching the battery on, turn the turret by hand to the home mark (magnet over the
   hall sensor). Switch the battery on and press GO: the battery is re-read, then the rack
   homes down to the microswitch, the turret homes to the hall sensor (bounded search, see
   "Turret homing"), parks at 0° and powers down.
3. If an axis runs the wrong way, flip `TURN_DIR_INVERT` / `RACK_DIR_INVERT` in `config.h`.
   The turret must turn **clockwise seen from above** for positive angles (seats are numbered
   clockwise), the rack must move **up** for positive steps.
4. Run the calibration below.

### Boot without a battery (USB only)

`setup()` enables nothing: the TMC2209 EN line is driven HIGH (outputs off) as the very first
motor step and the servo is not attached. It then reads the battery. Below `VBAT_ABSENT` (3 V)
it prints `Battery 0.xx V: not homing.` and `FAULT: NO BATTERY (USB)  (fix it, then press GO)`,
the OLED shows `NO BATTERY (USB)` in inverse video with `Fix, then press GO`, and the LED
blinks fast. Between 3 V and 10.5 V the same happens with `BATTERY LOW-CHARGE`. Nothing homes
and nothing moves. Switching the battery on does **not** start anything by itself: the
firmware waits for **GO**, then re-reads the battery; if it is now ≥ 10.5 V it homes
immediately (rack, then turret), parks at 0° and powers down. If it is still low, the fault
is shown again.

### Service / no-motion mode (setting VREF)

Hold **GO** while powering on or pressing the ESP32 EN/RST button, release after the LED
comes on. The firmware then:

* keeps EN HIGH (TMC2209 outputs disabled) and the servo detached, never homes, and skips the
  battery gate, so VM (12 V) can be on with the motors connected while the VREF pots are
  measured and adjusted (VREF is measurable with VM + VIO powered and EN high);
* prints `SERVICE MODE: no motion, steppers disabled (EN high), servo detached. Battery
  12.xx V. Set VREF now. Console: help. Reset to exit.`;
* shows on the OLED: `SERVICE / NO MOTION`, `Motors off (EN high)`, `Batt: 12.xxV`,
  `Rack: n/52`, `Set TMC2209 VREF now`, `Serial console: help`, `Reset to exit`;
* LED: on with a short gap every 2 s;
* serial console: `help`, `status`, `vbat`, `sensors`, `clear` work; every command that
  would power or move a motor (`on`, `off`, `home`, `rack`, `turret`, `seat`, `board`,
  `launch`, `feed`, `servo`, `deal`, `load`, `loadone`, `purge`) answers
  `service mode: no motion (reset to exit)`. The buttons do nothing.

Exit: reset or power-cycle without holding GO. (Holding SET at power-on is the purge, not
this mode.)

## Using it

Seats are numbered 1..N clockwise starting from the first seat after the board (seat 1 is
just clockwise of the board). The board lands next to the machine at `BOARD_DEG` (8°, fanned
0..16°), in the gap between seat N and seat 1; burned cards land on a burn spot next to it
at `BURN_DEG` (22°), before seat 1. The hall home / park position is 0°. The dealer button is shown as a seat number on the OLED and by LED blinks.

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
| Power-on | hold SET | Purge: throw every shelf the rack map marks as full onto the burn spot, clear the map |
| Power-on / reset | hold GO | SERVICE / NO MOTION mode (set TMC2209 VREF safely), exit by reset |

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
| On, 200 ms gap every 2 s | SERVICE / NO MOTION mode |
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
| `DOUBLE FEED` | loader beam blocked > `DOUBLE_MS` (but < `JAM_MS`) | Two cards went into one shelf | Press GO: the machine throws that shelf (both cards, `LAUNCH_PURGE`) onto the burn spot. Put those cards back in the hopper and press GO. |
| `CARD COUNT != 52` | rack map count after loading | The hopper ran empty before 52 cards were in the rack (card missing, or misfeeds) | Find the missing card(s), put them in the hopper, press GO to acknowledge, press GO again: only the empty shelves are filled. |
| `NO CARD IN SHELF` | launcher beam not blocked within `EXIT_WAIT_MS` after pushing | The map said the shelf was full but nothing came out (map wrong, or pusher/blade problem) | The shelf is marked empty. Press GO; the hand is aborted. Check the pusher. Re-collect cards and start a new hand (the loader refills the shelf). If it repeats, purge (hold SET at power-on) and reload. |
| `LAUNCHER JAM` | launcher beam blocked > `EXIT_JAM_MS` | The card stopped in the launcher nip | Remove the card from the launcher, press GO. The shelf is already marked empty. Hand aborted. |
| `RACK EMPTY` | rack map count = 0 during a deal | Should not happen after a verified load (map corrupted) | Press GO, start a new hand (it reloads). |
| `TURRET HOMING FAILED` | hall sensor not found inside the bounded search window (see "Turret homing"), sensor stuck active, or `TURN_HOME_TIMEOUT_MS` watchdog | Turret not near where the firmware expects it (turned by hand, power lost mid-deal), or hall sensor / magnet / belt / motor problem; no 12 V | **Turn the turret by hand to the home mark** (magnet over the sensor), press GO. If it repeats there: check magnet gap (≤ 3 mm), A3144 wiring, belt tension. |
| `MORE THAN 52 CARDS` | rack full, then the loader beam still sees a card from the hopper | Extra / foreign card in the deck | The probed card is pulled back into the hopper. Remove the extra cards from the hopper, press GO. The 52 loaded cards stay in the rack. |
| `RACK HOMING FAILED` | bottom microswitch not seen within `RACK_HOME_TIMEOUT_MS`, or stuck pressed | Rack stuck, switch broken, motor wiring; no 12 V | Check rods/leadscrew for binding, switch wiring. Press GO and retry. |
| `BATTERY LOW-CHARGE` | battery < `VBAT_MIN` (10.5 V) at GO | Battery too low to finish a hand reliably | Charge the TalentCell (its own charger via the panel DC jack). Hand refused. |
| `NO BATTERY (USB)` | battery < `VBAT_ABSENT` (3 V) | Running from USB only, the motors have no power | Switch the battery on. |

## Turret homing

The turret has a single hall sensor at 0° and no absolute position, and its cables allow
0..360° only. So homing never sweeps blindly: it searches a bounded window around where the
firmware believes the turret is (`dealer::TurretHoming`, tested in `test/test_homing`):

1. Believed angle `est`: 0 if parked (every power-down parks at 0°, and this is assumed at
   boot), otherwise the step counter after a fault (the motors were released, the turret may
   have been turned by hand).
2. If the hall is already active, go to step 5.
3. Seek negative (counter-clockwise) down to `est + TURN_SEARCH_WINDOW_DEG` below the start.
4. Then seek positive up to `TURN_SEARCH_WINDOW_DEG` above the start (a turret nudged
   counter-clockwise past the magnet is found from the other side). Not found: fault
   `TURRET HOMING FAILED`. It never sweeps further.
5. Found from either side: move positive until the hall releases (at most `TURN_ZONE_MAX_DEG`,
   else "stuck sensor" fault), `TURN_BACKOFF_DEG` further, then approach **slowly in the
   negative direction** (`TURN_HOMING_SLOW`). The first hall edge is 0°. The zero is always the
   same edge, approached from the same side, so it is repeatable.

Worst case the turret goes `TURN_SEARCH_WINDOW_DEG` (25°) past where it started in either
direction, so the cable service loop must allow about ±25° beyond the 0..360° range. If power
is lost mid-deal or the turret was turned far by hand, homing faults instead of winding the
cables: turn the turret to the home mark and press GO.

The rack homes against the bottom microswitch (always a safe direction: the rack can only go
down to it).

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
* **At boot** the battery is checked before anything moves: below 10.5 V (or no battery) the
  firmware shows the fault and waits; GO re-reads the battery. No homing on USB power only.
* The serial console refuses motion commands below 10.5 V.
* < 3 V: no battery / USB only (`NO BATTERY (USB)`).

## Calibration constants (`firmware/include/config.h`)

Geometry is **not** in config.h: it comes from `geometry.h` (`geom::SHELF_PITCH`,
`geom::STATION_OFFSET`, `geom::RACK_STEPS_PER_MM`, `geom::TURRET_STEPS_PER_DEG`,
`geom::BOARD_DEG`, `geom::BOARD_SPREAD_DEG`), generated from `cad/params.scad`. Change those
in params.scad and regenerate.

**Angle frame:** hall home = **0°** (turret homes and parks there; the magnet marks the
launcher +Y direction). The board centre is at **`BOARD_DEG`** (8°, params.scad), fanned
±2 × `BOARD_SPREAD_DEG` = 0..16°. Burns go at very low speed (`LAUNCH_BURN`) onto a **burn
spot on the table at `BURN_DEG`** (22°, params.scad), between the board fan and seat 1. There
is no burn tray. Seats sit at `BOARD_DEG + (seat + 0.5) × 360 / N`: seat 1 is at ≥ 26°
(N = 10), the highest at 350°. So every target is inside [0, 360) without wrapping and the
turret never makes a near-full-turn move (`test_all_angles_in_range_no_wrap`,
`test_seat_angles_even_and_in_range`). `normDeg()` is only a safety net. Purge (hold SET at
power-on, console `purge`) and double-feed clearing also throw onto the burn spot, with
`LAUNCH_PURGE` (default `LAUNCH_BOARD`) so two stacked cards clear the nip.

Rack positions (from the homed bottom position, mm): `eject(i) = (51 − i) × pitch`,
`load(i) = eject(i) + STATION_OFFSET`. Shelf 0 is the bottom shelf; homed = shelf 51 at the
eject station. Seats: `BOARD_DEG + (seat + 0.5) × 360 / N`; board slot k:
`BOARD_DEG + (k − 2) × BOARD_SPREAD_DEG`; burns at `BURN_DEG`. All wrapped into 0..360°.

| Constant | Default | What it does | How to calibrate |
|---|---|---|---|
| `RACK_ZERO_TRIM_MM` | 0.0 | Added to every rack target (eject and load). + = rack higher | Home, `rack eject 51`: shelf 51 must sit exactly at the eject station (pusher blade / launcher nip). Measure the error, set the trim, rebuild. Check with `rack eject 0` / `rack load 0`. |
| `TURRET_ZERO_TRIM_DEG` | 0.0 | Added to every turret target angle. + = more clockwise | `turret 0`: the launcher must point exactly along the machine's +Y mark. If not, set the trim to the error (degrees). |
| `TURRET_PARK_DEG` | 0.0 | Idle park angle (on the hall sensor) | Leave at 0 (spec). |
| `TURN_MAX_SPEED`, `TURN_ACCEL` | 6000 st/s, 12000 st/s² | Turret motion | Raise until it skips steps, then back off 30 %. |
| `RACK_MAX_SPEED`, `RACK_ACCEL` | 16000 st/s, 40000 st/s² | Rack motion (80 mm/s) | Same; check for lost steps by dealing a full deck and confirming the last shelves still line up. |
| `TURN_HOMING_SPEED`, `RACK_HOMING_SPEED` | 1500, 3000 st/s | Homing seek speed | Slower = more repeatable zero. |
| `TURN_BACKOFF_DEG`, `RACK_BACKOFF_MM` | 5°, 3 mm | Turret: extra travel after the hall releases, before the slow approach. Rack: back-off when the switch is already pressed | Rack: must exceed the switch travel. |
| `TURN_SEARCH_WINDOW_DEG` | 25° | Turret homing search margin around the believed angle | Larger tolerates bigger hand nudges but lets the cables go further past 0..360. |
| `TURN_ZONE_MAX_DEG` | 15° | Hall active longer than this = stuck sensor | > the measured active zone of the A3144 + magnet (typically 3–8°). |
| `TURN_HOMING_SLOW` | 400 st/s | Final slow approach to the hall edge (~4.5°/s) | Slower = more repeatable zero. |
| `TURN_DIR_INVERT`, `RACK_DIR_INVERT` | false | Flip motor direction in software | See first power-up. |
| `TURN_HOME_TIMEOUT_MS`, `RACK_HOME_TIMEOUT_MS`, `BACKOFF_TIMEOUT_MS` | 40000 / 8000 / 3000 | Homing watchdogs (the turret search is bounded by angle, not time) | Rack: longer than the full 81 mm travel at homing speed. |
| `LAUNCH_SEAT` | 255 | Launcher PWM for a throw to a seat | Deal hole cards to 10 seats; lower it if cards fly off the table, the farthest seat must still be reached. |
| `LAUNCH_BOARD` | 110 | Launcher PWM for the board (short throw next to the machine) | Board cards should land 10–15 cm from the base and not overlap seat 1 / seat N. |
| `LAUNCH_BURN` | 55 | Launcher PWM for a burn: very low throw onto the burn spot at `BURN_DEG` | Lowest PWM at which the card reliably leaves the nip and lands clear of the base, close to the machine, not on the board fan or seat 1's cards. |
| `LAUNCH_PURGE` | = `LAUNCH_BOARD` | Launcher PWM for purge and double-feed clearing (to the burn spot; may be two stacked cards) | Two stacked cards must leave the nip without a `LAUNCHER JAM`. |
| `LAUNCH_RAMP_MS` | 120 | Soft PWM ramp (12 steps) whenever the launcher speeds up | Limits the 130 motor's inrush (stall current 1.5–2.5 A at 5 V) so the DRV8833 over-current protection does not trip and the 5 V rail does not dip. Increase if the ESP32 browns out at spin-up. |
| `SPINUP_MS` | 250 | Wait after the ramp / a speed change | Increase if the first card after a speed change flies differently. |
| `EXTRA_CARD_PROBE`, `PROBE_REVERSE_MS` | true, 300 | 53rd-card check after the rack is full; reverse time to pull the probed card back | Reverse long enough that the card is back behind the roller nip; set false to skip the check. |
| `HW_ENTROPY_DURING_DEAL` | true | SAR-ADC entropy for `esp_random()` while dealing (see Notes) | Leave on. |
| `CONSOLE_IDLE_MS` | 60000 | Console motors power down after this long without a command | |
| `FEED_PWM` | 170 | Loader roller PWM | Lowest reliable feed; higher speeds make double feeds more likely. |
| `NO_CARD_MS` | 1500 | No card at the loader beam within this = hopper empty | Must exceed the time for one card to reach the beam. |
| `DOUBLE_MS` | 350 | Beam blocked longer = double feed | Console `loadone` prints the single-card block time; over a full deck set DOUBLE_MS ≈ 1.5 × the typical time and well below 2 ×. |
| `JAM_MS` | 1000 | Beam blocked longer = loader jam | > 2 × single-card time. |
| `SEAT_MS` | 150 | Extra roller run after the card clears the beam | Card must sit flush against the stop fingers; too long scuffs it. |
| `PUSH_HOME`, `PUSH_OUT` | 10°, 160° | Pusher servo angles = `geom::PUSH_HOME_DEG` / `geom::PUSH_OUT_DEG` (params.scad: pinion R 7.5 mm, 150° sweep = 19.6 mm stroke) + trims | Not edited directly. HOME: blade behind the rack's back face (rack moves freely). OUT: the blade stops at Y 6.6, mid-rack (not through the shelf), with the card pushed ~5 mm into the launcher nip. |
| `PUSH_HOME_TRIM_DEG`, `PUSH_OUT_TRIM_DEG` | 0, 0 | Servo-horn spline offset correction | Console `servo <angle>`: find the real retracted / full-stroke angles, put the difference here. |
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
hands. Motion commands check the battery, power up and home first and leave the motors on;
`off` parks the turret at 0° and releases everything (a GO press also works normally). After
`CONSOLE_IDLE_MS` (60 s) without a command the console powers everything down by itself
(steppers off, servo detached, loader and launcher stopped), so a forgotten session does not
hold the steppers. Faults are printed and
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
| `purge` | throw every full shelf onto the burn spot |
| `clear` | forget the rack map (after emptying the rack by hand) |

## Notes

* Rack and turret positions are always computed as geometry (params.scad) + trim (config.h);
  there is no other override of pitch, station offset, steps/mm or steps/deg.
* Randomness: `esp_random()` is a true RNG only while an entropy source runs (Wi-Fi/BT, or
  the SAR-ADC noise source). The radios are never on, so `Machine::executePlan()` and
  `purgeAll()` call `bootloader_random_enable()` for the duration of the deal and
  `bootloader_random_disable()` afterwards (`EntropyScope`). The entropy source uses the SAR
  ADC, which the battery sense (GPIO34, ADC1) also needs, so it is never on while the ADC is
  read: battery reads only happen between deals, and `readVbat()` re-applies the ADC width and
  attenuation and discards the first conversion. (Check on the hardware that `vbat` reads the
  same before and after a deal; if not, set `HW_ENTROPY_DURING_DEAL = false`, which falls back
  to the hardware-seeded pseudo-random stream.) `randBelow()` uses rejection sampling, so every
  occupied shelf is equally likely (tested with chi-square over 520 000 draws).
* The rack map is saved to NVS after each load, each street, at power-down and on every fault
  (only when it changed). If the machine loses power mid-deal, the map can be wrong: that
  shows up as `NO CARD IN SHELF` or `CARD COUNT != 52`; purge (hold SET at power-on) to reset.
* A loader jam leaves the jammed card out of the map: remove it and feed it again.
* 53rd-card probe: after the 52nd card the rack moves to its homed position, where the load
  station is `STATION_OFFSET` (3 pitches) above shelf 51, so no shelf faces it
  (`rackNoShelfAtLoadMm()`, tested). The roller runs until the loader beam sees a leading
  edge (max `NO_CARD_MS`), stops at once and reverses for `PROBE_REVERSE_MS`. A card is never
  pushed into a full shelf; at worst its leading edge touches the rack top plate. This needs
  the rack top plate (above shelf 51) to cover the load station at the homed position, and the
  loader beam to sit before the rack face. If the beam is placed so that a card can only
  reach it once it is past the reverse-able roller, set `EXTRA_CARD_PROBE = false`; then a 53rd
  card is not detected (known limitation).
