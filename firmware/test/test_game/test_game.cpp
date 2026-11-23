// Game state machine: deal order, streets, burns, button rotation.
#include <unity.h>

#include "dealer_core.h"

using namespace dealer;

void setUp() {}
void tearDown() {}

static void runOk(Game& g, Phase expect) {
  // helper: GO short, check command, report success
  Command c = g.onEvent(Event::GoShort);
  if (expect == Phase::Hole) {
    TEST_ASSERT_EQUAL(Cmd::StartHand, c.cmd);
  } else {
    TEST_ASSERT_EQUAL(Cmd::DealStreet, c.cmd);
  }
  TEST_ASSERT_EQUAL(expect, c.phase);
  TEST_ASSERT_TRUE(g.pending());
}

// Hole cards: 2 rounds clockwise starting left of the button, every seat
// exactly twice, all thrown at that seat's angle with the Seat profile.
static void checkHoleOrder(uint8_t n) {
  for (uint8_t b = 0; b < n; b++) {
    Game g(n, b);
    Plan p = g.plan(Phase::Hole);
    TEST_ASSERT_EQUAL_UINT8(2 * n, p.count);
    uint8_t perSeat[cfg::MAX_PLAYERS] = {0};
    for (uint8_t i = 0; i < p.count; i++) {
      const DealAction& a = p.actions[i];
      const uint8_t expected = static_cast<uint8_t>((b + 1 + (i % n)) % n);
      TEST_ASSERT_EQUAL_INT8(expected, a.seat);
      TEST_ASSERT_EQUAL(Launch::Seat, a.launch);
      TEST_ASSERT_EQUAL_INT8(-1, a.boardSlot);
      TEST_ASSERT_FLOAT_WITHIN(1e-4f, seatDeg(expected, n), a.deg);
      TEST_ASSERT_TRUE(a.deg >= 0.0f && a.deg < 360.0f);
      perSeat[a.seat]++;
    }
    // first card goes to the seat left of the button, the button gets the last card of each round
    TEST_ASSERT_EQUAL_INT8((b + 1) % n, p.actions[0].seat);
    TEST_ASSERT_EQUAL_INT8(b, p.actions[n - 1].seat);
    TEST_ASSERT_EQUAL_INT8(b, p.actions[2 * n - 1].seat);
    for (uint8_t s = 0; s < n; s++) TEST_ASSERT_EQUAL_UINT8(2, perSeat[s]);
  }
}

void test_hole_order_2_players() { checkHoleOrder(2); }
void test_hole_order_6_players() { checkHoleOrder(6); }
void test_hole_order_10_players() { checkHoleOrder(10); }
void test_hole_order_all_counts() {
  for (uint8_t n = cfg::MIN_PLAYERS; n <= cfg::MAX_PLAYERS; n++) checkHoleOrder(n);
}

void test_hole_order_explicit_6_button_3() {
  Game g(6, 3);
  Plan p = g.plan(Phase::Hole);
  const int8_t expect[12] = {4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3};
  for (int i = 0; i < 12; i++) TEST_ASSERT_EQUAL_INT8(expect[i], p.actions[i].seat);
}

void test_street_plans() {
  Game g(6, 0);
  Plan f = g.plan(Phase::Flop), t = g.plan(Phase::Turn), r = g.plan(Phase::River);
  TEST_ASSERT_EQUAL_UINT8(4, f.count);
  TEST_ASSERT_EQUAL_UINT8(2, t.count);
  TEST_ASSERT_EQUAL_UINT8(2, r.count);
  // each street starts with exactly one burn at the board angle
  const Plan* streets[3] = {&f, &t, &r};
  for (const Plan* p : streets) {
    TEST_ASSERT_EQUAL(Launch::Burn, p->actions[0].launch);
    TEST_ASSERT_FLOAT_WITHIN(1e-4f, burnDeg(), p->actions[0].deg);
    for (uint8_t i = 1; i < p->count; i++) TEST_ASSERT_EQUAL(Launch::Board, p->actions[i].launch);
  }
  // board slots 0,1,2 | 3 | 4 fanned (slot-2)*spread around the board
  TEST_ASSERT_EQUAL_INT8(0, f.actions[1].boardSlot);
  TEST_ASSERT_EQUAL_INT8(1, f.actions[2].boardSlot);
  TEST_ASSERT_EQUAL_INT8(2, f.actions[3].boardSlot);
  TEST_ASSERT_EQUAL_INT8(3, t.actions[1].boardSlot);
  TEST_ASSERT_EQUAL_INT8(4, r.actions[1].boardSlot);
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, boardDeg(0), f.actions[1].deg);
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, boardDeg(4), r.actions[1].deg);
}

