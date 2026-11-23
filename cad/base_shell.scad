// Static base puck: floor with battery door opening, battery cradle, deck ledge,
// UI notch (bezel), rocker + DC jack holes, electronics standoffs.
// Print: upright, floor on bed, no supports (UI cut is a notch from the top edge).
include <params.scad>
include <lib/lib.scad>

module base_profile2d() {   // r,z half-profile of the outer skin
    polygon([[60, 0], [TURRET_R - 2, 0], [TURRET_R, 2], [TURRET_R, BASE_TOP_Z - 0.8], [TURRET_R - 0.8, BASE_TOP_Z], [60, BASE_TOP_Z]]);
}
module radial_hole(a, z, d, depth = 10) { rotate(a) translate([TURRET_R - depth + 1, 0, z]) rotate([0, 90, 0]) cylinder(d = d, h = depth + 2); }

module base_shell() {
    difference() {
        union() {
            difference() {
                union() { rotate_extrude($fn = 240) base_profile2d(); cylinder(r = 61, h = BASE_TOP_Z, $fn = 240); }
                translate([0, 0, FLOOR_T]) cylinder(r = BASE_RI, h = 100, $fn = 240);
            }
            // battery cradle walls (deck rests on them)
            difference() {
                rbox_at(-CR_XO, CR_XO, -CR_YO, CR_YO, 0, DECK_Z, 2);
                rbox_at(-CR_XI, CR_XI, -CR_YI, CR_YI, -1, DECK_Z + 1, 1);
                box_at(CR_XI - 1, CR_XO + 1, -26, 30, FLOOR_T + 4, DECK_Z + 1);   // battery plug / USB exit
            }
            // raised floor ring over the door rebate
            difference() {
                rbox_at(-CR_XI - DOOR_REBATE - 1.5, CR_XI + DOOR_REBATE + 1.5, -CR_YI - DOOR_REBATE - 1.5, CR_YI + DOOR_REBATE + 1.5, 0, FLOOR_T + DOOR_RING_T, 4);
                rbox_at(-CR_XI, CR_XI, -CR_YI, CR_YI, -1, 10, 1);
            }
            // local wall thickening behind the flush rocker / jack recesses
            rotate(ROCKER_A) translate([BASE_RI - 1.5, 0, PWR_Z]) cube([4, 20, 15], center = true);
            rotate(JACK_A) translate([BASE_RI - 1.5, 0, PWR_Z]) rotate([0, 90, 0]) cylinder(d = 15, h = 4, center = true);
            // deck ledge ring
            difference() { cylinder(r = BASE_RI, h = DECK_Z, $fn = 240); translate([0, 0, -1]) cylinder(r = BASE_RI - 2.5, h = DECK_Z + 2, $fn = 240); }
            // deck screw bosses (insert at top)
            for (a = DECK_SCREW_A) rotate(a) translate([DECK_SCREW_R, 0, 0]) cylinder(d = 8, h = DECK_Z);
            // battery door screw bosses
            for (p = DOOR_SCREWS) translate([p[0], p[1], 0]) cylinder(d = 8, h = FLOOR_T + 6);
            // electronics supports: ESP32 end blocks, driver centre blocks, buck pad
            for (s = [-1, 1]) translate(ESP_POS + [s * 22.5, 0, 0]) box_at(-2.5, 2.5, -8, 8, -ESP_POS[2] + 0.5, 0);
            for (p = TMC_POS) translate(p) box_at(-4, 4, -7, 7, -p[2] + 0.5, 0);
            translate(DRV_POS) box_at(-5, 5, -7, 7, -DRV_POS[2] + 0.5, 0);
            translate(M560_POS) box_at(-7, 7, -9, 9, -M560_POS[2] + 0.5, 0);
        }
        // clearance behind the UI bezel (ledge / lips)
        intersection() { cylinder(r = BASE_RI + 0.01, h = 100, $fn = 240); box_at(-UI_NOTCH_X - 6, UI_NOTCH_X + 6, -TURRET_R, -75, UI_NOTCH_Z0, 50); }
        // battery door opening + rebate (underside)
        rbox_at(-CR_XI, CR_XI, -CR_YI, CR_YI, -1, FLOOR_T + 1, 1);
        rbox_at(-CR_XI - DOOR_REBATE, CR_XI + DOOR_REBATE, -CR_YI - DOOR_REBATE, CR_YI + DOOR_REBATE, -1, DOOR_REBATE_T, 3);
        for (p = DOOR_SCREWS) translate([p[0], p[1], DOOR_REBATE_T]) insert_hole(6);
        for (a = DECK_SCREW_A) rotate(a) translate([DECK_SCREW_R, 0, DECK_Z - M3_INSERT_L]) insert_hole();
        // UI notch (bezel sits here)
        box_at(-UI_NOTCH_X, UI_NOTCH_X, -TURRET_R - 5, -TURRET_R + 15, UI_NOTCH_Z0, BASE_TOP_Z + 1);
        // rocker switch (13 x 8.7 cut-out) and DC jack (M8)
        rotate(ROCKER_A) translate([TURRET_R - 6, 0, PWR_Z]) cube([12, 13.2, 8.9], center = true);
        rotate(ROCKER_A) translate([TURRET_R - 1.6 - UI_INSET + 2, 0, PWR_Z]) cube([4, 15.4, 10.9], center = true);
        radial_hole(JACK_A, PWR_Z, 8.2);
        rotate(JACK_A) translate([TURRET_R - 1.6 - UI_INSET, 0, PWR_Z]) rotate([0, 90, 0]) cylinder(d = 10.4, h = 4);
        // rubber feet recesses (4 x 10 mm)
        for (a = [45 : 90 : 359]) rotate(a) translate([88, 0, -1]) cylinder(d = 10.4, h = 1.6);
    }
}
module base_shell_print() base_shell();
base_shell_print();
