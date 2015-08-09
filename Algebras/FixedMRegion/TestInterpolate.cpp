/*
This class is a FixedMRegion.

*/
#include "TestInterpolate.h"


void TestInterpolate::testcalcMasspoint(){
  printf("testcalcMasspoint: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 0.0, 1.0);
  Point *p3 = new Point (true, 1.0, 0.0);
  Point *p4 = new Point (true, 1.0, 1.0);
  HalfSegment h1 (true, *p1, *p2);
  HalfSegment h2 (true, *p3, *p4);
  HalfSegment h3 (true, *p4, *p1);
  HalfSegment h4 (true, *p2, *p3);
  vector<HalfSegment> hs_list;
  hs_list.push_back(h1);
  hs_list.push_back(h2);
  hs_list.push_back(h3);
  hs_list.push_back(h4);
  Point res(0);
  FMRInterpolator fmr;
  res = fmr.calcMasspoint(hs_list);
  Point rtest(true, 0.5, 0.5);
  printf ("%s\n", (res == rtest) ? "OK" : "FAILED");
  if(res!=rtest){
    printf ("expected: (%f,%f)\n", rtest.GetX(),rtest.GetY());
    printf ("calculated: (%f,%f)\n", res.GetX(),res.GetY());
  }
}
void TestInterpolate::testcalcMaxDistPoint1(){
  printf("calcMaxDistPoint: ");
  Point *p1 = new Point (true, 6.0, 0.0);
  Point *p2 = new Point (true, 1.0, 9.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point rtest(true, 0.5, 0.5);
  Point p(0);
  FMRInterpolator fmr;
  p=fmr.calcMaxDistPoint(*r_tmp, rtest);
  Point res(true, 1.0, 9.0);
  printf ("%s\n", (res == p) ? "OK" : "FAILED");
  if(res!=p){
    printf ("expected: (%f,%f)\n", res.GetX(),res.GetY());
    printf ("calculated: (%f,%f)\n", p.GetX(),p.GetY());
  }
}
void TestInterpolate::testcalcMinDistPoint2(){
  printf("calcMinDistPoint: ");
  Point *p1 = new Point (true, 0.4, 0.4);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point rtest(true, 0.5, 0.5);
  Point p(0);
  FMRInterpolator fmr;
  p=fmr.calcMinDistPoint(*r_tmp, rtest);
  Point res(true, 0.4, 0.4);
  printf ("%s\n", (res == p) ? "OK" : "FAILED");
  if(res!=p){
    printf ("expected: (%f,%f)\n", res.GetX(),res.GetY());
    printf ("calculated: (%f,%f)\n", p.GetX(),p.GetY());
  }
}
void TestInterpolate::testcalcMaxDistPoint3(){
  printf("calcMaxDistPoint: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point rtest(true, 0.5, 0.5);
  Point p(0);
  FMRInterpolator fmr;
  p=fmr.calcMaxDistPoint(*r_tmp, rtest);
  printf ("%s\n", (!p.IsDefined()) ? "OK" : "FAILED");
  if(!p.IsDefined()){
    printf ("expected: false\n");
    printf ("calculated: (%f,%f)\n", p.GetX(),p.GetY());
  }
}
void TestInterpolate::testcalcMinDistPoint3(){
  printf("calcMinDistPoint: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point rtest(true, 0.5, 0.5);
  Point p(0);
  FMRInterpolator fmr;
  p=fmr.calcMinDistPoint(*r_tmp, rtest);
  printf ("%s\n", (!p.IsDefined()) ? "OK" : "FAILED");
  if(!p.IsDefined()){
    printf ("expected: false\n");
    printf ("calculated: (%f,%f)\n", p.GetX(),p.GetY());
  }
}

void TestInterpolate::testInList(){
  printf("testInList:\n");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  vector<Point> list;
  list.push_back(*p1);
  list.push_back(*p2);
  list.push_back(*p3);
  FMRInterpolator fmr;
  bool res;
  res = fmr.inList(list, *p1);
  printf("Test von Punkt in Liste:  %s\n", (res) ? "OK" : "FAILED");
  Point *p4 = new Point (true, 99.0, 19.0);
  res = fmr.inList(list, *p4);
  printf("Test von Punkt nicht in List  %s\n", (!res) ? "OK" : "FAILED");
}

void dummyMethod() {
  int x[10000];
  x[0]=0;
  for (int i=1; i<10000; i++)
    x[i]=x[i-1]+1;
}

void TestInterpolate::testCreatePointList(){
  printf("testCreatePointList:");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  vector<Point> list;
  list.push_back(*p1);
  list.push_back(*p2);
  list.push_back(*p3);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  FixedMRegion re = FixedMRegion (0, 0, 0, *r_tmp, 0, 0, 0,
    0, 1, 0);
  FMRInterpolator fmr;
  vector<HalfSegment> a;
  a = re.getHSFromRegion(*r_tmp);
  vector<Point> res(0);
  fmr.createPointList(a, res);
  dummyMethod();
  //for (int i = 0; i < res.size(); i++){
    //HalfSegment hs;
    //res.Get (i, hs);
    //printf("Point lp: %f, %f\n", res[i].GetX(), res[i].GetY());
    //const Point rp = hs.GetRightPoint ();
    //printf("Point rp: %f, %f\n", rp.GetX(), rp.GetY());
  //}
  printf("Test von Punkt in Liste:  %s\n", (res == list) ? "OK" : "FAILED");
}

void TestInterpolate::testSortList() {
  printf("testSortList:");
  Point p1(true, 0.0, 0.0);
  Point p2(true, 2.0, 0.0);
  Point p3(true, 0.5, 1.5);
  Point p4(true, 0.0, 2.0);

  vector<Point> p(0);
  p.push_back(p3);
  p.push_back(p1);
  p.push_back(p4);
  p.push_back(p2);

  FMRInterpolator fmr;
  fmr.sortList(p, Point(true, 1.0, 1.0));

  bool res=true;
  res&=p[0]==p1;
  res&=p[1]==p2;
  res&=p[2]==p3;
  res&=p[3]==p4;

  if (res==true) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    for (size_t i=0; i<p.size(); i++) {
      printf("%d: (%f, %f)\n", i, p[i].GetX(), p[i].GetY());
    }
    printf("*****************\n");
  }
}

void TestInterpolate::testGetSortedList() {
  printf("testGetSortedList:");
  Point p1(true, 0.0, 0.0);
  Point p2(true, 1.0, 0.0);
  Point p3(true, 0.0, 1.0);

  Region r(p1, p2, p3);

  FMRInterpolator fmr;
  vector<Point>p=fmr.getSortedList(r);

  bool res=true;
  res&=p[0]==p1;
  res&=p[1]==p2;
  res&=p[2]==p3;

  if (res==true) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    for (size_t i=0; i<p.size(); i++) {
      printf("%d: (%f, %f)\n", i, p[i].GetX(), p[i].GetY());
    }
    printf("*****************\n");
  }
}

/*
This method calculates the vector of distances to the masspoint.

*/
void TestInterpolate::testCalcDistVector(){
  printf("testCalcDistVector: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point m(true, 1.0, 1.0);
  FMRInterpolator fmr;
   //Point p(0);
//  FMRInterpolator fmr;
  //p=fmr.calcMaxMinDistPoint(*r_tmp, m);
  vector<double> res;
  res = fmr.calcDistVector(*r_tmp, m);
  vector<double> t;
  t.push_back(sqrt(2));
  t.push_back(1.0);
  t.push_back(1.0);
  printf ("%s\n", (res == t) ? "OK" : "FAILED");
  if(res != t){
    for(unsigned int i=0; i< t.size(); i++){
      printf ("expected %d: %f\n", i, t[i]);
    }
    for(unsigned int i=0; i< res.size(); i++){
      printf ("calculated %d: %f\n", i, res[i]);
    }
  }
}

void TestInterpolate::testmatchVectors(){
  printf("matchVectors: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point m(true, 1.0, 1.0);
  FMRInterpolator fmr;
  vector<double> t;
  t = fmr.calcDistVector(*r_tmp, m);
  Point *p1b = new Point (true, -1.0, 0.0);
  Point *p2b = new Point (true, 0.0, -1.0);
  Point *p3b = new Point (true, 0.0, 0.0);
  Region *r_tmpb = new Region (*p1b, *p2b, *p3b);
  Point mb(true, -1.0, -1.0);
  Point res (0);
  fmr.distVector=t;
  res = fmr.matchVectors(*r_tmpb, mb);
  Point e(true, 0.0, 0.0);
  printf ("%s\n", (res == e) ? "OK" : "FAILED");
  if(res != e){
    printf ("expected: (%f,%f)\n", e.GetX(), e.GetY());
    printf ("calculated: (%f,%f)\n", res.GetX(), res.GetY());
  }
}


void TestInterpolate::test_calcAngle(double x, double y) {
  //FMRInterpolator fmr;  
  printf("(%f, %f): %3.1f\n", x, y, 180*calcAngle(x, y)/M_PI);
}

void TestInterpolate::test_getTurnDir(double a1, double a2) {
  FMRInterpolator fmr;
  printf("%3.1f -> %3.1f: %s\n", a1, a2, (
    fmr.getTurnDir(M_PI/180*a1,  M_PI/180*a2))?"CCW":"CW");
}

void TestInterpolate::testCalcAngle(){
  printf("testCalcAngle:\n");
  test_calcAngle(1,0);
  test_calcAngle(2,1);
  test_calcAngle(2,4);
  test_calcAngle(1,2);
  test_calcAngle(0,2);
  test_calcAngle(-1,2);
  test_calcAngle(-2,1);
  test_calcAngle(-2,0);
  test_calcAngle(-2,-1);
  test_calcAngle(-1,-2);
  test_calcAngle(0,-1);
  test_calcAngle(1,-2);
  test_calcAngle(2,-1);
  test_calcAngle(2,0);
  printf("\n");
}

void TestInterpolate::testGetTurnDir(){
  printf("testGetTurnDir:\n");
  test_getTurnDir(0, 30);
  test_getTurnDir(0, 60);
  test_getTurnDir(0, 90);
  test_getTurnDir(0, 120);
  test_getTurnDir(0, 150);
  test_getTurnDir(0, -180);
  test_getTurnDir(0, -150);
  test_getTurnDir(0, -120);
  test_getTurnDir(0, -90);
  test_getTurnDir(0, -60);
  test_getTurnDir(0, -30);
  test_getTurnDir(0, 0);
  test_getTurnDir(30, 60);
  test_getTurnDir(30, -180);
  test_getTurnDir(30, -150);
  test_getTurnDir(30, -120);
  test_getTurnDir(150, -180);
  test_getTurnDir(150, 0);
  test_getTurnDir(150, -30);
  test_getTurnDir(150, -60);
  printf("\n");
}

void TestInterpolate::testcalculateAngleToXAxis(){
  printf("calculateAngleToXAxis: ");
  
  //double calculateAngleToXAxis(Region r, Point calcMasspoint);
}

void testPointStore() {
  printf("testPointStore\n");
  Point ref(true, 1.0, 1.0);
  PointStore p1(Point(true, 0.0, 0.0), ref);
  PointStore p2(Point(true, 2.0, 0.0), ref);
  PointStore p3(Point(true, 0.5, 1.5), ref);
  PointStore p4(Point(true, 0.0, 2.0), ref);
  PointStore p5(Point(true, 0.0, 2.0), ref);
  printf("getPoint(): %s\n", 
    (p1.getPoint()==Point(true, 0.0, 0.0))?"OK":"Failed");
  PointStore p6(p1);
  printf("CopyConstr: %s\n", 
    (p6.getPoint()==Point(true, 0.0, 0.0))?"OK":"Failed");
  printf("operator<1: %s\n", (p1<p2)?"OK":"Failed");
  printf("operator<2: %s\n", (p1<p3)?"OK":"Failed");
  printf("operator<3: %s\n", (p1<p4)?"OK":"Failed");
  printf("operator<4: %s\n", (p2<p3)?"OK":"Failed");
  printf("operator<5: %s\n", (p2<p4)?"OK":"Failed");
  printf("operator<6: %s\n", (p3<p4)?"OK":"Failed");
  printf("operator>1: %s\n", (p2>p1)?"OK":"Failed");
  printf("operator>2: %s\n", (p3>p1)?"OK":"Failed");
  printf("operator>3: %s\n", (p4>p1)?"OK":"Failed");
  printf("operator>4: %s\n", (p3>p2)?"OK":"Failed");
  printf("operator>5: %s\n", (p4>p2)?"OK":"Failed");
  printf("operator>6: %s\n", (p4>p3)?"OK":"Failed");
  printf("operator==: %s\n", (p1==p1)?"OK":"Failed");
  printf("operator==: %s\n", (!(p1==p2))?"OK":"Failed");
  printf("operator==: %s\n", (!(p3==p4))?"OK":"Failed");
  printf("operator==: %s\n", (p4==p5)?"OK":"Failed");
}


void TestInterpolate::testinterpolatetest(){
  printf("testinterpolatetest: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region r_tmp1(*p1, *p2, *p3);
  Point *p12 = new Point (true, 1.0, 0.0);
  Point *p22 = new Point (true, 1.0, 1.0);
  Point *p32 = new Point (true, 2.0, 1.0);
  Region r_tmp2(*p12, *p22, *p32);
  Point *p13 = new Point (true, 2.0, 0.0);
  Point *p23 = new Point (true, 2.0, 1.0);
  Point *p33 = new Point (true, 3.0, 1.0);
  Region r_tmp3(*p13, *p23, *p33);
  vector<Region> regions;
  regions.push_back(r_tmp1);
  regions.push_back(r_tmp2);
  regions.push_back(r_tmp3);
  //Point m(true, 1.0, 1.0);
  
  DateTime t1 (instanttype);
  t1.Set (2015, 3, 30, 8, 01);
  DateTime t2 (instanttype);
  t2.Set (2015, 3, 30, 9, 02);
  DateTime t3 (instanttype);
  t3.Set (2015, 4, 01, 7, 02);
  vector<DateTime> dates;
  dates.push_back(t1);
  dates.push_back(t2);
  dates.push_back(t3);
  
  FMRInterpolator fmr;
  //vector<double> t;
  fmr.start();
  IRegion ir1(t1, r_tmp1);
  fmr.addObservation(ir1);
  IRegion ir2(t2, r_tmp2);
  fmr.addObservation(ir2);
  IRegion ir3(t3, r_tmp3);
  fmr.addObservation(ir3);
  fmr.end();
  FixedMRegion res1 = fmr.getResult();
  //FIXME: Erwartungswert?
  
  Point m(true, 0.0, 0.0);
  Point *p1r = new Point (true, 0.5, 0.0);
  Point *p2r = new Point (true, 1.5, 0.0);
  Point *p3r = new Point (true, 0.5, 1.0);
  Region r(*p1r, *p2r, *p3r);
  
  
  fmr.start();
  IRegion ir11(t1, r);
  fmr.addObservation(ir11);
  IRegion ir21(t2, r);
  fmr.addObservation(ir21);
  IRegion ir31(t3, r);
  fmr.addObservation(ir31);
  fmr.end();
  FixedMRegion res11 = fmr.getResult();
  
  cout << "Resultat: " << res11 << "\\n";
  //FIXME: Erwartungswert?
  
  /*printf ("%s\n", (res == e) ? "OK" : "FAILED");
  if(res != e){
    printf ("expected: (%f,%f)\n", e.GetX(), e.GetY());
    printf ("calculated: (%f,%f)\n", res.GetX(), res.GetY());
  }*/
}








void TestInterpolate::testsetDistVector(){
  printf("testsetDistVector: ");
  vector<double> a;
  a.push_back(2.0);
  a.push_back(1.0);
  FMRInterpolator fmr;
  fmr.distVector=a;
  vector<double> b = fmr.distVector;
  printf ("%s\n", (a == b) ? "OK" : "FAILED");
  if(a != b){
    for (unsigned int i = 0; i < a.size(); i++){
      printf ("expected: %f\n", a[i]);
      printf ("calculated: %f\n", b[i]);
    }
  }
}
void TestInterpolate::testdetermineAngleAlgorithm(){
  printf("testdetermineAngleAlgorithm: ");
  Point *p1 = new Point (true, 6.0, 0.0);
  Point *p2 = new Point (true, 1.0, 9.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point pd(true, 1.0, 1.0);
  FMRInterpolator fmr;
  fmr.setReferenceRegion(*r_tmp, pd);
  double p = fmr.determineAngleAlgorithm();
  printf ("%s\n", (1 == p) ? "OK" : "FAILED");
  if(1!=p){
    printf ("expected: 1\n");
    printf ("calculated: %f\n", p);
  }
  printf("testdetermineAngleAlgorithm2: ");
  Point *p11 = new Point (true, 0.4, 0.4);
  Point *p21 = new Point (true, 1.0, 0.0);
  Point *p31 = new Point (true, 0.0, 1.0);
  Region *r_tmp1 = new Region (*p11, *p21, *p31);
  fmr.setReferenceRegion(*r_tmp1, pd);
  p = fmr.determineAngleAlgorithm();
  printf ("%s\n", (2 == p) ? "OK" : "FAILED");
  if(2!=p){
    printf ("expected: 2\n");
    printf ("calculated: %f\n", p);
  }
  //FIXME Test distvector-Fall fehlt
  printf("TEST FÜR DISTVECTOR_FALL FEHLT!\n");
}

void TestInterpolate::testcalcThisAngle(){
  printf("testcalcThisAngle: ");
  Point *p1 = new Point (true, 6.0, 1.0);
  Point *p2 = new Point (true, 1.0, 9.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point pd(true, 1.0, 1.0);
  FMRInterpolator fmr;
  fmr.angle_init=0;
  fmr.angle_method=1;
  double p=fmr.calcThisAngle(*r_tmp);
  double res =1.815775;
  if (fabs(p-res)<0.0001) {
    printf("OK\n");
  } else {
    printf ("Failed\n");
    printf ("expected: %f\n", res);
    printf ("calculated: %f\n", p);
  }
}

/*
This method creates all UMoves and puts them into a MMove that will be returned.

*/
//MMove createMMove(const vector<DateTime>&times, 
//vector<Point> final_translations, vector<double> angles);
void TestInterpolate::testcreateMMove(){
  printf("testcreateMMove: ");
  DateTime t1 (instanttype);
  t1.Set (2015, 7, 01, 8, 00);
  DateTime t2 (instanttype);
  t2.Set (2015, 7, 02, 9, 00);
  DateTime t3 (instanttype);
  t3.Set (2015, 7, 03, 8, 00);
  vector<DateTime> t;
  t.push_back(t1);
  t.push_back(t2);
  t.push_back(t3);
  vector<Point> v;
  v.push_back(Point(true, 0.0, 1.0));
  v.push_back(Point(true, 0.0, 2.0));
  v.push_back(Point(true, 1.0, 2.0));
  vector<double> a;
  a.push_back(0.0);
  a.push_back(M_PI/2);
  a.push_back(M_PI);
  FMRInterpolator fmr;
  fmr.observations.push_back(FMRObservation(v[0], a[0],t[0]));
  fmr.observations.push_back(FMRObservation(v[1], a[1],t[1]));
  fmr.observations.push_back(FMRObservation(v[2], a[2],t[2]));
  MMove res = fmr.createMMove();
  bool success=true;
  for (unsigned int i=0; i<t.size(); i++) {
    Intime<Point3> r1;
    res.AtInstant(t[i], r1);
    Point3 r2=r1.value;
    if (r2.GetX()!=v[i].GetX())
      success=false;
    if (r2.GetY()!=v[i].GetY())
      success=false;
    if (r2.GetAlpha()!=a[i])
      success=false;
  }
  if (success) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    for (unsigned int i=0; i<t.size(); i++) {
      Intime<Point3> r1;
      res.AtInstant(t[i], r1);
      cout << t[i] << "\n";
      Point3 r2=r1.value;
      printf("Result: (%f, %f, %f), Expected: (%f, %f, %f)\n",
             r2.GetX(), r2.GetY(), r2.GetAlpha(),
             v[i].GetX(), v[i].GetY(), a[i]);
    }
  }
}

ListExpr OutFixedMRegion(ListExpr typeInfo, Word value);

void createTestFMR() {
  cout << "testFMR\n";
  vector<Point> a(0);
  a.push_back(Point(true,0,10));
  a.push_back(Point(true,3,6));
  a.push_back(Point(true,3,-10));
  a.push_back(Point(true,-3,-10));
  a.push_back(Point(true,-3,6));
  a.push_back(Point(true,0,10));
  vector<vector<Point> > b(0);
  b.push_back(a);
  Region * tmpRegion=buildRegion2(b);
  FixedMRegion r(0, 20, 0, *tmpRegion, 0, 0, 0, 0, 0, -M_PI/2);
  ListExpr n=OutFixedMRegion(nl->TheEmptyList(), SetWord(&r));
  cout << "FMR: " << nl->ToString(n) << "\n";
  cout << "IRegions:\n(";
  for (double t=0; t<1.05; t+=0.1) {
    Region res(0);
    r.atinstant(t, res);
    IRegion res2(Instant(t), res);
    n=OutIntime<Region, OutRegion>(nl->TheEmptyList(), SetWord(&res2));
    cout << "(" <<nl->ToString(n) << ")";
  }
  cout << ")\n";
}

/*
This is the only test method and contains all tests.

*/
void runTestInterpolateMethod(){
  printf ("testInterpolateMethods: \n");
/*
  testPointStore();
  TestInterpolate t ;
  t.testcalcMasspoint();
  t.testcalcMaxDistPoint1();
  t.testcalcMinDistPoint2();
  t.testcalcMaxDistPoint3();
  t.testcalcMinDistPoint3();
  t.testInList();
  t.testCreatePointList();
  t.testSortList();
  t.testGetSortedList();
  t.testCalcDistVector();
  t.testmatchVectors();
  t.testCalcAngle();
  t.testGetTurnDir();
  t.testsetDistVector();
  t.testdetermineAngleAlgorithm();
  t.testcalcThisAngle();
  t.testcreateMMove();
  //FIXME
  t.testcalculateAngleToXAxis();
  t.testinterpolatetest();
  */
  createTestFMR();
}