// Full hand through the state machine: 2N hole, exactly 3 burns, 5 board cards,
// button advances by one at the end.
void test_full_hand_counts_and_button_advance() {
  for (uint8_t n = cfg::MIN_PLAYERS; n <= cfg::MAX_PLAYERS; n++) {
    for (uint8_t b = 0; b < n; b++) {
      Game g(n, b);
      TEST_ASSERT_EQUAL(State::WaitStart, g.state());
      runOk(g, Phase::Hole);
      TEST_ASSERT_EQUAL(Cmd::None, g.onDealResult(true));
      TEST_ASSERT_EQUAL(State::WaitFlop, g.state());
      TEST_ASSERT_EQUAL_UINT8(2 * n, g.holeDealt());
      TEST_ASSERT_EQUAL_UINT8(0, g.burnsDealt());
      runOk(g, Phase::Flop);
      TEST_ASSERT_EQUAL(Cmd::None, g.onDealResult(true));
      TEST_ASSERT_EQUAL(State::WaitTurn, g.state());
      TEST_ASSERT_EQUAL_UINT8(1, g.burnsDealt());
      TEST_ASSERT_EQUAL_UINT8(3, g.boardDealt());
      runOk(g, Phase::Turn);
      TEST_ASSERT_EQUAL(Cmd::None, g.onDealResult(true));
      TEST_ASSERT_EQUAL(State::WaitRiver, g.state());
      runOk(g, Phase::River);
      TEST_ASSERT_EQUAL(Cmd::HandOver, g.onDealResult(true));
      TEST_ASSERT_EQUAL(State::WaitStart, g.state());
      TEST_ASSERT_EQUAL_UINT8(2 * n, g.holeDealt());
      TEST_ASSERT_EQUAL_UINT8(3, g.burnsDealt());
      TEST_ASSERT_EQUAL_UINT8(5, g.boardDealt());
      TEST_ASSERT_TRUE(2 * n + 3 + 5 <= geom::SHELF_COUNT);
      TEST_ASSERT_EQUAL_UINT8((b + 1) % n, g.button());
    }
  }
}

// Fold (GO long) advances the button, misdeal (SET long) keeps it, at every street.
void test_fold_vs_misdeal() {
  const State inHand[3] = {State::WaitFlop, State::WaitTurn, State::WaitRiver};
  for (int stop = 0; stop < 3; stop++) {
    for (int fold = 0; fold < 2; fold++) {
      Game g(6, 4);
      runOk(g, Phase::Hole);
      g.onDealResult(true);
      if (stop >= 1) { runOk(g, Phase::Flop); g.onDealResult(true); }
      if (stop >= 2) { runOk(g, Phase::Turn); g.onDealResult(true); }
      TEST_ASSERT_EQUAL(inHand[stop], g.state());
      Command c = g.onEvent(fold ? Event::GoLong : Event::SetLong);
      TEST_ASSERT_EQUAL(fold ? Cmd::HandOver : Cmd::Misdeal, c.cmd);
      TEST_ASSERT_EQUAL(State::WaitStart, g.state());
      TEST_ASSERT_EQUAL_UINT8(fold ? 5 : 4, g.button());
    }
  }
}

void test_button_wraps_around() {
  Game g(3, 2);
  runOk(g, Phase::Hole); g.onDealResult(true);
  TEST_ASSERT_EQUAL(Cmd::HandOver, g.onEvent(Event::GoLong).cmd);
  TEST_ASSERT_EQUAL_UINT8(0, g.button());
}

// A hardware fault during any deal aborts the hand and keeps the button.
void test_fault_keeps_button() {
  for (int at = 0; at < 4; at++) {
    Game g(5, 1);
    const Phase ph[4] = {Phase::Hole, Phase::Flop, Phase::Turn, Phase::River};
    for (int i = 0; i < at; i++) { runOk(g, ph[i]); g.onDealResult(true); }
    runOk(g, ph[at]);
    TEST_ASSERT_EQUAL(Cmd::Misdeal, g.onDealResult(false));
    TEST_ASSERT_EQUAL(State::WaitStart, g.state());
    TEST_ASSERT_EQUAL_UINT8(1, g.button());
    TEST_ASSERT_FALSE(g.pending());
  }
}

void test_events_ignored_while_pending() {
  Game g(6, 0);
  runOk(g, Phase::Hole);
  TEST_ASSERT_EQUAL(Cmd::None, g.onEvent(Event::GoShort).cmd);
  TEST_ASSERT_EQUAL(Cmd::None, g.onEvent(Event::GoLong).cmd);
  TEST_ASSERT_EQUAL(Cmd::None, g.onEvent(Event::SetLong).cmd);
  TEST_ASSERT_EQUAL(Cmd::None, g.onEvent(Event::SetShort).cmd);
  TEST_ASSERT_EQUAL_UINT8(6, g.players());
  TEST_ASSERT_EQUAL(Cmd::None, Game().onDealResult(true));  // nothing pending
}

