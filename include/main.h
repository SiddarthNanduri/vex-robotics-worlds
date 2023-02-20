/**
 * \file main.h
 *
 * Contains common definitions and header files used throughout your PROS
 * project.
 *
 * Copyright (c) 2017-2018, Purdue University ACM SIGBots.
 * All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _PROS_MAIN_H_
#define _PROS_MAIN_H_

/**
 * If defined, some commonly used enums will have preprocessor macros which give
 * a shorter, more convenient naming pattern. If this isn't desired, simply
 * comment the following line out.
 *
 * For instance, E_CONTROLLER_MASTER has a shorter name: CONTROLLER_MASTER.
 * E_CONTROLLER_MASTER is pedantically correct within the PROS styleguide, but
 * not convienent for most student programmers.
 */
#define PROS_USE_SIMPLE_NAMES

/**
 * If defined, C++ literals will be available for use. All literals are in the
 * pros::literals namespace.
 *
 * For instance, you can do `4_mtr = 50` to set motor 4's target velocity to 50
 */
#define PROS_USE_LITERALS

//#include "api.h"
#include "pros/apix.h"

/**
 * You should add more #includes here
 */
//#include "okapi/api.hpp"
//#include "pros/api_legacy.h"
#include "custom/AutonData.h"
#include "custom/DriverProfile.h"
#include "custom/PPAuton.h"
#define waitUntil(condition) \
  while(!(condition)) { pros::delay(5); }
#define waitUntilT(condition, timelimit) {\
  int startTime = pros::millis(); \
  while(!(condition || (pros::millis() - startTime) > timelimit)) { pros::delay(5); } \
}
template <typename T> int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

#define VERSION_MAJOR 5
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_INFO ""
#define VERSION_YEAR 2020

#define ALG_NONE 0
#define ALG_ASSERT_RELEASE 1
#define ALG_RELEASE 2
#define ALG_TARESTACK 3

#define LIFTT 2500 // Lift motor timeout.
#define STACKT 2000 // Stack motor timeout.

/**
 * If you find doing pros::Motor() to be tedious and you'd prefer just to do
 * Motor, you can use the namespace with the following commented out line.
 *
 * IMPORTANT: Only the okapi or pros namespace may be used, not both
 * concurrently! The okapi namespace will export all symbols inside the pros
 * namespace.
 */
//using namespace pros;
//using namespace pros::literals;
// using namespace okapi;

/**
 * Prototypes for the competition control tasks are redefined here to ensure
 * that they can be called from user code (i.e. calling autonomous from a
 * button press in opcontrol() for testing purposes).
 */
#ifdef __cplusplus
extern "C" {
#endif
void autonomous(void);
void initialize(void);
void disabled(void);
void competition_initialize(void);
void opcontrol(void);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
/**
 * You can add C++-only headers here
 */
 #include <string>
 #include <iostream>
 #include <sstream>
 using namespace std;
#endif

#endif  // _PROS_MAIN_H_
