// Vendor model: GT2 20T pulley, 5 mm bore, for 10 mm belt, top flange removed (meshes the printed ring). Local: hub z 0..5, flange 5..6, teeth 6..14.
include <../params.scad>
include <../lib/lib.scad>
module vendor_gt2_pulley_20t() color([0.75,0.75,0.78]) difference() {
    union() {
        cylinder(d = PUL_HUB_D, h = PUL_HUB_H);
        translate([0, 0, PUL_HUB_H]) cylinder(d = PUL_HUB_D, h = PUL_FL_T);
        translate([0, 0, PUL_HUB_H + PUL_FL_T]) linear_extrude(PUL_TEETH_H) difference() {
            circle(r = PUL_PR - 0.254, $fn = 60);
            for (k = [0 : 19]) rotate(k * 18) translate([PUL_PR - 0.254 - 0.15, 0]) circle(r = 0.6, $fn = 16);
        }
    }
    translate([0, 0, -1]) cylinder(d = 5, h = 20);
    translate([0, 0, PUL_HUB_H / 2]) rotate([0, 90, 0]) cylinder(d = 3, h = 10);
}

vendor_gt2_pulley_20t();
