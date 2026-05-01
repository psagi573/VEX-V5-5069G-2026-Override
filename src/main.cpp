#include "main.h"
#include "Autons.h"
#include "drivetrain.h"
#include "robot-config.h"

// ============================================================
//  main.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

void initialize() {
  chassis.calibrate(true); // ~3s IMU calibration

  // Brake modes
  DriveL.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
  DriveR.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
  Claw.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  FourBar.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  Lift.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);

  // MVLib telemetry
  auto &logger = mvlib::Logger::getInstance();
  mvlib::setOdom(&chassis);
  logger.setRobot({.leftDrivetrain = &DriveL, .rightDrivetrain = &DriveR});
  logger.start();

  chassis.setPose(0, 0, 0);
}

void disabled() {}

void competition_initialize() {}

void autonomous() {
  // Uncomment the auton you want to run
  // autonLeft();
  // autonRight();
  // autonSkills();
}

void opcontrol() {
  new pros::Task(DriveTrainControls);
  new pros::Task(ClawControls);
  new pros::Task(FourBarControls);
  new pros::Task(LiftControls);

  while (true) {
    pros::delay(20);
  }
}