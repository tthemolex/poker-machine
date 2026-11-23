// =============================================================================
//  params.scad: ALL shared dimensions for the poker dealer.
//
//  Lines tagged  // @shared  are the single source of truth for values used by
//  the firmware too. tools/gen_geometry.py parses them (simple arithmetic on
//  earlier names only) and writes firmware/include/geometry.h.
//  tools/check_consistency.py fails the build if the two drift apart.
//  Keep @shared lines of the form:  NAME = <expr>; // @shared <comment>
// =============================================================================

// ---------------- Card ----------------
CARD_L = 88.9;          // @shared card long side (along X)
CARD_W = 63.5;          // @shared card short side (along Y, travel direction)
CARD_T = 0.30;          // @shared card thickness

// ---------------- Rack ----------------
SHELF_COUNT    = 52;    // @shared
SHELF_PITCH    = 1.5;   // @shared mm, see DECISIONS D-01
SHELF_FIN      = 0.9;   // @shared fin thickness (noted exception to 1.2 mm wall rule)
SHELF_GAP      = SHELF_PITCH - SHELF_FIN;   // @shared card slot height
RACK_CAP       = 1.2;   // @shared bottom plate thickness below shelf 0 slot
STATION_OFFSET = 3 * SHELF_PITCH;           // @shared load station above eject station
RACK_Z_MIN     = 44.5;  // @shared world z of rack bottom when homed (microswitch)
RACK_TRAVEL    = (SHELF_COUNT - 1) * SHELF_PITCH + STATION_OFFSET; // @shared mm
// world z of the card-bottom plane at the eject station (shelf 51 is there when homed)
Z_EJECT = RACK_Z_MIN + RACK_CAP + (SHELF_COUNT - 1) * SHELF_PITCH; // @shared
Z_LOAD  = Z_EJECT + STATION_OFFSET;                                // @shared

// ---------------- Rack drive (NEMA17 with integrated T8 leadscrew) ----------------
LEAD_MM       = 8;      // @shared T8x8 (4 starts)
MOTOR_STEPS   = 200;    // @shared full steps / rev
RACK_USTEPS   = 8;      // @shared TMC2209 MS1=MS2=GND
RACK_STEPS_PER_MM = MOTOR_STEPS * RACK_USTEPS / LEAD_MM;           // @shared

// ---------------- Turret drive ----------------
TURRET_PINION_T = 20;   // @shared GT2 pulley on the static NEMA17
TURRET_RING_T   = 200;  // @shared printed GT2-profile ring under the turntable, D-02
TURRET_USTEPS   = 16;   // @shared TMC2209 MS1=MS2=VCC
TURRET_RATIO    = TURRET_RING_T / TURRET_PINION_T;                 // @shared
TURRET_STEPS_PER_DEG = MOTOR_STEPS * TURRET_USTEPS * TURRET_RATIO / 360; // @shared
GT2_PITCH = 2;

// ---------------- Table / aiming ----------------
BOARD_DEG        = 8;   // @shared turret angle of the board centre; 8 = 2*spread so the fan [0,16] never wraps past 0/360 (hall home = 0)
BOARD_SPREAD_DEG = 4;   // @shared fan of the 5 board cards (index-2..+2)
TURRET_MAX_DEG   = 360; // @shared never rotate beyond 0..360 (cable loop)

// ---------------- Envelope limits ----------------
ENV_MAX_D   = 200;      // @shared hard limit footprint diameter
ENV_MAX_H   = 220;      // @shared hard limit height
ENV_STRETCH_D = 170;    // @shared
ENV_STRETCH_H = 180;    // @shared

// ---------------- Print tolerances / fasteners ----------------
TOL        = 0.2;       // default print clearance (per side)
MIN_WALL   = 1.2;
M3_CLEAR   = 3.4;       // M3 clearance hole
M3_INSERT_D = 4.0;      // hole for M3 x 5.7 heat-set insert (OD 4.6)
M3_INSERT_L = 6.0;
M3_HEAD_D  = 6.0;  M3_HEAD_H = 3.0;

