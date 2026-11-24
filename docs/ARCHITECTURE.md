# Architecture & layout brief (orchestrator-owned)

> **Status note (after QA cycles 1–2):** this is the original layout brief. Later changes are in DECISIONS.md:
> +6 mm bearing spacer for the harness clock spring (RACK_Z_MIN 44.5 → 50.5, D-19), the loader pinch pair (D-18), burns to a table
> burn spot instead of a turret tray (D-20), the rack motor at the bottom with a 100 mm screw (D-10), and a shell radius of 99.5.
> Measured result: **Ø199.0 × 213.5 mm** (renders/size_report.md). `cad/params.scad` is authoritative for all numbers.

This is the agreed layout that every agent works to. Numbers marked **[P]** live in
`cad/params.scad` (shared block) and are exported to `firmware/include/geometry.h`
by `tools/gen_geometry.py`. Never hard-code them anywhere else.

## Why the layout looks like this (height budget)

A random-access rack sweeps roughly *twice* its own height: any shelf must reach a
fixed station. Sweep = `H_rack + 51*pitch + STATION_OFFSET`.

The battery (TalentCell YB1206000-USB, **145 x 85 x 28 mm** per the maker's manual)
only fits in a Ø200 circle when it is centred, so it lies flat in the base under the turret.
Base stack (bottom up): floor 1.6, battery 28 (+0.4), deck 2.0, lazy susan 9.5,
turntable 2.5, clearance 0.5. That puts the rack's lowest bottom at z = 44.5.

| pitch | sweep | total height |
|---|---|---|
| 2.0 (draft) | ~218 | ~265 (fails) |
| 1.6 (target) | ~171 | ~219.5 (0.5 mm margin, too tight) |
| **1.5 (chosen)** | ~160 | **~209** |

The hard limit is 220 mm, so the final pitch is **1.5 mm** (0.9 mm fin + 0.6 mm gap, card 0.30, so clearance is 0.30).
See DECISIONS.md D-01.

## Frames

* World: Z up, z=0 at the underside of the base. The turret axis is the Z axis.
* Turret frame (rotates with the turret): at turret angle 0°, the launcher shoots along **+Y**.
  This is also the board direction (`BOARD_DEG = 0`).
* Cards lie flat with the long side (88.9) along X and the short side (63.5) along Y.
  Cards move along Y.

## Turret layout (top view, turret frame, mm)

```
 Y=+80  ┌─────── turret shell R≈97 ───────┐
        │  LAUNCHER  (Y 53..80) at Z_EJECT │  exit slot in shell at +Y
        │  burn pocket below it (Y 53..78, │
        │  z 44..~105, inclined)           │
 Y=+52  ├──────────────────────────────────┤ rack front face
        │  RACK 95 x 64 (X±47.5, Y-12..52) │  rods on +X side (2x Ø8, LM8UU)
        │  rack centre at Y=+20             │  T8 screw on -X side at Y=0,
 Y=-12  ├──────────────────────────────────┤  NEMA17 (integrated T8) hangs at top,
        │  PUSHER at Z_EJECT (blade X±12)   │  body centred X≈-70, Y=0
        │  HOPPER at Z_LOAD (Y -13..-80)    │
        │  loader feed + pinch rollers at   │
        │  X≈±32 (X-separated from pusher)  │
 Y=-80  └──────────────────────────────────┘
```

* **Rack**: two side combs (fins 0.9, gap 0.6) joined by a bottom plate and top plate. The card
  centre region is fin-free, so the 0.5 mm steel pusher blade only passes between cards.
* **Stations**: eject station at `Z_EJECT`, load station at `Z_LOAD = Z_EJECT + STATION_OFFSET`
  (4.5 mm = 3 pitches). Both are on the rack's open Y faces. Back face (-Y): loader at Z_LOAD
  (rollers at X≈±32) and pusher at Z_EJECT (blade X±12), separated in X so they do not collide
  despite the small offset. Front face (+Y): launcher nip at Z_EJECT (wheel at X=0). Load-stop
  fingers at Z_LOAD, X≈±38, sit just in front of the rack face so coasting cards stop flush.
* **Rack positions** (rack bottom z relative to `RACK_Z_MIN`, i.e. homed = 0):
  `eject(i) = (51 - i) * pitch`, `load(i) = eject(i) + STATION_OFFSET`. Max travel = 51p + offset.
  Shelf 0 is the bottom shelf.
* **Turret drive**: VXB-style 120 mm round aluminium lazy susan (ID 70, OD 120, 9.5 thick).
  A printed **200T GT2-profile ring** (pitch Ø127.3, r 63.66) hangs under the turntable *outside* the
  bearing OD. It meshes with a 20T GT2 pulley on a static pancake NEMA17 in the base crescent
  (motor centre r≈70 at +Y side of the battery). Ratio 10:1 (not 6:1, see D-02).
* **Hall home**: A3144 on the base deck, magnet in the turntable underside at 0°.
* **Base (static puck, Ø≈197, h≈32)**: battery cradle in the centre (long side along X),
  electronics in the ±Y crescents. Flush GO/SET, LED, OLED, rocker switch and DC charge jack on
  the base wall. The turret NEMA sits in one crescent. Cables go up through the bearing ID
  with a service loop (turret range 0..360° only).
* **Turret shell**: a rotating cylinder R≈97 with a lift-off top cap (access to the rack/pusher
  and hopper lid). The only opening is the card exit slot.

## Electrical

12 V battery feeds the main switch, which feeds TMC2209 VM (x2) and the Mini560 5 V buck.
The 5 V rail feeds ESP32 VIN, the servo, and DRV8833 VM (the DRV8833 max is 10.8 V, so 12 V is not allowed).
The launcher 130 motor and the loader N20 run at 5 V. Pins come from the draft `poker_dealer.ino`
(unchanged) plus OLED I2C SDA=21, SCL=22.
