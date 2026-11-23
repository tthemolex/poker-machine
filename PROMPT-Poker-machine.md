# Project: Automated Texas Hold'em Card Shuffler & Dealer (ESP32, battery-powered)

You are the **lead engineer / orchestrator** for this project. Build a complete, buildable, open-hardware project in this repo: firmware, parametric 3D models (OpenSCAD), assembly renders, wiring, BOM and build guide. Work autonomously until every acceptance criterion at the bottom passes, then stop.

A draft firmware `poker_dealer.ino` is in the repo root. It is the starting point and the source of truth for pins, behaviour and the mechanism concept. Refactor it freely, but keep the behaviour.

---

## 1. Product spec

**What it does (one button press per step):**
1. **Load:** the user drops the collected cards into the hopper. The machine feeds each card into an empty shelf of a 52-shelf rack and verifies there are 52 cards.
2. **Deal hole cards:** 2 cards per player, one at a time, clockwise, starting left of the dealer button.
3. **Burn + flop (3)**, then **burn + turn (1)**, then **burn + river (1)**. The machine waits for a GO press before each street.
4. **End of hand:** the dealer button advances. A long-press ends a hand early (folded); the button still advances. A misdeal keeps the button where it is.
- 2–10 players. Player count and button seat are stored in NVS.

**Mechanism (random-access rack, keep this concept):**
- **Rack:** 52 horizontal shelves, 2.0 mm pitch, on a NEMA17 with an integrated T8 leadscrew. Two 8 mm rods + LM8UU bearings guide it.
- **Shuffle:** each dealt card is pushed out of a uniformly random occupied shelf. This is the shuffle; there is no physical shuffle pass. Use the ESP32 `esp_random()` with rejection sampling.
- **Loader:** a hopper with a weighted deck, an N20 roller feeding the bottom card, and a pinch roller into the shelf at the "load station". The load station sits above the "eject station".
- **Pusher:** an MG90S servo with a rack-and-pinion blade pushes the selected card out of the eject station into the launcher.
- **Launcher:** one 130-size DC motor drives a rubber wheel, with a spring-loaded idler. Speed sets the throw: full power for a seat, medium for the board (short throw next to the machine, fanned ±2 × 4°), very low for a burn (the card drops into a burn tray mounted on the turret).
- **Turret:** the rack, loader, pusher and launcher all sit on a turntable. It uses a lazy susan bearing (100–150 mm) with a NEMA17 + GT2 20T pulley driving a printed ~120T ring (6:1). It homes with an A3144 hall sensor + magnet. Rotation stays within 0–360° (no cable wrap past one turn).
- **Sensors:** IR break-beam at the loader (card count + double-feed detection by blocked time) and at the launcher entry. Microswitch at the rack bottom.
- **UI:** GO and SET buttons, status LED. Optional SSD1306 OLED; include a mount for it and a firmware screen.
- **Machine sits at the table centre.** Seats are evenly spaced around 360°, with the board in the gap between the last and first seat.
- **Cards:** standard poker size, 63.5 × 88.9 mm, ~0.30 mm thick, 100% plastic.

**Size is a top priority: minimal and tiny, not bulky.** It must look like a sleek, compact table gadget.
- **Hard limits (whole machine incl. battery):** footprint inside a Ø200 mm circle, height ≤ 220 mm.
- **Stretch target:** Ø170 mm, height ≤ 180 mm.
- **Packaging:**
  - Battery, ESP32, drivers and buck go in the base, under or around the lazy susan, with no external boxes.
  - Shelf pitch as small as reliably printable (target 1.6 mm; justify the final value).
  - Rack travel / dual-station layout optimized to minimize tower height.
  - Hopper integrated into the turret envelope.
  - Tight, clean wire routing.
