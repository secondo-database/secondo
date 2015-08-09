/*
This class is a FixedMRegionIntepolator who creates a FixedMRegion of sights.

*/
using namespace std;
#include "FMRInterpolator.h"

/*
This is a constructor who expects a reference Region and the rotational center.

*/
FMRInterpolator::FMRInterpolator(const Region* _refRegion,
const Point* _rotCenter): result(0), refRegion(0), isValid(false) {
  if(_refRegion!=NULL){
    setReferenceRegion(*_refRegion, *_rotCenter);
  }else{
    refRegion.SetDefined(false);
  }
}

/*
This method signals the start. Please use one or more addObservation calls after 
this start method and finish with end.

*/
void FMRInterpolator::start() {
  observations.clear();
  isValid=true;
}

/*
This method signals the end. Please use start and one or more addObservation calls 
before calling this end method to finish.

*/
void FMRInterpolator::end() {
  if ((observations.size()<2) || (!isValid)) {
    result=FixedMRegion(0);
    return;
  }
  std::sort(observations.begin(), observations.end());
  calcFinalAngles();
  MMove mym(createMMove());
  result=FixedMRegion(refRegion, mym,
    Point(true, 0.0, 0.0), observations[0].getTime().ToDouble());
}

/*
This method adds an Observation. Please use start and one or more addObservation calls 
and the end method to finish.

*/
void FMRInterpolator::addObservation(const IRegion& obs) {
  ListExpr n=OutIntime<Region, OutRegion>(nl->TheEmptyList(), 
  SetWord(Address(&obs)));
  cout <<"IRegion:\n" << nl->ToString(n) << "\n";
  const Region& reg=obs.value;
  const Instant& ins=obs.instant;
  Point translation=calcMasspoint(getSortedList(reg));
  if (observations.size()==0) {
    if (!refRegion.IsDefined()) {
      setReferenceRegion(reg, translation);
    }
  }
  double angle=calcThisAngle(reg);
  LATransform l(0.0, 0.0, 0.0, 0.0, angle);
  Point mp=calcMasspoint(getSortedList(refRegion));
  Point corr(true, l.getImgX(mp.GetX(), mp.GetY()),
                   l.getImgY(mp.GetX(), mp.GetY()));
  vector<PointStore> tmp=calcRefVector(reg);
  bool obsvalid=checkRegion(tmp, angle);
  printf("Observation added, result: %s\n", (obsvalid)?"Gülig":"Ungültig");
  if (obsvalid) 
    observations.push_back(FMRObservation(translation-corr, angle, ins));
}
 
/*
This method returns the result, a FixedMRegion.

*/ 
FixedMRegion FMRInterpolator::getResult() {
  return result;
}