// ---------------- Base stack (world z) ----------------
FLOOR_T      = 1.6;
BATT_L = 145; BATT_W = 85; BATT_H = 28;     // TalentCell YB1206000-USB (maker manual)
DECK_Z       = FLOOR_T + BATT_H + 0.4;      // underside of deck
DECK_T       = 2.0;
LS_OD = 120; LS_ID = 70; LS_H = 9.5;        // VXB 120 mm round lazy susan
LS_Z         = DECK_Z + DECK_T;             // bearing bottom
TURNTABLE_Z  = LS_Z + LS_H;                 // turntable underside
TURNTABLE_T  = 2.5;
// sanity: rack must clear the turntable
assert(RACK_Z_MIN >= TURNTABLE_Z + TURNTABLE_T + 0.5 - 1e-6, "rack collides with turntable");

// --- everything below is owned by the CAD agent; add freely, keep names stable ---

// =============================================================================
//  CAD-owned parameters (mechanical agent). Frames: see docs/ARCHITECTURE.md.
//  World z=0 under the base, turret axis = Z, launcher shoots +Y at 0 deg.
//  Everything is derived from the @shared block where it can be.
// =============================================================================
EPS = 0.01;
CLR = 0.5;                       // running clearance moving <-> static parts
PI_ = 3.14159265358979;

// ---------------- Envelope / shells ----------------
TURRET_R   = 97.5;               // outer radius: turret shell, turntable, base
SHELL_T    = 1.8;                // turret shell wall
TURRET_RI  = TURRET_R - SHELL_T; // usable inner radius in the turret
BASE_WALL  = 2.0;
BASE_RI    = TURRET_R - BASE_WALL;
BASE_TOP_Z = TURNTABLE_Z - 0.5;  // base wall top, 0.5 shadow gap under turntable
TT_TOP     = TURNTABLE_Z + TURNTABLE_T;
CAP_T      = 2.0;                // top cap thickness
CAP_SKIRT  = 10;                 // cap skirt height (shell/cap split below cap)

// ---------------- Rack ----------------
RACK_Y0 = -12;  RACK_Y1 = 52;    // open faces (back = load/pusher, front = launcher)
RACK_YC = (RACK_Y0 + RACK_Y1) / 2;
RACK_LY = RACK_Y1 - RACK_Y0;
RACK_TOP_T = 1.2;                // top plate above slot 51
RACK_H = RACK_CAP + (SHELF_COUNT - 1) * SHELF_PITCH + SHELF_GAP + RACK_TOP_T;
CARD_CLR_X = 0.5;                // card end clearance to comb wall (per side)
CARD_R     = 3.2;                // card corner radius
COMB_IN    = CARD_L / 2 + CARD_CLR_X;    // comb wall inner face |X|
FIN_W      = 3.0;                // fin depth (card rests 2.5 mm on each fin)
FIN_IN     = COMB_IN - FIN_W;    // fin tip |X|
COMB_WALL  = 4.5;
COMB_OUT   = COMB_IN + COMB_WALL;
PLATE_REBATE = 2.5;              // plates sit in rebates in the comb walls
CARR_PAD   = 3.0;                // +X comb pad that carries the carriage
function shelf_z(i) = RACK_CAP + i * SHELF_PITCH;  // card-bottom plane, rel. rack bottom
RACK_TOP_MAX = RACK_Z_MIN + RACK_TRAVEL + RACK_H;  // highest point of the rack
CAP_IN_Z   = RACK_TOP_MAX + CLR;  // underside of top cap
CAP_TOP_Z  = CAP_IN_Z + CAP_T;    // overall height
SHELL_Z1   = CAP_TOP_Z - CAP_SKIRT; // turret shell top / cap split

