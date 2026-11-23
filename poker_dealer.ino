// ============================================================================
//  Poker Dealer — ESP32 firmware v0.1 (draft, not yet compiled/tested on HW)
//
//  Mechanism: random-access rack. 52 shelves on a T8 leadscrew.
//   - LOAD:  collected cards go in the hopper; loader roller feeds each card
//            into the next EMPTY shelf (order doesn't matter).
//   - DEAL:  every card is pushed out of a RANDOM occupied shelf into the
//            launcher wheels  ->  uniform shuffle, no physical shuffle passes,
//            no double-feeds on the deal side.
//   - AIM:   the whole rack+launcher sits on a turntable (NEMA17 + GT2 belt).
//   - BURN:  launcher at low speed "dribbles" the card into a tray mounted
//            on the turret under the exit, so burns work at any angle.
//
//  Controls (2 buttons):
//   Between hands:  SET short = cycle player count (2..10, LED blinks count)
//                   SET long  = move dealer button +1 seat
//                   GO short  = load cards + deal 2 hole cards each
//   During a hand:  GO short  = next street (burn+flop / burn+turn / burn+river)
//                   GO long   = hand over early (everyone folded) -> button moves
//                   SET long  = misdeal -> button stays
//   After river the hand ends and the button advances automatically.
//
//  Libraries: AccelStepper, ESP32Servo. Preferences is built-in.
//  If step rate becomes the bottleneck, swap AccelStepper for FastAccelStepper.
// ============================================================================

#include <AccelStepper.h>
#include <ESP32Servo.h>
#include <Preferences.h>

// ---------------- Pins ----------------
#define PIN_TURN_STEP 25
#define PIN_TURN_DIR  26
#define PIN_RACK_STEP 27
#define PIN_RACK_DIR  14
#define PIN_STEP_EN   13   // TMC2209 EN (shared), active LOW
#define PIN_FEED_IN1  32   // DRV8833 ch A -> loader roller
#define PIN_FEED_IN2  33
#define PIN_LAUNCH    19   // DRV8833 ch B IN3 (IN4 tied to GND) -> launcher wheels
#define PIN_PUSHER    23   // MG90S servo driving a linear pusher blade
#define PIN_IR_LOAD   15   // break-beam at loader, LOW = blocked
#define PIN_IR_EXIT   4    // break-beam at launcher entry, LOW = blocked
#define PIN_HOME_TURN 16   // hall sensor, LOW at 0 deg
#define PIN_HOME_RACK 17   // endstop, LOW at rack bottom
#define PIN_BTN_GO    18
#define PIN_BTN_SET   5
#define PIN_LED       2
#define PIN_VBAT      34   // battery voltage via 100k/22k divider (input-only pin)

// ---------------- Battery (3S Li-ion) ----------------
const float VBAT_RATIO = (100.0f + 22.0f) / 22.0f;
const float VBAT_WARN  = 10.9f;   // warn: ~15% left
const float VBAT_MIN   = 10.5f;   // refuse to start a hand below this

// ---------------- Geometry (calibrate) ----------------
const int   DECK = 52;
const int   MAX_PLAYERS = 10;
const float TURN_STEPS_PER_DEG = 3200.0f * 6.0f / 360.0f; // 16 usteps, 6:1 belt
const float RACK_STEPS_PER_MM  = 200.0f;                  // T8 lead 8 mm, 8 usteps
const float SHELF_PITCH_MM     = 2.0f;
const float LOAD_OFFSET_MM     = 20.0f;  // loader station is this far ABOVE pusher station
const float RACK_EJECT_ZERO_MM = 25.0f;  // rack pos where shelf 0 faces pusher (must be >= LOAD_OFFSET_MM)

// Table layout. Board lands at BOARD_DEG (short throw next to the machine);
// seats are spread evenly around it, numbered CLOCKWISE (flip DIR wire if not).
const float BOARD_DEG        = 0.0f;
const float BOARD_SPREAD_DEG = 4.0f;     // fan the 5 board cards slightly

// ---------------- Motion / timing (calibrate) ----------------
const float TURN_MAX_SPEED = 6000, TURN_ACCEL = 12000;   // steps/s, steps/s^2
const float RACK_MAX_SPEED = 16000, RACK_ACCEL = 40000;
const float HOMING_SPEED   = 1500;

const uint8_t LAUNCH_SEAT  = 255;  // PWM: throw to a player
const uint8_t LAUNCH_BOARD = 110;  // PWM: short throw to board
const uint8_t LAUNCH_BURN  = 55;   // PWM: dribble into turret burn tray
const uint8_t FEED_PWM     = 170;

