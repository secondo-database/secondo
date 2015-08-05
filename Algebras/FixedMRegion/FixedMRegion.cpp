/*
This class is a FixedMRegion.

*/
using namespace std;
#include "FixedMRegion.h"

/*
This is the default constructor. Do not use.

*/
FixedMRegion::FixedMRegion(): Attribute(false), m(0), r(0){}

/*
This is the copy constructor.

*/
FixedMRegion::FixedMRegion(const FixedMRegion & f):
Attribute(true),t(f.t),m(f.m),r(f.r),
l(f.l){}

/*
This method creates a MMove with the given values.

*/ 
MMove FixedMRegion::createMMove(const double _startX, const double _startY, 
const double _startangle, const Instant _startt, const double _endX, 
const double _endY, const double _endangle, const Instant _endt) const{
  Interval < Instant > iv (_startt, _endt, false, true);
  UMove u01param = UMove(iv, _startX, _startY, _startangle, _endX, _endY,
    _endangle);
  MMove res(0);
  res.Clear ();
  res.StartBulkLoad ();
  res.MergeAdd (u01param);
  res.EndBulkLoad ();
  return res;
}

/*
This method creates a MMove with the given values.

*/ 
MMove FixedMRegion::createMMove(const double _startX, const double _startY, 
const double _startangle, const double _endX, const double _endY, 
const double _endangle) const{
  DateTime t1 (0.0);
  DateTime t2 (1.0);
  return createMMove(_startX, _startY, _startangle, t1,
    _endX, _endY, _endangle, t2);
}

/*
This is the constructor that gets necessary information. 
\_region: object that shall be moved
\_startp: start point at starttime for the movement
\_startangle: start angle at starttime for the movement
\_startt: start time
\_endp: end point at endtime for the movement
\_endangle: end angle at endtime for the movement
\_endt: end time 
rot\_center: rotational center

*/
FixedMRegion::FixedMRegion(const Region & _region, const Point _startp, 
  const double _startangle, const Instant _startt, const Point _endp, 
  const double _endangle, const Instant _endt, const Point & rot_center):
  Attribute(true), r(_region){
  m = createMMove(_startp.GetX(), _startp.GetY(), _startangle, _startt, 
    _endp.GetX(), _endp.GetY(), _endangle, _endt);
  t = _startt.ToDouble();
  xm = rot_center.GetX ();
  ym = rot_center.GetY ();
  calculateInternalVars ();
}

/*
This is the constructor that gets necessary information. 
\_region: object that shall be moved
\_startp: start point at starttime for the movement
\_startangle: start angle at starttime for the movement
\_startt: start time
\_endp: end point at endtime for the movement
\_endangle: end angle at endtime for the movement
\_endt: end time
rot\_center: rotational center

*/
FixedMRegion::FixedMRegion(const Region & _region, const Point _startp, 
  const double _startangle, const double _startt, const Point _endp, 
  const double _endangle, const double _endt, const Point & rot_center):
  Attribute(true), r(_region){
  m = createMMove(_startp.GetX(), _startp.GetY(), _startangle, 
  _startt, _endp.GetX(), _endp.GetY(), _endangle, _endt);
  t = _startt;
  xm = rot_center.GetX ();
  ym = rot_center.GetY ();
  calculateInternalVars ();
}

/*
This is the constructor that gets necessary information. 
\_region: object that shall be moved
\_move: the move object
rot\_center: the center of the rotation. It will be moved with the object.
\_starttime: the start time of the movement

*/
FixedMRegion::FixedMRegion(const Region & _region, const MMove & _move,
const Point & rot_center, const double _starttime):
Attribute(true), m(_move), r(_region){
  t = _starttime;
  xm = rot_center.GetX ();
  ym = rot_center.GetY ();
  calculateInternalVars ();
}

