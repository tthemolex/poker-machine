// Launcher entry floor (top = Z_EJECT) between the side plates: idler pivot lugs,
// spring locating pin, exit IR receiver cup, end blocks with full-wall inserts.
// Print: upside down (top face on bed), no supports.
include <params.scad>
include <lib/lib.scad>
e = Z_EJECT;
LFL_XB = 38.5;   // end-block inner face |X|
module launcher_floor() difference() {
    union() {
        box_at(-LF_XI + GAP, LF_XI - GAP, LF_Y0, LFL_Y1, e - 3, e);
        for (s = [-1, 1]) box_at(s > 0 ? LFL_XB : -LF_XI + GAP, s > 0 ? LF_XI - GAP : -LFL_XB, LF_Y0, 62, e - 10.5, e);
        for (s = [-1, 1]) hull() {
            box_at(s > 0 ? 9.2 : -11.7, s > 0 ? 11.7 : -9.2, LF_Y0 + 3, LFL_Y1, e - 3.5, e - 3 + EPS);
            translate([s * 10.45, IDL_P[0], e + IDL_P[1]]) rotate([0, 90, 0]) cylinder(r = 3, h = 2.5, center = true);
        }
        translate([EXITIR_X, 0, e - 10]) box_at(-3.2, 3.2, LF_Y0, EXITIR_Y + 3.4, 0, 10);
        translate([0, IDL_SPRING_Y, e - 4.5]) cylinder(d = PEG_D, h = 1.5 + EPS);        // spring locating pin (spring ID 4)
    }
    for (s = [-1, 1]) translate([s * (LF_XI - GAP), LFL_INS[0], e + LFL_INS[1]]) rotate([0, -s * 90, 0]) insert_hole();
    translate([0, IDL_P[0], e + IDL_P[1]]) rotate([0, 90, 0]) cylinder(d = fit_free(SHAFT_D), h = 25, center = true);
    translate([EXITIR_X, EXITIR_Y, e - 11]) { cylinder(d = fit_slide(3.8), h = 9.5); cylinder(d = fit_free(3), h = 20); }   // 1.5 mm roof over the receiver
    translate([EXITIR_X, EXITIR_Y, e - 11]) box_at(-1, 1, -0.5, 0.5, 0, 4);
}
module launcher_floor_print() translate([0, 0, e]) rotate([180, 0, 0]) launcher_floor();
launcher_floor_print();
