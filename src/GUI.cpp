// ============================================================
//  GUI.cpp  —  Auton Selector + Debug for ZIPTIDE
//  Team 5069G  |  V5RC Override 2026-27
//  LVGL 9.2  (liblvgl, PROS 4 / kernel 4.2.1)
//
//  COLOR SCHEME: black bg, red accents, cream text. No blues.
// ============================================================

#include "GUI.h"
#include "Autons.h"
#include "main.h"
// ─────────────────────────────────────────────
//  COLOR HELPER
// ─────────────────────────────────────────────
static inline lv_color_t C(uint8_t r, uint8_t g, uint8_t b) {
    return lv_color_make(r, g, b);
}

// ── ZIPTIDE PALETTE (reds only, no blue/amber) ──
#define CLR_BG        C(13,  13,  13)    // near-black
#define CLR_PANEL     C(18,  10,  10)    // very dark red-tinted panel
#define CLR_RED       C(204, 34,  34)    // primary red
#define CLR_RED2      C(224, 51,  51)    // lighter red
#define CLR_RED3      C(255, 68,  68)    // bright red (alerts)
#define CLR_REDDIM    C(80,  18,  18)    // dark red (borders, dividers)
#define CLR_REDDIM2   C(50,  12,  12)    // very dark red (unselected bg)
#define CLR_CREAM     C(232, 224, 208)   // primary text
#define CLR_CREAM2    C(170, 155, 135)   // secondary text / labels
#define CLR_UNSEL     C(28,  18,  18)    // unselected button bg
#define CLR_GREEN     C(68,  187, 119)   // bat/temp ok
#define CLR_YELLOW    C(232, 184, 75)    // bat/temp warn
#define CLR_WHITE     C(255, 255, 255)

// ─────────────────────────────────────────────
//  AUTON LIST
// ─────────────────────────────────────────────
struct AutonEntry {
    AutonomousID id;
    const char*  name;
    const char*  desc;
    lv_color_t   selBg;   // selected button background
};

static const AutonEntry autonList[] = {
    {AUTON_NONE,        "DO NOTHING",   "Safe fallback",        C(36,  20, 20)},
    {AUTON_SPLIT_LEFT,  "SPLIT LEFT",   "Left side split",      C(60,  14, 14)},
    {AUTON_SPLIT_RIGHT, "SPLIT RIGHT",  "Right side split",     C(60,  14, 14)},
    {AUTON_LEFT_WING,   "LEFT WING",    "Wing push left",       C(80,  16, 16)},
    {AUTON_RIGHT_WING,  "RIGHT WING",   "Wing push right",      C(80,  16, 16)},
    {AUTON_SAWP,        "SAWP",         "Full SAWP routine",    C(100, 20, 20)},
    {AUTON_SKILLS,      "SKILLS",       "Programming skills",   C(120, 20, 20)},
};
static const int AUTON_COUNT = sizeof(autonList) / sizeof(autonList[0]);

// Pagination
#define AUTONS_PER_FIRST_PAGE  4
#define AUTONS_PER_MID_PAGE    3
#define AUTONS_PER_LAST_PAGE   4

// ─────────────────────────────────────────────
//  GLOBALS
// ─────────────────────────────────────────────
volatile int selectedAuton = AUTON_RIGHT_WING;

static pros::Task* debugTask         = nullptr;
static int         currentPage       = 0;
static lv_obj_t*   s_debugScreen     = nullptr;
static bool        s_selectorShowing = false;

// Selector right-panel labels
static lv_obj_t* rp_name = nullptr;
static lv_obj_t* rp_desc = nullptr;
static lv_obj_t* rp_id   = nullptr;

// Debug screen labels
static lv_obj_t *lbl_x, *lbl_y, *lbl_h;
static lv_obj_t *lbl_auton;
static lv_obj_t *lbl_bat, *lbl_bat_bar_inner;
static lv_obj_t *lbl_conn;
static lv_obj_t *lbl_td;   // drivetrain avg temp
static lv_obj_t *lbl_tc;   // claw temp
static lv_obj_t *lbl_tl;   // lift temp
static lv_obj_t *lbl_tf;   // 4-bar temp

static lv_obj_t* leftColumn = nullptr;

