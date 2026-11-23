// Deck: carries the lazy susan, hangs the turret NEMA (pancake), hall sensor post,
// cable hole. Print: upside down (top face on bed), bosses up.
include <params.scad>
include <lib/lib.scad>
module base_deck() {
    difference() {
        union() {
            translate([0, 0, DECK_Z]) cylinder(r = BASE_RI - 0.3, h = DECK_T, $fn = 240);
            // pancake NEMA standoffs (motor face screws)
            for (x = [-1, 1], y = [-1, 1]) translate([x * NEMA_HOLE / 2, TM_Y + y * NEMA_HOLE / 2, TM_FACE_Z + 0.1]) cylinder(d = 7, h = DECK_Z - TM_FACE_Z);
        }
        translate([0, 0, DECK_Z - 1]) cylinder(d = 36, h = 10);                   // cable pass
        translate([0, TM_Y, DECK_Z - 1]) cylinder(d = PUL_HUB_D + 2, h = 10);     // pulley
        for (x = [-1, 1], y = [-1, 1]) translate([x * NEMA_HOLE / 2, TM_Y + y * NEMA_HOLE / 2, TM_FACE_Z]) m3_clear(30);
        for (x = [-1, 1], y = [-1, 1]) translate([x * NEMA_HOLE / 2, TM_Y + y * NEMA_HOLE / 2, DECK_Z + DECK_T - 3]) cylinder(d = 6, h = 4);
        for (a = DECK_SCREW_A) rotate(a) translate([DECK_SCREW_R, 0, DECK_Z]) m3_clear(5);
        for (a = [0 : 90 : 359]) rotate(a) translate([LS_OD / 2 - 4.5, 0, DECK_Z]) m3_clear(5);   // bearing lower plate
        box_at(-UI_NOTCH_X - 6, UI_NOTCH_X + 6, -TURRET_R, -79, DECK_Z - 1, DECK_Z + 5);           // UI clearance
        translate([HALL_R, 0, DECK_Z - 1]) cylinder(d = 2.5, h = 5);                                     // hall post pin hole
    }
}
module base_deck_print() translate([0, 0, DECK_Z + DECK_T]) rotate([180, 0, 0]) base_deck();
base_deck_print();
