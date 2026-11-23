// Vendor model: 20-way 1.27 mm flat ribbon cable (12 mm wide x 0.9 mm), wound 3 turns as a
// clock spring in the bearing ID (static inner end, rotating outer end). Shown as the coil
// envelope plus the two tails. Local = world (base part). Rotating tail drawn at turret 0.
include <../params.scad>
include <../lib/lib.scad>
module vendor_ribbon_cable() color([0.75,0.75,0.8]) {
    translate([0, 0, SPOOL_Z0]) ring(SPOOL_RO, SPOOL_RI + 2.5, SPOOL_H);                       // coil envelope
    translate([-RIBBON_W / 2, -SPOOL_RI - 0.8, DECK_Z - 0.3]) cube([RIBBON_W, 0.9, SPOOL_Z0 + SPOOL_H - DECK_Z + 0.3]);  // inner tail down the hub slit
    translate([(RIBBON_SLOT_X[0] + RIBBON_SLOT_X[1]) / 2 - RIBBON_W / 2, RIBBON_SLOT_Y1 + 1, DECK_Z - 0.3]) cube([RIBBON_W, -RIBBON_SLOT_Y1 - SPOOL_RI - 1.5, 0.9]);  // along the deck slot
}
vendor_ribbon_cable();
