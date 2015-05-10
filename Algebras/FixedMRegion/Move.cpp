/*
This class calculates the movements.

*/
using namespace std;
#include "Move.h"

Move::Move() {}
/*
This constructor receives a starting point (x0, y0), a starting 
angle alpha0, and a moving vector (vx, vy) and a moving angle.

*/
Move::Move (double x0, double y0, double alpha0, double vx,
            double vy, double valpha){
  x_0 = x0;
  y_0 = y0;
  alpha_0 = alpha0;
  v_x = vx;
  v_y = vy;
  v_alpha = valpha;
  t_a = convert (0.0);
  t_e = convert (1.0);
}

/*
This constructor receives a starting point (xa, ya), a starting angle alphaa,
a start time ta, a moving vector (xe, ye), a moving angle alphae and
an end time te.

*/
Move::Move (double xa, double ya, double alphaa, double ta, double xe,
            double ye, double alphae, double te){
  x_0 = xa;
  y_0 = ya;
  alpha_0 = alphaa;
  v_x = xe - xa;
  v_y = ye - ya;
  v_alpha = alphae - alphaa;
  t_a = convert (ta);
  t_e = convert (te);
}

/*
This constructor receives a starting point (xa, ya), a starting angle alphaa,
a moving vector (xe, ye), a moving angle alphae and time t as instant.

*/
Move::Move (double xa, double ya, double alphaa, Instant ta, double xe,
            double ye, double alphae, Instant te){
  x_0 = xa;
  y_0 = ya;
  alpha_0 = alphaa;
  v_x = xe - xa;
  v_y = ye - ya;
  v_alpha = alphae - alphaa;
  t_a = ta;
  t_e = te;
}

/*
This method calculates the necessary set of (x,y) and angle alpha for a
given time t that is relative to the start point. relatime is not used.

*/
double *Move::attime (double t, bool relatime) const{
  double span = convert (t_e - t_a);
  if (t < 0)
    t = 0;
  if (t > span)
    t = span;
  double scale = t / span;
  double *tmp = new double[3];
  double x = x_0 + scale * v_x;
  tmp[0] = x;
  double y = y_0 + scale * v_y;
  tmp[1] = y;
  double a = alpha_0 + scale * v_alpha;
  tmp[2] = a;
  return tmp;
}

/*
This method calculates the necessary set of (x,y) and angle
alpha for a given time t as an absolute value.

*/

double *Move::attime (double t) const{
  return attime (t - convert (t_a), true);
}

double *Move::attime (Instant t) const{
  return attime (convert (t - t_a), true);
}

/*
This is the copy constructor.

*/
Move::Move (const Move & _m):x_0 (_m.x_0), y_0 (_m.y_0), alpha_0 (_m.alpha_0),
v_x (_m.v_x), v_y (_m.v_y), v_alpha (_m.v_alpha), t_a (_m.t_a),
t_e (_m.t_e)
{
}



/*
This is the destructor.

*/
Move::~Move (){}
/*
This method returns the start time of the valid interval as DateTime.

*/
DateTime Move::getStart () const{
  return t_a;
}
/*
This method returns the start time of the valid interval as an double
as absolute time.

*/
double Move::getStart (const double dummy) const{
  return convert (t_a);
}
/*
This method returns the end time of the valid interval as DateTime.

*/
DateTime Move::getEnd () const{
  return t_e;
}
/*
This method returns the end time of the valid interval as an double
as absolute time.

*/
double Move::getEnd (const double dummy) const{
  return convert (t_e);
}
/*
This method converts the given Datetime time into double.

*/
double Move::convert (DateTime t) const{
  return t.ToDouble ();
}
/*
This method converts the given double time into Datetime.

*/
DateTime Move::convert (double t) const{
  return DateTime (t);
}
