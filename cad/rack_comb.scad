// Rack side comb (qty 2, identical; the -X one is the same part turned 180 deg about Z).
// 51 fins 0.9 mm (noted exception to the 1.2 mm wall rule, D-01) on a 4.5 mm wall.
// Rebates top and bottom take the glued rack plates. Pilot holes for the carriage (+X)
// and nut arm (-X) patterns are both present (pattern is symmetric about the rack centre).
// Print: lying on its outer face, fins vertical, no supports.
include <params.scad>
include <lib/lib.scad>
// installed as the +X comb, rack at home (rack bottom at RACK_Z_MIN)
module rack_comb() {
    translate([0, 0, RACK_Z_MIN]) difference() {
        union() {
            box_at(COMB_IN, COMB_OUT, RACK_Y0, RACK_Y1, 0, RACK_H);
            for (i = [0 : SHELF_COUNT - 2]) box_at(FIN_IN, COMB_IN + EPS, RACK_Y0, RACK_Y1, shelf_z(i) + SHELF_GAP, shelf_z(i + 1));
        }
        box_at(COMB_IN - 1, COMB_IN + PLATE_REBATE, RACK_Y0 - 1, RACK_Y1 + 1, -1, RACK_CAP);
        box_at(COMB_IN - 1, COMB_IN + PLATE_REBATE, RACK_Y0 - 1, RACK_Y1 + 1, shelf_z(SHELF_COUNT - 1) + SHELF_GAP, RACK_H + 1);
        for (h = concat(CARR_HOLES, ARM_HOLES, [for (p = ARM_HOLES) [2 * RACK_YC - p[0], p[1]]]))
            translate([COMB_OUT + EPS, h[0], h[1]]) rotate([0, -90, 0]) cylinder(d = PILOT_D, h = PILOT_L);
        // lead-in chamfers on the fin tips at both open faces
        for (y = [RACK_Y0, RACK_Y1]) translate([FIN_IN, y, -1]) rotate(45) cube([1.2, 1.2, RACK_H + 2], center = true);
    }
}
module rack_comb_print() translate([0, 0, COMB_OUT]) rotate([0, 90, 0]) translate([0, 0, -RACK_Z_MIN]) rack_comb();
rack_comb_print();
