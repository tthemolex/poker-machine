// Hopper: floor (load plane = Z_LOAD) with feed-roller windows and the load IR hole,
// walls up to the cap. Front throat closed by the separate adjustable gate.
// Print: upright, floor on bed, no supports.
include <params.scad>
include <lib/lib.scad>
module hop_outline2d(grow = 0) {   // XY outline of the card zone grown by `grow`, front squared off at HOP_Y_FRONT
    hull() {
        for (s = [-1, 1]) translate([s * (HOP_XI - HOP_CR), HOP_YI1 + HOP_CR]) circle(r = HOP_CR + grow);
        translate([-(HOP_XI + grow), HOP_YI1 + HOP_CR]) square([2 * (HOP_XI + grow), HOP_Y_FRONT - HOP_YI1 - HOP_CR]);
    }
}
module hopper_body() difference() {
    union() {
        translate([0, 0, Z_LOAD - HOP_FLOOR_T + 4]) linear_extrude(HOP_Z1 - Z_LOAD + HOP_FLOOR_T - 4) hop_outline2d(HOP_WALL);
        translate([0, 0, Z_LOAD - HOP_FLOOR_T]) linear_extrude(4 + EPS) union() { hop_outline2d(HOP_WALL); for (s = [-1, 1], y = HOP_TAB_Y) translate([s * (HOP_XO + 3), y]) rrect([8, 8], 1.5); }
    }
    translate([0, 0, Z_LOAD]) linear_extrude(200) difference() { hop_outline2d(0); translate([-50, HOP_YI0]) square([100, 20]); }
    translate([0, 0, Z_LOAD]) box_at(-HOP_XI, HOP_XI, HOP_YI0 - 1, HOP_Y_FRONT + 1, 0, 200);        // throat / gate bay
    for (s = [-1, 1]) translate([s * FEED_X, FEED_Y, Z_LOAD - 5]) rbox([FEED_W + 4 * GAP, FEED_D + 4 * GAP, 10], 1);   // roller windows
    translate([LOADIR_X, LOADIR_Y, Z_LOAD - 5]) cylinder(d = fit_free(3), h = 10);
    for (s = [-1, 1], z = [14, 32]) translate([s * HOP_XI, GATE_YC, Z_LOAD + z]) hull() for (dz = [-1.5, 1.5]) translate([0, 0, dz]) rotate([0, 90, 0]) cylinder(d = M3_CLEAR, h = 10, center = true);
    translate([-HOP_XI - HOP_WALL / 2, PINCH_Y, PINCH_Z]) hull() for (dz = [-1.5, 2.5], dy = [0, 10]) translate([0, dy, dz]) rotate([0, 90, 0]) cylinder(d = fit_free(SHAFT_D), h = HOP_WALL + 2, center = true);   // open U-slot for the pinch shaft (throat adjust + float)
    for (s = [-1, 1], y = HOP_TAB_Y) translate([s * (HOP_XO + 3), y, Z_LOAD - HOP_FLOOR_T]) m3_clear(5);
}
module hopper_body_print() translate([0, 0, -Z_LOAD + HOP_FLOOR_T]) hopper_body();
hopper_body_print();