/*
deprecated
This is the constructor that should not be used externally. It gets necessary information. 
$\_t$: never used
$\_xm$: x-value of the rotation center, that will be moved, too
$\_ym$: y-value of the rotation center, that will be moved, too
$\_r$: the region to be moved
$\_x0$: the x-startvalue of the linear movement
$\_y0$: the y-startvalue of the linear movement
$\_alpha0$: the starting angle
$\_vx$: x-value of the linear movement
$\_vy$: y-value of the linear movement
$\_valpha$: the angle of movement.

*/
FixedMRegion::FixedMRegion (const double _t, const double _xm, 
const double _ym, const Region & _r, const double _x0, const double _y0,
const double _alpha0, const double _vx, const double _vy, 
const double _valpha):
Attribute(true), m(0), r(_r){
  t = _t;
  xm = _xm;
  ym = _ym;
  m = createMMove (_x0, _y0, _alpha0, _vx, _vy, _valpha);
  calculateInternalVars ();
}

/*
This is the constructor that should be used externally. It gets necessary information.
\_region: object that shall be moved
\_start: point at which the object ist placed at the beginning $t=0$
alpha\_start: angle of object at the bginning $t=0$
\_speed: speed or velociy (v) of the movement
alpha\_speed: speed or velociy (v) of the angle movement
rot\_center: the rotational center of the object
\_starttime: the start time of the movement

*/
FixedMRegion::FixedMRegion(const Region & _region, const Point & _start,
const double alpha_start, const Point & _speed, const double alpha_speed, 
const Point & rot_center, const double _starttime):
Attribute(true), r(_region){
  m = createMMove (_start.GetX (), _start.GetY (), alpha_start, _speed.GetX (),
    _speed.GetY (), alpha_speed);
  t = _starttime;
  xm = rot_center.GetX ();
  ym = rot_center.GetY ();
  calculateInternalVars ();
}

/*
This is the standard destructor.

*/
FixedMRegion::~FixedMRegion(){}

/*
This method will return the region that the FMRegion will have at the given 
time ti.

*/
void FixedMRegion::atinstant(const double ti, Region &result){
  sett (ti);
  result.Clear();
  Line tmp(r.Size());
  tmp.StartBulkLoad ();
  for (int i = 0; i < r.Size (); i++){
    HalfSegment hs;
    r.Get (i, hs);
    const Point lp = hs.GetLeftPoint ();
    double newx = l.getImgX (lp.GetX (), lp.GetY ());
    double newy = l.getImgY (lp.GetX (), lp.GetY ());
    Point newlp (true, newx, newy);
    const Point rp = hs.GetRightPoint ();
    newx = l.getImgX (rp.GetX (), rp.GetY ());
    newy = l.getImgY (rp.GetX (), rp.GetY ());
    Point newrp (true, newx, newy);
    HalfSegment hsnew(hs.IsLeftDomPoint (), newlp, newrp);
    hsnew.attr.edgeno = hs.attr.edgeno;
    tmp+=hsnew;
  }
  tmp.EndBulkLoad();
  tmp.Transform(result);
}

/*
This method will return a Point3 with the moving values x, y and alpha for the 
given double with time t.

*/
Point3 FixedMRegion::getMovingForTimeFromMMove(const double t) const{
  DateTime t2(t);
  return getMovingForTimeFromMMove(t2);
}

/*
This method will return a Point3 with the moving values x, y and alpha for the given 
DateTime t.

*/
Point3 FixedMRegion::getMovingForTimeFromMMove (const DateTime t) const{
  Intime < Point3 > tp;
  m.AtInstant (t, tp);
  return tp.value;
}

/*
This method returns the start time of the valid interval as an double
as absolute time.

*/
double FixedMRegion::getMMoveStart(const double dummy) const{
  //FIXME m nicht definiert oder 0 Komponenten
  UMove unit(0);
  m.Get (0, unit);
  Instant ita = unit.getTimeInterval ().start;
  return ita.ToDouble();
}

/*
This method uses the LATransform object to move the given Point inverse 
to the movement of the FixedMRegion for time t and returns it.

*/
void FixedMRegion::getInv(const double t, const Point & p, Point & result) 
const{
  Point3 coord = getMovingForTimeFromMMove(t);
  LATransform lt (coord.GetX(), coord.GetY(), xm, ym, coord.GetAlpha());
  double tmpx = lt.getOrigX (p.GetX (), p.GetY ());
  double tmpy = lt.getOrigY (p.GetX (), p.GetY ());
  result.SetDefined(true);
  result.Set(tmpx, tmpy);
} 

