#include "ui.h"

#include <Arduino.h>

#include "config.h"

#if HAS_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#endif

using dealer::LedPattern;

namespace {

dealer::Button goBtn(cfg::DEBOUNCE_MS, cfg::LONG_PRESS_MS);
dealer::Button setBtn(cfg::DEBOUNCE_MS, cfg::LONG_PRESS_MS);

LedPattern pattern = LedPattern::Ready;
uint32_t patternStart = 0;
uint8_t countN = 0;
uint32_t countStart = 0;
bool countActive = false;

bool oledOk = false;

#if HAS_OLED
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
uint32_t lastDraw = 0;

const char* stateText(dealer::State s) {
  switch (s) {
    case dealer::State::WaitStart: return "READY";
    case dealer::State::WaitFlop: return "WAIT FLOP";
    case dealer::State::WaitTurn: return "WAIT TURN";
    case dealer::State::WaitRiver: return "WAIT RIVER";
  }
  return "?";
}

const char* hintText(const UiStatus& s) {
  if (s.fault != dealer::Fault::None) return "Fix, then press GO";
  if (s.busy) return "Working...";
  switch (s.state) {
    case dealer::State::WaitStart: return "GO: load + deal";
    case dealer::State::WaitFlop: return "GO: flop";
    case dealer::State::WaitTurn: return "GO: turn";
    case dealer::State::WaitRiver: return "GO: river";
  }
  return "";
}
#endif  // HAS_OLED

}  // namespace

namespace ui {

void begin() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  pinMode(PIN_BTN_GO, INPUT_PULLUP);
  pinMode(PIN_BTN_SET, INPUT_PULLUP);
#if HAS_OLED
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  Wire.beginTransmission(cfg::OLED_I2C_ADDR);
  if (Wire.endTransmission() == 0 && oled.begin(SSD1306_SWITCHCAPVCC, cfg::OLED_I2C_ADDR)) {
    oledOk = true;
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.display();
  } else {
    Serial.println("OLED not found, running without screen");
  }
#endif
}

bool oledPresent() { return oledOk; }

dealer::Press pollGo() { return goBtn.update(digitalRead(PIN_BTN_GO) == LOW, millis()); }
dealer::Press pollSet() { return setBtn.update(digitalRead(PIN_BTN_SET) == LOW, millis()); }
bool setHeldRaw() { return digitalRead(PIN_BTN_SET) == LOW; }

void setPattern(LedPattern p) {
  if (p == pattern) return;
  pattern = p;
  patternStart = millis();
  if (p == LedPattern::Busy || p == LedPattern::Fault) countActive = false;
  tick();
}

void showCount(uint8_t n) {
  countN = n;
  countStart = millis();
  countActive = true;
  tick();
}

void tick() {
  const uint32_t now = millis();
  bool level;
  if (countActive && now - countStart < dealer::countPatternMs(countN)) {
    level = dealer::ledLevel(LedPattern::Count, now - countStart, countN);
  } else {
    countActive = false;
    level = dealer::ledLevel(pattern, now - patternStart);
  }
  digitalWrite(PIN_LED, level ? HIGH : LOW);
}

void draw(const UiStatus& s, bool force) {
#if HAS_OLED
  if (!oledOk) return;
  const uint32_t now = millis();
  if (!force && now - lastDraw < cfg::OLED_REFRESH_MS) return;
  lastDraw = now;
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.printf("POKER DEALER %5.2fV\n", static_cast<double>(s.vbat));
  oled.printf("Batt: %s\n", dealer::batteryText(s.battery));
  oled.printf("State: %s\n", s.busy ? "BUSY" : stateText(s.state));
  oled.printf("Players: %u  Btn: %u\n", static_cast<unsigned>(s.players),
              static_cast<unsigned>(s.button + 1));
  oled.printf("Rack: %d/%d\n", s.cards, static_cast<int>(geom::SHELF_COUNT));
  if (s.fault != dealer::Fault::None) {
    oled.fillRect(0, 40, 128, 9, SSD1306_WHITE);
    oled.setTextColor(SSD1306_BLACK);
    oled.setCursor(0, 41);
    oled.print(dealer::faultText(s.fault));
    oled.setTextColor(SSD1306_WHITE);
  } else {
    oled.setCursor(0, 41);
    oled.print(s.message ? s.message : "");
  }
  oled.setCursor(0, 56);
  oled.print(hintText(s));
  oled.display();
#else
  (void)s;
  (void)force;
#endif
}

}  // namespace ui
