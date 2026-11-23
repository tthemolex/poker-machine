// Vendor model: Rubber drive wheel 24x10 mm, 2 mm bore (launcher). Local: axis Z, centred.
include <../params.scad>
include <../lib/lib.scad>
module vendor_wheel_launcher() color([0.12,0.12,0.12]) difference() { translate([0, 0, -WHEEL_W / 2]) cylinder(d = WHEEL_D, h = WHEEL_W); cylinder(d = 2, h = 20, center = true); }

vendor_wheel_launcher();
