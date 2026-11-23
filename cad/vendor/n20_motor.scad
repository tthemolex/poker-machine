// Vendor model: N20 micro gearmotor (5 V, ~100 rpm), D-shaft 3 mm. Local: output face z=0, shaft +Z 0..10, body toward -Z.
include <../params.scad>
include <../lib/lib.scad>
module vendor_n20_motor() {
    color([0.85,0.7,0.3]) translate([-6, -5, -9]) cube([12, 10, 9]);
    color([0.75,0.75,0.78]) translate([0, 0, -24]) intersection() { cylinder(d = 12, h = 15); translate([-6, -5, 0]) cube([12, 10, 15]); }
    color([0.2,0.2,0.2]) translate([-5, -4, -25.5]) cube([10, 8, 1.5]);
    color("silver") difference() { cylinder(d = 3, h = 10); translate([0.9, -2, 1]) cube([2, 4, 10]); }
}

vendor_n20_motor();