/*
This method calculates the step with depending on alpha.

*/
int FixedMRegion::calcStepWith(const double _ta, const double _te) const {
  Intime<Point3> p(0);
  m.AtInstant(_ta, p);
  Point3 p3 = p.value;
  double alpha_a = p3.GetAlpha();
  m.AtInstant(_te, p);
  p3 = p.value;
  double alpha_e = p3.GetAlpha();
  double tmp = (alpha_e - alpha_a);
  ///(_te-_ta);
  double kr = M_PI/16;
  if(tmp <= kr){
    return 1;
  }
  int steps=(int)ceil((tmp)/(kr));
  return steps;
}

/*
This method calculates the aprroximated movement of the given MPoint
unter the condition that the FixedMRegion does not move. Therefore, 
it uses an inverse movement of the Region and lets the MPoint move 
that way in addition to its own movement.

*/
MPoint FixedMRegion::approxMovement(const MPoint & p) const{
  UPoint unit;
  RefinementPartition<MPoint,MMove,UPoint,UMove> rp(p, m);
  MPoint res (0);
  res.StartBulkLoad ();
  for (unsigned int i = 0; i < rp.Size(); i++) {
    Interval<Instant> iv;
    int urPos;
    int upPos;
    rp.Get(i, iv, urPos, upPos);
    double ta = iv.start.ToDouble();
    double te = iv.end.ToDouble();
    int steps = calcStepWith(ta, te);
    DateTime to (ta);
    Intime < Point > tpo;
    p.AtInstant (to, tpo);
    Point po(0);
    getInv(ta, tpo.value, po);
    for (int i=1; i<=steps; i++) {
      double now=((te-ta)/steps)*i+ta;
      DateTime tn(now);
      Intime < Point > tp;
      p.AtInstant (tn, tp);
      Point pn(0);
      getInv(now, tp.value, pn);
      Interval < Instant > in (to, tn, true, false);
      UPoint up (in, po, pn);
      res.MergeAdd (up);
      to = tn;
      po = pn;
    }
  }
  res.EndBulkLoad();
  return res;
}

/*
This method creates a non moving MRegion.

*/
MRegion *FixedMRegion::buildMovingRegion(){
  MRegion *mr;
  Region r(0);
  atinstant (getMMoveStart(0.0), r);
  MPoint rock (0);
  rock.Clear ();
  rock.StartBulkLoad ();
  DateTime t1 (instanttype);
  t1.Set (0, 1, 1, 0, 0);
  DateTime t2 (instanttype);
  t2.Set (3999, 12, 31, 23, 59);
  Interval < Instant > iv (t1, t2, false, true);
  UPoint ub (iv, 0, 0, 0, 0);
  rock.MergeAdd (ub);
  rock.EndBulkLoad ();
  mr = new MRegion (rock, r);
  return mr;
}

/*
This method will join adjacent UBools, if they have got the same value.

*/
MBool FixedMRegion::joinBools(const MBool a) const{
  MBool res(0);
  res.Clear();
  if (a.GetNoComponents()==0){
    return res;
  }
  res.StartBulkLoad();
  bool start=true;
  int i=0;   
  UBool up;
  a.Get(i, up);
  DateTime t_start = up.getTimeInterval().start;
  DateTime t_end = up.getTimeInterval().end;
  CcBool val = up.constValue;
  i++;
  while(i<a.GetNoComponents()){
    a.Get(i, up);
    if((t_end == up.getTimeInterval().start) && (val == up.constValue)){
      t_end = up.getTimeInterval().end;
    }else{
      if (t_start!=t_end) {
        Interval < Instant > iv (t_start, t_end, start, true);
        UBool ub (iv, val);
        res.MergeAdd (ub);
      }
      start=false;
      t_start = up.getTimeInterval().start;
      t_end = up.getTimeInterval().end;
      val = up.constValue;
    }
    i++;
  }
  if (t_start!=t_end) {
    Interval < Instant > ive (t_start, t_end, start, true);
    UBool ube (ive, val);
    res.MergeAdd (ube);
  }
  res.EndBulkLoad ();
  return res;
}

