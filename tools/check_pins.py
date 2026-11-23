#!/usr/bin/env python3
"""Check that firmware pins, the pin table and the wiring diagram agree 1:1.

Sources:
  firmware/include/config.h   `#define PIN_<SIGNAL> <gpio>`
  electronics/pins.csv         signal,gpio,direction,connects_to,notes
  electronics/PINS.md          human pin table (markdown, first table: Signal | GPIO | Dir | ...)
  electronics/wiring.yml       WireViz: connector `ESP32` with pin labels `GPIO<n>`; each cable
                               wire landing on an ESP32 GPIO has a wirelabel = signal name.

Asserts: same signal set in all four, same GPIO per signal, no GPIO used by two signals,
no flash pins (6-11), input-only pins (34-39) only used as inputs, every GPIO label on the
ESP32 connector is wired. Warns about strapping / UART pins.

Usage: python3 tools/check_pins.py [--root DIR]     exit 0 = PASS, 1 = FAIL
"""
import argparse
import csv
import os
import re
import sys

try:
    import yaml
except ImportError:  # pragma: no cover
    sys.exit("check_pins: PyYAML is required (pip install pyyaml)")

FLASH = set(range(6, 12))
INPUT_ONLY = set(range(34, 40))
STRAPPING = {0, 2, 5, 12, 15}
UART0 = {1, 3}
ESP = "ESP32"


def parse_config(path):
    pins = {}
    errs = []
    with open(path) as f:
        for ln, line in enumerate(f, 1):
            m = re.match(r"\s*#\s*define\s+PIN_(\w+)\s+(\d+)\b", line)
            if m:
                name, gpio = m.group(1), int(m.group(2))
                if name in pins and pins[name] != gpio:
                    errs.append(f"config.h:{ln}: PIN_{name} redefined ({pins[name]} -> {gpio})")
                pins[name] = gpio
    return pins, errs


def parse_csv(path):
    rows, errs = {}, []
    with open(path, newline="") as f:
        rd = csv.DictReader(f)
        need = {"signal", "gpio", "direction", "connects_to", "notes"}
        if not need.issubset(rd.fieldnames or []):
            errs.append(f"pins.csv: header must contain {sorted(need)}, got {rd.fieldnames}")
            return rows, errs
        for i, r in enumerate(rd, 2):
            s = (r["signal"] or "").strip()
            try:
                g = int(r["gpio"])
            except (TypeError, ValueError):
                errs.append(f"pins.csv:{i}: bad gpio {r['gpio']!r}")
                continue
            if s in rows:
                errs.append(f"pins.csv:{i}: duplicate signal {s}")
            d = (r["direction"] or "").strip().lower()
            if d not in ("in", "out", "io"):
                errs.append(f"pins.csv:{i}: direction must be in/out/io, got {d!r}")
            rows[s] = {"gpio": g, "dir": d, "to": r["connects_to"]}
    return rows, errs


def parse_md(path):
    """First markdown table whose header starts with 'Signal' and has a 'GPIO' column."""
    rows, errs = {}, []
    hdr = None
    with open(path) as f:
        for ln, line in enumerate(f, 1):
            line = line.strip()
            if not line.startswith("|"):
                if hdr is not None and rows:
                    break
                continue
            cells = [c.strip().strip("`*") for c in line.strip("|").split("|")]
            if hdr is None:
                if cells and cells[0].lower().startswith("signal") and any(c.lower() == "gpio" for c in cells):
                    hdr = [c.lower() for c in cells]
                continue
            if set(line) <= set("|-: "):
                continue
            row = dict(zip(hdr, cells))
            sig = cells[0]
            try:
                g = int(row["gpio"])
            except (KeyError, ValueError):
                errs.append(f"PINS.md:{ln}: bad gpio in {line!r}")
                continue
            if sig in rows:
                errs.append(f"PINS.md:{ln}: duplicate signal {sig}")
            rows[sig] = {"gpio": g, "dir": row.get("dir", "").lower()}
    if hdr is None:
        errs.append("PINS.md: no table with a 'Signal' and a 'GPIO' column found")
    return rows, errs


def expand(v):
    """WireViz pin/wire spec -> list. Supports scalars, lists and 'a-b' ranges of ints."""
    items = v if isinstance(v, list) else [v]
    out = []
    for it in items:
        if isinstance(it, str) and re.fullmatch(r"\d+-\d+", it):
            a, b = map(int, it.split("-"))
            step = 1 if b >= a else -1
            out.extend(range(a, b + step, step))
        else:
            out.append(it)
    return out


