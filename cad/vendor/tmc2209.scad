// Vendor model: TMC2209 StepStick driver (with heatsink), headers down. Local: PCB bottom z=0, centred, long side Y.
include <../params.scad>
include <../lib/lib.scad>
module vendor_tmc2209() {
    color([0.1,0.3,0.6]) translate([-7.62, -10.16, 0]) cube([15.24, 20.32, 1.6]);
    color([0.15,0.15,0.17]) translate([-4.5, -4.5, 1.6]) cube([9, 9, 6]);
    color([0.08,0.08,0.08]) for (x = [-1, 1]) translate([x * 6.35 - 1.27, -10.16, -2.5]) cube([2.54, 20.32, 2.5]);
    color([0.85,0.7,0.2]) for (x = [-1, 1], i = [0 : 7]) translate([x * 6.35 - 0.3, -8.89 + i * 2.54 - 0.3, -8.5]) cube([0.6, 0.6, 6]);
}

vendor_tmc2209();
