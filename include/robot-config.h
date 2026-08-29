#pragma once
#include "main.h"
#include "pros/adi.hpp"

// ============================================================
//  robot-config.h — ZIPPY 2 | Override 2026-2027
// ============================================================

// Controllers
extern pros::Controller master;
extern pros::Controller partner;

// Drivetrain Motors — 3 per side
// 2x 11W blue cartridge (600 rpm) + 1x 5.5W green cartridge (200 rpm,
// geared 3:1 internally so it also outputs 600 rpm at the shared shaft)
// External drivetrain reduction: 36t driver : 48t driven -> wheel speed 450 rpm
// Wheels: 2.75" omni
extern pros::Motor L1;   // Port 21 (reversed) — blue  — left front
extern pros::Motor L2;   // Port 17             — blue  — left back
extern pros::Motor L3;   // Port 18             — green, 3:1 stepped — left middle
extern pros::Motor R1;   // Port 6  (reversed) — blue  — right front
extern pros::Motor R2;   // Port 16             — blue  — right back
extern pros::Motor R3;   // Port 15 (reversed) — green, 3:1 stepped — right middle
extern pros::Motor cascade1; // Port 5  (reversed) — blue  — right middle
extern pros::Motor cascade2; // Port 7  (reversed) — blue  — right middle

extern pros::MotorGroup DriveL;
extern pros::MotorGroup DriveR;
extern pros::MotorGroup cascade;
//extern pros::MotorGroup FourBar;

// Mechanism Motors
//extern pros::Motor FourBar1; // Port 8  (reversed) — green
//extern pros::Motor FourBar2; // Port 19             — green
//extern pros::Motor intake;   // Port 2  (reversed) — blue

extern pros::adi::Pneumatics claw;

// Sensors
extern pros::Imu imu;           // Port 10
extern pros::Rotation trackY;   // Port 11 — vertical tracking wheel
extern pros::Rotation trackX;   // Port 12 — horizontal tracking wheel

// Expo drive curves
extern lemlib::ExpoDriveCurve throttle_curve;
extern lemlib::ExpoDriveCurve steer_curve;

// LemLib Chassis
extern lemlib::Chassis chassis;