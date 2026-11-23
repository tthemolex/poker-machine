// Vendor model: 1.5 mm nitrile O-ring cord belt (feed shaft -> pinch shaft, -X side).
// Local: belt loop in the YZ plane at x = 0 (world: X = BELT_X), modelled as the cord core.
include <../params.scad>
include <../lib/lib.scad>
module vendor_o_ring_belt() color([0.15,0.15,0.15]) rotate([90, 0, 90]) linear_extrude(0.8, center = true) difference() {
    hull() { translate([FEED_Y, FEED_Z]) circle(r = PUL_FEED_D / 2 + 1.4); translate([PINCH_Y, PINCH_Z]) circle(r = PUL_PINCH_D / 2 + 1.4); }
    hull() { translate([FEED_Y, FEED_Z]) circle(r = PUL_FEED_D / 2 + 0.4); translate([PINCH_Y, PINCH_Z]) circle(r = PUL_PINCH_D / 2 + 0.4); }
}
vendor_o_ring_belt();
