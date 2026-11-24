// Rack position and turret aiming math. All geometry comes from geometry.h
// (generated from cad/params.scad); nothing here is re-typed.
#pragma once
#include <stdint.h>

#include "geometry.h"

namespace dealer {

// ---------------- Rack (mm / steps above the homed bottom position) ----------------
// Homed (0) = shelf 51 at the eject station. Shelf 0 is the bottom shelf.
float rackEjectMm(int shelf);  // (51 - shelf) * pitch + cfg::RACK_ZERO_TRIM_MM
float rackLoadMm(int shelf);   // eject + STATION_OFFSET (load station is above eject)
int32_t rackMmToSteps(float mm);
int32_t rackEjectSteps(int shelf);
int32_t rackLoadSteps(int shelf);
constexpr float kRackMaxMm = geom::RACK_TRAVEL;
// Rack position for the 53rd-card probe (geom::PROBE_RACK_POS + trim): no shelf
// slot faces the load station; a probed card stalls against the solid back face
// of the rack top plate, past the pinch line, and can be reversed out.
float rackProbeMm();

// ---------------- Turret (degrees, clockwise from home = board) ----------------
// Wrap into [0, 360). The turret never crosses 360 (cable service loop), so every
// target angle is expressed in this window.
float normDeg(float deg);
// Seats evenly around 360 deg, numbered clockwise; the board sits in the gap
// between the last and the first seat: seat s at BOARD + (s + 0.5) * 360 / n.
float seatDeg(uint8_t seat, uint8_t players);
// Board card `slot` 0..4 fanned around the board: BOARD + (slot - 2) * spread.
float boardDeg(uint8_t slot);
// Burns are thrown at very low speed onto the burn spot on the table at
// geom::BURN_DEG, beside the board fan and before seat 0.
float burnDeg();
// Steps from the hall home for a table angle, incl. cfg::TURRET_ZERO_TRIM_DEG,
// always inside [0, 360) deg worth of steps.
int32_t turretDegToSteps(float deg);

}  // namespace dealer
