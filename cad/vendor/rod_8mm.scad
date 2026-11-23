// Vendor model: 8 mm hardened steel guide rod, cut to ROD_L. Local: z 0..ROD_L.
include <../params.scad>
include <../lib/lib.scad>
module vendor_rod_8mm(l = ROD_L) color([0.78,0.8,0.84]) cylinder(d = ROD_D, h = l);

vendor_rod_8mm();
