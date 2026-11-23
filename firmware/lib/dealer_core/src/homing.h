// Bounded turret homing search (pure logic, unit-tested).
//
// The turret has one hall sensor at 0 deg and no absolute position. Its cables
// allow 0..360 deg only, so a blind 360 deg seek in one direction is unsafe: a
// turret nudged counter-clockwise past the magnet would wind a full turn to reach
// it from the other side. Instead the search is confined to a window around the
// position where the firmware believes the turret is:
//
//   1. sensor already active             -> go to CLEAR
//   2. SEEK_NEG down to  -(est + window)  (est = believed angle above home, >= 0)
//   3. SEEK_POS up to    +window          (covers a turret nudged the other way)
//   4. not found                          -> FAIL (never sweeps further)
//   CLEAR:    move + until the sensor releases (at most zoneMax), else FAIL (stuck)
//   BACKOFF:  move + another `backoff`
//   APPROACH: move - slowly at most backoff + zoneMax; the first sensor edge is 0
//
// So 0 is always the positive edge of the sensor zone approached slowly in the
// negative direction, whichever way the magnet was found. All positions are in
// steps relative to where the search started.
#pragma once
#include <stdint.h>

namespace dealer {

struct HomingParams {
  int32_t estSteps;     // believed position above home (0 = parked at home)
  int32_t windowSteps;  // search margin around the believed position
  int32_t zoneMaxSteps; // longest the sensor can stay active (zone width + margin)
  int32_t backoffSteps; // extra positive travel before the slow approach
};

enum class HomingSpeed : uint8_t { Fast, Slow };

struct HomingCmd {
  enum Kind : uint8_t { Move, Done, Fail } kind;
  int32_t target;     // Move: target position (relative steps)
  HomingSpeed speed;  // Move: speed class
};

class TurretHoming {
 public:
  explicit TurretHoming(const HomingParams& p) : p_(p) {}

  // Call repeatedly with the current relative position and sensor state; drive the
  // motor one step toward Move.target at Move.speed. Done: the current position is
  // home (0 deg). Fail: give up (fault).
  HomingCmd step(int32_t pos, bool sensor);

  // Lowest / highest relative position the search can ever command.
  int32_t minReach() const { return -(p_.estSteps + p_.windowSteps); }
  int32_t maxReach() const;

 private:
  enum class Phase : uint8_t { Start, SeekNeg, SeekPos, Clear, Backoff, Approach, Done, Fail };
  HomingCmd enterClear(int32_t pos);
  HomingParams p_;
  Phase phase_ = Phase::Start;
  int32_t target_ = 0;
};

}  // namespace dealer
