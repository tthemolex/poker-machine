// =============================================================================
//  config.h: pins and calibration constants for the poker dealer firmware.
//
//  * Pins: one `#define PIN_<NAME> <gpio>` per line. tools/ scripts parse
//    `#define PIN_(\w+)\s+(\d+)` and cross-check them against the pin table and
//    the wiring diagram, so keep that exact form.
//  * Geometry (shelf pitch, station offset, steps/mm, steps/deg, board angle)
//    is NOT defined here: it comes from geometry.h, which is generated from
//    cad/params.scad by tools/gen_geometry.py.
//  * Everything else (speeds, PWMs, timeouts, servo angles, battery) is a
//    calibration value; see docs/FIRMWARE.md "Calibration" for how to tune it.
//
//  This header is plain C++ (no Arduino includes) so the host tests can use it.
// =============================================================================
#pragma once
#include <stdint.h>

#include "geometry.h"

// ---------------- Pins (ESP32 DevKit V1, unchanged from the draft) ----------------
#define PIN_TURN_STEP 25   // TMC2209 #1 STEP (turret)
#define PIN_TURN_DIR 26    // TMC2209 #1 DIR
#define PIN_RACK_STEP 27   // TMC2209 #2 STEP (rack leadscrew)
#define PIN_RACK_DIR 14    // TMC2209 #2 DIR
#define PIN_STEP_EN 13     // TMC2209 EN (shared), active LOW
#define PIN_FEED_IN1 32    // DRV8833 IN1/AIN1 -> loader N20 roller (PWM)
#define PIN_FEED_IN2 33    // DRV8833 IN2/AIN2
#define PIN_LAUNCH 19      // DRV8833 IN3/BIN1 (IN4/BIN2 tied to GND) -> launcher 130 motor (PWM)
#define PIN_PUSHER 23      // MG90S servo signal (pusher blade)
#define PIN_IR_LOAD 15     // IR break-beam at loader, LOW = blocked
#define PIN_IR_EXIT 4      // IR break-beam at launcher entry, LOW = blocked
#define PIN_HOME_TURN 16   // A3144 hall sensor, LOW at turret 0 deg
#define PIN_HOME_RACK 17   // microswitch, LOW at rack bottom
#define PIN_BTN_GO 18      // GO button to GND
#define PIN_BTN_SET 5      // SET button to GND
#define PIN_LED 2          // status LED (on-board LED on DevKit V1)
#define PIN_VBAT 34        // battery sense, 100k/22k divider (input-only pin)
#define PIN_OLED_SDA 21    // SSD1306 I2C SDA (optional)
#define PIN_OLED_SCL 22    // SSD1306 I2C SCL (optional)

// ---------------- Optional OLED ----------------
#ifndef HAS_OLED
#define HAS_OLED 1         // 0 = compile without the SSD1306 screen code
#endif

