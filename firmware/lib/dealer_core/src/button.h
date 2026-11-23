// Non-blocking debounced button with short / long press detection.
// Call update() every loop with the raw level (true = pressed).
#pragma once
#include <stdint.h>

namespace dealer {

enum class Press : uint8_t { None, Short, Long };

class Button {
 public:
  Button(uint32_t debounceMs, uint32_t longMs) : debounceMs_(debounceMs), longMs_(longMs) {}

  // Long fires once as soon as the press reaches longMs (while still held);
  // its release then produces nothing. Short fires on release of a shorter press.
  Press update(bool rawPressed, uint32_t nowMs);
  bool pressed() const { return stable_; }

 private:
  uint32_t debounceMs_, longMs_;
  bool raw_ = false;      // last raw sample
  bool stable_ = false;   // debounced level
  bool longFired_ = false;
  bool armed_ = false;    // a press started after init (ignore a button held at boot)
  bool initialized_ = false;
  uint32_t rawSince_ = 0;    // when raw_ last changed
  uint32_t pressStart_ = 0;  // when stable_ went true
};

}  // namespace dealer
