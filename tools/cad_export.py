#!/usr/bin/env python3
"""Export every printed part (print orientation) to cad/stl/<part>.stl and every vendor
model to cad/stl/vendor/<name>.stl, then check each mesh:
  * OpenSCAD render succeeds without warnings
  * trimesh: watertight + is_volume, single body (printed parts)
  * admesh: no edges fixed / facets removed / added / reversed, no backwards edges
  * printed parts: sit on the bed (min z = 0) and fit 220 x 220 x 250
  * info: downward-facing area (overhang > 50 deg from vertical, bed faces excluded)
Exit 1 on any FAIL.   Usage: python3 tools/cad_export.py [--only part ...]
"""
import argparse
import concurrent.futures as cf
import re
import subprocess
import sys

import numpy as np
import trimesh

sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parent))
from cad_common import CAD, JOBS, clean_stl, printed_parts, run_scad, table, vendor_parts  # noqa: E402

BED = (220.0, 220.0, 250.0)
STL = CAD / "stl"


def admesh_check(path):
    p = subprocess.run(["admesh", str(path)], capture_output=True, text=True)
    stats = {}
    for key in ["Number of parts", "Degenerate facets", "Edges fixed", "Facets removed", "Facets added",
                "Facets reversed", "Backwards edges", "Normals fixed"]:
        m = re.search(re.escape(key) + r"\s*:\s*(\d+)", p.stdout)
        stats[key] = int(m.group(1)) if m else -1
    bad = {k: v for k, v in stats.items()
           if k not in ("Number of parts", "Degenerate facets", "Normals fixed") and v != 0}
    return stats, bad


def overhang_area(m):
    n = m.face_normals
    c = m.triangles_center
    a = m.area_faces
    down = n[:, 2] < -np.sin(np.radians(50))
    on_bed = c[:, 2] < 0.05
    bridge = n[:, 2] < -0.999
    return float(a[down & ~on_bed & ~bridge].sum()), float(a[bridge & ~on_bed].sum())


def check(kind, name):
    src = CAD / (f"vendor/{name}.scad" if kind == "vendor" else f"{name}.scad")
    out = (STL / "vendor" if kind == "vendor" else STL) / f"{name}.stl"
    out.parent.mkdir(parents=True, exist_ok=True)
    raw = CAD / "build" / "raw" / out.relative_to(STL)
    raw.parent.mkdir(parents=True, exist_ok=True)
    rc, log = run_scad(["-o", raw, src])
    fails = []
    if rc != 0:
        return dict(kind=kind, name=name, fails=["openscad failed: " + log[-300:]])
    try:
        removed, f0, f1 = clean_stl(raw, out)
    except Exception as ex:  # not a manifold at all
        return dict(kind=kind, name=name, fails=[str(ex)])
    if re.search(r"WARNING|ERROR", log):
        fails.append("openscad warning: " + "; ".join(l for l in log.splitlines() if re.search("WARNING|ERROR", l))[:200])
    m = trimesh.load(out, force="mesh")
    wt, vol = bool(m.is_watertight), bool(m.is_volume)
    bodies = len(m.split(only_watertight=False))
    if not wt:
        fails.append("not watertight")
    if not vol:
        fails.append("not a volume")
    stats, bad = admesh_check(out)
    if bad:
        fails.append(f"admesh {bad}")
    ext = m.extents
    lo = m.bounds[0]
    fit = all(e <= b + 1e-6 for e, b in zip(sorted(ext[:2]), sorted(BED[:2]))) and ext[2] <= BED[2]
    if kind == "printed":
        if bodies != 1:
            fails.append(f"{bodies} bodies")
        if abs(lo[2]) > 0.01:
            fails.append(f"min z {lo[2]:.2f} (not on bed)")
        if not fit:
            fails.append("does not fit bed")
    oh, br = overhang_area(m) if kind == "printed" else (0.0, 0.0)
    return dict(kind=kind, name=name, fails=fails, wt=wt, vol=vol, bodies=bodies, ext=ext, cleaned=removed,
                volume=float(m.volume), admesh=stats, overhang=oh, bridge=br, fit=fit)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", nargs="*")
    a = ap.parse_args()
    jobs = [("printed", p) for p in printed_parts()] + [("vendor", v) for v in vendor_parts()]
    if a.only:
        jobs = [j for j in jobs if j[1] in a.only]
    with cf.ThreadPoolExecutor(JOBS) as ex:
        res = list(ex.map(lambda j: check(*j), jobs))
    rows = []
    for r in res:
        if "ext" not in r:
            rows.append([r["kind"], r["name"], "-", "-", "-", "-", "-", "-", "FAIL"])
            continue
        e = r["ext"]
        rows.append([r["kind"], r["name"], f"{e[0]:.1f}x{e[1]:.1f}x{e[2]:.1f}", f"{r['volume']/1000:.1f}",
                     "yes" if r["wt"] and r["vol"] else "NO", r["bodies"],
                     "yes" if (r["fit"] or r["kind"] == "vendor") else "NO",
                     f"{r['overhang']:.0f}/{r['bridge']:.0f}" if r["kind"] == "printed" else "-", r["cleaned"],
                     "PASS" if not r["fails"] else "FAIL"])
    print(table(rows, ["kind", "part", "bbox mm (print orient.)", "cm3", "manifold", "bodies", "fits bed",
                       "ovh>50/bridge mm2", "slivers", "result"]))
    nfail = sum(1 for r in res if r["fails"])
    for r in res:
        for f in r["fails"]:
            print(f"FAIL {r['name']}: {f}")
    n_p = sum(1 for r in res if r["kind"] == "printed")
    print(f"\n{len(res) - nfail}/{len(res)} PASS  ({n_p} printed parts -> cad/stl/, "
          f"{len(res) - n_p} vendor models -> cad/stl/vendor/)")
    print("slivers = zero-volume sliver shells dropped by the cleanup stage (raw OpenSCAD output in cad/build/raw/)")
    print("ovh>50 = area of faces steeper than 50 deg from vertical (excl. bed faces and flat bridges);"
          " see cad/parts.csv for the per-part support notes.")
    sys.exit(1 if nfail else 0)


if __name__ == "__main__":
    main()
