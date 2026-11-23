// Hall-sensor post: holds the A3144 0.5 mm under the turntable (magnet at turret 0 deg),
// inside the bearing ID. Pegs into the deck. Print: upright, no supports.
include <params.scad>
include <lib/lib.scad>
HP_Z1 = TURNTABLE_Z - 3.0;      // sensor face 0.6 below the 0.9 mm magnet protrusion
module hall_post() difference() {
    union() {
        translate([HALL_R, 0, DECK_Z + DECK_T]) box_at(-3, 3, -4, 3, 0, HP_Z1 - DECK_Z - DECK_T);
        translate([HALL_R, 0, DECK_Z + 0.5]) cylinder(d = 2.3, h = 2);
    }
    translate([HALL_R, -2.2, HP_Z1 - 0.8]) box_at(-2.1, 2.1, -2, 0.6, 0, 1);      // lead groove
    translate([HALL_R, -4.5, HP_Z1 - 3]) box_at(-1.5, 1.5, 0, 1.2, 0, 4);         // lead exit
}
module hall_post_print() translate([0, 0, -DECK_Z - 0.5]) hall_post();
hall_post_print();
