// Hopper gate (D-18): sets the single-card throat (THROAT above the floor) at its back
// edge and is the bearing for the driven pinch shaft (Ø7 rollers, X +-26, 0.15 mm rubber
// squeeze) that pushes the card home. Fixed position (gear mesh); change THROAT by reprinting.
// Carries the load IR emitter.
// Print: lying on its back face (Y), no supports.
include <params.scad>
include <lib/lib.scad>
GATE_H = 40;
module hopper_gate() difference() {
    box_at(-HOP_XI + GAP, HOP_XI - GAP, HOP_YI0, HOP_Y_FRONT, Z_LOAD + THROAT, Z_LOAD + GATE_H);
    // roller cavities, open to the front and bottom
    for (s = [-1, 1]) translate([s * PINCH_X, 0, 0]) box_at(-PINCH_W / 2 - 4 * GAP, PINCH_W / 2 + 4 * GAP, PINCH_Y - PINCH_D / 2 - 4 * GAP, HOP_Y_FRONT + 1, Z_LOAD - 1, PINCH_Z + PINCH_D / 2 + 1.2);
    // pinch shaft bore: fixed (exact gear centre distance); rubber gives the pinch compliance
    translate([SHAFT_X0 - 1, PINCH_Y, PINCH_Z]) rotate([0, 90, 0]) cylinder(d = fit_free(SHAFT_D), h = PINCH_X1 - SHAFT_X0 + 2);
    // load IR emitter pocket + lead channel
    translate([LOADIR_X, LOADIR_Y, Z_LOAD + THROAT - 1]) cylinder(d = fit_slide(3.8), h = 9);   // emitter bore (glued), no thin lip
    translate([LOADIR_X, LOADIR_Y, Z_LOAD + THROAT + 6]) box_at(-3, -1, -1, 1, 0, 40);   // lead channel, clear of the gate-screw insert
    for (s = [-1, 1], z = [14, 32]) translate([s * (HOP_XI - GAP), GATE_YC, Z_LOAD + z]) rotate([0, -s * 90, 0]) insert_hole();
    translate([0, HOP_YI0, Z_LOAD + THROAT]) rotate([0, 90, 0]) cylinder(r = 1.2, h = 100, center = true, $fn = 4); // lead-in chamfer
}
module hopper_gate_print() rotate([-90, 0, 0]) translate([0, -HOP_Y_FRONT, -Z_LOAD]) hopper_gate();
hopper_gate_print();
