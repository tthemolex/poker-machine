#include "battery.h"

#include "config.h"

namespace dealer {

float vbatFromPinMv(float pinMv) { return pinMv / 1000.0f * cfg::VBAT_RATIO * cfg::VBAT_CAL; }

BatteryLevel classifyBattery(float v) {
  if (v < cfg::VBAT_ABSENT) return BatteryLevel::Absent;
  if (v < cfg::VBAT_MIN) return BatteryLevel::Refuse;
  if (v < cfg::VBAT_WARN) return BatteryLevel::Warn;
  return BatteryLevel::Ok;
}

const char* batteryText(BatteryLevel l) {
  switch (l) {
    case BatteryLevel::Ok: return "OK";
    case BatteryLevel::Warn: return "LOW";
    case BatteryLevel::Refuse: return "EMPTY";
    case BatteryLevel::Absent: return "NONE";
  }
  return "?";
}

}  // namespace dealer
