// Rack carriage: 4 x LM8UU (2 per rod, pressed in), screwed to the +X comb (4 x M3 thread-forming).
// Print: upright (bores vertical), no supports.
include <params.scad>
include <lib/lib.scad>
CARR_OD = fit_slide(LM8_D) + 2 * CARR_WALL;
module rack_carriage() translate([0, 0, RACK_Z_MIN]) difference() {
    union() {
        for (y = ROD_Y) translate([ROD_X, y, CARR_Z0]) cylinder(d = CARR_OD, h = CARR_Z1 - CARR_Z0);
        box_at(COMB_OUT, COMB_OUT + 4, ROD_Y[0] - CARR_OD / 2, ROD_Y[1] + CARR_OD / 2, CARR_Z0, CARR_Z1);
        for (y = ROD_Y) box_at(COMB_OUT, ROD_X, y - 4, y + 4, CARR_Z0, CARR_Z1);
        box_at(COMB_OUT, ROD_X, ROD_Y[0], ROD_Y[1], CARR_Z0, CARR_Z0 + 4);
        box_at(COMB_OUT, ROD_X, ROD_Y[0], ROD_Y[1], CARR_Z1 - 4, CARR_Z1);
    }
    for (y = ROD_Y) {
        translate([ROD_X, y, CARR_Z0 + 1.2]) cylinder(d = fit_slide(LM8_D), h = CARR_Z1 - CARR_Z0 - 2.4);
        translate([ROD_X, y, CARR_Z0 - 1]) cylinder(d = ROD_D + 2, h = 100);
    }
    for (h = CARR_HOLES) translate([COMB_OUT - 1, h[0], h[1]]) rotate([0, 90, 0]) { cylinder(d = M3_CLEAR, h = 10); translate([0, 0, 5]) cylinder(d = M3_HEAD_CB, h = 20); }
}
module rack_carriage_print() translate([0, 0, -RACK_Z_MIN - CARR_Z0]) rack_carriage();
rack_carriage_print();
