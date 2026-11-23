# PLAN

Orchestrator: main session. Owns `cad/params.scad` shared block, `tools/gen_geometry.py`,
consistency checks, `Makefile`, QA cycles and commits.

| # | Milestone | Owner | Output |
|---|-----------|-------|--------|
| M0 | Architecture + shared params | Orchestrator | docs/ARCHITECTURE.md, cad/params.scad (@shared), tools/gen_geometry.py |
| M1 | Firmware refactor + native tests | Firmware agent | firmware/ (PlatformIO esp32dev + native), FIRMWARE.md |
| M2 | CAD: vendor models, printed parts, assembly, checks | Mechanical/CAD agent | cad/*.scad, cad/vendor, cad/stl, renders/, tools/cad_*.py |
| M3 | Electronics + docs | Electronics/Docs agent | electronics/ (WireViz, pins.csv, power budget), docs/BOM.csv, BUILD.md, README.md |
| M4 | Integration | Orchestrator | Makefile (`make all`), tools/check_*.py |
| M5 | QA cycle N | Fresh QA agents | qa/cycle-N.md |
| M6 | Size-optimization pass | CAD agent + Orchestrator | DECISIONS.md entries, re-run checks |
| M7 | Final | Orchestrator | qa/FINAL.md, README, final commit |

Cycle: DEVELOP → QA → APPROVE / REJECT (max 6 QA cycles). Commit after each approved milestone.
