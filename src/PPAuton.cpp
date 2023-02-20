#include "custom/PPAuton.h"

void drive(double y, int speed, pros::Motor *ldrive, pros::Motor *rdrive) // WARNING: this function screws with the encoders.  TODO: don't do that.
{
  // y is measured in inches
  const double pi = 3.141593;
  double wheelCirc = 4 * pi;
  double deg = (y * 360.0) / wheelCirc;
  ldrive->tare_position(); // screwing with encoders
  rdrive->tare_position();
  pros::motor_brake_mode_e_t li = ldrive->get_brake_mode();
  pros::motor_brake_mode_e_t ri = rdrive->get_brake_mode();
  ldrive->set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
  rdrive->set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
  ldrive->move_relative(deg, speed);
  rdrive->move_relative(deg, speed);
  waitUntil(fabs(ldrive->get_position()) > fabs(deg) && fabs(rdrive->get_position()) > fabs(deg));
  waitUntil(fabs(ldrive->get_actual_velocity()) < 10 && fabs(rdrive->get_actual_velocity()) < 10);
  ldrive->set_brake_mode(li);
  rdrive->set_brake_mode(ri);
}

void pointTurn(int degrees, int speed, pros::Motor *ldrive, pros::Motor *rdrive) // TODO
{
  const double pi = 3.141593;
  double theta = (degrees * pi) / 180.0;
  double wheelCirc = 4 * pi;
  double frameRadius = 17.0 / 2;
  // TODO
  waitUntil(ldrive->is_stopped() && rdrive->is_stopped());
}

void releaseStack(pros::Motor *ldrive, pros::Motor *rdrive, pros::Motor *intake0, pros::Motor *intake1)
{
  intake0->move(-30);
  intake1->move(-30);
  pros::delay(150);
  intake0->move(-55);
  intake1->move(-55);
  drive(-10, 60, ldrive, rdrive); // not full speed
  intake0->move(0);
  intake1->move(0);
}

void assertStack(pros::Motor *stack, pros::Motor *intake0, pros::Motor *intake1) // assert dominance
{
  stack->move_velocity(130);
  //intake0->move(-30); // TODO: tune these values
  //intake1->move(-30);
  waitUntilT(stack->get_position() >= 290*5, 4000);
  stack->move_velocity(0);
  //intake0->move(0);
  //intake1->move(0);
}

void tareStack(pros::Motor *stack, pros::ADIButton *stackBtn, int stackt)
{
  stack->move(-127);
  waitUntilT(stackBtn->get_value(), stackt);
  stack->move(0);
  stack->tare_position();
}

void tareLift(pros::Motor *lift0, pros::Motor *lift1, pros::ADIButton *liftBtn, int liftt)
{
  lift0->move(-90);
	lift1->move(-90);
	waitUntilT(liftBtn->get_value(), liftt);
	lift0->move(0);
	lift1->move(0);
	lift0->tare_position();
	lift1->tare_position();
}

void deploy(pros::Motor *lift0, pros::Motor *lift1, pros::Motor *stack) // Releases wheels and stack at beginning of match.  Not yet implemented or finished (TODO).
{
  lift0->move(127);
  lift1->move(127);
  stack->move(127);
  pros::delay(300);
  lift0->move(0);
  lift1->move(0);
  stack->move(0);
}
