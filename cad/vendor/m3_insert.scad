// Vendor model: M3 x 5.7 brass heat-set insert, OD 4.6 (representative). Local: top flush z=0, body toward -Z.
include <../params.scad>
include <../lib/lib.scad>
module vendor_m3_insert() color([0.85,0.65,0.25]) difference() { translate([0, 0, -5.7]) cylinder(d = 4.6, h = 5.7); translate([0, 0, -6]) cylinder(d = 3, h = 7); }

vendor_m3_insert();
