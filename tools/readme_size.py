#!/usr/bin/env python3
"""Copy renders/size_report.md (from cad_size.py) into README.md between SIZE_REPORT markers."""
import pathlib, re
R = pathlib.Path(__file__).resolve().parent.parent
rep = (R / "renders/size_report.md").read_text().strip().splitlines()
body = "\n".join(l for l in rep if not l.startswith("# "))
block = ("<!-- SIZE_REPORT -->\n" + body +
         "\n\nThe size-optimization pass and every size trade-off are in [docs/DECISIONS.md](docs/DECISIONS.md)."
         " The stretch target cannot be met with this battery and a 52-slot random-access rack (D-01, D-04)."
         "\n<!-- /SIZE_REPORT -->")
rd = R / "README.md"
t = rd.read_text()
t = re.sub(r"<!-- SIZE_REPORT -->.*?<!-- /SIZE_REPORT -->", block, t, flags=re.S) \
    if "<!-- /SIZE_REPORT -->" in t else t.replace("<!-- SIZE_REPORT -->", block)
rd.write_text(t)
print("README size section updated")
