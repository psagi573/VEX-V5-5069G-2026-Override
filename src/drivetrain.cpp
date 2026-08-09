#include "drivetrain.h"
#include "pros/misc.h"
#include "robot-config.h"
#include <cmath>
// ============================================================
//  drivetrain.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================


// --------- Arcade DRIVE WITH EXPONENTIAL SCALING ---------
// Cubic scaling for precision at low stick values
double linearAfter15(double input) {
  double deadband = 4.0;
  double cutoff = 19.0; // 15% of 127

  if (fabs(input) < deadband) return 0;

  // under 15 percent is handled by chassis.arcade
  // this function is only for manual linear after 15
  return input;
}

float tovolt(float percentage) {
  return (percentage * 12000.0 / 100.0);
}

// void DriveTrainControls() {
//     while (true) {
//         int forward = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y); // forward/back
//         int turn = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);  // left/right

//         float leftVolt = tovolt(forward + turn);
//         float rightVolt = tovolt(forward - turn);

//         DriveL.move(leftVolt);
//         DriveR.move(rightVolt);
//         }
//         pros::delay(10);
//     }


void DriveTrainControls() {
  while (true) {
    double forward = master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
    double turn = master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);

    double cutoff = 19.0; // about 15 percent joystick

    if (fabs(forward) < cutoff && fabs(turn) < cutoff) {
      // lemlib arcade with expo curves
      chassis.arcade(forward, turn);
    } 
    else {
      // manual linear arcade after 15 percent
      forward = linearAfter15(forward);
      turn = linearAfter15(turn);

      double leftPower = forward + turn;
      double rightPower = forward - turn;

      leftPower = std::clamp(leftPower, -127.0, 127.0);
      rightPower = std::clamp(rightPower, -127.0, 127.0);

      DriveL.move_voltage(tovolt(leftPower / 127.0 * 100.0));
      DriveR.move_voltage(tovolt(rightPower / 127.0 * 100.0));
    }

    pros::delay(10);
  }
}
//--------- CLAW ---------
//R1 = forward | R2 = reverse

void ClawControls() {
    static bool wing = false;

    while (true) {
        if (master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)) {
            wing = !wing;
            if (wing)
                claw.extend();
            else
                claw.retract();
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
void IntakeControls() {
    while (true) {
        if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            intake.move(127);
        } else if (master.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            intake.move(-127);
        } else {
            intake.brake();
        }
        pros::delay(10);
    }
} 