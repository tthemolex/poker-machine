// Hopper gate: sets the single-card throat (THROAT above the floor), carries the load
// IR emitter. Slotted fixings in the hopper walls allow +-1.5 mm throat adjustment.
// Print: lying on its back face, no supports.
include <params.scad>
include <lib/lib.scad>
GATE_H = 40;
module hopper_gate() difference() {
    box_at(-HOP_XI + 0.2, HOP_XI - 0.2, HOP_YI0, HOP_Y_FRONT, Z_LOAD + THROAT, Z_LOAD + GATE_H);
    translate([LOADIR_X, LOADIR_Y, Z_LOAD + THROAT - 1]) { cylinder(d = 3.2, h = 3); translate([0, 0, 2]) cylinder(d = 4.0, h = 7); }
    translate([LOADIR_X, LOADIR_Y, Z_LOAD + THROAT + 8.5]) box_at(-1, 1, -1.2, 1.2, 0, 40);  // lead channel
    for (s = [-1, 1], z = [10, 30]) translate([s * (HOP_XI + 0.2), LOADIR_Y, Z_LOAD + z]) rotate([0, -s * 90, 0]) insert_hole();
    translate([0, HOP_YI0, Z_LOAD + THROAT]) rotate([0, 90, 0]) cylinder(r = 1.2, h = 100, center = true, $fn = 4); // lead-in chamfer
}
module hopper_gate_print() rotate([-90, 0, 0]) translate([0, -HOP_Y_FRONT, -Z_LOAD]) hopper_gate();
hopper_gate_print();
