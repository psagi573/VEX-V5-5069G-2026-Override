#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include "pros/motor_group.hpp"
#include "pros/rotation.hpp"
#include "pros/rtos.hpp"

enum class CascadeLevel {
    START = 0,
    LOW_GOAL = 1,
    MIDDLE_GOAL = 2,
    HIGH_GOAL = 3,
    COUNT
};

class CascadeController {
public:
    CascadeController(
        pros::MotorGroup& motors,
        int rotationPort,
        std::vector<double> basePositions,
        double pinHeightDeg,
        double kP,
        double kI,
        double kD
    );

    void init();

    void inchesTOdegrees(double myinches);

    void setTarget(
        CascadeLevel level,
        int pinsAlreadyInHole = 0
    );

    void setTargetDegrees(double degrees);

    double heightFor(
        CascadeLevel level,
        int pinsAlreadyInHole
    ) const;

    double getPosition();
    double getTarget();
    double getError();

    bool isSettled(double toleranceDeg = 1.0);

    void waitUntilSettled(
        double toleranceDeg = 1.0,
        int timeoutMs = 3000
    );

    void disable();
    void enable();

private:
    void step();
    static void taskFn(void* param);

    pros::MotorGroup& m_motors;

    int m_rotationPort;

    pros::Rotation* m_rotation = nullptr;

    double m_kP;
    double m_kI;
    double m_kD;

    double m_integral = 0.0;
    double m_prevError = 0.0;
    double m_target = 0.0;

    bool m_enabled = true;
    bool m_initialized = false;

    std::vector<double> m_basePositions;

    double m_pinHeightDeg;

    pros::Task* m_task = nullptr;

    static constexpr double INTEGRAL_LIMIT = 20.0;
    static constexpr double MAX_OUTPUT = 127.0;
};