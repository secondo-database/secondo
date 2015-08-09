/*
This class can store and compare Points.

*/
#ifndef __POINTSTORE_H
#define __POINTSTORE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "Secondo_Include.h"

/*
This method calculates the angle between (1,0) - (0,0) - (x,y) and returns
the angle -pi<=alpha<pi, with a positiv orientation of the angle.

*/
inline double calcAngle(double x, double y) {
  double h=sqrt(x*x+y*y);
  if (AlmostEqual(h, 0.0)) 
    return 0;
  double a = asin(y/h);
  if (x < 0) {
    if (a > 0) {
      a = M_PI-a;
    } else {
      a = -M_PI-a;
    }
  }
  return a;
}

/*
This class can store and compare Points.

*/
class PointStore {
public:
/*
This is the constructor and expects a point and a reference point to which the 
distance will be calculated.

*/
  inline PointStore(const Point& _p, const Point & ref): p(_p) {
    Point tmp=_p-ref;
    alpha=calcAngle(tmp.GetX(), tmp.GetY());
    dist=sqrt(tmp.GetX()*tmp.GetX()+tmp.GetY()*tmp.GetY());
  }
/*
This is the copy constructor.

*/  
  inline PointStore(const PointStore& o): p(o.p), alpha(o.alpha), dist(o.dist){}
/*
This is the default constructor.

*/    
  inline PointStore(): p(false, 0.0, 0.0), alpha(0), dist(0) {}
/*
This is the less operator.

*/
  inline bool operator<(const PointStore& o) const {
    if (alpha<o.alpha)
      return true;
    if (alpha>o.alpha)
      return false;
    return (dist<o.dist);
  }
/*
This is the bigger operator.

*/
  inline bool operator>(const PointStore& o) const {
    if (alpha>o.alpha)
      return true;
    if (alpha<o.alpha)
      return false;
    return (dist>o.dist);
  }
/*
This is the equality operator.

*/
  inline bool operator==(const PointStore& o) const {
    return ((alpha==o.alpha) && (dist==o.dist));
  }
/*
This method returns the point..

*/
  inline Point getPoint() const { return p;}
  inline double getDist() const { return dist;}
  inline double getAlpha() const { return alpha;}
private:
  Point p;
  double alpha;
  double dist;
};

#endif