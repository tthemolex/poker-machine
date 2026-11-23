// =============================================================================
//  Full assembly: every printed and bought part placed.
//  openscad -D turret_deg=30 -D rack_pos=40 -D explode=0.6 -D section=true assembly.scad
//  -D only="name"      renders a single placed part (used by tools/cad_*.py)
//  -D only="__list__"  echoes the part registry (name, group) and renders a dummy
// =============================================================================
include <params.scad>
include <lib/lib.scad>
use <base_shell.scad>
use <battery_door.scad>
use <base_deck.scad>
use <hall_post.scad>
use <ui_bezel.scad>
use <turntable.scad>
use <rack_comb.scad>
use <rack_bottom_plate.scad>
use <rack_top_plate.scad>
use <rack_carriage.scad>
use <rack_nut_arm.scad>
use <rack_motor_mount.scad>
use <rail_frame.scad>
use <hopper_body.scad>
use <hopper_gate.scad>
use <hopper_weight.scad>
use <hopper_lid.scad>
use <loader_frame.scad>
use <loader_coupler.scad>
use <pusher_guide.scad>
use <pusher_slider.scad>
use <pusher_pinion.scad>
use <servo_mount.scad>
use <launcher_frame.scad>
use <launcher_floor.scad>
use <idler_arm.scad>
use <burn_tray.scad>
use <turret_shell.scad>
use <top_cap.scad>
use <vendor/nema17_pancake.scad>
use <vendor/nema17_t8.scad>
use <vendor/t8_nut.scad>
use <vendor/rod_8mm.scad>
use <vendor/lm8uu.scad>
use <vendor/lazy_susan_120.scad>
use <vendor/gt2_pulley_20t.scad>
use <vendor/mg90s.scad>
use <vendor/n20_motor.scad>
use <vendor/motor_130.scad>
use <vendor/wheel_launcher.scad>
use <vendor/idler_roller.scad>
use <vendor/feed_roller.scad>
use <vendor/shaft_3mm.scad>
use <vendor/esp32_devkit.scad>
use <vendor/tmc2209.scad>
use <vendor/drv8833.scad>
use <vendor/mini560.scad>
use <vendor/battery_talentcell.scad>
use <vendor/dc_jack_panel.scad>
use <vendor/rocker_switch.scad>
use <vendor/button_12mm.scad>
use <vendor/led_5mm.scad>
use <vendor/gt2_belt.scad>
use <vendor/ssd1306.scad>
use <vendor/a3144.scad>
use <vendor/magnet_6x3.scad>
use <vendor/ir_beam.scad>
use <vendor/microswitch.scad>
use <vendor/pusher_blade.scad>
use <vendor/spring_5x10.scad>
use <vendor/m3_screw.scad>
use <vendor/m3_insert.scad>

turret_deg = 0;      // 0..360
rack_pos   = 30;     // 0..RACK_TRAVEL (rack bottom above RACK_Z_MIN)
explode    = 0;      // 0..1
section    = false;  // cut away X > 0 (plane through rack / pusher / launcher)
only       = "";     // render one part by name
show_cards = true;
hide       = [];     // part names to leave out (render views)
only_base  = false;  // base group only, deck/bearing hidden (electronics view)
EXPLODE_MM = 130;

