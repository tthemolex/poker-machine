# make all = firmware + tests + STLs + renders + wiring + every check.
# Works from a fresh clone given: PlatformIO, OpenSCAD (>= 2025, manifold backend),
# python3 (trimesh numpy manifold3d wireviz), graphviz, admesh, imagemagick.  See docs/BUILD.md.

PIO    ?= $(shell command -v pio 2>/dev/null || echo $(HOME)/.platformio/penv/bin/pio)
PY     ?= python3
BUILD  := build

.PHONY: all geometry firmware test cad stl renders printability interference cardpath size wiring checks clean

all: geometry firmware test stl wiring renders checks
	@echo; echo "make all: OK"

geometry:
	$(PY) tools/gen_geometry.py

firmware: geometry
	@mkdir -p $(BUILD)
	cd firmware && $(PIO) run -e esp32dev 2>&1 | tee ../$(BUILD)/fw_build.log
	@! grep -E "(src|lib/dealer_core|include)/[^ ]*: warning:" $(BUILD)/fw_build.log \
	  || (echo "FAIL: warnings in project code"; exit 1)
	@echo "firmware: 0 errors, 0 project warnings"

test: geometry
	cd firmware && $(PIO) test -e native

stl:
	$(PY) tools/cad_export.py

renders:
	$(PY) tools/cad_render.py

printability:
	$(PY) tools/cad_printability.py

interference:
	$(PY) tools/cad_interference.py

cardpath:
	$(PY) tools/cad_cardpath.py

size:
	$(PY) tools/cad_size.py
	$(PY) tools/readme_size.py

wiring:
	cd electronics && wireviz wiring.yml
	dot -Tsvg electronics/power.dot -o electronics/power.svg
	dot -Tpng -Gdpi=150 electronics/power.dot -o electronics/power.png
	$(PY) electronics/power_budget.py --check

checks: printability interference cardpath size
	$(PY) tools/check_consistency.py
	$(PY) tools/check_pins.py
	$(PY) tools/check_bom.py

clean:
	rm -rf $(BUILD) firmware/.pio
