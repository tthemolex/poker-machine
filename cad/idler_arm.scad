// Idler bell-crank: carries the 12 mm idler roller; a 5 mm spring pushes the lever
// end down so the idler presses the card up against the drive wheel.
// Print: standing on the lever-end face (Y vertical), holes horizontal, no supports.
include <params.scad>
include <lib/lib.scad>
e = Z_EJECT;
module idl_pt(p) translate([0, p[0], e + p[1]]) rotate([0, 90, 0]) children();
module idler_arm() difference() {
    union() {
        for (s = [-1, 1]) translate([s * 7.65, 0, 0]) {
            hull() { idl_pt(IDL_A) cylinder(r = 2.5, h = 2.3, center = true); idl_pt(IDL_P) cylinder(r = 2.5, h = 2.3, center = true); }
            hull() { idl_pt(IDL_P) cylinder(r = 2.5, h = 2.3, center = true); idl_pt(IDL_L) cylinder(r = 1.25, h = 2.3, center = true); }
        }
        translate([0, IDL_L[0], e + IDL_L[1]]) box_at(-8.8, 8.8, -2.5, 2.5, -1.25, 1.25);
    }
    idl_pt(IDL_A) cylinder(d = SHAFT_D - 0.05, h = 30, center = true);
    idl_pt(IDL_P) cylinder(d = SHAFT_D + 0.2, h = 30, center = true);
    translate([0, IDL_SPRING_Y, e + IDL_L[1] + 0.75]) cylinder(d = 5.6, h = 1);
}
module idler_arm_print() rotate([90, 0, 0]) translate([0, -(IDL_L[0] - 2.5), -e]) idler_arm();
idler_arm_print();