/*
This method calculates the mass point of the given points in the 
vector of HalfSegments.

*/
Point FMRInterpolator::calcMasspoint(const vector<HalfSegment>& a) const{
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
Point FMRInterpolator::calcMasspoint(const vector<Point> & a) const{
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
This method calculates a list of all regionpoints, sorted bei their angle and
distance to the masspoint.

*/
vector<PointStore> FMRInterpolator::calcRefVector(const Region& r) const {
  vector<PointStore> res(0);
  vector<HalfSegment> a = FixedMRegion::getHSFromRegion(r);
  vector<Point> tmp(0);
  createPointList(a, tmp);
  Point mass = calcMasspoint(a);
  for (size_t i=0; i<tmp.size(); i++) {
    res.push_back(PointStore(tmp[i], mass));
  }
  std::sort(tmp.begin(), tmp.end());
  return res;
}

/*
This method will return the sum of the angles alpha and beta and take care of 
over- and underflows. The result will be between -M\_PI and +M\_PI.

*/
double FMRInterpolator::addAngles(double alpha, double beta) const {
  double tmp=alpha+beta;
  while (tmp>=M_PI)
    tmp-=2*M_PI;
  while (tmp<-M_PI)
    tmp+=2*M_PI;
  return tmp;
}

/*
This method will compare two angles and take care of the limits 
of -M\_PI and +M\_PI.

*/
bool FMRInterpolator::AlmostEqualAngles(double alpha, double beta) const {
  double tmp=addAngles(alpha, -beta);
  return AlmostEqual(tmp, 0);
}

/*
This method will check, if the observation of the region equals the reference region.

*/
bool FMRInterpolator::checkRegion(const vector<PointStore> & v, 
                                  double alpha) const {
  //Fall 1: Anzahl Punkte nicht identisch
  if (v.size()!=refVector.size()) {
    return false;
  }
  
  //Verschiebung finden
  unsigned int i=0;
  double lDist=refVector[0].getDist();
  double lAlpha=addAngles(refVector[0].getAlpha(), alpha);
  while (((!AlmostEqual(v[i].getDist(),lDist)) || 
         (!AlmostEqualAngles(v[i].getAlpha(),lAlpha))) &&
         (i<v.size()))
    i++;
  //Fall 2: Verschiebung nicht gefunden
  if (i==v.size()) {
    return false;
  }
  //Fall 3: Vektorvergleich
  for (unsigned int j=0; j<v.size(); j++) {
    //Fall 3a: Längen stimmen nicht
    if (!AlmostEqual(refVector[j].getDist(), v[i].getDist())) {
      return false;
    }
    //Fall 3b: Winkel stimmen nicht
    if (!AlmostEqualAngles(refVector[j].getAlpha()+alpha, v[i].getAlpha())) {
      return false;
    }
    i=(i+1)%v.size();
  }
  return true;
}
/*
This method sets the given object as a reference. The zero point will be the 
calculated mass point, not the given rotational center.

*/
void FMRInterpolator::setReferenceRegion(const Region &r, 
const Point &masspoint){
  refRegion.Clear();
  r.Translate(-masspoint.GetX(), -masspoint.GetY(), refRegion);
  determineAngleAlgorithm();
  angle_init=calcThisAngle(refRegion);
  refVector=calcRefVector(refRegion);
}

/*
This method calculates the distance between the two points.

*/
double FMRInterpolator::getDist(const Point &a, const Point &b) const {
  return a.Distance(b);
}

/*
This method calculates the point of the region r, that has got the maximum 
distance from masspoint. If this does not exist, it will return the point 
with false.

*/
Point FMRInterpolator::calcMaxDistPoint(const Region &r, 
const Point &masspoint) const{
  vector<HalfSegment> a = FixedMRegion::getHSFromRegion(r);
  HalfSegment hs;
  hs = a[0];
  double maxDist = getDist(hs.GetLeftPoint(), masspoint);
  Point max(0);
  max = hs.GetLeftPoint();
  int mult_max = 0;
  for(size_t i = 0; i < a.size(); i++){
    hs = a[i];
    Point lp(hs.GetLeftPoint());
    double distlp = getDist(lp, masspoint);
    Point rp(hs.GetRightPoint());
    double distrp = getDist(rp, masspoint);
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
This method calculates the point of the region r, that has got the minimum 
distance from masspoint. If this does not exist, it will return the point 
with false.

*/
Point FMRInterpolator::calcMinDistPoint(const Region &r, 
const Point & masspoint) const{
  vector<HalfSegment> a = FixedMRegion::getHSFromRegion(r);
  HalfSegment hs;
  hs = a[0];
  double minDist = getDist(hs.GetLeftPoint(), masspoint);
  Point min(0);
  min = hs.GetLeftPoint();
  int mult_min = 0;
  for(size_t i = 0; i < a.size(); i++){
    hs = a[i];
    Point lp(hs.GetLeftPoint());
    double distlp = getDist(lp, masspoint);
    Point rp(hs.GetRightPoint());
    double distrp = getDist(rp, masspoint);
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
bool FMRInterpolator::inList(const vector<Point> &list, const Point &p) const{
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
void FMRInterpolator::createPointList(const vector<HalfSegment>& a, 
  vector<Point>& result) const{
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
void FMRInterpolator::sortList(vector<Point>& list, const Point& ref) const {
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
vector<Point> FMRInterpolator::getSortedList(const Region &re) const{
  vector<HalfSegment> a = FixedMRegion::getHSFromRegion(re);
  vector<Point> tmp(0);
  createPointList(a, tmp);
  Point mass = calcMasspoint(a);
  sortList(tmp, mass);
  return tmp;
}

/*
This method calculates the vector of distances to the masspoint.

*/
vector<double> FMRInterpolator::calcDistVector(const Region &r, 
const Point& masspoint) const{
  vector<Point> a = getSortedList(r);
  vector<double> res(a.size());
  for(size_t i = 0; i < a.size(); i++){
    res[i] = getDist(a[i], masspoint);
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
masspoint. It permutates the vector until it finds a solution that equals 
distVector and it will return the first point of it.

*/
Point FMRInterpolator::matchVectors(const Region &r, 
const Point &masspoint) const{
  vector<double> dist2 = calcDistVector(r, masspoint);
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
int FMRInterpolator::determineAngleAlgorithm() {
  Point m = calcMasspoint(getSortedList(refRegion));
  Point  p = calcMaxDistPoint(refRegion, m);
  if(p.IsDefined())
    return (angle_method=1);
  p = calcMinDistPoint(refRegion, m);
  if ((p.IsDefined() && (!AlmostEqual(getDist(p, m), 0))))
    return (angle_method=2);
  distVector=calcDistVector(refRegion, m);
  return (angle_method=3);
}

/*
This method will calculate the angle of the given region with the chosen 
method.

*/
double FMRInterpolator::calcThisAngle(const Region &r) const{
  double d = 0;
  Point m = calcMasspoint(getSortedList(r));
  Point p(0);
  switch(angle_method){
    case 1:
      p = calcMaxDistPoint(r, m);
      break;
    case 2:
      p = calcMinDistPoint(r, m);
      break;
    case 3:
      p = matchVectors(r, m);
      break;
    default:
      printf("Error non standard case");
      break;
  }
  d = calcAngle(p.GetX()-m.GetX(), p.GetY()-m.GetY());
  d -= angle_init;
  while (d>=M_PI)
    d-=2*M_PI;
  while (d<-M_PI)
    d+=2*M_PI;
  return d;
}

/*
This method calculates the final angles. the result will always be the angle
with the shortest path from the angle before to this one.

*/
void FMRInterpolator::calcFinalAngles(){
  double st = observations[0].getAngle();
  for(unsigned int i = 1; i<observations.size(); i++){
    double tmp=observations[i].getAngle();
    double a=observations[i].getAngle();
    if (getTurnDir(st, a)) {
      while (a<observations[i-1].getAngle())
        a+=2*M_PI;
    }else{
      while (a>observations[i-1].getAngle())
        a-=2*M_PI;
    }
    observations[i].setAngle(a);
    st=tmp;
  }
}

/*
This method creates all UMoves and puts them into a MMove that will be returned.

*/
MMove FMRInterpolator::createMMove() const{
  MMove res(0);
  res.Clear ();
  res.StartBulkLoad ();
  const vector<FMRObservation>& obs=observations;
  for(unsigned int i = 1; i<obs.size(); i++){
    Interval < Instant > iv (obs[i-1].getTime(), obs[i].getTime(), false, true);
    Point p1(obs[i-1].getTranslation());
    Point p2(obs[i].getTranslation());
    UMove u01param = UMove(iv, p1.GetX(), p1.GetY(), obs[i-1].getAngle(),
      p2.GetX(), p2.GetY(), obs[i].getAngle());
    res.MergeAdd (u01param);
  }
  res.EndBulkLoad ();
  return res;
}
