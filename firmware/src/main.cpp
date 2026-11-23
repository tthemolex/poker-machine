// =============================================================================
//  Poker Dealer firmware (ESP32 DevKit V1). Refactor of the draft poker_dealer.ino.
//
//  Controls (2 buttons):
//   Between hands:  SET short = cycle player count (2..10, LED blinks count)
//                   SET long  = move dealer button +1 seat (LED blinks seat)
//                   GO short  = load cards + deal 2 hole cards each
//   During a hand:  GO short  = next street (burn+flop / burn+turn / burn+river)
//                   GO long   = hand over early (everyone folded) -> button moves
//                   SET long  = misdeal -> button stays
//   After the river the hand ends and the button advances automatically.
//   Faults: everything stops, LED blinks fast, OLED shows the fault; fix it, press GO.
//   SET held while powering on: purge the rack (every shelf into the burn tray).
//   Serial console (115200, "help") for calibration between hands.
//
//  Game logic lives in lib/dealer_core (unit-tested on the host); this file only
//  wires buttons -> dealer::Game -> Machine.
// =============================================================================
#include <Arduino.h>

#include "config.h"
#include "console.h"
#include "dealer_core.h"
#include "machine.h"
#include "storage.h"
#include "ui.h"

using dealer::Cmd;
using dealer::Fault;
using dealer::LedPattern;

namespace {

Machine machine;
dealer::Game game;
float vbat = 0.0f;
dealer::BatteryLevel battery = dealer::BatteryLevel::Ok;
uint32_t lastVbatMs = 0;
const char* message = "";
const char* startNote = nullptr;  // why the last start failed, if special

UiStatus status(bool busy, Fault f = Fault::None) {
  return UiStatus{game.state(), busy, game.players(), game.button(), machine.rack().count(),
                  vbat, battery, f, message};
}

void idlePattern() {
  if (game.state() != dealer::State::WaitStart) {
    ui::setPattern(LedPattern::InHand);
  } else {
    ui::setPattern(battery == dealer::BatteryLevel::Ok ? LedPattern::Ready
                                                       : LedPattern::ReadyLowBat);
  }
}

void busy(const char* what) {
  message = what;
  Serial.println(what);
  ui::setPattern(LedPattern::Busy);
  ui::draw(status(true), true);
}

void updateBattery() {
  vbat = machine.readVbat();
  battery = dealer::classifyBattery(vbat);
  lastVbatMs = millis();
}

// Stop, show the fault, wait for GO. The machine is already released (safeStop).
void faultWait(Fault f) {
  Serial.printf("FAULT: %s  (fix it, then press GO)\n", dealer::faultText(f));
  ui::setPattern(LedPattern::Fault);
  for (;;) {
    ui::tick();
    ui::draw(status(false, f));
    if (ui::pollGo() != dealer::Press::None) break;
    ui::pollSet();
    delay(2);
  }
  message = "";
  idlePattern();
}

// Machine fault -> wait for acknowledgement. Returns true if there was a fault.
bool check(Fault f) {
  if (f == Fault::None) return false;
  faultWait(f);
  return true;
}

// Battery check, load, verify 52 cards, deal hole cards.
bool startHand() {
  updateBattery();
  Serial.printf("Battery: %.2f V (%s)\n", static_cast<double>(vbat), dealer::batteryText(battery));
  if (!dealer::batteryAllowsStart(battery)) {
    faultWait(battery == dealer::BatteryLevel::Absent ? Fault::NoBattery : Fault::BatteryLow);
    return false;
  }
  if (battery == dealer::BatteryLevel::Warn) Serial.println("Battery low: charge soon");

  busy("Homing");
  if (check(machine.powerUp())) return false;

  busy("Loading cards");
  const Fault lf = machine.loadCards();
  if (lf == Fault::DoubleFeed) {
    const int shelf = machine.lastLoadedShelf();
    faultWait(lf);
    // Recovery: throw the doubled shelf into the burn tray. The user puts those
    // cards back in the hopper and presses GO again.
    busy("Clearing double");
    if (!check(machine.powerUp()) && !check(machine.purgeShelf(shelf))) machine.powerDown();
    startNote = "Return burn tray cards";
    return false;
  }
  if (check(lf)) return false;

  const int n = machine.rack().count();
  if (n != geom::SHELF_COUNT) {
    Serial.printf("Rack has %d cards, expected %d\n", n, static_cast<int>(geom::SHELF_COUNT));
    machine.powerDown();
    faultWait(Fault::CountMismatch);
    return false;
  }

  busy("Dealing hole cards");
  const Fault df = machine.executePlan(game.plan(dealer::Phase::Hole));
  if (check(df)) return false;
  machine.powerDown();
  return true;
}

bool dealStreet(dealer::Phase p) {
  static const char* names[] = {"", "Dealing flop", "Dealing turn", "Dealing river"};
  busy(names[static_cast<int>(p)]);
  if (check(machine.powerUp())) return false;
  const Fault f = machine.executePlan(game.plan(p));
  if (check(f)) return false;
  machine.powerDown();
  return true;
}

// `why` overrides the default message (e.g. after a failed start).
void handEnded(Cmd c, const char* why = nullptr) {
  machine.powerDown();
  storage::saveSettings(game.players(), game.button());
  message = why ? why : (c == Cmd::Misdeal) ? "Misdeal: button stays" : "Hand over";
  Serial.printf("%s. Button at seat %u. Put cards in hopper, press GO.\n", message,
                static_cast<unsigned>(game.button() + 1));
}

void handle(dealer::Command c) {
  switch (c.cmd) {
    case Cmd::None:
      return;
    case Cmd::PlayersChanged:
      storage::saveSettings(game.players(), game.button());
      Serial.printf("Players: %u\n", static_cast<unsigned>(game.players()));
      message = "";
      ui::showCount(game.players());
      return;
    case Cmd::ButtonMoved:
      storage::saveSettings(game.players(), game.button());
      Serial.printf("Button seat: %u\n", static_cast<unsigned>(game.button() + 1));
      message = "";
      ui::showCount(static_cast<uint8_t>(game.button() + 1));
      return;
    case Cmd::StartHand: {
      startNote = nullptr;
      const bool ok = startHand();
      const char* why = startNote ? startNote : "Hand not started";
      const Cmd r = game.onDealResult(ok);
      if (r != Cmd::None) handEnded(r, why);  // failed start: button kept
      else message = "";
      break;
    }
    case Cmd::DealStreet: {
      const Cmd r = game.onDealResult(dealStreet(c.phase));
      if (r != Cmd::None) handEnded(r);
      else message = "";
      break;
    }
    case Cmd::HandOver:
    case Cmd::Misdeal:
      handEnded(c.cmd);
      break;
  }
  idlePattern();
  ui::draw(status(false), true);
}

dealer::Event toEvent(dealer::Press p, bool isGo) {
  if (isGo) return p == dealer::Press::Long ? dealer::Event::GoLong : dealer::Event::GoShort;
  return p == dealer::Press::Long ? dealer::Event::SetLong : dealer::Event::SetShort;
}

}  // namespace

