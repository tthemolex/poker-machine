// Pusher carrier: T-profile slider with an m1 gear rack underneath; the steel blade is
// screwed (2 x M2) on top. Print: upside down (flange on bed), teeth up, no supports.
include <params.scad>
include <lib/lib.scad>
e = Z_EJECT;
SL_Y0 = CARRIER_Y0 - CARRIER_L;
module pusher_slider(push = 0) translate([0, push, 0]) difference() {
    union() {
        box_at(-12, 12, SL_Y0, CARRIER_Y0, e - 3.3, CARRIER_TOP);
        box_at(-7, 7, SL_Y0, CARRIER_Y0, CARRIER_Z0, e - 3.3 + EPS);
        // gear rack: 2D x -> world Y, 2D y -> world Z (teeth point down)
        translate([-PIN_W / 2, SL_Y0, RACKG_PITCH_Z]) multmatrix([[0, 0, 1, 0], [1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1]]) linear_extrude(PIN_W) gear_rack2d(PIN_M, CARRIER_L, 0.01);
    }
    for (x = [-7, 7]) translate([x, -BLADE_L + BLADE_TIP_Y + 7.5 + 0, e - 8]) cylinder(d = M2_PILOT_D, h = 20);
}
module pusher_slider_print() translate([0, 0, CARRIER_TOP]) rotate([180, 0, 0]) pusher_slider();
pusher_slider_print();
