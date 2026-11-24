// Pinch drive gear on the pinch shaft (-X end), m0.5 x 21T (pinch surface speed 1.024 x feed).
// Print: flat, no supports.
include <params.scad>
include <lib/lib.scad>
use <loader_gear_feed.scad>
LG_ANG2 = atan2(PINCH_Z - FEED_Z, PINCH_Y - FEED_Y);
module loader_gear_pinch() lg_place(PINCH_Y, PINCH_Z) linear_extrude(LG_W) loader_gear2d(LG_T_PINCH, LG_ANG2 + 180 + 180 / LG_T_PINCH);   // gap faces the feed gear tooth
module loader_gear_pinch_print() linear_extrude(LG_W) loader_gear2d(LG_T_PINCH, 0);
loader_gear_pinch_print();