/*
This methode calculates alle times for which mp is inside the 
FixedMRegion.
It returns an MBool, that is true for all times for which mp is inside
and false for all other times.

*/
MBool FixedMRegion::inside (const MPoint & mp){
  MRegion * rfix = buildMovingRegion ();
  MPoint tmp = approxMovement(mp);
  MBool res (0);
  rfix->Inside (tmp, res);
  delete rfix;
  MBool result = joinBools(res);
  printf("Inside: \n");
  cout << result << "\n";
  printf("");
  return result;
}

/*
This method intersects the FixedMRegion with MPoint mp.
It returns an MPoint that is defined for alle times for which mp is 
inside the FixedMRegion.

*/
MPoint FixedMRegion::intersection(const MPoint & mp){
  MBool parts = inside(mp);
  MPoint result(0);
  result.Clear();
  result.StartBulkLoad();
  RefinementPartition<MPoint,MBool,UPoint,UBool> rp(mp, parts);
  printf("Teile: %d\n", rp.Size());
  for (unsigned int i = 0; i < rp.Size(); i++) {
    Interval<Instant> iv;
    int urPos;
    int upPos;
    rp.Get(i, iv, urPos, upPos);
    DateTime tx = iv.start;
    Intime<CcBool> val(0);
    parts.AtInstant(tx, val);
    cout<< "Intervall: " << iv << " Value: "<<val.value << "\n";
    if(val.value==(CcBool)true){
      Intime<Point> ip_start(0);
      Intime<Point> ip_end(0);
      mp.AtInstant(iv.start, ip_start);
      mp.AtInstant(iv.end, ip_end);
      Point p_start = ip_start.value;
      Point p_end = ip_end.value;
      printf("%s, %s, %s\n",
             (p_start.IsDefined())?"ja":"nein",
             (p_end.IsDefined())?"ja":"nein",
             (iv.IsDefined())?"ja":"nein");
      UPoint ub(iv, p_start, p_end);
      if (ub.IsDefined()) {
        printf("Merging...\n");
        result.MergeAdd(ub);
      } else {
        printf("Skipping Merge\n");
      }
    }
  }
  result.EndBulkLoad();
  return result;
}

/*
This method calculates the 
intersection point of the given lines p1p2 and p3p4.

*/
Point FixedMRegion::getIntersectionPoint (const Point & p1,
  const Point & p2,const Point & p3, const Point & p4) const{
  double x1 = p1.GetX ();
  double y1 = p1.GetY ();
  double x2 = p2.GetX ();
  double y2 = p2.GetY ();
  double x3 = p3.GetX ();
  double y3 = p3.GetY ();
  double x4 = p4.GetX ();
  double y4 = p4.GetY ();

  double a, b, c, d, e, f, D, Dx;
  double t;

  a = x1 - x3;
  b = x1 - x2;
  c = x4 - x3;

  d = y1 - y3;
  e = y1 - y2;
  f = y4 - y3;

  D = b * f - c * e;
  Dx = a * f - c * d;
  t = Dx / D;

  double x = x1 + (x2 - x1) * t;
  double y = y1 + (y2 - y1) * t;
  Point p_res = Point (true, x, y);
  return p_res;
}

