#include "drivetrain.h"
#include "pros/misc.h"
#include "robot-config.h"
#include <cmath>
#include <algorithm>
// ============================================================
//  drivetrain.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

static constexpr double JOYSTICK_DEADBAND = 4.0;
static constexpr double EXPO_CUTOFF = 19.0; // ~15% of 127

// Manual linear pass-through used above EXPO_CUTOFF.
// Below EXPO_CUTOFF, chassis.arcade() drives the expo curves
// defined in robot-config.cpp instead of this function.
double linearAfter15(double input) {
    if (fabs(input) < JOYSTICK_DEADBAND) return 0;
    return input;
}

float tovolt(float percentage) {
    return (percentage * 12000.0 / 100.0);
}

void DriveTrainControls() {
    while (true) {
        double forward = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        double turn = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

        if (fabs(forward) < EXPO_CUTOFF && fabs(turn) < EXPO_CUTOFF) {
            // Below cutoff: LemLib arcade with the expo curves for fine control.
            chassis.arcade(forward, turn);
        } else {
            // Above cutoff: direct linear response for full-speed driving.
            forward = linearAfter15(forward);
            turn = linearAfter15(turn);

            double leftPower = std::clamp(forward + turn, -127.0, 127.0);
            double rightPower = std::clamp(forward - turn, -127.0, 127.0);

            DriveL.move_voltage(tovolt(leftPower / 127.0 * 100.0));
            DriveR.move_voltage(tovolt(rightPower / 127.0 * 100.0));
        }

        pros::delay(10);
    }
}

// --------- CLAW (pneumatic) ---------
// Y = toggle extend/retract
// void ClawControls() {
//     static bool clawExtended = false;

//     while (true) {
//         if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)) {
//             clawExtended = !clawExtended;
//             if (clawExtended) claw.extend();
//             else claw.retract();
//         }
//         pros::delay(10);
//     }
// }

// // --------- 4-BAR ---------
// // L1 = up | L2 = down
// void FourBarControls() {
//     while (true) {
//         if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
//             FourBar.move(127);
//         } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
//             FourBar.move(-127);
//         } else {
//             FourBar.brake();
//         }
//         pros::delay(10);
//     }
// }

// // --------- INTAKE ---------
// // R1 = in | R2 = out
// void IntakeControls() {
//     while (true) {
//         if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
//             intake.move(127);
//         } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
//             intake.move(-127);
//         } else {
//             intake.brake();
//         }
//         pros::delay(10);
//     }
// }