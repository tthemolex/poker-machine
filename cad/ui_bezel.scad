// Front UI bezel (flush): OLED window, GO / SET 12 mm buttons, status LED.
// Fills the notch in the base wall. Print: on its back edge (Z up = installed Z), no supports.
include <params.scad>
include <lib/lib.scad>
module ui_bezel() {
    difference() {
        union() {
            intersection() {   // flush skin
                ring(TURRET_R, BASE_RI - 2 * GAP, BASE_TOP_Z, $fn = 240);
                box_at(-UI_NOTCH_X + GAP, UI_NOTCH_X - GAP, -TURRET_R - 1, 0, UI_NOTCH_Z0 + GAP, BASE_TOP_Z);
            }
            for (x = [-BTN_X, BTN_X]) rotate(ui_az(x)) translate([BASE_RI - 2.6, 0, BTN_Z]) rotate([0, 90, 0]) cylinder(d = 18, h = 2.3);  // button bosses (>= 1.2 behind the flush recess)
            for (s = [-1, 1]) intersection() {   // retaining lips behind the wall (ends only)
                ring(BASE_RI - GAP, BASE_RI - 2.2, BASE_TOP_Z, $fn = 240);
                box_at(s > 0 ? UI_NOTCH_X - 6 : -UI_NOTCH_X - 5, s > 0 ? UI_NOTCH_X + 5 : -UI_NOTCH_X + 6, -TURRET_R - 1, 0, UI_NOTCH_Z0 + GAP, BASE_TOP_Z);
            }
        }
        translate([0, -80, OLED_Z - 1]) rotate([90, 0, 0]) linear_extrude(30) rrect([23, 12.5], 1);          // OLED window
        for (x = [-BTN_X, BTN_X]) rotate(ui_az(x)) translate([TURRET_R - 20, 0, BTN_Z]) rotate([0, 90, 0]) cylinder(d = fit_free(11.8), h = 30); // buttons (radial)
        for (x = [-BTN_X, BTN_X]) rotate(ui_az(x)) translate([TURRET_R - 1 - UI_INSET, 0, BTN_Z]) rotate([0, 90, 0]) cylinder(d = fit_free(14), h = 5);   // flush head recess
        rotate(ui_az(LED_X)) translate([TURRET_R - 20, 0, LED_Z]) rotate([0, 90, 0]) cylinder(d = fit_slide(5), h = 30);                      // LED
        translate([0, -sqrt(TURRET_R ^ 2) + 0.6, OLED_Z - 1]) rotate([90, 0, 0]) linear_extrude(2) rrect([25, 14.5], 1.5);  // window chamfer recess   // geom
    }
}
module ui_bezel_print() translate([0, 0, -UI_NOTCH_Z0 - GAP]) ui_bezel();
ui_bezel_print();
