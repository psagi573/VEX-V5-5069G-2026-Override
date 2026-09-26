#include "main.h"
#include "drivetrain.h"
#include "robot-config.h"
#include "cascade.h"
#include "GUI.h"
#include "DSR.h"
#include "Autons.h"

pros::Task* odomTask = nullptr;
// ============================================================
//  main.cpp — ZIPPY 2 | Override 2026-2027
// ============================================================

CascadeController lift(
    cascade,                        // your existing MotorGroup
    16,                             // "Lift" rotation sensor port
    {225, 1100, 1450, 1800},    // START, LOW, MIDDLE, HIGH
    800.0,                           // degrees of lift per pin already in the hole
    0.9, 0.0, 0.0                  // kP, kI, kD
);

dsr_sensor north({-6, 0}, 17);//
dsr_sensor east({5.5, -1.5}, 13);//
dsr_sensor south({-6, -3}, 14);//
dsr_sensor west({-5, -3}, 15);   

dsr_chassis dsr_system(&chassis, {&north, &east, &south, &west});



void odomDebug(void *) {
  master.clear();
  Lift.set_position(0);
  pros::delay(50);
  while (true) {
    lemlib::Pose pose = chassis.getPose();
    //master.print(0, 0, "X%5.1f Y%5.1f H%5.1f", pose.x, pose.y, pose.theta);
    //master.print(2, 0, "X true:%5.1f Y true:%5.1f", trackX.get_position(),trackY.get_position());
    master.print(0, 0, "degrees:%5.1f",Lift.get_position()/100.0);
    pros::delay(50);
  }
}
void initialize() {
    
    
    chassis.calibrate(true); // ~3s IMU calibration

    DriveL.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
    DriveR.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
    cascade1.set_brake_mode_all(pros::E_MOTOR_BRAKE_HOLD);
    cascade2.set_brake_mode_all(pros::E_MOTOR_BRAKE_HOLD);
    intake.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);

    

    chassis.setPose(0, 0, 0);

    odomTask = new pros::Task(odomDebug);
    //GUI_runAutonSelector();
}

void autonRed() {
    chassis.setPose(-8, -62, 180); // set this
    chassis.moveToPoint(-8, -65, 400); /// toggle
    chassis.moveToPose(-14,-48 ,90 , 1000, {.forwards = false, .lead = 0.3}, false); /// allaince
    pros::delay(400);
    lift.setTarget(CascadeLevel::MIDDLE_GOAL, 0); // first pin in the middle goal 
    claw.extend();
    chassis.turnToHeading(180, 1000);
    pros::delay(500);
    chassis.moveToPose(-24,-58,0, 1000, {.forwards = true, .lead = 0}, true);
    lift.setTarget(CascadeLevel::START);
    chassis.moveToPoint(-24, -62, 1000);
    claw.retract();
    chassis.moveToPose(-24,-58,180, 1000, {.forwards = true, .lead = 0}, true);
    

    


    // TODO: build the left-side routine once the robot exists
    // and field coordinates are measured.
}

void disabled() {
    //GUI_showDebugScreen();
}

void competition_initialize() {
    // Selector screen stays up from initialize() until the match starts.
}

void autonomous() {
    odomTask = new pros::Task(odomDebug);
    lift.init();


    autonRed();


    // chassis.turnToHeading(90, 1000);
    // chassis.turnToHeading(180, 1000);
    // chassis.turnToHeading(0, 1000);



    //chassis.moveToPoint(0.0, 24.0, 1000);

    //chassis.moveToPoint(0.0, 0.0, 1000);
    // if (selectedAuton >= 0 && selectedAuton < AUTON_COUNT) {
    //     AUTONS[selectedAuton].run();
    // }

    // Drive to the middle goal with lemlib as usual
    // chassis.moveToPose(...);
 
    // First goal, hole is empty
    //lift.setTarget(CascadeLevel::MIDDLE_GOAL, 1);
    // lift.waitUntilSettled();
    // ... place pin ...

    // Different goal entirely, also empty -- still 0, no carryover
    //  lift.setTarget(CascadeLevel::HIGH_GOAL, 0);
    // lift.waitUntilSettled();
    // ... place pin ...

    // Back to the first goal, which now has 1 pin in it
    // lift.setTarget(CascadeLevel::MIDDLE_GOAL, 1);
    //lift.waitUntilSettled();
}

void opcontrol() {
    odomTask = new pros::Task(odomDebug);
    //GUI_showDebugScreen();

    new pros::Task(DriveTrainControls);
    new pros::Task(CascadeControls);
    new pros::Task(IntakeControls);
    new pros::Task(wristControls);
    new pros::Task(ClawControls);



    

}