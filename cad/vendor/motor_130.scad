// Vendor model: 130-size brushed DC motor (launcher). Local: face z=0, shaft +Z (2 mm x 8), body toward -Z, flats +-7.5 in X.
include <../params.scad>
include <../lib/lib.scad>
module vendor_motor_130() {
    color([0.78,0.78,0.8]) translate([0, 0, -25]) intersection() { cylinder(d = 20, h = 25); translate([-7.5, -11, 0]) cube([15, 22, 25]); }
    color([0.85,0.85,0.85]) cylinder(d = 6, h = 1.5);
    color([0.9,0.9,0.9]) translate([0, 0, -27]) cylinder(d = 8, h = 2);
    color("silver") cylinder(d = 2, h = 8);
}

vendor_motor_130();
