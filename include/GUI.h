// ============================================================
//  GUI.h  —  Auton Selector + Debug Header for ZIPTIDE
//  Team 5069G  |  V5RC Override 2026-27
// ============================================================

#pragma once

#include "api.h"

/**
 * Enumeration for Autonomous routines.
 * Matches the IDs used in the AutonEntry list.
 */
enum AutonomousID {
    AUTON_NONE = 0,
    AUTON_SPLIT_LEFT,
    AUTON_SPLIT_RIGHT,
    AUTON_LEFT_WING,
    AUTON_RIGHT_WING,
    AUTON_SAWP,
    AUTON_SKILLS
};

// ─────────────────────────────────────────────
//  GLOBAL DATA
// ─────────────────────────────────────────────

/**
 * The currently selected autonomous ID. 
 * Marked volatile as it is accessed by the GUI task and the main competition thread.
 */
extern volatile int selectedAuton;

// ─────────────────────────────────────────────
//  PUBLIC API
// ─────────────────────────────────────────────

/**
 * Initializes the GUI and displays the Autonomous Selector screen.
 * This should typically be called in initialize() or disabled().
 */
void GUI_runAutonSelector();

/**
 * Initializes and starts the background debug task.
 * Switches the screen to the Debug dashboard.
 */
void GUI_showDebugScreen();

/**
 * The task loop for updating sensor values and motor temps on the debug screen.
 * @param param Unused task parameter.
 */
void GUI_debugTask(void* param);

/**
 * Helper to ensure the debug task is initialized and the screen loaded.
 */
void GUI_initDebugTask();

