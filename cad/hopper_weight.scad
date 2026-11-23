// Deck weight: rests on the cards in the hopper (presses the bottom card on the feed
// rollers). Two pockets take M8 nuts / steel slugs. Print: flat, 100 % infill.
include <params.scad>
include <lib/lib.scad>
module hopper_weight(z0 = Z_LOAD) difference() {
    union() {
        translate([0, (HOP_YI0 + HOP_YI1) / 2, z0]) rbox([2 * HOP_XI - 1.2, HOP_YI0 - HOP_YI1 - 1.2, WEIGHT_T], 3.5);
        translate([0, (HOP_YI0 + HOP_YI1) / 2, z0 + WEIGHT_T - 1]) rbox([40, 8, 7], 3);
    }
    for (x = [-24, 24]) translate([x, (HOP_YI0 + HOP_YI1) / 2, z0 + 2]) cylinder(d = 15.2, h = 20, $fn = 6);
    translate([0, (HOP_YI0 + HOP_YI1) / 2, z0 + WEIGHT_T + 2.5]) rotate([90, 0, 0]) hull() for (x = [-12, 12]) translate([x, 0, 0]) cylinder(d = 4, h = 30, center = true);
}
module hopper_weight_print() translate([0, 0, -Z_LOAD]) hopper_weight();
hopper_weight_print();
