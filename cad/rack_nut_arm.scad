// Nut arm: ties the -X comb to the T8 flange nut. Flange on top (4 x M3 thread-forming), body through the arm.
// Print: arm flat on the bed, comb flange vertical, no supports.
include <params.scad>
include <lib/lib.scad>
module rack_nut_arm() translate([0, 0, RACK_Z_MIN]) difference() {
    union() {
        hull() {
            box_at(-COMB_OUT - 3, -COMB_OUT, -10.5, 12, ARM_Z, ARM_Z + ARM_T);
            translate([LM_X, LM_Y, ARM_Z]) cylinder(d = NUT_FL_D + 2, h = ARM_T);
        }
        box_at(-COMB_OUT - 3, -COMB_OUT, -10.5, 12, ARM_Z, ARM_Z + 21);
    }
    translate([LM_X, LM_Y, ARM_Z - 1]) cylinder(d = fit_free(NUT_BODY_D), h = 20);
    for (a = [45 : 90 : 359]) translate([LM_X + NUT_PCD / 2 * cos(a), LM_Y + NUT_PCD / 2 * sin(a), ARM_Z - 1]) cylinder(d = PILOT_D, h = ARM_T + 2);   // M3 thread-forming (keeps 1.45 mm to the nut bore)
    for (h = ARM_HOLES) translate([-COMB_OUT - 4, h[0], h[1]]) rotate([0, 90, 0]) cylinder(d = M3_CLEAR, h = 10);
}
module rack_nut_arm_print() translate([0, 0, -RACK_Z_MIN - ARM_Z]) rack_nut_arm();
rack_nut_arm_print();
