// Shaft coupler: N20 D-shaft (3 mm) to the 3 mm roller shaft, 2 x M3 grub screws.
// Print: upright.
include <params.scad>
include <lib/lib.scad>
CPL_X0 = SHAFT_X1 - 4;  CPL_X1 = N20_FACE_X - 2.5 * GAP;
module loader_coupler() translate([CPL_X0, FEED_Y, FEED_Z]) rotate([0, 90, 0]) difference() {
    cylinder(d = 8, h = CPL_X1 - CPL_X0);
    translate([0, 0, -1]) cylinder(d = fit_press(SHAFT_D) + TOL / 2, h = 30);
    for (z = [2.5, CPL_X1 - CPL_X0 - 2.5]) translate([0, 0, z]) rotate([0, 90, 0]) cylinder(d = PILOT_D, h = 10);   // M3 grub, self-tapped
}
module loader_coupler_print() rotate([0, -90, 0]) translate([-CPL_X0, -FEED_Y, -FEED_Z]) loader_coupler();
loader_coupler_print();