// Layout — exact V5 brain screen: 480x272
static const int SW    = 480;
static const int SH    = 272;
static const int TOP   = 26;   // title bar height
static const int LX    = 6;    // left column x
static const int LW    = 210;  // left column width
static const int BTN_H = 36;
static const int GAP   = 4;

// ─────────────────────────────────────────────
//  HELPERS
// ─────────────────────────────────────────────
static lv_obj_t* make_label(lv_obj_t* parent, const char* text,
                             int x, int y,
                             lv_color_t col, const lv_font_t* font) {
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_pos(l, x, y);
    lv_obj_set_style_text_color(l, col, 0);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_bg_opa(l, LV_OPA_TRANSP, 0);
    return l;
}

static lv_obj_t* make_rect(lv_obj_t* parent,
                            int x, int y, int w, int h,
                            lv_color_t bg, int radius = 0) {
    lv_obj_t* p = lv_obj_create(parent);
    lv_obj_set_size(p, w, h);
    lv_obj_set_pos(p, x, y);
    lv_obj_set_style_bg_color(p, bg, 0);
    lv_obj_set_style_bg_opa(p, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(p, 0, 0);
    lv_obj_set_style_pad_all(p, 0, 0);
    lv_obj_set_style_radius(p, radius, 0);
    lv_obj_set_scrollbar_mode(p, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(p, LV_OBJ_FLAG_SCROLLABLE);
    return p;
}

static void make_hline(lv_obj_t* parent, int x, int y, int w) {
    make_rect(parent, x, y, w, 1, CLR_REDDIM);
}

// Corner bracket: two 1px bars forming an L corner
static void make_bracket(lv_obj_t* parent, int x, int y,
                          bool mirrorX, bool mirrorY) {
    const int LEN = 12;
    make_rect(parent, mirrorX ? x - LEN : x, y, LEN, 1, CLR_RED);
    make_rect(parent, mirrorX ? x - 1 : x, mirrorY ? y - LEN : y, 1, LEN, CLR_RED);
}

static void update_right_panel() {
    if (!rp_name || !rp_desc || !rp_id) return;
    for (int i = 0; i < AUTON_COUNT; i++) {
        if (autonList[i].id == (AutonomousID)selectedAuton) {
            lv_label_set_text(rp_name, autonList[i].name);
            lv_label_set_text(rp_desc, autonList[i].desc);
            char buf[16];
            snprintf(buf, sizeof(buf), "ID: %d", (int)autonList[i].id);
            lv_label_set_text(rp_id, buf);
            return;
        }
    }
}

// ─────────────────────────────────────────────
//  FORWARD DECL
// ─────────────────────────────────────────────
static void rebuild_left_column();

// ─────────────────────────────────────────────
//  CALLBACKS
// ─────────────────────────────────────────────
static void auton_btn_cb(lv_event_t* e) {
    selectedAuton = (int)(intptr_t)lv_event_get_user_data(e);
    update_right_panel();
    rebuild_left_column();
}

static void up_cb(lv_event_t*) {
    if (currentPage > 0) { currentPage--; rebuild_left_column(); }
}

static void down_cb(lv_event_t*) {
    int start = (currentPage == 0) ? 0
        : AUTONS_PER_FIRST_PAGE + (currentPage - 1) * AUTONS_PER_MID_PAGE;
    int slots = (currentPage == 0) ? AUTONS_PER_FIRST_PAGE : AUTONS_PER_MID_PAGE;
    if (start + slots < AUTON_COUNT) { currentPage++; rebuild_left_column(); }
}

static void test_btn_cb(lv_event_t*) {
    static bool running = false;
    if (running) return;
    running = true;
    new pros::Task([](void*) { autonomous(); },
                   nullptr, TASK_PRIORITY_DEFAULT + 1, 0x8000, "test_auton");
}

// ─────────────────────────────────────────────
//  BUTTON BUILDERS
// ─────────────────────────────────────────────
static lv_obj_t* make_auton_btn(lv_obj_t* parent, int slotY, int idx) {
    const AutonEntry& e   = autonList[idx];
    bool              sel = (e.id == (AutonomousID)selectedAuton);

    lv_obj_t* btn = lv_obj_create(parent);
    lv_obj_set_size(btn, LW, BTN_H);
    lv_obj_set_pos(btn, 0, slotY);
    lv_obj_set_style_bg_color(btn, sel ? e.selBg : CLR_UNSEL, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn, sel ? CLR_RED : CLR_REDDIM, 0);
    lv_obj_set_style_border_width(btn, sel ? 2 : 1, 0);
    lv_obj_set_style_radius(btn, 5, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, CLR_REDDIM, LV_STATE_PRESSED);
    lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);

    // Red left accent stripe on selected
    if (sel) make_rect(btn, 0, 0, 3, BTN_H, CLR_RED);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, e.name);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, sel ? CLR_WHITE : CLR_CREAM2, 0);
    lv_obj_set_pos(lbl, 10, BTN_H / 2 - 8);

    lv_obj_add_event_cb(btn, auton_btn_cb, LV_EVENT_CLICKED,
                        (void*)(intptr_t)e.id);
    return btn;
}

