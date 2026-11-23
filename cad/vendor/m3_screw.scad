// Vendor model: M3 socket-head cap screw (representative fastener). Local: head z 0..3 on top of z=0, shank toward -Z.
include <../params.scad>
include <../lib/lib.scad>
module vendor_m3_screw(l = 8) color([0.25,0.25,0.27]) difference() { union() { cylinder(d = 5.5, h = 3); translate([0, 0, -l]) cylinder(d = 2.9, h = l); } translate([0, 0, 1.5]) cylinder(d = 2.5, h = 3, $fn = 6); }

vendor_m3_screw();