namespace cfg {

// ---------------- Table ----------------
constexpr uint8_t MIN_PLAYERS = 2;
constexpr uint8_t MAX_PLAYERS = 10;
constexpr uint8_t DEFAULT_PLAYERS = 6;

// ---------------- Calibration trims (measured on the machine) ----------------
// Added to every rack target (eject and load). Positive = rack higher. Set it so
// that, when homed, shelf 51 sits exactly at the eject station (see FIRMWARE.md).
constexpr float RACK_ZERO_TRIM_MM = 0.0f;
// Added to every turret target angle. Positive = more clockwise. Use it when the
// hall magnet is not exactly on the launcher +Y direction (true 0 deg).
constexpr float TURRET_ZERO_TRIM_DEG = 0.0f;
// Turret park / home angle (hall sensor). The spec parks at 0 deg.
constexpr float TURRET_PARK_DEG = 0.0f;

// ---------------- Motion (steps/s, steps/s^2) ----------------
constexpr float TURN_MAX_SPEED = 6000.0f;   // turret stepper max speed
constexpr float TURN_ACCEL = 12000.0f;      // turret acceleration
constexpr float RACK_MAX_SPEED = 16000.0f;  // rack stepper max speed (80 mm/s at 200 steps/mm)
constexpr float RACK_ACCEL = 40000.0f;      // rack acceleration
constexpr float TURN_HOMING_SPEED = 1500.0f;  // turret homing seek speed (~17 deg/s)
constexpr float TURN_HOMING_SLOW = 400.0f;    // final slow approach to the hall edge (~4.5 deg/s)
constexpr float RACK_HOMING_SPEED = 3000.0f;  // rack homing seek speed (15 mm/s)
constexpr float TURN_BACKOFF_DEG = 5.0f;    // travel past the released hall zone before the slow approach
constexpr float TURN_SEARCH_WINDOW_DEG = 25.0f;  // homing search margin around the believed angle
constexpr float TURN_ZONE_MAX_DEG = 15.0f;  // hall active longer than this = stuck sensor
constexpr float RACK_BACKOFF_MM = 3.0f;     // back off the microswitch if already pressed
constexpr bool TURN_DIR_INVERT = false;     // flip if the turret turns counter-clockwise
constexpr bool RACK_DIR_INVERT = false;     // flip if the rack moves down on +steps

// ---------------- Homing timeouts (ms) ----------------
constexpr uint32_t TURN_HOME_TIMEOUT_MS = 40000;   // watchdog; the search itself is bounded by angle
constexpr uint32_t RACK_HOME_TIMEOUT_MS = 8000;    // full 81 mm travel at 15 mm/s is ~5.4 s
constexpr uint32_t BACKOFF_TIMEOUT_MS = 3000;

// ---------------- Launcher PWM (0..255 on DRV8833 B) ----------------
constexpr uint8_t LAUNCH_SEAT = 255;   // full throw to a player
constexpr uint8_t LAUNCH_BOARD = 110;  // short throw to the board next to the machine
constexpr uint8_t LAUNCH_BURN = 55;    // very low throw onto the burn spot (geom::BURN_DEG)
constexpr uint8_t LAUNCH_PURGE = LAUNCH_BOARD;  // purge / double-feed clearing to the burn spot
// Burn kick: once the burn card's tail passes IR_EXIT (still in the nip), run the
// wheel at LAUNCH_BURN_KICK for BURN_KICK_MS so the card clears the exit lip.
// LAUNCH_BURN_KICK <= LAUNCH_BURN disables the kick.
constexpr uint8_t LAUNCH_BURN_KICK = 110;
constexpr uint32_t BURN_KICK_MS = 200;
constexpr uint32_t SPINUP_MS = 250;    // settle time after a launcher speed change
constexpr uint32_t LAUNCH_RAMP_MS = 120;  // soft PWM ramp on spin-up (limits the DRV8833 inrush)
constexpr uint32_t MOTOR_PWM_HZ = 20000;  // DRV8833 PWM frequency (inaudible)

// ---------------- Loader ----------------
constexpr uint8_t FEED_PWM = 170;      // N20 loader roller PWM
constexpr uint32_t NO_CARD_MS = 1500;  // beam never blocked within this -> hopper empty
constexpr uint32_t JAM_MS = 1000;      // beam blocked longer than this -> jam
constexpr uint32_t DOUBLE_MS = 350;    // beam blocked longer than this -> two cards (~1.5x single)
constexpr uint32_t SEAT_MS = 150;      // extra roller run after the card clears the beam
constexpr bool EXTRA_CARD_PROBE = true;   // after the 52nd card, probe the hopper for a 53rd
constexpr uint32_t PROBE_REVERSE_MS = 300;  // pull a probed extra card back into the hopper

// ---------------- Pusher servo (MG90S) ----------------
// Angles come from the CAD (geom::PUSH_HOME_DEG / PUSH_OUT_DEG, pinion R 7.5 mm,
// 150 deg sweep = 19.6 mm stroke); the trims correct servo-horn spline offsets.
constexpr int PUSH_HOME_TRIM_DEG = 0;
constexpr int PUSH_OUT_TRIM_DEG = 0;
// Blade retracted behind the rack's back face: the rack can move freely.
constexpr int PUSH_HOME = geom::PUSH_HOME_DEG + PUSH_HOME_TRIM_DEG;
// Full stroke: the blade stops at Y 6.6 (mid-rack, NOT through the shelf) after
// pushing the card ~5 mm into the launcher nip, which then pulls it out.
constexpr int PUSH_OUT = geom::PUSH_OUT_DEG + PUSH_OUT_TRIM_DEG;
static_assert(PUSH_HOME >= 0 && PUSH_OUT <= 180 && PUSH_HOME < PUSH_OUT, "servo range");
constexpr uint32_t PUSH_RETRACT_MS = 200;  // wait for the blade to clear the rack before moving it
constexpr uint32_t SERVO_SETTLE_MS = 50;

// ---------------- Deal side ----------------
constexpr uint32_t EXIT_WAIT_MS = 1500;  // card must reach the launcher beam within this
constexpr uint32_t EXIT_JAM_MS = 1000;   // launcher beam blocked longer than this -> jam

// ---------------- Battery (3S Li-ion, TalentCell YB1206000-USB) ----------------
constexpr float VBAT_R_TOP_K = 100.0f;  // divider top resistor, kOhm
constexpr float VBAT_R_BOT_K = 22.0f;   // divider bottom resistor, kOhm
constexpr float VBAT_RATIO = (VBAT_R_TOP_K + VBAT_R_BOT_K) / VBAT_R_BOT_K;
constexpr float VBAT_WARN = 10.9f;      // warn (~15 % left)
constexpr float VBAT_MIN = 10.5f;       // refuse to start a hand below this
constexpr float VBAT_ABSENT = 3.0f;     // below this: no battery (USB only), motors unpowered
constexpr float VBAT_MAX_PLAUSIBLE = 14.0f;  // 3S full is 12.6 V: above this = sensor fault
constexpr float VBAT_MAX_JUMP = 1.0f;   // change between consecutive idle reads above this = fault
constexpr float VBAT_CAL = 1.0f;        // multiply reading by this to match a multimeter
constexpr uint8_t VBAT_SAMPLES = 16;

// ---------------- RNG ----------------
// 32-bit words of true entropy collected at boot (bootloader_random_enable, before
// any ADC use) to seed xoshiro256**. 16 words = 512 bits (>= 256 required).
constexpr uint8_t RNG_SEED_WORDS = 16;

// ---------------- Serial console ----------------
constexpr uint32_t CONSOLE_IDLE_MS = 60000;  // power down after this long without a command

// ---------------- Buttons / UI ----------------
constexpr uint32_t DEBOUNCE_MS = 30;
constexpr uint32_t LONG_PRESS_MS = 800;
constexpr uint32_t OLED_REFRESH_MS = 250;
constexpr uint8_t OLED_I2C_ADDR = 0x3C;

}  // namespace cfg
