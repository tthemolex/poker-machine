// Vendor model: LM8UU linear ball bearing 8x15x24. Local: z 0..24.
include <../params.scad>
include <../lib/lib.scad>
module vendor_lm8uu() color([0.6,0.62,0.66]) difference() {
    cylinder(d = LM8_D, h = LM8_L);
    translate([0, 0, -1]) cylinder(d = ROD_D + 0.2, h = LM8_L + 2);
    for (z = [3.3, LM8_L - 4.4]) translate([0, 0, z]) ring(LM8_D / 2 + 1, 14.3 / 2, 1.1);
}

vendor_lm8uu();
