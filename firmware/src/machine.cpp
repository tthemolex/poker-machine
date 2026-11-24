#include "machine.h"

#include <AccelStepper.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include <bootloader_random.h>
#include <esp_random.h>

#include "config.h"
#include "storage.h"

using dealer::Fault;

namespace {

AccelStepper turret(AccelStepper::DRIVER, PIN_TURN_STEP, PIN_TURN_DIR);
AccelStepper rackAxis(AccelStepper::DRIVER, PIN_RACK_STEP, PIN_RACK_DIR);
Servo pusher;
ESP32PWM feedPwm;   // DRV8833 IN1 (forward)
ESP32PWM feedRev;   // DRV8833 IN2 (reverse, extra-card probe)
ESP32PWM launchPwmOut;

// Shuffle RNG: xoshiro256** seeded once at boot with RNG_SEED_WORDS x 32 bits of
// true entropy (seedShuffleRng), XOR esp_random() on every draw.
class EspRandom : public dealer::RandomSource {
 public:
  uint32_t next32() override { return esp_random(); }
};
EspRandom hwRng;
dealer::Xoshiro256ss xoshiro;
dealer::MixedSource shuffleRng(xoshiro, hwRng);
bool rngSeeded = false;

inline bool active(int pin) { return digitalRead(pin) == LOW; }  // all sensors active LOW

// Wait until `pin` reaches `wantActive` or the timeout expires. Returns true on success.
bool waitFor(int pin, bool wantActive, uint32_t timeoutMs) {
  const uint32_t t0 = millis();
  while (active(pin) != wantActive) {
    if (millis() - t0 > timeoutMs) return false;
  }
  return true;
}

// Seek the home sensor in the negative direction at constant speed. If the sensor
// is already active, back off in the positive direction first so the zero is
// always approached from the same side.
bool homeAxis(AccelStepper& s, int pin, float speed, int32_t backoffSteps, uint32_t timeoutMs) {
  if (active(pin)) {
    s.setCurrentPosition(0);
    s.moveTo(backoffSteps);
    const uint32_t t0 = millis();
    while (s.distanceToGo() != 0) {
      s.run();
      if (millis() - t0 > cfg::BACKOFF_TIMEOUT_MS) return false;
    }
    if (active(pin)) return false;  // sensor stuck active
  }
  s.setSpeed(-speed);
  const uint32_t t0 = millis();
  while (!active(pin)) {
    s.runSpeed();
    if (millis() - t0 > timeoutMs) {
      s.setSpeed(0);
      return false;
    }
  }
  s.setCurrentPosition(0);  // also zeroes the speed
  return true;
}

}  // namespace

void seedShuffleRng() {
  // IDF rule: bootloader_random_disable() must run BEFORE the ADC is initialised.
  // This is the first thing setup() does, before any analogRead / attenuation call.
  uint32_t seed[cfg::RNG_SEED_WORDS];
  bootloader_random_enable();  // SAR-ADC noise into the hardware RNG (radios are off)
  for (uint8_t i = 0; i < cfg::RNG_SEED_WORDS; i++) {
    delayMicroseconds(50);     // let fresh entropy accumulate between reads
    seed[i] = esp_random();
  }
  bootloader_random_disable();
  xoshiro.seedWords(seed, cfg::RNG_SEED_WORDS);
  for (uint8_t i = 0; i < cfg::RNG_SEED_WORDS; i++) seed[i] = 0;
  rngSeeded = true;
}

