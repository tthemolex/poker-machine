// Loader beam timing verdict (the hardware measures, this decides).
#pragma once
#include <stdint.h>

namespace dealer {

enum class FeedVerdict : uint8_t {
  HopperEmpty,  // the beam was never blocked within NO_CARD_MS
  Jam,          // the beam is still blocked after JAM_MS
  Double,       // blocked longer than DOUBLE_MS: two cards went into the shelf
  Single,       // one card
};

// sawCard: beam got blocked before NO_CARD_MS. blockedMs: how long it stayed
// blocked (capped by the caller at JAM_MS when it gives up waiting).
// stillBlocked: the beam had not cleared when the caller gave up.
FeedVerdict feedVerdict(bool sawCard, uint32_t blockedMs, bool stillBlocked);

}  // namespace dealer
