// Bounded turret homing search against a simulated turret.
#include <unity.h>

#include "dealer_core.h"

using namespace dealer;

void setUp() {}
void tearDown() {}

// Simulated turret: true position in steps (home magnet zone [zoneLo, zoneHi]).
struct Sim {
  int32_t truePos;  // absolute, 0 = nominal home
  int32_t zoneLo, zoneHi;
  int32_t minSeen, maxSeen;
  bool sensor() const { return truePos >= zoneLo && truePos <= zoneHi; }
};

struct Result {
  bool ok;
  int32_t homeAbs;  // absolute position where Done was reported
  int32_t minAbs, maxAbs;
  int slowSteps;
};

static const HomingParams kP{0, 300, 200, 100};  // window 300, zone max 200, backoff 100

static Result run(int32_t start, HomingParams p, int32_t zoneLo = -20, int32_t zoneHi = 20) {
  Sim s{start, zoneLo, zoneHi, start, start};
  TurretHoming h(p);
  Result r{false, 0, start, start, 0};
  for (int guard = 0; guard < 1000000; guard++) {
    const int32_t rel = s.truePos - start;
    HomingCmd c = h.step(rel, s.sensor());
    if (c.kind == HomingCmd::Done) { r.ok = true; r.homeAbs = s.truePos; return r; }
    if (c.kind == HomingCmd::Fail) return r;
    if (c.target > rel) s.truePos++;
    else if (c.target < rel) s.truePos--;
    if (c.speed == HomingSpeed::Slow) r.slowSteps++;
    if (s.truePos < r.minAbs) r.minAbs = s.truePos;
    if (s.truePos > r.maxAbs) r.maxAbs = s.truePos;
  }
  TEST_FAIL_MESSAGE("homing did not terminate");
  return r;
}

// From anywhere within the window around home, zero lands on the same edge.
void test_repeatable_zero_from_both_sides() {
  for (int32_t start = -280; start <= 280; start += 7) {
    Result r = run(start, kP);
    TEST_ASSERT_TRUE_MESSAGE(r.ok, "not found inside window");
    TEST_ASSERT_EQUAL_INT32(20, r.homeAbs);  // positive edge of the zone, approached going -
    TEST_ASSERT_TRUE(r.slowSteps > 0);
  }
}

// Search never goes beyond the window (no blind 360 deg sweep).
void test_bounded_travel() {
  TurretHoming probe(kP);
  for (int32_t start = -1000; start <= 1000; start += 13) {
    Result r = run(start, kP);
    TEST_ASSERT_TRUE(r.minAbs - start >= probe.minReach());
    TEST_ASSERT_TRUE(r.maxAbs - start <= probe.maxReach());
  }
}

// Outside the window: fail instead of winding the cables.
void test_fails_outside_window() {
  TEST_ASSERT_FALSE(run(400, kP).ok);    // too far clockwise for est = 0
  TEST_ASSERT_FALSE(run(-400, kP).ok);   // nudged far counter-clockwise past the magnet
  Result r = run(-400, kP);
  TEST_ASSERT_TRUE(r.minAbs >= -400 - 300);
}

// A known estimate (e.g. counter after a fault) extends the negative search only.
void test_estimate_extends_negative_search() {
  HomingParams p = kP;
  p.estSteps = 32000;  // ~360 deg at 88.9 steps/deg
  Result r = run(31000, p);
  TEST_ASSERT_TRUE(r.ok);
  TEST_ASSERT_EQUAL_INT32(20, r.homeAbs);
  TEST_ASSERT_TRUE(r.minAbs >= 31000 - 32000 - 300);
  // estimate right but turret nudged CCW past the magnet: found on the + window
  p.estSteps = 0;
  Result r2 = run(-250, p);
  TEST_ASSERT_TRUE(r2.ok);
  TEST_ASSERT_EQUAL_INT32(20, r2.homeAbs);
  TEST_ASSERT_TRUE(r2.minAbs >= -250 - 300);
}

void test_starts_on_sensor_and_stuck_sensor() {
  Result r = run(0, kP);  // parked on the magnet
  TEST_ASSERT_TRUE(r.ok);
  TEST_ASSERT_EQUAL_INT32(20, r.homeAbs);
  // stuck sensor (zone wider than zoneMax): fail
  Result s = run(0, kP, -5000, 5000);
  TEST_ASSERT_FALSE(s.ok);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_repeatable_zero_from_both_sides);
  RUN_TEST(test_bounded_travel);
  RUN_TEST(test_fails_outside_window);
  RUN_TEST(test_estimate_extends_negative_search);
  RUN_TEST(test_starts_on_sensor_and_stuck_sensor);
  return UNITY_END();
}
