#include "Autons.h"
#include "robot-config.h"
#include "main.h"

// ============================================================
//  Autons.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

void left() {
    auto& logger = mvlib::Logger::getInstance();
    logger.info("Running: autonLeft");

    chassis.setPose(0, 0, 0);

    // Example waypoint — update coordinates once you know your field positions
    auto wp1 = logger.addWaypoint("Left Target", {
        .tarX       = 0,
        .tarY       = 24,
        .linearTol  = 2.0f,
        .timeoutMs  = 3000
    });

    chassis.moveToPoint(0, 24, 3000);

    if (wp1.reached())
        logger.info("Left Target: reached");
    else if (wp1.timedOut())
        logger.warn("Left Target: timed out — offset X:%.1f Y:%.1f",
            wp1.getOffset().offX, wp1.getOffset().offY);
}

void right() {
    auto& logger = mvlib::Logger::getInstance();
    logger.info("Running: autonRight");

    chassis.setPose(0, 0, 0);

    auto wp1 = logger.addWaypoint("Right Target", {
        .tarX       = 0,
        .tarY       = 24,
        .linearTol  = 2.0f,
        .timeoutMs  = 3000
    });

    chassis.moveToPoint(0, 24, 3000);

    if (wp1.reached())
        logger.info("Right Target: reached");
    else if (wp1.timedOut())
        logger.warn("Right Target: timed out — offset X:%.1f Y:%.1f",
            wp1.getOffset().offX, wp1.getOffset().offY);
}

void skills() {
    auto& logger = mvlib::Logger::getInstance();
    logger.info("Running: autonSkills");

    chassis.setPose(0, 0, 0);

    // Add waypoints here as you build out your skills route
}