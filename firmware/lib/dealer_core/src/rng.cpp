#include "rng.h"

namespace dealer {

uint32_t randBelow(RandomSource& src, uint32_t n) {
  if (n <= 1) return 0;
  // Largest multiple of n that fits: accept r < limit, reject the tail.
  const uint32_t limit = UINT32_MAX - (UINT32_MAX % n);
  uint32_t r;
  do {
    r = src.next32();
  } while (r >= limit);
  return r % n;
}

uint64_t SplitMix64::next64() {
  uint64_t z = (state_ += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}

}  // namespace dealer