static lv_obj_t* make_arrow_btn(lv_obj_t* parent, int slotY,
                                  bool isUp, lv_event_cb_t cb) {
    lv_obj_t* btn = lv_obj_create(parent);
    lv_obj_set_size(btn, LW, BTN_H);
    lv_obj_set_pos(btn, 0, slotY);
    lv_obj_set_style_bg_color(btn, C(22, 10, 10), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn, CLR_REDDIM, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_radius(btn, 5, 0);
    lv_obj_set_style_pad_all(btn, 4, 0);
    lv_obj_set_style_bg_color(btn, CLR_REDDIM, LV_STATE_PRESSED);
    lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, isUp ? LV_SYMBOL_UP : LV_SYMBOL_DOWN);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, CLR_RED2, 0);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    return btn;
}

// ─────────────────────────────────────────────
//  REBUILD LEFT COLUMN  (pagination)
// ─────────────────────────────────────────────
static void rebuild_left_column() {
    if (!leftColumn) return;
    lv_obj_clean(leftColumn);

    int startIdx = (currentPage == 0) ? 0
        : AUTONS_PER_FIRST_PAGE + (currentPage - 1) * AUTONS_PER_MID_PAGE;

    bool hasUp   = (currentPage > 0);
    int  slots   = hasUp ? AUTONS_PER_MID_PAGE : AUTONS_PER_FIRST_PAGE;
    int  endIdx  = startIdx + slots;
    if (endIdx > AUTON_COUNT) endIdx = AUTON_COUNT;
    bool hasDown = (endIdx < AUTON_COUNT);

    // Last page: no down arrow so fit one extra row
    if (hasUp && !hasDown) {
        int extra = startIdx + AUTONS_PER_LAST_PAGE;
        endIdx = (extra > AUTON_COUNT) ? AUTON_COUNT : extra;
    }

    int slot = 0;
    auto Y = [](int s) { return s * (BTN_H + GAP); };

    if (hasUp)   make_arrow_btn(leftColumn, Y(slot++), true,  up_cb);
    for (int i = startIdx; i < endIdx; i++)
        make_auton_btn(leftColumn, Y(slot++), i);
    if (hasDown) make_arrow_btn(leftColumn, Y(slot), false, down_cb);
}

