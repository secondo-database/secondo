/*
This my test methd.

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

class PointStore {
public:
  inline PointStore(const Point& _p, const Point & ref): p(_p) {
    Point tmp=_p-ref;
    alpha=calcAngle(tmp.GetX(), tmp.GetY());
    dist=sqrt(tmp.GetX()*tmp.GetX()+tmp.GetY()*tmp.GetY());
  }
  
  inline PointStore(const PointStore& o): p(o.p), alpha(o.alpha), dist(o.dist){}
  
  inline PointStore(): p(false, 0.0, 0.0), alpha(0), dist(0) {}
  
  inline bool operator<(const PointStore& o) const {
    if (alpha<o.alpha)
      return true;
    if (alpha>o.alpha)
      return false;
    return (dist<o.dist);
  }

  inline bool operator>(const PointStore& o) const {
    if (alpha>o.alpha)
      return true;
    if (alpha<o.alpha)
      return false;
    return (dist>o.dist);
  }
  
  inline bool operator==(const PointStore& o) const {
    return ((alpha==o.alpha) && (dist==o.dist));
  }
  
  inline Point getPoint() const { return p;}
private:
  Point p;
  double alpha;
  double dist;
};

#endif