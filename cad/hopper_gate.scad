// Hopper gate (D-18): sets the single-card throat (THROAT above the floor) at its back
// edge and carries the driven pinch rollers (Ø6, X +-26) that push the card home into the
// shelf. The pinch shaft floats in vertical slots, pressed down by two springs. Carries the
// load IR emitter. Slotted fixings in the hopper walls give +-1.5 mm throat adjustment.
// Print: lying on its back face (Y), no supports.
include <params.scad>
include <lib/lib.scad>
GATE_H = 40;
GSPRING_X = [-10, 10];
GSPRING_Y = PINCH_Y - 1.2;   // springs sit just behind the shaft axis (1.5 mm front wall)
module hopper_gate() difference() {
    box_at(-HOP_XI + GAP, HOP_XI - GAP, HOP_YI0, HOP_Y_FRONT, Z_LOAD + THROAT, Z_LOAD + GATE_H);
    // roller cavities, open to the front and bottom
    for (s = [-1, 1]) translate([s * PINCH_X, 0, 0]) box_at(-PINCH_W / 2 - 0.8, PINCH_W / 2 + 0.8, PINCH_Y - PINCH_D / 2 - 0.8, HOP_Y_FRONT + 1, Z_LOAD - 1, PINCH_Z + PINCH_D / 2 + 1.2);
    // shaft slot (+1.0 upward float; >= 1.5 mm floor below)
    translate([0, PINCH_Y, PINCH_Z]) hull() for (dz = [0, 1.0]) translate([SHAFT_X0 - 1, 0, dz]) rotate([0, 90, 0]) cylinder(d = fit_free(SHAFT_D), h = PINCH_X1 - SHAFT_X0 + 2);
    // spring pockets above the shaft
    for (x = GSPRING_X) translate([x, GSPRING_Y, PINCH_Z + SHAFT_D / 2 - 0.5]) cylinder(d = fit_free(5), h = 9);
    // load IR emitter pocket + lead channel
    translate([LOADIR_X, LOADIR_Y, Z_LOAD + THROAT - 1]) cylinder(d = fit_slide(3.8), h = 9);   // emitter bore (glued), no thin lip
    translate([LOADIR_X, LOADIR_Y, Z_LOAD + THROAT + 6]) box_at(-3, -1, -1, 1, 0, 40);   // lead channel, clear of the gate-screw insert
    for (s = [-1, 1], z = [14, 32]) translate([s * (HOP_XI - GAP), GATE_YC, Z_LOAD + z]) rotate([0, -s * 90, 0]) insert_hole();
    translate([0, HOP_YI0, Z_LOAD + THROAT]) rotate([0, 90, 0]) cylinder(r = 1.2, h = 100, center = true, $fn = 4); // lead-in chamfer
}
module hopper_gate_print() rotate([-90, 0, 0]) translate([0, -HOP_Y_FRONT, -Z_LOAD]) hopper_gate();
hopper_gate_print();
