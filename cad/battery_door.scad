// Battery door: plug + flange in the floor rebate, 4 x M3 into floor inserts.
// The battery drops out downward. Print: flat, outside face on bed.
include <params.scad>
include <lib/lib.scad>
module battery_door() {
    difference() {
        union() {
            rbox_at(-CR_XI - DOOR_REBATE + 0.2, CR_XI + DOOR_REBATE - 0.2, -CR_YI - DOOR_REBATE + 0.2, CR_YI + DOOR_REBATE - 0.2, 0, DOOR_REBATE_T, 2.8);
            rbox_at(-CR_XI + 0.2, CR_XI - 0.2, -CR_YI + 0.2, CR_YI - 0.2, 0, FLOOR_T, 0.8);
        }
        for (p = DOOR_SCREWS) translate([p[0], p[1], 0]) m3_csk(5);
        translate([0, -CR_YI + 8, -1]) rbox([30, 6, 1.6], 3);            // finger pull
    }
}
module battery_door_print() battery_door();
battery_door_print();
