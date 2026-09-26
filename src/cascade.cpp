#include "cascade.h"

CascadeController::CascadeController(
    pros::MotorGroup& motors,
    int rotationPort,
    std::vector<double> basePositions,
    double pinHeightDeg,
    double kP,
    double kI,
    double kD
)
    : m_motors(motors),
      m_rotationPort(rotationPort),
      m_kP(kP),
      m_kI(kI),
      m_kD(kD),
      m_basePositions(std::move(basePositions)),
      m_pinHeightDeg(pinHeightDeg) {

    // Make sure the vector contains one position for every CascadeLevel.
    m_basePositions.resize(
        static_cast<size_t>(CascadeLevel::COUNT),
        0.0
    );
}


void CascadeController::init() {

    if (m_initialized) {
        return;
    }

    // Create the rotation sensor only after PROS has finished
    // constructing the robot configuration.
    m_rotation = new pros::Rotation(m_rotationPort);

    // Zero the rotation sensor.
    m_rotation->reset_position();

    // Hold the cascade when the PID is disabled.
    m_motors.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);

    // Start by holding the current physical position.
    m_target = getPosition();

    m_prevError = 0.0;
    m_integral = 0.0;

    m_initialized = true;

    // Start the background PID task.
    m_task = new pros::Task(
        taskFn,
        this,
        "cascadePID"
    );
}


void CascadeController::inchesTOdegrees(double myinches) {

    // This function currently calculates the conversion but does not
    // return a value because the original interface is void.
    //
    // If you want this function to actually return the converted
    // degrees later, change the declaration/definition to double.

    double inchesToDegrees =
        myinches * 360.0 / (0.25 * M_PI);

    (void)inchesToDegrees;
}


double CascadeController::getPosition() {

    if (!m_initialized || m_rotation == nullptr) {
        return 0.0;
    }

    // PROS Rotation::get_position() returns centidegrees.
    // Convert to degrees.
    return m_rotation->get_position() / 100.0;
}


double CascadeController::getTarget() {

    return m_target;
}


double CascadeController::getError() {

    return m_target - getPosition();
}


double CascadeController::heightFor(
    CascadeLevel level,
    int pinsAlreadyInHole
) const {

    size_t index = static_cast<size_t>(level);

    if (index >= m_basePositions.size()) {
        return 0.0;
    }

    if (pinsAlreadyInHole < 0) {
        pinsAlreadyInHole = 0;
    }

    return m_basePositions[index]
         + (pinsAlreadyInHole * m_pinHeightDeg);
}


void CascadeController::setTarget(
    CascadeLevel level,
    int pinsAlreadyInHole
) {

    size_t index = static_cast<size_t>(level);

    if (index >= m_basePositions.size()) {
        return;
    }

    m_target = heightFor(
        level,
        pinsAlreadyInHole
    );

    m_integral = 0.0;
}


void CascadeController::setTargetDegrees(double degrees) {

    m_target = degrees;

    m_integral = 0.0;
}


bool CascadeController::isSettled(double toleranceDeg) {

    return std::fabs(getError()) <= toleranceDeg;
}


void CascadeController::waitUntilSettled(
    double toleranceDeg,
    int timeoutMs
) {

    int waited = 0;

    while (
        !isSettled(toleranceDeg)
        && waited < timeoutMs
    ) {
        pros::delay(10);
        waited += 10;
    }
}


void CascadeController::enable() {

    if (!m_initialized) {
        return;
    }

    m_integral = 0.0;

    m_prevError = getError();

    m_target = getPosition();

    m_enabled = true;
}


void CascadeController::disable() {

    m_enabled = false;

    m_motors.move(0);
}


void CascadeController::step() {

    if (!m_initialized || !m_enabled) {
        return;
    }

    double error = getError();

    // Integral.
    m_integral += error;

    m_integral = std::clamp(
        m_integral,
        -INTEGRAL_LIMIT,
        INTEGRAL_LIMIT
    );

    // Derivative.
    double derivative = error - m_prevError;

    m_prevError = error;

    // PID calculation.
    double output =
        (m_kP * error)
        + (m_kI * m_integral)
        + (m_kD * derivative);

    // Limit motor output to PROS motor range.
    output = std::clamp(
        output,
        -MAX_OUTPUT,
        MAX_OUTPUT
    );

    m_motors.move(output);
}


void CascadeController::taskFn(void* param) {

    auto* self =
        static_cast<CascadeController*>(param);

    while (true) {

        self->step();

        pros::delay(10);
    }
}