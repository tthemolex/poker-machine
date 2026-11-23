// Rail frame (+X): bottom and top blocks hold the two 8 mm rods, joined by an outer web.
// Microswitch pocket in the bottom block (actuated by the carriage at home).
// Print: lying on the web's outer face, teardrop rod holes, no supports.
include <params.scad>
include <lib/lib.scad>
RF_X0 = ROD_X - 9;  RF_Y0 = -10;  RF_Y1 = 50;
MSW_X = ROD_X;  MSW_Z0 = RAIL_BOT_Z1 + 0.1 - 6.5;    // switch body top 0.1 above block (lever 0.3)
module rail_frame() difference() {
    union() {
        box_at(RF_X0, RAIL_WEB_X1, RF_Y0, RF_Y1, TT_TOP, RAIL_BOT_Z1);
        box_at(RF_X0, RAIL_WEB_X1, RF_Y0, RF_Y1, RAIL_TOP_Z0, RAIL_TOP_Z1);
        box_at(RAIL_WEB_X0, RAIL_WEB_X1, RF_Y0, RF_Y1, TT_TOP, RAIL_TOP_Z1);
    }
    for (y = ROD_Y) {
        translate([ROD_X, y, ROD_Z0]) cylinder(d = fit_snug(ROD_D), h = RAIL_BOT_Z1 - ROD_Z0 + 1);
        translate([ROD_X, y, RAIL_TOP_Z0 - 1]) cylinder(d = fit_snug(ROD_D), h = ROD_Z1 - RAIL_TOP_Z0 + 1);
    }
    translate([MSW_X, MSW_Y, MSW_Z0]) box_at(-3.1, 3.1, -6.6, 6.6, 0, 10);
    translate([MSW_X, MSW_Y, MSW_Z0 - 4]) box_at(-1.5, 8, -6.6, 6.6, 0, 5);           // lead slot
    for (y = [-3.25, 3.25]) translate([MSW_X - 10, MSW_Y + y, MSW_Z0 + 1.75]) rotate([0, 90, 0]) cylinder(d = M2_PILOT_D, h = 20);
    for (p = FIX_RAIL) translate([p[0], p[1], TT_TOP]) insert_hole();
}
module rail_frame_print() rotate([0, -90, 0]) translate([-RAIL_WEB_X1, 0, -TT_TOP]) mirror([1, 0, 0]) translate([-2 * RAIL_WEB_X1, 0, 0]) rail_frame();
rail_frame_print();
