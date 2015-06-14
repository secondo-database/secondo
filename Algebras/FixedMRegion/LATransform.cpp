/*
This class calculates those values, that are fixed, as soon as its 
constructor is called.

*/
using namespace std;
#include "LATransform.h"
/*
This is the constructor. It gets a linear movement (x,y), the 
middle of a circle (xm, ym) and an ankle alpha. 
This is possible because another class knows t and therefore does 
already know alpha, x and y. 
Later on, the methods of this class can be called for various points,
without the already given information. 

*/
LATransform::LATransform(double x, double y, double _xm, double _ym,
  double alpha):xm(_xm), ym(_ym){
  //A'=M*A+(D-M*D+W) is equation of movement.
  //M is rotational matrix with angle alpha
  //D=(xm, ym), rotaional center; D!=d completely different meaning!
  //W=(x,y) linear movement
  //A point to move
  //M=(a00 a01)
  //  (a10 a11)     
  a00 = cos (alpha);
  a01 = -sin (alpha);
  a10 = sin (alpha);
  a11 = cos (alpha);
  //c=(D-M*D+W) for movement
  cx = xm - (a00 * xm + a01 * ym) + x;
  cy = ym - (a10 * xm + a11 * ym) + y;
  //M'=M^(-1)
  // because M is rotaional matrix: M'=M^T (transposed)
  //inverse of movement is:
  //A=M'*A'-(M'*(D+W)+D)
  //d=-(M'*(D+W)+D)
  dx = -(a00 * (x + xm) + a10 * (y + ym)) + xm;
  dy = -(a01 * (x + xm) + a11 * (y + ym)) + ym;
}

/*
This method calculates the new x value that the given point will get 
after its movement.

*/
double
LATransform::getImgX(double x, double y){
  double tmp = 0;
  tmp = a00 * x + a01 * y + cx;
  return tmp;
}

/*
This method calculates the new y value that the given point will 
get after its 
movement.

*/
double
LATransform::getImgY(double x, double y){
  double tmp;
  tmp = a10 * x + a11 * y + cy;
  return tmp;
}
/*
This method calculates the original x value that the given point would have before its movement.
It undoes the movement, that is caused from LATransform.

*/
double
LATransform::getOrigX(double x, double y){
  double tmp = 0;
  tmp = a00 * x + a10 * y + dx;
  return tmp;
}
/*
This method calculates the original y value that the given point would have before its movement.
It undoes the movement, that is caused from LATransform.

*/
double
LATransform::getOrigY(double x, double y){
  double tmp;
  tmp = a01 * x + a11 * y + dy;
  return tmp;
}

/*
usual destructor

*/
LATransform::~LATransform(){
}

/*
This method returns the turning points x value.

*/
double
LATransform::getXM(){
  return xm;
}

/*
This method returns the turning points y value.

*/
double
LATransform::getYM(){
  return ym;
}

void LATransform::Set(double x, double y, double _xm, double _ym,
  double alpha){
  //A'=M*A+(D-M*D+W) is equation of movement.
  //M is rotational matrix with angle alpha
  //D=(xm, ym), rotaional center; D!=d completely different meaning!
  //W=(x,y) linear movement
  //A point to move
  //M=(a00 a01)
  //  (a10 a11)     
  xm=_xm;
  ym=_ym;
  a00 = cos (alpha);
  a01 = -sin (alpha);
  a10 = sin (alpha);
  a11 = cos (alpha);
  //c=(D-M*D+W) for movement
  cx = xm - (a00 * xm + a01 * ym) + x;
  cy = ym - (a10 * xm + a11 * ym) + y;
  //M'=M^(-1)
  // because M is rotaional matrix: M'=M^T (transposed)
  //inverse of movement is:
  //A=M'*A'-(M'*(D+W)+D)
  //d=-(M'*(D+W)+D)
  dx = -(a00 * (x + xm) + a10 * (y + ym)) + xm;
  dy = -(a01 * (x + xm) + a11 * (y + ym)) + ym;
}


;