/*
This method will find out, which case the two lines p1p2 and p3p4 
belong to. The cases have got a short explanation in getTraversedArea.
A two digit value in a number system of base 4 is the result.

*/
int FixedMRegion::getTraversedCase (const Point & p1, const Point & p2,
 const Point & p3, const Point & p4) const{
  double x1 = p1.GetX ();
  double y1 = p1.GetY ();
  double x2 = p2.GetX ();
  double y2 = p2.GetY ();
  double x3 = p3.GetX ();
  double y3 = p3.GetY ();
  double x4 = p4.GetX ();
  double y4 = p4.GetY ();
  
  double a, b, c, d, e, f, D, Dx, Dy;
  double t, s;

  a = x1 - x3;
  b = x1 - x2;
  c = x4 - x3;

  d = y1 - y3;
  e = y1 - y2;
  f = y4 - y3;

  D = b * f - c * e;

  if (AlmostEqual (D, 0.0))
    {
      //Fallunterscheidung Geraden parallel oder aufeinander:
      //Strecken parallel zur x-Achse
      if (AlmostEqual (x1, x2))
        {
          if (AlmostEqual (x3, x1))
            {
              return -2;
            }
          else
            {
              return -1;
            }
        }
      double t = (x3 - x1) / (x2 - x1);
      if (AlmostEqual (y3, y1 + (y2 - y1) * t))
        {
          //Geraden liegen aufeinander
          return -2;
        }
      else
        {
          //Geraden liegen parallel
          return -1;
        }
    }

  Dx = a * f - c * d;
  Dy = b * d - a * e;

  t = Dx / D;
  s = Dy / D;
  //Strecke1: (x1, y1), (x2, y2)
  //Strecke2: (x3, y3), (x4, y4)
  int cc = 0;                   // Schnitt außerhalb
  //Fallunterscheidung nach (x1, y1), (x2, y2)
  if ((t > 0) && (t < 1))
    cc = 3;                     //Schnitt zwischen (x1, y1) und (x2, y2)
  if (AlmostEqual (t, 0))
    cc = 1;                     //Schnitt auf (x1, y1)
  if (AlmostEqual (t, 1))
    cc = 2;                     //Schnitt auf (x2, y2)
  //Fallunterscheidung nach (x3, y3), (x4, y4)
  if (AlmostEqual (s, 0)) {
    cc += 4;                    //Schnitt auf (x3, y3)
  } else if (AlmostEqual (s, 1)) {
    cc += 8;                    // Schnitt auf (x4, y4)
  } else if ((s > 0) && (s < 1)) {
    cc += 12;                   //Schnitt zwischen (x3, y3) und (x4, y4)
  }
  return cc;
}

/*
This method creates a vectorvector and puts the given vector into it.

*/
vector < vector < Point > > FixedMRegion::traversedGetVectorVector(
const vector < Point > v) const{
  vector < vector < Point > >vv;
  vv.push_back (v);
  return vv;
}

/*
This methods creates a polygon of the given Points.
This method adds four edges, the given points, to a vector. This is a 
preparation to create a quadrangle.

*/
vector < vector < Point > >FixedMRegion::traversedCalculateQuadrangle(
const Point & p1,const Point & p2, const Point & p3, const Point & p4) const{
  vector < Point > v;
  v.push_back (p1);
  v.push_back (p2);
  v.push_back (p3);
  v.push_back (p4);
  v.push_back (p1);
  return traversedGetVectorVector(v);
}

/*
This is an internal method that is used from traversedCalculateTriangle.
This methods creates a polygon of the given Points.
This method adds three edges, the given points, to a vector. This is a 
preparation to create a triangle.

*/
vector < Point > FixedMRegion::traversedCalcTriangle(const Point & p1,
const Point & p2, const Point & p3) const{
  vector < Point > v;   
  v.push_back (p1);
  v.push_back (p2);
  v.push_back (p3);
  v.push_back (p1);  
  return v;
}

/*
This methods creates a polygon of the given Points.
This method adds three edges, the given points, to a vector. This is a 
preparation to create a triangle.

*/
vector < vector < Point > >FixedMRegion::traversedCalculateTriangle(
const Point & p1,const Point & p2, const Point & p3) const{
  vector < Point > v;   
  v = traversedCalcTriangle (p1, p2, p3);
  return traversedGetVectorVector (v);
}

/*
This methods creates two polygons of the given Points, using the intersection
of the lines p1p2 and p3p4.
It will prepare two vectors with triangles and gives them back in a vectorvector.

*/
vector < vector < Point > >FixedMRegion::traversedCalculateTwoTriangles(
const Point p1, const Point p2, const Point p3, const Point p4) const{
  Point pi = getIntersectionPoint (p1, p2, p3, p4);
  vector < Point > t1;  
  vector < Point > t2;
  t1 = traversedCalcTriangle (pi, p1, p3);
  t2 = traversedCalcTriangle (pi, p2, p4);
  vector < vector < Point > >vv;
  vv.push_back (t1);
  vv.push_back (t2);
  return vv;
}

