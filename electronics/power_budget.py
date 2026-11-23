#!/usr/bin/env python3
"""Power budget calculator for the poker dealer.

Prints the markdown tables used in electronics/POWER.md. Deterministic: all inputs are
constants below (datasheet / measured-typical values, documented in POWER.md).
Exit code 1 if the 12 V peak budget is >= 3 A or the 5 V firmware-reachable peak
exceeds the Mini560 rating.

    python3 electronics/power_budget.py            # print tables
    python3 electronics/power_budget.py --check    # only assert limits
"""
import sys

V12, V12_MIN, V5 = 12.0, 10.5, 5.0
BUCK_EFF, BUCK_EFF_LIGHT = 0.90, 0.80       # Mini560: ~90 % at 0.5-2.5 A, ~80 % at <0.2 A
BATT_AH, BATT_VNOM, USABLE = 6.0, 11.1, 0.85  # 85 % usable down to the 10.5 V cut-off
LIMIT_12V_A, BUCK_MAX_A = 3.0, 3.0

# name, rail, idle A, typical A (while working), peak A, duty note
LOADS = [
    ("Turret stepper (TMC2209 + NEMA17 pancake, 0.8 A RMS)", 12, 0.005, 0.45, 0.70, "moves before every card; holds otherwise"),
    ("Rack stepper (TMC2209 + NEMA17 T8, 0.8 A RMS)",        12, 0.005, 0.40, 0.70, "moves before every card / load"),
    ("Battery divider 100k/22k",                              12, 0.0001, 0.0001, 0.0001, "always (after switch)"),
    ("ESP32 DevKit (WiFi/BT off)",                            5, 0.050, 0.080, 0.250, "always"),
    ("MG90S pusher servo",                                    5, 0.006, 0.250, 0.700, "~0.4 s per dealt card; detached when idle"),
    ("N20 loader motor (DRV8833 ch A)",                       5, 0.0, 0.120, 0.600, "loading only (~0.5 s per card)"),
    ("130 launcher motor (DRV8833 ch B)",                     5, 0.0, 0.500, 1.500, "dealing only; 1.5 A = start-up surge"),
    ("IR break-beams x2 (emitter + receiver) + A3144 hall",                           5, 0.044, 0.044, 0.044, "always"),
    ("3V3 loads: OLED, TMC VIO x2",          5, 0.019, 0.025, 0.030, "always (via ESP32 LDO)"),
]

def at12(i5, eff=BUCK_EFF, v=V12):
    """12 V-side current drawn by the buck for a 5 V load current."""
    return i5 * V5 / eff / v

def sums(col):
    a12 = sum(l[col] for l in LOADS if l[1] == 12)
    a5 = sum(l[col] for l in LOADS if l[1] == 5)
    return a12, a5