def parse_wiring(path):
    """Return ({gpio: set(signals)}, [labels of ESP32 GPIO pins], errors)."""
    errs = []
    with open(path) as f:
        doc = yaml.safe_load(f)
    conns, cables = doc.get("connectors", {}), doc.get("cables", {})
    if ESP not in conns:
        return {}, [], [f"wiring.yml: no connector named {ESP}"]
    esp = conns[ESP]
    labels = [str(x) for x in esp.get("pinlabels", [])]
    pins = [p for p in esp.get("pins", [])] or list(range(1, len(labels) + 1))
    num2label = dict(zip(pins, labels))
    gpio_labels = [l for l in labels if re.fullmatch(r"GPIO\d+", l)]
    for l in labels:
        if re.match(r"(?i)gpio", l) and not re.fullmatch(r"GPIO\d+", l):
            errs.append(f"wiring.yml: ESP32 pin label {l!r} is not of the form GPIO<n>")
    if len(set(gpio_labels)) != len(gpio_labels):
        errs.append("wiring.yml: duplicate GPIO labels on the ESP32 connector")

    found = {}
    for si, cset in enumerate(doc.get("connections", []), 1):
        ents = []
        for e in cset:
            if not isinstance(e, dict) or len(e) != 1:
                errs.append(f"wiring.yml: connection set {si}: unsupported entry {e!r}")
                ents.append((None, []))
                continue
            (k, v), = e.items()
            ents.append((k, expand(v)))
        for i, (name, plist) in enumerate(ents):
            if name != ESP:
                continue
            for j in (i - 1, i + 1):
                if not (0 <= j < len(ents)) or ents[j][0] not in cables:
                    continue
                cname, wires = ents[j]
                cab = cables[cname]
                wl = [str(x) for x in cab.get("wirelabels", [])]
                if len(wires) != len(plist):
                    errs.append(f"wiring.yml: set {si}: {ESP} has {len(plist)} pins but {cname} has {len(wires)} wires")
                    continue
                for p, w in zip(plist, wires):
                    lab = num2label.get(p, str(p)) if isinstance(p, int) else str(p)
                    if lab not in labels:
                        errs.append(f"wiring.yml: set {si}: {ESP} pin {p!r} not found")
                        continue
                    m = re.fullmatch(r"GPIO(\d+)", lab)
                    if not m:
                        continue  # VIN / GND / 3V3
                    if not isinstance(w, int) or not 1 <= w <= len(wl):
                        errs.append(f"wiring.yml: set {si}: cable {cname} wire {w!r} has no wirelabel")
                        continue
                    found.setdefault(int(m.group(1)), set()).add(wl[w - 1])
    return found, gpio_labels, errs


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--root", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    p_cfg = os.path.join(root, "firmware/include/config.h")
    p_csv = os.path.join(root, "electronics/pins.csv")
    p_yml = os.path.join(root, "electronics/wiring.yml")
    p_md = os.path.join(root, "electronics/PINS.md")

    errs, warns = [], []
    for p in (p_cfg, p_csv, p_yml, p_md):
        if not os.path.exists(p):
            errs.append(f"missing {os.path.relpath(p, root)}")
    if errs:
        print("\n".join("FAIL: " + e for e in errs))
        return 1

    fw, e1 = parse_config(p_cfg)
    tb, e2 = parse_csv(p_csv)
    wgpio, glabels, e3 = parse_wiring(p_yml)
    md, e4 = parse_md(p_md)
    errs += e1 + e2 + e3 + e4
    for s_, r in md.items():
        if s_ in tb and r["dir"] != tb[s_]["dir"]:
            errs.append(f"{s_}: direction PINS.md={r['dir']!r} pins.csv={tb[s_]['dir']!r}")

    # wiring: gpio -> exactly one signal
    wsig = {}
    for g, sigs in sorted(wgpio.items()):
        if len(sigs) > 1:
            errs.append(f"wiring.yml: GPIO{g} carries several signals {sorted(sigs)}")
        for s in sigs:
            if s in wsig and wsig[s] != g:
                errs.append(f"wiring.yml: signal {s} on GPIO{wsig[s]} and GPIO{g}")
            wsig[s] = g
    for l in glabels:
        if int(l[4:]) not in wgpio:
            errs.append(f"wiring.yml: ESP32 pin {l} is labelled but not wired")

    signals = sorted(set(fw) | set(tb) | set(wsig) | set(md), key=lambda s: (fw.get(s, tb.get(s, {}).get("gpio", 99)), s))
    rows = []
    for s in signals:
        f = fw.get(s)
        t = tb[s]["gpio"] if s in tb else None
        w = wsig.get(s)
        m = md[s]["gpio"] if s in md else None
        ok = f is not None and f == t == w == m
        if not ok:
            errs.append(f"{s}: config.h={f} pins.csv={t} wiring.yml={w} PINS.md={m}")
        rows.append((s, f, t, w, m, tb.get(s, {}).get("dir", "-"), "OK" if ok else "MISMATCH"))

    # duplicate gpios per source
    for src, mp in (("config.h", fw), ("pins.csv", {k: v["gpio"] for k, v in tb.items()}),
                    ("PINS.md", {k: v["gpio"] for k, v in md.items()})):
        seen = {}
        for s, g in mp.items():
            if g in seen:
                errs.append(f"{src}: GPIO{g} used by {seen[g]} and {s}")
            seen[g] = s

    # electrical rules
    for s, g in fw.items():
        if g in FLASH:
            errs.append(f"{s}: GPIO{g} is a flash pin (6-11)")
        if g > 39 or g in (20, 24, 28, 29, 30, 31):
            errs.append(f"{s}: GPIO{g} does not exist on ESP32-WROOM-32")
        if g in INPUT_ONLY and tb.get(s, {}).get("dir") != "in":
            errs.append(f"{s}: GPIO{g} is input-only but pins.csv direction is {tb.get(s, {}).get('dir')!r}")
        if g in STRAPPING:
            warns.append(f"{s}: GPIO{g} is a strapping pin (check boot level; see PINS.md)")
        if g in UART0:
            warns.append(f"{s}: GPIO{g} is UART0 (USB serial)")

    w = max([len(r[0]) for r in rows] + [6])
    print(f"{'signal':<{w}}  config.h  pins.csv  wiring.yml  PINS.md  dir  result")
    print("-" * (w + 53))
    for s, f, t, wv, m, d, r in rows:
        fmt = lambda x: "-" if x is None else str(x)
        print(f"{s:<{w}}  {fmt(f):>8}  {fmt(t):>8}  {fmt(wv):>10}  {fmt(m):>7}  {d:>3}  {r}")
    print()
    for x in warns:
        print("WARN: " + x)
    if errs:
        for x in errs:
            print("FAIL: " + x)
        print(f"\ncheck_pins: FAIL ({len(errs)} problem(s))")
        return 1
    print(f"check_pins: PASS ({len(rows)} signals agree in config.h, pins.csv, wiring.yml and PINS.md)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
