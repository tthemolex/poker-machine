// Servo plate: holds the MG90S (shaft along -X) under the carrier; screws up into the
// pusher-guide wings. Print: lying on its -X face, flanges up, no supports.
include <params.scad>
include <lib/lib.scad>
e = Z_EJECT;
SM_X0 = 14.3;  SM_X1 = 16.8;
module servo_mount() difference() {
    union() {
        box_at(SM_X0, SM_X1, SERVO_FIX[1][1] - 4, SERVO_FIX[0][1] + 4, PIN_Z - 8.5, WING_Z0);
        for (p = SERVO_FIX) box_at(SM_X0, 24, p[1] - 4, p[1] + 4, WING_Z0 - 2.5, WING_Z0);
    }
    box_at(SM_X0 - 1, SM_X1 + 1, PIN_Y - 16.9 - 0.3, PIN_Y + 5.9 + 0.3, PIN_Z - 6.1 - 0.3, PIN_Z + 6.1 + 0.3);
    for (y = [PIN_Y - 19.3, PIN_Y + 8.3]) translate([SM_X0 - 1, y, PIN_Z]) rotate([0, 90, 0]) cylinder(d = M2_PILOT_D, h = 5);
    for (p = SERVO_FIX) translate([p[0], p[1], WING_Z0 - 3]) m3_clear(4);
}
module servo_mount_print() rotate([0, -90, 0]) translate([-SM_X0, 0, 0]) servo_mount();
servo_mount_print();