void setup() {
  Serial.begin(115200);
  ui::begin();
  storage::begin();
  const Settings s = storage::load();
  game = dealer::Game(s.players, s.button);
  machine.begin(s.rackMask);
  const bool purge = ui::setHeldRaw();
  updateBattery();

  busy("Homing");
  if (!check(machine.powerUp())) {
    if (purge) {
      busy("Purging rack");
      if (!check(machine.purgeAll())) machine.powerDown();
    } else {
      machine.powerDown();
    }
  }
  message = "";
  Serial.printf("Ready: %u players, button seat %u, %d cards in rack, battery %.2f V, OLED %s\n",
                static_cast<unsigned>(game.players()), static_cast<unsigned>(game.button() + 1),
                machine.rack().count(), static_cast<double>(vbat), ui::oledPresent() ? "yes" : "no");
  idlePattern();
  ui::showCount(game.players());
  ui::draw(status(false), true);
}

void loop() {
  const dealer::Press go = ui::pollGo();
  const dealer::Press set = ui::pollSet();
  if (go != dealer::Press::None) handle(game.onEvent(toEvent(go, true)));
  if (set != dealer::Press::None) handle(game.onEvent(toEvent(set, false)));

  // Serial calibration console, between hands only.
  if (game.state() == dealer::State::WaitStart && !game.pending()) {
    if (console::poll(machine, game)) ui::draw(status(false), true);
  }

  // Idle: motors are off, so the battery reading is the resting voltage.
  if (millis() - lastVbatMs > 5000) {
    updateBattery();
    idlePattern();
  }
  ui::tick();
  ui::draw(status(false));
}
