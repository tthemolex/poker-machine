// Turret shell: smooth cylinder; front notch holds the burn drawer and forms the
// card exit slot (SLOT_Z0..SLOT_Z1). Four wedge tabs screw to the turntable.
// Print: upside down (top rim on bed), notch opens upward, no supports.
include <params.scad>
include <lib/lib.scad>
module turret_shell() difference() {
    union() {
        translate([0, 0, TT_TOP]) ring(TURRET_R, TURRET_RI, SHELL_Z1 - TT_TOP, $fn = 240);
        for (a = SHELL_TAB_A) rotate(a) hull() {
            translate([TURRET_RI - 6, -4, TT_TOP]) cube([6.2, 8, 4]);
            translate([TURRET_RI - 0.1, -4, TT_TOP]) cube([0.3, 8, 10]);
        }
    }
    box_at(-BT_XO - 0.2, BT_XO + 0.2, 0, TURRET_R + 5, TT_TOP - 1, SLOT_Z1);
    for (a = SHELL_TAB_A) rotate(a) translate([SHELL_TAB_R, 0, TT_TOP]) insert_hole();
}
module turret_shell_print() translate([0, 0, SHELL_Z1]) rotate([180, 0, 0]) turret_shell();
turret_shell_print();
