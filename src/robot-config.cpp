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

// Drivetrain Motors — negative port = reversed
pros::Motor L1(-10, pros::MotorGears::blue);  // left front
pros::Motor L2(-1, pros::MotorGears::blue);   // left back
pros::Motor L3(-6, pros::MotorGears::green);  // left middle, 3:1 stepped to 600 rpm
pros::Motor R1(20, pros::MotorGears::blue);   // right front
pros::Motor R2(16, pros::MotorGears::blue);   // right back
pros::Motor R3(11, pros::MotorGears::green); // right middle, 3:1 stepped to 600 rpm

//pros::Motor FourBar1(-8, pros::MotorGears::green);
//pros::Motor FourBar2(19, pros::MotorGears::green);
//pros::Motor intake(-2, pros::MotorGears::blue); // FIXED: was pros::MotorGear (wrong type, likely a compile error)

pros::MotorGroup DriveL({-10, -1, -6});
pros::MotorGroup DriveR({20, 16, 11});
//pros::MotorGroup FourBar({19, -8});

pros::adi::Pneumatics claw('H', true);

// Sensors
pros::Imu imu(10);
pros::Rotation trackY(11); // vertical
pros::Rotation trackX(12); // horizontal

// ============================================================
//  LemLib Config
// ============================================================

lemlib::Drivetrain drivetrain(&DriveL, &DriveR,
                              12.5, // track width in inches [TUNE after robot is built]
                              lemlib::Omniwheel::NEW_275, // 2.75" wheels
                              // Wheel speed: motors run 600 rpm at the shared shaft
                              // (blue direct, green stepped 3:1 from 200) through a
                              // 36t:48t external drivetrain reduction -> 450 rpm at the wheel
                              // 600 * (36/48) = 450
                              450,
                              8 // horizontal drift
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