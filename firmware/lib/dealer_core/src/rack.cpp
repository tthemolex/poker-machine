#include "rack.h"

namespace dealer {

static inline bool validShelf(int s) { return s >= 0 && s < Rack::kShelves; }

bool Rack::isOccupied(int shelf) const {
  return validShelf(shelf) && ((mask_ >> shelf) & 1ULL);
}

int Rack::count() const {
  int c = 0;
  for (uint64_t m = mask_; m; m &= m - 1) c++;
  return c;
}

int Rack::nextEmpty() const {
  for (int i = 0; i < kShelves; i++)
    if (!isOccupied(i)) return i;
  return -1;
}

bool Rack::markLoaded(int shelf) {
  if (!validShelf(shelf) || isOccupied(shelf)) return false;
  mask_ |= (1ULL << shelf);
  return true;
}

bool Rack::markDealt(int shelf) {
  if (!isOccupied(shelf)) return false;
  mask_ &= ~(1ULL << shelf);
  return true;
}

int Rack::kthOccupied(int k) const {
  if (k < 0) return -1;
  for (int i = 0; i < kShelves; i++) {
    if (isOccupied(i)) {
      if (k == 0) return i;
      k--;
    }
  }
  return -1;
}

int Rack::pickRandom(RandomSource& src) const {
  const int n = count();
  if (n == 0) return -1;
  return kthOccupied(static_cast<int>(randBelow(src, static_cast<uint32_t>(n))));
}

}  // namespace dealer
