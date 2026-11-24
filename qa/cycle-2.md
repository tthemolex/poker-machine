# QA cycle 2 — verdict: REJECT

- [cycle-2-fw.md](cycle-2-fw.md): firmware / electronics / docs. 13 PASS, 1 FAIL (BUILD.md feed-shaft length, missing pinch shaft step).
  Fresh-clone `make all`: exit 0, ~30 min, clean tree. All cycle-1 findings fixed except L4 (BOM check relies on a hand-kept vendor list; verified complete by hand).
- [cycle-2-cad.md](cycle-2-cad.md): CAD / renders / size. 7 PASS, 3 FAIL (5 tolerances, 6 battery door screws, 9 plausibility).
  HIGH: pinch rollers counter-rotate (open O-ring loop); clock-spring ribbon is 25.4 mm wide in a 9.5 mm gap; battery door screws miss the flange.
  Size confirmed independently: Ø199.0 × 207.5 mm.

Accepted deviations: D-02 (10:1 ring), D-20 (burns to a table burn spot), D-01 (pitch 1.5, fins 0.9).

Rework assigned:
- CAD: pinch reversal + direction check, harness redesign (base stack may grow ≤ 8 mm, RACK_Z_MIN may change),
  door screws on the flange + modelled, remaining fit literals → TOL + wider lint, probe row, stale D-17 text, burn release info.
- Firmware: boot-time entropy seeding (no ADC conflict) + VBAT plausibility guard, homing reach docs, probe text,
  loadOne uses FeedVerdict, console clamps, burn clears slot.
- Docs: BUILD shaft lengths (FAIL), burn/board radial separation acceptance test + Known issues, POWER ramp text,
  Known deviations D-01/D-02, harness conductor allocation after the CAD change.
