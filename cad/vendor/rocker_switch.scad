// Vendor model: Mini rocker switch KCD11 (15x10.5 bezel, cut-out 13x8.7). Local: front face z=0, body toward -Z, long side X.
include <../params.scad>
include <../lib/lib.scad>
module vendor_rocker_switch() {
    color([0.08,0.08,0.08]) { translate([-7.5, -5.25, -1.5]) cube([15, 10.5, 1.5]); translate([-6.5, -4.3, -13.5]) cube([13, 8.6, 12]); }
    color([0.15,0.15,0.15]) translate([-6, -3.8, -1.2]) cube([12, 7.6, 1.4]);
    color([0.8,0.8,0.8]) for (x = [-3.5, 3.5]) translate([x - 1.5, -0.3, -19]) cube([3, 0.6, 5.5]);
}

vendor_rocker_switch();
