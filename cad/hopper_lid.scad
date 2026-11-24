// Hopper loading lid, drops into the top cap flush. Print: flat, top face on bed.
include <params.scad>
include <lib/lib.scad>
LID_OPEN = [76, HOP_YI0 - HOP_YI1 - 7];  LID_YC = (HOP_YI0 + HOP_YI1) / 2;
module hopper_lid() difference() {
    translate([0, LID_YC, CAP_IN_Z]) linear_extrude(CAP_T) rrect(LID_OPEN - [2 * GAP, 2 * GAP], 5 - GAP);
    translate([0, LID_YC + LID_OPEN[1] / 2 - 6, CAP_IN_Z - 1]) linear_extrude(5) rrect([24, 6], 3);   // finger slot (through)
}
module hopper_lid_print() translate([0, 0, CAP_TOP_Z]) rotate([180, 0, 0]) hopper_lid();
hopper_lid_print();
