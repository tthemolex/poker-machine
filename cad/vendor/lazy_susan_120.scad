// Vendor model: Round aluminium lazy susan bearing ID70 OD120 H9.5 (VXB-style). Local: z 0..9.5.
include <../params.scad>
include <../lib/lib.scad>
module vendor_lazy_susan_120() {
    color([0.82,0.83,0.85]) difference() {
        union() {
            ring(LS_OD / 2, LS_ID / 2, 1.5);                       // static lower plate
            translate([0, 0, 1.5]) ring(52, 43, LS_H - 3);         // ball race
            translate([0, 0, LS_H - 1.5]) ring(LS_OD / 2, LS_ID / 2, 1.5);  // rotating upper plate
        }
        for (a = [0 : 90 : 359]) rotate(a) translate([LS_OD / 2 - 4.5, 0, -1]) cylinder(d = 4.2, h = LS_H + 2);
        for (a = [45 : 90 : 359]) rotate(a) translate([LS_ID / 2 + 4.5, 0, -1]) cylinder(d = 4.2, h = LS_H + 2);
    }
}

vendor_lazy_susan_120();
