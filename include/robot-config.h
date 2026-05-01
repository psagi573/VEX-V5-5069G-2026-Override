#pragma once
#include "main.h"

// ============================================================
//  robot-config.h — ZIPPY 2 | Override 2026-2027
// ============================================================

// Controllers
extern pros::Controller master;
extern pros::Controller partner;

// Drivetrain Motors
// 6 motor: 2x 11W + 1x 5.5W per side | Blue (600RPM) | 48:84 | 4" wheels
extern pros::Motor dt_L1;   // [PORT 1]  Left front  — 11W
extern pros::Motor dt_L2;   // [PORT 2]  Left back   — 11W
extern pros::Motor dt_L3;   // [PORT 3]  Left middle — 5.5W
extern pros::Motor dt_R1;   // [PORT 4]  Right front — 11W
extern pros::Motor dt_R2;   // [PORT 5]  Right back  — 11W
extern pros::Motor dt_R3;   // [PORT 6]  Right middle— 5.5W

extern pros::MotorGroup DriveL;
extern pros::MotorGroup DriveR;

// Mechanism Motors
extern pros::Motor Claw;    // [PORT 7]  — cartridge TBD
extern pros::Motor FourBar; // [PORT 8]  — cartridge TBD
extern pros::Motor Lift;    // [PORT 9]  — cartridge TBD

// Sensors
extern pros::Imu imu;           // [PORT 10]
extern pros::Rotation trackY;   // [PORT 11] vertical tracking wheel
extern pros::Rotation trackX;   // [PORT 12] horizontal tracking wheel

// LemLib Chassis
extern lemlib::Chassis chassis;