void test_settings_between_hands() {
  Game g(9, 8);
  TEST_ASSERT_EQUAL(Cmd::PlayersChanged, g.onEvent(Event::SetShort).cmd);
  TEST_ASSERT_EQUAL_UINT8(10, g.players());
  TEST_ASSERT_EQUAL_UINT8(8, g.button());
  TEST_ASSERT_EQUAL(Cmd::PlayersChanged, g.onEvent(Event::SetShort).cmd);
  TEST_ASSERT_EQUAL_UINT8(2, g.players());       // wraps 10 -> 2
  TEST_ASSERT_EQUAL_UINT8(0, g.button());        // 8 % 2
  TEST_ASSERT_EQUAL(Cmd::ButtonMoved, g.onEvent(Event::SetLong).cmd);
  TEST_ASSERT_EQUAL_UINT8(1, g.button());
  TEST_ASSERT_EQUAL(Cmd::ButtonMoved, g.onEvent(Event::SetLong).cmd);
  TEST_ASSERT_EQUAL_UINT8(0, g.button());
  TEST_ASSERT_EQUAL(Cmd::None, g.onEvent(Event::GoLong).cmd);  // GO long idle: nothing
  // SET short during a hand does nothing
  runOk(g, Phase::Hole); g.onDealResult(true);
  TEST_ASSERT_EQUAL(Cmd::None, g.onEvent(Event::SetShort).cmd);
  TEST_ASSERT_EQUAL_UINT8(2, g.players());
}

void test_constructor_clamps() {
  TEST_ASSERT_EQUAL_UINT8(2, Game(0, 0).players());
  TEST_ASSERT_EQUAL_UINT8(10, Game(200, 0).players());
  TEST_ASSERT_EQUAL_UINT8(3, Game(6, 9).button());
}

// Every action angle of every phase, for n = 2..10 and every button seat, is in
// [0, 360) and equals the un-wrapped formula: nothing relies on normDeg wrapping.
void test_all_angles_in_range_no_wrap() {
  const Phase phases[4] = {Phase::Hole, Phase::Flop, Phase::Turn, Phase::River};
  for (uint8_t n = cfg::MIN_PLAYERS; n <= cfg::MAX_PLAYERS; n++) {
    for (uint8_t b = 0; b < n; b++) {
      Game g(n, b);
      for (Phase ph : phases) {
        Plan p = g.plan(ph);
        for (uint8_t i = 0; i < p.count; i++) {
          const DealAction& a = p.actions[i];
          TEST_ASSERT_TRUE(a.deg >= 0.0f && a.deg < 360.0f);
          float raw;
          if (a.launch == Launch::Seat) raw = geom::BOARD_DEG + (a.seat + 0.5f) * 360.0f / n;
          else if (a.launch == Launch::Board) raw = geom::BOARD_DEG + (a.boardSlot - 2.0f) * geom::BOARD_SPREAD_DEG;
          else raw = geom::BOARD_DEG;
          TEST_ASSERT_TRUE(raw >= 0.0f && raw < 360.0f);   // no wrap needed
          TEST_ASSERT_FLOAT_WITHIN(1e-4f, raw, a.deg);
        }
      }
    }
  }
  // Board fan is monotonic and inside [0, 16] with the current params.
  Game g(10, 0);
  Plan f = g.plan(Phase::Flop), t = g.plan(Phase::Turn), r = g.plan(Phase::River);
  const float fan[5] = {f.actions[1].deg, f.actions[2].deg, f.actions[3].deg, t.actions[1].deg, r.actions[1].deg};
  for (int k = 0; k < 4; k++) TEST_ASSERT_TRUE(fan[k] < fan[k + 1]);
  TEST_ASSERT_TRUE(fan[0] >= 0.0f);
  TEST_ASSERT_TRUE(fan[4] <= geom::BOARD_DEG + 2.0f * geom::BOARD_SPREAD_DEG + 1e-4f);
  // Highest seat angle (n = 10, last seat) stays below 360.
  TEST_ASSERT_TRUE(seatDeg(9, 10) < 360.0f);
  TEST_ASSERT_FLOAT_WITHIN(1e-3f, geom::BOARD_DEG + 9.5f * 36.0f, seatDeg(9, 10));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_hole_order_2_players);
  RUN_TEST(test_hole_order_6_players);
  RUN_TEST(test_hole_order_10_players);
  RUN_TEST(test_hole_order_all_counts);
  RUN_TEST(test_hole_order_explicit_6_button_3);
  RUN_TEST(test_street_plans);
  RUN_TEST(test_full_hand_counts_and_button_advance);
  RUN_TEST(test_fold_vs_misdeal);
  RUN_TEST(test_button_wraps_around);
  RUN_TEST(test_fault_keeps_button);
  RUN_TEST(test_events_ignored_while_pending);
  RUN_TEST(test_settings_between_hands);
  RUN_TEST(test_constructor_clamps);
  RUN_TEST(test_all_angles_in_range_no_wrap);
  return UNITY_END();
}