/*
This methods finds out the correct case and calls other methods to calculate 
the polygon that was traversed in this step. It uses the halfsegments' start 
and end position. It has to deal with al lot of different cases.

*/
vector < vector < Point > >FixedMRegion::getTraversedArea(
const HalfSegment & hsold,const HalfSegment & hsnew) const{
  vector < vector < Point > >res;
  Point p1 = hsold.GetDomPoint ();
  Point p2 = hsold.GetSecPoint ();
  Point p3 = hsnew.GetDomPoint ();
  Point p4 = hsnew.GetSecPoint ();
  vector < Point > v;
  int casetype = getTraversedCase (p1, p2, p3, p4);
  switch (casetype){
    case -1:   //Strecken sind parallel
    case 0:    //Schnittpunkt liegt außerhalb beider Strecken
    case 3:    //Schnittpunkt liegt innerhalb Strecke1 und außerhalb Strecke2
    case 12:   //Schnittpunkt liegt außerhalb Strecke1 und innerhalb Strecke2
      res = traversedCalculateQuadrangle (p1, p2, p4, p3);
      break;
    case 15:   //Schnittpunkt liegt innerhalb Strecke1 und innerhalb Strecke2
      res = traversedCalculateTwoTriangles (p1, p2, p3, p4);
      break;
    case -2:   //Strecken liegen auf der selben Geraden
      //Entartung
      break;
    case 2:    //Schnittpunkt liegt auf P2 und außerhalb Strecke2
      res = traversedCalculateTriangle (p1, p2, p3);
      break;
    case 1:    //Schnittpunkt liegt auf P1 und außerhalb Strecke2
      res = traversedCalculateTriangle (p1, p2, p4);
      break;
    case 4:    //Schnittpunkt liegt außerhalb Strecke1 und auf P3
      res = traversedCalculateTriangle (p2, p3, p4);
      break;
    case 5:    //Schnittpunkt liegt auf P1 und auf P3
      res = traversedCalculateTriangle (p1, p2, p4);
      break;
    case 8:    //Schnittpunkt liegt außerhalb Strecke1 und auf P4
      res = traversedCalculateTriangle (p1, p3, p4);
      break;
    case 6:    //Schnittpunkt liegt auf P2 und auf P3
      res = traversedCalculateTriangle (p1, p2, p4);
      break;
    case 7:    //Schnittpunkt liegt innerhalb Strecke1 und auf P3
      res = traversedCalculateTriangle (p1, p2, p4);
      break;
    case 9:    //Schnittpunkt liegt auf P1 und auf P4
      res = traversedCalculateTriangle (p1, p2, p3);
      break;
    case 10:   //Schnittpunkt liegt auf P2 und auf P4
      res = traversedCalculateTriangle (p1, p2, p3);
      break;
    case 11:   //Schnittpunkt liegt innerhalb Strecke1 und auf P4
      res = traversedCalculateTriangle (p1, p3, p4);
      break;
    case 13:   //Schnittpunkt liegt auf P1 und innerhalb Strecke2
      res = traversedCalculateTriangle (p1, p2, p4);
      break;
    case 14:   //Schnittpunkt liegt auf P2 und innerhalb Strecke2
      res = traversedCalculateTriangle (p1, p2, p3);
      break;

    default:
      assert (false);
      break;
    }
  return res;
}

/*
This method orientates the given polygons clockwise. Therefore, they will be 
faces.

*/
void FixedMRegion::traversedCreateCyclesNotHoles(
vector < vector < Point > >&v_list) const{
  for (size_t i = 0; i < v_list.size(); i++){
    if(!getDir(v_list[i])){
      reverseCycle(v_list[i]);
    }  
  }
}

/*
This method extracts a list of halfsegments from the region.

*/
vector < HalfSegment > FixedMRegion::getHSFromRegion() const{
  vector < HalfSegment > result;
  for(int i = 0; i < r.Size(); i++){
    HalfSegment tmp;
    r.Get(i, tmp);
    if(tmp.IsLeftDomPoint()){
      result.push_back(tmp);
    }  
  }
  return result;
}

