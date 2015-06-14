/*

This class calculates those values, that are fixed, as soon as its constructor is called.

*/
#ifndef __LATRANSFORM_H
#define __LATRANSFORM_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Secondo_Include.h"
class LATransform
{
public:
/*
This is the constructor. It gets a linear movement (x,y), the middle of a circle (xm, ym) and an angle alpha. 
This is possible because another class knows t and therefore does already know alpha, x and y. 
Later on, the methods of this class can be called for various points, without the already given information. 

*/
  LATransform (double x, double y, double _xm, double _ym, double alpha);
/*
This is the standard constructor.

*/
    LATransform ():a00 (1), a01 (0), a10 (0), a11 (1), cx (0), cy (0)
  {
  };
/*
This is the standard destructor.

*/
  ~LATransform ();
/*
This method calculates the new x value that the given point will get after its movement.

*/
  double getImgX (double x, double y);
/*
This method calculates the new y value that the given point will get after its movement.

*/
  double getImgY (double x, double y);
/*
This method calculates the original x value that the given point would have before its movement.
It undoes the movement, that is caused from LATransform.

*/
  double getOrigX (double x, double y);
/*
This method calculates the original y value that the given point would have before its movement.
It undoes the movement, that is caused from LATransform.

*/
  double getOrigY (double x, double y);
/*
This method returns the turning points x value.

*/
  double getXM ();
/*
This method returns the turning points y value.

*/
  double getYM ();

  void print() {
    printf("(%f, %f, %f, %f) - (%f, %f) - (%f, %f) - (%f, %f)\n", 
          a00, a01, a10, a11, cx, cy, dx, dy, xm, ym);
  }

  void Set(double x, double y, double _xm, double _ym, double alpha);

private:
  double a00, a01, a10, a11;//matrix values
  double cx, cy;//movement (D-M*D+W) look up in constructor
  double xm, ym;//rotational center
  double dx, dy;// inverse movement -(M'*(D+W)+D) look up in constructor
};
#endif
