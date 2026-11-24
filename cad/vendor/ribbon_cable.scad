// Vendor model: 2 x 10-way 1.27 mm flat ribbon (12.7 mm wide, 0.9 mm thick each), laid flat on
// each other and wound together on edge as a clock spring in the bearing ID (D-19). Shown as the
// coil envelope plus the static inner tail (down beside the hub, then flat along the deck slot).
include <../params.scad>
include <../lib/lib.scad>
RB_T = RIBBON_N * RIBBON_T;
module vendor_ribbon_cable() color([0.75,0.75,0.8]) {
    translate([0, 0, SPOOL_Z0 + (SPOOL_H - RIBBON_W) / 2]) ring(SPOOL_RO, SPOOL_RI + 2.5, RIBBON_W);           // coil (ribbon on edge)
    translate([-RIBBON_W / 2, -SPOOL_RI - 0.2 - RB_T, DECK_Z - 0.3]) cube([RIBBON_W, RB_T, SPOOL_Z0 + SPOOL_H - DECK_Z + 0.3 - 0.6]);  // inner tail
    translate([(RIBBON_SLOT_X[0] + RIBBON_SLOT_X[1]) / 2 - RIBBON_W / 2, RIBBON_SLOT_Y1 + 1, BATT_H + FLOOR_T + 0.1]) cube([RIBBON_W, -RIBBON_SLOT_Y1 - SPOOL_RI - 3, RB_T]);  // flat run on the battery
}
vendor_ribbon_cable();
