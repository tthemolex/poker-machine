// Front UI bezel (flush): OLED window, GO / SET 12 mm buttons, status LED.
// Fills the notch in the base wall. Print: on its back edge (Z up = installed Z), no supports.
include <params.scad>
include <lib/lib.scad>
module ui_bezel() {
    difference() {
        union() {
            intersection() {   // flush skin
                ring(TURRET_R, BASE_RI - 0.4, BASE_TOP_Z, $fn = 240);
                box_at(-UI_NOTCH_X + 0.2, UI_NOTCH_X - 0.2, -TURRET_R - 1, 0, UI_NOTCH_Z0 + 0.2, BASE_TOP_Z);
            }
            for (s = [-1, 1]) intersection() {   // retaining lips behind the wall (ends only)
                ring(BASE_RI - 0.2, BASE_RI - 2.2, BASE_TOP_Z, $fn = 240);
                box_at(s > 0 ? UI_NOTCH_X - 6 : -UI_NOTCH_X - 5, s > 0 ? UI_NOTCH_X + 5 : -UI_NOTCH_X + 6, -TURRET_R - 1, 0, UI_NOTCH_Z0 + 0.2, BASE_TOP_Z);
            }
        }
        translate([0, -80, OLED_Z - 1]) rotate([90, 0, 0]) linear_extrude(30) rrect([23, 12.5], 1);          // OLED window
        for (x = [-BTN_X, BTN_X]) rotate(ui_az(x)) translate([TURRET_R - 20, 0, BTN_Z]) rotate([0, 90, 0]) cylinder(d = 12.2, h = 30); // buttons (radial)
        for (x = [-BTN_X, BTN_X]) rotate(ui_az(x)) translate([TURRET_R - 1 - UI_INSET, 0, BTN_Z]) rotate([0, 90, 0]) cylinder(d = 14.4, h = 5);   // flush head recess
        rotate(ui_az(LED_X)) translate([TURRET_R - 20, 0, LED_Z]) rotate([0, 90, 0]) cylinder(d = 5.1, h = 30);                      // LED
        translate([0, -sqrt(TURRET_R ^ 2) + 0.6, OLED_Z - 1]) rotate([90, 0, 0]) linear_extrude(2) rrect([25, 14.5], 1.5);  // window chamfer recess
    }
}
module ui_bezel_print() translate([0, 0, -UI_NOTCH_Z0 - 0.2]) ui_bezel();
ui_bezel_print();
