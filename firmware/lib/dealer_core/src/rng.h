// Unbiased random integers from an injectable 32-bit source.
// On the ESP32 the source wraps esp_random(); in tests a deterministic PRNG.
#pragma once
#include <stdint.h>

namespace dealer {

class RandomSource {
 public:
  virtual ~RandomSource() = default;
  virtual uint32_t next32() = 0;
};

// Uniform integer in [0, n) by rejection sampling (no modulo bias).
// n == 0 returns 0.
uint32_t randBelow(RandomSource& src, uint32_t n);

// SplitMix64 (Steele/Lea/Flood): small, fast, passes BigCrush on its output.
// Used by the host tests and as a fallback source.
class SplitMix64 : public RandomSource {
 public:
  explicit SplitMix64(uint64_t seed) : state_(seed) {}
  uint64_t next64();
  uint32_t next32() override { return static_cast<uint32_t>(next64() >> 32); }

 private:
  uint64_t state_;
};

// xoshiro256** (Blackman/Vigna): 256-bit state, the firmware's shuffle generator.
// Seeded once at boot from >= 256 bits of true hardware entropy (expanded through
// SplitMix64 so any seed gives a well-mixed, non-zero state).
class Xoshiro256ss : public RandomSource {
 public:
  Xoshiro256ss() { seedWords(nullptr, 0); }
  // Raw state (for reference vectors). Must not be all zero.
  void setState(uint64_t s0, uint64_t s1, uint64_t s2, uint64_t s3);
  // Absorb `n` 32-bit seed words (n may exceed 8): SplitMix64 over all of them.
  void seedWords(const uint32_t* words, int n);
  uint64_t next64();
  uint32_t next32() override { return static_cast<uint32_t>(next64() >> 32); }

 private:
  uint64_t s_[4];
};

// XOR of two sources: the result is at least as unpredictable as the better one.
// The firmware mixes xoshiro256** with esp_random() on every draw.
class MixedSource : public RandomSource {
 public:
  MixedSource(RandomSource& a, RandomSource& b) : a_(a), b_(b) {}
  uint32_t next32() override { return a_.next32() ^ b_.next32(); }

 private:
  RandomSource& a_;
  RandomSource& b_;
};

}  // namespace dealer
