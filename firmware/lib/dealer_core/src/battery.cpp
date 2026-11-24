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
    case BatteryLevel::Invalid: return "ERR";
  }
  return "?";
}

BatteryLevel VbatMonitor::update(float v) {
  const bool absentNow = v < cfg::VBAT_ABSENT;
  const bool absentBefore = have_ && last_ < cfg::VBAT_ABSENT;
  float jump = have_ ? v - last_ : 0.0f;
  if (jump < 0.0f) jump = -jump;
  if (!(v >= 0.0f) || v > cfg::VBAT_MAX_PLAUSIBLE) {  // also catches NaN
    level_ = BatteryLevel::Invalid;
  } else if (have_ && !absentNow && !absentBefore && jump > cfg::VBAT_MAX_JUMP) {
    level_ = BatteryLevel::Invalid;
  } else {
    level_ = classifyBattery(v);
  }
  have_ = true;
  last_ = v;
  return level_;
}

}  // namespace dealer
