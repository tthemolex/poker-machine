// Launcher frame: two side plates + top plate, 130-motor cheeks, load-stop fingers
// (stop cards loaded at Z_LOAD flush with the rack face), exit IR emitter holder,
// wedge feet to the turntable. The slot panel screws to the side plates.
// Print: upside down (top plate on bed); feet are 45 deg wedges, motor holes bridge 20 mm.
include <params.scad>
include <lib/lib.scad>
e = Z_EJECT;
LF_TOP_Z1 = LF_TOP_Z0 + LF_TOP_T;
CHEEKS = [[10, 13], [27, 30]];
module m130_profile2d(grow = 0) intersection() { circle(d = 20 + 2 * grow); square([22 + 2 * grow, 15 + 2 * grow], center = true); }
module launcher_frame() difference() {
    union() {
        for (s = [-1, 1]) box_at(s > 0 ? LF_XI : -LF_XO, s > 0 ? LF_XO : -LF_XI, LF_Y0, LF_Y1, TT_TOP, LF_TOP_Z1);
        box_at(-LF_XO, LF_XO, LF_Y0, LF_Y1, LF_TOP_Z0, LF_TOP_Z1);
        for (s = [-1, 1]) hull() {     // wedge feet
            box_at(s > 0 ? LF_XO - EPS : -LF_XO - 6.6, s > 0 ? LF_XO + 6.6 : -LF_XO + EPS, LF_Y0 + 1, 74, TT_TOP, TT_TOP + M3_INSERT_L + MIN_WALL);
            box_at(s > 0 ? LF_XO - EPS : -LF_XO - 0.1, s > 0 ? LF_XO + 0.1 : -LF_XO + EPS, LF_Y0 + 1, 74, TT_TOP, TT_TOP + 10);
        }
        for (c = CHEEKS) box_at(c[0], c[1], NIP_Y - 11.6, NIP_Y + 11.6, WHEEL_Z - 9.1, LF_TOP_Z0 + EPS);
        for (s = [-1, 1]) {   // finger posts (set back) + short stop tabs at the load plane
            box_at(s * FINGER_X - 3, s * FINGER_X + 3, LF_Y0, LF_Y0 + 3, Z_LOAD - 1, LF_TOP_Z0 + EPS);
            box_at(s * FINGER_X - 3, s * FINGER_X + 3, FINGER_Y, LF_Y0 + EPS, Z_LOAD - 0.6, Z_LOAD + 2.4);
        }
        box_at(EXITIR_X - 3.5, EXITIR_X + 3.5, LF_Y0, EXITIR_Y + 3.5, e + 2, LF_TOP_Z0 + EPS);
    }
    for (c = CHEEKS) translate([c[0] - 1, NIP_Y, WHEEL_Z]) rotate([0, 90, 0]) rotate(90) linear_extrude(c[1] - c[0] + 2) m130_profile2d(GAP);
    translate([EXITIR_X, EXITIR_Y, e + 2 - EPS]) { cylinder(d = fit_free(3), h = 1.6); cylinder(d = fit_slide(3.8), h = 7.6); }
    translate([EXITIR_X, EXITIR_Y, e + 8]) box_at(-1, 1, -1, 4, 0, 60);
    for (p = FIX_LAUNCH) translate([p[0], p[1], TT_TOP]) insert_hole();
    for (s = [-1, 1]) translate([s * LF_XI, SP_INS[0], SP_INS[1]]) rotate([0, 90, 0]) cylinder(d = M3_CLEAR, h = 10, center = true);   // slot-panel screws
    for (s = [-1, 1]) translate([s * LF_XI, LFL_INS[0], e + LFL_INS[1]]) rotate([0, 90, 0]) cylinder(d = M3_CLEAR, h = 10, center = true);
    translate([0, (LF_Y0 + LF_Y1) / 2 + 4, LF_TOP_Z0 - 1]) linear_extrude(10) rrect([50, 14], 4);   // lightening / wire pass
}
module launcher_frame_print() translate([0, 0, LF_TOP_Z1]) rotate([180, 0, 0]) launcher_frame();
launcher_frame_print();
