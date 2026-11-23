// Burn drawer under the exit slot. Its curved front is a flush patch of the turret
// shell; it slides out radially between the launcher side plates.
// Print: upright (floor on bed), no supports.
include <params.scad>
include <lib/lib.scad>
module bt_body(grow = 0) intersection() {
    box_at(-BT_XO - grow, BT_XO + grow, BT_Y0 - grow, TURRET_R + 5, BT_Z0 - grow, BT_Z1 + grow);
    cylinder(r = TURRET_RI - 0.3 + grow, h = 300, $fn = 240);
}
module burn_tray() difference() {
    union() {
        bt_body();
        intersection() {
            ring(TURRET_R, TURRET_RI - 0.31, 300, $fn = 240);
            box_at(-BT_XO, BT_XO, 0, TURRET_R + 1, BT_Z0, SLOT_Z0);
        }
    }
    intersection() {
        box_at(-BT_XO + BT_WALL, BT_XO - BT_WALL, BT_Y0 + BT_WALL, TURRET_R + 5, BT_Z0 + 1.6, BT_Z1 + 1);
        cylinder(r = TURRET_RI - 0.3 - BT_WALL, h = 300, $fn = 240);
    }
    // finger recess in the front skin
    translate([0, TURRET_R + 2, BT_Z0 + 14]) rotate([90, 0, 0]) linear_extrude(4.2) rrect([30, 10], 5);
}
module burn_tray_print() translate([0, 0, -BT_Z0]) burn_tray();
burn_tray_print();
