#ifndef PPAUTON_H_INCLUDED
#define PPAUTON_H_INCLUDED


#include "pros/apix.h"
#include <cmath>

#define waitUntil(condition) \
  while(!(condition)) { pros::delay(5); }
#define waitUntilT(condition, timelimit) {\
  int startTime = pros::millis(); \
  while(!(condition || (pros::millis() - startTime) > timelimit)) { pros::delay(5); } \
}

void drive(double, int, pros::Motor *, pros::Motor *);
void pointTurn(int, int, pros::Motor *, pros::Motor *);
void releaseStack(pros::Motor *, pros::Motor *, pros::Motor *, pros::Motor *);
void assertStack(pros::Motor *, pros::Motor *, pros::Motor *);
void tareStack(pros::Motor *, pros::ADIButton *, int);
void tareLift(pros::Motor *, pros::Motor *, pros::ADIButton *, int);
void deploy(pros::Motor *, pros::Motor *, pros::Motor *);


#endif // PPAUTON_H_INCLUDED