void Machine::begin(uint64_t rackMask) {
  rack_.setMask(rackMask);

  pinMode(PIN_STEP_EN, OUTPUT);
  digitalWrite(PIN_STEP_EN, HIGH);  // drivers off until needed
  for (int p : {PIN_IR_LOAD, PIN_IR_EXIT, PIN_HOME_TURN, PIN_HOME_RACK}) pinMode(p, INPUT_PULLUP);
  analogSetPinAttenuation(PIN_VBAT, ADC_11db);

  for (int t = 0; t < 4; t++) ESP32PWM::allocateTimer(t);
  feedPwm.attachPin(PIN_FEED_IN1, cfg::MOTOR_PWM_HZ, 8);
  feedRev.attachPin(PIN_FEED_IN2, cfg::MOTOR_PWM_HZ, 8);
  feedRev.write(0);
  launchPwmOut.attachPin(PIN_LAUNCH, cfg::MOTOR_PWM_HZ, 8);
  feedPwm.write(0);
  launchPwmOut.write(0);
  pusher.setPeriodHertz(50);

  turret.setPinsInverted(cfg::TURN_DIR_INVERT, false, false);
  rackAxis.setPinsInverted(cfg::RACK_DIR_INVERT, false, false);
  turret.setMaxSpeed(cfg::TURN_MAX_SPEED);
  turret.setAcceleration(cfg::TURN_ACCEL);
  rackAxis.setMaxSpeed(cfg::RACK_MAX_SPEED);
  rackAxis.setAcceleration(cfg::RACK_ACCEL);
}

// ---------------- Power ----------------

Fault Machine::powerUp() {
  digitalWrite(PIN_STEP_EN, LOW);
  if (!pusher.attached()) pusher.attach(PIN_PUSHER);
  pusher.write(cfg::PUSH_HOME);
  delay(cfg::SERVO_SETTLE_MS);
  powered_ = true;
  // The rack leadscrew is self-locking, so it only re-homes at boot and after a fault.
  if (!rackHomed_ && !homeRack()) return fail(Fault::RackHoming);
  // The turret may have been bumped while unpowered: always re-home, with a search
  // bounded to a window around where it should be (parked at 0, or the last
  // counter value after a fault). See dealer::TurretHoming.
  if (!homeTurret()) return fail(Fault::TurretHoming);
  return Fault::None;
}

void Machine::powerDown() {
  stopMotors();
  if (powered_ && turretHomed_) {
    turret.moveTo(lroundf(cfg::TURRET_PARK_DEG * geom::TURRET_STEPS_PER_DEG));  // park at home: short re-home
    runBoth();
    turretParked_ = true;
  }
  delay(cfg::PUSH_RETRACT_MS);
  pusher.detach();
  digitalWrite(PIN_STEP_EN, HIGH);
  powered_ = false;
  storage::saveRack(rack_.mask());
}

void Machine::stopMotors() {
  feedOff();
  launchSet(0);
  if (pusher.attached()) pusher.write(cfg::PUSH_HOME);
}

void Machine::safeStop() {
  stopMotors();
  delay(cfg::PUSH_RETRACT_MS);
  pusher.detach();
  digitalWrite(PIN_STEP_EN, HIGH);
  powered_ = false;
  turretHomed_ = turretParked_ = rackHomed_ = false;
  storage::saveRack(rack_.mask());
}

Fault Machine::fail(Fault f) {
  Serial.printf("FAULT: %s\n", dealer::faultText(f));
  safeStop();
  return f;
}

