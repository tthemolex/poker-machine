#include "motion.h"

#include <math.h>

#include "config.h"

namespace dealer {

float rackEjectMm(int shelf) {
  return static_cast<float>(geom::SHELF_COUNT - 1 - shelf) * geom::SHELF_PITCH +
         cfg::RACK_ZERO_TRIM_MM;
}

float rackLoadMm(int shelf) { return rackEjectMm(shelf) + geom::STATION_OFFSET; }

int32_t rackMmToSteps(float mm) {
  return static_cast<int32_t>(lroundf(mm * geom::RACK_STEPS_PER_MM));
}

int32_t rackEjectSteps(int shelf) { return rackMmToSteps(rackEjectMm(shelf)); }
int32_t rackLoadSteps(int shelf) { return rackMmToSteps(rackLoadMm(shelf)); }

float normDeg(float deg) {
  const float full = static_cast<float>(geom::TURRET_MAX_DEG);
  float d = fmodf(deg, full);
  if (d < 0.0f) d += full;
  if (d >= full) d -= full;  // fmodf rounding at exactly -0 / 360
  return d;
}

float seatDeg(uint8_t seat, uint8_t players) {
  if (players == 0) return normDeg(static_cast<float>(geom::BOARD_DEG));
  const float step = 360.0f / static_cast<float>(players);
  return normDeg(static_cast<float>(geom::BOARD_DEG) +
                 (static_cast<float>(seat % players) + 0.5f) * step);
}

float boardDeg(uint8_t slot) {
  return normDeg(static_cast<float>(geom::BOARD_DEG) +
                 (static_cast<float>(slot) - 2.0f) * static_cast<float>(geom::BOARD_SPREAD_DEG));
}

float burnDeg() { return normDeg(static_cast<float>(geom::BOARD_DEG)); }

int32_t turretDegToSteps(float deg) {
  return static_cast<int32_t>(
      lroundf(normDeg(deg + cfg::TURRET_ZERO_TRIM_DEG) * geom::TURRET_STEPS_PER_DEG));
}

}  // namespace dealer
