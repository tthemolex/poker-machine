// O-ring belt pulley on the feed shaft (-X end): groove Ø12 for a 1.5 mm cord. M3 grub.
// Print: flat, no supports (V groove 45 deg).
include <params.scad>
include <lib/lib.scad>
module pulley(dg, x0, y, z) translate([x0, y, z]) rotate([0, 90, 0]) difference() {
    rotate_extrude($fn = 64) polygon([[fit_free(SHAFT_D) / 2 + 0.01, 0], [dg / 2 + 1.2, 0], [dg / 2 + 1.2, 0.8], [dg / 2, 2.0], [dg / 2 + 1.2, 3.2], [dg / 2 + 1.2, 4], [fit_free(SHAFT_D) / 2 + 0.01, 4]]);   // 45 deg V groove
    translate([0, 0, -1]) cylinder(d = fit_press(SHAFT_D), h = 6);
}
module loader_pulley_feed() pulley(PUL_FEED_D, BELT_X - 2, FEED_Y, FEED_Z);
module loader_pulley_feed_print() translate([0, 0, 0]) rotate([0, -90, 0]) translate([-(BELT_X - 2), -FEED_Y, -FEED_Z]) loader_pulley_feed();
loader_pulley_feed_print();
