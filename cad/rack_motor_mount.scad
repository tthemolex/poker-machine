// Rack NEMA17 (integrated T8) mount: motor stands face-up on the turntable, plate on
// the motor face (4 x M3), two side legs, feet to the turntable (4 x CSK M3 from below).
// Print: upright; the top plate bridges 47 mm between the legs (no supports).
include <params.scad>
include <lib/lib.scad>
LMM_LX0 = LM_X - 14; LMM_LX1 = LM_X + 14;     // leg X span
LMM_YL  = NEMA_W / 2 + 0.25;                   // leg inner |Y|
module lmm_plate2d() union() {
    translate([LM_X, LM_Y]) offset(delta = 3.5, chamfer = true) square(2 * 15.5 + 0.1, center = true);
    translate([(LMM_LX0 + LMM_LX1) / 2, LM_Y]) square([LMM_LX1 - LMM_LX0, 2 * (LMM_YL + 2)], center = true);
}
module rack_motor_mount() difference() {
    union() {
        translate([0, 0, LM_FACE]) linear_extrude(LM_PLATE_T) lmm_plate2d();
        for (s = [-1, 1]) box_at(LMM_LX0, LMM_LX1, s > 0 ? LMM_YL : -LMM_YL - 2, s > 0 ? LMM_YL + 2 : -LMM_YL, TT_TOP, LM_FACE + LM_PLATE_T);
        // feet + motor seat
        translate([0, 0, TT_TOP]) linear_extrude(LM_Z0 - TT_TOP) translate([LM_X, LM_Y]) offset(delta = NEMA_CH, chamfer = true) square(NEMA_W - 2 * NEMA_CH, center = true);
        for (s = [-1, 1]) hull() for (p = FIX_LMM) if (sign(p[1]) == s) translate([p[0], p[1], TT_TOP]) cylinder(d = 9, h = 4);
        for (s = [-1, 1]) box_at(LMM_LX0, LMM_LX1, s > 0 ? LMM_YL : -30, s > 0 ? 30 : -LMM_YL, TT_TOP, TT_TOP + 4);
        box_at(LMM_LX0, LMM_LX1, -30, 30, TT_TOP, LM_Z0);
    }
    translate([LM_X, LM_Y, LM_FACE - 1]) cylinder(d = NEMA_BOSS_D + 2, h = 10);
    for (x = [-1, 1], y = [-1, 1]) translate([LM_X + x * NEMA_HOLE / 2, LM_Y + y * NEMA_HOLE / 2, LM_FACE]) m3_clear(LM_PLATE_T);
    for (p = FIX_LMM) translate([p[0], p[1], TT_TOP]) insert_hole();
    translate([LM_X, LM_Y, TT_TOP - 1]) cylinder(d = 26, h = 5);     // motor rear vent / label
}
module rack_motor_mount_print() translate([0, 0, -TT_TOP]) rack_motor_mount();
rack_motor_mount_print();
