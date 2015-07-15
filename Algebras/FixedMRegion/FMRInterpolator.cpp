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
Point FMRInterpolator::calcMasspoint(const vector<HalfSegment>& a){
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
This method calculates the mass point of the given points.

*/
Point FMRInterpolator::calcMasspoint(vector<Point> a){
  double x = 0.0;
  double y = 0.0;
  int s = 0;
  for(size_t i = 0; i < a.size(); i++){
    const Point lp = a[i];
    x += lp.GetX();
    y += lp.GetY();
    s++;
  }
  x=x/s;
  y=y/s;
  Point res(true, x, y);
  return res;
}
/*
This method sets the given object as a reference. The zero point will be the 
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
This method sets the given object.

*/
void FMRInterpolator::setRotcenter(Point _rotpoint){
  rotcenter = Point(_rotpoint);
}
/*
This method returns the masspoint.

*/
Point FMRInterpolator::getRotcenter(){
  return Point(rotcenter);
}
/*  
This method sets the angle of the refRegion for internal use.

*/
void FMRInterpolator::setDistVector(vector<double> a){
  distVector = a;
}
/*  
This method returns the angle of the refRegion for internal use.

*/
vector<double> FMRInterpolator::getDistVector(){
  return vector<double>(distVector);
}
/*  
This method sets the angle of the refRegion for internal use.

*/
void FMRInterpolator::setAngleInit(double a){
 angle_init = a;
}
/*  
This method returns the angle of the refRegion for internal use.

*/
double FMRInterpolator::getAngleInit(){
  return angle_init;
}
/*
This method calculates the distance between the two points.

*/
double FMRInterpolator::getDist(Point a, Point b){
  return a.Distance(b);
}
/*
This method calculates the point, that has got the maximum distance from
\_calcMasspoint. If this does not exist, it will return the point with false.

*/
Point FMRInterpolator::calcMaxDistPoint(Region r, Point calcMasspoint){
  FixedMRegion fmr;
  vector<HalfSegment> a = fmr.getHSFromRegion(r);
  HalfSegment hs;
  hs = a[0];
  double maxDist = getDist(hs.GetLeftPoint(), calcMasspoint);
  Point max(0);
  max = hs.GetLeftPoint();
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
  Point res(false, -9999, -9999);
  return res;
}
/*
This method calculates the point, that has got the minimum distance from
\_calcMasspoint. If this does not exist, it will return the point with false.

*/
Point FMRInterpolator::calcMinDistPoint(Region r, Point calcMasspoint){
  FixedMRegion fmr;
  vector<HalfSegment> a = fmr.getHSFromRegion(r);
  HalfSegment hs;
  hs = a[0];
  double minDist = getDist(hs.GetLeftPoint(), calcMasspoint);
  Point min(0);
  min = hs.GetLeftPoint();
  int mult_min = 0;
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
  }
  if(mult_min == 0){
    return min;
  }
  Point res(false, -9999, -9999);
  return res;
}
/*
This method checks, if the given point is already in the list.

*/
bool FMRInterpolator::inList(vector<Point> list, Point p){
  for(size_t i = 0; i < list.size(); i++){
    if(list[i] == p){
      return true;
    }
  }
  return false;
}
/*
This method creates a list of all region points.

*/
void FMRInterpolator::createPointList(vector<HalfSegment> a, 
  vector<Point>& result){
  for(size_t i = 0; i < a.size(); i++){
    Point dp(true, a[i].GetLeftPoint().GetX(), a[i].GetLeftPoint().GetY());
    if(!(inList(result, dp))){
      result.push_back(dp);
    }
    Point rp(true, a[i].GetRightPoint().GetX(), a[i].GetRightPoint().GetY());
    if(!(inList(result, rp))){
      result.push_back(rp);
    }
  }
}
/*
This method sorts the points of the given list clockwise and according to the 
points distance to the given point ref.

*/
void FMRInterpolator::sortList(vector<Point>& list, const Point& ref) {
  vector<PointStore> tmp(0);
  for (size_t i=0; i<list.size(); i++) {
    tmp.push_back(PointStore(list[i], ref));
  }
  std::sort(tmp.begin(), tmp.end());
  for (size_t i=0; i<list.size(); i++) {
    list[i]=tmp[i].getPoint();
  }
}
/*
This method creates a clockwise sorted list of region points.

*/
vector<Point> FMRInterpolator::getSortedList(Region re){
  FixedMRegion fmr;
  vector<HalfSegment> a = fmr.getHSFromRegion(re);
  vector<Point> tmp(0);
  createPointList(a, tmp);
  Point mass = calcMasspoint(a);
  sortList(tmp, mass);
  return tmp;
}
/*
This method calculates the vector of distances to the masspoint.

*/
vector<double> FMRInterpolator::calcDistVector(Region r, Point calcMasspoint){
  //printf("huhu\n");
  vector<Point> a = getSortedList(r);
  vector<double> res(a.size());
  for(size_t i = 0; i < a.size(); i++){
    //Point lp(a[i].GetLeftPoint());
    res[i] = getDist(a[i], calcMasspoint);
  }
  return res;
}
/*
This method compares the values of the vector $d1$ with those of $d2$ at position 
plus offset.

*/
bool matchAtPosition(const vector<double>& d1, const vector<double>& d2, 
  size_t offset) {
  size_t module=d1.size();
  for (size_t i=0; i<module; i++) {
    if (!AlmostEqual(d1[i], d2[(i+offset)%module]))
      return false;
  }
  return true;
}
/*
This method calculates the distance vector for all points of r to
calcMasspoint. It permutates the vector until it finds a solution that equals 
distVector and it will return the first point of it.

*/
Point FMRInterpolator::matchVectors(vector<double> distVector, 
  Region r, Point calcMasspoint){
  vector<double> dist2 = calcDistVector(r, calcMasspoint);
  vector<Point> a = getSortedList(r);

  if (matchAtPosition(distVector, dist2, 0))
    return a[0];
  size_t numsteps=(distVector.size()+1)/2;
  for (size_t i=1; i<numsteps; i++) {
    if (matchAtPosition(distVector, dist2, i))
      return a[i];
    if (matchAtPosition(distVector, dist2, distVector.size()-i))
      return a[distVector.size()-i];
  }
  return Point(false, 0, 0);
}
/*
This method calculates the orientation between two angles. It will give back
true, if the shortest movement is positiv and else false.

*/
bool FMRInterpolator::getTurnDir(double a1, double a2) const {
  double a = a2-a1;
  printf("%f\n", a);
  if (a > M_PI)
    return false;
  if (a < -M_PI)
    return true;
  return (a > 0);
}
/*
This method finds out which method will identify a special point of th region.
It returns the following values: $1$ maximum, $2$ minimum, $3$ vector.

*/
int FMRInterpolator::determineAngleAlgorithm(){
  Region r = getReferenceRegion();
  Point m = calcMasspoint(getSortedList(r));//getRotcenter();
  Point  p = calcMaxDistPoint(r, m);
  if(p.IsDefined())
    return 1;
  p = calcMinDistPoint(r, m);
  if(p.IsDefined())
    return 2;
  setDistVector(calcDistVector(r, m));
  return 3;
}
/*
This method will calculate the angle of the given region with the chosen 
method.

*/
double FMRInterpolator::calcThisAngle(Region r, int method){
  double d = 0;
  Point m = calcMasspoint(getSortedList(r));
  Point p(0);
  switch(method){
    case 1:
      p = calcMaxDistPoint(r, m);
      break;
    case 2:
      p = calcMinDistPoint(r, m);
      break;
    case 3:
      p = matchVectors(getDistVector(), r, m);
      break;
    default:
      printf("Error non standard case");
      break;
  }
  d = calcAngle(p.GetX()-m.GetX(), p.GetY()-m.GetY());
  d -= getAngleInit();
  while (d>=M_PI)
    d-=2*M_PI;
  while (d<-M_PI)
    d+=2*M_PI;
  return d;
}
/*
This method calculates the angles for all given objects and returns them in 
the angle vector.

*/
void FMRInterpolator::calcAngles(vector<Region> rs, int method, 
   vector<double> &angles){
   //FIXME: Quick& dirty
   while (angles.size()<rs.size()) 
     angles.push_back(0.0);
     
  for(unsigned int i = 0; i<rs.size(); i++){
    angles[i] = calcThisAngle(rs[i], method);
  }
}
/*
This method calculates the translation for all given objects and returns them in 
the translation vector.

*/
void FMRInterpolator::calcTranslations(vector<Region> rs, 
  vector<Point> &translations){
   //FIXME: Quick& dirty
   while (translations.size()<rs.size()) 
     translations.push_back(Point(0));
  for(unsigned int i = 0; i<rs.size(); i++){
    translations[i] = calcMasspoint(getSortedList(rs[i]));
  }
}
/*
This method calculates the translation from the rotational center for all 
given objects and returns them in the final translation vector.

*/
void FMRInterpolator::calcTranslationFromRotcenter(vector<Point>& translations,
  Point rotcenter, vector<Point> &final_translations){
//FIXME
  for (int i=0; i<translations.size(); i++)
    final_translations.push_back(translations[i]);
    
}
/*
This method calculates the final angles. the result will always be the angle
with the shortest path from the angle before to this one.

*/
void FMRInterpolator::calcFinalAngles(vector<double> &angles){
  double st = angles[0];
  for(unsigned int i = 1; i<angles.size(); i++){
    double tmp=angles[i];
    if (getTurnDir(st, angles[i])) {
      while (angles[i]<angles[i-1])
        angles[i]+=2*M_PI;
    } else {
      while (angles[i]>angles[i-1])
        angles[i]-=2*M_PI;
    }
    st=tmp;
  }
}
/*
This method creates all UMoves and puts them into a MMove that will be returned.

*/
MMove FMRInterpolator::createMMove(const vector<DateTime>&times, 
vector<Point> fin, vector<double> angles){
  assert((times.size()==fin.size()) && (times.size()==angles.size()));
  MMove res(0);
  res.Clear ();
  res.StartBulkLoad ();
  for(unsigned int i = 1; i<angles.size(); i++){
    Interval < Instant > iv (times[i-1], times[i], false, true);
    UMove u01param = UMove(iv, fin[i-1].GetX(), fin[i-1].GetY(), angles[i-1],
      fin[i].GetX(), fin[i].GetY(), angles[i]);
    printf("%d: (%f, %f, %f) - (%f, %f, %f)\n", i,
           fin[i-1].GetX(), fin[i-1].GetY(), angles[i-1],
           fin[i].GetX(), fin[i].GetY(), angles[i]);
    res.MergeAdd (u01param);
  }
  res.EndBulkLoad ();
  return res;
}
/*
This method creates the FixedMRegion from the sights.

*/
FixedMRegion FMRInterpolator::interpolatetest(const vector<Region>& regions, 
  const vector<DateTime>& times, const Region* ref_Region, 
  const Point* start_Point){
  FixedMRegion result;
//  MMove res_move;
  
  Point m1 = calcMasspoint(getSortedList(regions[0]));
  if (start_Point==NULL) {
    setRotcenter(m1);
  } else {
    setRotcenter(*start_Point);
  }

  if (ref_Region!=NULL) {
    setReferenceRegion(*ref_Region, getRotcenter());
  } else {
    setReferenceRegion(regions[0], getRotcenter());
  }
  
  double caseangle = determineAngleAlgorithm();
  setAngleInit(calcThisAngle(getReferenceRegion(), caseangle));
  vector<double> angles;
  calcAngles(regions, caseangle, angles);
  vector<Point> translations;
  calcTranslations(regions, translations);
  vector<Point> final_translations;
  calcTranslationFromRotcenter(translations, getRotcenter(), 
    final_translations);
  calcFinalAngles(angles);
  MMove mym(createMMove(times, final_translations, angles));
  FixedMRegion fmr(getReferenceRegion(), mym,
    Point(true, 0.0, 0.0), times[0].ToDouble());
  return fmr;
}

FixedMRegion FMRInterpolator::interpolate(const std::vector<IRegion> &spots){
  vector<Region> regions(0);
  vector<DateTime> times(0);
  for (int i=0; i<spots.size(); i++) {
    regions.push_back(spots[i].value);
    times.push_back(spots[i].instant);
  }
  return interpolatetest(regions, times);
}