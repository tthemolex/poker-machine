// Vendor model: Rubber feed roller 14x8 mm, 3 mm bore (loader). Local: axis Z, centred.
include <../params.scad>
include <../lib/lib.scad>
module vendor_feed_roller() color([0.18,0.18,0.2]) difference() { translate([0, 0, -FEED_W / 2]) cylinder(d = FEED_D, h = FEED_W); cylinder(d = 3, h = 20, center = true); }

vendor_feed_roller();
