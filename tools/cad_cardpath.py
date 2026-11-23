#!/usr/bin/env python3
"""Card-path geometry check. Values come from cad/params.scad (echo) and are verified
against the exported meshes of the placed parts by ray casting (turret 0, rack_pos 0).

Checks
  1. every shelf slot (52) is >= CARD_T + 0.3 tall (measured fin surfaces) and the card
     has >= 0.3 mm end clearance to the comb walls
  2. measured card planes: eject(i) and load(i) rack positions put every shelf within
     +-0.2 mm of Z_EJECT / Z_LOAD
  3. load station: hopper floor + feed roller tops at Z_LOAD +-0.2; throat passes one card only
  4. eject station: launcher entry floor and idler top (nip) at Z_EJECT +-0.2; wheel squeezes the card
  5. pusher: blade straddles the card edge (incl. sag) and clears the neighbouring cards;
     stroke >= distance needed to push the card past the nip by NIP_GRIP
  6. rack travel covers shelves 0..51 at both stations
  7. stopped card clears every static launcher face except the stop fingers
Exit 1 on any FAIL.
"""
import sys

import numpy as np

sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parent))
from cad_common import export_placed, load_mesh, params, table  # noqa: E402

TOL = 0.2
rows = []


def check(name, value, limit, ok, unit="mm"):
    rows.append([name, f"{value:.3f} {unit}" if isinstance(value, float) else str(value), limit, "PASS" if ok else "FAIL"])


def zhits(mesh, x, y, z0=400.0, down=True):
    o = np.array([[x, y, z0 if down else -50.0]])
    d = np.array([[0, 0, -1.0 if down else 1.0]])
    loc, _, _ = mesh.ray.intersects_location(o, d, multiple_hits=True)
    return np.sort(loc[:, 2]) if len(loc) else np.array([])


def first_hit(mesh, origin, direction):
    loc, _, _ = mesh.ray.intersects_location(np.array([origin], float), np.array([direction], float), multiple_hits=False)
    return loc[0] if len(loc) else None