// ─────────────────────────────────────────────
//  SELECTOR SCREEN
// ─────────────────────────────────────────────
static lv_obj_t* build_selector() {
    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, CLR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // Title bar background + bottom edge
    make_rect(scr, 0, 0, SW, TOP, C(20, 6, 6));
    make_rect(scr, 0, TOP - 1, SW, 1, CLR_RED);

    // "Z" badge
    lv_obj_t* badge = make_rect(scr, 6, 4, 18, 18, CLR_RED, 3);
    lv_obj_t* bLbl  = lv_label_create(badge);
    lv_label_set_text(bLbl, "Z");
    lv_obj_set_style_text_font(bLbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(bLbl, CLR_WHITE, 0);
    lv_obj_align(bLbl, LV_ALIGN_CENTER, 0, 0);

    // Title text
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "ZIPTIDE  |  SELECT AUTONOMOUS  |  5069G");
    lv_obj_set_style_text_color(title, CLR_RED2, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(title, 30, 6);

    // Corner brackets
    make_bracket(scr, LX,      TOP,      false, false);
    make_bracket(scr, SW - 6,  SH - 6,   true,  true);

    // Left column container
    leftColumn = lv_obj_create(scr);
    lv_obj_set_size(leftColumn, LW, SH - TOP - 6);
    lv_obj_set_pos(leftColumn, LX, TOP + 3);
    lv_obj_set_style_bg_opa(leftColumn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(leftColumn, 0, 0);
    lv_obj_set_style_pad_all(leftColumn, 0, 0);
    lv_obj_set_style_radius(leftColumn, 0, 0);
    lv_obj_set_scrollbar_mode(leftColumn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(leftColumn, LV_OBJ_FLAG_SCROLLABLE);

    currentPage = 0;
    rebuild_left_column();

    // Right info panel
    const int RX = LX + LW + 8;
    const int RW = SW - RX - 6;
    const int RH = SH - TOP - 6;

    lv_obj_t* rp = lv_obj_create(scr);
    lv_obj_set_size(rp, RW, RH);
    lv_obj_set_pos(rp, RX, TOP + 3);
    lv_obj_set_style_bg_color(rp, CLR_PANEL, 0);
    lv_obj_set_style_bg_opa(rp, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(rp, CLR_REDDIM, 0);
    lv_obj_set_style_border_width(rp, 1, 0);
    lv_obj_set_style_radius(rp, 5, 0);
    lv_obj_set_style_pad_all(rp, 10, 0);
    lv_obj_set_scrollbar_mode(rp, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(rp, LV_OBJ_FLAG_SCROLLABLE);

    make_label(rp, "SELECTED AUTON", 0,   0,  CLR_RED2,   &lv_font_montserrat_10);
    make_hline(rp, 0, 14, RW - 20);

    rp_name = make_label(rp, "",  0,  20, CLR_WHITE,  &lv_font_montserrat_14);
    rp_desc = make_label(rp, "",  0,  40, CLR_CREAM2, &lv_font_montserrat_12);
    rp_id   = make_label(rp, "",  0,  56, CLR_REDDIM, &lv_font_montserrat_10);

    make_hline(rp, 0, 72, RW - 20);
    make_label(rp, "5069G",    0,  80, CLR_RED2,   &lv_font_montserrat_14);
    make_label(rp, "ZIPTIDE",  0,  98, CLR_CREAM2, &lv_font_montserrat_10);
    make_label(rp, "OVERRIDE", 0, 112, CLR_CREAM2, &lv_font_montserrat_10);

    // Run button
    lv_obj_t* runBtn = lv_obj_create(rp);
    lv_obj_set_size(runBtn, RW - 20, 28);
    lv_obj_set_pos(runBtn, 0, RH - 48);
    lv_obj_set_style_bg_color(runBtn, C(13, 50, 22), 0);
    lv_obj_set_style_bg_opa(runBtn, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(runBtn, C(8, 30, 14), LV_STATE_PRESSED);
    lv_obj_set_style_border_color(runBtn, C(26, 100, 46), 0);
    lv_obj_set_style_border_width(runBtn, 1, 0);
    lv_obj_set_style_radius(runBtn, 4, 0);
    lv_obj_set_style_pad_all(runBtn, 0, 0);
    lv_obj_set_scrollbar_mode(runBtn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(runBtn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(runBtn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* runLbl = lv_label_create(runBtn);
    lv_label_set_text(runLbl, LV_SYMBOL_PLAY " RUN AUTON (TEST)");
    lv_obj_set_style_text_font(runLbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(runLbl, CLR_GREEN, 0);
    lv_obj_align(runLbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(runBtn, test_btn_cb, LV_EVENT_CLICKED, nullptr);

    update_right_panel();
    return scr;
}

// ─────────────────────────────────────────────
//  DEBUG SCREEN
// ─────────────────────────────────────────────
static lv_obj_t* build_debug() {
    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, CLR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // Title bar — same style as selector
    make_rect(scr, 0, 0, SW, TOP, C(20, 6, 6));
    make_rect(scr, 0, TOP - 1, SW, 1, CLR_RED);

    lv_obj_t* dbgTitle = lv_label_create(scr);
    lv_label_set_text(dbgTitle, "ZIPTIDE DEBUG  |  5069G");
    lv_obj_set_style_text_color(dbgTitle, CLR_RED2, 0);
    lv_obj_set_style_text_font(dbgTitle, &lv_font_montserrat_12, 0);
    lv_obj_set_pos(dbgTitle, SW / 2 - 78, 6);

    // Two-column layout constants
    const int C1    = 8;           // left col x
    const int C2    = SW / 2 + 6;  // right col x
    const int Y0    = TOP + 6;     // first row y
    const int LH    = 20;          // row height
    const int LBL_W = 42;          // label width before value

    // Vertical divider
    make_rect(scr, SW / 2 - 1, TOP + 2, 1, SH - TOP - 4, CLR_REDDIM);

    // ── LEFT COLUMN ─────────────────────────

    // ODOMETRY
    make_label(scr, "ODOMETRY", C1, Y0, CLR_RED2, &lv_font_montserrat_10);
    make_hline(scr, C1, Y0 + 12, SW / 2 - 14);

    make_label(scr, "X",   C1,           Y0 + 16,          CLR_CREAM2, &lv_font_montserrat_10);
    lbl_x = make_label(scr, "---", C1 + LBL_W, Y0 + 14,          CLR_CREAM, &lv_font_montserrat_14);

    make_label(scr, "Y",   C1,           Y0 + 16 + LH,     CLR_CREAM2, &lv_font_montserrat_10);
    lbl_y = make_label(scr, "---", C1 + LBL_W, Y0 + 14 + LH,     CLR_CREAM, &lv_font_montserrat_14);

    make_label(scr, "HDG", C1,           Y0 + 16 + LH * 2, CLR_CREAM2, &lv_font_montserrat_10);
    lbl_h = make_label(scr, "---", C1 + LBL_W, Y0 + 14 + LH * 2, CLR_CREAM, &lv_font_montserrat_14);

    // AUTON
    const int AU_Y = Y0 + 16 + LH * 3 + 4;
    make_label(scr, "AUTON", C1, AU_Y, CLR_RED2, &lv_font_montserrat_10);
    make_hline(scr, C1, AU_Y + 12, SW / 2 - 14);
    lv_obj_t* autonBox = make_rect(scr, C1, AU_Y + 16, SW / 2 - 18, 18, CLR_REDDIM2, 3);
    lbl_auton = make_label(autonBox, "---", 5, 2, CLR_CREAM, &lv_font_montserrat_12);

    // CONNECTION
    const int CN_Y = AU_Y + 38;
    make_label(scr, "CONN",  C1,           CN_Y, CLR_CREAM2, &lv_font_montserrat_10);
    lbl_conn = make_label(scr, "FIELD", C1 + LBL_W, CN_Y - 2, CLR_GREEN, &lv_font_montserrat_12);

    // ── RIGHT COLUMN ────────────────────────

    // BATTERY
    make_label(scr, "BATTERY", C2, Y0, CLR_RED2, &lv_font_montserrat_10);
    make_hline(scr, C2, Y0 + 12, SW / 2 - 14);

    make_label(scr, "BAT", C2,           Y0 + 16, CLR_CREAM2, &lv_font_montserrat_10);
    lbl_bat = make_label(scr, "---%", C2 + LBL_W, Y0 + 14, CLR_GREEN, &lv_font_montserrat_14);

    // Battery fill bar
    lv_obj_t* batOuter   = make_rect(scr, C2, Y0 + 32, SW / 2 - 18, 6, C(30, 10, 10), 3);
    lbl_bat_bar_inner    = make_rect(batOuter, 0, 0, 80, 6, CLR_GREEN, 3);

    // MOTOR TEMPS
    const int TMP_Y = Y0 + 46;
    make_label(scr, "MOTOR TEMPS", C2, TMP_Y, CLR_RED2, &lv_font_montserrat_10);
    make_hline(scr, C2, TMP_Y + 12, SW / 2 - 14);

    make_label(scr, "DRIVE", C2,           TMP_Y + 16,          CLR_CREAM2, &lv_font_montserrat_10);
    lbl_td = make_label(scr, "--°C", C2 + LBL_W, TMP_Y + 14,          CLR_GREEN, &lv_font_montserrat_14);

    make_label(scr, "CLAW",  C2,           TMP_Y + 16 + LH,     CLR_CREAM2, &lv_font_montserrat_10);
    lbl_tc = make_label(scr, "--°C", C2 + LBL_W, TMP_Y + 14 + LH,     CLR_GREEN, &lv_font_montserrat_14);

    make_label(scr, "LIFT",  C2,           TMP_Y + 16 + LH * 2, CLR_CREAM2, &lv_font_montserrat_10);
    lbl_tl = make_label(scr, "--°C", C2 + LBL_W, TMP_Y + 14 + LH * 2, CLR_GREEN, &lv_font_montserrat_14);

    make_label(scr, "4-BAR", C2,           TMP_Y + 16 + LH * 3, CLR_CREAM2, &lv_font_montserrat_10);
    lbl_tf = make_label(scr, "--°C", C2 + LBL_W, TMP_Y + 14 + LH * 3, CLR_GREEN, &lv_font_montserrat_14);

    return scr;
}

// ─────────────────────────────────────────────
//  DEBUG TASK
// ─────────────────────────────────────────────
static lv_color_t tempColor(double t) {
    if (t < 45.0) return CLR_GREEN;
    if (t < 58.0) return CLR_YELLOW;
    return CLR_RED3;
}

void GUI_debugTask(void*) {
    char buf[32];
    while (true) {
        // Odometry
        auto pose = chassis.getPose();
        snprintf(buf, sizeof(buf), "%.1f",  pose.x);     lv_label_set_text(lbl_x, buf);
        snprintf(buf, sizeof(buf), "%.1f",  pose.y);     lv_label_set_text(lbl_y, buf);
        snprintf(buf, sizeof(buf), "%.1f°", pose.theta); lv_label_set_text(lbl_h, buf);

        // Auton name
        for (int i = 0; i < AUTON_COUNT; i++) {
            if (autonList[i].id == (AutonomousID)selectedAuton) {
                lv_label_set_text(lbl_auton, autonList[i].name);
                break;
            }
        }

        // Battery
        int batt = pros::battery::get_capacity();
        lv_color_t bc = (batt > 60) ? CLR_GREEN : (batt > 30 ? CLR_YELLOW : CLR_RED3);
        lv_obj_set_style_text_color(lbl_bat, bc, 0);
        snprintf(buf, sizeof(buf), "%d%%", batt);
        lv_label_set_text(lbl_bat, buf);
        int barW = batt * 2; // 100% = 200px
        if (barW < 2)   barW = 2;
        if (barW > 200) barW = 200;
        lv_obj_set_size(lbl_bat_bar_inner, barW, 6);
        lv_obj_set_style_bg_color(lbl_bat_bar_inner, bc, 0);

        // Connection
        bool comp = pros::competition::is_connected();
        lv_obj_set_style_text_color(lbl_conn, comp ? CLR_GREEN : CLR_YELLOW, 0);
        lv_label_set_text(lbl_conn, comp ? "FIELD" : "CTRLR");

        // Drivetrain avg temp — update motor names to match your globals
        double td = (DriveL.get_temperature() + DriveR.get_temperature()) / 2.0;
        lv_obj_set_style_text_color(lbl_td, tempColor(td), 0);
        snprintf(buf, sizeof(buf), "%.0f°C", td);
        lv_label_set_text(lbl_td, buf);

        // Claw — update ClawMotor to match your global
        double tc = Claw.get_temperature();
        lv_obj_set_style_text_color(lbl_tc, tempColor(tc), 0);
        snprintf(buf, sizeof(buf), "%.0f°C", tc);
        lv_label_set_text(lbl_tc, buf);

        // Lift — update LiftMotor to match your global
        double tl = Lift.get_temperature();
        lv_obj_set_style_text_color(lbl_tl, tempColor(tl), 0);
        snprintf(buf, sizeof(buf), "%.0f°C", tl);
        lv_label_set_text(lbl_tl, buf);

        // 4-bar — update FourBarMotor to match your global
        double tf = FourBar.get_temperature();
        lv_obj_set_style_text_color(lbl_tf, tempColor(tf), 0);
        snprintf(buf, sizeof(buf), "%.0f°C", tf);
        lv_label_set_text(lbl_tf, buf);

        pros::delay(200);
    }
}

// ─────────────────────────────────────────────
//  PUBLIC API
// ─────────────────────────────────────────────
void GUI_initDebugTask() {
    if (!s_debugScreen)
        s_debugScreen = build_debug();
    lv_screen_load(s_debugScreen);
    s_selectorShowing = false;
    if (!debugTask)
        debugTask = new pros::Task(GUI_debugTask);
}

void GUI_runAutonSelector() {
    if (s_selectorShowing) return;
    currentPage = 0;
    rp_name = rp_desc = rp_id = nullptr;
    leftColumn = nullptr;
    lv_obj_t* scr = build_selector();
    lv_screen_load(scr);
    s_selectorShowing = true;
}

void GUI_showDebugScreen() {
    GUI_initDebugTask();
}