bool Machine::homeTurret() {
  const float spd = geom::TURRET_STEPS_PER_DEG;
  const int32_t full = lroundf(geom::TURRET_MAX_DEG * spd);
  // Believed position above home: 0 when parked (or at boot, the turret parks at 0
  // before every power-down); the step counter after a fault.
  int32_t est = 0;
  if (!turretParked_ && turretCounterValid_) {
    est = turret.currentPosition() - lroundf(cfg::TURRET_ZERO_TRIM_DEG * spd);
    if (est < 0) est = 0;
    if (est > full) est = full;
  }
  dealer::TurretHoming search({est, lroundf(cfg::TURN_SEARCH_WINDOW_DEG * spd),
                               lroundf(cfg::TURN_ZONE_MAX_DEG * spd),
                               lroundf(cfg::TURN_BACKOFF_DEG * spd), full});
  turret.setCurrentPosition(0);  // search positions are relative to here
  turretHomed_ = turretParked_ = false;
  const uint32_t t0 = millis();
  for (;;) {
    const int32_t pos = turret.currentPosition();
    const dealer::HomingCmd c = search.step(pos, active(PIN_HOME_TURN));
    if (c.kind == dealer::HomingCmd::Done) break;
    if (c.kind == dealer::HomingCmd::Fail || millis() - t0 > cfg::TURN_HOME_TIMEOUT_MS) {
      turret.setSpeed(0);
      turretCounterValid_ = false;
      Serial.println("Turret: hall not found near the expected angle. Turn the turret by hand "
                     "to the home mark, then press GO.");
      return false;
    }
    if (c.target != pos) {
      const float v = (c.speed == dealer::HomingSpeed::Slow) ? cfg::TURN_HOMING_SLOW
                                                             : cfg::TURN_HOMING_SPEED;
      turret.setSpeed(c.target > pos ? v : -v);
      turret.runSpeed();
    }
  }
  turret.setCurrentPosition(0);  // home = 0 deg (trim is applied to targets)
  turretHomed_ = turretParked_ = turretCounterValid_ = true;
  return true;
}

bool Machine::homeRack() {
  const int32_t backoff = dealer::rackMmToSteps(cfg::RACK_BACKOFF_MM);
  rackHomed_ = homeAxis(rackAxis, PIN_HOME_RACK, cfg::RACK_HOMING_SPEED, backoff,
                        cfg::RACK_HOME_TIMEOUT_MS);
  return rackHomed_;
}

// ---------------- Motion ----------------

void Machine::aimAt(float deg) {
  turret.moveTo(dealer::turretDegToSteps(deg));  // always inside 0..360 deg
  turretParked_ = false;
}

void Machine::runBoth() {
  while (turret.distanceToGo() != 0 || rackAxis.distanceToGo() != 0) {
    turret.run();
    rackAxis.run();
  }
}

void Machine::feedOn() {
  feedRev.write(0);
  feedPwm.write(cfg::FEED_PWM);
}
void Machine::feedOff() {
  feedPwm.write(0);
  feedRev.write(0);
}

void Machine::launchSet(uint8_t pwm) {
  if (pwm == launchNow_) return;
  if (pwm > launchNow_) {
    // Soft ramp up: a 130 motor from standstill at full PWM can trip the DRV8833
    // over-current protection and dip the 5 V rail.
    constexpr int kSteps = 12;
    for (int i = 1; i <= kSteps; i++) {
      launchPwmOut.write(launchNow_ + (pwm - launchNow_) * i / kSteps);
      delay(cfg::LAUNCH_RAMP_MS / kSteps);
    }
  }
  launchPwmOut.write(pwm);
  if (pwm > 0) delay(cfg::SPINUP_MS);  // settle to the new speed
  launchNow_ = pwm;
}

uint8_t Machine::launchPwm(dealer::Launch l) {
  switch (l) {
    case dealer::Launch::Seat: return cfg::LAUNCH_SEAT;
    case dealer::Launch::Board: return cfg::LAUNCH_BOARD;
    case dealer::Launch::Burn: return cfg::LAUNCH_BURN;
  }
  return cfg::LAUNCH_BURN;
}

// ---------------- Loader ----------------

