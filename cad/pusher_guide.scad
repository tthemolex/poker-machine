// Pusher guide: blade channel (front), carrier channel with lips (rear), roof under the
// hopper floor, wings to the loader-frame ledges, load-IR receiver cup.
// Print: upright (wing bottoms on bed); roof/floor are short bridges (<= 28 mm).
include <params.scad>
include <lib/lib.scad>
e = Z_EJECT;
GW_X = 14;  GW_IN = 12.4;
module pusher_guide() difference() {
    union() {
        box_at(-GW_X, GW_X, WING_B[0], HOP_Y_FRONT, e + 0.35, e + 1.95);                       // roof
        for (s = [-1, 1]) box_at(s > 0 ? GW_IN : -GW_X, s > 0 ? GW_X : -GW_IN, WING_B[0], HOP_Y_FRONT, WING_Z0, e + 1.95);
        box_at(-GW_X, GW_X, -23.5, HOP_Y_FRONT, e - 1.65, e - 0.45);                          // front blade floor
        for (s = [-1, 1]) box_at(s > 0 ? 7.5 : -GW_X, s > 0 ? GW_X : -7.5, WING_B[0], -28.5, WING_Z0, e - 3.6);  // lips
        for (w = [WING_F, WING_B], s = [-1, 1]) box_at(s > 0 ? GW_X - EPS : -43.5, s > 0 ? 43.5 : -GW_X + EPS, w[0], w[1], WING_Z0, WING_Z1);   // wings (outside the channel)
        for (p = concat(WING_FIX, SERVO_FIX)) translate([p[0], p[1], WING_Z0]) rbox([8.5, 8, WING_Z1 - WING_Z0], 1);
        // load-IR receiver cup (+X, under the throat)
        box_at(GW_X - EPS, LOADIR_X + 3.4, LOADIR_Y - 3, HOP_Y_FRONT, e - 6.2, e - 4.6);
        box_at(LOADIR_X + 2.1, LOADIR_X + 3.4, LOADIR_Y - 3, HOP_Y_FRONT, e - 6.2, e + 1.95);
    }
    for (p = WING_FIX) translate([p[0], p[1], WING_Z1]) rotate([180, 0, 0]) m3_csk(WING_Z1 - WING_Z0);
    for (p = SERVO_FIX) translate([p[0], p[1], WING_Z0]) insert_hole();
    translate([LOADIR_X, LOADIR_Y, e - 7]) cylinder(d = 3.9, h = 20);
    box_at(-GW_X - 1, GW_X + 1, FEED_Y - SHAFT_D / 2 - 1, FEED_Y + SHAFT_D / 2 + 1, WING_Z0 - 1, e - 0.6);   // feed-shaft pass
}
module pusher_guide_print() translate([0, 0, -(Z_EJECT - 6.2)]) pusher_guide();
pusher_guide_print();
