// Front slot panel (D-20): flush curved patch of the turret shell under the card exit slot
// (closes the shell notch; the 6 mm slot is between its top edge and the shell). Two inside
// tabs take inserts for M3 screws through the launcher side plates. Burns are thrown onto
// the table (D-20), so there is no burn drawer any more.
// Print: upright, no supports.
include <params.scad>
include <lib/lib.scad>
module slot_panel() difference() {
    union() {
        intersection() {
            ring(TURRET_R, TURRET_RI - 0.31, 300, $fn = 240);
            box_at(-BT_XO, BT_XO, 0, TURRET_R + 1, BT_Z0, SLOT_Z0);
        }
        for (s = [-1, 1]) intersection() {
            box_at(s > 0 ? SP_TAB_X0 : -BT_XO, s > 0 ? BT_XO : -SP_TAB_X0, SP_INS[0] - 5, TURRET_R, BT_Z0, SP_INS[1] + 7);   // tabs stand on the bed
            cylinder(r = TURRET_RI - 0.3, h = 300, $fn = 240);
        }
    }
    for (s = [-1, 1]) translate([s * BT_XO, SP_INS[0], SP_INS[1]]) rotate([0, -s * 90, 0]) insert_hole();
}
module slot_panel_print() translate([0, 0, -BT_Z0]) slot_panel();
slot_panel_print();
