// Pusher guide: blade channel (front), carrier channel with lips (rear), roof under the
// hopper floor, wings to the loader-frame ledges, load-IR receiver cup.
// Print: upright (wing bottoms on bed); roof/floor are short bridges (<= 28 mm).
include <params.scad>
include <lib/lib.scad>
e = Z_EJECT;
GW_X = 14;  GW_IN = BLADE_W / 2 + 2 * GAP;
module pusher_guide() difference() {
    union() {
        box_at(-GW_X, GW_X, WING_B[0], HOP_Y_FRONT, BLADE_Z0 + BLADE_T + BLADE_CLR, BLADE_Z0 + BLADE_T + BLADE_CLR + 1.6);                       // roof
        for (s = [-1, 1]) box_at(s > 0 ? GW_IN : -GW_X, s > 0 ? GW_X : -GW_IN, WING_B[0], HOP_Y_FRONT, WING_Z0, e + 1.95);
        box_at(-GW_X, GW_X, -23.5, HOP_Y_FRONT, BLADE_Z0 - BLADE_CLR - MIN_WALL, BLADE_Z0 - BLADE_CLR);                          // front blade floor
        for (s = [-1, 1]) box_at(s > 0 ? 7.5 : -GW_X, s > 0 ? GW_X : -7.5, WING_B[0], -28.5, WING_Z0, e - 3.6);  // lips
        for (w = [WING_F, WING_B], s = [-1, 1]) box_at(s > 0 ? GW_X - EPS : -43.5, s > 0 ? 43.5 : -GW_X + EPS, w[0], w[1], WING_Z0, WING_Z1);   // wings (outside the channel)
        for (p = concat(WING_FIX, SERVO_FIX)) translate([p[0], p[1], WING_Z0]) rbox([8.5, 8, WING_Z1 - WING_Z0], 1);
    }
    for (p = WING_FIX) translate([p[0], p[1], WING_Z1]) rotate([180, 0, 0]) m3_csk(WING_Z1 - WING_Z0);
    for (p = SERVO_FIX) translate([p[0], p[1], WING_Z0]) insert_hole();
    box_at(-GW_X - 1, GW_X + 1, FEED_Y - SHAFT_D / 2 - 1, FEED_Y + SHAFT_D / 2 + 1, WING_Z0 - 1, e - 0.6);   // feed-shaft pass   // geom
}
module pusher_guide_print() translate([0, 0, -WING_Z0]) pusher_guide();
pusher_guide_print();
