// Rack top plate, glued into the comb rebates above slot 51. Print: flat.
include <params.scad>
include <lib/lib.scad>
module rack_top_plate() translate([0, 0, RACK_Z_MIN]) difference() {
    box_at(-COMB_IN - PLATE_REBATE + GAP / 2, COMB_IN + PLATE_REBATE - GAP / 2, RACK_Y0, RACK_Y1, shelf_z(SHELF_COUNT - 1) + SHELF_GAP, RACK_H);
    for (x = [-25, 0, 25]) translate([x, RACK_YC, 0]) linear_extrude(200) rrect([16, 30], 4);
}
module rack_top_plate_print() translate([0, 0, -RACK_Z_MIN - shelf_z(SHELF_COUNT - 1) - SHELF_GAP]) rack_top_plate();
rack_top_plate_print();
