#include "drivetrain.h"
#include "robot-config.h"
#include "main.h"
#include <cmath>
#include <algorithm>

// ============================================================
//  drivetrain.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

static double applyDeadband(double value, double deadband) {
    if (fabs(value) < deadband) return 0.0;
    return value;
}

// --------- CURVATURE DRIVE WITH EXPONENTIAL SCALING ---------
// Left stick  = forward/back
// Right stick = curvature (turn radius scales with speed)
// Cubic scaling for precision at low stick values
void DriveTrainControls() {
    while (true) {
        double raw_throttle = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y) / 127.0;
        double raw_turn     = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X) / 127.0;

        // Deadband
        raw_throttle = applyDeadband(raw_throttle, 0.05);
        raw_turn     = applyDeadband(raw_turn, 0.05);

        // Cubic exponential scaling
        double throttle = raw_throttle * raw_throttle * raw_throttle;
        double turn     = raw_turn * raw_turn * raw_turn;

        double leftVolt, rightVolt;

        if (fabs(throttle) < 0.01) {
            // Point turn when nearly stopped
            leftVolt  =  turn * 12000.0;
            rightVolt = -turn * 12000.0;
        } else {
            // Curvature: turn scales with speed
            leftVolt  = (throttle + fabs(throttle) * turn) * 12000.0;
            rightVolt = (throttle - fabs(throttle) * turn) * 12000.0;

            // Normalize so neither side exceeds max voltage
            double maxVolt = std::max(fabs(leftVolt), fabs(rightVolt));
            if (maxVolt > 12000.0) {
                leftVolt  *= 12000.0 / maxVolt;
                rightVolt *= 12000.0 / maxVolt;
            }
        }

        DriveL.move_voltage(leftVolt);
        DriveR.move_voltage(rightVolt);

        pros::delay(10);
    }
}

// --------- CLAW ---------
// R1 = forward | R2 = reverse
void ClawControls() {
    while (true) {
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            Claw.move(127);
        } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            Claw.move(-127);
        } else {
            Claw.brake();
        }
        pros::delay(10);
    }
}

// --------- 4-BAR ---------
// L1 = up | L2 = down
void FourBarControls() {
    while (true) {
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
            FourBar.move(127);
        } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
            FourBar.move(-127);
        } else {
            FourBar.brake();
        }
        pros::delay(10);
    }
}

// --------- LIFT ---------
// Up arrow = up | Down arrow = down
void LiftControls() {
    while (true) {
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_UP)) {
            Lift.move(127);
        } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN)) {
            Lift.move(-127);
        } else {
            Lift.brake();
        }
        pros::delay(10);
    }
}