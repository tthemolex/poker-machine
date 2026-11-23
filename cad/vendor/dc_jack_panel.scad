// Vendor model: Panel-mount DC jack 5.5x2.1 mm, M8 thread (charge input). Local: front face z=0, body toward -Z.
include <../params.scad>
include <../lib/lib.scad>
module vendor_dc_jack_panel() {
    color([0.1,0.1,0.1]) difference() { translate([0, 0, -1.5]) cylinder(d = 10, h = 1.5); translate([0, 0, -1]) cylinder(d = 5.5, h = 2); }
    color([0.15,0.15,0.15]) translate([0, 0, -14]) cylinder(d = 7.8, h = 12.5);
    color([0.8,0.8,0.8]) for (a = [0, 120, 240]) rotate(a) translate([2.5, -1, -18]) cube([0.5, 2, 4]);
}

vendor_dc_jack_panel();
