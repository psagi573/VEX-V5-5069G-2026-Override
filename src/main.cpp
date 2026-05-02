#include "main.h"
#include "Autons.h"
#include "drivetrain.h"
#include "robot-config.h"
#include "GUI.h"

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

    // ── MVLib Setup ──────────────────────────────────────────
    auto& logger = mvlib::Logger::getInstance();

    mvlib::setOdom(&chassis);
    logger.setRobot({
        .leftDrivetrain  = &DriveL,
        .rightDrivetrain = &DriveR
    });

    // ── Watches ─────────────────────────────────────────────

    // Battery capacity with low-battery warning
    logger.watch("Battery %", LogLevel::INFO, 5_mvS,
        []() { return (int32_t)pros::battery::get_capacity(); },
        mvlib::LevelOverride<int32_t>{
            .elevatedLevel = LogLevel::WARN,
            .predicate     = PREDICATE(v < 30),
            .label         = "Battery Low"
        });

    // Battery voltage
    logger.watch("Battery mV", LogLevel::INFO, 5_mvS,
        []() { return (int32_t)pros::battery::get_voltage(); },
        mvlib::LevelOverride<int32_t>{});

    // Drivetrain avg temp with overheat warning
    logger.watch("Drive Temp", LogLevel::INFO, 3_mvS,
        [&]() {
            return (DriveL.get_temperature() + DriveR.get_temperature()) / 2.0;
        },
        mvlib::LevelOverride<double>{
            .elevatedLevel = LogLevel::WARN,
            .predicate     = mvlib::asPredicate<double>([](const double& v) {
                return v > 50.0;
            }),
            .label = "Drive Temp High"
        },
        "%.1f");

    // Claw temp
    logger.watch("Claw Temp", LogLevel::INFO, 3_mvS,
        [&]() { return Claw.get_temperature(); },
        mvlib::LevelOverride<double>{
            .elevatedLevel = LogLevel::WARN,
            .predicate     = mvlib::asPredicate<double>([](const double& v) {
                return v > 50.0;
            }),
            .label = "Claw Temp High"
        },
        "%.1f");

    // FourBar temp
    logger.watch("FourBar Temp", LogLevel::INFO, 3_mvS,
        [&]() { return FourBar.get_temperature(); },
        mvlib::LevelOverride<double>{
            .elevatedLevel = LogLevel::WARN,
            .predicate     = mvlib::asPredicate<double>([](const double& v) {
                return v > 50.0;
            }),
            .label = "FourBar Temp High"
        },
        "%.1f");

    // Lift temp
    logger.watch("Lift Temp", LogLevel::INFO, 3_mvS,
        [&]() { return Lift.get_temperature(); },
        mvlib::LevelOverride<double>{
            .elevatedLevel = LogLevel::WARN,
            .predicate     = mvlib::asPredicate<double>([](const double& v) {
                return v > 50.0;
            }),
            .label = "Lift Temp High"
        },
        "%.1f");

    // Competition connection status (on-change)
    logger.watch("Field Conn", LogLevel::INFO, true,
        []() { return pros::competition::is_connected(); },
        mvlib::LevelOverride<bool>{});

    // Selected auton (on-change) — updates if you change it on the brain
    logger.watch("Auton ID", LogLevel::INFO, true,
        []() { return (int32_t)selectedAuton; },
        mvlib::LevelOverride<int32_t>{});

    // ── Start Logger ─────────────────────────────────────────
    logger.start();

    logger.info("Robot initialized | Auton ID: %d", (int)selectedAuton);

    chassis.setPose(0, 0, 0);

    GUI_runAutonSelector();
}

void disabled() {
    mvlib::Logger::getInstance().info("Robot disabled");
}

void competition_initialize() {
    mvlib::Logger::getInstance().info("Competition initialize");
}

void autonomous() {
    auto& logger = mvlib::Logger::getInstance();
    logger.info("Autonomous started | Auton ID: %d", (int)selectedAuton);

    switch ((AutonomousID)selectedAuton) {
        case AUTON_LEFT:
            left();
            break;
        case AUTON_RIGHT:
            right();
            break;
        case AUTON_SKILLS:
            skills();
            break;
        default:
            logger.warn("No auton selected or AUTON_NONE — doing nothing");
            break;
    }

    logger.info("Autonomous finished");
}

void opcontrol() {
    mvlib::Logger::getInstance().info("Opcontrol started");

    new pros::Task(DriveTrainControls);
    new pros::Task(ClawControls);
    new pros::Task(FourBarControls);
    new pros::Task(LiftControls);

    while (true) {
        pros::delay(20);
    }
}