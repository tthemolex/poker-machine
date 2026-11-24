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

static inline uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

void Xoshiro256ss::setState(uint64_t s0, uint64_t s1, uint64_t s2, uint64_t s3) {
  s_[0] = s0; s_[1] = s1; s_[2] = s2; s_[3] = s3;
  if ((s0 | s1 | s2 | s3) == 0) s_[0] = 1;  // the all-zero state is a fixed point
}

void Xoshiro256ss::seedWords(const uint32_t* words, int n) {
  // Fold every word into a SplitMix64 stream, then draw the 4 state words from it.
  SplitMix64 mix(0x6A09E667F3BCC908ULL);
  uint64_t acc = 0;
  for (int i = 0; i < n; i++) {
    acc = mix.next64() ^ (acc * 0x9E3779B97F4A7C15ULL) ^ words[i];
    mix = SplitMix64(acc);
  }
  setState(mix.next64(), mix.next64(), mix.next64(), mix.next64());
}

uint64_t Xoshiro256ss::next64() {
  const uint64_t result = rotl(s_[1] * 5, 7) * 9;
  const uint64_t t = s_[1] << 17;
  s_[2] ^= s_[0];
  s_[3] ^= s_[1];
  s_[1] ^= s_[2];
  s_[0] ^= s_[3];
  s_[2] ^= t;
  s_[3] = rotl(s_[3], 45);
  return result;
}

}  // namespace dealer
