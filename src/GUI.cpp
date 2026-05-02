// ============================================================
//  GUI.cpp  —  Auton Selector + Debug | ZIPTIDE 5069G
//  V5RC Override 2026-27  |  LVGL 9.2  |  PROS 4
//
//  DESIGN: Black bg, crimson red accents, cream text.
//  Selector: full-width card grid with side info panel.
//  Debug:    two-column live telemetry dashboard.
// ============================================================

#include "GUI.h"
#include "Autons.h"
#include "liblvgl/core/lv_obj_tree.h"
#include "main.h"

// ─────────────────────────────────────────────────────────────
//  PALETTE
// ─────────────────────────────────────────────────────────────
static inline lv_color_t C(uint8_t r, uint8_t g, uint8_t b) {
  return lv_color_make(r, g, b);
}

// Core
#define CLR_BG C(10, 10, 10)       // true black bg
#define CLR_SURFACE C(18, 12, 12)  // card surface
#define CLR_SURFACE2 C(24, 16, 16) // slightly lighter surface

// Red ramp
#define CLR_RED_HOT C(220, 38, 38) // primary red
#define CLR_RED_MID C(185, 28, 28) // mid red (borders)
#define CLR_RED_DIM C(90, 20, 20)  // dim red (unselected borders)
#define CLR_RED_DEEP C(40, 12, 12) // very dark red (unsel bg)

// Text
#define CLR_TEXT_PRI C(236, 228, 212) // cream primary
#define CLR_TEXT_SEC C(160, 148, 128) // cream secondary
#define CLR_TEXT_DIM C(80, 72, 60)    // dim hint text

// Status
#define CLR_GREEN C(52, 211, 153)      // ok
#define CLR_YELLOW C(251, 191, 36)     // warn
#define CLR_RED_ALERT C(248, 113, 113) // alert/error
#define CLR_WHITE C(255, 255, 255)

// ─────────────────────────────────────────────────────────────
//  SCREEN CONSTANTS  (V5 Brain: 480 × 272)
// ─────────────────────────────────────────────────────────────
#define SW 480
#define SH 272
#define TH 24 // title bar height

// ─────────────────────────────────────────────────────────────
//  AUTON REGISTRY
// ─────────────────────────────────────────────────────────────
struct AutonEntry {
  AutonomousID id;
  const char *name;
  const char *desc;
  const char *side; // "LEFT" | "RIGHT" | "SKILLS" | "SAFE"
};

static const AutonEntry kAutons[] = {
    {AUTON_NONE, "Do Nothing", "Safe fallback — no movement", "SAFE"},
    {AUTON_LEFT, "Split Left", "Left-side split push", "LEFT"},
    {AUTON_RIGHT, "Split Right", "Right-side split push", "RIGHT"},
    {AUTON_SKILLS, "Skills", "Programming skills run", "SKILLS"},
};
static const int kAutonCount = (int)(sizeof(kAutons) / sizeof(kAutons[0]));

volatile int selectedAuton = AUTON_RIGHT;

// ─────────────────────────────────────────────────────────────
//  STATE
// ─────────────────────────────────────────────────────────────
static pros::Task *s_debugTask = nullptr;
static lv_obj_t *s_selectorScr = nullptr;
static lv_obj_t *s_debugScr = nullptr;
static bool s_debugRunning = false;

// Selector — right panel labels
static lv_obj_t *s_rpName = nullptr;
static lv_obj_t *s_rpDesc = nullptr;
static lv_obj_t *s_rpSide = nullptr;
static lv_obj_t *s_rpID = nullptr;

// Grid button array (for highlight refresh)
static lv_obj_t *s_btns[kAutonCount] = {};

// Debug labels
static lv_obj_t *s_lX, *s_lY, *s_lH;
static lv_obj_t *s_lAuton;
static lv_obj_t *s_lBat, *s_lBatBar;
static lv_obj_t *s_lConn;
static lv_obj_t *s_lTDrive, *s_lTClaw, *s_lTFour, *s_lTLift;

