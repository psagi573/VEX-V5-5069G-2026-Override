#include "drivetrain.h"
#include "robot-config.h"
#include <cmath>
// ============================================================
//  drivetrain.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================


// --------- CURVATURE DRIVE WITH EXPONENTIAL SCALING ---------
// Left stick  = forward/back
// Right stick = curvature (turn radius scales with speed)
// Cubic scaling for precision at low stick values
void DriveTrainControls() {
    while (true) {
        int throttle = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        int turn     = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

        chassis.curvature(throttle, turn);

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