// Vendor model: GT2 belt 6 mm x 400 mm (200 T) glued teeth-out around the turntable hub. Local: centred on the turret axis, z 0..BELT_W.
include <../params.scad>
include <../lib/lib.scad>
module vendor_gt2_belt() color([0.1,0.1,0.1]) linear_extrude(BELT_W) gt2_ring2d(TURRET_RING_T, RING_LAND, RING_HUB_R + 0.001);
vendor_gt2_belt();
