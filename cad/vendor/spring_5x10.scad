// Vendor model: Compression spring 5 mm OD x 10 mm free, 0.5 mm wire (launcher idler preload). Local: z 0..8 (installed).
include <../params.scad>
include <../lib/lib.scad>
module vendor_spring_5x10(l = 8) color([0.75,0.75,0.78]) for (i = [0 : 9]) translate([0, 0, i * l / 10]) rotate([4, 0, 0]) ring(2.5, 2.0, l / 10 * 0.6);

vendor_spring_5x10();
