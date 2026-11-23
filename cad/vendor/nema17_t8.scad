// Vendor model: NEMA17 40 mm with integrated T8x8 leadscrew (rack drive). Local: body z 0..40, face up, screw +Z, length LEADSCREW_L from the face.
include <../params.scad>
include <../lib/lib.scad>
use <nema17_pancake.scad>
module vendor_nema17_t8(screw = LEADSCREW_L) {
    difference() {
        nema17_body(NEMA_T8_L);
        for (x = [-1, 1], y = [-1, 1]) translate([x * NEMA_HOLE / 2, y * NEMA_HOLE / 2, NEMA_T8_L - 4]) cylinder(d = 2.6, h = 5);
    }
    color([0.7,0.7,0.72]) translate([0, 0, NEMA_T8_L]) cylinder(d = NEMA_BOSS_D, h = NEMA_BOSS_H);
    color([0.8,0.8,0.82]) translate([0, 0, NEMA_T8_L]) cylinder(d = LEAD_D, h = screw);
}

vendor_nema17_t8();
