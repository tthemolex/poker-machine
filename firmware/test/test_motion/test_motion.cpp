// Rack position math and turret angles (all from geometry.h).
#include <math.h>
#include <unity.h>

#include "dealer_core.h"

using namespace dealer;

void setUp() {}
void tearDown() {}

void test_rack_eject_positions() {
  const float t = cfg::RACK_ZERO_TRIM_MM;
  TEST_ASSERT_FLOAT_WITHIN(1e-5f, t, rackEjectMm(51));  // homed: shelf 51 at eject
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, 51 * geom::SHELF_PITCH + t, rackEjectMm(0));
  for (int i = 0; i < 51; i++)
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, geom::SHELF_PITCH, rackEjectMm(i) - rackEjectMm(i + 1));
}

void test_rack_load_positions_and_travel() {
  for (int i = 0; i < 52; i++) {
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, rackEjectMm(i) + geom::STATION_OFFSET, rackLoadMm(i));
    TEST_ASSERT_TRUE(rackEjectMm(i) >= 0.0f && rackEjectMm(i) <= kRackMaxMm + 1e-4f);
    TEST_ASSERT_TRUE(rackLoadMm(i) >= 0.0f && rackLoadMm(i) <= kRackMaxMm + 1e-4f);
  }
  // Shelf 0 at the load station is the top of the travel.
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, geom::RACK_TRAVEL + cfg::RACK_ZERO_TRIM_MM, rackLoadMm(0));
}

void test_rack_steps() {
  for (int i = 0; i < 52; i++) {
    TEST_ASSERT_EQUAL_INT32(lroundf(rackEjectMm(i) * geom::RACK_STEPS_PER_MM), rackEjectSteps(i));
    TEST_ASSERT_EQUAL_INT32(lroundf(rackLoadMm(i) * geom::RACK_STEPS_PER_MM), rackLoadSteps(i));
  }
  // with the current params: 1.5 mm pitch, 200 steps/mm -> 300 steps per shelf
  TEST_ASSERT_EQUAL_INT32(lroundf(geom::SHELF_PITCH * geom::RACK_STEPS_PER_MM),
                          rackEjectSteps(50) - rackEjectSteps(51));
  TEST_ASSERT_EQUAL_INT32(lroundf((geom::RACK_TRAVEL + cfg::RACK_ZERO_TRIM_MM) * geom::RACK_STEPS_PER_MM),
                          rackLoadSteps(0));
}

void test_norm_deg() {
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.0f, normDeg(0.0f));
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.0f, normDeg(360.0f));
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, 352.0f, normDeg(-8.0f));
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, 10.0f, normDeg(730.0f));
  TEST_ASSERT_TRUE(normDeg(-1e-7f) >= 0.0f && normDeg(-1e-7f) < 360.0f);
}

void test_seat_angles_even_and_in_range() {
  for (uint8_t n = cfg::MIN_PLAYERS; n <= cfg::MAX_PLAYERS; n++) {
    const float step = 360.0f / n;
    for (uint8_t s = 0; s < n; s++) {
      const float d = seatDeg(s, n);
      TEST_ASSERT_TRUE(d >= 0.0f && d < 360.0f);
      TEST_ASSERT_FLOAT_WITHIN(1e-3f, normDeg(geom::BOARD_DEG + (s + 0.5f) * step), d);
      if (s + 1 < n) TEST_ASSERT_FLOAT_WITHIN(1e-3f, step, normDeg(seatDeg(s + 1, n) - d));
    }
    // The board sits in the middle of the gap between the last and the first seat.
    const float gapMid = normDeg(seatDeg(n - 1, n) + step / 2.0f);
    TEST_ASSERT_FLOAT_WITHIN(1e-3f, normDeg(geom::BOARD_DEG), normDeg(gapMid + 1e-4f) - 1e-4f);
    // The burn spot sits after the board fan and before seat 0, for every table size.
    TEST_ASSERT_TRUE(burnDeg() > boardDeg(4));
    TEST_ASSERT_TRUE(burnDeg() < seatDeg(0, n));
    // The fanned board stays inside that gap even with 10 seats.
    for (uint8_t b = 0; b < 5; b++) {
      float off = normDeg(boardDeg(b) - geom::BOARD_DEG);
      if (off > 180.0f) off -= 360.0f;
      TEST_ASSERT_TRUE(fabsf(off) < step / 2.0f);
    }
  }
}