// ─────────────────────────────────────────────────────────────
//  LOW-LEVEL HELPERS
// ─────────────────────────────────────────────────────────────
static lv_obj_t *mk_rect(lv_obj_t *par, int x, int y, int w, int h,
                         lv_color_t bg, int r = 0) {
  lv_obj_t *o = lv_obj_create(par);
  lv_obj_set_size(o, w, h);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_style_bg_color(o, bg, 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(o, 0, 0);
  lv_obj_set_style_radius(o, r, 0);
  lv_obj_set_style_pad_all(o, 0, 0);
  lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
  return o;
}

static lv_obj_t *mk_label(lv_obj_t *par, const char *txt, int x, int y,
                          lv_color_t col, const lv_font_t *font) {
  lv_obj_t *l = lv_label_create(par);
  lv_label_set_text(l, txt);
  lv_obj_set_pos(l, x, y);
  lv_obj_set_style_text_color(l, col, 0);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_bg_opa(l, LV_OPA_TRANSP, 0);
  return l;
}

static void mk_hline(lv_obj_t *par, int x, int y, int w) {
  mk_rect(par, x, y, w, 1, CLR_RED_DIM);
}

// Pill / badge
static lv_obj_t *mk_pill(lv_obj_t *par, const char *txt, int x, int y,
                         lv_color_t bg, lv_color_t fg,
                         const lv_font_t *font = &lv_font_montserrat_10) {
  lv_obj_t *p = mk_rect(par, x, y, 0, 14, bg, 7);
  lv_obj_set_width(p, LV_SIZE_CONTENT);
  lv_obj_set_style_pad_hor(p, 6, 0);
  lv_obj_set_style_pad_ver(p, 2, 0);

  lv_obj_t *l = lv_label_create(p);
  lv_label_set_text(l, txt);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, fg, 0);
  lv_obj_align(l, LV_ALIGN_CENTER, 0, 0);
  return p;
}

// ─────────────────────────────────────────────────────────────
//  SIDE BADGE COLOR
// ─────────────────────────────────────────────────────────────
static lv_color_t sideColor(const char *side) {
  if (strcmp(side, "LEFT") == 0)
    return C(37, 99, 235); // blue
  if (strcmp(side, "RIGHT") == 0)
    return C(220, 38, 38); // red
  if (strcmp(side, "SKILLS") == 0)
    return C(124, 58, 237); // purple
  return C(55, 55, 55);     // gray (SAFE)
}

// ─────────────────────────────────────────────────────────────
//  SELECTOR — RIGHT PANEL UPDATE
// ─────────────────────────────────────────────────────────────
static void rp_refresh() {
  if (!s_rpName)
    return;
  for (int i = 0; i < kAutonCount; i++) {
    if (kAutons[i].id != (AutonomousID)selectedAuton)
      continue;
    lv_label_set_text(s_rpName, kAutons[i].name);
    lv_label_set_text(s_rpDesc, kAutons[i].desc);
    lv_label_set_text(s_rpSide, kAutons[i].side);
    lv_obj_set_style_bg_color(lv_obj_get_parent(s_rpSide),
                              sideColor(kAutons[i].side), 0);

    char buf[16];
    snprintf(buf, sizeof(buf), "ID %d", (int)kAutons[i].id);
    lv_label_set_text(s_rpID, buf);
    return;
  }
}

// ─────────────────────────────────────────────────────────────
//  SELECTOR — GRID HIGHLIGHT REFRESH
// ─────────────────────────────────────────────────────────────
static void grid_refresh() {
  for (int i = 0; i < kAutonCount; i++) {
    if (!s_btns[i])
      continue;
    bool sel = (kAutons[i].id == (AutonomousID)selectedAuton);

    lv_obj_set_style_bg_color(s_btns[i], sel ? CLR_SURFACE2 : CLR_RED_DEEP, 0);
    lv_obj_set_style_border_color(s_btns[i], sel ? CLR_RED_HOT : CLR_RED_DIM,
                                  0);
    lv_obj_set_style_border_width(s_btns[i], sel ? 2 : 1, 0);

    // Name label is always child 0, update its color
    lv_obj_t *lbl = lv_obj_get_child(s_btns[i], 0);
    if (lbl)
      lv_obj_set_style_text_color(lbl, sel ? CLR_TEXT_PRI : CLR_TEXT_SEC, 0);
  }
}

// ─────────────────────────────────────────────────────────────
//  SELECTOR — BUTTON CLICK CALLBACK
// ─────────────────────────────────────────────────────────────
static void btn_cb(lv_event_t *e) {
  selectedAuton = (int)(intptr_t)lv_event_get_user_data(e);
  grid_refresh();
  rp_refresh();
}