const int PUSH_HOME = 10, PUSH_OUT = 160;       // servo angles
const unsigned long NO_CARD_MS      = 1500;
const unsigned long JAM_MS          = 1000;
const unsigned long DOUBLE_MS       = 350;      // ~1.5x single-card beam-block time at FEED_PWM
const unsigned long SEAT_MS         = 150;      // extra loader run to seat card fully
const unsigned long SPINUP_MS       = 250;
const unsigned long PUSH_RETRACT_MS = 200;

// ---------------- State ----------------
AccelStepper turret(AccelStepper::DRIVER, PIN_TURN_STEP, PIN_TURN_DIR);
AccelStepper rack(AccelStepper::DRIVER, PIN_RACK_STEP, PIN_RACK_DIR);
Servo pusher;
Preferences prefs;

bool    occupied[DECK];
uint8_t numPlayers = 6;
uint8_t buttonSeat = 0;
int     boardIdx   = 0;
uint8_t launchNow  = 0;

enum State { WAIT_START, WAIT_FLOP, WAIT_TURN, WAIT_RIVER };
State state = WAIT_START;

// ---------------- Helpers ----------------
// Unbiased uniform integer in [0, n) from the ESP32 hardware RNG.
// (True RNG when WiFi/BT is on; seeded pseudo-random otherwise.)
uint32_t randBelow(uint32_t n) {
  uint32_t limit = UINT32_MAX - (UINT32_MAX % n);
  uint32_t r;
  do { r = esp_random(); } while (r >= limit);
  return r % n;
}

int countCards() { int c = 0; for (bool o : occupied) c += o; return c; }

void saveRack() {
  uint64_t m = 0;
  for (int i = 0; i < DECK; i++) if (occupied[i]) m |= (1ULL << i);
  prefs.putULong64("rack", m);
}
void saveSettings() {
  prefs.putUChar("players", numPlayers);
  prefs.putUChar("button", buttonSeat);
}

void blink(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(PIN_LED, HIGH); delay(150);
    digitalWrite(PIN_LED, LOW);  delay(150);
  }
}

void feedOn()  { analogWrite(PIN_FEED_IN1, FEED_PWM); digitalWrite(PIN_FEED_IN2, LOW); }
void feedOff() { analogWrite(PIN_FEED_IN1, 0);        digitalWrite(PIN_FEED_IN2, LOW); }

void launchSet(uint8_t pwm) {
  if (pwm == launchNow) return;
  analogWrite(PIN_LAUNCH, pwm);
  if (pwm > 0) delay(SPINUP_MS);   // spin up / settle down to new speed
  launchNow = pwm;
}

void stopAll() { feedOff(); launchSet(0); pusher.write(PUSH_HOME); }

float readVbat() {
  uint32_t mv = 0;
  for (int i = 0; i < 8; i++) mv += analogReadMilliVolts(PIN_VBAT);
  return (mv / 8.0f / 1000.0f) * VBAT_RATIO;
}

// Steppers + servo draw power even when holding still, so they are only
// powered while the machine is actually moving cards.
bool homeAxis(AccelStepper& s, int pin, unsigned long timeoutMs);
void runBoth();
void aimAt(float deg);
void fault(const char* msg);

bool powerUp() {
  digitalWrite(PIN_STEP_EN, LOW);
  pusher.attach(PIN_PUSHER);
  pusher.write(PUSH_HOME);
  delay(50);
  // Turret may have been bumped while unpowered; it was parked at BOARD_DEG (=0), so this is quick.
  if (!homeAxis(turret, PIN_HOME_TURN, 5000)) { fault("turret homing failed"); return false; }
  return true;  // rack leadscrew is self-locking, no re-home needed
}

void powerDown() {
  stopAll();
  aimAt(BOARD_DEG); runBoth();   // park at home so the next re-home is short
  delay(200);
  pusher.detach();
  digitalWrite(PIN_STEP_EN, HIGH);
}

float seatDeg(uint8_t seat) { return BOARD_DEG + (seat + 0.5f) * 360.0f / numPlayers; }

void aimAt(float deg)       { turret.moveTo(lround(deg * TURN_STEPS_PER_DEG)); }
void rackToEject(int shelf) { rack.moveTo(lround((RACK_EJECT_ZERO_MM + shelf * SHELF_PITCH_MM) * RACK_STEPS_PER_MM)); }
void rackToLoad(int shelf)  { rack.moveTo(lround((RACK_EJECT_ZERO_MM + shelf * SHELF_PITCH_MM - LOAD_OFFSET_MM) * RACK_STEPS_PER_MM)); }

