// Shell anchor (x4): screwed to the turntable from below (CSK M3, assembly time); the turret
// shell's tabs sit on top and are screwed down from ABOVE (cap off, long Allen key), so the
// shell lifts off without separating turret and base. Print: upright, no supports.
include <params.scad>
include <lib/lib.scad>
module shell_anchor(a = SHELL_TAB_A[0]) rotate(a) difference() {
    translate([SHELL_TAB_R, 0, TT_TOP]) rbox([7.5, 16, ANCHOR_H], 1.5);
    translate([SHELL_TAB_R, -4, TT_TOP]) insert_hole();
    translate([SHELL_TAB_R, 4, TT_TOP + ANCHOR_H - M3_INSERT_L]) insert_hole(M3_INSERT_L + 1);
}
module shell_anchor_print() translate([-SHELL_TAB_R, 0, -TT_TOP]) shell_anchor(0);
shell_anchor_print();