Fault Machine::loadOne(int shelf, uint32_t* blockedMs, bool* hopperEmpty) {
  *hopperEmpty = false;
  *blockedMs = 0;
  rackAxis.moveTo(dealer::rackLoadSteps(shelf));
  runBoth();

  feedOn();
  const bool saw = waitFor(PIN_IR_LOAD, true, cfg::NO_CARD_MS);
  uint32_t blocked = 0;
  bool still = false;
  if (saw) {
    const uint32_t tb = millis();
    still = !waitFor(PIN_IR_LOAD, false, cfg::JAM_MS);
    blocked = millis() - tb;
  }
  *blockedMs = blocked;
  const dealer::FeedVerdict verdict = dealer::feedVerdict(saw, blocked, still);
  switch (verdict) {
    case dealer::FeedVerdict::HopperEmpty:
      feedOff();
      *hopperEmpty = true;
      return Fault::None;
    case dealer::FeedVerdict::Jam:
      feedOff();  // card not counted: clear it and put it back in the hopper
      return fail(Fault::LoaderJam);
    case dealer::FeedVerdict::Double:
    case dealer::FeedVerdict::Single:
      break;
  }
  delay(cfg::SEAT_MS);  // let the roller seat the card against the stop fingers
  feedOff();
  rack_.markLoaded(shelf);
  lastLoaded_ = shelf;
  if (verdict == dealer::FeedVerdict::Double) {
    Serial.printf("Shelf %d probably holds 2 cards (beam blocked %lu ms)\n", shelf,
                  static_cast<unsigned long>(blocked));
    return fail(Fault::DoubleFeed);
  }
  return Fault::None;
}

Fault Machine::loadCards() {
  for (int shelf = rack_.nextEmpty(); shelf >= 0; shelf = rack_.nextEmpty()) {
    uint32_t blocked;
    bool empty;
    const Fault f = loadOne(shelf, &blocked, &empty);
    if (f != Fault::None) return f;
    if (empty) {
      storage::saveRack(rack_.mask());
      return Fault::None;  // hopper empty before the rack was full
    }
  }
  storage::saveRack(rack_.mask());
  return cfg::EXTRA_CARD_PROBE ? probeExtraCard() : Fault::None;  // rack full
}

// Rack full: is there still a card in the hopper? The rack goes to a position
// where no shelf faces the load station, the roller runs until the loader beam
// sees a card's leading edge (or NO_CARD_MS), then reverses to pull it back.
// The card is never driven into a full shelf.
Fault Machine::probeExtraCard() {
  rackAxis.moveTo(dealer::rackMmToSteps(dealer::rackProbeMm()));
  runBoth();
  feedOn();
  const bool saw = waitFor(PIN_IR_LOAD, true, cfg::NO_CARD_MS);
  feedPwm.write(0);
  if (!saw) {
    feedOff();
    return Fault::None;
  }
  feedRev.write(cfg::FEED_PWM);
  delay(cfg::PROBE_REVERSE_MS);
  feedOff();
  Serial.println("Rack is full but the hopper still has a card: remove the extra cards.");
  return fail(Fault::ExtraCards);
}

// ---------------- Deal ----------------

Fault Machine::ejectShelf(int shelf, float deg, uint8_t pwm, bool emptyIsFault, bool kick) {
  if (!rack_.isOccupied(shelf)) return Fault::RackEmpty;
  aimAt(deg);
  rackAxis.moveTo(dealer::rackEjectSteps(shelf));
  runBoth();
  launchSet(pwm);

  pusher.write(cfg::PUSH_OUT);
  if (!waitFor(PIN_IR_EXIT, true, cfg::EXIT_WAIT_MS)) {
    pusher.write(cfg::PUSH_HOME);
    rack_.markDealt(shelf);  // the map was wrong: the shelf is empty
    Serial.printf("Shelf %d was empty\n", shelf);
    if (emptyIsFault) return fail(Fault::NoCardInShelf);
    delay(cfg::PUSH_RETRACT_MS);
    return Fault::None;
  }
  pusher.write(cfg::PUSH_HOME);  // the launcher has the card now
  rack_.markDealt(shelf);        // it has left the shelf, whatever happens next
  if (!waitFor(PIN_IR_EXIT, false, cfg::EXIT_JAM_MS)) return fail(Fault::LauncherJam);
  if (kick && cfg::LAUNCH_BURN_KICK > pwm) {
    // Burn: the tail has passed the entry beam but is still in the nip. Speed the
    // wheel up for the last part so the card is flung clear of the exit lip
    // instead of tipping back at release.
    launchPwmOut.write(cfg::LAUNCH_BURN_KICK);
    delay(cfg::BURN_KICK_MS);
    launchPwmOut.write(pwm);
  }
  delay(cfg::PUSH_RETRACT_MS);   // the blade must clear the rack before it moves
  return Fault::None;
}

