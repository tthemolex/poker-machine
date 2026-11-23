// Battery thresholds, button debouncing, LED patterns, loader verdicts, fault texts.
#include <string.h>
#include <unity.h>

#include "dealer_core.h"

using namespace dealer;

void setUp() {}
void tearDown() {}

void test_battery_divider() {
  // 100k/22k: 12.6 V full pack -> 2.272 V at the pin
  TEST_ASSERT_FLOAT_WITHIN(1e-3f, 122.0f / 22.0f, cfg::VBAT_RATIO);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.6f, vbatFromPinMv(12600.0f * 22.0f / 122.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.5f, vbatFromPinMv(10500.0f * 22.0f / 122.0f));
  // full pack stays below the ESP32 ADC's ~3.1 V range at 11 dB
  TEST_ASSERT_TRUE(12.6f / cfg::VBAT_RATIO < 3.1f);
}

void test_battery_thresholds() {
  TEST_ASSERT_EQUAL_FLOAT(10.9f, cfg::VBAT_WARN);
  TEST_ASSERT_EQUAL_FLOAT(10.5f, cfg::VBAT_MIN);
  TEST_ASSERT_EQUAL(BatteryLevel::Ok, classifyBattery(12.6f));
  TEST_ASSERT_EQUAL(BatteryLevel::Ok, classifyBattery(10.9f));
  TEST_ASSERT_EQUAL(BatteryLevel::Warn, classifyBattery(10.89f));
  TEST_ASSERT_EQUAL(BatteryLevel::Warn, classifyBattery(10.5f));
  TEST_ASSERT_EQUAL(BatteryLevel::Refuse, classifyBattery(10.49f));
  TEST_ASSERT_EQUAL(BatteryLevel::Refuse, classifyBattery(5.0f));
  TEST_ASSERT_EQUAL(BatteryLevel::Absent, classifyBattery(0.1f));
  TEST_ASSERT_TRUE(batteryAllowsStart(classifyBattery(11.0f)));
  TEST_ASSERT_TRUE(batteryAllowsStart(classifyBattery(10.6f)));
  TEST_ASSERT_FALSE(batteryAllowsStart(classifyBattery(10.4f)));
  TEST_ASSERT_FALSE(batteryAllowsStart(classifyBattery(0.0f)));
}

// Feed a button a level for `ms` milliseconds in 1 ms ticks, collecting events.
static int shorts, longs;
static uint32_t now;
static void hold(Button& b, bool level, uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    Press p = b.update(level, now++);
    if (p == Press::Short) shorts++;
    if (p == Press::Long) longs++;
  }
}

void test_button_short_long_and_bounce() {
  Button b(cfg::DEBOUNCE_MS, cfg::LONG_PRESS_MS);
  shorts = longs = 0;
  now = 1000;
  hold(b, false, 100);
  // bounce shorter than debounce: nothing
  hold(b, true, 5); hold(b, false, 5); hold(b, true, 5); hold(b, false, 100);
  TEST_ASSERT_EQUAL_INT(0, shorts + longs);
  // short press
  hold(b, true, 200); hold(b, false, 100);
  TEST_ASSERT_EQUAL_INT(1, shorts); TEST_ASSERT_EQUAL_INT(0, longs);
  // long press fires once while held, release adds nothing
  hold(b, true, 2000);
  TEST_ASSERT_EQUAL_INT(1, longs);
  hold(b, false, 100);
  TEST_ASSERT_EQUAL_INT(1, shorts); TEST_ASSERT_EQUAL_INT(1, longs);
  // bouncy release of a short press still yields one short
  hold(b, true, 100); hold(b, false, 3); hold(b, true, 3); hold(b, false, 100);
  TEST_ASSERT_EQUAL_INT(2, shorts);
}

void test_button_held_at_boot_ignored_and_millis_wrap() {
  Button b(cfg::DEBOUNCE_MS, cfg::LONG_PRESS_MS);
  shorts = longs = 0;
  now = 0xFFFFFF00u;  // wraps during the test
  hold(b, true, 3000); hold(b, false, 100);
  TEST_ASSERT_EQUAL_INT(0, shorts + longs);
  hold(b, true, 1000); hold(b, false, 100);
  TEST_ASSERT_EQUAL_INT(1, longs);
}

void test_led_patterns() {
  TEST_ASSERT_TRUE(ledLevel(LedPattern::Busy, 12345));
  TEST_ASSERT_FALSE(ledLevel(LedPattern::Off, 0));
  TEST_ASSERT_TRUE(ledLevel(LedPattern::Fault, 50));
  TEST_ASSERT_FALSE(ledLevel(LedPattern::Fault, 150));
  TEST_ASSERT_TRUE(ledLevel(LedPattern::Ready, 2010));
  TEST_ASSERT_FALSE(ledLevel(LedPattern::Ready, 1000));
  TEST_ASSERT_TRUE(ledLevel(LedPattern::ReadyLowBat, 210));
  TEST_ASSERT_FALSE(ledLevel(LedPattern::Ready, 210));
  TEST_ASSERT_TRUE(ledLevel(LedPattern::Service, 100));
  TEST_ASSERT_FALSE(ledLevel(LedPattern::Service, 1900));
  // Count n: exactly n rising edges
  for (uint8_t n = 1; n <= 10; n++) {
    int edges = 0;
    bool prev = false;
    for (uint32_t t = 0; t < countPatternMs(n); t++) {
      bool l = ledLevel(LedPattern::Count, t, n);
      if (l && !prev) edges++;
      prev = l;
    }
    TEST_ASSERT_EQUAL_INT(n, edges);
  }
}

void test_loader_verdicts() {
  TEST_ASSERT_EQUAL(FeedVerdict::HopperEmpty, feedVerdict(false, 0, false));
  TEST_ASSERT_EQUAL(FeedVerdict::Single, feedVerdict(true, 120, false));
  TEST_ASSERT_EQUAL(FeedVerdict::Single, feedVerdict(true, cfg::DOUBLE_MS, false));
  TEST_ASSERT_EQUAL(FeedVerdict::Double, feedVerdict(true, cfg::DOUBLE_MS + 1, false));
  TEST_ASSERT_EQUAL(FeedVerdict::Jam, feedVerdict(true, cfg::JAM_MS + 1, false));
  TEST_ASSERT_EQUAL(FeedVerdict::Jam, feedVerdict(true, 10, true));
  TEST_ASSERT_TRUE(cfg::DOUBLE_MS < cfg::JAM_MS);
}

void test_fault_texts_fit_oled() {
  for (int f = 0; f <= static_cast<int>(kLastFault); f++) {
    const char* t = faultText(static_cast<Fault>(f));
    TEST_ASSERT_NOT_NULL(t);
    TEST_ASSERT_TRUE(strlen(t) <= 21);
    if (f) TEST_ASSERT_TRUE(strlen(t) > 0);
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_battery_divider);
  RUN_TEST(test_battery_thresholds);
  RUN_TEST(test_button_short_long_and_bounce);
  RUN_TEST(test_button_held_at_boot_ignored_and_millis_wrap);
  RUN_TEST(test_led_patterns);
  RUN_TEST(test_loader_verdicts);
  RUN_TEST(test_fault_texts_fit_oled);
  return UNITY_END();
}
