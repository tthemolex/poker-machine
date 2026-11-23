#!/usr/bin/env python3
"""Render the assembly views and every printed part to renders/*.png (OpenSCAD manifold),
plus an optional turret-rotation GIF (ImageMagick). Usage: python3 tools/cad_render.py [--no-gif]"""
import concurrent.futures as cf
import shutil
import subprocess
import sys

sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parent))
from cad_common import CAD, JOBS, ROOT, printed_parts, run_scad  # noqa: E402

OUT = ROOT / "renders"
SIZE = "1600,1200"
SCHEME = "Tomorrow"
SHELLS = '["turret_shell","top_cap","hopper_lid","burn_tray"]'
VIEWS = {
    # name: (defines, camera tx,ty,tz,rx,ry,rz,dist)
    "assembly_front_exterior": ([], "0,0,100,72,0,25,640"),       # exit slot + burn drawer (+Y side)
    "assembly_back_ui": ([], "0,0,90,72,0,205,640"),               # UI bezel side (-Y)
    "assembly_top_closed": ([], "0,0,100,35,0,335,640"),
    "assembly_open_iso": ([f"hide={SHELLS}", "rack_pos=30"], "0,0,115,62,0,40,720"),
    "assembly_open_back": ([f"hide={SHELLS}", "rack_pos=30"], "0,0,120,62,0,215,560"),
    "assembly_open_top": ([f'hide={SHELLS[:-1]},"hopper_weight","cards_hopper"]', "rack_pos=81"], "0,0,120,0,0,0,520"),
    "exploded": (["explode=1", "rack_pos=30"], "0,0,175,74,0,30,1450"),
    "section_rack_pusher_launcher": (["section=true", "rack_pos=30"], "0,0,110,90,0,90,560"),
    "section_detail_stations": (["section=true", "rack_pos=30"], "0,20,122,90,0,90,240"),
    "section_rack_top": (["section=true", "rack_pos=81"], "0,0,110,90,0,90,560"),
    "base_electronics": (["only_base=true"], "0,-20,10,48,0,20,430"),
}


def render(name, defines, cam, src=CAD / "assembly.scad", out=None):
    out = out or OUT / f"{name}.png"
    args = ["--render", "-o", out, "--imgsize", SIZE, "--colorscheme", SCHEME, "--camera", cam, "--projection", "o" if name.startswith("section") else "p"]
    for d in defines:
        args += ["-D", d]
    rc, log = run_scad(args + [src])
    return name, rc, log


def part_png(part):
    out = OUT / "parts" / f"{part}.png"
    rc, log = run_scad(["--render", "-o", out, "--imgsize", "1000,800", "--colorscheme", SCHEME, "--viewall", "--autocenter",
                        "--camera", "0,0,0,55,0,30,300", "--projection", "p", CAD / f"{part}.scad"])
    return part, rc, log


def main():
    OUT.mkdir(exist_ok=True)
    (OUT / "parts").mkdir(exist_ok=True)
    jobs = []
    with cf.ThreadPoolExecutor(JOBS) as ex:
        for n, (d, c) in VIEWS.items():
            jobs.append(ex.submit(render, n, d, c))
        for p in printed_parts():
            jobs.append(ex.submit(part_png, p))
        if "--no-gif" not in sys.argv:
            (CAD / "build" / "gif").mkdir(parents=True, exist_ok=True)
            for k, a in enumerate(range(0, 360, 30)):
                jobs.append(ex.submit(render, f"gif{k:02d}", [f"turret_deg={a}", f"hide={SHELLS}", "rack_pos=40"],
                                      "0,0,110,62,0,30,620", CAD / "assembly.scad", CAD / "build" / "gif" / f"f{k:02d}.png"))
        res = [j.result() for j in jobs]
    bad = [(n, log[-400:]) for n, rc, log in res if rc != 0]
    for n, log in bad:
        print("FAIL", n, log)
    if "--no-gif" not in sys.argv and shutil.which("magick"):
        subprocess.run(["magick", "-delay", "40", "-loop", "0", str(CAD / "build" / "gif" / "f*.png"), "-resize", "800x600",
                        str(OUT / "turret_rotation.gif")], check=False)
    n_ok = len(res) - len(bad)
    print(f"{n_ok}/{len(res)} renders OK -> renders/ (views: {', '.join(VIEWS)}; parts: renders/parts/)")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