// ---------------- Guide rods + carriage (+X side) ----------------
ROD_D = 8;  ROD_X = 62;  ROD_Y = [2, 38];
LM8_D = 15; LM8_L = 24;  LM8_Z = [14, 42];     // rel. rack bottom
CARR_Z0 = 12; CARR_Z1 = 68;                    // carriage extent rel. rack bottom
CARR_WALL = 1.6;
RAIL_BOT_Z1 = RACK_Z_MIN + CARR_Z0 - CLR;
RAIL_TOP_Z0 = RACK_Z_MIN + RACK_TRAVEL + CARR_Z1 + CLR;
RAIL_TOP_Z1 = CAP_IN_Z - CLR;
RAIL_WEB_X0 = 72.5; RAIL_WEB_X1 = 75;
ROD_Z0 = TT_TOP + 2;  ROD_Z1 = RAIL_TOP_Z1 - 2;
ROD_L  = ROD_Z1 - ROD_Z0;
MSW_Y  = (ROD_Y[0] + ROD_Y[1]) / 2;           // rack-bottom microswitch

// ---------------- Rack drive (-X side) ----------------
NEMA_W = 42.3;  NEMA_CH = 3.5;  NEMA_HOLE = 31;  NEMA_BOSS_D = 22; NEMA_BOSS_H = 2;
NEMA_T8_L = 40;  NEMA_PANCAKE_L = 23;
LEADSCREW_L = 100;               // D-10: 100 mm (150 mm does not fit the height)
LEAD_D = 8;
LM_X = -(COMB_OUT + CLR + 0.4 + NEMA_W / 2);
LM_Y = 0;
LM_Z0 = TT_TOP + 1;              // motor body bottom (sits on its mount)
LM_FACE = LM_Z0 + NEMA_T8_L;
LM_PLATE_T = 4;                  // mount plate on the motor face
NUT_FL_D = 22; NUT_FL_T = 3.5; NUT_BODY_D = 10.2; NUT_L = 15; NUT_PCD = 16;
ARM_T = 6;
ARM_Z = LM_FACE + NEMA_BOSS_H + CLR + (NUT_L - NUT_FL_T - ARM_T) - RACK_Z_MIN; // rel. rack bottom
assert(RACK_Z_MIN + RACK_TRAVEL + ARM_Z + ARM_T + NUT_FL_T <= LM_FACE + LEADSCREW_L, "nut runs off the leadscrew");
assert(RACK_Z_MIN + ARM_Z >= LM_FACE + LM_PLATE_T + CLR, "arm hits motor plate");

// ---------------- Hopper / loader (-Y side, load station) ----------------
HOP_XI = 45;  HOP_WALL = 1.6;  HOP_XO = HOP_XI + HOP_WALL;
HOP_Y_FRONT = RACK_Y0 - 0.8;     // gate front face (static, behind the rack)
GATE_T = 6.4;
HOP_YI0 = HOP_Y_FRONT - GATE_T;  // card zone front
HOP_YI1 = HOP_YI0 - (CARD_W + 1.0);
HOP_YO1 = HOP_YI1 - HOP_WALL;
HOP_CR  = 4.4;                   // inner corner radius (card corner r3.2 fits up to 4.5)
HOP_FLOOR_T = 1.6;
HOP_Z1  = CAP_IN_Z - 4.0;       // clears the cap's inner rim chamfer at the hopper corners
THROAT  = 0.45;                  // single-card gap under the gate
FEED_D = 14; FEED_W = 8; FEED_X = 32; FEED_Y = -26;
FEED_Z = Z_LOAD - FEED_D / 2;    // roller tops flush with the hopper floor
SHAFT_D = 3;
SHAFT_X0 = -(HOP_XO + 0.4);  SHAFT_X1 = 51.5;
N20_FACE_X = 61.5;               // N20 gearbox output face
LOADIR_X = 18;  LOADIR_Y = (HOP_Y_FRONT + HOP_YI0) / 2;  // load break-beam
WEIGHT_T = 10;

