#!/usr/bin/env python3
"""Printability check of every printed STL (cad/stl/*.stl, print orientation; run
tools/cad_export.py first).

1. Wall thickness: ~N samples per part, uniformly on the surface. From each sample a ray
   goes inward along -normal; the first hit whose face is anti-parallel (dot < -0.9) gives
   the local wall thickness. Samples < MIN_WALL fail unless covered by a documented
   exception (THIN_OK: part -> (min allowed, reason)).
2. Overhangs: faces steeper than 50 deg from vertical (n_z < -sin 50), not on the bed, are
   grouped into connected regions. Regions > 25 mm2 must be documented in OVERHANG_OK with
   their kind (bridge / cantilever / hole roof) and a reason.
3. Fit lint: printed-part sources must not use bare fit literals (e.g. "d = 12.2",
   "SHAFT_D + 0.3", or any literal < 1 mm); fits come from TOL via fit_free / fit_slide /
   fit_snug / fit_press / GAP / BLADE_CLR. Pure geometry lines carry an explicit "// geom".
Exit 1 on any FAIL.
"""
import pathlib
import re
import sys

import numpy as np
import trimesh

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from cad_common import CAD, params, printed_parts, table  # noqa: E402

N = 12000
SEED = 1
OVH_SIN = np.sin(np.radians(50))
REGION_MM2 = 25.0

THIN_OK = {
    "rack_comb": (0.40, "shelf fins 0.9 mm incl. their 45 deg lead-in chamfers at the open faces (D-01 exception)"),
    "pusher_pinion": (0.45, "m1 involute tooth tips"),
    "pusher_slider": (0.45, "m1 gear-rack tooth tips"),
    "turntable": (0.75, "feather edge where the CSK cone meets the 2.5 mm plate (inherent to countersunk screws)"),
    "loader_gear_feed": (0.25, "m0.5 involute tooth tips"),
    "loader_gear_pinch": (0.25, "m0.5 involute tooth tips"),
    }
# part -> list of (kind, max area mm2, reason). A region is accepted if some entry's area >= region area.
OVERHANG_OK = {
    "base_shell": [("cantilever", 2600, "battery-door rebate ceiling: 4 mm horizontal cantilever ring at z 1.2 (PETG prints <= 5 mm cantilevers cleanly)"),
                   ("hole roof", 60, "13 mm rocker cut-out and M8 jack hole roofs in the vertical wall")],
    "turntable": [("cantilever", 260, "GT2 belt seat lip: 0.6 mm outward ledge on the hub (prints as a trivial overhang)")],
    "rack_motor_mount": [("bridge", 1300, "seat: one solid 43 mm bridge between the legs (no holes in it)")],
    "rail_frame": [("hole roof", 120, "Ø8.1 rod bores and 2 mm switch-pin holes lying horizontal")],
    "rack_carriage": [("hole roof", 420, "horizontal M3 screw counterbores (Ø6.2) through the carriage plate")],
    "pusher_guide": [("bridge", 3500, "blade-channel roof and floor: bridges <= 28 mm between the guide walls")],
    "loader_frame": [("bridge", 420, "N20 column roof and harness-window tops: bridges <= 18 mm"),
                     ("hole roof", 60, "feed-shaft bore")],
    "launcher_frame": [("hole roof", 160, "130-motor cheek holes (bridge 20 mm) and M3 holes")],
    "launcher_floor": [("hole roof", 60, "pivot / insert holes")],
    "hopper_body": [("hole roof", 60, "slotted gate / pinch-shaft holes in the walls")],
    "hopper_gate": [("bridge", 200, "pinch-roller cavity roofs (8 mm) printed on the back face"), ("hole roof", 120, "pinch-shaft slot, insert holes")],
    "hopper_weight": [("bridge", 220, "finger-loop slot roof (bridge 4 mm)")],
    "top_cap": [("cantilever", 1400, "hopper-lid ledge: 1.5 mm inward lip"), ("bridge", 1400, "tab tops")],
    "hopper_lid": [("bridge", 200, "finger slot ends")],
    "battery_door": [("bridge", 220, "flange step at the plug edge (0.4 mm)")],
    "ui_bezel": [("hole roof", 140, "radial 12 mm button and 5 mm LED holes, OLED window top (23 mm bridge)")],
    "slot_panel": [("hole roof", 40, "insert holes")],
    "idler_arm": [("hole roof", 40, "pivot / axle holes")],
    "rack_nut_arm": [("hole roof", 60, "horizontal M3 holes in the comb flange")],
    "servo_mount": [("hole roof", 40, "servo window top")],
    "turret_shell": [("hole roof", 40, "screw holes in the anchor tabs")],
    "loader_coupler": [("hole roof", 20, "grub holes")],
    "hall_post": [("bridge", 40, "lead groove")],
    "base_deck": [("hole roof", 120, "standoff counterbores")],
    "shell_anchor": [("bridge", 40, "insert hole ends")],
    "spool_hub": [("bridge", 60, "ribbon slit roof")],
    "loader_gear_feed": [], "loader_gear_pinch": [], "bearing_spacer": [],
}
FIT_LINT = [re.compile(r"\bd\s*=\s*\d+\.\d+"),            # bare diameters
            re.compile(r"_D\s*[+-]\s*\d*\.\d+"),             # <PARAM>_D +- literal
            re.compile(r"(?<![\w.])0?\.\d+(?![\w.])")]        # any literal below 1 mm (fits must use TOL / GAP / fit_*)