Fault Machine::executePlan(const dealer::Plan& plan) {
  if (!rngSeeded) seedShuffleRng();  // never happens: setup() seeds first
  for (uint8_t i = 0; i < plan.count; i++) {
    const dealer::DealAction& a = plan.actions[i];
    const int shelf = rack_.pickRandom(shuffleRng);  // the shuffle
    if (shelf < 0) return fail(Fault::RackEmpty);
    const Fault f = ejectShelf(shelf, a.deg, launchPwm(a.launch), true,
                               a.launch == dealer::Launch::Burn);
    if (f != Fault::None) return f;
  }
  launchSet(0);
  storage::saveRack(rack_.mask());
  return Fault::None;
}

Fault Machine::purgeShelf(int shelf) {
  const Fault f = ejectShelf(shelf, dealer::burnDeg(), cfg::LAUNCH_PURGE, false, false);
  launchSet(0);
  return f == Fault::RackEmpty ? Fault::None : f;
}

Fault Machine::purgeAll() {
  for (int s = 0; s < dealer::Rack::kShelves; s++) {
    if (!rack_.isOccupied(s)) continue;
    const Fault f = ejectShelf(s, dealer::burnDeg(), cfg::LAUNCH_PURGE, false, false);
    if (f != Fault::None) return f;
  }
  launchSet(0);
  storage::saveRack(rack_.mask());
  return Fault::None;
}

// ---------------- Calibration hooks (serial console) ----------------

void Machine::calRackMm(float mm) {
  if (mm < 0.0f) mm = 0.0f;
  if (mm > dealer::kRackMaxMm + cfg::RACK_ZERO_TRIM_MM) mm = dealer::kRackMaxMm + cfg::RACK_ZERO_TRIM_MM;
  rackAxis.moveTo(dealer::rackMmToSteps(mm));
  runBoth();
}
void Machine::calRackSteps(int32_t steps) {
  const int32_t top = dealer::rackMmToSteps(dealer::kRackMaxMm + cfg::RACK_ZERO_TRIM_MM);
  if (steps < 0) steps = 0;
  if (steps > top) steps = top;
  rackAxis.moveTo(steps);
  runBoth();
}
void Machine::calAim(float deg) {
  // Console input is clamped (not wrapped) into the allowed 0..360 deg window.
  if (!(deg >= 0.0f)) deg = 0.0f;
  if (deg > geom::TURRET_MAX_DEG - 0.01f) deg = geom::TURRET_MAX_DEG - 0.01f;
  aimAt(deg);
  runBoth();
}
void Machine::calLaunch(uint8_t pwm) { launchSet(pwm); }
void Machine::calFeed(uint8_t pwm) {
  feedRev.write(0);
  feedPwm.write(pwm);
}
void Machine::calServo(int angle) {
  if (!pusher.attached()) pusher.attach(PIN_PUSHER);
  pusher.write(angle);
}
void Machine::clearMap() {
  rack_.clear();
  storage::saveRack(rack_.mask());
}
int32_t Machine::rackSteps() const { return rackAxis.currentPosition(); }
int32_t Machine::turretSteps() const { return turret.currentPosition(); }
bool Machine::sensor(int pin) { return active(pin); }

// ---------------- Battery ----------------

float Machine::readVbat() {
  uint32_t mv = 0;
  for (uint8_t i = 0; i < cfg::VBAT_SAMPLES; i++) mv += analogReadMilliVolts(PIN_VBAT);
  return dealer::vbatFromPinMv(static_cast<float>(mv) / cfg::VBAT_SAMPLES);
}

dealer::BatteryLevel Machine::battery(float* volts) {
  const float v = readVbat();
  if (volts) *volts = v;
  return vmon_.update(v);
}
