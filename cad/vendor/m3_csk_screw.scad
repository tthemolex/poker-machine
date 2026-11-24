// Vendor model: M3 countersunk screw (ISO 10642), head flush at z = 0, shank toward +Z.
include <../params.scad>
include <../lib/lib.scad>
module vendor_m3_csk_screw(l = 10) color([0.3,0.3,0.32]) { cylinder(d1 = 6.0, d2 = 2.9, h = 1.65); cylinder(d = 2.9, h = l); }
vendor_m3_csk_screw();
