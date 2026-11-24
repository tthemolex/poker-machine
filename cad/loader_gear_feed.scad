// Loader drive gear on the feed shaft (-X end), m0.5 x 43T. Meshes the 21T pinch gear once,
// so the pinch shaft turns the opposite way: feed rollers (under the card) and pinch rollers
// (above the card) both push the card +Y into the rack (D-18). Print: flat, no supports.
include <params.scad>
include <lib/lib.scad>
// angle (in the gear plane: 2D x = world Y, 2D y = world Z) from the feed axis to the pinch axis
LG_ANG = atan2(PINCH_Z - FEED_Z, PINCH_Y - FEED_Y);
module lg_place(y, z) translate([LG_X - LG_W / 2, y, z]) multmatrix([[0, 0, 1, 0], [1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1]]) children();
module loader_gear2d(n, rot) difference() { rotate(rot) involute_gear2d(LG_M, n); circle(d = fit_press(SHAFT_D)); }
module loader_gear_feed() lg_place(FEED_Y, FEED_Z) linear_extrude(LG_W) loader_gear2d(LG_T_FEED, LG_ANG);     // tooth on the centre line
module loader_gear_feed_print() linear_extrude(LG_W) loader_gear2d(LG_T_FEED, 0);
loader_gear_feed_print();
