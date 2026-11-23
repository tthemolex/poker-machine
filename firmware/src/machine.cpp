#include "machine.h"

#include <AccelStepper.h>
#include <Arduino.h>
#include <ESP32Servo.h>
#include <esp_random.h>

#include "config.h"
#include "storage.h"

using dealer::Fault;

namespace {

AccelStepper turret(AccelStepper::DRIVER, PIN_TURN_STEP, PIN_TURN_DIR);
AccelStepper rackAxis(AccelStepper::DRIVER, PIN_RACK_STEP, PIN_RACK_DIR);
Servo pusher;
ESP32PWM feedPwm;
ESP32PWM launchPwmOut;

// Hardware RNG. True random while the RF subsystem runs; otherwise seeded from
// the SAR ADC / bootloader entropy (still non-repeating across boots).
class EspRandom : public dealer::RandomSource {
 public:
  uint32_t next32() override { return esp_random(); }
};
EspRandom hwRng;

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

void Machine::begin(uint64_t rackMask) {
  rack_.setMask(rackMask);

  pinMode(PIN_STEP_EN, OUTPUT);
  digitalWrite(PIN_STEP_EN, HIGH);  // drivers off until needed
  pinMode(PIN_FEED_IN2, OUTPUT);
  digitalWrite(PIN_FEED_IN2, LOW);
  for (int p : {PIN_IR_LOAD, PIN_IR_EXIT, PIN_HOME_TURN, PIN_HOME_RACK}) pinMode(p, INPUT_PULLUP);
  analogSetPinAttenuation(PIN_VBAT, ADC_11db);

  for (int t = 0; t < 4; t++) ESP32PWM::allocateTimer(t);
  feedPwm.attachPin(PIN_FEED_IN1, cfg::MOTOR_PWM_HZ, 8);
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
  // The turret may have been bumped while unpowered: always re-home. It was
  // parked at 0 deg, so this is normally a short move.
  const uint32_t to = turretParked_ ? cfg::TURN_REHOME_TIMEOUT_MS : cfg::TURN_HOME_TIMEOUT_MS;
  if (!homeTurret(to)) return fail(Fault::TurretHoming);
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

bool Machine::homeTurret(uint32_t timeoutMs) {
  const int32_t backoff = lroundf(cfg::TURN_BACKOFF_DEG * geom::TURRET_STEPS_PER_DEG);
  turretHomed_ = homeAxis(turret, PIN_HOME_TURN, cfg::TURN_HOMING_SPEED, backoff, timeoutMs);
  turretParked_ = turretHomed_;
  return turretHomed_;
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

void Machine::feedOn() { feedPwm.write(cfg::FEED_PWM); }
void Machine::feedOff() { feedPwm.write(0); }

void Machine::launchSet(uint8_t pwm) {
  if (pwm == launchNow_) return;
  launchPwmOut.write(pwm);
  if (pwm > 0) delay(cfg::SPINUP_MS);  // spin up / settle to the new speed
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
  switch (dealer::feedVerdict(saw, blocked, still)) {
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
  if (blocked > cfg::DOUBLE_MS) {
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
    if (empty) break;
  }
  storage::saveRack(rack_.mask());
  return Fault::None;
}

// ---------------- Deal ----------------

Fault Machine::ejectShelf(int shelf, float deg, uint8_t pwm, bool emptyIsFault) {
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
  delay(cfg::PUSH_RETRACT_MS);   // the blade must clear the rack before it moves
  return Fault::None;
}

Fault Machine::executePlan(const dealer::Plan& plan) {
  for (uint8_t i = 0; i < plan.count; i++) {
    const dealer::DealAction& a = plan.actions[i];
    const int shelf = rack_.pickRandom(hwRng);  // the shuffle
    if (shelf < 0) return fail(Fault::RackEmpty);
    const Fault f = ejectShelf(shelf, a.deg, launchPwm(a.launch), true);
    if (f != Fault::None) return f;
  }
  launchSet(0);
  storage::saveRack(rack_.mask());
  return Fault::None;
}

Fault Machine::purgeShelf(int shelf) {
  const Fault f = ejectShelf(shelf, dealer::burnDeg(), cfg::LAUNCH_BURN, false);
  launchSet(0);
  return f == Fault::RackEmpty ? Fault::None : f;
}

Fault Machine::purgeAll() {
  for (int s = 0; s < dealer::Rack::kShelves; s++) {
    if (!rack_.isOccupied(s)) continue;
    const Fault f = ejectShelf(s, dealer::burnDeg(), cfg::LAUNCH_BURN, false);
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
  rackAxis.moveTo(steps);
  runBoth();
}
void Machine::calAim(float deg) {
  aimAt(deg);
  runBoth();
}
void Machine::calLaunch(uint8_t pwm) { launchSet(pwm); }
void Machine::calFeed(uint8_t pwm) { feedPwm.write(pwm); }
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
