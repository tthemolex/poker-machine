#include "console.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

using dealer::Fault;

namespace {

char line[64];
size_t len = 0;
uint32_t lastCmdMs = 0;
bool motorsTouched = false;  // a console command powered or ran something
bool serviceMode = false;

bool refuseInService() {
  if (!serviceMode) return false;
  Serial.println("service mode: no motion (reset to exit)");
  return true;
}

void help() {
  Serial.println(
      "Commands (between hands; motion commands power up + home first, 'off' to release):\n"
      "  (motors power down automatically after 60 s without a command)\n"
      "  help | status | vbat | sensors\n"
      "  on | off | home            power up + home / park + power down / re-home both axes\n"
      "  rack eject <shelf>         shelf 0..51 to the eject station\n"
      "  rack load <shelf>          shelf 0..51 to the load station\n"
      "  rack mm <mm> | rack steps <n>   absolute rack position from home\n"
      "  turret <deg>               aim at a table angle (0..360)\n"
      "  seat <n> | board <slot>    aim at seat 1..N / board slot 0..4\n"
      "  launch <pwm> [ms]          run launcher (default 2000 ms; ms=0 leaves it on)\n"
      "  feed <pwm> [ms]            run loader roller (default 1000 ms)\n"
      "  servo <angle>              pusher servo angle\n"
      "  deal seat <n> | deal board <slot> | deal burn   deal one random card\n"
      "  loadone                    feed one card into the next empty shelf, print blocked ms\n"
      "  load                       fill empty shelves from the hopper\n"
      "  purge                      every full shelf onto the burn spot\n"
      "  clear                      forget the rack map (after emptying the rack by hand)");
}

bool report(Fault f) {
  if (f == Fault::None) return true;
  Serial.printf("FAULT: %s (motors released)\n", dealer::faultText(f));
  return false;
}

bool ensurePowered(Machine& m) {
  if (refuseInService()) return false;
  motorsTouched = true;
  if (m.powered()) return true;
  float v;
  const dealer::BatteryLevel b = m.battery(&v);
  if (!dealer::batteryAllowsStart(b)) {
    Serial.printf("battery %.2f V (%s): motors not powered\n", static_cast<double>(v),
                  dealer::batteryText(b));
    return false;
  }
  return report(m.powerUp());
}

void status(Machine& m, const dealer::Game& g) {
  float v;
  const dealer::BatteryLevel bl = m.battery(&v);
  Serial.printf("players %u, button seat %u, rack %d/52, mask 0x%013llx\n",
                static_cast<unsigned>(g.players()), static_cast<unsigned>(g.button() + 1),
                m.rack().count(), static_cast<unsigned long long>(m.rack().mask()));
  Serial.printf("battery %.2f V (%s), motors %s, rack %ld steps (%.2f mm), turret %ld steps (%.2f deg)\n",
                static_cast<double>(v), dealer::batteryText(bl),
                m.powered() ? "ON" : "off", static_cast<long>(m.rackSteps()),
                static_cast<double>(m.rackSteps() / geom::RACK_STEPS_PER_MM),
                static_cast<long>(m.turretSteps()),
                static_cast<double>(m.turretSteps() / geom::TURRET_STEPS_PER_DEG));
  Serial.printf("trims: RACK_ZERO_TRIM_MM %.2f, TURRET_ZERO_TRIM_DEG %.2f, BOARD_DEG %d\n",
                static_cast<double>(cfg::RACK_ZERO_TRIM_MM),
                static_cast<double>(cfg::TURRET_ZERO_TRIM_DEG), static_cast<int>(geom::BOARD_DEG));
}

void sensors() {
  Serial.printf("IR_LOAD %s, IR_EXIT %s, HOME_TURN %s, HOME_RACK %s, GO %s, SET %s\n",
                Machine::sensor(PIN_IR_LOAD) ? "BLOCKED" : "clear",
                Machine::sensor(PIN_IR_EXIT) ? "BLOCKED" : "clear",
                Machine::sensor(PIN_HOME_TURN) ? "ACTIVE" : "-",
                Machine::sensor(PIN_HOME_RACK) ? "ACTIVE" : "-",
                Machine::sensor(PIN_BTN_GO) ? "down" : "up",
                Machine::sensor(PIN_BTN_SET) ? "down" : "up");
}

uint8_t pwmArg(const char* s) {
  const long v = s ? strtol(s, nullptr, 10) : 0;
  return static_cast<uint8_t>(v < 0 ? 0 : v > 255 ? 255 : v);
}

bool shelfArg(const char* s, int* out) {
  if (!s) return false;
  const long v = strtol(s, nullptr, 10);
  if (v < 0 || v >= dealer::Rack::kShelves) return false;
  *out = static_cast<int>(v);
  return true;
}

// Returns true if the rack map changed.
bool run(Machine& m, const dealer::Game& g, char* cmdline) {
  char* save = nullptr;
  const char* cmd = strtok_r(cmdline, " \t", &save);
  const char* a1 = strtok_r(nullptr, " \t", &save);
  const char* a2 = strtok_r(nullptr, " \t", &save);
  if (!cmd) return false;
  const uint64_t before = m.rack().mask();

  if (!strcmp(cmd, "help")) {
    help();
  } else if (!strcmp(cmd, "status")) {
    status(m, g);
  } else if (!strcmp(cmd, "vbat")) {
    float v;
    const dealer::BatteryLevel bl = m.battery(&v);
    Serial.printf("%.3f V (%s)\n", static_cast<double>(v), dealer::batteryText(bl));
  } else if (!strcmp(cmd, "sensors")) {
    sensors();
  } else if (!strcmp(cmd, "on")) {
    if (ensurePowered(m)) Serial.println("powered, homed");
  } else if (!strcmp(cmd, "off")) {
    if (refuseInService()) return false;
    m.powerDown();
    motorsTouched = false;
    Serial.println("parked, motors off");
  } else if (!strcmp(cmd, "home")) {
    if (refuseInService()) return false;
    m.safeStop();  // forget both zeros, then home both
    if (report(m.powerUp())) Serial.println("homed");
  } else if (!strcmp(cmd, "rack")) {
    int shelf;
    if (!a1 || !a2) {
      Serial.println("usage: rack eject|load <shelf> | rack mm <mm> | rack steps <n>");
    } else if (ensurePowered(m)) {
      if (!strcmp(a1, "eject") && shelfArg(a2, &shelf)) {
        m.calRackSteps(dealer::rackEjectSteps(shelf));
      } else if (!strcmp(a1, "load") && shelfArg(a2, &shelf)) {
        m.calRackSteps(dealer::rackLoadSteps(shelf));
      } else if (!strcmp(a1, "mm")) {
        m.calRackMm(strtof(a2, nullptr));
      } else if (!strcmp(a1, "steps")) {
        m.calRackSteps(strtol(a2, nullptr, 10));
      } else {
        Serial.println("bad rack argument");
      }
      Serial.printf("rack at %ld steps\n", static_cast<long>(m.rackSteps()));
    }
  } else if (!strcmp(cmd, "turret") || !strcmp(cmd, "seat") || !strcmp(cmd, "board")) {
    if (!a1) {
      Serial.println("usage: turret <deg> | seat <1..N> | board <0..4>");
    } else if (ensurePowered(m)) {
      float deg;
      if (cmd[0] == 't') deg = strtof(a1, nullptr);
      else if (cmd[0] == 's') deg = dealer::seatDeg(static_cast<uint8_t>(atoi(a1) - 1), g.players());
      else deg = dealer::boardDeg(static_cast<uint8_t>(atoi(a1) % 5));
      m.calAim(deg);
      Serial.printf("turret at %.2f deg\n", static_cast<double>(dealer::normDeg(deg)));
    }
  } else if (!strcmp(cmd, "launch") || !strcmp(cmd, "feed")) {
    const bool launch = cmd[0] == 'l';
    const uint8_t pwm = pwmArg(a1);
    const long ms = a2 ? strtol(a2, nullptr, 10) : (launch ? 2000 : 1000);
    if (refuseInService()) return false;
    motorsTouched = true;
    if (launch) m.calLaunch(pwm); else m.calFeed(pwm);
    Serial.printf("%s PWM %u\n", cmd, static_cast<unsigned>(pwm));
    if (ms > 0) {
      delay(static_cast<uint32_t>(ms));
      if (launch) m.calLaunch(0); else m.calFeed(0);
      Serial.printf("%s off\n", cmd);
    }
  } else if (!strcmp(cmd, "servo")) {
    if (a1 && ensurePowered(m)) {
      m.calServo(atoi(a1));
      Serial.printf("servo %d\n", atoi(a1));
    }
  } else if (!strcmp(cmd, "deal")) {
    dealer::Plan p{};
    p.count = 1;
    if (a1 && !strcmp(a1, "seat") && a2) {
      const uint8_t s = static_cast<uint8_t>((atoi(a2) - 1 + g.players()) % g.players());
      p.actions[0] = {dealer::seatDeg(s, g.players()), dealer::Launch::Seat, static_cast<int8_t>(s), -1};
    } else if (a1 && !strcmp(a1, "board") && a2) {
      const uint8_t k = static_cast<uint8_t>(atoi(a2) % 5);
      p.actions[0] = {dealer::boardDeg(k), dealer::Launch::Board, -1, static_cast<int8_t>(k)};
    } else if (a1 && !strcmp(a1, "burn")) {
      p.actions[0] = {dealer::burnDeg(), dealer::Launch::Burn, -1, -1};
    } else {
      p.count = 0;
      Serial.println("usage: deal seat <n> | deal board <slot> | deal burn");
    }
    if (p.count && ensurePowered(m) && report(m.executePlan(p))) Serial.println("dealt");
  } else if (!strcmp(cmd, "loadone")) {
    const int shelf = m.rack().nextEmpty();
    if (shelf < 0) {
      Serial.println("rack full");
    } else if (ensurePowered(m)) {
      uint32_t blocked;
      bool empty;
      const Fault f = m.loadOne(shelf, &blocked, &empty);
      if (empty) Serial.println("hopper empty (no card within NO_CARD_MS)");
      else Serial.printf("shelf %d: beam blocked %lu ms (DOUBLE_MS %lu)\n", shelf,
                         static_cast<unsigned long>(blocked),
                         static_cast<unsigned long>(cfg::DOUBLE_MS));
      report(f);
    }
  } else if (!strcmp(cmd, "load")) {
    if (ensurePowered(m) && report(m.loadCards()))
      Serial.printf("rack %d/52\n", m.rack().count());
  } else if (!strcmp(cmd, "purge")) {
    if (ensurePowered(m) && report(m.purgeAll())) Serial.println("rack purged");
  } else if (!strcmp(cmd, "clear")) {
    m.clearMap();
    Serial.println("rack map cleared");
  } else {
    Serial.printf("unknown command '%s', type help\n", cmd);
  }
  return m.rack().mask() != before;
}

}  // namespace

namespace console {

void setServiceMode(bool on) { serviceMode = on; }

bool poll(Machine& m, const dealer::Game& g) {
  bool changed = false;
  if (motorsTouched && millis() - lastCmdMs > cfg::CONSOLE_IDLE_MS) {
    m.powerDown();  // also stops the loader / launcher and detaches the servo
    motorsTouched = false;
    Serial.println("console idle: parked, motors off");
  }
  while (Serial.available() > 0) {
    const int c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (len == 0) continue;
      line[len] = '\0';
      len = 0;
      Serial.printf("> %s\n", line);
      lastCmdMs = millis();
      changed |= run(m, g, line);
    } else if (len + 1 < sizeof line) {
      line[len++] = static_cast<char>(c);
    }
  }
  return changed;
}

}  // namespace console
