#!/usr/bin/env python3
"""Pairwise interference check of the placed assembly (turret 0 deg) with the rack at the
bottom (rack_pos 0) and top (rack_pos RACK_TRAVEL) of its travel, plus optional
intermediate rack positions (--sweep N).

Every part of cad/assembly.scad's registry is exported on its own (-D only=name),
boxes are pre-filtered, then the exact intersection volume is computed with manifold3d.
A pair FAILS if the volume >= TOL_MM3 unless it is listed in ALLOWED with a reason.
Exit 1 on any failure.
"""
import argparse
import itertools
import sys
import time

import numpy as np

sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parent))
from cad_common import BUILD, assembly_parts, export_placed, load_mesh, params, table, to_manifold  # noqa: E402

TOL_MM3 = 1.0

# Intended material overlaps (press fits / heat-set inserts / meshing teeth). Each needs a reason.
ALLOWED = {   # pair -> (reason, max mm3 = ~1.1 x the expected overlap)
    frozenset({"v_deck_inserts", "base_shell"}): ("heat-set inserts (OD 4.6) melted into 4.0 mm holes", 4 * 23.1 * 1.1),
    frozenset({"v_idler_shafts", "idler_arm"}): ("3 mm idler axle pressed into a 2.95 mm hole", 1.3),
    frozenset({"v_mg90s", "pusher_pinion"}): ("pinion pressed onto the 4.8 mm servo spline (4.7 mm bore)", 2.5),
    frozenset({"v_nut_screws", "rack_nut_arm"}): ("M3 thread-forming screws (2.9) in 2.5 mm pilot holes, 6 mm deep", 4 * 1.70 * 6 * 1.1),
    frozenset({"card_load", "v_pinch_rollers"}): ("pinch rollers squeeze the card being fed by 0.15 mm (intended)", 3.0),
}


def bbox_overlap(a, b, pad=0.0):
    return bool(np.all(a[0] <= b[1] + pad) and np.all(b[0] <= a[1] + pad))


def run_config(parts, rack_pos):
    names = [n for n, _ in parts]
    paths = export_placed(names, rack_pos, 0)
    meshes, mans, boxes = {}, {}, {}
    for n in names:
        if paths[n] is None:
            continue
        m = load_mesh(paths[n])
        if len(m.faces) == 0:
            continue
        meshes[n] = m
        boxes[n] = m.bounds
    for n, m in meshes.items():
        mans[n] = to_manifold(m)
    results = []
    for a, b in itertools.combinations(sorted(meshes), 2):
        if not bbox_overlap(boxes[a], boxes[b]):
            continue
        vol = (mans[a] ^ mans[b]).volume()
        results.append((a, b, vol))
    return results, len(meshes)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sweep", type=int, default=0, help="also check N intermediate rack positions")
    a = ap.parse_args()
    P = params()
    travel = P["RACK_TRAVEL"]
    positions = [0.0, travel]
    if a.sweep:
        positions += [round(travel * (k + 1) / (a.sweep + 1) / P["SHELF_PITCH"]) * P["SHELF_PITCH"] for k in range(a.sweep)]
    parts = assembly_parts()
    t0 = time.time()
    nfail = 0
    summary = []
    for pos in positions:
        res, nparts = run_config(parts, pos)
        rows, fails = [], 0
        for x, y, vol in res:
            key = frozenset({x, y})
            if vol < 1e-3 and key not in ALLOWED:
                continue
            if vol < TOL_MM3:
                verdict = "PASS (contact)"
            elif key in ALLOWED and vol <= ALLOWED[key][1]:
                verdict = f"ALLOWED (cap {ALLOWED[key][1]:.1f}): " + ALLOWED[key][0]
            else:
                verdict = "FAIL"
                fails += 1
            rows.append([x, y, f"{vol:.3f}", verdict])
        print(f"\n=== turret 0 deg, rack_pos {pos:g} mm: {nparts} parts, {len(res)} bbox-overlapping pairs checked ===")
        if rows:
            print(table(rows, ["part A", "part B", "overlap mm3", "result"]))
        print(f"-> {fails} FAIL, {sum(1 for r in rows if r[3].startswith('ALLOWED'))} allowed, "
              f"{len(res) - fails} pairs below {TOL_MM3} mm3")
        summary.append((pos, nparts, len(res), fails))
        nfail += fails
    print("\nSUMMARY")
    print(table([[f"{p:g}", n, c, f, "PASS" if f == 0 else "FAIL"] for p, n, c, f in summary],
                ["rack_pos", "parts", "pairs checked", "fails", "result"]))
    print(f"allowed intended overlaps: {len(ALLOWED)} (listed with reasons above); tolerance {TOL_MM3} mm3;"
          f" {time.time() - t0:.0f} s")
    sys.exit(1 if nfail else 0)


if __name__ == "__main__":
    main()
