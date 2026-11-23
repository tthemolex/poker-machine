// Vendor model: Mini lever microswitch 12.8x5.8x6.5 (KW10 style), shown actuated. Local: body z 0..6.5, long side Y, lever on top.
include <../params.scad>
include <../lib/lib.scad>
module vendor_microswitch() {
    color([0.15,0.15,0.15]) difference() { translate([-2.9, -6.4, 0]) cube([5.8, 12.8, 6.5]); for (y = [-3.25, 3.25]) translate([-4, y, 1.75]) rotate([0, 90, 0]) cylinder(d = 2, h = 8); }
    color([0.85,0.85,0.88]) translate([-1.5, -5.5, 6.5]) cube([3, 11, 0.3]);
    color([0.8,0.8,0.8]) for (y = [-5, 0, 5]) translate([-0.25, y - 1, -3.5]) cube([0.5, 2, 3.5]);
}

vendor_microswitch();
