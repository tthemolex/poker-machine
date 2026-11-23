// Launcher entry floor (top = Z_EJECT) between the side plates: idler pivot lugs,
// spring seat, exit IR receiver cup. Print: upside down (top face on bed), no supports.
include <params.scad>
include <lib/lib.scad>
e = Z_EJECT;
module launcher_floor() difference() {
    union() {
        box_at(-LF_XI + 0.1, LF_XI - 0.1, LF_Y0, LFL_Y1, e - 3, e);
        for (s = [-1, 1]) box_at(s > 0 ? 40 : -LF_XI + 0.1, s > 0 ? LF_XI - 0.1 : -40, LF_Y0, LFL_Y1, e - 9, e);
        for (s = [-1, 1]) hull() {
            box_at(s > 0 ? 9.2 : -11.7, s > 0 ? 11.7 : -9.2, LF_Y0 + 3, LFL_Y1, e - 3.5, e - 3 + EPS);
            translate([s * 10.45, IDL_P[0], e + IDL_P[1]]) rotate([0, 90, 0]) cylinder(r = 3, h = 2.5, center = true);
        }
        translate([EXITIR_X, EXITIR_Y, e - 10]) box_at(-3.2, 3.2, LF_Y0 - EXITIR_Y + 0.4, 3, 0, 7 + EPS);
    }
    for (s = [-1, 1]) translate([s * LF_XI, (LF_Y0 + LFL_Y1) / 2, e - 5]) rotate([0, -s * 90, 0]) insert_hole();
    translate([0, IDL_P[0], e + IDL_P[1]]) rotate([0, 90, 0]) cylinder(d = SHAFT_D + 0.2, h = 40, center = true);
    translate([0, IDL_SPRING_Y, e - 4]) cylinder(d = 5.6, h = 1.5);                          // spring seat
    translate([EXITIR_X, EXITIR_Y, e - 10.1]) { cylinder(d = 4.0, h = 9.6); cylinder(d = 3.2, h = 20); }
    translate([EXITIR_X, EXITIR_Y - 1, e - 10.1]) box_at(-1, 1, -2, 0, 0, 3);
}
module launcher_floor_print() translate([0, 0, e]) rotate([180, 0, 0]) launcher_floor();
launcher_floor_print();
