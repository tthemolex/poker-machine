// Rack bookkeeping invariants: never double-allocate, never deal from an empty shelf.
#include <unity.h>

#include "dealer_core.h"

using namespace dealer;

void setUp() {}
void tearDown() {}

void test_empty_rack() {
  Rack r;
  SplitMix64 g(1);
  TEST_ASSERT_TRUE(r.empty());
  TEST_ASSERT_EQUAL_INT(0, r.count());
  TEST_ASSERT_EQUAL_INT(0, r.nextEmpty());
  TEST_ASSERT_EQUAL_INT(-1, r.pickRandom(g));
  TEST_ASSERT_EQUAL_INT(-1, r.kthOccupied(0));
  for (int i = 0; i < 52; i++) TEST_ASSERT_FALSE(r.markDealt(i));
}

void test_load_fills_lowest_first_and_full() {
  Rack r;
  for (int i = 0; i < 52; i++) {
    TEST_ASSERT_EQUAL_INT(i, r.nextEmpty());
    TEST_ASSERT_TRUE(r.markLoaded(r.nextEmpty()));
  }
  TEST_ASSERT_TRUE(r.full());
  TEST_ASSERT_EQUAL_INT(52, r.count());
  TEST_ASSERT_EQUAL_INT(-1, r.nextEmpty());
  TEST_ASSERT_EQUAL_UINT64(Rack::kAllMask, r.mask());
}

void test_no_double_allocation_and_bounds() {
  Rack r;
  TEST_ASSERT_TRUE(r.markLoaded(17));
  TEST_ASSERT_FALSE(r.markLoaded(17));
  TEST_ASSERT_EQUAL_INT(1, r.count());
  TEST_ASSERT_FALSE(r.markLoaded(-1));
  TEST_ASSERT_FALSE(r.markLoaded(52));
  TEST_ASSERT_FALSE(r.markDealt(52));
  TEST_ASSERT_FALSE(r.isOccupied(64));
  TEST_ASSERT_TRUE(r.markDealt(17));
  TEST_ASSERT_FALSE(r.markDealt(17));
}

void test_mask_persistence_roundtrip() {
  Rack a;
  for (int i = 0; i < 52; i += 5) a.markLoaded(i);
  Rack b;
  b.setMask(a.mask());
  TEST_ASSERT_EQUAL_UINT64(a.mask(), b.mask());
  TEST_ASSERT_EQUAL_INT(a.count(), b.count());
  b.setMask(~0ULL);  // garbage in NVS: bits above shelf 51 are dropped
  TEST_ASSERT_EQUAL_INT(52, b.count());
  TEST_ASSERT_EQUAL_UINT64(Rack::kAllMask, b.mask());
}

void test_kth_occupied() {
  Rack r;
  r.markLoaded(3); r.markLoaded(10); r.markLoaded(51);
  TEST_ASSERT_EQUAL_INT(3, r.kthOccupied(0));
  TEST_ASSERT_EQUAL_INT(10, r.kthOccupied(1));
  TEST_ASSERT_EQUAL_INT(51, r.kthOccupied(2));
  TEST_ASSERT_EQUAL_INT(-1, r.kthOccupied(3));
  TEST_ASSERT_EQUAL_INT(-1, r.kthOccupied(-1));
}

// Randomized load / deal sequences against a shadow model.
void test_randomized_invariants() {
  SplitMix64 g(0xDEA1E5ULL);
  for (int trial = 0; trial < 2000; trial++) {
    Rack r;
    bool shadow[52] = {false};
    int shadowCount = 0;
    r.setMask(static_cast<uint64_t>(g.next64()));  // random initial state
    for (int i = 0; i < 52; i++) { shadow[i] = r.isOccupied(i); shadowCount += shadow[i]; }
    for (int step = 0; step < 200; step++) {
      const uint32_t op = randBelow(g, 3);
      if (op == 0) {  // load one card into the next empty shelf
        const int s = r.nextEmpty();
        if (shadowCount == 52) {
          TEST_ASSERT_EQUAL_INT(-1, s);
        } else {
          TEST_ASSERT_TRUE(s >= 0 && !shadow[s]);
          TEST_ASSERT_TRUE(r.markLoaded(s));
          shadow[s] = true; shadowCount++;
        }
      } else if (op == 1) {  // deal one random card
        const int s = r.pickRandom(g);
        if (shadowCount == 0) {
          TEST_ASSERT_EQUAL_INT(-1, s);
        } else {
          TEST_ASSERT_TRUE(s >= 0 && shadow[s]);  // never deal from an empty shelf
          TEST_ASSERT_TRUE(r.markDealt(s));
          shadow[s] = false; shadowCount--;
        }
      } else {  // hostile: try to load an arbitrary shelf / deal an arbitrary shelf
        const int s = static_cast<int>(randBelow(g, 52));
        if (randBelow(g, 2)) {
          TEST_ASSERT_EQUAL(!shadow[s], r.markLoaded(s));
          if (!shadow[s]) { shadow[s] = true; shadowCount++; }
        } else {
          TEST_ASSERT_EQUAL(shadow[s], r.markDealt(s));
          if (shadow[s]) { shadow[s] = false; shadowCount--; }
        }
      }
      TEST_ASSERT_EQUAL_INT(shadowCount, r.count());
      for (int i = 0; i < 52; i++) TEST_ASSERT_EQUAL(shadow[i], r.isOccupied(i));
    }
  }
}

// A whole game's worth of cards: load 52, deal a 10-player hand, never repeats a shelf.
void test_hand_never_repeats_shelf() {
  SplitMix64 g(99);
  for (int hand = 0; hand < 500; hand++) {
    Rack r;
    while (!r.full()) TEST_ASSERT_TRUE(r.markLoaded(r.nextEmpty()));
    bool used[52] = {false};
    for (int c = 0; c < 2 * 10 + 8; c++) {
      const int s = r.pickRandom(g);
      TEST_ASSERT_FALSE(used[s]);
      used[s] = true;
      TEST_ASSERT_TRUE(r.markDealt(s));
    }
    TEST_ASSERT_EQUAL_INT(52 - 28, r.count());
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_empty_rack);
  RUN_TEST(test_load_fills_lowest_first_and_full);
  RUN_TEST(test_no_double_allocation_and_bounds);
  RUN_TEST(test_mask_persistence_roundtrip);
  RUN_TEST(test_kth_occupied);
  RUN_TEST(test_randomized_invariants);
  RUN_TEST(test_hand_never_repeats_shelf);
  return UNITY_END();
}
