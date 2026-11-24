// Battery door: plug + flange in the floor rebate, 4 x M3 into floor inserts.
// The battery drops out downward. Print: flat, outside face on bed.
include <params.scad>
include <lib/lib.scad>
module battery_door() {
    difference() {
        union() {
            difference() {   // flange in the rebate (2 mm: takes the CSK heads)
                rbox_at(-CR_XI - DOOR_REBATE_X + GAP, CR_XI + DOOR_REBATE_X - GAP, -CR_YI - DOOR_REBATE_Y + GAP, CR_YI + DOOR_REBATE_Y - GAP, 0, DOOR_REBATE_T, 2.8);
                rbox_at(-CR_XI, CR_XI, -CR_YI, CR_YI, -1, 5, 1);
            }
            rbox_at(-CR_XI + GAP, CR_XI - GAP, -CR_YI + GAP, CR_YI - GAP, 0, FLOOR_T, 0.8);   // plug: battery rests on it at FLOOR_T   // geom
            rbox_at(-CR_XI - 1, CR_XI + 1, -CR_YI - 1, CR_YI + 1, 0, FLOOR_T, 1);
        }
        for (p = DOOR_SCREWS) translate([p[0], p[1], 0]) m3_csk(DOOR_REBATE_T + 1);
        translate([0, -CR_YI - DOOR_REBATE_Y + 1.5, -1]) rbox([30, 3, 5], 1.4);  // finger notch through the flange edge
    }
}
module battery_door_print() battery_door();
battery_door_print();
