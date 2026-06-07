#include "robot-config.h"
#include "lemlib/chassis/trackingWheel.hpp"
#include "pros/adi.h"
#include "pros/adi.hpp"
#include "pros/motors.hpp"

// ============================================================
//  robot-config.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

// Controllers
pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Controller partner(pros::E_CONTROLLER_PARTNER);

// Drivetrain Motors
// Negative port = reversed. Adjust when robot is built.
pros::Motor L1(-21, pros::MotorGears::blue); // [PORT] Left front  11W
pros::Motor L2(17, pros::MotorGears::blue); // [PORT] Left back   11W
pros::Motor L3(18, pros::MotorGears::green); // [PORT] Left middle 5.5W
pros::Motor R1(-6, pros::MotorGears::blue);  // [PORT] Right front 11W
pros::Motor R2(16, pros::MotorGears::blue);  // [PORT] Right back  11W
pros::Motor R3(-15, pros::MotorGears::green);  // [PORT] Right middle 5.5W
pros::Motor FourBar1(-8, pros::MotorGears::green); // [PORT]
pros::Motor FourBar2(19, pros::MotorGears::green); // [PORT]
pros::Motor intake (-2, pros::MotorGear::blue);

pros::MotorGroup DriveL({-21, 17, 18}); // [PORT]
pros::MotorGroup DriveR({-6, 16, -15});    // [PORT]
pros::MotorGroup FourBar({19, -8});    // [PORT]


pros::adi::Pneumatics claw('H', true);

// Mechanism Motors — update cartridge once decided
// pros::Motor Claw(7, pros::MotorGears::green);    // [PORT]
// pros::Motor Lift(9, pros::MotorGears::green);    // [PORT]

// Sensors
pros::Imu imu(10);         // [PORT]
pros::Rotation trackY(11); // [PORT] vertical
pros::Rotation trackX(12); // [PORT] horizontal

// ============================================================
//  LemLib Config
// ============================================================

lemlib::Drivetrain drivetrain(&DriveL, &DriveR,
                              12.5, //  track width in inches
                              lemlib::Omniwheel::NEW_275, // 4 inch wheels
                              450, // RPM after ratio: 600 * (36/48) ≈ 450
                              2    // horizontal drift
);

lemlib::TrackingWheel vertWheel(&trackY, 2.0, 0.0);  // [TUNE] diameter, offset
lemlib::TrackingWheel horizWheel(&trackX, 2.0, 0.0); // [TUNE] diameter, offset

lemlib::OdomSensors sensors(&vertWheel, nullptr, &horizWheel, nullptr, &imu);

// [TUNE] after robot is built
lemlib::ControllerSettings lateralPID(10,  // kP
                                      0,   // kI
                                      3,   // kD
                                      3,   // anti-windup
                                      1,   // small error range (in)
                                      100, // small error timeout (ms)
                                      3,   // large error range (in)
                                      500, // large error timeout (ms)
                                      20   // max acceleration slew
);

lemlib::ControllerSettings angularPID(2,   // kP
                                      0,   // kI
                                      10,  // kD
                                      3,   // anti-windup
                                      1,   // small error range (deg)
                                      100, // small error timeout (ms)
                                      3,   // large error range (deg)
                                      500, // large error timeout (ms)
                                      0    // max acceleration slew
);

// Expo drive curves
lemlib::ExpoDriveCurve throttle_curve(3, 10, 1.01);
lemlib::ExpoDriveCurve steer_curve(3, 10, 1.01);

lemlib::Chassis chassis(drivetrain, lateralPID, angularPID, sensors,
                        &throttle_curve, &steer_curve);