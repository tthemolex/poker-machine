// Vendor model: 12 mm flush metal momentary push button (GO / SET). Local: front face z=0, body toward -Z.
include <../params.scad>
include <../lib/lib.scad>
module vendor_button_12mm() {
    color([0.72,0.73,0.75]) { translate([0, 0, -1]) cylinder(d = 14, h = 1); translate([0, 0, -15]) cylinder(d = 11.8, h = 14); }
    color([0.55,0.56,0.58]) translate([0, 0, -1.01]) cylinder(d = 9, h = 1.02);
    color([0.6,0.6,0.6]) translate([0, 0, -9]) cylinder(d = 15, h = 1.5, $fn = 6);
    color([0.85,0.7,0.2]) for (x = [-3, 3]) translate([x - 0.8, -0.2, -19]) cube([1.6, 0.4, 4]);
}

vendor_button_12mm();
