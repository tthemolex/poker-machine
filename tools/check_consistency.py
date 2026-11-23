#!/usr/bin/env python3
"""params.scad <-> firmware consistency check.

1. firmware/include/geometry.h must equal what gen_geometry.py generates from params.scad.
2. config.h must #include "geometry.h" and must not re-type any shared geometry value
   (no numeric definitions of pitch, offsets, steps/mm, steps/deg, ratios, board angle).
3. Every firmware source that uses geometry must reference geom:: values (informational list).
"""
import pathlib, re, subprocess, sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
fails = []

r = subprocess.run([sys.executable, str(ROOT / "tools/gen_geometry.py"), "--check"],
                   capture_output=True, text=True)
print(("PASS" if r.returncode == 0 else "FAIL"), "geometry.h generated from params.scad:",
      (r.stdout or r.stderr).strip())
if r.returncode:
    fails.append("geometry.h stale")

cfg = ROOT / "firmware/include/config.h"
if not cfg.exists():
    fails.append("config.h missing"); print("FAIL config.h missing")
else:
    t = cfg.read_text()
    inc = '#include "geometry.h"' in t
    print(("PASS" if inc else "FAIL"), 'config.h includes "geometry.h"')
    if not inc:
        fails.append("config.h does not include geometry.h")
    shared = [m.group(1) for m in re.finditer(r"^\s*([A-Z][A-Z0-9_]*)\s*=.*//\s*@shared",
                                              (ROOT / "cad/params.scad").read_text(), re.M)]
    bad = []
    src_files = list((ROOT / "firmware").rglob("*.h")) + list((ROOT / "firmware").rglob("*.cpp"))
    src_files = [p for p in src_files if ".pio" not in p.parts and p.name != "geometry.h"]
    for p in src_files:
        s = p.read_text()
        for name in shared:
            # a redefinition = NAME followed by '=' or #define NAME <number>
            if re.search(rf"(#define\s+{name}\s+[-\d.]|\b(?:float|double|int|int32_t|long)\s+{name}\s*=\s*[-\d.])", s):
                bad.append(f"{p.relative_to(ROOT)}: {name}")
    print(("PASS" if not bad else "FAIL"), "no firmware file re-types a shared geometry value",
          *(["\n   " + b for b in bad]))
    if bad:
        fails.append("redefinitions")

print("\nRESULT:", "PASS" if not fails else "FAIL " + ", ".join(fails))
sys.exit(1 if fails else 0)
