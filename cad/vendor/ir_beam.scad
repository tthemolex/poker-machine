// Vendor model: 3 mm IR break-beam sensor half (emitter or receiver), aims along -Z from z=0. Local: lens face z=0, body toward +Z.
include <../params.scad>
include <../lib/lib.scad>
module vendor_ir_beam(rx = false) {
    color(rx ? [0.1,0.1,0.1] : [0.2,0.2,0.5]) { cylinder(d = 3, h = 1.5); translate([0, 0, 1.5]) cylinder(d = 3.8, h = 6); }
    color([0.8,0.8,0.8]) for (x = [-1, 1]) translate([x - 0.2, -0.2, 7.5]) cube([0.4, 0.4, 3]);
}

vendor_ir_beam();
