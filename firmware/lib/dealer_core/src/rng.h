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

}  // namespace dealer
