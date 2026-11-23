"""Shared helpers for the CAD check scripts (tools/cad_*.py).

All geometry comes from OpenSCAD (manifold backend). Parameters are read by echoing
them from cad/check_values.scad, so nothing here duplicates a dimension.
"""
import concurrent.futures as cf
import os
import pathlib
import re
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
CAD = ROOT / "cad"
BUILD = CAD / "build"
OPENSCAD = shutil.which("openscad") or "/opt/homebrew/bin/openscad"
JOBS = max(2, (os.cpu_count() or 4))

# files in cad/ that are not printed parts
NON_PARTS = {"params", "assembly", "check_values"}


def run_scad(args, timeout=600):
    cmd = [OPENSCAD, "--backend=manifold"] + [str(a) for a in args]
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, cwd=CAD)
    return p.returncode, p.stdout + p.stderr


def scad_value(text):
    text = text.strip()
    if text in ("true", "false"):
        return text == "true"
    if text.startswith("["):
        return [scad_value(t) for t in re.findall(r"-?[\d.e+-]+|true|false", text)]
    try:
        return float(text)
    except ValueError:
        return text.strip('"')


def params():
    """Return dict of values echoed by cad/check_values.scad (VAL|name|value)."""
    out = BUILD / "check_values.echo"
    BUILD.mkdir(parents=True, exist_ok=True)
    rc, log = run_scad(["-o", out, CAD / "check_values.scad"])
    if rc != 0:
        sys.exit("check_values.scad failed:\n" + log)
    vals = {}
    for line in out.read_text().splitlines():
        m = re.match(r'ECHO: "VAL\|([^|]+)\|(.*)"$', line)
        if m:
            vals[m.group(1)] = scad_value(m.group(2))
    return vals


def printed_parts():
    return sorted(p.stem for p in CAD.glob("*.scad") if p.stem not in NON_PARTS)


def vendor_parts():
    return sorted(p.stem for p in (CAD / "vendor").glob("*.scad"))


def assembly_parts():
    """[(name, group)] from the assembly registry."""
    out = BUILD / "list.echo"
    BUILD.mkdir(parents=True, exist_ok=True)
    rc, log = run_scad(["-o", out, "-D", 'only="__list__"', CAD / "assembly.scad"])
    if rc != 0:
        sys.exit("assembly list failed:\n" + log)
    parts = []
    for line in out.read_text().splitlines():
        m = re.match(r'ECHO: "PART\|([^|]+)\|([^|]+)"', line)
        if m:
            parts.append((m.group(1), m.group(2)))
    return parts


def export_placed(names, rack_pos, turret_deg=0, outdir=None, extra=()):
    """Export each placed assembly part to STL. Returns {name: path or None (empty)}."""
    outdir = pathlib.Path(outdir or BUILD / f"placed_r{rack_pos:g}_t{turret_deg:g}")
    outdir.mkdir(parents=True, exist_ok=True)

    def one(name):
        path = outdir / f"{name}.stl"
        rc, log = run_scad(["-o", path, "-D", f'only="{name}"', "-D", f"rack_pos={rack_pos}",
                            "-D", f"turret_deg={turret_deg}", *extra, CAD / "assembly.scad"])
        if "UNKNOWN PART" in log:
            raise RuntimeError(f"assembly does not know part {name}")
        if rc != 0:
            if "empty" in log.lower():
                return name, None
            raise RuntimeError(f"export of {name} failed:\n{log[-2000:]}")
        return name, path

    with cf.ThreadPoolExecutor(JOBS) as ex:
        return dict(ex.map(one, names))


def load_mesh(path):
    import trimesh
    m = trimesh.load(path, force="mesh", process=True)
    return m


def to_manifold(tm):
    import numpy as np
    import manifold3d as m3
    mesh = m3.Mesh(vert_properties=np.asarray(tm.vertices, dtype=np.float32),
                   tri_verts=np.asarray(tm.faces, dtype=np.uint32))
    man = m3.Manifold(mesh)
    if man.status() != m3.Error.NoError:
        mm = trimesh_fix(tm)
        man = m3.Manifold(m3.Mesh(vert_properties=np.asarray(mm.vertices, dtype=np.float32),
                                  tri_verts=np.asarray(mm.faces, dtype=np.uint32)))
    return man


def trimesh_fix(tm):
    tm = tm.copy()
    tm.merge_vertices()
    tm.update_faces(tm.nondegenerate_faces())
    tm.fix_normals()
    return tm


def clean_stl(src, dst):
    """Deterministic mesh cleanup of an OpenSCAD/manifold STL: merge vertices, collapse
    sub-micron sliver edges (Manifold.simplify), drop zero-volume components, write binary STL.
    Returns (removed_components, faces_before, faces_after)."""
    import numpy as np
    import manifold3d as m3
    import trimesh
    tm = trimesh.load(src, force="mesh", process=True)
    man = m3.Manifold(m3.Mesh(vert_properties=np.asarray(tm.vertices, dtype=np.float32),
                              tri_verts=np.asarray(tm.faces, dtype=np.uint32)))
    if man.status() != m3.Error.NoError:
        raise RuntimeError(f"{src}: not a manifold ({man.status()})")
    man = man.simplify(1e-4)
    comps = man.decompose()
    keep = [c for c in comps if c.volume() > 1e-3]
    man = m3.Manifold.compose(keep) if len(keep) != 1 else keep[0]
    mesh = man.to_mesh()
    out = trimesh.Trimesh(vertices=np.asarray(mesh.vert_properties)[:, :3], faces=np.asarray(mesh.tri_verts), process=False)
    out.export(dst, file_type="stl")
    return len(comps) - len(keep), len(tm.faces), len(out.faces)


def table(rows, headers):
    w = [max(len(str(r[i])) for r in rows + [headers]) for i in range(len(headers))]
    line = "  ".join(h.ljust(w[i]) for i, h in enumerate(headers))
    out = [line, "-" * len(line)]
    for r in rows:
        out.append("  ".join(str(c).ljust(w[i]) for i, c in enumerate(r)))
    return "\n".join(out)
