// Vendor model: Brass T8 flange nut (flange 22x3.5, body 10.2, L15). Local: flange z 0..3.5, body hangs below.
include <../params.scad>
include <../lib/lib.scad>
module vendor_t8_nut() color([0.85,0.65,0.25]) difference() {
    union() { cylinder(d = NUT_FL_D, h = NUT_FL_T); translate([0, 0, NUT_FL_T - NUT_L]) cylinder(d = NUT_BODY_D, h = NUT_L); }
    translate([0, 0, -20]) cylinder(d = LEAD_D, h = 40);   // nominal: thread modelled as a plain bore
    for (a = [45 : 90 : 360]) rotate(a) translate([NUT_PCD / 2, 0, -1]) cylinder(d = 3.5, h = 6);
}

vendor_t8_nut();