LINT_OK = re.compile(r"//\s*geom\b")   # mark a non-fit geometric literal with "// geom"


def wall_samples(m):
    pts, fi = trimesh.sample.sample_surface_even(m, N, seed=SEED)
    n = m.face_normals[fi]
    org = pts - n * 1e-3
    loc, ray_i, tri = m.ray.intersects_location(org, -n, multiple_hits=False)
    t = np.full(len(pts), np.inf)
    good = np.einsum("ij,ij->i", m.face_normals[tri], n[ray_i]) < -0.9
    d = np.linalg.norm(loc - org[ray_i], axis=1)
    t[ray_i[good]] = d[good]
    return pts, t


def overhang_regions(m):
    nz = m.face_normals[:, 2]
    c = m.triangles_center
    sel = np.where((nz < -OVH_SIN) & (c[:, 2] > 0.05))[0]
    if len(sel) == 0:
        return []
    selset = set(sel.tolist())
    adj = [e for e in m.face_adjacency if e[0] in selset and e[1] in selset]
    comps = trimesh.graph.connected_components(adj, nodes=sel, min_len=1)
    out = []
    for comp in comps:
        comp = np.asarray(list(comp))
        area = float(m.area_faces[comp].sum())
        flat = float(np.mean(nz[comp] < -0.99))
        z = float(c[comp][:, 2].mean())
        ext = np.ptp(c[comp][:, :2], axis=0) if len(comp) > 1 else np.zeros(2)
        out.append(dict(area=area, flat=flat, z=z, ext=ext))
    return out


def lint(part):
    bad = []
    for i, line in enumerate((CAD / f"{part}.scad").read_text().splitlines(), 1):
        code = line.split("//")[0]
        if LINT_OK.search(line):
            continue
        for rx in FIT_LINT:
            for mm in rx.finditer(code):
                bad.append(f"{part}.scad:{i}: '{mm.group(0)}'")
    return bad


def main():
    P = params()
    minw = P["MIN_WALL"]
    rows, fails = [], []
    for part in printed_parts():
        m = trimesh.load(CAD / "stl" / f"{part}.stl", force="mesh")
        pts, t = wall_samples(m)
        floor, why = THIN_OK.get(part, (minw, ""))
        thin = t < minw - 0.05
        hard = t < floor - 0.05
        tmin = float(t[np.isfinite(t)].min())
        frac = 100.0 * thin.mean()
        wall_ok = not hard.any() if part in THIN_OK else not thin.any()
        if not wall_ok:
            where = pts[hard if part in THIN_OK else thin][:3].round(1).tolist()
            fails.append(f"{part}: wall {tmin:.2f} mm < {floor if part in THIN_OK else minw} at {where}")
        regs = [r for r in overhang_regions(m) if r["area"] > REGION_MM2]
        undoc = []
        docs = OVERHANG_OK.get(part, [])
        for r in regs:
            if not any(r["area"] <= a for _, a, _ in docs):
                undoc.append(r)
        for r in undoc:
            fails.append(f"{part}: undocumented overhang region {r['area']:.0f} mm2 at z {r['z']:.1f} (extent {r['ext'][0]:.1f} x {r['ext'][1]:.1f})")
        lb = lint(part)
        fails += [f"fit literal {b}" for b in lb]
        rows.append([part, f"{tmin:.2f}", f"{frac:.2f}%", why or "-", len(regs),
                     f"{max([r['area'] for r in regs], default=0):.0f}", len(undoc), len(lb),
                     "PASS" if wall_ok and not undoc and not lb else "FAIL"])
    print(table(rows, ["part", "min wall", "<1.2 samples", "documented thin exception", "ovh regions>25", "largest mm2",
                       "undocumented", "fit literals", "result"]))
    for f in fails:
        print("FAIL", f)
    print("\nDocumented overhangs (kind, reason):")
    for p, docs in OVERHANG_OK.items():
        for k, a, r in docs:
            print(f"  {p:20s} {k:10s} <= {a:5.0f} mm2  {r}")
    nf = sum(r[-1] == "FAIL" for r in rows)
    print(f"\n{len(rows) - nf}/{len(rows)} PASS  ({N} wall samples per part; MIN_WALL {minw}; overhang > 50 deg regions > {REGION_MM2:g} mm2 must be documented)")
    sys.exit(1 if nf else 0)


if __name__ == "__main__":
    main()
