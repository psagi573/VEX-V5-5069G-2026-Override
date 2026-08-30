#include "robot-config.h"

#include "lemlib/chassis/trackingWheel.hpp"

#include "pros/abstract_motor.hpp"
#include "pros/adi.h"
#include "pros/adi.hpp"
#include "pros/motors.hpp"

// ============================================================
//  robot-config.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

// ============================================================
// Controllers
// ============================================================

pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Controller partner(pros::E_CONTROLLER_PARTNER);

// ============================================================
// Drivetrain Motors
// ============================================================
// Negative port numbers reverse the motor direction.
//
// Left drivetrain:
//   L1 — Port 21, reversed — blue cartridge — left front
//   L2 — Port 6,  reversed — blue cartridge — left back
//   L3 — Port 1,  reversed — green cartridge — left middle
//
// Right drivetrain:
//   R1 — Port 9  — blue cartridge — right front
//   R2 — Port 10 — blue cartridge — right back
//   R3 — Port 8  — green cartridge — right middle
//
// The green-cartridge motors are used with the drivetrain's
// mechanical gearing to achieve the desired wheel speed.

pros::Motor L1(-21, pros::MotorGears::blue);   // Left front
pros::Motor L2(-6,  pros::MotorGears::blue);   // Left back
pros::Motor L3(-1,  pros::MotorGears::green);  // Left middle

pros::Motor R1(9, pros::MotorGears::blue);     // Right front
pros::Motor R2(10, pros::MotorGears::blue);    // Right back
pros::Motor R3(8, pros::MotorGears::green);    // Right middle

// ============================================================
// Mechanism Motors
// ============================================================

// Cascade motors
pros::Motor cascade1(-20, pros::MotorGears::blue);
pros::Motor cascade2(19, pros::MotorGears::blue);

pros::MotorGroup cascade({-20, 19});

// Intake motor
pros::Motor intake(18, pros::MotorGears::blue);

// Legacy / unused four-bar motors
//pros::Motor FourBar1(-8, pros::MotorGears::green);
//pros::Motor FourBar2(19, pros::MotorGears::green);

// Legacy intake declaration
//pros::Motor intake(-2, pros::MotorGears::blue);

// ============================================================
// Drivetrain Motor Groups
// ============================================================

pros::MotorGroup DriveL({-21, -1, -6});
pros::MotorGroup DriveR({9, 10, 8});

// Legacy / unused four-bar motor group
//pros::MotorGroup FourBar({19, -8});

// ============================================================
// Pneumatics
// ============================================================

pros::adi::Pneumatics claw('H', true);

// ============================================================
// Sensors
// ============================================================

// IMU port is currently a placeholder and will be assigned later.
pros::Imu imu(10);

// Tracking wheel rotation sensors
pros::Rotation trackY(11);  // Vertical tracking wheel
pros::Rotation trackX(12);  // Horizontal tracking wheel

// ============================================================
// LemLib Drivetrain Configuration
// ============================================================

lemlib::Drivetrain drivetrain(
    &DriveL,
    &DriveR,

    12.5,  // Track width in inches [TUNE after robot is built]

    lemlib::Omniwheel::NEW_275,  // 2.75" wheels

    // Configured drivetrain wheel speed.
    //
    // The drivetrain motors are configured around a 600 RPM
    // motor output, with the external gearing reducing the
    // resulting wheel speed.
    450,

    8  // Horizontal drift [TUNE]
);

// ============================================================
// LemLib Tracking Wheels
// ============================================================

// Tracking wheel diameter and offset are temporary values
// and should be tuned after the robot is physically built.

lemlib::TrackingWheel vertWheel(
    &trackY,
    2.0,  // Diameter in inches [TUNE]
    0.0   // Offset in inches [TUNE]
);

lemlib::TrackingWheel horizWheel(
    &trackX,
    2.0,  // Diameter in inches [TUNE]
    0.0   // Offset in inches [TUNE]
);

// ============================================================
// LemLib Odometry Sensors
// ============================================================

lemlib::OdomSensors sensors(
    &vertWheel,
    nullptr,
    &horizWheel,
    nullptr,
    &imu
);

// ============================================================
// LemLib Lateral PID
// ============================================================
// Values are initial tuning values and should be tuned on the
// completed robot.

lemlib::ControllerSettings lateralPID(
    10,   // kP
    0,    // kI
    3,    // kD
    3,    // anti-windup
    1,    // small error range (in)
    100,  // small error timeout (ms)
    3,    // large error range (in)
    500,  // large error timeout (ms)
    20    // maximum acceleration slew
);

// ============================================================
// LemLib Angular PID
// ============================================================
// Values are initial tuning values and should be tuned on the
// completed robot.

lemlib::ControllerSettings angularPID(
    2,    // kP
    0,    // kI
    10,   // kD
    3,    // anti-windup
    1,    // small error range (deg)
    100,  // small error timeout (ms)
    3,    // large error range (deg)
    500,  // large error timeout (ms)
    0     // maximum acceleration slew
);

// ============================================================
// LemLib Driver-Control Expo Curves
// ============================================================

lemlib::ExpoDriveCurve throttle_curve(
    3,     // Joystick deadband
    10,    // Minimum output
    1.01   // Exponential curve strength
);

lemlib::ExpoDriveCurve steer_curve(
    3,     // Joystick deadband
    10,    // Minimum output
    1.01   // Exponential curve strength
);

// ============================================================
// LemLib Chassis
// ============================================================

lemlib::Chassis chassis(
    drivetrain,
    lateralPID,
    angularPID,
    sensors,
    &throttle_curve,
    &steer_curve
);