/*
This method extracts a list of halfsegments from the given region.

*/
vector < HalfSegment > FixedMRegion::getHSFromRegion(const Region reg) const{
  vector < HalfSegment > result;
  for(int i = 0; i < reg.Size(); i++){
    HalfSegment tmp;
    reg.Get(i, tmp);
    if(tmp.IsLeftDomPoint()){
      result.push_back(tmp);
    }  
  }
  return result;
}

/*
This method will return a list of Halfsegments that the FMRegion will have at the 
given time ti.

*/
void FixedMRegion::atinstant (const double ti, const vector < HalfSegment > &v, 
vector < HalfSegment > & res){
  sett (ti);
  HalfSegment hs;
  for (size_t i = 0; i < v.size(); i++){
    hs = v[i];
    const Point lp = hs.GetLeftPoint();
    double newx = l.getImgX(lp.GetX(), lp.GetY());
    double newy = l.getImgY(lp.GetX(), lp.GetY());
    Point newlp(true, newx, newy);
    const Point rp = hs.GetRightPoint();
    newx = l.getImgX(rp.GetX(), rp.GetY());
    newy = l.getImgY(rp.GetX(), rp.GetY());
    Point newrp(true, newx, newy);
    hs.Set(hs.IsLeftDomPoint(), newlp, newrp);
    res.push_back(hs);
  }
}

/*
This methods creates a region that contains the traversed area between 
resultold and resultnew.

*/
Region * FixedMRegion::getDiffRegion (const vector < HalfSegment > *resultold,
const vector < HalfSegment > *resultnew) const{
  Region *diffregion = NULL;
  Region *tmp2 = NULL;
  Region *tmp_region = NULL;
  HalfSegment hsold;
  HalfSegment hsnew;
  for (size_t i = 0; i < resultold->size (); i++){
    hsold = (*resultold)[i];
    hsnew = (*resultnew)[i];
    vector < vector < Point > >tmp_polygons = getTraversedArea (hsold, hsnew);
    if(tmp_polygons.size () > 0){
      //FIXME: Routine zum orientierung prüfen und korrigieren
      traversedCreateCyclesNotHoles (tmp_polygons);
      tmp_region = buildRegion2 (tmp_polygons); 
      if (diffregion == NULL){
        diffregion = tmp_region;
      }else{
        tmp2 = new Region (0);
        RobustPlaneSweep::robustUnion (*diffregion, *tmp_region, *tmp2);
        delete diffregion;
        delete tmp_region;
        diffregion = tmp2;
      }
    }
  }
  return diffregion;
}


/*
This method will calculate the Region which contains all points / areas, that
the FMRegion has at least on time (or more often) traversed in the given 
intervall ta to te. 

*/
void FixedMRegion::traversedNew(Region & result,double ta, double te){
  result.Clear();
  for ( int i = 0; i < m.GetNoComponents(); i++) {
    UMove um(0);
    m.Get(i, um);
    Interval<Instant> iv=um.getTimeInterval();
    double _ta = iv.start.ToDouble();
    double _te = iv.end.ToDouble();
    if (_ta<ta)
      _ta=ta;
    if (_te>te)
      _te=te;
    if(_ta<_te){
      Region * res= new Region(0);
      atinstant(_ta, *res);
      vector < HalfSegment > vhs = getHSFromRegion();
      vector < HalfSegment > tiold;
      atinstant(_ta, vhs, tiold);
      int steps = calcStepWith(_ta, _te);
      for(int i=1; i<=steps; i++){
        vector < HalfSegment > tinew;
        atinstant(_ta + (_te-_ta)/steps*i, vhs, tinew);
        Region *tmp = getDiffRegion(&tiold, &tinew);
        if (tmp != NULL){
          Region *tmp2 = new Region(0);
          tmp2->Clear();
          RobustPlaneSweep::robustUnion (*res, *tmp, *tmp2);
          delete tmp;
          delete res;
          res = tmp2;
        }
        tiold = tinew;
      }
      Region tmp(result);
      result.Clear();
      RobustPlaneSweep::robustUnion(*res, tmp, result);
      delete res;
    }
  }
  return;
}

/*
This method returns the region of the FixedMRegion.

*/
const void FixedMRegion::getRegion(Region & result) const{
  result= r;
}

/*
This method sets the region of the FixedMRegion.

*/
void FixedMRegion::setRegion(const Region & _r){
  r = _r;
}

