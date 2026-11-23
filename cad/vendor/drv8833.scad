// Vendor model: DRV8833 dual H-bridge module. Local: PCB bottom z=0, centred.
include <../params.scad>
include <../lib/lib.scad>
module vendor_drv8833() {
    color([0.7,0.1,0.1]) translate([-9.25, -8.25, 0]) cube([18.5, 16.5, 1.6]);
    color([0.1,0.1,0.1]) translate([-3, -3, 1.6]) cube([6, 6, 1.2]);
    color([0.85,0.7,0.2]) for (x = [-1, 1], i = [0 : 5]) translate([x * 7.6 - 0.3, -6.35 + i * 2.54 - 0.3, -6]) cube([0.6, 0.6, 6]);
}

vendor_drv8833();