def main():
    out = []
    p = out.append
    # ---- per-load table
    p("| Load | Rail | Idle (A) | Typical (A) | Peak (A) | Peak @12 V (A) | Duty |")
    p("|---|---|---|---|---|---|---|")
    for name, rail, idle, typ, peak, duty in LOADS:
        p12 = peak if rail == 12 else at12(peak)
        p(f"| {name} | {rail} V | {idle:.3f} | {typ:.3f} | {peak:.3f} | {p12:.3f} | {duty} |")

    # ---- peaks
    s12, s5 = sums(4)
    total_sum = s12 + at12(s5)
    n20 = next(l[4] for l in LOADS if l[0].startswith("N20"))
    s5_fw = s5 - n20                     # firmware never runs feed and launcher together
    total_fw = s12 + at12(s5_fw)
    # realistic coincident: both steppers at peak, launcher running, servo attached idle
    real5 = 0.250 + 0.500 + 0.010 + 0.044 + 0.030
    total_real = s12 + at12(real5)
    P_fw = s12 * V12 + s5_fw * V5 / BUCK_EFF
    P_real = s12 * V12 + real5 * V5 / BUCK_EFF
    P_sum = s12 * V12 + s5 * V5 / BUCK_EFF
    p("")
    p("| Peak case | 5 V rail (A) | 12 V rail (A) at 12.0 V | at 10.5 V cut-off | Power (W) |")
    p("|---|---|---|---|---|")
    p(f"| A. Sum of every peak (physically impossible: feed + launch never together) | {s5:.2f} | **{total_sum:.2f}** | {P_sum / V12_MIN:.2f} | {P_sum:.1f} |")
    p(f"| B. Firmware-reachable worst case (A without the N20) | {s5_fw:.2f} | **{total_fw:.2f}** | {P_fw / V12_MIN:.2f} | {P_fw:.1f} |")
    p(f"| C. Realistic coincident peak (both steppers accelerating, launcher running) | {real5:.2f} | **{total_real:.2f}** | {P_real / V12_MIN:.2f} | {P_real:.1f} |")

    # ---- idle
    i12, i5 = sums(2)
    P_idle = i12 * V12 + i5 * V5 / BUCK_EFF_LIGHT
    p("")
    p(f"IDLE: 12 V loads {i12 * 1000:.0f} mA + 5 V loads {i5 * 1000:.0f} mA "
      f"(buck at {BUCK_EFF_LIGHT:.0%} light-load efficiency) = **{P_idle:.2f} W = {P_idle / V12 * 1000:.0f} mA at 12 V**")

    # ---- energy per hand (6 players: 52 loads, 12 hole + 3 burn + 5 board = 20 deals)
    E_usable = BATT_AH * BATT_VNOM * USABLE
    phases = [
        # name, seconds, 12 V stepper A, 5 V A
        ("Load 52 cards (0.7 s/card)", 52 * 0.7, 0.20 + 0.25, 0.080 + 0.120 * 0.7 + 0.010 + 0.074),
        ("Deal 20 cards (1.2 s/card)", 20 * 1.2, 0.35 + 0.30, 0.080 + 0.450 + 0.150 + 0.074),
        ("Home / park, 4 power-ups (3 s each)", 4 * 3.0, 0.45 + 0.20, 0.080 + 0.010 + 0.074),
    ]
    p("")
    p("| Phase (6 players) | Time (s) | Avg 12 V steppers (A) | Avg 5 V (A) | Avg power (W) | Energy (J) |")
    p("|---|---|---|---|---|---|")
    E_act, t_act = 0.0, 0.0
    for name, t, a12, a5 in phases:
        P = a12 * V12 + a5 * V5 / BUCK_EFF
        E_act += P * t; t_act += t
        p(f"| {name} | {t:.0f} | {a12:.2f} | {a5:.3f} | {P:.1f} | {P * t:.0f} |")
    hand_s = 180.0                         # one hand every 3 min = 20 hands/h
    E_idle = (hand_s - t_act) * P_idle
    E_hand = E_act + E_idle
    p(f"| Idle while players act ({hand_s - t_act:.0f} s of a 3 min hand) | {hand_s - t_act:.0f} | 0.01 | {i5:.3f} | {P_idle:.2f} | {E_idle:.0f} |")
    p(f"| **Total per hand** | {hand_s:.0f} | | | | **{E_hand:.0f} J = {E_hand / 3600:.3f} Wh** |")

    hands = E_usable / (E_hand / 3600)
    p("")
    p(f"RUNTIME: usable energy {BATT_AH:.0f} Ah x {BATT_VNOM} V x {USABLE:.0%} = **{E_usable:.1f} Wh**  ")
    p(f"Hands per charge: {E_usable:.1f} / {E_hand / 3600:.3f} = **{hands:.0f} hands**  ")
    p(f"At 20 hands/h: **{hands / 20:.1f} h** of play. With a 2x safety factor (retries, "
      f"stepper resonance, cold battery): **{hands / 40:.1f} h** (~{hands / 2:.0f} hands).  ")
    p(f"Idle only (switched on, nothing dealt): {E_usable:.1f} / {P_idle:.2f} = **{E_usable / P_idle:.0f} h**.")

    ok = total_sum < LIMIT_12V_A and s5_fw <= BUCK_MAX_A and P_fw / V12_MIN < LIMIT_12V_A
    p("")
    p(f"CHECK: sum-of-peaks {total_sum:.2f} A < {LIMIT_12V_A} A at 12 V; firmware-reachable 5 V peak "
      f"{s5_fw:.2f} A <= {BUCK_MAX_A} A buck; firmware-reachable at 10.5 V {P_fw / V12_MIN:.2f} A < 3 A: "
      f"{'PASS' if ok else 'FAIL'}")
    if "--check" not in sys.argv:
        print("\n".join(out))
    else:
        print(out[-1])
    return 0 if ok else 1

if __name__ == "__main__":
    sys.exit(main())