void runBoth() {
  while (turret.distanceToGo() || rack.distanceToGo()) { turret.run(); rack.run(); }
}

enum Press { NONE, SHORT, LONG };
Press readBtn(int pin) {
  if (digitalRead(pin) == HIGH) return NONE;
  delay(30);
  if (digitalRead(pin) == HIGH) return NONE;
  unsigned long t = millis();
  while (digitalRead(pin) == LOW) {
    if (millis() - t > 800) { while (digitalRead(pin) == LOW) {} return LONG; }
  }
  return SHORT;
}

// Stop everything, blink fast, wait for GO. Caller aborts the hand.
void fault(const char* msg) {
  stopAll();
  Serial.printf("FAULT: %s  (press GO to continue)\n", msg);
  while (readBtn(PIN_BTN_GO) == NONE) digitalWrite(PIN_LED, (millis() / 100) % 2);
  digitalWrite(PIN_LED, LOW);
}

bool homeAxis(AccelStepper& s, int pin, unsigned long timeoutMs) {
  s.setSpeed(-HOMING_SPEED);
  unsigned long t0 = millis();
  while (digitalRead(pin) == HIGH) {
    s.runSpeed();
    if (millis() - t0 > timeoutMs) return false;
  }
  s.setCurrentPosition(0);
  return true;
}

// ---------------- Card operations ----------------
// Feed cards from the hopper into empty shelves until the hopper is empty.
bool loadCards() {
  for (int shelf = 0; shelf < DECK; shelf++) {
    if (occupied[shelf]) continue;
    rackToLoad(shelf); runBoth();

    feedOn();
    unsigned long t0 = millis();
    while (digitalRead(PIN_IR_LOAD) == HIGH) {
      if (millis() - t0 > NO_CARD_MS) { feedOff(); saveRack(); return true; } // hopper empty
    }
    unsigned long tb = millis();
    while (digitalRead(PIN_IR_LOAD) == LOW) {
      if (millis() - tb > JAM_MS) { fault("loader jam"); return false; }
    }
    unsigned long blocked = millis() - tb;
    delay(SEAT_MS);
    feedOff();
    occupied[shelf] = true;

    if (blocked > DOUBLE_MS) {
      Serial.printf("Shelf %d probably has 2 cards\n", shelf);
      saveRack();
      fault("double feed while loading");
      return false;
    }
  }
  saveRack();
  return true;
}

// Pull a random card from the rack and launch it toward `deg`.
bool dealCard(float deg, uint8_t launchPwm) {
  int n = countCards();
  if (n == 0) { fault("rack empty"); return false; }

  int k = randBelow(n), shelf = -1;
  for (int i = 0; i < DECK; i++) if (occupied[i] && k-- == 0) { shelf = i; break; }

  aimAt(deg); rackToEject(shelf); runBoth();
  launchSet(launchPwm);

  pusher.write(PUSH_OUT);
  unsigned long t0 = millis();
  while (digitalRead(PIN_IR_EXIT) == HIGH) {
    if (millis() - t0 > NO_CARD_MS) {
      pusher.write(PUSH_HOME);
      occupied[shelf] = false;  // rack map was wrong
      fault("no card in shelf");
      return false;
    }
  }
  pusher.write(PUSH_HOME);      // launcher has the card now
  unsigned long tb = millis();
  while (digitalRead(PIN_IR_EXIT) == LOW) {
    if (millis() - tb > JAM_MS) { fault("launcher jam"); return false; }
  }
  occupied[shelf] = false;
  delay(PUSH_RETRACT_MS);       // pusher must clear the rack before it moves
  return true;
}

bool dealHoleCards() {
  for (int round = 0; round < 2; round++) {
    for (int i = 1; i <= numPlayers; i++) {          // start left of the button
      uint8_t seat = (buttonSeat + i) % numPlayers;
      if (!dealCard(seatDeg(seat), LAUNCH_SEAT)) return false;
    }
  }
  launchSet(0);
  saveRack();
  return true;
}

// Burn 1, then deal `count` community cards.
bool dealStreet(int count) {
  if (!dealCard(BOARD_DEG, LAUNCH_BURN)) return false;
  for (int i = 0; i < count; i++) {
    float d = BOARD_DEG + (boardIdx - 2) * BOARD_SPREAD_DEG;
    boardIdx++;
    if (!dealCard(d, LAUNCH_BOARD)) return false;
  }
  launchSet(0);
  saveRack();
  return true;
}

