// Deck: carries the lazy susan, hangs the turret NEMA (pancake), hall sensor post,
// cable hole. Print: upside down (top face on bed), bosses up.
include <params.scad>
include <lib/lib.scad>
module base_deck() {
    difference() {
        union() {
            translate([0, 0, DECK_Z]) cylinder(r = BASE_RI - 0.3, h = DECK_T, $fn = 240);
            // pancake NEMA standoffs (motor face screws)
            for (x = [-1, 1], y = [-1, 1]) translate([x * NEMA_HOLE / 2, TM_Y + y * NEMA_HOLE / 2, TM_FACE_Z + 0.1]) cylinder(d = 9.5, h = DECK_Z - TM_FACE_Z);   // geom: standoff

        }
        translate([0, 0, DECK_Z - 1]) cylinder(r = SPOOL_RI - 2.5 + TOL, h = 30);         // spool-hub peg hole
        // radial ribbon slot through the deck over the battery, to the -Y electronics crescent (D-19)
        translate([0, 0, DECK_Z - 1]) box_at(RIBBON_SLOT_X[0], RIBBON_SLOT_X[1], RIBBON_SLOT_Y1, -SPOOL_RI + 2, 0, 10);
        translate([0, 0, DECK_Z - 1]) box_at(RIBBON_SLOT_X[0], RIBBON_W / 2 + 1, -SPOOL_RI - 2.5, -SPOOL_RI + 0.3, 0, 10);
        translate([0, TM_Y, DECK_Z - 1]) cylinder(d = PUL_HUB_D + 2, h = 10);     // pulley
        for (x = [-1, 1], y = [-1, 1]) translate([x * NEMA_HOLE / 2, TM_Y + y * NEMA_HOLE / 2, TM_FACE_Z]) m3_clear(30);
        for (x = [-1, 1], y = [-1, 1]) translate([x * NEMA_HOLE / 2, TM_Y + y * NEMA_HOLE / 2, DECK_Z + DECK_T - 3]) cylinder(d = M3_HEAD_CB, h = 4);
        for (a = DECK_SCREW_A) rotate(a) translate([DECK_SCREW_R, 0, DECK_Z]) m3_clear(5);
        for (a = [0 : 90 : 359]) rotate(a) translate([LS_OD / 2 - 4.5, 0, DECK_Z]) m3_clear(5);   // bearing lower plate
        box_at(-TURRET_R, TURRET_R, -TURRET_R, -79, DECK_Z - 1, DECK_Z + 5);                       // UI clearance (full chord, no slivers)
        translate([HALL_R, 0, DECK_Z - 1]) cylinder(d = fit_slide(PEG_D), h = 5);                         // hall post peg hole
    }
}
module base_deck_print() translate([0, 0, DECK_Z + DECK_T]) rotate([180, 0, 0]) base_deck();
base_deck_print();