// ---- registry: [name, group, colour, explode direction] ----
//      group: base (static) | turret (rotates) | rack (rotates + rides rack_pos)
PARTS = [
 ["base_shell",        "base",   [0.20,0.21,0.23], [0,0,-0.9]],
 ["battery_door",      "base",   [0.30,0.31,0.34], [0,0,-1.5]],
 ["base_deck",         "base",   [0.58,0.62,0.68], [0,0,-0.25]],
 ["hall_post",         "base",   [0.95,0.45,0.20], [0,0,-0.15]],
 ["ui_bezel",          "base",   [0.07,0.07,0.08], [0,-0.6,-0.9]],
 ["turntable",         "turret", [0.30,0.52,0.78], [0,0,0]],
 ["rack_comb_px",      "rack",   [0.95,0.55,0.15], [0.25,0,0.75]],
 ["rack_comb_nx",      "rack",   [0.95,0.55,0.15], [-0.25,0,0.75]],
 ["rack_bottom_plate", "rack",   [0.98,0.82,0.25], [0,0,0.6]],
 ["rack_top_plate",    "rack",   [0.98,0.82,0.25], [0,0,0.95]],
 ["rack_carriage",     "rack",   [0.15,0.68,0.58], [0.6,0,0.75]],
 ["rack_nut_arm",      "rack",   [0.60,0.30,0.72], [-0.6,0,0.75]],
 ["rack_motor_mount",  "turret", [0.85,0.30,0.30], [-0.9,0,0.1]],
 ["rail_frame",        "turret", [0.30,0.42,0.85], [0.9,0,0.1]],
 ["loader_frame",      "turret", [0.48,0.74,0.34], [0,-0.8,0.1]],
 ["hopper_body",       "turret", [0.74,0.84,0.95], [0,-0.8,0.7]],
 ["hopper_gate",       "turret", [0.35,0.55,0.92], [0,-0.4,0.9]],
 ["hopper_weight",     "turret", [0.42,0.42,0.47], [0,-0.8,1.1]],
 ["loader_coupler",    "turret", [0.92,0.90,0.30], [0.5,-0.8,0.2]],
 ["pusher_guide",      "turret", [0.95,0.68,0.75], [0,-0.8,0.35]],
 ["pusher_slider",     "turret", [0.88,0.36,0.52], [0,-0.8,0.25]],
 ["pusher_pinion",     "turret", [0.65,0.15,0.30], [0,-0.8,0.12]],
 ["servo_mount",       "turret", [0.62,0.50,0.38], [0.3,-0.8,0.0]],
 ["launcher_frame",    "turret", [0.30,0.74,0.86], [0,0.8,0.45]],
 ["launcher_floor",    "turret", [0.16,0.50,0.62], [0,0.8,0.2]],
 ["idler_arm",         "turret", [0.96,0.84,0.36], [0,0.8,0.05]],
 ["burn_tray",         "turret", [0.86,0.86,0.83], [0,1.4,0]],
 ["turret_shell",      "turret", [0.93,0.93,0.90], [0,0,1.25]],
 ["top_cap",           "turret", [0.97,0.97,0.95], [0,0,1.6]],
 ["hopper_lid",        "turret", [0.80,0.80,0.78], [0,-0.3,1.8]],
 // ---- vendor ----
 ["v_battery",         "base",   undef, [0,0,-1.25]],
 ["v_nema17_pancake",  "base",   undef, [0,0.4,-0.55]],
 ["v_gt2_pulley_20t",  "base",   undef, [0,0.4,-0.3]],
 ["v_lazy_susan_120",  "base",   undef, [0,0,-0.12]],
 ["v_esp32_devkit",    "base",   undef, [0,-0.3,-0.6]],
 ["v_tmc2209_a",       "base",   undef, [-0.2,-0.3,-0.6]],
 ["v_tmc2209_b",       "base",   undef, [0.2,-0.3,-0.6]],
 ["v_drv8833",         "base",   undef, [0.2,0.3,-0.6]],
 ["v_mini560",         "base",   undef, [-0.2,0.3,-0.6]],
 ["v_dc_jack_panel",   "base",   undef, [0.5,0.4,-0.5]],
 ["v_rocker_switch",   "base",   undef, [-0.5,0.4,-0.5]],
 ["v_button_go",       "base",   undef, [-0.1,-0.9,-0.9]],
 ["v_button_set",      "base",   undef, [0.1,-0.9,-0.9]],
 ["v_led_5mm",         "base",   undef, [0,-0.9,-0.9]],
 ["v_ssd1306",         "base",   undef, [0,-0.75,-0.9]],
 ["v_a3144",           "base",   undef, [0,0,-0.2]],
 ["v_deck_screws",     "base",   undef, [0,0,-0.25]],
 ["v_deck_inserts",    "base",   undef, [0,0,-0.9]],
 ["v_magnet_6x3",      "turret", undef, [0,0,-0.05]],
 ["v_gt2_belt",        "turret", undef, [0,0,-0.08]],
 ["v_nema17_t8",       "turret", undef, [-0.9,0,0.1]],
 ["v_rods",            "turret", undef, [0.9,0,0.1]],
 ["v_microswitch",     "turret", undef, [0.9,0,0.2]],
 ["v_feed_rollers",    "turret", undef, [0,-0.8,0.25]],
 ["v_feed_shaft",      "turret", undef, [0,-0.8,0.25]],
 ["v_n20_motor",       "turret", undef, [0.5,-0.8,0.25]],
 ["v_ir_load_tx",      "turret", undef, [0,-0.4,0.9]],
 ["v_ir_load_rx",      "turret", undef, [0,-0.8,0.35]],
 ["v_pusher_blade",    "turret", undef, [0,-0.8,0.3]],
 ["v_mg90s",           "turret", undef, [0.3,-0.8,0.0]],
 ["v_motor_130",       "turret", undef, [0,0.8,0.45]],
 ["v_wheel_launcher",  "turret", undef, [0,0.8,0.45]],
 ["v_idler_roller",    "turret", undef, [0,0.8,0.05]],
 ["v_idler_shafts",    "turret", undef, [0,0.8,0.05]],
 ["v_spring_idler",    "turret", undef, [0,0.8,0.12]],
 ["v_ir_exit_tx",      "turret", undef, [0,0.8,0.45]],
 ["v_ir_exit_rx",      "turret", undef, [0,0.8,0.2]],
 ["v_t8_nut",          "rack",   undef, [-0.6,0,0.75]],
 ["v_lm8uu",           "rack",   undef, [0.6,0,0.75]],
 ["v_nut_screws",      "rack",   undef, [-0.6,0,0.85]],
 ["v_nut_inserts",     "rack",   undef, [-0.6,0,0.75]],
 // ---- cards ----
 ["cards_rack",        "rack",   [0.99,0.97,0.90], [0,0,0.75]],
 ["card_eject",        "rack",   [0.95,0.30,0.25], [0,0,0.75]],
 ["card_load",         "turret", [0.25,0.55,0.95], [0,-0.4,0.9]],
 ["cards_hopper",      "turret", [0.99,0.97,0.90], [0,-0.8,1.05]],
];

