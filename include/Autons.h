#pragma once

// ============================================================
//  Autons.h — AVRO | Override 2026-2027
// ============================================================
//
//  Add a new autonomous routine:
//  1. Write the routine function below.
//  2. Add one entry to the AUTONS[] array in Autons.cpp.
//
//  The controller auton selector and autonomous() both use
//  the AUTONS[] array, so no additional registration is needed.
// ============================================================

struct AutonEntry {

    const char* name;

    void (*run)();
};

void autonNone();

void autonLeft();

void autonRight();

void autonSkills();

extern const AutonEntry AUTONS[];

extern const int AUTON_COUNT;