def main():
    P = params()
    n, p, t = int(P["SHELF_COUNT"]), P["SHELF_PITCH"], P["CARD_T"]
    names = ["rack_comb_px", "rack_bottom_plate", "rack_top_plate", "hopper_body", "hopper_gate", "v_feed_rollers",
             "launcher_floor", "launcher_frame", "v_idler_roller", "v_wheel_launcher", "v_pusher_blade",
             "v_ir_exit_tx", "idler_arm"]
    paths = export_placed(names, 0, 0)
    M = {k: load_mesh(v) for k, v in paths.items()}

    # 1. slots, measured in the fin zone of the +X comb
    xf = (P["FIN_IN"] + P["COMB_IN"]) / 2
    yc = P["RACK_YC"]
    comb = zhits(M["rack_comb_px"], xf, yc)
    bot = zhits(M["rack_bottom_plate"], xf, yc)
    top = zhits(M["rack_top_plate"], xf, yc)
    # fin surfaces come in pairs (bottom, top); comb wall rebates mean the comb ray only meets fins here
    fins = comb.reshape(-1, 2)
    floors = np.concatenate([[bot.max()], fins[:, 1]])          # card-bottom planes, shelf 0..51
    ceilings = np.concatenate([fins[:, 0], [top.min()]])
    gaps = ceilings - floors
    check("shelves measured", len(floors), f"= {n}", len(floors) == n, "")
    check("min slot height", float(gaps.min()), f">= CARD_T+0.3 = {t + 0.3:.2f}", gaps.min() >= t + 0.3 - 1e-3)
    check("min vertical card clearance", float(gaps.min() - t), ">= 0.300", gaps.min() - t >= 0.3 - 1e-3)
    wall = first_hit(M["rack_comb_px"], [0.0, yc, floors[25] + 0.3], [1, 0, 0])
    endclr = wall[0] - P["CARD_L"] / 2
    check("card end clearance to comb wall", float(endclr), ">= 0.300", endclr >= 0.3 - 1e-3)
    check("fin thickness (D-01 exception)", float((ceilings[1:] - floors[1:]).mean() and (fins[:, 1] - fins[:, 0]).min()), "0.9 (documented)", abs((fins[:, 1] - fins[:, 0]).min() - P["SHELF_FIN"]) < 0.01)

    # 2. stations for every shelf
    eje = [floors[i] + (n - 1 - i) * p for i in range(n)]
    lod = [floors[i] + (n - 1 - i) * p + P["STATION_OFFSET"] for i in range(n)]
    err_e = max(abs(z - P["Z_EJECT"]) for z in eje)
    err_l = max(abs(z - (P["Z_LOAD"] + 0)) for z in lod)
    # rack_pos for load(i) = eject(i) + STATION_OFFSET, so the plane lands at Z_EJECT + OFFSET
    err_l = max(abs((floors[i] + (n - 1 - i) * p + P["STATION_OFFSET"]) - P["Z_LOAD"]) for i in range(n))
    check("eject: all 52 shelf planes vs Z_EJECT", float(err_e), f"<= {TOL}", err_e <= TOL)
    check("load: all 52 shelf planes vs Z_LOAD", float(err_l), f"<= {TOL}", err_l <= TOL)

    # 3. load station
    hcy = (P["HOP_YI0"] + P["HOP_YI1"]) / 2
    hf = zhits(M["hopper_body"], 0.0, hcy)
    floor_top = hf[hf < P["Z_LOAD"] + 5].max()
    check("hopper floor top vs Z_LOAD", float(floor_top - P["Z_LOAD"]), f"|d| <= {TOL}", abs(floor_top - P["Z_LOAD"]) <= TOL)
    rt = zhits(M["v_feed_rollers"], P["FEED_X"], P["FEED_Y"]).max()
    check("feed roller top vs Z_LOAD", float(rt - P["Z_LOAD"]), f"|d| <= {TOL}", abs(rt - P["Z_LOAD"]) <= TOL)
    gb = zhits(M["hopper_gate"], 0.0, P["HOP_Y_FRONT"] - 1.0, down=False).min()
    thr = gb - P["Z_LOAD"]
    check("throat under gate (single card)", float(thr), f"{t:.2f} < g < {2 * t:.2f}", t < thr < 2 * t)
    check("hopper card end clearance", float(P["HOP_XI"] - P["CARD_L"] / 2), ">= 0.3", P["HOP_XI"] - P["CARD_L"] / 2 >= 0.3)

    # 4. eject station / launcher nip
    lf = zhits(M["launcher_floor"], 0.0, (P["LF_Y0"] + P["LFL_Y1"]) / 2).max()
    check("launcher entry floor top vs Z_EJECT", float(lf - P["Z_EJECT"]), f"|d| <= {TOL}", abs(lf - P["Z_EJECT"]) <= TOL)
    it = zhits(M["v_idler_roller"], 0.0, P["NIP_Y"]).max()
    check("nip (idler top) vs Z_EJECT", float(it - P["Z_EJECT"]), f"|d| <= {TOL}", abs(it - P["Z_EJECT"]) <= TOL)
    wb = zhits(M["v_wheel_launcher"], 0.0, P["NIP_Y"], down=False).min()
    sq = t - (wb - it)
    check("nip squeeze on the card (wheel-idler gap)", float(sq), f"0 < s < {t:.2f}", 0 < sq < t)
    check("nip vs load plane offset", float(P["Z_LOAD"] - P["Z_EJECT"]), f"= STATION_OFFSET {P['STATION_OFFSET']}", abs(P["Z_LOAD"] - P["Z_EJECT"] - P["STATION_OFFSET"]) < 1e-6)

    # 5. pusher
    bl = M["v_pusher_blade"].bounds
    bz0, bz1 = bl[0][2], bl[1][2]
    e = P["Z_EJECT"]
    sag = 0.38  # self-weight sag of a 0.30 mm PVC card on an 84 mm span (D-13)
    check("blade overlaps flat card edge", float(min(bz1, e + t) - max(bz0, e)), "> 0.10", min(bz1, e + t) - max(bz0, e) > 0.1)
    check("blade overlaps sagged card edge", float(min(bz1, e + t - sag) - max(bz0, e - sag)), "> 0.10", min(bz1, e + t - sag) - max(bz0, e - sag) > 0.1)
    check("blade clears card below (top)", float(bz0 - (e - p + t)), "> 0.30", bz0 - (e - p + t) > 0.3)
    check("blade clears card above (bottom)", float((e + p) - bz1), "> 0.30", (e + p) - bz1 > 0.3)
    tip = bl[1][1]
    card_back = P["RACK_YC"] - P["CARD_W"] / 2
    card_front = P["RACK_YC"] + P["CARD_W"] / 2
    need = (card_back - tip) + (P["NIP_Y"] + P["NIP_GRIP"] - card_front)
    check("blade retracted clear of rack", float(P["RACK_Y0"] - tip), ">= 0.5", P["RACK_Y0"] - tip >= 0.5 - 1e-6)
    check("pusher stroke", float(P["PUSH_STROKE"]), f">= required {need:.2f}", P["PUSH_STROKE"] >= need)
    check("stroke margin past the nip", float(P["PUSH_STROKE"] - need + P["NIP_GRIP"]), f">= NIP_GRIP {P['NIP_GRIP']}", P["PUSH_STROKE"] - need >= 0)
    check("blade tip at full stroke inside rack", float(tip + P["PUSH_STROKE"]), f"< RACK_Y1 {P['RACK_Y1']}", tip + P["PUSH_STROKE"] < P["RACK_Y1"])

    # 6. travel
    ej = [(n - 1 - i) * p for i in range(n)]
    ld = [x + P["STATION_OFFSET"] for x in ej]
    ok = min(ej + ld) >= -1e-9 and max(ej + ld) <= P["RACK_TRAVEL"] + 1e-9
    check("rack travel covers shelves 0-51 (eject+load)", float(max(ej + ld)), f"in [0, RACK_TRAVEL={P['RACK_TRAVEL']}]", ok)

    # 7. stopped card vs launcher statics (finger stop = FINGER_Y)
    lfm = M["launcher_frame"]
    v = lfm.vertices
    band = (np.abs(v[:, 0]) < P["CARD_L"] / 2) & (np.abs(np.abs(v[:, 0]) - P["FINGER_X"]) > 3.01)
    ymin_frame = v[band][:, 1].min()
    fing = (np.abs(np.abs(v[:, 0]) - P["FINGER_X"]) <= 3.01) & (np.abs(v[:, 2] - P["Z_LOAD"]) < 1.0)
    stop = v[fing][:, 1].min()
    others = min(ymin_frame, M["launcher_floor"].bounds[0][1], M["v_ir_exit_tx"].bounds[0][1], M["v_wheel_launcher"].bounds[0][1], M["idler_arm"].bounds[0][1])
    check("load-stop finger face (card front stops here)", float(stop), f"rack face {P['RACK_Y1']} + >= 0.5", stop - P["RACK_Y1"] >= 0.5 - 1e-6)
    check("stopped card clears other launcher parts", float(others - stop), ">= 0.5", others - stop >= 0.5 - 1e-6)

    print(table(rows, ["check", "value", "limit", "result"]))
    nf = sum(r[3] == "FAIL" for r in rows)
    print(f"\n{len(rows) - nf}/{len(rows)} PASS   (Z_EJECT={P['Z_EJECT']:.2f}, Z_LOAD={P['Z_LOAD']:.2f}, "
          f"pitch {p}, slot {P['SHELF_GAP']:.2f}, stroke {P['PUSH_STROKE']:.2f} mm, need {need:.2f} mm)")
    sys.exit(1 if nf else 0)


if __name__ == "__main__":
    main()
