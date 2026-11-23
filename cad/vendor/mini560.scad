// Vendor model: Mini560 12V->5V 5A synchronous buck module. Local: PCB bottom z=0, centred.
include <../params.scad>
include <../lib/lib.scad>
module vendor_mini560() {
    color([0.1,0.35,0.15]) translate([-11, -8.5, 0]) cube([22, 17, 1.6]);
    color([0.25,0.25,0.27]) translate([-2, -4, 1.6]) cube([8, 8, 5]);
    color([0.3,0.3,0.3]) translate([-9, -6, 1.6]) cube([5, 4, 2]);
}

vendor_mini560();
