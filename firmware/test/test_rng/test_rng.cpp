// RNG: randBelow rejection sampling and uniform shelf selection (chi-square).
#include <stdio.h>
#include <unity.h>

#include "dealer_core.h"

using namespace dealer;

void setUp() {}
void tearDown() {}

// chi-square 0.999 / 0.001 quantiles (scipy.stats.chi2.ppf)
static const double CHI2_HI_DF51 = 87.968, CHI2_LO_DF51 = 25.368;
static const double CHI2_HI_DF6 = 22.458, CHI2_LO_DF6 = 0.381;
static const double CHI2_HI_DF9 = 27.877, CHI2_LO_DF9 = 1.152;

static double chi2(const uint32_t* counts, int k, uint32_t draws) {
  const double e = static_cast<double>(draws) / k;
  double x = 0;
  for (int i = 0; i < k; i++) {
    const double d = counts[i] - e;
    x += d * d / e;
  }
  return x;
}

// Scripted source for exercising the rejection path.
class Scripted : public RandomSource {
 public:
  Scripted(const uint32_t* v, int n) : v_(v), n_(n) {}
  uint32_t next32() override { calls++; return v_[i_++ % n_]; }
  int calls = 0;
 private:
  const uint32_t* v_;
  int n_, i_ = 0;
};

void test_randBelow_rejects_biased_tail() {
  // n = 10: UINT32_MAX % 10 = 5, so the 6 values >= 4294967290 must be rejected.
  const uint32_t seq[] = {4294967295u, 4294967290u, 4294967289u};
  Scripted s(seq, 3);
  TEST_ASSERT_EQUAL_UINT32(4294967289u % 10u, randBelow(s, 10));
  TEST_ASSERT_EQUAL_INT(3, s.calls);
  const uint32_t ok[] = {17u};
  Scripted s2(ok, 1);
  TEST_ASSERT_EQUAL_UINT32(7u, randBelow(s2, 10));
  TEST_ASSERT_EQUAL_INT(1, s2.calls);
}

void test_randBelow_edge_cases() {
  SplitMix64 r(1);
  TEST_ASSERT_EQUAL_UINT32(0, randBelow(r, 0));
  TEST_ASSERT_EQUAL_UINT32(0, randBelow(r, 1));
  for (int i = 0; i < 10000; i++) TEST_ASSERT_TRUE(randBelow(r, 52) < 52);
  for (int i = 0; i < 1000; i++) TEST_ASSERT_TRUE(randBelow(r, 0x80000001u) < 0x80000001u);
}

static void uniformityOf(uint32_t n, double lo, double hi, uint64_t seed) {
  SplitMix64 r(seed);
  uint32_t counts[16] = {0};
  const uint32_t draws = 210000;
  for (uint32_t i = 0; i < draws; i++) counts[randBelow(r, n)]++;
  const double x = chi2(counts, static_cast<int>(n), draws);
  char msg[64];
  snprintf(msg, sizeof msg, "n=%u chi2=%.2f", (unsigned)n, x);
  TEST_ASSERT_TRUE_MESSAGE(x < hi && x > lo, msg);
}

void test_randBelow_uniform_n7() { uniformityOf(7, CHI2_LO_DF6, CHI2_HI_DF6, 0xC0FFEEULL); }
void test_randBelow_uniform_n10() { uniformityOf(10, CHI2_LO_DF9, CHI2_HI_DF9, 0xBADC0DEULL); }

// Full rack, 520k draws (10k expected per shelf).
void test_shelf_selection_uniform_full_rack() {
  Rack rack;
  rack.setMask(Rack::kAllMask);
  SplitMix64 r(0x5EED5EEDULL);
  uint32_t counts[52] = {0};
  const uint32_t draws = 520000;
  for (uint32_t i = 0; i < draws; i++) {
    const int s = rack.pickRandom(r);
    TEST_ASSERT_TRUE(s >= 0 && s < 52);
    counts[s]++;
  }
  const double x = chi2(counts, 52, draws);
  char msg[64];
  snprintf(msg, sizeof msg, "chi2=%.2f (df 51, crit %.2f)", x, CHI2_HI_DF51);
  TEST_ASSERT_TRUE_MESSAGE(x < CHI2_HI_DF51, msg);
  TEST_ASSERT_TRUE_MESSAGE(x > CHI2_LO_DF51, msg);
}

// Partially filled rack (every 3rd shelf empty): only occupied shelves picked, uniformly.
void test_shelf_selection_uniform_partial_rack() {
  Rack rack;
  int occ[52], n = 0;
  for (int i = 0; i < 52; i++)
    if (i % 3) { rack.markLoaded(i); occ[n++] = i; }
  SplitMix64 r(42);
  uint32_t counts[52] = {0};
  const uint32_t draws = 340000;
  for (uint32_t i = 0; i < draws; i++) {
    const int s = rack.pickRandom(r);
    TEST_ASSERT_TRUE(rack.isOccupied(s));
    counts[s]++;
  }
  uint32_t c2[52];
  for (int i = 0; i < n; i++) c2[i] = counts[occ[i]];
  const double x = chi2(c2, n, draws);
  // df = 33; 0.999 quantile = 63.87
  TEST_ASSERT_TRUE(n == 34);
  TEST_ASSERT_TRUE(x < 63.87);
}

// Dealing a whole deck: which shelf comes out first and last is uniform,
// i.e. the deal is a uniform permutation at both ends (20000 decks).
void test_deal_whole_deck_first_and_last_uniform() {
  SplitMix64 r(7);
  uint32_t first[52] = {0}, last[52] = {0};
  const uint32_t decks = 20000;
  for (uint32_t d = 0; d < decks; d++) {
    Rack rack;
    rack.setMask(Rack::kAllMask);
    for (int k = 0; k < 52; k++) {
      const int s = rack.pickRandom(r);
      TEST_ASSERT_TRUE(rack.markDealt(s));
      if (k == 0) first[s]++;
      if (k == 51) last[s]++;
    }
    TEST_ASSERT_TRUE(rack.empty());
  }
  TEST_ASSERT_TRUE(chi2(first, 52, decks) < CHI2_HI_DF51);
  TEST_ASSERT_TRUE(chi2(last, 52, decks) < CHI2_HI_DF51);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_randBelow_rejects_biased_tail);
  RUN_TEST(test_randBelow_edge_cases);
  RUN_TEST(test_randBelow_uniform_n7);
  RUN_TEST(test_randBelow_uniform_n10);
  RUN_TEST(test_shelf_selection_uniform_full_rack);
  RUN_TEST(test_shelf_selection_uniform_partial_rack);
  RUN_TEST(test_deal_whole_deck_first_and_last_uniform);
  return UNITY_END();
}
