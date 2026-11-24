// Rack bottom plate (RACK_CAP thick, card 0 rests on it), glued into the comb rebates.
// Print: flat.
include <params.scad>
include <lib/lib.scad>
module rack_bottom_plate() translate([0, 0, RACK_Z_MIN]) difference() {
    box_at(-COMB_IN - PLATE_REBATE + GAP / 2, COMB_IN + PLATE_REBATE - GAP / 2, RACK_Y0, RACK_Y1, 0, RACK_CAP);
    for (x = [-25, 0, 25]) translate([x, RACK_YC, -1]) linear_extrude(5) rrect([16, 30], 4);   // lightening, keeps card support
}
module rack_bottom_plate_print() translate([0, 0, -RACK_Z_MIN]) rack_bottom_plate();
rack_bottom_plate_print();
