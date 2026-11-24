// Turntable (rotating) with the integrated 200T ring hub underneath (GT2 belt glued teeth-out, D-11).
// CSK holes for every turret part, magnet pocket at 0 deg, cable hole in the bearing ID.
// Print: upside down (top face on bed), ring teeth vertical, no supports.
include <params.scad>
include <lib/lib.scad>
module turntable() {
    difference() {
        union() {
            translate([0, 0, TURNTABLE_Z]) cylinder(r = TURRET_R, h = TURNTABLE_T, $fn = 240);
            translate([0, 0, RING_Z0]) ring(RING_HUB_R, RING_RI, TURNTABLE_Z - RING_Z0 + EPS, $fn = 400);   // belt hub
            translate([0, 0, RING_Z0]) ring(RING_HUB_R + 0.6, RING_RI, 1.2, $fn = 400);                     // belt seat lip   // geom
        }
        translate([CABLE_HOLE[0], CABLE_HOLE[1], TURNTABLE_Z - 1]) cylinder(d = CABLE_HOLE_D, h = 10);   // ribbon clock-spring outer end
        translate([HALL_R, 0, TURNTABLE_Z - 1]) cylinder(d = fit_slide(6), h = 5);   // 6x3 magnet glued in a through hole (protrudes 0.9 below)
        for (p = FIX_ALL) translate([p[0], p[1], TURNTABLE_Z]) m3_csk(TURNTABLE_T);
        for (a = [45 : 90 : 359]) rotate(a) translate([LS_ID / 2 + 4.5, 0, TURNTABLE_Z]) m3_csk(TURNTABLE_T);  // bearing upper plate
    }
}
module turntable_print() translate([0, 0, TT_TOP]) rotate([180, 0, 0]) turntable();
turntable_print();
