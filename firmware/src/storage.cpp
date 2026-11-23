#include "storage.h"

#include <Preferences.h>

#include "config.h"
#include "game.h"

namespace {
Preferences prefs;
uint8_t lastPlayers = 0, lastButton = 0xFF;
uint64_t lastMask = ~0ULL;
}  // namespace

namespace storage {

void begin() { prefs.begin("dealer", false); }

Settings load() {
  Settings s;
  s.players = dealer::Game::clampPlayers(prefs.getUChar("players", cfg::DEFAULT_PLAYERS));
  s.button = static_cast<uint8_t>(prefs.getUChar("button", 0) % s.players);
  s.rackMask = prefs.getULong64("rack", 0);
  lastPlayers = s.players;
  lastButton = s.button;
  lastMask = s.rackMask;
  return s;
}

// Writes only on change to spare the flash.
void saveSettings(uint8_t players, uint8_t button) {
  if (players != lastPlayers) prefs.putUChar("players", players);
  if (button != lastButton) prefs.putUChar("button", button);
  lastPlayers = players;
  lastButton = button;
}

void saveRack(uint64_t mask) {
  if (mask != lastMask) prefs.putULong64("rack", mask);
  lastMask = mask;
}

}  // namespace storage
