// Echo the values the CAD check scripts need (VAL|name|value). Used by tools/cad_*.py.
include <params.scad>
include <lib/lib.scad>
use <pusher_pinion.scad>
module v(n, x) echo(str("VAL|", n, "|", x));
v("CARD_L", CARD_L); v("CARD_W", CARD_W); v("CARD_T", CARD_T);
v("SHELF_COUNT", SHELF_COUNT); v("SHELF_PITCH", SHELF_PITCH); v("SHELF_GAP", SHELF_GAP); v("SHELF_FIN", SHELF_FIN);
v("RACK_CAP", RACK_CAP); v("RACK_Z_MIN", RACK_Z_MIN); v("RACK_TRAVEL", RACK_TRAVEL); v("STATION_OFFSET", STATION_OFFSET);
v("Z_EJECT", Z_EJECT); v("Z_LOAD", Z_LOAD); v("RACK_H", RACK_H);
v("RACK_Y0", RACK_Y0); v("RACK_Y1", RACK_Y1); v("RACK_YC", RACK_YC);
v("COMB_IN", COMB_IN); v("FIN_IN", FIN_IN); v("COMB_OUT", COMB_OUT);
v("HOP_XI", HOP_XI); v("HOP_YI0", HOP_YI0); v("HOP_YI1", HOP_YI1); v("HOP_Y_FRONT", HOP_Y_FRONT);
v("THROAT", THROAT); v("FEED_X", FEED_X); v("FEED_Y", FEED_Y); v("FEED_D", FEED_D); v("FEED_Z", FEED_Z);
v("BLADE_Z0", BLADE_Z0); v("BLADE_T", BLADE_T); v("BLADE_W", BLADE_W); v("BLADE_TIP_Y", BLADE_TIP_Y);
v("PIN_R", PIN_R); v("SERVO_SWEEP", SERVO_SWEEP); v("PUSH_STROKE", PUSH_STROKE);
v("NIP_Y", NIP_Y); v("NIP_GAP", NIP_GAP); v("NIP_GRIP", NIP_GRIP); v("WHEEL_D", WHEEL_D); v("IDLER_D", IDLER_D);
v("LF_Y0", LF_Y0); v("LFL_Y1", LFL_Y1); v("FINGER_X", FINGER_X);
v("TURRET_R", TURRET_R); v("CAP_TOP_Z", CAP_TOP_Z);
v("ENV_MAX_D", ENV_MAX_D); v("ENV_MAX_H", ENV_MAX_H); v("ENV_STRETCH_D", ENV_STRETCH_D); v("ENV_STRETCH_H", ENV_STRETCH_H);
v("TOL", TOL); v("M3_CLEAR", M3_CLEAR); v("M3_INSERT_D", M3_INSERT_D); v("MIN_WALL", MIN_WALL);
v("LEADSCREW_L", LEADSCREW_L); v("LM_FACE", LM_FACE); v("ARM_Z", ARM_Z); v("ARM_T", ARM_T); v("NUT_FL_T", NUT_FL_T); v("NUT_L", NUT_L);
v("PIN_PHASE", PIN_PHASE);
cube(1);
