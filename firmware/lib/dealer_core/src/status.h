// Fault codes and status LED patterns (shared by the LED, OLED, serial log
// and docs/FIRMWARE.md).
#pragma once
#include <stdint.h>

namespace dealer {

enum class Fault : uint8_t {
  None,
  TurretHoming,   // hall sensor not found within the timeout
  RackHoming,     // rack bottom switch not found within the timeout
  LoaderJam,      // loader beam blocked longer than JAM_MS
  DoubleFeed,     // loader beam blocked longer than DOUBLE_MS (two cards in one shelf)
  CountMismatch,  // rack holds != 52 cards after loading
  NoCardInShelf,  // pushed a shelf the map says is full, nothing reached the launcher
  LauncherJam,    // launcher beam blocked longer than EXIT_JAM_MS
  RackEmpty,      // a deal was requested with no cards in the rack
  BatteryLow,     // below VBAT_MIN: hand refused
  NoBattery,      // below VBAT_ABSENT: running from USB only
  ExtraCards,     // rack full but the hopper still holds a card (more than 52)
  BatterySense,   // implausible battery reading (VbatMonitor): hand refused
};
constexpr Fault kLastFault = Fault::BatterySense;

// Short text, <= 21 chars (one 128 px OLED line at text size 1).
const char* faultText(Fault f);

enum class LedPattern : uint8_t {
  Off,
  Ready,       // between hands, battery OK: 60 ms blip every 2 s
  ReadyLowBat, // between hands, battery low: double blip every 2 s
  InHand,      // waiting for GO (next street): 1 Hz, 50 %
  Busy,        // moving cards: solid on
  Fault,       // fault, waiting for GO: 5 Hz fast blink
  Count,       // n blinks (150 ms on / 150 ms off) after a SET press, then off
  Service,     // service / no-motion mode: on, with a 200 ms gap every 2 s
};

// LED level for a pattern, `ms` since the pattern started. `n` is used by Count.
bool ledLevel(LedPattern p, uint32_t ms, uint8_t n = 0);
// Length of a Count pattern (after this, go back to the idle pattern).
inline uint32_t countPatternMs(uint8_t n) { return 300u * n + 500u; }

}  // namespace dealer
