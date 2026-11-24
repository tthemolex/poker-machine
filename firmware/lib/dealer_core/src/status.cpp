#include "status.h"

namespace dealer {

const char* faultText(Fault f) {
  switch (f) {
    case Fault::None: return "";
    case Fault::TurretHoming: return "TURRET HOMING FAILED";
    case Fault::RackHoming: return "RACK HOMING FAILED";
    case Fault::LoaderJam: return "LOADER JAM";
    case Fault::DoubleFeed: return "DOUBLE FEED";
    case Fault::CountMismatch: return "CARD COUNT != 52";
    case Fault::NoCardInShelf: return "NO CARD IN SHELF";
    case Fault::LauncherJam: return "LAUNCHER JAM";
    case Fault::RackEmpty: return "RACK EMPTY";
    case Fault::BatteryLow: return "BATTERY LOW-CHARGE";
    case Fault::NoBattery: return "NO BATTERY (USB)";
    case Fault::ExtraCards: return "MORE THAN 52 CARDS";
    case Fault::BatterySense: return "BATTERY SENSE ERROR";
  }
  return "?";
}

bool ledLevel(LedPattern p, uint32_t ms, uint8_t n) {
  switch (p) {
    case LedPattern::Off: return false;
    case LedPattern::Busy: return true;
    case LedPattern::Ready: return (ms % 2000u) < 60u;
    case LedPattern::ReadyLowBat: {
      const uint32_t t = ms % 2000u;
      return t < 60u || (t >= 200u && t < 260u);
    }
    case LedPattern::InHand: return (ms % 1000u) < 500u;
    case LedPattern::Fault: return (ms % 200u) < 100u;
    case LedPattern::Service: return (ms % 2000u) < 1800u;
    case LedPattern::Count:
      if (ms >= 300u * n) return false;
      return (ms % 300u) < 150u;
  }
  return false;
}

}  // namespace dealer
