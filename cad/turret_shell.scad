// Turret shell: smooth cylinder; the front notch holds the flush slot panel and forms the
// card exit slot (SLOT_Z0..SLOT_Z1). Four wedge tabs screw down into the shell anchors
// from above (reachable with the cap off).
// Print: upside down (top rim on bed), notch opens upward, no supports.
include <params.scad>
include <lib/lib.scad>
module turret_shell() difference() {
    union() {
        translate([0, 0, TT_TOP]) ring(TURRET_R, TURRET_RI, SHELL_Z1 - TT_TOP, $fn = 240);
        for (a = SHELL_TAB_A) rotate(a) hull() {     // tabs rest on the shell anchors (45 deg wedge)
            translate([SHELL_TAB_R - 3, 0.5, TT_TOP + ANCHOR_H + GAP]) cube([TURRET_RI - SHELL_TAB_R + 3.1, 7.5, 4]);   // geom
            translate([TURRET_RI - 0.1, 0.5, TT_TOP + ANCHOR_H + GAP]) cube([0.3, 7.5, 10]);   // geom
        }
    }
    box_at(-BT_XO - GAP, BT_XO + GAP, 0, TURRET_R + 5, TT_TOP - 1, SLOT_Z1);
    for (p = SHELL_SCREW) translate([p[0], p[1], TT_TOP]) m3_clear(ANCHOR_H + 20);
}
module turret_shell_print() translate([0, 0, SHELL_Z1]) rotate([180, 0, 0]) turret_shell();
turret_shell_print();
