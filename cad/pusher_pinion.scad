// Pusher pinion: m1, 15 teeth (r 7.5 mm -> 19.6 mm stroke for 150 deg servo travel),
// pressed on the MG90S spline + horn screw. Print: flat.
include <params.scad>
include <lib/lib.scad>
// phase: put a rack gap over the pinion's top tooth
_p = PI_ * PIN_M;  _y0 = CARRIER_Y0 - CARRIER_L;
_gap = _y0 + _p / 4 + _p * round((PIN_Y - _y0 - _p / 4) / _p);
PIN_PHASE = (_gap - PIN_Y) / PIN_R * 180 / PI_;
module pusher_pinion2d() difference() { involute_gear2d(PIN_M, PIN_T); circle(d = fit_press(4.8) - TOL / 4); }   // MG90S 4.8 mm spline, press
module pusher_pinion(rot = 0) translate([-PIN_W / 2, PIN_Y, PIN_Z]) rotate([0, 90, 0]) rotate(180 - PIN_PHASE + rot) linear_extrude(PIN_W) pusher_pinion2d();
module pusher_pinion_print() linear_extrude(PIN_W) pusher_pinion2d();
pusher_pinion_print();
