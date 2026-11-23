// Buttons, status LED and the optional SSD1306 OLED.
#pragma once
#include <stdint.h>

#include "dealer_core.h"

struct UiStatus {
  dealer::State state;
  bool busy;
  uint8_t players;
  uint8_t button;
  int cards;
  float vbat;
  dealer::BatteryLevel battery;
  dealer::Fault fault;
  const char* message;  // one free-form line ("Dealing flop", "Misdeal", ...), may be ""
};

namespace ui {
void begin();
bool oledPresent();

// Debounced button events, call often.
dealer::Press pollGo();
dealer::Press pollSet();
bool setHeldRaw();  // raw SET level (boot-time purge check)
bool goHeldRaw();   // raw GO level (boot-time service mode check)
// Static service-mode screen.
void drawService(float vbat, int cards);

// LED: base pattern plus an optional one-shot count overlay.
void setPattern(dealer::LedPattern p);
void showCount(uint8_t n);
void tick();  // drive the LED, call often

// Redraw the OLED (rate limited unless force).
void draw(const UiStatus& s, bool force = false);
}  // namespace ui