// Test-run callback
static void run_cb(lv_event_t *) {
  static bool running = false;
  if (running)
    return;
  running = true;
  new pros::Task([](void *) { autonomous(); }, nullptr,
                 TASK_PRIORITY_DEFAULT + 1, 0x8000, "test_auton");
}

// ─────────────────────────────────────────────────────────────
//  SELECTOR — BUILD ONE AUTON BUTTON
// ─────────────────────────────────────────────────────────────
//  Grid layout: 3 columns × 3 rows (7 autons + empty slot)
//  Each cell: (SW-rightPanelW-margins) / 3  wide, 36px tall
// ─────────────────────────────────────────────────────────────
static const int kGridX = 6;
static const int kGridY = TH + 4;
static const int kRPW = 160;                       // right panel width
static const int kGridW = SW - kGridX - kRPW - 12; // ~294
static const int kCols = 3;
static const int kCellW = kGridW / kCols; // ~98
static const int kCellH = 36;
static const int kCellG = 4;

static lv_obj_t *mk_auton_btn(lv_obj_t *par, int idx) {
  const AutonEntry &e = kAutons[idx];
  bool sel = (e.id == (AutonomousID)selectedAuton);

  int col = idx % kCols;
  int row = idx / kCols;
  int bx = kGridX + col * (kCellW + kCellG);
  int by = kGridY + row * (kCellH + kCellG);

  lv_obj_t *btn = lv_obj_create(par);
  lv_obj_set_size(btn, kCellW, kCellH);
  lv_obj_set_pos(btn, bx, by);
  lv_obj_set_style_bg_color(btn, sel ? CLR_SURFACE2 : CLR_RED_DEEP, 0);
  lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(btn, sel ? CLR_RED_HOT : CLR_RED_DIM, 0);
  lv_obj_set_style_border_width(btn, sel ? 2 : 1, 0);
  lv_obj_set_style_radius(btn, 5, 0);
  lv_obj_set_style_pad_all(btn, 0, 0);
  lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);

  // Left accent stripe when selected
  if (sel)
    mk_rect(btn, 0, 0, 3, kCellH, CLR_RED_HOT);

  // Name label
  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, e.name);
  lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(lbl, sel ? CLR_TEXT_PRI : CLR_TEXT_SEC, 0);
  lv_obj_set_pos(lbl, 8, kCellH / 2 - 7);

  // Side badge (top-right corner)
  lv_color_t sc = sideColor(e.side);
  lv_obj_t *badge = mk_rect(btn, kCellW - 36, 3, 32, 12, sc, 6);
  lv_obj_t *bLbl = lv_label_create(badge);
  lv_label_set_text(bLbl, e.side);
  lv_obj_set_style_text_font(bLbl, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(bLbl, CLR_WHITE, 0);
  lv_obj_align(bLbl, LV_ALIGN_CENTER, 0, 0);

  lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, (void *)(intptr_t)e.id);

  s_btns[idx] = btn;
  return btn;
}

