// Vendor model: 0.5 mm spring-steel pusher blade, BLADE_W x BLADE_L (cut from shim stock). Local: tip at y=0, extends -Y, z 0..0.5.
include <../params.scad>
include <../lib/lib.scad>
module vendor_pusher_blade() color([0.7,0.72,0.76]) difference() {
    translate([0, -BLADE_L, 0]) linear_extrude(BLADE_T) hull() { translate([-BLADE_W / 2, 0]) square([BLADE_W, 1]); for (x = [-1, 1]) translate([x * (BLADE_W / 2 - 2), BLADE_L - 2]) circle(r = 2); }
    for (x = [-7, 7]) translate([x, -BLADE_L + 7.5, -1]) cylinder(d = 2.4, h = 3);
}

vendor_pusher_blade();