// ---------------- Pusher (eject station) ----------------
BLADE_W = 24;  BLADE_T = 0.5;
BLADE_Z0 = Z_EJECT - 0.30;       // D-13: straddles the card edge incl. sag
BLADE_TIP_Y = RACK_Y0 - 1.0;     // retracted tip
PIN_M = 1;  PIN_T = 15;  PIN_R = PIN_M * PIN_T / 2;  PIN_W = 6;
SERVO_SWEEP = 150;               // deg, PUSH_HOME 10 -> PUSH_OUT 160 in firmware
PUSH_STROKE = PIN_R * SERVO_SWEEP * PI_ / 180;
CARRIER_L = 33;  CARRIER_Y0 = -48.5;   // carrier front (retracted)
CARRIER_TOP = BLADE_Z0;
CARRIER_Z0 = Z_EJECT - 7.3;
RACKG_PITCH_Z = CARRIER_Z0 - 1.25 * PIN_M;  // gear-rack pitch line
PIN_Y = -56;  PIN_Z = RACKG_PITCH_Z - PIN_R;
BLADE_L = (BLADE_TIP_Y - CARRIER_Y0) + 15;
assert(CARRIER_Y0 + PUSH_STROKE <= FEED_Y - SHAFT_D / 2 - 1, "carrier hits feed shaft");

// ---------------- Launcher (+Y side, eject station) ----------------
NIP_Y = 65;  WHEEL_D = 24;  WHEEL_W = 10;  IDLER_D = 12;  IDLER_W = 12;
NIP_GAP = 0.1;                   // wheel above idler top (card 0.30 squeezed 0.2)
WHEEL_Z = Z_EJECT + NIP_GAP + WHEEL_D / 2;
IDLER_Z = Z_EJECT - IDLER_D / 2;
NIP_GRIP = 2;                    // card must pass the nip by this much
FINGER_Y = RACK_Y1 + CLR;       // load-stop face: a loaded card stops 0.75 mm proud of nominal
LF_Y0 = FINGER_Y + CLR;  LF_Y1 = 80;   // every other static launcher face clears a stopped card by 0.5
LF_XI = 46.4;  LF_XO = 48.4;
LF_TOP_Z0 = Z_EJECT + 24.8;  LF_TOP_T = 3;
M130_X0 = 7;                     // 130 motor face
EXITIR_X = -22;  EXITIR_Y = 56;
FINGER_X = 40;
SLOT_Z0 = Z_EJECT - 1.5;  SLOT_Z1 = Z_EJECT + 4.5;   // exit slot in the shell
assert(NIP_Y - WHEEL_D / 2 >= RACK_Y1 + CLR, "launcher wheel overlaps rack sweep");

// ---------------- Burn tray (drawer under the exit) ----------------
BT_XO = 46.2;  BT_Y0 = 54;  BT_Z0 = TT_TOP + 0.5;  BT_Z1 = 106;  BT_WALL = 1.6;

