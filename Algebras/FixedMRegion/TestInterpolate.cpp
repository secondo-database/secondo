/*
This class is a FixedMRegion.

*/
#include "TestInterpolate.h"


void TestInterpolate::testcalcMassPoint(){
  printf("testcalcMassPoint: ");
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
//  double min1a[] = { 0.0, 0.0 };
//  double max1a[] = { 1.0, 0.001 };
//  Region *r_tmp = new Region (Rectangle < 2 > (true, min1a, max1a));
//  FixedMRegion fmr = FixedMRegion (0.0, 0.0, 0.0, *r_tmp, 0.0, 0.0, 0.0,
//     0.0, 0.0, M_PI / 2);
  FixedMRegion fmr=FixedMRegion();
  res = fmr.calcMassPoint(hs_list);
  Point rtest(true, 0.5, 0.5);
  printf ("%s\n", (res == rtest) ? "OK" : "FAILED");
  if(res!=rtest){
    printf ("erwartet: (%f,%f)\n", rtest.GetX(),rtest.GetY());
    printf ("berechnet: (%f,%f)\n", res.GetX(),res.GetY());
  }
}

void TestInterpolate::testsetReferenceRegion(){
  printf("setReferenceRegion: ");
  //void setReferenceRegion(Region _r, Point _calcMasspoint);
}

void TestInterpolate::testcalcMaxMinDistPoint(){
  printf("calcMaxMinDistPoint: ");
  //Point calcMaxMinDistPoint(Region _r, Point _calcMasspoint);
}

void TestInterpolate::testcalcDistVectorsIdentSmallestRotFirstPoint(){
  printf("calcDistVectorsIdentSmallestRotFirstPoint: ");
  //Point calcDistVectorsIdentSmallestRotFirstPoint(vector<dounle> distVector, 
}

void TestInterpolate::testcalculateAngleToXAxis(){
  printf("calculateAngleToXAxis: ");
  //double calculateAngleToXAxis(Region _r, Point _calcMasspoint);
}




/*
This is the only test method and contains all tests.

*/
void runTestInterpolateMethod(){
  printf ("testInterpolateMethods: \n");
  TestInterpolate t ;
  t.testcalcMassPoint();
  t.testsetReferenceRegion();
  t.testcalcMaxMinDistPoint();
  t.testcalcDistVectorsIdentSmallestRotFirstPoint();
  t.testcalculateAngleToXAxis();
}