- **Look:** clean minimal exterior with no exposed electronics: smooth shell, rounded edges, flush buttons, a small slot for the card exit.
- **Do not trade reliability for size.** The card path, clearances and serviceability win (battery removable or charge jack reachable, rack and pusher accessible for clearing jams). Log every size trade-off in DECISIONS.md.
- After the first working assembly, run a dedicated **size-optimization pass**: identify the dimensions that drive the envelope, shrink them, re-run all checks.

**Electronics & power:**
- ESP32 DevKit V1 (WROOM-32), 2× TMC2209 (STEP/DIR, shared EN), DRV8833 (A = loader, B = launcher with IN4 to GND).
- **Battery:** TalentCell YB1206000-USB (3S Li-ion, 11.1 V, 6 Ah, 36 W max, 5.5×2.1 mm DC barrel, own charger). The enclosure needs a cradle or strap for it and a panel DC jack. Do not design a custom battery or charger.
- 12 V→5 V buck (Mini560 or LM2596, 3 A) for the ESP32, servo and DC motors. Main power switch.
- Battery sense: 100k/22k divider to GPIO34. Warn at 10.9 V, refuse to start a hand below 10.5 V.
- Power saving: steppers are disabled (EN high) and the servo detached whenever the machine is idle. The turret parks at 0° and re-homes before each action.
- Keep the total peak draw under 3 A at 12 V. TMC2209 current ~0.8 A RMS. Document the power budget and runtime estimate.

---

## 2. Deliverables (repo layout)

```
/firmware        PlatformIO project (esp32dev, Arduino framework), src/, include/config.h, test/
/cad             OpenSCAD sources: params.scad (ALL shared dimensions), one file per part, assembly.scad
/cad/stl         Exported printable STLs, one per part, in print orientation
/cad/vendor      Simple dimensionally accurate models of bought parts (NEMA17, leadscrew, LM8UU, rods,
                 lazy susan, servo, N20, 130 motor, ESP32, drivers, battery, buttons, sensors, wheels)
/renders         PNGs: full assembly (3+ angles), exploded view, section view through rack/pusher/launcher,
                 each printed part alone; optional turret-rotation GIF
/electronics     Wiring diagram (WireViz YAML → SVG/PNG, or Graphviz), pin table, power budget
/docs            BUILD.md (print settings, assembly steps referencing renders, calibration procedure),
                 BOM.csv (part, qty, spec, est. price PLN/EUR, example source), FIRMWARE.md (usage, buttons)
/qa              One review report per cycle: qa/cycle-N.md
README.md        Overview, hero render, feature list, links to everything
Makefile         `make all` = compile firmware + run tests + export STLs + renders + wiring diagram + checks
```

Install whatever tools you need (openscad, platformio, wireviz, graphviz, admesh/trimesh, imagemagick). Use OpenSCAD headless (`openscad -o ... --render --colorscheme ... --imgsize ... --camera ...`) for STLs and PNGs.

---

## 3. Team & process

Use subagents for parallel work. Suggested roles:
- **Mechanical/CAD agent:** params.scad, all parts, vendor models, assembly, STL export.
- **Firmware agent:** PlatformIO project, config.h with all calibration constants, unit tests for the game logic (deal order, button rotation, street sequence, RNG uniformity, rack bookkeeping). Put the logic in hardware-independent modules so they run under `pio test -e native`.
- **Electronics/Docs agent:** wiring diagram, pin table matching firmware, power budget, BOM, BUILD.md, README.
- **QA agent(s):** independent reviewers. **Never** the same agent that wrote the work. Spawn them fresh each cycle with no authoring context.

You, the orchestrator, own `params.scad` ↔ `config.h` consistency. Shelf pitch, rack offsets, gear ratios, steps/mm and angles must come from one documented set of values. Generate one from the other or check them with a script.

