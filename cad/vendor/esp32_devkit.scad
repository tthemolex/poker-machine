// Vendor model: ESP32 DevKit V1 (WROOM-32, 30 pin), headers down. Local: PCB bottom z=0, centred, long side X.
include <../params.scad>
include <../lib/lib.scad>
module vendor_esp32_devkit() {
    color([0.1,0.1,0.12]) translate([-25.75, -14.15, 0]) cube([51.5, 28.3, 1.6]);
    color([0.6,0.6,0.62]) translate([25.75 - 18, -9, 1.6]) cube([18, 18, 3.2]);
    color([0.75,0.75,0.75]) translate([-25.75 - 0.5, -4, 1.6]) cube([6, 8, 3]);
    color([0.08,0.08,0.08]) for (y = [-1, 1]) translate([-19, y * 12.7 - 1.27, -2.5]) cube([38.1, 2.54, 2.5]);
    color([0.85,0.7,0.2]) for (y = [-1, 1], i = [0 : 14]) translate([-17.78 + i * 2.54 - 0.3, y * 12.7 - 0.3, -8.5]) cube([0.6, 0.6, 6]);
}

vendor_esp32_devkit();
