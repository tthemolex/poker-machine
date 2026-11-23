// Serial calibration / test console (115200 baud, line based, between hands only).
// Type "help" for the command list; see docs/FIRMWARE.md "Serial console".
#pragma once
#include "dealer_core.h"
#include "machine.h"

namespace console {
// Service / no-motion mode: every command that would move or power a motor is refused.
void setServiceMode(bool on);
// Read serial input; run a complete line. Returns true if the rack map changed.
bool poll(Machine& m, const dealer::Game& g);
}  // namespace console
