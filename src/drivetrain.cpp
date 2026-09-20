#include "drivetrain.h"
#include "pros/misc.h"
#include "robot-config.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

// ============================================================
// drivetrain.cpp — AVRO | Override 2026-2027
// Driver Control V2
// ============================================================

static constexpr double JOYSTICK_DEADBAND = 4.0;

// expo cutoff after will be linear
static constexpr double EXPO_CUTOFF = 19.0;

// time to hold the combo to toggle
static constexpr std::uint32_t DRIVE_TOGGLE_HOLD_MS = 1200;


// ------------------------------------------------------------
// DRIVE MODE
// ------------------------------------------------------------

enum class DriveMode {
    ARCADE,
    TANK
};

// Start split arcade.
static DriveMode driveMode = DriveMode::ARCADE;


// ------------------------------------------------------------
// LINEAR JOYSTICK input
// ------------------------------------------------------------

double linearJoystick(double input) {
    if (std::fabs(input) < JOYSTICK_DEADBAND) {
        return 0.0;
    }

    return input;
}


// ------------------------------------------------------------
// CONTROLLER VALUE into MOTOR VOLTAGE
//
// Input range is -127 to +127
//
// Output range value is -12000 → +12000 mV
// ------------------------------------------------------------

int joystickToVoltage(double joystick) {

    joystick = std::clamp(joystick, -127.0, 127.0);

    return static_cast<int>(
        joystick * 12000.0 / 127.0
    );
}


// ------------------------------------------------------------
// DRIVE MODE TOGGLE
//
// hold up and x for 2 seconds to change drive mode.
// ------------------------------------------------------------

static bool comboTiming = false;
static bool comboTriggered = false;
static std::uint32_t comboStartTime = 0;

void checkDriveModeToggle() {

    const bool upHeld =
        master.get_digital(pros::E_CONTROLLER_DIGITAL_UP);

    const bool xHeld =
        master.get_digital(pros::E_CONTROLLER_DIGITAL_X);

    const bool comboHeld = upHeld && xHeld;


    // --------------------------------------------------------
    // Combo code start
    // --------------------------------------------------------

    if (comboHeld && !comboTiming) {

        comboTiming = true;
        comboTriggered = false;
        comboStartTime = pros::millis();
    }


    // --------------------------------------------------------
    // Combo buttons held
    // --------------------------------------------------------

    if (comboHeld && comboTiming && !comboTriggered) {

        const std::uint32_t heldTime =
            pros::millis() - comboStartTime;

        if (heldTime >= DRIVE_TOGGLE_HOLD_MS) {

            // Toggle drive mode.
            if (driveMode == DriveMode::ARCADE) {

                driveMode = DriveMode::TANK;

                master.rumble("-");

            } else {

                driveMode = DriveMode::ARCADE;

                master.rumble(".");
            }

            // Prevent another toggle until release.
            comboTriggered = true;
        }
    }


    // --------------------------------------------------------
    // Combo released
    // --------------------------------------------------------

    if (!comboHeld) {

        comboTiming = false;
        comboTriggered = false;
    }
}


// ------------------------------------------------------------
// SPLIT ARCADE
//
// LEFT Y  = throttle
// RIGHT X = turn
//
// before cutoff uses LemLib ExpoDriveCurve.
//
// after cutoff uses full linear arcade 
// ------------------------------------------------------------

void arcadeDrive() {

    double forward =
        master.get_analog(
            pros::E_CONTROLLER_ANALOG_LEFT_Y
        );

    double turn =
        master.get_analog(
            pros::E_CONTROLLER_ANALOG_RIGHT_X
        );


    // --------------------------------------------------------
    // EXPO RANGE
    // --------------------------------------------------------

    if (std::fabs(forward) < EXPO_CUTOFF &&
        std::fabs(turn) < EXPO_CUTOFF) {

        // LemLib applies the ExpoDriveCurve configured
        // in robot-config.cpp.
        chassis.arcade(forward, turn);

        return;
    }


    // --------------------------------------------------------
    // LINEARIZATION
    // --------------------------------------------------------

    forward = linearJoystick(forward);
    turn = linearJoystick(turn);


    double leftPower =
        std::clamp(
            forward + turn,
            -127.0,
            127.0
        );

    double rightPower =
        std::clamp(
            forward - turn,
            -127.0,
            127.0
        );


    DriveL.move_voltage(
        joystickToVoltage(leftPower)
    );

    DriveR.move_voltage(
        joystickToVoltage(rightPower)
    );
}


// ------------------------------------------------------------
// TANK DRIVE
//
// LEFT Y  = left drivetrain
// RIGHT Y = right drivetrain
//
// full linear.
// No ExpoDriveCurve.
// ------------------------------------------------------------

void tankDrive() {

    double left =
        master.get_analog(
            pros::E_CONTROLLER_ANALOG_LEFT_Y
        );

    double right =
        master.get_analog(
            pros::E_CONTROLLER_ANALOG_RIGHT_Y
        );


    left = linearJoystick(left);
    right = linearJoystick(right);


    DriveL.move_voltage(
        joystickToVoltage(left)
    );

    DriveR.move_voltage(
        joystickToVoltage(right)
    );
}


// ------------------------------------------------------------
// MAIN DRIVER CONTROL
// ------------------------------------------------------------

void DriveTrainControls() {

    while (true) {

        // Check for UP + X drive-mode toggle.
        checkDriveModeToggle();


        // Select current drive mode.
        if (driveMode == DriveMode::ARCADE) {

            arcadeDrive();

        } else {

            tankDrive();
        }


        pros::delay(10);
    }
}


// ============================================================
// CASCADE
// ============================================================

// L1 = up
// L2 = down

void CascadeControls() {

    while (true) {

        if (master.get_digital(
                pros::E_CONTROLLER_DIGITAL_L1)) {

            cascade.move(127);

        } else if (master.get_digital(
                       pros::E_CONTROLLER_DIGITAL_L2)) {

            cascade.move(-127);

        } else {

            cascade.brake();
        }

        pros::delay(10);
    }
}


// ============================================================
// INTAKE
// ============================================================

// R1 = in
// R2 = out

void IntakeControls() {

    while (true) {

        if (master.get_digital(
                pros::E_CONTROLLER_DIGITAL_R1)) {

            intake.move(127);

        } else if (master.get_digital(
                       pros::E_CONTROLLER_DIGITAL_R2)) {

            intake.move(-127);

        } else {

            intake.brake();
        }

        pros::delay(10);
    }
}