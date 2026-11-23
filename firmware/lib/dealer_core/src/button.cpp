#include "button.h"

namespace dealer {

Press Button::update(bool rawPressed, uint32_t now) {
  if (!initialized_) {
    initialized_ = true;
    raw_ = stable_ = rawPressed;
    rawSince_ = now;
    armed_ = false;  // a button held at boot must be released first
    return Press::None;
  }
  if (rawPressed != raw_) {
    raw_ = rawPressed;
    rawSince_ = now;
  }
  Press out = Press::None;
  if (raw_ != stable_ && (uint32_t)(now - rawSince_) >= debounceMs_) {
    stable_ = raw_;
    if (stable_) {
      pressStart_ = now;
      longFired_ = false;
      armed_ = true;
    } else {
      if (armed_ && !longFired_) out = Press::Short;
      armed_ = false;
    }
  }
  if (stable_ && armed_ && !longFired_ && (uint32_t)(now - pressStart_) >= longMs_) {
    longFired_ = true;
    out = Press::Long;
  }
  return out;
}

}  // namespace dealer