// ---------------- Base ----------------
BATT_CLR = 0.3;  CRADLE_T = 1.6;
CR_XI = BATT_L / 2 + BATT_CLR;  CR_YI = BATT_W / 2 + BATT_CLR;
CR_XO = CR_XI + CRADLE_T;       CR_YO = CR_YI + CRADLE_T;
RING_PR = TURRET_RING_T * GT2_PITCH / (2 * PI_);    // 63.66
PUL_PR  = TURRET_PINION_T * GT2_PITCH / (2 * PI_);  // 6.37
RING_RI = LS_OD / 2 + 1.0;
BELT_BACK = 0.63;  BELT_W = 6;   // GT2 belt glued teeth-out on the printed hub (D-11)
RING_HUB_R = RING_PR - BELT_BACK;      // printed hub outer radius
RING_LAND = RING_PR;                   // belt land between teeth (pulley tips touch here)
RING_TIP  = RING_LAND + 0.75;
TM_Y    = RING_LAND + PUL_PR - 0.254;  // turret motor axis (static, +Y crescent)
PUL_TOP_Z = TURNTABLE_Z - 0.5;
PUL_TEETH_H = 8;  PUL_FL_T = 1;  PUL_HUB_H = 5;  PUL_HUB_D = 16;
RING_Z0 = PUL_TOP_Z - PUL_TEETH_H + 0.2;
TM_FACE_Z = PUL_TOP_Z - PUL_TEETH_H - PUL_FL_T - PUL_HUB_H - 0.2 - NEMA_BOSS_H;
HALL_R = 26;                     // hall sensor + magnet radius (inside bearing ID), at +X
CABLE_HOLE = [0, -24];  CABLE_HOLE_D = 16;   // turntable cable pass (in bearing ID)
DECK_SCREW_R = 88;  DECK_SCREW_A = [25, 155, 205, 335];
UI_Z = 24;                       // UI row centre height on the base wall
// ---------------- Base layout (world, static) ----------------
UI_NOTCH_X = 44;  UI_NOTCH_Z0 = 8;
OLED_Z = 26;  BTN_X = 33;  BTN_Z = 20;  LED_X = -21;  LED_Z = 14;
ESP_POS = [0, -59.65, FLOOR_T + 10];  TMC_POS = [[-50, -58, FLOOR_T + 10], [50, -58, FLOOR_T + 10]];
DRV_POS = [45, 60, FLOOR_T + 6.4];   M560_POS = [-45, 60, FLOOR_T + 3];
ROCKER_A = 140;  JACK_A = 40;  PWR_Z = 20;
DOOR_SCREWS = [[-77.8, -20], [-77.8, 20], [77.8, -20], [77.8, 20]];
DOOR_REBATE = 4;  DOOR_REBATE_T = 1.2;  DOOR_RING_T = 1.2;   // raised floor ring keeps 1.6 mm over the rebate
// turntable fixing points (CSK M3 from below into inserts in the turret parts).
// Keep clear of the ring gear (r 60..66).
FIX_RAIL   = [[69, -4], [69, 44]];
FIX_LMM    = [[-86, -26], [-86, 26], [-66, -27], [-66, 27]];
FIX_LOADER = [[-40, -20], [40, -20], [-40, -78], [40, -78]];
FIX_LAUNCH = [[-51.4, 58], [51.4, 58], [-51.4, 70], [51.4, 70]];
SHELL_TAB_A = [30, 150, 210, 330];  SHELL_TAB_R = TURRET_RI - 3;
function polar(r, a) = [r * cos(a), r * sin(a)];
FIX_SHELL  = [for (a = SHELL_TAB_A) polar(SHELL_TAB_R, a)];
FIX_ALL    = concat(FIX_RAIL, FIX_LMM, FIX_LOADER, FIX_LAUNCH, FIX_SHELL);
// ---------------- Rack fastening (pilot holes for M3 thread-forming screws) ----------------
PILOT_D = 2.5;  PILOT_L = 4.0;
CARR_HOLES = [[12, 22], [28, 22], [12, 58], [28, 58]];               // [y, z rel. rack bottom]
ARM_HOLES  = [[-5, ARM_Z + 10], [5, ARM_Z + 10], [-5, ARM_Z + 17], [5, ARM_Z + 17]];
// ---------------- Loader frame / pusher guide joints ----------------
WING_F = [-42, -34];  WING_B = [-81, -78];        // pusher-guide wing Y bands
WING_FIX = [[-39.25, -38], [39.25, -38], [-39.25, -77], [39.25, -77]];  // CSK M3 into loader-frame ledges
SERVO_FIX = [[20, -38], [20, -77]];               // servo plate -> wing inserts
WING_Z0 = Z_EJECT - 5.5;  WING_Z1 = Z_EJECT + 1.95;
LEDGE_Z0 = WING_Z0 - 6.5;
HOP_TAB_Y = [-40, -70];
// ---------------- Launcher idler linkage (Y, z rel. Z_EJECT) ----------------
IDL_A = [NIP_Y, -IDLER_D / 2];      // idler axle
IDL_P = [57.5, -10.5];              // pivot (8.7 mm from the idler axle)
IDL_L = [55.6, -13.0];              // lever end (spring seat)
IDL_SPRING_Y = IDL_L[0];
LFL_Y1 = NIP_Y - IDLER_D / 2 - 0.5; // launcher floor end (just before the idler)
UI_INSET = 0.6;                  // flat UI parts sit this far inside the curved skin (flush, corners do not protrude)
function ui_az(x) = 270 + asin(x / TURRET_R);    // azimuth of a UI element at chord offset x (front, -Y)
