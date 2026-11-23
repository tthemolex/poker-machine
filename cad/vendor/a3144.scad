// Vendor model: A3144 hall-effect switch, TO-92S, bent flat. Local: sensing face up, centred, body z 0..1.5.
include <../params.scad>
include <../lib/lib.scad>
module vendor_a3144() {
    color([0.1,0.1,0.1]) translate([-2, -1.5, 0]) cube([4, 3, 1.5]);
    color([0.8,0.8,0.8]) for (x = [-1.27, 0, 1.27]) translate([x - 0.2, -14, 0.5]) cube([0.4, 12.5, 0.4]);
}

vendor_a3144();
