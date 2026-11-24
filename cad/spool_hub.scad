// Clock-spring hub (static): pegs into the deck centre; the ribbon's inner end is clamped to
// it and runs down beside it into the deck slot (D-19). Print: upside down (top on bed), no supports.
include <params.scad>
include <lib/lib.scad>
SH_Z1 = SPOOL_Z0 + SPOOL_H + 0.5;   // geom
module spool_hub() difference() {
    union() {
        translate([0, 0, DECK_Z + DECK_T]) cylinder(r = SPOOL_RI, h = SH_Z1 - DECK_Z - DECK_T);
        translate([0, 0, DECK_Z + 0.3]) cylinder(r = SPOOL_RI - 2.5, h = DECK_T);       // peg   // geom
    }
    translate([0, 0, SH_Z1 - 4]) rotate([90, 0, 0]) cylinder(d = 2.2, h = 20);        // geom: cable-tie hole
}
module spool_hub_print() translate([0, 0, SH_Z1]) rotate([180, 0, 0]) spool_hub();
spool_hub_print();