**Cycle: DEVELOP → QA → APPROVE / REJECT**
1. **Plan:** write `PLAN.md` with milestones and task owners.
2. **Develop:** agents implement their tasks.
3. **QA:** fresh reviewer agents check the work against the acceptance criteria below. They run every check themselves (compile, tests, renders, scripts) and **look at the rendered PNGs** with the image viewer. They write `qa/cycle-N.md` with a verdict per criterion, **PASS / FAIL**, plus concrete evidence (command output, file, render) and required fixes.
4. **Approve or reject:** any FAIL means rework only the failing items and go back to QA. All PASS means the project is approved.
5. Commit after each approved milestone with a clear message.

Rules:
- Reviewers must be skeptical: verify, don't trust claims. A criterion with no evidence is a FAIL.
- Max 6 QA cycles. If something still fails after that, stop. Document it in README under "Known issues", with what was tried and what's left.
- Don't ask me questions mid-run. Make reasonable engineering decisions and log them in `docs/DECISIONS.md`.

---

## 4. Acceptance criteria (all must PASS)

**Firmware**
- [ ] `pio run -e esp32dev` compiles with zero errors and no warnings in project code.
- [ ] `pio test -e native` passes. Tests cover: hole-card order for 2, 6 and 10 players and any button seat; street sequence and burn count (total 3 burns + 5 board cards); button advance vs misdeal; uniform shelf selection (chi-square over ≥100k draws); rack never double-allocates or deals from an empty shelf.
- [ ] Pins in firmware = pin table = wiring diagram (checked by a script).
- [ ] Battery check, idle power-down, fault handling (jam / no card / double feed / count ≠ 52) are all present and documented.

**CAD**
- [ ] Every part renders without errors; every STL is manifold (admesh/trimesh check script).
- [ ] Every printed part fits a 220 × 220 × 250 mm bed and is printable without supports or with minimal ones (document which). No walls under 1.2 mm, no unsupported overhangs over 50° unless noted.
- [ ] Assembly with ALL vendor and printed parts placed. An automated interference check (OpenSCAD `intersection()` of part pairs → empty or a tiny tolerance volume) passes for all static parts, at turret 0° and with the rack at the top and bottom of its travel.
- [ ] Card path is geometrically valid: card fits a shelf with ≥0.3 mm clearance; load station, eject station and launcher nip line up within ±0.2 mm; pusher stroke fully ejects the card into the nip; rack travel covers shelves 0–51 at both stations.
- [ ] Clearances for 3 mm heat-set inserts / M3 screws; print tolerances parameterized (default 0.2 mm).
- [ ] The battery fits and can be removed for charging, or the charge jack is reachable. The power switch and buttons are reachable from outside.
- [ ] Size: a script measures the full assembly bounding box (all parts incl. vendor) and the max radius from the turret axis. It must be within the hard limits (Ø200 mm, 220 mm tall). The result is reported in README with the stretch-target gap. The size-optimization pass is done and documented.
- [ ] No electronics visible from outside in the renders; the exterior reads as a clean, minimal product (reviewer judges from renders).

**Renders & docs**
- [ ] Renders exist for full assembly (3+ angles), exploded view, section view and every printed part. A reviewer looked at each one and confirms that parts are visible, correctly placed, distinctly coloured, and that nothing floats or intersects.
- [ ] BOM.csv covers every bought part used in the assembly (script cross-checks vendor models vs BOM) with quantities, specs and prices.
- [ ] BUILD.md lets a person with a 3D printer and basic soldering build it: print list + settings, assembly order with render references, wiring, first flash, calibration (shelf pitch, DOUBLE_MS, launcher PWMs, seat angles).
- [ ] Power budget shows peak draw < 3 A at 12 V and estimates runtime on 6 Ah.
- [ ] `make all` runs clean from a fresh clone.

---

## 5. Done

When the final QA cycle approves everything:
1. Write `qa/FINAL.md`: criteria table with PASS + evidence links, and the known limitations.
2. Make sure README shows the hero render and links everything.
3. Make a final commit.
4. Print a short summary to me: what was built, final dimensions, where the renders are, and the top 3 things to watch out for in the physical build.

Then stop.
