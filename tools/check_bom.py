#!/usr/bin/env python3
"""Cross-check modelled vendor parts against the BOM.

  cad/vendor/vendor_parts.csv   name,qty,description   (every bought part modelled in CAD)
  docs/BOM.csv                  part,qty,spec,price_pln,price_eur,example_source,category

FAIL if: a vendor part has no BOM line (exact name, else fuzzy fallback), a BOM qty is < 1 or
smaller than the modelled qty, a BOM spec/source is empty, or a price is missing / not > 0.
INFO: BOM lines that are not modelled (wire, resistors, filament... are expected here).

Usage: python3 tools/check_bom.py [--root DIR] [--allow-missing-cad]   exit 0 = PASS
"""
import argparse
import csv
import difflib
import os
import re
import sys

BOM_COLS = ["part", "qty", "spec", "price_pln", "price_eur", "example_source", "category"]
CAD_COLS = ["name", "qty", "description"]


def norm(s):
    return re.sub(r"[^a-z0-9]+", "", (s or "").lower())


def num(v):
    try:
        return float(str(v).strip())
    except ValueError:
        return None


def fuzzy(name, bom_names):
    """Exact -> normalised -> containment -> difflib. Returns (bom_name, how) or (None, None)."""
    if name in bom_names:
        return name, "exact"
    n = norm(name)
    by_norm = {norm(b): b for b in bom_names}
    if n in by_norm:
        return by_norm[n], "normalised"
    cont = [b for b in bom_names if norm(b) and (norm(b) in n or n in norm(b))]
    if len(cont) == 1:
        return cont[0], "fuzzy-contains"
    m = difflib.get_close_matches(n, list(by_norm), n=1, cutoff=0.75)
    if m:
        return by_norm[m[0]], "fuzzy-similar"
    return None, None


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--root", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    ap.add_argument("--allow-missing-cad", action="store_true",
                    help="do not fail when cad/vendor/vendor_parts.csv does not exist yet")
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    p_bom = os.path.join(root, "docs/BOM.csv")
    p_cad = os.path.join(root, "cad/vendor/vendor_parts.csv")
    errs, warns = [], []

    if not os.path.exists(p_bom):
        print("FAIL: missing docs/BOM.csv")
        return 1
    with open(p_bom, newline="") as f:
        rd = csv.DictReader(f)
        if rd.fieldnames != BOM_COLS:
            errs.append(f"BOM.csv header must be {','.join(BOM_COLS)}, got {rd.fieldnames}")
        bom = {}
        for i, r in enumerate(rd, 2):
            p = (r.get("part") or "").strip()
            if not p:
                errs.append(f"BOM.csv:{i}: empty part name")
                continue
            if p in bom:
                errs.append(f"BOM.csv:{i}: duplicate part {p}")
            q = num(r.get("qty"))
            if q is None or q < 1:
                errs.append(f"BOM.csv:{i}: {p}: qty must be >= 1 (got {r.get('qty')!r})")
            for c in ("spec", "example_source", "category"):
                if not (r.get(c) or "").strip():
                    errs.append(f"BOM.csv:{i}: {p}: empty {c}")
            for c in ("price_pln", "price_eur"):
                v = num(r.get(c))
                if v is None or v <= 0:
                    errs.append(f"BOM.csv:{i}: {p}: {c} must be a number > 0 (got {r.get(c)!r})")
            bom[p] = {**r, "qty": q or 0}

    cad = {}
    if os.path.exists(p_cad):
        with open(p_cad, newline="") as f:
            rd = csv.DictReader(f)
            if [c.strip() for c in (rd.fieldnames or [])][:3] != CAD_COLS:
                errs.append(f"vendor_parts.csv header must start with {','.join(CAD_COLS)}, got {rd.fieldnames}")
            for i, r in enumerate(rd, 2):
                n = (r.get("name") or "").strip()
                if not n or n.startswith("#"):
                    continue
                q = num(r.get("qty"))
                cad[n] = cad.get(n, 0) + (q if q is not None else 1)
    elif a.allow_missing_cad:
        warns.append("cad/vendor/vendor_parts.csv not found: cross-check skipped (--allow-missing-cad)")
    else:
        errs.append("cad/vendor/vendor_parts.csv not found (run the CAD export first)")

    rows, used = [], set()
    for n in sorted(cad):
        b, how = fuzzy(n, list(bom))
        if b is None:
            errs.append(f"vendor part '{n}' (qty {cad[n]:g}) has no BOM line")
            rows.append((n, f"{cad[n]:g}", "-", "-", "MISSING"))
            continue
        used.add(b)
        if how != "exact":
            warns.append(f"vendor part '{n}' matched BOM '{b}' by {how}: use the canonical name")
        st = "OK"
        if bom[b]["qty"] < cad[n]:
            errs.append(f"BOM qty of '{b}' ({bom[b]['qty']:g}) < modelled qty of '{n}' ({cad[n]:g})")
            st = "QTY<CAD"
        rows.append((n, f"{cad[n]:g}", b, f"{bom[b]['qty']:g}", st))

    if rows:
        w1 = max(len(r[0]) for r in rows + [("vendor part",)])
        w2 = max(len(r[2]) for r in rows + [("", "", "BOM part")])
        print(f"{'vendor part':<{w1}}  cad_qty  {'BOM part':<{w2}}  bom_qty  result")
        print("-" * (w1 + w2 + 30))
        for n, cq, b, bq, st in rows:
            print(f"{n:<{w1}}  {cq:>7}  {b:<{w2}}  {bq:>7}  {st}")
        print()
    unmodelled = [b for b in bom if b not in used]
    if cad and unmodelled:
        print("INFO: BOM lines not modelled in CAD (consumables / small parts): " + ", ".join(unmodelled))
    tot_pln = sum(num(r["price_pln"]) or 0 for r in bom.values())
    tot_eur = sum(num(r["price_eur"]) or 0 for r in bom.values())
    print(f"INFO: BOM {len(bom)} lines, estimated total {tot_pln:.0f} PLN / {tot_eur:.0f} EUR")
    for x in warns:
        print("WARN: " + x)
    if errs:
        for x in errs:
            print("FAIL: " + x)
        print(f"\ncheck_bom: FAIL ({len(errs)} problem(s))")
        return 1
    print(f"check_bom: PASS ({len(cad)} vendor parts covered by the BOM)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
