#include "robot-config.h"

// ============================================================
//  robot-config.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

// Controllers
pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Controller partner(pros::E_CONTROLLER_PARTNER);

// Drivetrain Motors
// Negative port = reversed. Adjust when robot is built.
pros::Motor dt_L1(-1, pros::MotorGears::blue);  // [PORT] Left front  11W
pros::Motor dt_L2(-2, pros::MotorGears::blue);  // [PORT] Left back   11W
pros::Motor dt_L3(-3, pros::MotorGears::blue);  // [PORT] Left middle 5.5W
pros::Motor dt_R1(4,  pros::MotorGears::blue);  // [PORT] Right front 11W
pros::Motor dt_R2(5,  pros::MotorGears::blue);  // [PORT] Right back  11W
pros::Motor dt_R3(6,  pros::MotorGears::blue);  // [PORT] Right middle 5.5W

pros::MotorGroup DriveL({-1, -2, -3}, pros::MotorGears::blue); // [PORT]
pros::MotorGroup DriveR({4, 5, 6},    pros::MotorGears::blue); // [PORT]

// Mechanism Motors — update cartridge once decided
pros::Motor Claw(7,    pros::MotorGears::green); // [PORT]
pros::Motor FourBar(8, pros::MotorGears::green); // [PORT]
pros::Motor Lift(9,    pros::MotorGears::green); // [PORT]

// Sensors
pros::Imu imu(10);         // [PORT]
pros::Rotation trackY(11); // [PORT] vertical
pros::Rotation trackX(12); // [PORT] horizontal

// ============================================================
//  LemLib Config
// ============================================================

lemlib::Drivetrain drivetrain(
    &DriveL,
    &DriveR,
    12.5,                        // [TUNE] track width in inches
    lemlib::Omniwheel::NEW_4,    // 4 inch wheels
    343,                         // RPM after ratio: 600 * (48/84) ≈ 343
    2                            // horizontal drift
);

lemlib::TrackingWheel vertWheel(&trackY, 2.0, 0.0);  // [TUNE] diameter, offset
lemlib::TrackingWheel horizWheel(&trackX, 2.0, 0.0); // [TUNE] diameter, offset

lemlib::OdomSensors sensors(
    &vertWheel,
    nullptr,
    &horizWheel,
    nullptr,
    &imu
);

// [TUNE] after robot is built
lemlib::ControllerSettings lateralPID(
    10,  // kP
    0,   // kI
    3,   // kD
    3,   // anti-windup
    1,   // small error range (in)
    100, // small error timeout (ms)
    3,   // large error range (in)
    500, // large error timeout (ms)
    20   // max acceleration slew
);

lemlib::ControllerSettings angularPID(
    2,   // kP
    0,   // kI
    10,  // kD
    3,   // anti-windup
    1,   // small error range (deg)
    100, // small error timeout (ms)
    3,   // large error range (deg)
    500, // large error timeout (ms)
    0    // max acceleration slew
);

lemlib::Chassis chassis(drivetrain, lateralPID, angularPID, sensors);