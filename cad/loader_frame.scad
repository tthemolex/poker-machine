// Loader frame: walls from the turntable to the hopper floor, feed-shaft bearings,
// N20 cradle column (+X), ledges carrying the pusher guide, tabs for the hopper.
// Print: upright, bottom lip on bed; ledges have 45 deg under-chamfers; no supports.
include <params.scad>
include <lib/lib.scad>
use <hopper_body.scad>
LF_Z1 = Z_LOAD - HOP_FLOOR_T;
IRRX_LENS = Z_LOAD - HOP_FLOOR_T - 0.1;   // receiver lens just under the floor   // geom
IRRX_TOP = LF_Z1 - 1.5 * GAP;
N20C_X1 = N20_FACE_X + 10.5;
module loader_frame() difference() {
    union() {
        translate([0, 0, TT_TOP]) linear_extrude(LF_Z1 - TT_TOP) difference() { hop_outline2d(HOP_WALL); hop_outline2d(0); }
        // bottom lip
        translate([0, 0, TT_TOP]) linear_extrude(3) difference() { hop_outline2d(HOP_WALL); offset(delta = -8) hop_outline2d(0); }
        // ledges for the pusher-guide wings (45 deg chamfer below)
        for (p = WING_FIX) hull() {
            translate([p[0] + sign(p[0]) * 0.5, p[1], LEDGE_Z0]) rbox([8.5, 8, WING_Z0 - LEDGE_Z0], 0.5);   // geom
            translate([sign(p[0]) * (HOP_XI + 0.5), p[1], LEDGE_Z0 - 8]) cube([1, 8, 0.1], center = true);   // geom
        }
        // tabs for the hopper body
        for (s = [-1, 1], y = HOP_TAB_Y) hull() {
            translate([s * (HOP_XO + 3), y, LF_Z1 - 7]) rbox([8, 8, 7], 1.5);
            translate([s * (HOP_XO + 0.5), y, LF_Z1 - 15]) cube([1, 8, 0.1], center = true);   // geom
        }
        // N20 column with cradle
        box_at(HOP_XO - EPS, N20C_X1, FEED_Y - 7.6, FEED_Y + 7.6, TT_TOP, FEED_Z - 5.2);
        for (sy = [-1, 1]) box_at(N20_FACE_X - 1, N20C_X1, FEED_Y + sy * 6.2 - (sy > 0 ? 0 : 1.4), FEED_Y + sy * 6.2 + (sy > 0 ? 1.4 : 0), FEED_Z - 5.3, FEED_Z + 2);
        // shaft bearing bosses
        translate([-(HOP_XI - 1), FEED_Y, FEED_Z]) rotate([0, 90, 0]) cylinder(d = 8, h = 4, center = true);
        // turntable fixing bosses (full-depth inserts)
        for (p = FIX_LOADER) translate([p[0], p[1], TT_TOP]) rbox([9, 9, M3_INSERT_L + 1.2], 1);
        // load IR receiver block (lens up under the floor hole), 45 deg chamfer to the +X wall
        hull() {
            box_at(LOADIR_X - 3.5, HOP_XI + EPS, LOADIR_Y - 3.5, LOADIR_Y + 3.5, Z_EJECT - 7, IRRX_TOP);
            box_at(HOP_XI - 0.1, HOP_XI + EPS, LOADIR_Y - 3.5, LOADIR_Y + 3.5, Z_EJECT - 7 - (HOP_XI - LOADIR_X + 3.5), IRRX_TOP);   // geom
        }
    }
    // hollow column (open bottom) + cradle pocket + coupler channel
    box_at(HOP_XO + 2, N20C_X1 - 2, FEED_Y - 5.6, FEED_Y + 5.6, TT_TOP - 1, FEED_Z - 9);
    translate([N20_FACE_X - GAP, FEED_Y, FEED_Z]) box_at(0, 30, -6.2, 6.2, -5.2, 20);
    translate([HOP_XI - 2, FEED_Y, FEED_Z]) rotate([0, 90, 0]) cylinder(d = 10, h = N20_FACE_X - HOP_XI + 2);  // coupler channel (N20 gearbox is the +X bearing)
    translate([HOP_XI - 2, FEED_Y, FEED_Z]) box_at(0, N20_FACE_X - HOP_XI + 2, -5, 5, 0, 20);
    translate([N20_FACE_X + 4, FEED_Y, FEED_Z - 7.6]) box_at(0, 4, -10, 10, 0, 1.2);      // zip-tie tunnel under the cradle
    // shaft bores
    for (s = [-1, 1]) translate([s * HOP_XI, FEED_Y, FEED_Z]) rotate([0, 90, 0]) cylinder(d = fit_free(SHAFT_D), h = 20, center = true);
    // IR receiver bore + pin / lead exit
    translate([LOADIR_X, LOADIR_Y, IRRX_LENS - 7.5]) cylinder(d = fit_slide(3.8), h = 20);
    translate([LOADIR_X, LOADIR_Y, Z_EJECT - 20]) cylinder(d = fit_free(3), h = 20);
    // harness windows (D-19): -X to the rack NEMA, +X to the microswitch / launcher route
    box_at(-HOP_XO - 1, -HOP_XI + 1, -34, -18, TT_TOP + 5, TT_TOP + 25);
    box_at(HOP_XI - 1, HOP_XO + 1, -64, -46, TT_TOP + 5, TT_TOP + 25);
    // inserts
    for (p = WING_FIX) translate([p[0], p[1], WING_Z0 - M3_INSERT_L]) insert_hole(M3_INSERT_L + 1);
    for (s = [-1, 1], y = HOP_TAB_Y) translate([s * (HOP_XO + 3), y, LF_Z1 - M3_INSERT_L]) insert_hole(M3_INSERT_L + 1);
    for (p = FIX_LOADER) translate([p[0], p[1], TT_TOP]) insert_hole();
}
module loader_frame_print() translate([0, 0, -TT_TOP]) loader_frame();
loader_frame_print();
