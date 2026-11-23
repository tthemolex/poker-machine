// Shared helpers. Include AFTER params.scad (needs TOL, M3_* etc.).
$fa = 4; $fs = 0.5;

module rrect(size, r) {            // 2D rounded rectangle, centred
    r2 = min(r, min(size[0], size[1]) / 2 - 0.001);
    offset(r = r2) square([size[0] - 2 * r2, size[1] - 2 * r2], center = true);
}
module rbox(size, r, center_xy = true) {   // box with rounded vertical edges
    translate(center_xy ? [0, 0, 0] : [size[0] / 2, size[1] / 2, 0])
        linear_extrude(size[2]) rrect([size[0], size[1]], r);
}
module rbox_at(x0, x1, y0, y1, z0, z1, r = 0.01) {
    translate([(x0 + x1) / 2, (y0 + y1) / 2, z0]) rbox([x1 - x0, y1 - y0, z1 - z0], r);
}
module box_at(x0, x1, y0, y1, z0, z1) { translate([x0, y0, z0]) cube([x1 - x0, y1 - y0, z1 - z0]); }

module ring(ro, ri, h) { difference() { cylinder(r = ro, h = h); translate([0, 0, -1]) cylinder(r = ri, h = h + 2); } }
module ring_sector(ro, ri, h, a0, a1) {
    intersection() {
        ring(ro, ri, h);
        linear_extrude(h) polygon(concat([[0, 0]], [for (a = [a0 : (a1 - a0) / 24 : a1]) 2 * ro * [cos(a), sin(a)]]));
    }
}
// teardrop hole along an axis lying horizontal (axis = Y after rotation), printable w/o support
module teardrop2d(d) { union() { circle(d = d); rotate(45) square(d / 2); } }
// vertical heat-set insert hole, opening at z=0 going +z (depth l)
module insert_hole(l = M3_INSERT_L) { translate([0, 0, -EPS]) cylinder(d = M3_INSERT_D, h = l + EPS); }
module m3_clear(l) { translate([0, 0, -EPS]) cylinder(d = M3_CLEAR, h = l + 2 * EPS); }
module m3_csk(l) {  // countersunk clearance from z=0 upward, head at z=0 side
    translate([0, 0, -EPS]) { cylinder(d = M3_CLEAR, h = l + 2 * EPS); cylinder(d1 = 6.4, d2 = M3_CLEAR, h = 1.7); }
}
module card2d() { rrect([CARD_L, CARD_W], CARD_R); }
module card_at(cy, z) { translate([0, cy, z]) linear_extrude(CARD_T) card2d(); }

// ---- gears ----
function _inv(a) = tan(a) - a * PI_ / 180;           // involute function, a in deg
function _invpt(rb, r, ang0, s) = let(al = acos(rb / r), th = ang0 - s * (_inv(al) * 180 / PI_)) [r * cos(th), r * sin(th)];
module involute_gear2d(m, z, pa = 20, clearance = 0.05) {
    rp = m * z / 2; rb = rp * cos(pa); ra = rp + m - clearance; rf = rp - 1.25 * m;
    half = 90 / z + _inv(pa) * 180 / PI_;            // tooth half-angle at base (deg)
    n = 8;
    rs = [for (i = [0 : n]) max(rb, rf) + (ra - max(rb, rf)) * i / n];
    union() {
        circle(r = rf, $fn = z * 4);
        for (k = [0 : z - 1]) rotate(k * 360 / z) polygon(concat(
            [[rf * cos(-half), rf * sin(-half)]],
            [for (r = rs) let(p = _invpt(rb, r, half, 1)) [p[0], -p[1]]],
            [for (i = [n : -1 : 0]) _invpt(rb, rs[i], half, 1)],
            [[rf * cos(half), rf * sin(half)]]));
    }
}
// straight gear rack (2D in XY: teeth along +x, pointing -y, pitch line at y=0)
module gear_rack2d(m, len, back = 3) {
    p = PI_ * m; ad = m; de = 1.25 * m; t = tan(20);
    n = ceil(len / p) + 1;
    intersection() {
        union() {
            translate([0, de]) square([len, back]);
            for (i = [0 : n]) if (i * p - p / 4 - (p / 4 + de * t) >= 0 && i * p + de * t <= len) translate([i * p - p / 4, 0]) polygon([
                [-(p / 4 + de * t), de + 0.01], [p / 4 + de * t + 0.01, de + 0.01],
                [p / 4 - ad * t, -ad], [-(p / 4 - ad * t), -ad] ]);
        }
        translate([0, -ad - 1]) square([len, ad + de + back + 2]);
    }
}
// GT2 belt-tooth profile ring (teeth outward), 2D
module gt2_ring2d(n, r_land, ri) {
    union() {
        difference() { circle(r = r_land + 0.01, $fn = n * 2); circle(r = ri, $fn = n * 2); }
        for (k = [0 : n - 1]) rotate(k * 360 / n)
            translate([r_land, 0]) hull() { translate([-0.2, 0]) square([0.2, 1.0], center = true); translate([0.25, 0]) circle(r = 0.5, $fn = 12); }
    }
}
