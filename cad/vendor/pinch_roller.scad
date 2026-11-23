// Vendor model: rubber pinch roller Ø6 x 6 mm, 3 mm bore (loader pinch, x2). Local: axis Z, centred.
include <../params.scad>
include <../lib/lib.scad>
module vendor_pinch_roller() color([0.2,0.2,0.22]) difference() { translate([0, 0, -PINCH_W / 2]) cylinder(d = PINCH_D, h = PINCH_W); cylinder(d = SHAFT_D, h = 20, center = true); }
vendor_pinch_roller();