void endHand(bool advanceButton) {
  powerDown();
  if (advanceButton) buttonSeat = (buttonSeat + 1) % numPlayers;
  saveSettings();
  Serial.printf("Hand over. Button at seat %d. Put cards in hopper, press GO.\n", buttonSeat);
  state = WAIT_START;
}

void startHand() {
  float v = readVbat();
  Serial.printf("Battery: %.2f V\n", v);
  if (v < VBAT_MIN) { fault("battery empty - charge it"); return; }
  if (v < VBAT_WARN) { Serial.println("Battery low"); blink(10); }

  boardIdx = 0;
  if (!powerUp()) { powerDown(); return; }
  if (!loadCards()) { powerDown(); return; }
  int n = countCards();
  if (n != DECK) {
    Serial.printf("Rack has %d cards, expected %d\n", n, DECK);
    fault("card count mismatch");
    powerDown();
    return;
  }
  if (dealHoleCards()) { powerDown(); state = WAIT_FLOP; }
  else endHand(false);
}

// Burn + deal a street with motors powered only for the duration.
bool street(int count) {
  if (!powerUp()) return false;
  bool ok = dealStreet(count);
  powerDown();
  return ok;
}

// ---------------- Arduino ----------------
void setup() {
  Serial.begin(115200);

  pinMode(PIN_STEP_EN, OUTPUT);  digitalWrite(PIN_STEP_EN, LOW);
  pinMode(PIN_FEED_IN1, OUTPUT); pinMode(PIN_FEED_IN2, OUTPUT);
  pinMode(PIN_LAUNCH, OUTPUT);   pinMode(PIN_LED, OUTPUT);
  for (int p : {PIN_IR_LOAD, PIN_IR_EXIT, PIN_HOME_TURN, PIN_HOME_RACK, PIN_BTN_GO, PIN_BTN_SET})
    pinMode(p, INPUT_PULLUP);
  stopAll();

  turret.setMaxSpeed(TURN_MAX_SPEED); turret.setAcceleration(TURN_ACCEL);
  rack.setMaxSpeed(RACK_MAX_SPEED);   rack.setAcceleration(RACK_ACCEL);

  pusher.attach(PIN_PUSHER);
  pusher.write(PUSH_HOME);
  delay(300);

  prefs.begin("dealer");
  numPlayers = constrain(prefs.getUChar("players", 6), 2, MAX_PLAYERS);
  buttonSeat = prefs.getUChar("button", 0) % numPlayers;
  uint64_t m = prefs.getULong64("rack", 0);
  for (int i = 0; i < DECK; i++) occupied[i] = (m >> i) & 1ULL;

  if (!homeAxis(rack, PIN_HOME_RACK, 15000))   fault("rack homing failed");
  if (!homeAxis(turret, PIN_HOME_TURN, 20000)) fault("turret homing failed");
  powerDown();

  Serial.printf("Ready: %d players, button seat %d, %d cards in rack, battery %.2f V\n",
                numPlayers, buttonSeat, countCards(), readVbat());
  blink(numPlayers);
}

void loop() {
  Press go  = readBtn(PIN_BTN_GO);
  Press set = readBtn(PIN_BTN_SET);

  if (state != WAIT_START) {
    if (set == LONG) { Serial.println("Misdeal"); endHand(false); return; }
    if (go  == LONG) { endHand(true); return; }
  }

  switch (state) {
    case WAIT_START:
      if (set == SHORT) {
        numPlayers = (numPlayers >= MAX_PLAYERS) ? 2 : numPlayers + 1;
        buttonSeat %= numPlayers;
        saveSettings();
        Serial.printf("Players: %d\n", numPlayers);
        blink(numPlayers);
      } else if (set == LONG) {
        buttonSeat = (buttonSeat + 1) % numPlayers;
        saveSettings();
        Serial.printf("Button seat: %d\n", buttonSeat);
        blink(buttonSeat + 1);
      } else if (go == SHORT) {
        startHand();
      }
      break;

    case WAIT_FLOP:
      if (go == SHORT) { if (street(3)) state = WAIT_TURN;  else endHand(false); }
      break;
    case WAIT_TURN:
      if (go == SHORT) { if (street(1)) state = WAIT_RIVER; else endHand(false); }
      break;
    case WAIT_RIVER:
      if (go == SHORT) { if (street(1)) endHand(true);      else endHand(false); }
      break;
  }
}
