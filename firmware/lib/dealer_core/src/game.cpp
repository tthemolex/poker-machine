#include "game.h"

#include "motion.h"

namespace dealer {

uint8_t Game::clampPlayers(uint8_t n) {
  if (n < cfg::MIN_PLAYERS) return cfg::MIN_PLAYERS;
  if (n > cfg::MAX_PLAYERS) return cfg::MAX_PLAYERS;
  return n;
}

Game::Game(uint8_t players, uint8_t button)
    : players_(clampPlayers(players)), button_(static_cast<uint8_t>(button % players_)) {}

Plan Game::plan(Phase p) const {
  Plan pl{};
  pl.count = 0;
  auto add = [&pl](float deg, Launch l, int8_t seat, int8_t slot) {
    if (pl.count < Plan::kMax) pl.actions[pl.count++] = DealAction{deg, l, seat, slot};
  };
  switch (p) {
    case Phase::Hole:
      // Two rounds, one card at a time, clockwise, starting left of the button.
      for (int round = 0; round < 2; round++)
        for (uint8_t i = 1; i <= players_; i++) {
          const uint8_t seat = static_cast<uint8_t>((button_ + i) % players_);
          add(seatDeg(seat, players_), Launch::Seat, static_cast<int8_t>(seat), -1);
        }
      break;
    case Phase::Flop:
      add(burnDeg(), Launch::Burn, -1, -1);
      for (uint8_t s = 0; s < 3; s++) add(boardDeg(s), Launch::Board, -1, static_cast<int8_t>(s));
      break;
    case Phase::Turn:
      add(burnDeg(), Launch::Burn, -1, -1);
      add(boardDeg(3), Launch::Board, -1, 3);
      break;
    case Phase::River:
      add(burnDeg(), Launch::Burn, -1, -1);
      add(boardDeg(4), Launch::Board, -1, 4);
      break;
  }
  return pl;
}

void Game::endHand(bool advanceButton) {
  if (advanceButton) button_ = static_cast<uint8_t>((button_ + 1) % players_);
  state_ = State::WaitStart;
  pending_ = false;
}

Command Game::onEvent(Event e) {
  Command none{Cmd::None, Phase::Hole};
  if (pending_) return none;

  if (state_ == State::WaitStart) {
    switch (e) {
      case Event::SetShort:
        players_ = (players_ >= cfg::MAX_PLAYERS) ? cfg::MIN_PLAYERS
                                                  : static_cast<uint8_t>(players_ + 1);
        button_ = static_cast<uint8_t>(button_ % players_);
        return {Cmd::PlayersChanged, Phase::Hole};
      case Event::SetLong:
        button_ = static_cast<uint8_t>((button_ + 1) % players_);
        return {Cmd::ButtonMoved, Phase::Hole};
      case Event::GoShort:
        holeDealt_ = burnsDealt_ = boardDealt_ = 0;
        pending_ = true;
        pendingPhase_ = Phase::Hole;
        return {Cmd::StartHand, Phase::Hole};
      case Event::GoLong:
        return none;
    }
    return none;
  }

  // In a hand.
  if (e == Event::SetLong) {
    endHand(false);
    return {Cmd::Misdeal, Phase::Hole};
  }
  if (e == Event::GoLong) {  // everyone folded: hand over early
    endHand(true);
    return {Cmd::HandOver, Phase::Hole};
  }
  if (e == Event::GoShort) {
    Phase next = Phase::Flop;
    if (state_ == State::WaitTurn) next = Phase::Turn;
    if (state_ == State::WaitRiver) next = Phase::River;
    pending_ = true;
    pendingPhase_ = next;
    return {Cmd::DealStreet, next};
  }
  return none;  // SET short during a hand does nothing
}

Cmd Game::onDealResult(bool ok) {
  if (!pending_) return Cmd::None;
  pending_ = false;
  if (!ok) {  // any fault aborts the hand like a misdeal: button stays
    endHand(false);
    return Cmd::Misdeal;
  }
  // Tally what was actually dealt from the executed plan.
  const Plan done = plan(pendingPhase_);
  for (uint8_t i = 0; i < done.count; i++) {
    switch (done.actions[i].launch) {
      case Launch::Seat: holeDealt_++; break;
      case Launch::Burn: burnsDealt_++; break;
      case Launch::Board: boardDealt_++; break;
    }
  }
  switch (pendingPhase_) {
    case Phase::Hole: state_ = State::WaitFlop; return Cmd::None;
    case Phase::Flop: state_ = State::WaitTurn; return Cmd::None;
    case Phase::Turn: state_ = State::WaitRiver; return Cmd::None;
    case Phase::River: endHand(true); return Cmd::HandOver;
  }
  return Cmd::None;
}

}  // namespace dealer
