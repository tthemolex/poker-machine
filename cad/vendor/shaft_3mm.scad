// Vendor model: 3 mm silver-steel shaft (loader roller axle / idler axle), cut to length. Local: axis Z, z 0..l.
include <../params.scad>
include <../lib/lib.scad>
module vendor_shaft_3mm(l = SHAFT_X1 - SHAFT_X0) color([0.8,0.8,0.82]) cylinder(d = SHAFT_D, h = l);

vendor_shaft_3mm();
