#include "main.h"
#include "Autons.h"
#include "drivetrain.h"
#include "robot-config.h"
#include "GUI.h"

// ============================================================
//  main.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

void initialize() {
    chassis.calibrate(true); // ~3s IMU calibration

    DriveL.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
    DriveR.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
    cascade.set_brake_mode_all(pros::E_MOTOR_BRAKE_HOLD);
    intake.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);

    

    chassis.setPose(0, 0, 0);
    //GUI_runAutonSelector();
}

void disabled() {
    //GUI_showDebugScreen();
}

void competition_initialize() {
    // Selector screen stays up from initialize() until the match starts.
}

void autonomous() {
    // if (selectedAuton >= 0 && selectedAuton < AUTON_COUNT) {
    //     AUTONS[selectedAuton].run();
    // }
}

void opcontrol() {
    //GUI_showDebugScreen();

    new pros::Task(DriveTrainControls);
    new pros::Task(CascadeControls);
    new pros::Task(IntakeControls);

    while (true) {
        pros::delay(20);
    }
}