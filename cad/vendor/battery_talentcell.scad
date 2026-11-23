// Vendor model: TalentCell YB1206000-USB 12 V 6 Ah Li-ion pack 145x85x28 with DC in/out jacks on the +X end. Local: centred XY, z 0..28.
include <../params.scad>
include <../lib/lib.scad>
module vendor_battery_talentcell() {
    color([0.12,0.12,0.13]) translate([0, 0, 0]) hull() for (x = [-1, 1], y = [-1, 1]) translate([x * (BATT_L / 2 - 3), y * (BATT_W / 2 - 3), 0]) cylinder(r = 3, h = BATT_H);
    color([0.05,0.05,0.05]) for (y = [-18, 0]) translate([BATT_L / 2 - 1, y, BATT_H / 2]) rotate([0, 90, 0]) difference() { cylinder(d = 9, h = 1.5); cylinder(d = 5.5, h = 4, center = true); }
    color([0.6,0.6,0.6]) translate([BATT_L / 2 - 1, 14, BATT_H / 2 - 3.5]) cube([1.5, 13, 6]);
}

vendor_battery_talentcell();
