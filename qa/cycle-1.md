# QA cycle 1 — verdict: REJECT

Two independent reviewers with no authoring context:
- [cycle-1-fw.md](cycle-1-fw.md): firmware, electronics, docs, fresh-clone `make all`. 9 PASS, 2 FAIL.
- [cycle-1-cad.md](cycle-1-cad.md): CAD, renders, size. 7 PASS, 3 FAIL.

| Area | Criterion | Verdict |
|---|---|---|
| FW | compile, 0 warnings | PASS |
| FW | native tests + coverage | PASS |
| FW | pins = table = wiring (script) | PASS |
| FW | battery / idle / faults present + documented | PASS |
| CAD | renders + manifold STLs | PASS |
| CAD | bed fit, supports, walls ≥1.2, overhangs | **FAIL** (undocumented thin walls, cantilever mislabeled) |
| CAD | all parts placed + interference | PASS |
| CAD | card path | PASS |
| CAD | inserts / tolerances parameterized | **FAIL** (TOL unused, insert hole mismatch) |
| CAD | battery removable / jack / switch / buttons | PASS |
| CAD | size script, limits, README, optimization pass | PASS (Ø195.0 × 207.5) |
| CAD | no visible electronics, clean exterior | PASS |
| Docs | renders reviewed (and mechanical plausibility) | **FAIL** (exploded cropped; burn drawer, no pinch roller, harness route) |
| Docs | BOM coverage | PASS |
| Docs | BUILD.md | **FAIL** (USB-only boot claim, unsafe VREF procedure) |
| Docs | power budget | PASS |
| Docs | README / links | **FAIL** (qa/ link, fixed by committing reports) |
| All | `make all` from fresh clone | PASS |

Rework assigned: CAD agent (C2, C5, C9, H1–H4, servo sweep link, shell screws), firmware agent
(M1 bounded turret homing, M2 boot battery check, M3 console idle timeout, L2/L5/L7),
docs agent (D2, L1, L3, L6, L9–L11).
