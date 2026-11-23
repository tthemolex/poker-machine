// Vendor model: MG90S metal-gear micro servo. Local: shaft axis +Z at (0,0); body bottom z=0, top face z=22.7, spline top z=29.7; body extends toward -X.
include <../params.scad>
include <../lib/lib.scad>
module vendor_mg90s() {
    color([0.1,0.1,0.12]) {
        translate([-5.5 - 11.4, -6.1, 0]) cube([22.8, 12.2, 22.7]);
        translate([-5.5 - 16.1, -6.1, 15.6]) difference() { cube([32.2, 12.2, 2.5]); for (x = [2.3, 32.2 - 2.3]) translate([x, 6.1, -1]) cylinder(d = 2.2, h = 5); }
        translate([0, 0, 22.7]) cylinder(d = 11.8, h = 4);
        translate([-8, 0, 22.7]) cylinder(d = 5, h = 2);
    }
    color([0.85,0.85,0.85]) translate([0, 0, 26.7]) cylinder(d = 4.8, h = 3);
}

vendor_mg90s();
