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

void TestInterpolate::testsetReferenceRegion(){
  printf("setReferenceRegion: ");
  double min1a[] = { 0.0, 0.0 };
  double max1a[] = { 1.0, 1.0 };
  Region *r_tmp = new Region (Rectangle < 2 > (true, min1a, max1a));
  Point m(true, 0.5, 0.5);
  double min1b[] = { -0.5, -0.5 };
  double max1b[] = { 0.5, 0.5 };
  Region *exp = new Region (Rectangle < 2 > (true, min1b, max1b));
  FMRInterpolator fmr;
  fmr.setReferenceRegion(*r_tmp, m);
  Region res = fmr.getReferenceRegion();
  printf("%s\n", (res == *exp) ? "OK" : "FAILED");
  if(res != *exp){
    printf("expected region points:\n");
    for (int i = 0; i < (*exp).Size (); i++){
      HalfSegment hs;
      (*exp).Get (i, hs);
      const Point lp = hs.GetLeftPoint ();
      printf("Point lp: %f, %f\n", (lp.GetX(), lp.GetY()));
      const Point rp = hs.GetRightPoint ();
      //printf("Point rp: %f, %f\n", rp.GetX(), rp.GetY());
    }
    printf("calculated region points:\n");
    for (int i = 0; i < res.Size (); i++){
      HalfSegment hs;
      res.Get (i, hs);
      const Point lp = hs.GetLeftPoint ();
      printf("Point lp: %f, %f\n", (lp.GetX(), lp.GetY()));
      const Point rp = hs.GetRightPoint ();
      //printf("Point rp: %f, %f\n", rp.GetX(), rp.GetY());
    }
  }
}

void TestInterpolate::testcalcMaxMinDistPoint1(){
  printf("calcMaxMinDistPoint: ");
  Point *p1 = new Point (true, 6.0, 0.0);
  Point *p2 = new Point (true, 1.0, 9.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point rtest(true, 0.5, 0.5);
  Point p(0);
  FMRInterpolator fmr;
  p=fmr.calcMaxMinDistPoint(*r_tmp, rtest);
  Point res(true, 1.0, 9.0);
  printf ("%s\n", (res == p) ? "OK" : "FAILED");
  if(res!=p){
    printf ("expected: (%f,%f)\n", res.GetX(),res.GetY());
    printf ("calculated: (%f,%f)\n", p.GetX(),p.GetY());
  }
}
void TestInterpolate::testcalcMaxMinDistPoint2(){
  printf("calcMaxMinDistPoint: ");
  Point *p1 = new Point (true, 0.4, 0.4);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point rtest(true, 0.5, 0.5);
  Point p(0);
  FMRInterpolator fmr;
  p=fmr.calcMaxMinDistPoint(*r_tmp, rtest);
  Point res(true, 0.4, 0.4);
  printf ("%s\n", (res == p) ? "OK" : "FAILED");
  if(res!=p){
    printf ("expected: (%f,%f)\n", res.GetX(),res.GetY());
    printf ("calculated: (%f,%f)\n", p.GetX(),p.GetY());
  }
}
void TestInterpolate::testcalcMaxMinDistPoint3(){
  printf("calcMaxMinDistPoint: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *r_tmp = new Region (*p1, *p2, *p3);
  Point rtest(true, 0.5, 0.5);
  Point p(0);
  FMRInterpolator fmr;
  p=fmr.calcMaxMinDistPoint(*r_tmp, rtest);
  printf ("%s\n", (!p.IsDefined()) ? "OK" : "FAILED");
  if(!p.IsDefined()){
    printf ("expected: false\n");
    printf ("calculated: (%f,%f)\n", p.GetX(),p.GetY());
  }
}
/*
This method calculates the vector of distances to the masspoint.

*/
void TestInterpolate::testCalcDistVector(){
  //vector<double> calcDistVector(Region r, Point masspoint)
}

void TestInterpolate::testmatchVectors(){
  printf("matchVectors: ");
  //Point matchVectors(vector<double> distVector, 
  //  Region r, Point calcMasspoint);
}

void TestInterpolate::testcalculateAngleToXAxis(){
  printf("calculateAngleToXAxis: ");
  //double calculateAngleToXAxis(Region r, Point calcMasspoint);
}




/*
This is the only test method and contains all tests.

*/
void runTestInterpolateMethod(){
  printf ("testInterpolateMethods: \n");
  TestInterpolate t ;
  t.testcalcMasspoint();
  t.testsetReferenceRegion();
  t.testcalcMaxMinDistPoint1();
  t.testcalcMaxMinDistPoint2();
  t.testcalcMaxMinDistPoint3();
  t.testcalcDistVectorsIdentSmallestRotFirstPoint();
  t.testcalculateAngleToXAxis();
}
