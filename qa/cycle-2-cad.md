# QA cycle 2: CAD, renders, size (independent review)

Reviewer: fresh QA agent, no authoring context. Date: 2026-11-24.
Scope: the CAD acceptance criteria (spec section 4 "CAD"), every render, and the cycle-1 CAD findings.
Source state: part `.scad` files last changed 16:16-16:25, `params.scad` 20:54; STLs 22:09, renders 22:09-22:25.
Every render and STL is newer than the sources. I re-ran every tool myself (see "Commands run").
Temporary files are in the QA scratchpad. The only project file I changed is this report. Running the tools regenerated `cad/stl`, `cad/build`, `cad/size_report.json` and `renders/size_report.md`; the output is deterministic and unchanged in content.

**Verdict: REJECT. 3 of 10 criteria FAIL (5, 6, 9).** There are two new HIGH mechanical defects:
the pinch rollers are driven the wrong way, and the ribbon clock spring cannot fit its space.
There is also one HIGH buildability defect: the battery door cannot be screwed on.

## Criteria

| # | Criterion | Result | Evidence | Required fix |
|---|---|---|---|---|
| 1 | Every part renders; every STL is manifold | **PASS** | `python3 tools/cad_export.py`: **69/69 PASS** (33 printed, 36 vendor), rc 0, no OpenSCAD warnings, **0 slivers removed** (cycle-1 L1 is fixed at the source). I ran my own check: `trimesh` on 6 full-assembly exports (all parts in one OpenSCAD run) gave valid meshes, and the OpenSCAD log shows "Status: NoError" for all 6. The export script runs admesh on every STL (0 edges fixed, 0 facets added/removed/reversed). Every printed part has 1 body and min z 0. | None. |
| 2 | Bed 220x220x250; supports documented; walls >= 1.2; overhangs > 50° noted | **PASS** | Largest parts: base_shell, top_cap and turntable at 199x199, turret_shell 199x199x153.5; all fit. `tools/cad_printability.py`: 33/33 PASS, all min walls >= 1.19 except the documented comb fins (0.45 at the chamfer) and gear tips, 0 undocumented overhang regions, 0 fit-literal lint hits. **Independent check:** 80 000 surface samples per part on 28 parts, inward rays. Strict test (opposite face anti-parallel, dot < -0.9): only **turntable** has 3 samples at **0.81 mm**, at the rim of a countersink near (79, 49). A 1.7 mm CSK in a 2.5 mm plate leaves a feather edge there. That is inherent to countersinking and LOW. Loose test (dot < -0.5): hits only on chamfers, V-grooves and 45° wedge tips. I probed each of the smallest ones (rail_frame (-7,16), ui_bezel (-49,-82), loader_frame IR pocket (36,-17,69), battery_door CSK rim, turret_shell tab wedge tips). All are external edges between oblique faces, not walls. The cycle-1 thin walls are fixed: launcher_floor min 1.20, rack_nut_arm 1.46, loader_frame 1.19, rack_motor_mount 1.20, idler_arm 1.29. New parts: slot_panel 1.20, shell_anchor 1.75, spool_hub 2.93, pulleys 4.0 / 2.39. In `parts.csv` the base_shell rebate is now called a 4 mm cantilever, and the motor-mount seat is a solid 43 mm bridge (printed upside down). | None. LOW: the printability script accepts an overhang region if *any* documented entry for that part has a larger area (kind and location are not matched), and its 12 000 random samples can miss local spots (it missed the turntable CSK rim). |
| 3 | All parts placed; interference at turret 0°, rack bottom and top | **PASS** | `tools/cad_interference.py`: rack 0 has 83 parts and 272 pairs; rack 81 has 82 parts and 271 pairs; **0 FAIL**. Allowed list (5 entries) now has volume caps: inserts 92.2 / cap 101.6, axle 1.06 / 1.3, spline 2.10 / 2.5, nut screws 30.6 / 44.9, pinch squeeze 2.30 / 3.0. I read the script: it uses all pairs (`itertools.combinations`), a bbox prefilter, and exact manifold intersection. | None for the criterion. NOTE: the battery-door screws are not modelled. If they were, this check would have found defect H-C below. LOW: the nut-screw cap is 1.47x the measured volume (the "expected" formula overestimates). Overlaps under 1 mm³ pass silently, e.g. hopper_gate/IR tx 0.60 and spool_hub/ribbon 0.93. |
| 4 | Card path: shelf clearance >= 0.3; stations and nip ±0.2; stroke into the nip; travel 0-51 | **PASS** | `tools/cad_cardpath.py`: **34/34 PASS**. **Independent checks:** (a) I ray-cast the `rack_comb_nx` mesh (the script uses px) at y = -8, 5 and 45: 51 fins, all 0.900, all 50 gaps exactly 0.600, bottom plate top 45.70, so a 0.30 card has 0.30 clearance. (b) Placed-mesh bounds at rack 0: card_eject bottom 122.20, launcher_floor top 122.20, idler top 122.20, wheel bottom 122.30. That gives nip squeeze 0.20 and alignment error 0.000, against Z_EJECT 122.20. Pinch rollers: bottom 126.85 = Z_LOAD + 0.30 - 0.15 (0.15 squeeze), front face -12.50 = RACK_Y0 - 0.5. (c) Stroke: 7.5 mm × 150° = 19.635 mm. Need: (-11.75 - (-13.0)) + (65 + 2 - 51.75) = 16.5 mm, so it passes. Firmware geometry.h has PUSH_HOME 10 and OUT 160, which are @shared (cycle-1 M6 fixed). **Pinch coast row:** 52.5 - (-16.0 + 63.5) = 5.0 mm. This is computed from params, not from meshes, but my mesh bounds confirm PINCH_Y. **53rd-card probe row:** there is **no such row** in cad_cardpath.py. I checked it by hand: `rackNoShelfAtLoadMm()` = rackEjectMm(51) = rack 0. At rack 0 the rack top plate ends at **z 124.0**, and Z_LOAD is 126.7. So the probed card slides **2.7 mm above** the rack top. FIRMWARE.md:371-379 says "at worst its leading edge touches the rack top plate" and that the top plate "covers the load station". Both statements are false. The probe still works: the beam (Y -17) trips while the card tail is held by the feed rollers (Y -28), so reverse is possible. | Low: correct FIRMWARE.md:371-379 (the card front can overrun into the free space above the rack, not onto the top plate), or add a probe row to cad_cardpath.py that checks it. |
| 5 | M3 insert / screw clearances; tolerances from TOL | **FAIL** | Now present: `fit_free/slide/press/snug`, `GAP = TOL`, `M3_HEAD_CB` (params.scad:84-92), used in 20 files. Insert walls are >= 1.2 (see #2). The lint passes, but it only catches `d = N.N` and `<X>_D ± .N`. My own grep of the part files finds fit clearances that are still bare literals and do not follow TOL: `battery_door.scad:8-9` (±0.2 door-to-rebate), `rack_top_plate.scad:5` / `rack_bottom_plate.scad:6` (-0.1 plate-to-rebate), `ui_bezel.scad:9-15` (0.4 / 0.2 bezel-to-notch), `slot_panel.scad:11,16` (0.31 / 0.3 panel-to-shell), `turret_shell.scad:15` (±0.2 panel notch), `servo_mount.scad:12` (±0.3 servo window), `top_cap.scad:34-35` (0.2 / 0.5 cap rim), `base_deck.scad:8` (0.3 deck-in-base), `loader_frame.scad:41` (0.2 N20 face), `launcher_frame.scad:17` (0.1). If someone changes TOL, about 12 mating fits do not follow. The cycle-1 required fix was "derive **every** fit clearance from TOL". | Replace these with `GAP` / `fit_*` (or named params built on TOL). Widen the lint, e.g. flag `[+-] 0.[0-9]` inside box_at / rbox_at / ring / cylinder arguments, or require a `// geom` tag. |
| 6 | Battery fits and is removable, or charge jack reachable; switch and buttons reachable | **FAIL** | Reachable: the DC jack (40°) and rocker (140°) are on the static base wall and shown in `assembly_base_power.png` (cycle-1 L3 fixed). GO, SET, LED and OLED are in the flush bezel (`assembly_back_ui.png`). **But the battery door cannot be fastened** (H-C): `DOOR_SCREWS` x = ±77.8, while the door flange ends at x ±76.6 and the base rebate at ±76.8. `trimesh.contains` on base_shell.stl at (77.8, 20, z 0.3 / 0.9) returns **solid**. There is no clearance or CSK hole through the 1.2 mm floor, and the floor insert hole only starts at z 1.2. The door's own `m3_csk` lies 1.2 mm off the door edge, so only a 2 mm sliver of the head cone would overlap the door. The CSK is 1.7 deep but the flange is 1.2 thick, so even a drilled screw would stand about 0.2 mm proud of the underside. The battery drops out downward, so the door is what holds it in. | Move DOOR_SCREWS inward onto the flange (e.g. x ±74.7, i.e. CR_XI + DOOR_REBATE/2), or widen the flange. Make the flange (or the screw boss) >= CSK head depth, or use a pan head in a counterbore. Model the 4 door screws in the assembly so the interference check covers them. |
| 7 | Size within Ø200 / 220; README with stretch gap; size pass documented | **PASS** | `tools/cad_size.py`: **Ø199.00, H 207.50**, PASS, 8 angles x 2 rack positions. **Independent check:** I exported the whole `assembly.scad` as one mesh at turret/rack 0/0, 0/81, 37/40.5, 123/13.5, 200/81 and 333/0. All six give bbox [-99.5,-99.5,0]..[99.5,99.5,207.5], max r **99.500**, height 207.5. Protrusion check: the per-part max radius is 99.50 for the shells, cap, turntable, slot_panel and bezel. Rocker 99.28 and buttons 99.15 sit inside the skin; the jack nut is inside. The modelled fasteners (deck, nut-arm) are internal. Shell-tab screws are fitted from above under the cap, slot-panel screws from inside, door screws are CSK on the underside. No feet exist. So nothing pokes out beyond 99.5. The real 1.0 mm diameter margin is intact, except that a mating DC plug sticks out in use (normal). README:36-52 shows Ø199.0 (+1.0 margin, +29.0 stretch gap) and H 207.5 (+12.5, +27.5). D-17 has the size table including the D-18 step. | LOW: the D-17 narrative is stale. It says "hopper / loader-frame back corners (r 95.1) … shell 0.5 mm outside them". I measured **r 97.19** for both against shell inner r 97.7. Update the text. |
| 8 | No electronics visible; clean minimal exterior | **PASS** | `assembly_front_exterior.png` (exit side), `assembly_back_ui.png` (UI side), `assembly_top_closed.png` and `assembly_base_power.png`: smooth cylinder, chamfered cap, flush lid, flush bezel/OLED, flush rocker and jack. No PCBs or wires are visible. Through the 6 mm slot you can see the launcher wheel, the 130 motor and the edge of the exit IR sensor. That is acceptable for a card exit. The render names now match their content (cycle-1 L2 fixed). | LOW: the slot_panel seams (a 92 x 76 mm rectangle under the slot) are visible in `assembly_base_power.png`. Consider a tighter or chamfered seam or a contrasting colour, so it reads as intended. |
| 9 | Renders complete; parts visible, placed, coloured, nothing floats; mechanical plausibility | **FAIL** | **Looked at:** all 14 top-level PNGs and all 33 `renders/parts/*.png` (in 3x3 montages; each shows one plausible single part matching its name, nothing garbled), plus 4 GIF frames (0/90/180/270°). `exploded.png` is now fully in frame: battery door, base shell, bezel, buttons, ESP32, drivers, bearing, turntable, loader frame, rack, rail frame, shell, cap and lid (cycle-1 (a) fixed). Section views agree with #4: eject card (red) at the nip height, load card (blue) at the hopper floor, blade, pinion and gear rack, wheel and idler with spring. Present and plausible: hopper + weight, N20 + feed rollers + pinch pair (D-18), MG90S + m1 rack and pinion, 130 motor + wheel + spring idler, lazy susan + pancake NEMA + GT2 20T + belt ring, hall post + magnet, both IR beams, microswitch, OLED bezel, rods + LM8UU, NEMA17 T8 + nut arm. **Two HIGH plausibility defects:** **H-A**, the pinch rollers counter-rotate against the card; **H-B**, the ribbon clock spring cannot physically fit (details below). | Fix H-A and H-B, and re-render the affected views. |
| 10 | params.scad ↔ firmware consistency | **PASS** | `python3 tools/check_consistency.py`: PASS (35 values, config.h includes geometry.h, no re-typed values). `python3 tools/gen_geometry.py --check`: "geometry.h up to date (35 values)", rc 0. PUSH_* and BURN_DEG are now @shared and present in geometry.h:31-40. | None. |

## Cycle-1 findings: fixed?

| Cycle-1 item | Status | Evidence |
|---|---|---|
| H1 burn drawer cannot catch a card | **Resolved by deviation D-20 (accepted, see note)** | Drawer removed; fixed `slot_panel`. The cardpath exit sweep (85 steps) touches nothing but the nip; slot 1.95 per side and 1.5 above/below. README "Known deviations" lists it. **Judgement:** the physics argument is right: a pinched card cannot release inside the 33 mm free path, as cycle 1 itself showed. Throwing the burn face down onto a table spot is how a human dealer does it. BURN_DEG 22 sits between the board fan (0-16) and seat 0 (>= 26), and the spec's intent (burn visible and kept apart) is met. Accepted as a documented spec deviation. **Residual risk (MED):** at release the card's centre of mass is about 2.7 mm inside the slot lip (tail Y 65 + 31.75 = 96.75 vs skin 99.5). If LAUNCH_BURN is too low, the card can stop balanced in the slot with its tail on the floor lip, and the next throw jams. BUILD.md calibration should set LAUNCH_BURN by "card fully clears the slot every time over N trials", not only "lands on the burn spot". |
| H2 no pinch roller | **Implemented (D-18), but new HIGH H-A** | Pinch pair is in the gate, 0.15 squeeze, coast 5.0 mm. Its drive direction is wrong (see H-A). |
| H3 harness blocked by the battery | **Re-designed (D-19), but new HIGH H-B** | Deck radial slot plus clock spring in the bearing ID (`base_electronics.png`). The ribbon width is wrong for the space (see H-B). |
| H4 no harness exit from the loader-frame box | **Fixed** | Windows in loader_frame (-X / +X), visible in `assembly_open_iso.png`, `harness_route.png` and the loader_frame part render. The wires themselves are not modelled in the turret (LOW). |
| M1 sub-1.2 insert walls | Fixed | nut arm uses thread-forming screws in 2.5 mm pilots (min wall 1.46); launcher_floor 1.20. |
| M2 loader_frame 2.9 mm insert pockets | Fixed | `insert_hole()` at full M3_INSERT_L on 9x9x7.2 bosses (loader_frame.scad:32,56). |
| M3 base_shell rebate called a bridge | Fixed | Documented as a 4 mm cantilever (parts.csv, printability OVERHANG_OK). |
| M4 rack_motor_mount 1.0 seat / holed bridge | Fixed | Seat 1.2, printed upside down, solid 43 mm bridge (1198 mm²). |
| M5 TOL unused | **Partly fixed, still FAIL** | See criterion 5: about 12 fits are still literals. |
| M6 SERVO_SWEEP not tied to firmware | Fixed | PUSH_* are @shared; geometry.h:36-40. |
| M7 shell only removable from below | Fixed | Shell anchors are CSK-fixed to the turntable; shell tabs are screwed from above under the cap. The shell lifts straight up: its front notch is open at the bottom, so it clears the slot_panel. BUILD.md §9 has jam clearing. |
| L1 raw slivers | Fixed | Export reports 0 slivers for every part. |
| L2 render names swapped | Fixed | front = exit slot, back = UI. |
| L3 no DC jack render | Fixed | `assembly_base_power.png`. |
| L4 no ALLOWED cap | Fixed | Caps present (nut-screw cap loose, LOW). |
| L5 fasteners not modelled | **Open** | Still only deck inserts/screws and nut-arm screws. This hid H-C. |
| L6 small sub-1.2 features | Fixed | Not found by my 80k-sample check, except the turntable CSK rim (inherent). |
| L7 stale STLs | Fixed | `export_placed` purges unregistered parts; no leftovers in `cad/build/placed_r0_t0`. |

## Other defects

### HIGH

* **H-A Pinch rollers are driven against the card (D-18).** `vendor/o_ring_belt.scad` is an **open** loop
  (hull of both pulleys, not crossed) between the feed shaft (Y -28, z 119.7, rollers *under* the card) and
  the pinch shaft (Y -16, z 130.35, rollers *above* the card). An open belt turns both shafts the **same** way.
  When the feed roller top moves the card toward +Y, the pinch roller bottom surface moves toward **-Y**, about 3 %
  faster. The two drives then fight over the 12 mm overlap: the card stalls, buckles, or is pulled back.
  DECISIONS D-18 and BUILD.md step 12 ("Turning the feed shaft by hand must turn the pinch rollers too") do not
  mention crossing. cad_cardpath.py checks only the speed *magnitude* (1.034) and not the direction.
  **Fix:** reverse the pinch shaft. Options: a crossed (figure-8) O-ring with a check that the crossing point
  clears (centre distance 16 mm); a pair of meshing gears or an idler gear; or passive spring-loaded pinch
  idlers over a driven lower roller at the throat. Add a direction check to cad_cardpath.py and BUILD.md.
* **H-B Ribbon clock spring cannot fit (D-19).** BOM, BUILD and D-19 specify a **20-way 1.27 mm** ribbon. That is
  **25.4 mm wide**. The vendor model draws it **12 mm** wide (`RIBBON_W = 12`, ribbon_cable.scad comment
  "12 mm wide"). A clock spring winds the ribbon on edge, so the coil height must be at least the ribbon width. The model's coil is
  **7 mm** tall (SPOOL_H, z 33-40), and the bearing ID only offers about 9.5 mm between the deck top and the turntable.
  It does not fit at 25.4 mm or at 12 mm. Winding it flat (width radial) would need in-plane bending, which is impossible, and the radial room is
  only 10.5 mm anyway. The Ø16 turntable hole also has to pass a 25 mm ribbon.
  **Fix:** choose a harness that fits the 9.5 mm space and carries the currents, then re-model and re-render it. Options:
  a capsule slip ring through or under the bearing; a narrower conductor set (e.g. 0.5 mm-pitch FFC on edge ≤ 9 mm wide,
  with the current budget re-checked); or more height in the base for the coil. Correct the BOM, D-19 and the vendor model.
* **H-C Battery door cannot be screwed on** (criterion 6). Screw axes at x ±77.8 run through solid base floor
  (rebate edge 76.8, door edge 76.6). No hole is modelled. The CSK is deeper than the 1.2 mm flange. Details and fix under #6.

### MEDIUM

* M-1 Fit literals not derived from TOL (criterion 5 list).
* M-2 Burn release margin (see the H1 note): about 2.7 mm of centre-of-mass travel is still needed after the nip
  releases. LAUNCH_BURN must guarantee the card clears the slot; add that to the calibration step and, ideally,
  to the firmware fault logic. The exit IR beam (Y 56) is before the nip, so it cannot see a card stuck in the slot.

### LOW

* L-1 FIRMWARE.md:371-379 says the 53rd-card probe card meets the rack top plate. In fact it passes 2.7 mm above it (rack top 124.0 vs Z_LOAD 126.7).
  There is no probe row in cad_cardpath.py.
* L-2 D-17 envelope-driver text is stale (r 95.1 / 94.6). Measured: hopper and loader frame r 97.19, rack NEMA r 94.60, against shell inner r 97.7.
* L-3 Turntable CSK rims leave 0.8 mm feather edges at the shell-anchor holes (inherent to the CSK; acceptable, note it in parts.csv).
* L-4 Printability overhang documentation matches by area only; the wall check uses 12 000 random samples.
* L-5 Interference: overlaps under 1 mm³ pass without being listed. The nut-screw cap is 1.47x the measured volume.
* L-6 Turret wiring above the turntable is not modelled. Only the windows exist, so the route can only be judged from the text.
* L-7 Fasteners are still mostly unmodelled (this is how H-C went unnoticed).

## Commands run

```
python3 tools/cad_export.py            # 69/69 PASS, 0 slivers
python3 tools/cad_printability.py      # 33/33 PASS
python3 tools/cad_interference.py      # rack 0: 83 parts/272 pairs, rack 81: 82/271, 0 FAIL
python3 tools/cad_cardpath.py          # 34/34 PASS
python3 tools/cad_size.py              # D 199.00, H 207.50 PASS
python3 tools/check_consistency.py     # PASS (35 values)
python3 tools/gen_geometry.py --check  # up to date
openscad --backend=manifold -o full_tX_rY.stl -D turret_deg=X -D rack_pos=Y cad/assembly.scad
    # 6 configs -> bbox ±99.5 x 207.5, max r 99.500
own scripts: 80k-sample wall thickness on 28 parts + local probes; comb_nx fin/gap ray cast at 3 y;
    placed-part bounds (nip, floor, idler, wheel, pinch, rack top at rack 0); per-part max radius vs shell;
    base_shell / battery_door point-containment at the door screw axes;
    3x3 montages of all 33 part renders; 4 GIF frames
grep: fit literals in cad/*.scad; ribbon spec in BOM/BUILD/vendor; o_ring_belt geometry
```
