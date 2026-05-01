#include "Autons.h"
#include "robot-config.h"
#include "main.h"

// ============================================================
//  Autons.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

void autonLeft() {
    // TODO: write left side auton
    chassis.setPose(0, 0, 0);
    chassis.moveToPoint(0, 24, 3000);
}

void autonRight() {
    // TODO: write right side auton
    chassis.setPose(0, 0, 0);
    chassis.moveToPoint(0, 24, 3000);
}

void autonSkills() {
    // TODO: write skills auton
    chassis.setPose(0, 0, 0);
}