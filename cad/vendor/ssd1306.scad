// Vendor model: SSD1306 0.96in 128x64 I2C OLED module. Local: glass front face z=0 (viewing +Z), PCB behind, centred.
include <../params.scad>
include <../lib/lib.scad>
module vendor_ssd1306() {
    color([0.05,0.05,0.08]) translate([-13.35, -9.65 - 2, -1.5]) cube([26.7, 19.3, 1.5]);
    color([0.1,0.2,0.6]) translate([-10.87, -5.43 - 1, -0.01]) cube([21.74, 10.86, 0.02]);
    color([0.1,0.25,0.55]) translate([-13.65, -13.9, -2.7]) cube([27.3, 27.8, 1.2]);
    color([0.85,0.7,0.2]) for (i = [0 : 3]) translate([-3.81 + i * 2.54 - 0.3, 11.5 - 0.3, -8.7]) cube([0.6, 0.6, 6]);
}

vendor_ssd1306();