/*
This method returns the time t.
not used

*/
const double FixedMRegion::gett() const{
  return t;
}

/*
This method sets the time t.

*/
void FixedMRegion::sett(double _t){
  t = _t;
  calculateInternalVars();
}

/*
This method calculates the internal variables. It is used, when
the time has changed.

*/
void FixedMRegion::calculateInternalVars(){
  Point3 coord = getMovingForTimeFromMMove(DateTime(t));
  l = LATransform (coord.GetX(), coord.GetY(), 
    xm, ym, coord.GetAlpha());
}

/*
This method returns the Move.

*/
const MMove &FixedMRegion::getMove() const{
  return m;
}

/*
This method sets the Move.

*/
void FixedMRegion::setMove(const MMove & _m){
  m = _m;
}

/*
This calculates the euclidian distance between the two points.

*/
double FixedMRegion::getOneDistance(const Point &p1, const Point &p2) 
const{
  //FIXME
  //https://de.serlo.org/mathe/deutschland/bayern/gymnasium/
  //klasse-12/geraden-und-ebenen-im-raum/abstand-zweier-punkte-berechnen
  double res = sqrt(pow(p2.GetX()-p1.GetX(),2) + pow(p2.GetY()-p1.GetY(),2));
  return res;
}

/*
This method calculates the euclidian distance between the points and the 
one indicated by numberOfPoint.

*/
vector<double> FixedMRegion::getDistancesForPoint(const int numberOfPoint, 
const vector<Point> &p) const{
  vector<double> res = vector<double>(0);
  for(size_t i=0;i<p.size();i++){
    double z= getOneDistance(p[i], p[numberOfPoint]);
    res.push_back(z);
  }
  return res;
}

/*
This method returns the size of the object.

*/
size_t FixedMRegion::Sizeof() const{
  return sizeof(*this); 
}

/*
This method implements the compare functionality.

*/
int FixedMRegion::Compare(const Attribute* x) const{
  FixedMRegion *other=(FixedMRegion*)x;
  Region m;
  Region o;
  this->getRegion(m);
  other->getRegion(o);
  MMove v(0);
  v=this->getMove();
  MMove p(0);
  p=getMove();
  if(m.Compare(&o)!=0)
    return m.Compare(&o);
  if(v.Compare(&p))
    return v.Compare(&p);
  if (xm<other->xm)
    return -1;
  if (xm>other->xm)
    return 1;
  if (ym<other->ym)
    return -1;
  if (ym>other->ym)
    return 1;
  return 0;
}

/*
This method implements the adjacent functionality.

*/
bool FixedMRegion::Adjacent(const Attribute* x) const{
  FixedMRegion *other=(FixedMRegion*)x;
  Region m;
  Region o;
  this->getRegion(m);
  other->getRegion(o);
  MMove v(0);
  v=this->getMove();
  MMove p(0);
  p=getMove();
  return ((m.Adjacent(&o)) && (v.Adjacent(&p)));
}

/*
This method implements the clone functionality.

*/
Attribute* FixedMRegion::Clone() const{
  return new FixedMRegion(*this);
}

/*
This method implements the HashValue functionality.

*/
size_t FixedMRegion::HashValue() const{
  return m.HashValue() + r.HashValue(); 
}

/*
This method copies all internal information from another FixedMRegion.

*/
void FixedMRegion::CopyFrom(const Attribute* x){
  FixedMRegion *other=(FixedMRegion*)x;
  m.CopyFrom(&(other->m));
  l=other->l;
  r.CopyFrom(&(other->r));
  t=other->t;
  xm=other->xm;
  ym=other->ym;
}

/*
This method implements the NumOfFLOBs functionality.

*/
int FixedMRegion::NumOfFLOBs() const{
  return 1;
}

/*
This method implements the GetFLOB functionality.

*/
Flob* FixedMRegion::GetFLOB(const int i) {
  assert(i == 0);
  return r.GetFLOB(0); 
}

/*
This method implements the Print functionality.

*/
ostream& FixedMRegion::Print(ostream &os) const{
  os << "( FixedMRegionEmb NOT IMPLEMENTED YET )";
  return os;
}

