// ============================================================
//  GUI.h  —  Auton Selector + Debug | ZIPTIDE 5069G
//  V5RC Override 2026-27  |  LVGL 9.2  |  PROS 4
// ============================================================
#pragma once
#include "api.h"

enum AutonomousID {
    AUTON_NONE  = 0,
    AUTON_LEFT  = 1,
    AUTON_RIGHT = 2,
    AUTON_SKILLS  = 3,
};

extern volatile int selectedAuton;

// ── Public API ───────────────────────────────────────────────
void GUI_runAutonSelector();   // call in initialize()
void GUI_showDebugScreen();    // call in opcontrol() or disabled()
void GUI_debugTask(void* param);
void GUI_initDebugTask();