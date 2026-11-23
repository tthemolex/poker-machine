// O-ring belt pulley on the pinch shaft (-X end): groove Ø5 (pinch surface speed 1.03 x feed).
// Print: flat, no supports.
include <params.scad>
include <lib/lib.scad>
use <loader_pulley_feed.scad>
module loader_pulley_pinch() pulley(PUL_PINCH_D, BELT_X - 2, PINCH_Y, PINCH_Z);
module loader_pulley_pinch_print() rotate([0, -90, 0]) translate([-(BELT_X - 2), -PINCH_Y, -PINCH_Z]) loader_pulley_pinch();
loader_pulley_pinch_print();
