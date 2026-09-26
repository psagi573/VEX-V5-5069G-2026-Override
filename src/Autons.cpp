// #include "Autons.h"

// #include "robot-config.h"
// #include "main.h"
// #include "cascade.h"
// // ============================================================
// //  Autons.cpp — AVRO | Override 2026-2027
// // ============================================================

// void autonNone() {

//     // fallback
// }

// void autonRed() {
//     chassis.setPose(-8, -62, 180); // set this
//     chassis.moveToPoint(-8, -65, 400); /// toggle
//     chassis.moveToPose(-14,-48 ,90 , 1000, {.forwards = false, .lead = 0.3}, false); /// allaince
//     pros::delay(400);
//     lift.setTarget(CascadeLevel::MIDDLE_GOAL, 0); // first pin in the middle goal 
//     claw.extend();
//     chassis.turnToHeading(180, 1000);
//     pros::delay(500);
//     chassis.moveToPose(-24,-58,0, 1000, {.forwards = true, .lead = 0}, true);
//     lift.setTarget(CascadeLevel::START);
//     chassis.moveToPoint(-24, -62, 1000);
//     claw.retract();
//     chassis.moveToPose(-24,-58,180, 1000, {.forwards = true, .lead = 0}, true);
    

    


//     // TODO: build the left-side routine once the robot exists
//     // and field coordinates are measured.
// }

// void autonLeft() {

//     chassis.setPose(0, 0, 0);

    
// }

// void autonRight() {

//     chassis.setPose(0, 0, 0);

   
// }

// void autonSkills() {

//     chassis.setPose(0, 0, 0);

    
// }

// const AutonEntry AUTONS[] = {

//     {"Nothing", autonNone},
//     {"Left",       autonLeft},
//     {"Right",      autonRight},
//     {"Skills",     autonSkills},

// };

// const int AUTON_COUNT = sizeof(AUTONS) / sizeof(AUTONS[0]);