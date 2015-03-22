/*
This class calculates the movements.

*/
using namespace std;
#include "Move.h"
/*
This constructor receives a starting point (x0, y0), a starting 
angle alpha0, and a moving vector (vx, vy) and a moving angle.

*/
  Move::Move(double x0, double y0, double alpha0, double vx, 
   double vy, double valpha)
    {
      x_0=x0;
      y_0=y0;
      alpha_0=alpha0;
      v_x=vx;
      v_y=vy;
      v_alpha=valpha;
      //printf("Hello World. I am a litle move obejct.\n");
    }
/*
This method calculates the necessary set of (x,y) and angle
alpha for a given time t.

*/
    double* Move::attime(double t){
      double *tmp=new double[3];
      double x=x_0+t*v_x;
      tmp[0]=x;
      double y=y_0+t*v_y;
      tmp[1]=y;
      double a=alpha_0+t*v_alpha;
      tmp[2]=a;
      return tmp;
    }
/*
This is the copy constructor.

*/
    Move::Move(const Move &_m): x_0(_m.x_0), y_0(_m.y_0), 
alpha_0(_m.alpha_0), v_x(_m.v_x), v_y(_m.v_y), v_alpha(_m.v_alpha) {}

/*
This is the constructor. Do not use.

*/
    Move::Move(){}
/*
This is the destructor.

*/
    Move::~Move(){}
;