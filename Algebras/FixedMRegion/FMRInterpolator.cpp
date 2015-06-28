/*
This class is a FixedMRegion.

*/
using namespace std;
#include "FMRInterpolator.h"
//#define DEBUG_VERBOSE
//#define DEBUG_VERBOSE2

FMRInterpolator::FMRInterpolator(): refRegion(0) {}

/*
This method calculates the mass point of the given points.


*/

Point FMRInterpolator::calcMasspoint(vector<HalfSegment> a){
  double x = 0.0;
  double y = 0.0;
  int s = 0;
  for(size_t i = 0; i < a.size(); i++){
    HalfSegment hs;
    hs=a[i];
    const Point lp = hs.GetLeftPoint();
    x += lp.GetX();
    y += lp.GetY();
    s++;
    const Point rp = hs.GetRightPoint();
    x += rp.GetX();
    y += rp.GetY();
    s++;
  }
  x=x/s;
  y=y/s;
  Point res(true, x, y);
  return res;
}

/*
This method sets the given object as a reference. The (0,0) will be the 
calculated, not the given mass point.

*/
void FMRInterpolator::setReferenceRegion(Region _r, Point _calcMasspoint){
  refRegion.Clear();
  _r.Translate(-_calcMasspoint.GetX(), -_calcMasspoint.GetY(), refRegion);
}
/*
This method returns the reference region.

*/
Region FMRInterpolator::getReferenceRegion(){
  return Region(refRegion);
}
/*
This method calculates the orientation vector of the given Region and uses the
given, calculated mass point as the central point.

*/
//void FMRInterpolator::calcOrientationVector(Region _r, Point _calcMasspoint){
  
//}
/*
This method calculates the distance between the two points.

*/
double FMRInterpolator::getDist(Point a, Point b){
  return a.Distance(b);
}
/*
This method calculates the point, that has got the maximum distance from
\_calcMasspoint. If this does not exist, it will return the point with 
the minimum distance from \_calcMasspoint.

*/
Point FMRInterpolator::calcMaxMinDistPoint(Region r, 
  Point calcMasspoint){
  FixedMRegion fmr;
  vector<HalfSegment> a = fmr.getHSFromRegion(r);
  HalfSegment hs;
  hs = a[0];
  double minDist = getDist(hs.GetLeftPoint(), calcMasspoint);
  double maxDist = getDist(hs.GetLeftPoint(), calcMasspoint);
  Point max(0);
  max = hs.GetLeftPoint();
  Point min(0);
  min = hs.GetLeftPoint();
  int mult_min = 0;
  int mult_max = 0;
  for(size_t i = 0; i < a.size(); i++){
    hs = a[i];
    Point lp(hs.GetLeftPoint());
    double distlp = getDist(lp, calcMasspoint);
    Point rp(hs.GetRightPoint());
    double distrp = getDist(rp, calcMasspoint);
    if (distlp>distrp) {
      Point tmp=lp;
      lp=rp;
      rp=tmp;
      double tmp2=distlp;
      distlp=distrp;
      distrp=tmp2;
    }
    if(minDist > distlp){
      minDist = distlp;
      min = lp;
      mult_min = 0;
    }
    if(AlmostEqual(minDist, distlp)){
      double tmp = getDist(lp, min);
      if(!AlmostEqual(tmp,0)){
        mult_min++;
      }
    }
    if(maxDist < distrp){
      maxDist = distrp;
      max = rp;
      mult_max = 0;
    }
    if(AlmostEqual(maxDist,distrp)){
      double tmp = getDist(rp, max);
      if(!AlmostEqual(tmp,0)){
        mult_max++;
      }
    }
  }
  if(mult_max == 0){
    return max;
  }
  if(mult_min == 0){
    return min;
  }
  Point res(false, -9999, -9999);
  return res;
}
/*
This method calculates the vector of distances to the masspoint.

*/
vector<double> calcDistVector(Region r, Point masspoint){
  return vector<double> (0);
}
/*
This method calculates the distance vector for all points of \_r to
\_calcMasspoint. It permutates the vector until it finds a solution that equals 
distVector and it will return the first point of it.

*/
Point FMRInterpolator::matchVectors(
  vector<double> distVector, Region r, Point calcMasspoint){
  return Point(0);
}
/*
This method calculates the angle between the given region and the x-axis.

*/
double FMRInterpolator::calculateAngleToXAxis(Region r, 
  Point calcMasspoint){
  return 0.0;
}