// ─────────────────────────────────────────────────────────────
//  SELECTOR — BUILD SCREEN
// ─────────────────────────────────────────────────────────────
static lv_obj_t *build_selector() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, CLR_BG, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(scr, 0, 0);
  lv_obj_set_style_border_width(scr, 0, 0);
  lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  // ── Title bar ────────────────────────────────────────────
  mk_rect(scr, 0, 0, SW, TH, C(16, 6, 6));
  mk_rect(scr, 0, TH - 1, SW, 1, CLR_RED_HOT);

  // Z badge
  lv_obj_t *zBadge = mk_rect(scr, 5, 4, 16, 16, CLR_RED_HOT, 3);
  lv_obj_t *zLbl = lv_label_create(zBadge);
  lv_label_set_text(zLbl, "Z");
  lv_obj_set_style_text_font(zLbl, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(zLbl, CLR_WHITE, 0);
  lv_obj_align(zLbl, LV_ALIGN_CENTER, 0, 0);

  mk_label(scr, "ZIPTIDE  5069G  |  SELECT AUTON", 26, 6, CLR_RED_MID,
           &lv_font_montserrat_10);

  // ── Auton grid ───────────────────────────────────────────
  for (int i = 0; i < kAutonCount; i++)
    mk_auton_btn(scr, i);

  // ── Vertical divider ─────────────────────────────────────
  int divX = kGridX + kGridW + 6;
  mk_rect(scr, divX, TH + 2, 1, SH - TH - 4, CLR_RED_DIM);

  // ── Right panel ──────────────────────────────────────────
  int rpX = divX + 5;
  int rpY = TH + 4;
  int rpH = SH - TH - 6;

  // Section header
  mk_label(scr, "SELECTED", rpX, rpY, CLR_RED_MID, &lv_font_montserrat_10);
  mk_hline(scr, rpX, rpY + 13, kRPW - 8);

  // Name
  s_rpName =
      mk_label(scr, "", rpX, rpY + 17, CLR_TEXT_PRI, &lv_font_montserrat_14);

  // Desc
  s_rpDesc =
      mk_label(scr, "", rpX, rpY + 35, CLR_TEXT_SEC, &lv_font_montserrat_10);
  lv_label_set_long_mode(s_rpDesc, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(s_rpDesc, kRPW - 8);

  // Side pill container
  lv_obj_t *pillBox = mk_rect(scr, rpX, rpY + 62, 50, 16, sideColor("SAFE"), 8);
  s_rpSide = lv_label_create(pillBox);
  lv_label_set_text(s_rpSide, "SAFE");
  lv_obj_set_style_text_font(s_rpSide, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(s_rpSide, CLR_WHITE, 0);
  lv_obj_align(s_rpSide, LV_ALIGN_CENTER, 0, 0);

  // ID
  s_rpID = mk_label(scr, "", rpX + 56, rpY + 65, CLR_TEXT_DIM,
                    &lv_font_montserrat_10);

  // Divider
  mk_hline(scr, rpX, rpY + 82, kRPW - 8);

  // Team info
  mk_label(scr, "5069G", rpX, rpY + 88, CLR_RED_MID, &lv_font_montserrat_12);
  mk_label(scr, "ZIPTIDE  |  OVERRIDE 26-27", rpX, rpY + 104, CLR_TEXT_DIM,
           &lv_font_montserrat_10);

  // ── Run test button ──────────────────────────────────────
  int btnY = rpY + rpH - 28;
  lv_obj_t *runBtn = lv_obj_create(scr);
  lv_obj_set_size(runBtn, kRPW - 8, 24);
  lv_obj_set_pos(runBtn, rpX, btnY);
  lv_obj_set_style_bg_color(runBtn, C(10, 40, 20), 0);
  lv_obj_set_style_bg_color(runBtn, C(6, 25, 12), LV_STATE_PRESSED);
  lv_obj_set_style_bg_opa(runBtn, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(runBtn, C(22, 90, 44), 0);
  lv_obj_set_style_border_width(runBtn, 1, 0);
  lv_obj_set_style_radius(runBtn, 4, 0);
  lv_obj_set_style_pad_all(runBtn, 0, 0);
  lv_obj_set_scrollbar_mode(runBtn, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(runBtn, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(runBtn, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *runLbl = lv_label_create(runBtn);
  lv_label_set_text(runLbl, LV_SYMBOL_PLAY "  TEST RUN");
  lv_obj_set_style_text_font(runLbl, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(runLbl, CLR_GREEN, 0);
  lv_obj_align(runLbl, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(runBtn, run_cb, LV_EVENT_CLICKED, nullptr);

  rp_refresh();
  return scr;
}

// ─────────────────────────────────────────────────────────────
//  DEBUG SCREEN — BUILD
// ─────────────────────────────────────────────────────────────
static lv_obj_t *build_debug() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, CLR_BG, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(scr, 0, 0);
  lv_obj_set_style_border_width(scr, 0, 0);
  lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  // ── Title bar ────────────────────────────────────────────
  mk_rect(scr, 0, 0, SW, TH, C(16, 6, 6));
  mk_rect(scr, 0, TH - 1, SW, 1, CLR_RED_HOT);

  lv_obj_t *dTitle = lv_label_create(scr);
  lv_label_set_text(dTitle, "ZIPTIDE DEBUG  |  5069G  |  OVERRIDE 26-27");
  lv_obj_set_style_text_font(dTitle, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(dTitle, CLR_RED_MID, 0);
  lv_obj_align(dTitle, LV_ALIGN_TOP_MID, 0, 6);

  // ── Layout constants ─────────────────────────────────────
  const int C1 = 8;
  const int C2 = SW / 2 + 6;
  const int Y0 = TH + 5;
  const int LH = 19;
  const int LBW = 48; // label column width

  // Vertical divider
  mk_rect(scr, SW / 2 - 1, TH + 2, 1, SH - TH - 4, CLR_RED_DIM);

  // ── LEFT: ODOMETRY ───────────────────────────────────────
  mk_label(scr, "ODOMETRY", C1, Y0, CLR_RED_MID, &lv_font_montserrat_10);
  mk_hline(scr, C1, Y0 + 12, SW / 2 - 14);

  mk_label(scr, "X", C1, Y0 + 15, CLR_TEXT_SEC, &lv_font_montserrat_10);
  s_lX = mk_label(scr, "---", C1 + LBW, Y0 + 13, CLR_TEXT_PRI,
                  &lv_font_montserrat_14);

  mk_label(scr, "Y", C1, Y0 + 15 + LH, CLR_TEXT_SEC, &lv_font_montserrat_10);
  s_lY = mk_label(scr, "---", C1 + LBW, Y0 + 13 + LH, CLR_TEXT_PRI,
                  &lv_font_montserrat_14);

  mk_label(scr, "HDG", C1, Y0 + 15 + LH * 2, CLR_TEXT_SEC,
           &lv_font_montserrat_10);
  s_lH = mk_label(scr, "---", C1 + LBW, Y0 + 13 + LH * 2, CLR_TEXT_PRI,
                  &lv_font_montserrat_14);

  // ── LEFT: AUTON ──────────────────────────────────────────
  const int AY = Y0 + 15 + LH * 3 + 4;
  mk_label(scr, "AUTON", C1, AY, CLR_RED_MID, &lv_font_montserrat_10);
  mk_hline(scr, C1, AY + 12, SW / 2 - 14);

  lv_obj_t *aBox = mk_rect(scr, C1, AY + 15, SW / 2 - 18, 20, CLR_RED_DEEP, 4);
  s_lAuton = mk_label(aBox, "---", 6, 3, CLR_TEXT_PRI, &lv_font_montserrat_12);

  // ── LEFT: CONNECTION ─────────────────────────────────────
  const int CY = AY + 40;
  mk_label(scr, "CONN", C1, CY, CLR_TEXT_SEC, &lv_font_montserrat_10);
  s_lConn = mk_label(scr, "OFFLINE", C1 + LBW, CY - 2, CLR_YELLOW,
                     &lv_font_montserrat_12);

  // ── RIGHT: BATTERY ───────────────────────────────────────
  mk_label(scr, "BATTERY", C2, Y0, CLR_RED_MID, &lv_font_montserrat_10);
  mk_hline(scr, C2, Y0 + 12, SW / 2 - 14);

  mk_label(scr, "CAP", C2, Y0 + 15, CLR_TEXT_SEC, &lv_font_montserrat_10);
  s_lBat = mk_label(scr, "---%", C2 + LBW, Y0 + 13, CLR_GREEN,
                    &lv_font_montserrat_14);

  // Battery bar track
  lv_obj_t *batTrack =
      mk_rect(scr, C2, Y0 + 32, SW / 2 - 18, 5, C(30, 10, 10), 3);
  s_lBatBar = mk_rect(batTrack, 0, 0, 80, 5, CLR_GREEN, 3);

  // ── RIGHT: MOTOR TEMPS ───────────────────────────────────
  const int TY = Y0 + 44;
  mk_label(scr, "TEMPS", C2, TY, CLR_RED_MID, &lv_font_montserrat_10);
  mk_hline(scr, C2, TY + 12, SW / 2 - 14);

  mk_label(scr, "DRIVE", C2, TY + 15, CLR_TEXT_SEC, &lv_font_montserrat_10);
  s_lTDrive = mk_label(scr, "--°C", C2 + LBW, TY + 13, CLR_GREEN,
                       &lv_font_montserrat_14);

  mk_label(scr, "CLAW", C2, TY + 15 + LH, CLR_TEXT_SEC, &lv_font_montserrat_10);
  s_lTClaw = mk_label(scr, "--°C", C2 + LBW, TY + 13 + LH, CLR_GREEN,
                      &lv_font_montserrat_14);

  mk_label(scr, "4-BAR", C2, TY + 15 + LH * 2, CLR_TEXT_SEC,
           &lv_font_montserrat_10);
  s_lTFour = mk_label(scr, "--°C", C2 + LBW, TY + 13 + LH * 2, CLR_GREEN,
                      &lv_font_montserrat_14);

  mk_label(scr, "LIFT", C2, TY + 15 + LH * 3, CLR_TEXT_SEC,
           &lv_font_montserrat_10);
  s_lTLift = mk_label(scr, "--°C", C2 + LBW, TY + 13 + LH * 3, CLR_GREEN,
                      &lv_font_montserrat_14);

  return scr;
}

// ─────────────────────────────────────────────────────────────
//  DEBUG TASK — LIVE UPDATES
// ─────────────────────────────────────────────────────────────
static lv_color_t tempClr(double t) {
  if (t < 45.0)
    return CLR_GREEN;
  if (t < 58.0)
    return CLR_YELLOW;
  return CLR_RED_ALERT;
}

static lv_color_t batClr(int pct) {
  if (pct > 60)
    return CLR_GREEN;
  if (pct > 30)
    return CLR_YELLOW;
  return CLR_RED_ALERT;
}

void GUI_debugTask(void *) {
  char buf[32];
  while (true) {
    // Odometry
    auto pose = chassis.getPose();
    snprintf(buf, sizeof(buf), "%.1f", pose.x);
    lv_label_set_text(s_lX, buf);
    snprintf(buf, sizeof(buf), "%.1f", pose.y);
    lv_label_set_text(s_lY, buf);
    snprintf(buf, sizeof(buf), "%.1f°", pose.theta);
    lv_label_set_text(s_lH, buf);

    // Auton name
    for (int i = 0; i < kAutonCount; i++) {
      if (kAutons[i].id == (AutonomousID)selectedAuton) {
        lv_label_set_text(s_lAuton, kAutons[i].name);
        break;
      }
    }

    // Connection
    bool conn = pros::competition::is_connected();
    lv_label_set_text(s_lConn, conn ? "FIELD" : "CTRLR");
    lv_obj_set_style_text_color(s_lConn, conn ? CLR_GREEN : CLR_YELLOW, 0);

    // Battery
    int bat = (int)pros::battery::get_capacity();
    auto bc = batClr(bat);
    snprintf(buf, sizeof(buf), "%d%%", bat);
    lv_label_set_text(s_lBat, buf);
    lv_obj_set_style_text_color(s_lBat, bc, 0);
    // Bar fill — track is SW/2-18 wide, ~222px max
    int bw = (bat * 222) / 100;
    if (bw < 2)
      bw = 2;
    if (bw > 222)
      bw = 222;
    lv_obj_set_size(s_lBatBar, bw, 5);
    lv_obj_set_style_bg_color(s_lBatBar, bc, 0);

    // Motor temps
    auto setTemp = [](lv_obj_t *lbl, double t) {
      char b[16];
      snprintf(b, sizeof(b), "%.0f°C", t);
      lv_label_set_text(lbl, b);
      lv_obj_set_style_text_color(lbl, tempClr(t), 0);
    };

    double tDrive = (DriveL.get_temperature() + DriveR.get_temperature()) / 2.0;
    setTemp(s_lTDrive, tDrive);
    setTemp(s_lTClaw, Claw.get_temperature());
    setTemp(s_lTFour, FourBar.get_temperature());
    setTemp(s_lTLift, Lift.get_temperature());

    pros::delay(250);
  }
}

// ─────────────────────────────────────────────────────────────
//  PUBLIC API
// ─────────────────────────────────────────────────────────────
void GUI_runAutonSelector() {
  // Reset button refs
  for (int i = 0; i < kAutonCount; i++)
    s_btns[i] = nullptr;
  s_rpName = s_rpDesc = s_rpSide = s_rpID = nullptr;

  if (s_selectorScr) {
    lv_obj_delete(s_selectorScr);
    s_selectorScr = nullptr;
  }

  s_selectorScr = build_selector();
  lv_screen_load(s_selectorScr);
}

void GUI_initDebugTask() {
  if (!s_debugScr)
    s_debugScr = build_debug();
  lv_screen_load(s_debugScr);

  if (!s_debugTask) {
    s_debugTask = new pros::Task(GUI_debugTask);
    s_debugRunning = true;
  }
}

void GUI_showDebugScreen() { GUI_initDebugTask(); }