// Rack bookkeeping: which of the 52 shelves hold a card.
// The bitmap is also the NVS persistence format (bit i = shelf i occupied).
#pragma once
#include <stdint.h>

#include "geometry.h"
#include "rng.h"

namespace dealer {

class Rack {
 public:
  static constexpr int kShelves = geom::SHELF_COUNT;
  static_assert(kShelves > 0 && kShelves <= 64, "rack bitmap is a uint64_t");
  static constexpr uint64_t kAllMask =
      (kShelves == 64) ? ~0ULL : ((1ULL << kShelves) - 1ULL);

  void clear() { mask_ = 0; }
  // Restore from NVS. Bits above the last shelf are dropped.
  void setMask(uint64_t m) { mask_ = m & kAllMask; }
  uint64_t mask() const { return mask_; }

  bool isOccupied(int shelf) const;
  int count() const;
  bool full() const { return mask_ == kAllMask; }
  bool empty() const { return mask_ == 0; }

  // Lowest empty shelf to load the next card into, -1 if the rack is full.
  int nextEmpty() const;
  // Record a card fed into `shelf`. Refuses (false) an occupied or invalid shelf,
  // so a shelf can never be double-allocated.
  bool markLoaded(int shelf);
  // Record a card pushed out of `shelf`. Refuses (false) an empty or invalid
  // shelf, so the firmware never deals from an empty shelf.
  bool markDealt(int shelf);

  // Index of the k-th occupied shelf (k counted from 0, lowest shelf first),
  // -1 if k >= count().
  int kthOccupied(int k) const;
  // Uniformly random occupied shelf, -1 if the rack is empty. Does not modify.
  int pickRandom(RandomSource& src) const;

 private:
  uint64_t mask_ = 0;
};

}  // namespace dealer
