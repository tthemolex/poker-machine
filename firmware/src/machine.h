// Hardware layer: steppers, loader, pusher, launcher, sensors, power.
// Executes the abstract deal plans produced by dealer::Game.
#pragma once
#include <stdint.h>

#include "dealer_core.h"

// Seed the shuffle RNG from true hardware entropy. Call FIRST in setup(), before
// any ADC use (bootloader_random_enable/disable must precede ADC initialisation).
void seedShuffleRng();

class Machine {
 public:
  void begin(uint64_t rackMask);

  // Steppers enabled, servo attached, axes (re-)homed. Fault::None on success.
  dealer::Fault powerUp();
  // Idle: motors off, turret parked at 0 deg (if homed), servo detached, EN high.
  void powerDown();
  // After a fault: stop every motor immediately and release the steppers and the
  // servo so jams can be cleared by hand. Both axes re-home on the next powerUp().
  void safeStop();

  // Feed cards from the hopper into empty shelves until it is empty or the rack is full.
  dealer::Fault loadCards();
  // Execute a plan: one random occupied shelf per action.
  dealer::Fault executePlan(const dealer::Plan& plan);
  // Throw one specific shelf onto the burn spot (double-feed recovery), with the
  // LAUNCH_PURGE profile (two stacked cards need more than LAUNCH_BURN).
  dealer::Fault purgeShelf(int shelf);
  // Purge every shelf the map marks occupied. Shelves found empty are just cleared.
  dealer::Fault purgeAll();

  // Feed one card into `shelf` (must be empty). Reports the beam-blocked time
  // (DOUBLE_MS calibration) and whether the hopper was empty.
  dealer::Fault loadOne(int shelf, uint32_t* blockedMs, bool* hopperEmpty);

  // Calibration hooks for the serial console. Call powerUp() first.
  void calRackMm(float mm);        // absolute, from the homed bottom (clamped to travel)
  void calRackSteps(int32_t steps);
  void calAim(float deg);          // table angle, clamped into 0..360
  void calLaunch(uint8_t pwm);
  void calFeed(uint8_t pwm);
  void calServo(int angle);
  void clearMap();                 // forget the rack contents (rack emptied by hand)
  bool powered() const { return powered_; }
  int32_t rackSteps() const;
  int32_t turretSteps() const;
  static bool sensor(int pin);     // true = active (LOW)

  float readVbat();  // raw volts (no plausibility check)
  // Read the battery and run it through the plausibility guard (VbatMonitor).
  dealer::BatteryLevel battery(float* volts);
  const dealer::Rack& rack() const { return rack_; }
  int lastLoadedShelf() const { return lastLoaded_; }
  void stopMotors();

 private:
  dealer::Fault fail(dealer::Fault f);
  dealer::Fault ejectShelf(int shelf, float deg, uint8_t pwm, bool emptyIsFault, bool kick);
  bool homeTurret();
  dealer::Fault probeExtraCard();
  bool homeRack();
  void aimAt(float deg);
  void runBoth();
  void feedOn();
  void feedOff();
  void launchSet(uint8_t pwm);
  static uint8_t launchPwm(dealer::Launch l);

  dealer::Rack rack_;
  dealer::VbatMonitor vmon_;
  bool powered_ = false;
  bool turretHomed_ = false;  // position counter trusted
  bool turretParked_ = true;  // at 0 deg (powerDown parks there; assumed at boot)
  bool turretCounterValid_ = false;  // step counter usable as a homing estimate
  bool rackHomed_ = false;
  uint8_t launchNow_ = 0;
  int lastLoaded_ = -1;
};
