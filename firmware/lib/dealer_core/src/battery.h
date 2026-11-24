// Battery sense (100k/22k divider on GPIO34) and the start-of-hand check.
#pragma once
#include <stdint.h>

namespace dealer {

enum class BatteryLevel : uint8_t {
  Ok,      // >= VBAT_WARN
  Warn,    // VBAT_MIN <= v < VBAT_WARN: play allowed, warning shown
  Refuse,  // VBAT_ABSENT <= v < VBAT_MIN: refuse to start a hand
  Absent,  // < VBAT_ABSENT: no battery (USB only), motors unpowered: refuse
  Invalid, // implausible reading (> VBAT_MAX_PLAUSIBLE, or a jump > VBAT_MAX_JUMP): refuse
};

// Pin voltage in millivolts (ADC, calibrated) -> battery volts.
float vbatFromPinMv(float pinMv);
BatteryLevel classifyBattery(float volts);
inline bool batteryAllowsStart(BatteryLevel l) {
  return l == BatteryLevel::Ok || l == BatteryLevel::Warn;
}
const char* batteryText(BatteryLevel l);

// Plausibility guard on top of classifyBattery(): a reading above
// VBAT_MAX_PLAUSIBLE, or one that differs from the previous reading by more than
// VBAT_MAX_JUMP, is Invalid (sensor/ADC fault) and refuses a hand instead of
// silently passing the 10.5 V check. Switching the battery on or off (a step
// from or to below VBAT_ABSENT) is not a jump. The comparison is with the
// previous raw reading, so a single glitch costs two readings, then it heals.
class VbatMonitor {
 public:
  BatteryLevel update(float volts);
  BatteryLevel level() const { return level_; }

 private:
  bool have_ = false;
  float last_ = 0.0f;
  BatteryLevel level_ = BatteryLevel::Absent;
};

}  // namespace dealer