// With the current params the whole board fan stays on one side of 0/360, so
// dealing the board never needs a near-full-turn move.
void test_board_fan_does_not_wrap() {
  for (uint8_t b = 0; b + 1 < 5; b++) TEST_ASSERT_TRUE(boardDeg(b) < boardDeg(b + 1));
  TEST_ASSERT_TRUE(geom::BOARD_DEG - 2 * geom::BOARD_SPREAD_DEG >= 0);
  TEST_ASSERT_TRUE(geom::BOARD_DEG + 2 * geom::BOARD_SPREAD_DEG < geom::TURRET_MAX_DEG);
}

void test_board_and_burn_angles() {
  for (uint8_t b = 0; b < 5; b++) {
    const float d = boardDeg(b);
    TEST_ASSERT_TRUE(d >= 0.0f && d < 360.0f);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, normDeg(geom::BOARD_DEG + (b - 2.0f) * geom::BOARD_SPREAD_DEG), d);
  }
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, normDeg(geom::BOARD_DEG), boardDeg(2));
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, static_cast<float>(geom::BURN_DEG), burnDeg());
  TEST_ASSERT_TRUE(geom::BURN_DEG >= 0 && geom::BURN_DEG < geom::TURRET_MAX_DEG);  // no wrap
  // burn spot is clear of the board fan
  TEST_ASSERT_TRUE(burnDeg() > boardDeg(4));
}

void test_turret_steps_in_range() {
  const int32_t maxSteps = lroundf(geom::TURRET_MAX_DEG * geom::TURRET_STEPS_PER_DEG);
  for (float d = -720.0f; d <= 720.0f; d += 0.37f) {
    const int32_t s = turretDegToSteps(d);
    TEST_ASSERT_TRUE(s >= 0 && s <= maxSteps);
  }
  const float tr = cfg::TURRET_ZERO_TRIM_DEG;
  TEST_ASSERT_EQUAL_INT32(lroundf(normDeg(90.0f + tr) * geom::TURRET_STEPS_PER_DEG), turretDegToSteps(90.0f));
  TEST_ASSERT_EQUAL_INT32(lroundf(normDeg(tr) * geom::TURRET_STEPS_PER_DEG), turretDegToSteps(0.0f));
}

// At the probe position no shelf slot faces the load station: an incoming card band
// [0, CARD_T] above the load plane must not overlap any shelf gap. With the rack at r,
// shelf i's card plane sits at r - load(i) relative to the load plane.
void test_probe_position_faces_no_shelf() {
  const float r = rackProbeMm();
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, geom::PROBE_RACK_POS + cfg::RACK_ZERO_TRIM_MM, r);
  TEST_ASSERT_TRUE(r >= 0.0f && r <= kRackMaxMm + cfg::RACK_ZERO_TRIM_MM);
  float minClear = 1e9f;
  for (int i = 0; i < 52; i++) {
    const float gapLo = r - rackLoadMm(i), gapHi = gapLo + geom::SHELF_GAP;
    const bool below = gapHi <= 0.0f, above = gapLo >= geom::CARD_T;
    TEST_ASSERT_TRUE_MESSAGE(below || above, "a shelf slot faces the load station");
    const float c = below ? -gapHi : gapLo - geom::CARD_T;
    if (c < minClear) minClear = c;
  }
  // clearance to the nearest slot (shelf 51 just below): >= 0.3 mm (0.45 mm with 3.45)
  TEST_ASSERT_TRUE(minClear >= 0.3f);
  // and it is above shelf 51's load position: the card meets the top plate, no shelf
  TEST_ASSERT_TRUE(r < rackLoadMm(51));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_rack_eject_positions);
  RUN_TEST(test_rack_load_positions_and_travel);
  RUN_TEST(test_rack_steps);
  RUN_TEST(test_norm_deg);
  RUN_TEST(test_seat_angles_even_and_in_range);
  RUN_TEST(test_board_and_burn_angles);
  RUN_TEST(test_board_fan_does_not_wrap);
  RUN_TEST(test_turret_steps_in_range);
  RUN_TEST(test_probe_position_faces_no_shelf);
  return UNITY_END();
}
