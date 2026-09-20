#include "Autons.h"

#include "robot-config.h"
#include "main.h"

// ============================================================
//  Autons.cpp — AVRO | Override 2026-2027
// ============================================================

void autonNone() {

    // fallback
}

void autonLeft() {

    chassis.setPose(0, 0, 0);

    
}

void autonRight() {

    chassis.setPose(0, 0, 0);

   
}

void autonSkills() {

    chassis.setPose(0, 0, 0);

    
}

const AutonEntry AUTONS[] = {

    {"Nothing", autonNone},
    {"Left",       autonLeft},
    {"Right",      autonRight},
    {"Skills",     autonSkills},

};

const int AUTON_COUNT = sizeof(AUTONS) / sizeof(AUTONS[0]);