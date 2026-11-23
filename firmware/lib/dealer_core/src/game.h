// Hand / game state machine. Pure logic: it turns button events into
// commands and abstract deal plans ("deal a card to angle X with launch
// profile P"). The hardware layer executes the plan and reports the result.
#pragma once
#include <stdint.h>

#include "config.h"

namespace dealer {

enum class Launch : uint8_t { Seat, Board, Burn };

struct DealAction {
  float deg;         // turret angle, always in [0, 360)
  Launch launch;     // launcher speed profile
  int8_t seat;       // target seat for hole cards, -1 otherwise
  int8_t boardSlot;  // 0..4 for board cards, -1 otherwise
};

enum class Phase : uint8_t { Hole, Flop, Turn, River };

struct Plan {
  static constexpr uint8_t kMax = 2 * cfg::MAX_PLAYERS;
  DealAction actions[kMax];
  uint8_t count;
};

enum class State : uint8_t { WaitStart, WaitFlop, WaitTurn, WaitRiver };
enum class Event : uint8_t { GoShort, GoLong, SetShort, SetLong };

enum class Cmd : uint8_t {
  None,
  PlayersChanged,  // between hands: player count cycled (save settings)
  ButtonMoved,     // between hands: dealer button moved one seat (save settings)
  StartHand,       // battery check, load, verify 52, then deal plan(Phase::Hole)
  DealStreet,      // deal plan(phase): burn + board cards
  HandOver,        // hand finished (river done or everyone folded): button advanced
  Misdeal,         // hand aborted (SET long or a fault): button kept
};

struct Command {
  Cmd cmd;
  Phase phase;  // valid for StartHand (Hole) and DealStreet
};

class Game {
 public:
  Game(uint8_t players = cfg::DEFAULT_PLAYERS, uint8_t button = 0);

  // A debounced button event. Ignored (Cmd::None) while a deal is pending.
  Command onEvent(Event e);
  // The hardware finished the pending StartHand / DealStreet command.
  // Returns HandOver after a successful river, Misdeal after a failure, else None.
  Cmd onDealResult(bool ok);

  // The deal sequence for a phase, using the current players and button.
  Plan plan(Phase p) const;

  State state() const { return state_; }
  bool pending() const { return pending_; }
  uint8_t players() const { return players_; }
  uint8_t button() const { return button_; }
  // Seat that receives the first hole card (left of the button).
  uint8_t firstSeat() const { return static_cast<uint8_t>((button_ + 1) % players_); }

  // Cards successfully dealt in the current (or last) hand.
  uint8_t holeDealt() const { return holeDealt_; }
  uint8_t burnsDealt() const { return burnsDealt_; }
  uint8_t boardDealt() const { return boardDealt_; }

  static uint8_t clampPlayers(uint8_t n);

 private:
  void endHand(bool advanceButton);

  State state_ = State::WaitStart;
  uint8_t players_;
  uint8_t button_;
  bool pending_ = false;
  Phase pendingPhase_ = Phase::Hole;
  uint8_t holeDealt_ = 0, burnsDealt_ = 0, boardDealt_ = 0;
};

}  // namespace dealer
