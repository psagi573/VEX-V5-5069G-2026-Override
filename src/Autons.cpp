#include "Autons.h"
#include "robot-config.h"
#include "main.h"

// ============================================================
//  Autons.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

void autonNone() {
    // Safe fallback. No movement.
}

void autonLeft() {
    chassis.setPose(0, 0, 0);
    // TODO: build the left-side routine once the robot exists
    // and field coordinates are measured.
}

void autonRight() {
    chassis.setPose(0, 0, 0);
    // TODO: build the right-side routine.
}

void autonSkills() {
    chassis.setPose(0, 0, 0);
    // TODO: build the skills routine.
}

// Edit this array to add, remove, or reorder autons.
// Both the selector GUI and autonomous() read directly from it.
const AutonEntry AUTONS[] = {
    {"Do Nothing", autonNone},
    {"Left",       autonLeft},
    {"Right",      autonRight},
    {"Skills",     autonSkills},
};

const int AUTON_COUNT = sizeof(AUTONS) / sizeof(AUTONS[0]);