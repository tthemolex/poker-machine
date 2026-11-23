// Vendor model: NEMA17 pancake stepper 42x42x23 (turret drive). Local: body z 0..23, face up, shaft +Z.
include <../params.scad>
include <../lib/lib.scad>
module nema17_body(l) {
    color([0.15,0.15,0.16]) linear_extrude(l) offset(delta = NEMA_CH, chamfer = true) square(NEMA_W - 2 * NEMA_CH, center = true);
    color([0.75,0.75,0.78]) for (z = [0, l - 7]) translate([0, 0, z]) linear_extrude(7) offset(delta = NEMA_CH, chamfer = true) square(NEMA_W - 2 * NEMA_CH - 0.01, center = true);
}
module vendor_nema17_pancake(shaft = 15.5) {
    difference() {
        nema17_body(NEMA_PANCAKE_L);
        for (x = [-1, 1], y = [-1, 1]) translate([x * NEMA_HOLE / 2, y * NEMA_HOLE / 2, NEMA_PANCAKE_L - 4]) cylinder(d = 2.6, h = 5);
    }
    color([0.7,0.7,0.72]) translate([0, 0, NEMA_PANCAKE_L]) cylinder(d = NEMA_BOSS_D, h = NEMA_BOSS_H);
    color("silver") translate([0, 0, NEMA_PANCAKE_L]) cylinder(d = 5, h = shaft);
}

vendor_nema17_pancake();
