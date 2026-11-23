// Vendor model: Rubber idler roller 12x12 mm on 3 mm axle (launcher pinch idler). Local: axis Z, centred.
include <../params.scad>
include <../lib/lib.scad>
module vendor_idler_roller() color([0.2,0.2,0.22]) difference() { translate([0, 0, -IDLER_W / 2]) cylinder(d = IDLER_D, h = IDLER_W); cylinder(d = 3.1, h = 20, center = true); }

vendor_idler_roller();
