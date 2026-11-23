// NVS persistence (Preferences): player count, button seat, rack occupancy mask.
#pragma once
#include <stdint.h>

struct Settings {
  uint8_t players;
  uint8_t button;
  uint64_t rackMask;
};

namespace storage {
void begin();
Settings load();
void saveSettings(uint8_t players, uint8_t button);
void saveRack(uint64_t mask);
}  // namespace storage
