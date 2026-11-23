// Vendor model: 5 mm status LED in a panel bezel hole. Local: dome tip z=0, body toward -Z.
include <../params.scad>
include <../lib/lib.scad>
module vendor_led_5mm() {
    color([0.2,0.9,0.3,0.9]) { translate([0, 0, -2.5]) sphere(d = 5); translate([0, 0, -7.6]) cylinder(d = 5, h = 5.1); translate([0, 0, -8.6]) cylinder(d = 5.8, h = 1); }
    color([0.8,0.8,0.8]) for (x = [-1.27, 1.27]) translate([x - 0.25, -0.25, -20]) cube([0.5, 0.5, 11.4]);
}
vendor_led_5mm();
