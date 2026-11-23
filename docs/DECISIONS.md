# Design decisions & size trade-offs

## D-01 Shelf pitch 1.5 mm (target was 1.6)
Height is set by the rack sweep (≈ 2 × rack height). With the 28 mm battery under the turret,
the base stack puts the rack bottom at z = 44.5. Pitch 1.6 gives ≈219.5 mm total, a 0.5 mm
margin to the 220 mm hard limit, which leaves no room for tolerances. Pitch 2.0 (the draft) gives ≈265 mm.
1.5 mm = 0.9 mm fin + 0.6 mm slot, with a 0.30 mm card, so clearance is 0.30 mm (meets the ≥0.3 criterion).
Fins are a **noted exception** to the 1.2 mm minimum wall: they are short (3 mm) non-structural
ribs, printed as 2 perimeters with the comb lying flat (fins vertical, no overhang). The comb
walls themselves are ≥1.2 mm. Calibrate with the test comb (BUILD.md).

## D-02 Turret ring 200T / 10:1 instead of ~120T / 6:1
The battery (145×85) has to sit centred in the base, so the turret NEMA can only go in a crescent
beside it (r ≈ 70). A 120T ring (pitch Ø76) cannot share a plane with the lazy susan (OD 120) and still
mesh a pinion at r 70 without adding an extra belt layer (+8 mm height, which breaks the 220 mm limit).
A 200T GT2-profile ring (pitch Ø127.3 / r 63.7, just outside the bearing OD r 60) meshes directly with the 20T pulley
at the lazy susan's level and adds no height. 10:1 also gives a finer aim (88.9 steps/°).

## D-03 Lazy susan: 120 mm round aluminium (ID 70, OD 120, 9.5 mm)
This is the smallest bearing in the allowed range that leaves room for the ring and pinion inside the base.

## D-04 Battery placement
TalentCell YB1206000-USB is 145 × 85 × 28 mm (maker manual; verify yours). It fits Ø200 only when
centred, so it lies flat in the base. The stretch target of 180 mm height is not reachable with this battery
and a 52-slot random-access rack (sweep ≈ 160 mm + base 44.5 mm).

## D-05 Board at 8°, not 0°
The turret may not rotate past 0/360 (cable loop) and homes at 0°. With the board at 0°, the fan slots at
−8° and −4° would wrap to 352°/356°, forcing two almost-full turns per flop. With BOARD_DEG = 8 = 2 × spread,
the fan is [0°, 16°]. Seats sit at BOARD_DEG + (s + ½)·360/n, at most 350° for n = 10, so no move ever wraps.

