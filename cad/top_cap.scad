// Top cap: lift-off lid (rack, pusher and hopper access), chamfered+rounded edge,
// hopper loading opening with a drop-in lid at the back (-Y). 4 locating tabs.
// Print: upside down (top on bed); 1.5 mm lid ledge overhang is acceptable.
include <params.scad>
include <lib/lib.scad>
CAP_CH = 4;
LID_OPEN = [76, HOP_YI0 - HOP_YI1 - 7];  LID_YC = (HOP_YI0 + HOP_YI1) / 2;
// printable fillet: 45 deg chamfer from the top face blending into an r4 round (prints top-down).
// Built as a convex hull (rotate_extrude of an arc profile gives a broken mesh in this OpenSCAD build).
CAP_RHO = 4;  CAP_A = 5;
_zc = CAP_TOP_Z - (CAP_A - CAP_RHO * (1 - cos(45))) - CAP_RHO * sin(45);
module cap_outer() intersection() {
    hull() {
        translate([0, 0, _zc]) rotate_extrude($fn = 240) translate([TURRET_R - CAP_RHO, 0]) circle(r = CAP_RHO, $fn = 48);
        translate([0, 0, CAP_TOP_Z - 0.01]) cylinder(r = TURRET_R - CAP_A, h = 0.01, $fn = 240);
        translate([0, 0, SHELL_Z1]) cylinder(r = TURRET_R, h = _zc - SHELL_Z1, $fn = 240);
    }
    translate([0, 0, SHELL_Z1]) cylinder(r = TURRET_R + 1, h = CAP_TOP_Z - SHELL_Z1);
}
// inner surface = outer offset inward by ~CAP_T (keeps >= 1.8 mm at the rounded rim)
_x_in = (TURRET_R - CAP_A + CAP_TOP_Z) - CAP_T * sqrt(2) - CAP_IN_Z;
module cap_cavity() intersection() {
    hull() {
        translate([0, 0, _zc]) rotate_extrude($fn = 240) translate([TURRET_R - CAP_RHO, 0]) circle(r = CAP_RHO - SHELL_T, $fn = 48);
        translate([0, 0, CAP_IN_Z - 0.01]) cylinder(r = _x_in, h = 0.01, $fn = 240);
        translate([0, 0, SHELL_Z1 - 1]) cylinder(r = TURRET_RI, h = _zc - SHELL_Z1 + 1, $fn = 240);
    }
    translate([0, 0, SHELL_Z1 - 2]) cylinder(r = TURRET_R, h = CAP_IN_Z - SHELL_Z1 + 2);
}
module top_cap() difference() {
    union() {
        difference() { cap_outer(); cap_cavity(); }
        for (a = [0 : 90 : 359]) {
            translate([0, 0, SHELL_Z1 - 6]) ring_sector(TURRET_RI - 0.2, TURRET_RI - 2.2, 6 + EPS, a - 6, a + 6, $fn = 240);
            translate([0, 0, SHELL_Z1]) ring_sector(TURRET_RI + 0.5, TURRET_RI - 2.2, 3, a - 6, a + 6, $fn = 240);
        }
        translate([0, LID_YC, CAP_IN_Z - 1.5]) linear_extrude(1.5 + EPS) difference() { rrect(LID_OPEN + [6, 6], 5); rrect(LID_OPEN - [3, 3], 3.5); }
    }
    translate([0, LID_YC, CAP_IN_Z - 0.01]) linear_extrude(10) rrect(LID_OPEN, 5);
}
module top_cap_print() translate([0, 0, CAP_TOP_Z]) rotate([180, 0, 0]) top_cap();
top_cap_print();
