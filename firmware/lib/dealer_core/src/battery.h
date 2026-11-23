// Battery sense (100k/22k divider on GPIO34) and the start-of-hand check.
#pragma once
#include <stdint.h>

namespace dealer {

enum class BatteryLevel : uint8_t {
  Ok,      // >= VBAT_WARN
  Warn,    // VBAT_MIN <= v < VBAT_WARN: play allowed, warning shown
  Refuse,  // VBAT_ABSENT <= v < VBAT_MIN: refuse to start a hand
  Absent,  // < VBAT_ABSENT: no battery (USB only), motors unpowered: refuse
};

// Pin voltage in millivolts (ADC, calibrated) -> battery volts.
float vbatFromPinMv(float pinMv);
BatteryLevel classifyBattery(float volts);
inline bool batteryAllowsStart(BatteryLevel l) {
  return l == BatteryLevel::Ok || l == BatteryLevel::Warn;
}
const char* batteryText(BatteryLevel l);

}  // namespace dealer