// ---- card state derived from rack_pos ----
_ie = round(SHELF_COUNT - 1 - rack_pos / SHELF_PITCH);        // shelf at the eject station
_il = _ie + round(STATION_OFFSET / SHELF_PITCH);               // shelf at the load station (load is 3 pitches above eject)
EJECT_OK = _ie >= 0 && _ie < SHELF_COUNT;
LOAD_OK  = _il >= 0 && _il < SHELF_COUNT;
RACK_CARDS = [for (i = [0, 1, 2, 3, 5, 8, 12, 17, 23, 30, 38, 47]) if (i != _ie && i != _il) i];
HOP_N = 8;
HOP_CY = (HOP_YI0 + HOP_YI1) / 2;
LOAD_FRONT = 30;                                               // load card front edge (Y) while feeding

module ip_draw(name) {
    if (name == "base_shell") base_shell();
    else if (name == "battery_door") battery_door();
    else if (name == "base_deck") base_deck();
    else if (name == "ui_bezel") ui_bezel();
    else if (name == "hall_post") hall_post();
    else if (name == "turntable") turntable();
    else if (name == "rack_comb_px") rack_comb();
    else if (name == "rack_comb_nx") translate([0, RACK_YC, 0]) rotate(180) translate([0, -RACK_YC, 0]) rack_comb();
    else if (name == "rack_bottom_plate") rack_bottom_plate();
    else if (name == "rack_top_plate") rack_top_plate();
    else if (name == "rack_carriage") rack_carriage();
    else if (name == "rack_nut_arm") rack_nut_arm();
    else if (name == "rack_motor_mount") rack_motor_mount();
    else if (name == "rail_frame") rail_frame();
    else if (name == "loader_frame") loader_frame();
    else if (name == "hopper_body") hopper_body();
    else if (name == "hopper_gate") hopper_gate();
    else if (name == "hopper_weight") hopper_weight(Z_LOAD + (HOP_N + 1) * CARD_T);
    else if (name == "hopper_lid") hopper_lid();
    else if (name == "loader_coupler") loader_coupler();
    else if (name == "pusher_guide") pusher_guide();
    else if (name == "pusher_slider") pusher_slider();
    else if (name == "pusher_pinion") pusher_pinion();
    else if (name == "servo_mount") servo_mount();
    else if (name == "launcher_frame") launcher_frame();
    else if (name == "launcher_floor") launcher_floor();
    else if (name == "idler_arm") idler_arm();
    else if (name == "burn_tray") burn_tray();
    else if (name == "turret_shell") turret_shell();
    else if (name == "top_cap") top_cap();
    // vendor
    else if (name == "v_battery") translate([0, 0, FLOOR_T]) vendor_battery_talentcell();
    else if (name == "v_nema17_pancake") translate([0, TM_Y, TM_FACE_Z - NEMA_PANCAKE_L]) vendor_nema17_pancake();
    else if (name == "v_gt2_pulley_20t") translate([0, TM_Y, TM_FACE_Z + NEMA_BOSS_H + 0.2]) vendor_gt2_pulley_20t();
    else if (name == "v_lazy_susan_120") translate([0, 0, LS_Z]) vendor_lazy_susan_120();
    else if (name == "v_esp32_devkit") translate(ESP_POS) vendor_esp32_devkit();
    else if (name == "v_tmc2209_a") translate(TMC_POS[0]) vendor_tmc2209();
    else if (name == "v_tmc2209_b") translate(TMC_POS[1]) vendor_tmc2209();
    else if (name == "v_drv8833") translate(DRV_POS) vendor_drv8833();
    else if (name == "v_mini560") translate(M560_POS) rotate(90) vendor_mini560();
    else if (name == "v_dc_jack_panel") rotate(JACK_A) translate([TURRET_R - UI_INSET, 0, PWR_Z]) rotate([0, 90, 0]) vendor_dc_jack_panel();
    else if (name == "v_rocker_switch") rotate(ROCKER_A) translate([TURRET_R - UI_INSET, 0, PWR_Z]) rotate([0, 90, 0]) rotate(90) vendor_rocker_switch();
    else if (name == "v_button_go") rotate(ui_az(-BTN_X)) translate([TURRET_R - UI_INSET, 0, BTN_Z]) rotate([0, 90, 0]) vendor_button_12mm();
    else if (name == "v_button_set") rotate(ui_az(BTN_X)) translate([TURRET_R - UI_INSET, 0, BTN_Z]) rotate([0, 90, 0]) vendor_button_12mm();
    else if (name == "v_led_5mm") rotate(ui_az(LED_X)) translate([TURRET_R - UI_INSET, 0, LED_Z]) rotate([0, 90, 0]) vendor_led_5mm();
    else if (name == "v_ssd1306") translate([0, -sqrt((BASE_RI - 0.4) ^ 2 - 13.6 ^ 2) + 0.05, OLED_Z]) rotate([90, 0, 0]) vendor_ssd1306();
    else if (name == "v_a3144") translate([HALL_R, 0, TURNTABLE_Z - 3.0]) vendor_a3144();
    else if (name == "v_deck_screws") for (a = DECK_SCREW_A) rotate(a) translate([DECK_SCREW_R, 0, DECK_Z + DECK_T]) vendor_m3_screw(8);
    else if (name == "v_deck_inserts") for (a = DECK_SCREW_A) rotate(a) translate([DECK_SCREW_R, 0, DECK_Z]) vendor_m3_insert();
    else if (name == "v_magnet_6x3") translate([HALL_R, 0, TURNTABLE_Z + 2.1 - 3]) vendor_magnet_6x3();
    else if (name == "v_gt2_belt") translate([0, 0, PUL_TOP_Z - PUL_TEETH_H / 2 - BELT_W / 2]) vendor_gt2_belt();
    else if (name == "v_nema17_t8") translate([LM_X, LM_Y, LM_Z0]) vendor_nema17_t8();
    else if (name == "v_rods") for (y = ROD_Y) translate([ROD_X, y, ROD_Z0]) vendor_rod_8mm();
    else if (name == "v_microswitch") translate([ROD_X, MSW_Y, RAIL_BOT_Z1 + 0.1 - 6.5]) vendor_microswitch();
    else if (name == "v_feed_rollers") for (s = [-1, 1]) translate([s * FEED_X, FEED_Y, FEED_Z]) rotate([0, 90, 0]) vendor_feed_roller();
    else if (name == "v_feed_shaft") translate([SHAFT_X0, FEED_Y, FEED_Z]) rotate([0, 90, 0]) vendor_shaft_3mm(SHAFT_X1 - SHAFT_X0);
    else if (name == "v_n20_motor") multmatrix([[0, 0, -1, N20_FACE_X], [-1, 0, 0, FEED_Y], [0, 1, 0, FEED_Z], [0, 0, 0, 1]]) vendor_n20_motor();
    else if (name == "v_ir_load_tx") translate([LOADIR_X, LOADIR_Y, Z_LOAD + THROAT]) vendor_ir_beam(false);
    else if (name == "v_ir_load_rx") translate([LOADIR_X, LOADIR_Y, Z_LOAD - HOP_FLOOR_T - 0.1]) mirror([0, 0, 1]) vendor_ir_beam(true);
    else if (name == "v_pusher_blade") translate([0, BLADE_TIP_Y, BLADE_Z0]) vendor_pusher_blade();
    else if (name == "v_mg90s") multmatrix([[0, 0, -1, 29.9], [1, 0, 0, PIN_Y], [0, -1, 0, PIN_Z], [0, 0, 0, 1]]) vendor_mg90s();
    else if (name == "v_motor_130") multmatrix([[0, 0, -1, M130_X0], [0, 1, 0, NIP_Y], [1, 0, 0, WHEEL_Z], [0, 0, 0, 1]]) vendor_motor_130();
    else if (name == "v_wheel_launcher") translate([0, NIP_Y, WHEEL_Z]) rotate([0, 90, 0]) vendor_wheel_launcher();
    else if (name == "v_idler_roller") translate([0, IDL_A[0], Z_EJECT + IDL_A[1]]) rotate([0, 90, 0]) vendor_idler_roller();
    else if (name == "v_idler_shafts") {
        translate([-8.8, IDL_A[0], Z_EJECT + IDL_A[1]]) rotate([0, 90, 0]) vendor_shaft_3mm(17.6);
        for (x0 = [-12.2, 6.5]) translate([x0, IDL_P[0], Z_EJECT + IDL_P[1]]) rotate([0, 90, 0]) vendor_shaft_3mm(5.7);   // two stub pivot pins
    }
    else if (name == "v_spring_idler") translate([0, IDL_SPRING_Y, Z_EJECT + IDL_L[1] + 1.25]) vendor_spring_5x10(-3 - IDL_L[1] - 1.25);
    else if (name == "v_ir_exit_tx") translate([EXITIR_X, EXITIR_Y, Z_EJECT + 2]) vendor_ir_beam(false);
    else if (name == "v_ir_exit_rx") translate([EXITIR_X, EXITIR_Y, Z_EJECT - 0.3]) mirror([0, 0, 1]) vendor_ir_beam(true);
    else if (name == "v_t8_nut") translate([LM_X, LM_Y, RACK_Z_MIN + ARM_Z + ARM_T]) vendor_t8_nut();
    else if (name == "v_lm8uu") for (y = ROD_Y, z = LM8_Z) translate([ROD_X, y, RACK_Z_MIN + z]) vendor_lm8uu();
    else if (name == "v_nut_screws") for (a = [45 : 90 : 359]) translate([LM_X + NUT_PCD / 2 * cos(a), LM_Y + NUT_PCD / 2 * sin(a), RACK_Z_MIN + ARM_Z + ARM_T + NUT_FL_T]) vendor_m3_screw(8);
    else if (name == "v_nut_inserts") for (a = [45 : 90 : 359]) translate([LM_X + NUT_PCD / 2 * cos(a), LM_Y + NUT_PCD / 2 * sin(a), RACK_Z_MIN + ARM_Z + ARM_T]) vendor_m3_insert();
    // cards
    else if (name == "cards_rack") { if (show_cards) for (i = RACK_CARDS) card_at(RACK_YC, RACK_Z_MIN + shelf_z(i)); }
    else if (name == "card_eject") { if (show_cards && EJECT_OK) card_at(RACK_YC, RACK_Z_MIN + shelf_z(_ie)); }
    else if (name == "card_load") { if (show_cards) { if (LOAD_OK) card_at(LOAD_FRONT - CARD_W / 2, Z_LOAD); else card_at(HOP_CY, Z_LOAD + HOP_N * CARD_T); } }
    else if (name == "cards_hopper") { if (show_cards) for (k = [0 : HOP_N - 1]) card_at(HOP_CY, Z_LOAD + (k + (LOAD_OK ? 1 : 0)) * CARD_T); }
    else echo(str("UNKNOWN PART ", name));
}

