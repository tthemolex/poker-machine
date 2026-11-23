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

## D-14 Loader: roller feed + throat gate, no separate pinch idler (CAD)
There is no room for a pinch idler between the hopper and the rack: the gate (6.4 mm) and a
0.8 mm running gap are all that fits behind the rack face before the hopper corners reach the
shell. The weighted deck (printed weight + 2 M8 nuts) presses the bottom card on two Ø14 N20
rollers at Y -26. A gate 0.45 mm above the floor passes one card (adjustable ±1.5 mm with
slotted screws). The card leaves the rollers with its front at Y 37.5 and coasts the last
~14 mm into the stop fingers (Z_LOAD, X ±40). The fingers stop it 0.75 mm proud of nominal.
Every other static launcher face is 0.5 mm further out, so a seated card never rubs anything
but the finger tabs. If coasting proves unreliable, raise FEED_PWM or move to a Ø16 roller
(+1 mm under the floor).

## D-15 Burn drawer inside the turret, under the exit (CAD)
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