## D-10 Rack motor at the bottom, leadscrew 100 mm (not 150 mm) (CAD)
The NEMA17 with integrated T8 stands face-up on the turntable beside the rack (X -71.5) and the
nut arm sits 48.5 mm up the rack, so the motor body overlaps the rack's sweep in height. A
top-hung motor would add ~40 mm (motor above the rack's highest position, about 250 mm total).
With travel 81 + nut 15 + boss 2.5, the screw needs 98.5 mm, so we specify **100 mm** (order
it, or cut a 150 mm screw). A 150 mm screw would end 15 mm below the turntable top or 50 mm
above the cap. Top of the nut at full travel: 183.5, screw end 185.

## D-11 Turret ring = printed hub + GT2 belt glued teeth-out (CAD, orchestrator decision)
The hub radius is 63.03 (pitch r 63.66 minus the 0.63 belt back), with a 0.6 mm seat lip. The
belt is 6 mm x 400 mm (200 T). The 20T pulley axis sits at r 69.77 (land + pulley OD radius)
on the +Y side, and the pulley's top flange is removed so the belt-ring can overlap it. The
belt teeth and pulley grooves are modelled; the interference check sees no overlap.

## D-12 Rack construction: two identical combs + glued plates; 4.5 mm comb wall (CAD)
Comb fins are 0.9 / 3.0 mm deep (D-01 exception). The card has 0.5 mm end clearance and rests
2.5 mm on each fin. The comb wall is 4.5 mm, not thicker, because the rack NEMA has to fit
between the rack (|X| <= 49.45) and the shell. At 7.5 mm the motor corner would reach r 97.3
and need a Ø200+ shell. That leaves no room for heat-set inserts in the comb, so the carriage
and nut arm use M3 thread-forming screws in 2.5 mm pilot holes. The plates (1.2 mm) are glued
(CA) into rebates. One comb part is used twice (turned 180°), so the hole patterns are symmetric.

## D-13 Pusher blade straddles the card plane (CAD)
A 0.30 mm PVC card supported only at its ends (84 mm span) sags about 0.3–0.4 mm under its own
weight, so its trailing edge sits below the fin plane in the middle. The blade (0.5 mm) runs
at Z_EJECT-0.30..+0.20 in the fin-free centre (X ±12). It covers the edge of a flat card
(0.20 mm overlap) and of a sagged one (0.22 mm), and clears the cards above and below by
1.3 / 0.9 mm (tools/cad_cardpath.py). Stroke: m1 15T pinion x 150° = 19.6 mm vs 16.5 mm needed
(card 2 mm past the nip). Calibrate PUSH_OUT down if the blade over-travels.

## D-14 Loader: roller feed + throat gate, no separate pinch idler (CAD). SUPERSEDED by D-18 (QA cycle 1)
There is no room for a pinch idler between the hopper and the rack: the gate (6.4 mm) and a
0.8 mm running gap are all that fits behind the rack face before the hopper corners reach the
shell. The weighted deck (printed weight + 2 M8 nuts) presses the bottom card on two Ø14 N20
rollers at Y -26. A gate 0.45 mm above the floor passes one card (adjustable ±1.5 mm with
slotted screws). The card leaves the rollers with its front at Y 37.5 and coasts the last
~14 mm into the stop fingers (Z_LOAD, X ±40). The fingers stop it 0.75 mm proud of nominal.
Every other static launcher face is 0.5 mm further out, so a seated card never rubs anything
but the finger tabs. If coasting proves unreliable, raise FEED_PWM or move to a Ø16 roller
(+1 mm under the floor).

## D-15 Burn drawer inside the turret, under the exit (CAD). SUPERSEDED by D-20 (QA cycle 1)
After the nip (Y 65) there is a ~25 mm floor-less gap above a pull-out drawer (front face =
flush patch of the shell). At burn PWM the card leaves the nip slowly and drops tail-first
into the drawer; seat and board throws carry over the gap and out of the 6 mm slot. This
works on paper only: the burn PWM and the gap need testing on the prototype (see known
issues). The drawer slides out radially for clearing burns between hands.

## D-16 Base layout (CAD)
The battery is centred and drops out through a floor door (4x M3). Its DC plugs exit the
cradle at +X. The ESP32, both TMC2209 and the UI sit in the -Y crescent, the Mini560, DRV8833
and pancake NEMA in the +Y crescent. Rocker (140°) and charge jack (40°) are on the back wall,
OLED / GO / SET / LED in a flush front bezel. Flat UI parts are inset 0.6 mm so their corners do
not stand proud of the curved skin. The hall sensor sits on a post inside the bearing ID (r 26),
with the 6x3 magnet in the turntable at 0°. Cables come up through the bearing ID and the Ø16
turntable hole at (0,-24), outside the rack footprint, with a service loop (0..360° only).

## D-17 Size-optimisation pass (CAD)
Measured with tools/cad_size.py (all parts incl. vendor and cards, 8 turret angles, both rack ends).

| step | change | footprint Ø | height |
|---|---|---|---|
| first working assembly | shell/base R 99 | 198.8 (UI parts proud) | 207.5 |
| flush UI | buttons / rocker / jack / LED inset 0.6 mm | 198.0 | 207.5 |
| shrink shell to the drivers | TURRET_R 99 -> 97.5; hopper corner r 4.0 -> 4.4 | **195.0** | 207.5 |
| keep cap clear | hopper top 2 mm lower (clears the cap's inner rim chamfer) | 195.0 | 207.5 |
| QA cycle 1: pinch pair at the throat (D-18) | gate 6.4 -> 9.4, hopper wall 1.6 -> 1.2, card Y clearance 1.0 -> 0.8, TURRET_R 97.5 -> 99.5 | **199.0** | 207.5 |

What sets the size now:
* Radius: the hopper / loader-frame back corners (r 95.1), set by card length + gate 6.4 + rack
  back face. Then the rack NEMA corner (r 94.6), set by comb wall 4.5 + 0.9 gap + motor
  42.3. The shell inner face is 0.5 mm outside them and the wall is 1.8 mm. Going smaller needs
  a narrower motor (e.g. NEMA14 + T8) or a hopper that does not sit fully behind the rack.
* Height: rack sweep. The rack top reaches 205.0 at full travel, then 0.5 mm clearance and the
  2 mm cap. The only CAD-owned levers left are under 1 mm (cap 2 -> 1.6). The real lever is
  @shared: STATION_OFFSET 3 -> 2 pitches (-1.5 mm) is possible if the hopper floor doubles as the
  blade-guide roof. It is proposed to the orchestrator and not applied.
* Stretch target Ø170 / 180 is not reachable with this battery (corner r 84) and a
  52-slot random-access rack (sweep about 160 mm + base 44.5 mm). Gap: +25.0 mm Ø, +27.5 mm height.

## D-18 Loader pinch pair at the throat (QA cycle 1, replaces D-14) (CAD)
QA rejected D-14: the coast was too long (about 15 mm). Two driven Ø7 rubber rollers (X ±26) now sit in the gate,
right behind the rack face (roller face 0.5 mm behind it, Y -16.0). They press the card onto the
hopper floor with 0.15 mm squeeze. Their shaft floats in vertical slots, held down by 2 springs. The
pinch shaft is driven from the feed shaft by a 1.5 mm O-ring belt on the -X side (pulleys Ø12 / Ø5.8),
so the pinch surface runs 3 % faster than the feed rollers and the card stays taut. The card now
leaves positive drive with its front at Y 47.5 and coasts **5.0 mm** (was ~15) into the stop
fingers (tools/cad_cardpath.py). The feed rollers moved to X ±34, Y -28 so they clear the pinch
and the IR beam. The load IR beam moved to X 38, Y -17 (outside the rollers and the shaft).
The belt plane (X -52) sits outboard of the -X comb, and the nut arm was narrowed on that side.
Size trade-off: the gate grew from 6.4 to 9.4 mm. To get most of that back, the hopper wall went to
1.2 and the hopper Y clearance to 0.8. The hopper corners still moved out 2 mm, so the shell went from
R 97.5 to 99.5: **Ø195.0 -> 199.0** (hard limit 200, 1.0 mm margin). Height is unchanged.

## D-19 Harness route: ribbon clock spring through the bearing ID (QA cycle 1) (CAD)
The battery fills the whole bearing ID (deck underside 0.4 mm above it), so a round bundle cannot
leave the bearing centre. The harness is a 20-way 1.27 mm flat ribbon wound 3 turns as a clock spring
around a static hub (r 7..20, z 33..40) inside the bearing ID. That gives 360° of rotation with no
twisting.
* The inner end goes down beside the hub. It runs along a 14 mm radial slot in the deck (X -21..-7),
  lying flat on the battery top (2.4 mm of room under the bearing plate), and drops over a notch in
  the cradle wall into the -Y crescent at the ESP32.
* The outer end rises through the Ø16 turntable hole (0,-24) into the loader-frame box.
* From there it fans out through two wall windows: -X to the rack NEMA and the belt side, +X along
  the rail frame to the microswitch, then forward between the rail frame and the shell to the
  launcher. Loader, pusher and load IR wiring stay inside the box.
* Stepper and motor currents use 2-3 conductors in parallel (AWG28 ribbon is about 1 A per wire).
* The hall sensor moved to r 29.5 to clear the coil.
* Shown in renders/base_electronics.png (coil, hub and slot run) and renders/assembly_open_iso.png
  (loader-frame window).

## D-20 Burns are thrown onto the table, not into a turret tray (QA cycle 1, replaces D-15) (CAD + firmware)
A pinched card moves in a straight line at any speed until its tail leaves the nip. The nip cannot
be closer than Y 61-65 (the wheel has to clear the rack sweep), so at release the card's centre of
mass is at the slot lip and 30 mm of the card is outside the shell. An internal pocket needs at least
63.5 mm of free path after the nip, and we have about 33 mm. A deflector cannot pick out slow cards,
an external tray needs about Ø225, and anything else needs a second actuator. So the burn card is
thrown face down at the lowest PWM (LAUNCH_BURN) onto a burn spot on the table just in front of the machine,
at the shared BURN_DEG = 22°, between the board fan (0–16°) and seat 0 (≥ 26° for n = 10).
That is what a human dealer does, it keeps the burn visible to players, and it has no geometric failure mode.
Rejected: burn in place (mark the shelf burned). It is statistically equivalent but has no visible burn.
Rejected: a servo flap and chute into a drawer: an extra actuator, pin and failure point.
**This is a deviation from the spec** ("burn tray mounted on the turret") and is listed in README. The drawer is gone: a fixed
flush **slot_panel** closes the shell under the 6 mm exit slot. tools/cad_cardpath.py sweeps the eject
card out through the nip and slot (85 steps): nothing touches it except the nip. The slot has 1.95 mm
per side and 1.5 mm above and below the card plane.

## D-21 QA cycle 1 printability / tolerance fixes (CAD)
* All fits derive from TOL: `fit_free` (+2·TOL), `fit_slide` (+TOL), `fit_snug` (+TOL/2),
  `fit_press` (−TOL/4), plus `GAP`. tools/cad_printability.py lints the part sources for bare fit
  literals.
* Walls are ≥ 1.2 mm everywhere except the documented fins and gear tooth tips. This is checked by
  ray casting 12 000 inward samples per part (tools/cad_printability.py). Fixes made:
  - launcher-floor end blocks enlarged; the IR cup has 1.2/1.4 mm walls and a 1.5 mm roof.
  - The T8 nut uses thread-forming screws in 2.5 mm pilots (1.45 mm to the bore) instead of inserts.
  - Loader-frame turntable bosses take full-depth inserts.
  - Motor-mount seat is 1.2 mm.
  - Idler-arm bosses r 3.0.
  - Button bosses behind the flush recesses.
  - Comb pilots are 3.3 mm deep.
  - Carriage counterbores moved between the bearing housings.
  - The gear rack has whole teeth only.
  - The magnet hole goes through the turntable.
  - Thin recess floors removed: feet recesses, door pull, lid pull.
* Every overhang region > 25 mm² is documented in the script with its kind. The base-shell door
  rebate ceiling is a 4 mm cantilever, not a bridge. The motor mount now prints upside down, so its
  seat is one solid 43 mm bridge.
* The shell is now screwed from **above** into four shell anchors (CSK-fixed to the turntable). It
  lifts off with the cap off and the turret stays on the base.
* The pusher angles are @shared: PUSH_HOME_DEG 10, PUSH_SWEEP_DEG 150, PUSH_OUT_DEG 160,
  PUSH_PINION_R 7.5, PUSH_STROKE_MM 19.63 (firmware: geom::PUSH_*).