module ip_section(col) {
    if (section) intersection() { children(); color(is_undef(col) ? [0.55, 0.55, 0.58] : col) translate([-500, -500, -10]) cube([500, 1000, 400]); }
    else children();
}
module ip_place(p) {
    name = p[0]; grp = p[1]; col = p[2]; ex = p[3];
    ip_section(col)
    rotate(grp == "base" ? 0 : turret_deg)
    translate(explode * EXPLODE_MM * ex)
    translate([0, 0, grp == "rack" ? rack_pos : 0])
    if (is_undef(col)) ip_draw(name); else color(col) ip_draw(name);
}

BASE_HIDE = ["base_deck", "v_lazy_susan_120", "hall_post", "v_a3144", "v_deck_screws"];
function hidden(n) = len([for (h = hide) if (h == n) 1]) > 0 || (only_base && len([for (h = BASE_HIDE) if (h == n) 1]) > 0);
function grp(n) = [for (p = PARTS) if (p[0] == n) p[1]][0];

if (only == "__list__") {
    for (p = PARTS) echo(str("PART|", p[0], "|", p[1]));
    cube(1);
} else {
    for (p = PARTS) if ((only == "" && !hidden(p[0]) && (!only_base || p[1] == "base")) || only == p[0]) ip_place(p);
}
