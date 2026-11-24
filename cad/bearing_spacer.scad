// Bearing spacer (D-19): 6 mm ring between deck and lazy susan. It raises the bearing ID space to
// ~15 mm for the 2-ribbon clock spring. The bearing screws pass through it into the deck.
// Print: flat, no supports.
include <params.scad>
include <lib/lib.scad>
module bearing_spacer() difference() {
    translate([0, 0, DECK_Z + DECK_T]) ring(LS_OD / 2, LS_ID / 2 + CLR, LS_SPACER, $fn = 200);
    for (a = [0 : 90 : 359]) rotate(a) translate([LS_OD / 2 - 4.5, 0, DECK_Z]) m3_clear(20);
}
module bearing_spacer_print() translate([0, 0, -DECK_Z - DECK_T]) bearing_spacer();
bearing_spacer_print();
