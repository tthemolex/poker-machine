#include "loader.h"

#include "config.h"

namespace dealer {

FeedVerdict feedVerdict(bool sawCard, uint32_t blockedMs, bool stillBlocked) {
  if (!sawCard) return FeedVerdict::HopperEmpty;
  if (stillBlocked || blockedMs > cfg::JAM_MS) return FeedVerdict::Jam;
  if (blockedMs > cfg::DOUBLE_MS) return FeedVerdict::Double;
  return FeedVerdict::Single;
}

}  // namespace dealer
