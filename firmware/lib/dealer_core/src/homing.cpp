#include "homing.h"

namespace dealer {

int32_t TurretHoming::maxReach() const {
  // worst case: found at the top of the positive window, then clear + backoff
  return p_.windowSteps + p_.zoneMaxSteps + p_.backoffSteps;
}

HomingCmd TurretHoming::enterClear(int32_t pos) {
  phase_ = Phase::Clear;
  target_ = pos + p_.zoneMaxSteps;
  return {HomingCmd::Move, target_, HomingSpeed::Fast};
}

HomingCmd TurretHoming::step(int32_t pos, bool sensor) {
  switch (phase_) {
    case Phase::Start:
      if (sensor) return enterClear(pos);
      phase_ = Phase::SeekNeg;
      target_ = minReach();
      return {HomingCmd::Move, target_, HomingSpeed::Fast};

    case Phase::SeekNeg:
      if (sensor) return enterClear(pos);
      if (pos <= target_) {
        phase_ = Phase::SeekPos;
        target_ = p_.windowSteps;
      }
      return {HomingCmd::Move, target_, HomingSpeed::Fast};

    case Phase::SeekPos:
      if (sensor) return enterClear(pos);
      if (pos >= target_) {
        phase_ = Phase::Fail;
        return {HomingCmd::Fail, pos, HomingSpeed::Fast};
      }
      return {HomingCmd::Move, target_, HomingSpeed::Fast};

    case Phase::Clear:
      if (!sensor) {
        phase_ = Phase::Backoff;
        target_ = pos + p_.backoffSteps;
      } else if (pos >= target_) {
        phase_ = Phase::Fail;  // sensor stuck active
        return {HomingCmd::Fail, pos, HomingSpeed::Fast};
      }
      return {HomingCmd::Move, target_, HomingSpeed::Fast};

    case Phase::Backoff:
      if (pos >= target_) {
        phase_ = Phase::Approach;
        target_ = pos - (p_.backoffSteps + p_.zoneMaxSteps);
        return {HomingCmd::Move, target_, HomingSpeed::Slow};
      }
      return {HomingCmd::Move, target_, HomingSpeed::Fast};

    case Phase::Approach:
      if (sensor) {
        phase_ = Phase::Done;
        return {HomingCmd::Done, pos, HomingSpeed::Slow};
      }
      if (pos <= target_) {
        phase_ = Phase::Fail;
        return {HomingCmd::Fail, pos, HomingSpeed::Slow};
      }
      return {HomingCmd::Move, target_, HomingSpeed::Slow};

    case Phase::Done:
      return {HomingCmd::Done, pos, HomingSpeed::Slow};
    case Phase::Fail:
      return {HomingCmd::Fail, pos, HomingSpeed::Fast};
  }
  return {HomingCmd::Fail, pos, HomingSpeed::Fast};
}

}  // namespace dealer
