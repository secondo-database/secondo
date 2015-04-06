/*
This is the file with the tests.

*/
using namespace std;
#include "FixedMRegiontest.h"
#include "Secondo_Include.h"
#include <vector>
extern NestedList* nl;
extern QueryProcessor* qp;

/*
This method tests the class Move.

*/
void testMove(){
  //Move tests
  printf("Test Move\n");
  Move *m = new Move(0,0,0,2,3,4);
  double *res1=m->attime(0);
  double *res2=m->attime(1);
  printf("m-attime(0):%s\n",((res1[0]==0)&&(res1[1]==0)&&
  (res1[2]==0))?"OK":"FAILED");
  printf("m-attime(1):%s\n",((res2[0]==2)&&(res2[1]==3)&&
  (res2[2]==4))?"OK":"FAILED");
  delete res1;
  delete res2;
  delete m;
  
  Move *m1 = new Move(0,0,0,3,3,3);
  double *res11=m1->attime(0);
  double *res21=m1->attime(1);
  printf("m1-attime(0):%s\n",((res11[0]==0)&&(res11[1]==0)&&
  (res11[2]==0))?"OK":"FAILED");
  printf("m1-attime(1):%s\n",((res21[0]==3)&&(res21[1]==3)&&
  (res21[2]==3))?"OK":"FAILED");
  delete res11;
  delete res21;
  delete m1;
  
  Move *m2 = new Move(1,1,1,0,1,0);
  double *res12=m2->attime(0);
  double *res22=m2->attime(1);
  printf("m2-attime(0):%s\n",((res12[0]==1)&&(res12[1]==1)&&
  (res12[2]==1))?"OK":"FAILED");
  printf("m2-attime(1):%s\n",((res22[0]==1)&&(res22[1]==2)&&
  (res22[2]==1))?"OK":"FAILED");
  delete res12;
  delete res22;
  delete m2;
}
/*
This method tests the class LATransform.

*/
void testLATransform(){
  //LATransform tests
  printf("Test LATransform\n");
  LATransform *l=new LATransform(1,1, 0,0, 0);
  printf("Translation(1,0):%s\n",((l->getImgX(1.0, 0.0)==2)&&
  (l->getImgY(1.0, 0.0)==1))?"OK":"FAILED");
  printf("Translation(0,0):%s\n",((l->getImgX(0.0, 0.0)==1)&&
  (l->getImgY(0.0, 0.0)==1))?"OK":"FAILED");
  delete l;
  
  LATransform *l1=new LATransform(0,0, 0,0, M_PI/2);
  printf("RotationUrsprungUmPiHalbe:%s\n",
((abs(l1->getImgX(1.0, 0.0)-0.0)<0.000001)
  &&(abs(l1->getImgY(1.0, 0.0)-1.0)<0.000001))?"OK":"FAILED");
  delete l1;
  
  LATransform *l2=new LATransform(0,0, 0,0, M_PI);
  printf("RotationUrsprungUmPi:%s\n",
 ((abs(l2->getImgX(1.0, 0.0)+1.0)<0.000001)
  &&(abs(l2->getImgY(1.0, 0.0))<0.000001))?"OK":"FAILED");
  delete l2;
  
  LATransform *l3=new LATransform(0,0, 1,1, 2*M_PI);
  printf("RotationNichtUrsprungUm2Pi:%s\n",
((abs(l3->getImgX(0.0, 0.0))<0.000001)
  &&(abs(l3->getImgY(0.0, 0.0))<0.000001))?"OK":"FAILED");
  delete l3;
  
  LATransform *l4=new LATransform(0,0, 1,1, M_PI);
  printf("RotationNichtUrsprungUmPi:%s\n",
 ((abs(l4->getImgX(0.0, 0.0)-2.0)<0.000001)
  &&(abs(l4->getImgY(0.0, 0.0)-2.0)<0.000001))?"OK":"FAILED");
  delete l4;
  
  LATransform *l5=new LATransform(1,1,0,0, M_PI);
  printf("TranslatioinMitRotationUmPi:%s\n",
((abs(l5->getImgX(1.0, 0.0)-0.0)<0.000001)
  &&(abs(l5->getImgY(1.0, 0.0)-1.0)<0.000001))?"OK":"FAILED");
  delete l5;
  
  LATransform *l6=new LATransform(1,1,0,0, 2*M_PI);
  printf("TranslatioinMitRotationUm2Pi:%s\n",
((abs(l6->getImgX(1.0, 0.0)-2.0)<0.000001)
  &&(abs(l6->getImgY(1.0, 0.0)-1.0)<0.000001))?"OK":"FAILED");
  delete l6;

    LATransform *l7=new LATransform(1,1,1,1,M_PI);
  printf("TranslatioinMitRotationUm2Pi:%s\n",
((abs(l7->getImgX(2.0,2.0)-1.0)<0.000001)
  &&(abs(l7->getImgY(2.0, 2.0)-1.0)<0.000001))?"OK":"FAILED");
  delete l7;
}
/*
This method creates a region.

*/
void testRegion(){
  printf("Test Region\n");
  Point *p1 = new Point( true, 0.0, 0.0);
  Point *p2 = new Point( true, 1.0, 0.0);
  Point *p3 = new Point( true, 0.0, 1.0);
  Region *result= new Region(*p1, *p2, *p3);

  HalfSegment hs;
  for( int i = 0; i <2; i++ )
  {
    result->Get( i, hs );
    const Point l=hs.GetLeftPoint();
    printf("P%f=(%f,%f)\n",(double)i,l.GetX(),l.GetY());
    Point r=hs.GetRightPoint();
    printf("P%f=(%f,%f)\n",(double)i,r.GetX(),r.GetY());
  }
  delete p1;
  delete p2;
  delete p3;
  delete result;
}
/*
This Method tests wether 2 Regions are identical
if their edgepoints match

*/
void testRegionCompare() {
  printf("Test RegionCompare\n");
  Point *p1 = new Point( true, 0.0, 0.0);
  Point *p2 = new Point( true, 1.0, 0.0);
  Point *p3 = new Point( true, 0.0, 1.0);
  Region *result= new Region(*p1, *p2, *p3);
  p1 = new Point( true, 0.0, 0.0);
  p2 = new Point( true, 1.0, 0.0);
  p3 = new Point( true, 0.0, 1.0);
  Region * res2=new Region(*p1, *p2, *p3);
  p1 = new Point( true, 0.0, 0.0);
  p2 = new Point( true, 0.0, 1.0);
  p3 = new Point( true, 1.0, 0.0);
  Region * res3=new Region(*p1, *p2, *p3);
  printf("Same Order: %s\n", (*result==*res2)?"OK":"FAILED");
  printf("Different Order : %s\n", (*result==*res3)?"OK":"FAILED");
  delete p1;
  delete p2;
  delete p3;
  delete result;
  delete res2;
  delete res3;
}

/*
This method tests the method atinstant without a movement.

*/
void testatinstantNoMove(){
  printf("Test atinstantNoMove: ");
  Point *p1 = new Point( true, 0.0, 0.0);
  Point *p2 = new Point( true, 1.0, 0.0);
  Point *p3 = new Point( true, 0.0, 1.0);
  Region *result= new Region(*p1, *p2, *p3);
  FixedMRegion fmr = FixedMRegion
        (0, 0, 0, result, 0, 0, 0, 0, 0, 0); 
  Region * res2=fmr.atinstant(2);
  printf("%s\n", (*result==*res2)?"OK":"FAILED");
  delete p1;
  delete p2;
  delete p3;
  delete res2;
}

void printHS(HalfSegment hs) {
  printf("((%f, %f) - (%f, %f))\n",
    hs.GetLeftPoint().GetX(),
    hs.GetLeftPoint().GetY(),
    hs.GetRightPoint().GetX(),
    hs.GetRightPoint().GetY());
}

int gotPoint(const Point & p, const Point* list, int size) {
    for (int i=0; i<size; i++) {
      if (p.Compare(list+i)==0)
        return i;
    }
    return -1;
}

bool checkRegionPoints(Region *r, const Point *list, int size) {
  bool *res = new bool[size];
  for (int i=0; i<size; i++)
    res[i]=false;
  for (int i=0; i<r->Size(); i++) {
    HalfSegment hs;
    r->Get(i, hs);
    int tmp;
    tmp=gotPoint(hs.GetLeftPoint(), list, size);
    if (tmp==-1)
      return false;
    res[tmp]=true;
    tmp=gotPoint(hs.GetRightPoint(), list, size);
    if (tmp==-1)
      return false;
    res[tmp]=true;
  }
  bool ret=true;
  for (int i=0; i<size; i++)
    ret&=res[i];
  delete res;
  return ret;
}
/*
This method tests the method atinstant with a linear movement.

*/
void testatinstantLinearMove(){
  printf("Test atinstantLinearMove: ");
  Point *p1 = new Point( true, 0.0, 0.0);
  Point *p2 = new Point( true, 1.0, 0.0);
  Point *p3 = new Point( true, 0.0, 1.0);
  Region *result= new Region(*p1, *p2, *p3);
  Point expected[]= {Point(true, 2.0, 4.0), Point(true, 3.0, 4.0), 
    Point(true, 2.0, 5.0)};

  FixedMRegion fmr = FixedMRegion(0, 0, 0, result, 0, 0, 0, 
  1, 2, 0);   
  Region * res=fmr.atinstant(2);
  printf("%s\n", (checkRegionPoints(res, expected, 3))?"OK":"FAILED");
  delete p1;
  delete p2;
  delete p3;
  delete res;
}
  
void testatinstantRotate(){
  printf("Test atinstantRotate: ");
  Point *p1 = new Point( true, 0.0, 0.0);
  Point *p2 = new Point( true, 1.0, 0.0);
  Point *p3 = new Point( true, 0.0, 1.0);
  Region *result= new Region(*p1, *p2, *p3);
  Point expected[]= {Point(true, 1.0, 0.0), Point(true, 0.0, 1.0), 
    Point(true, 1.0, 1.0)};
  
  FixedMRegion fmr = FixedMRegion(1, 0.5, 0.5, result, 0, 0, 0, 
  0, 0, M_PI); 
  Region * res=fmr.atinstant(1); 
  printf("%s\n", (checkRegionPoints(res, expected, 3))?"OK":"FAILED");
  delete p1;
  delete p2;
  delete p3;
  delete res;
}

void testtraversed1(){  
  printf("Test traversed simple: ");
  double min[]={0.0, 0.0};
  double max[]={3.0, 3.0};
  Region *rbig = new Region(Rectangle<2>(true,min, max));
  FixedMRegion fmr = FixedMRegion(0, 0, 0, rbig, 0, 0, 0, 
  0, 0.5, 0); 
  Region * res2=fmr.traversed(0.0,1,0.1);

  double min1[]={0.0, 0.0};
  double max1[]={3.0, 3.5};
  Region *r_test = new Region(Rectangle<2>(true,min1, max1));
  Region *me = new Region(*r_test);
  r_test->Minus(*res2, *me);
  double r1=me->Area();
  me=new Region(*res2);
  res2->Minus(*r_test,*me);
  double r2=me->Area();
  printf("%s\n", (r1==r2)?"OK":"FAILED");
  delete res2;
  delete r_test;
  delete me;
}

void testtraversed2(){  
  printf("Test traversed with hole: ");
  double min[]={0.0, 0.0};
  double max[]={3.0, 3.0};
  Region *rbigwo = new Region(Rectangle<2>(true,min, max));
  
  double minh[]={1.0, 1.0};
  double maxh[]={2.01, 2.01};
  Region *rbigh = new Region(Rectangle<2>(true,minh, maxh));
  
  Region *rbig = new Region(*rbigwo);
  
  printf("ping11\n");
  rbigwo->Minus(*rbigh, *rbig);
  printf("ping12\n");
  
  FixedMRegion fmr = FixedMRegion(0, 0, 0, rbig, 0, 0, 0, 
  1, 0, 0); 
  Region * res2=fmr.traversed(0.0,0.5,0.5);
  printf("ping13\n");

  double min1[]={0.0, 0.0};
  double max1[]={3.0, 3.5};
  Region *r_res1 = new Region(Rectangle<2>(true,min1, max1));
  
  double min1a[]={1.0, 1.5};
  double max1a[]={2.0, 2.0};
  Region *r_tmp = new Region(Rectangle<2>(true,min1a, max1a));  
  
  Region *r_test = new Region(*r_tmp);
  
  printf("ping1\n");
  r_res1->Minus(*r_tmp, *r_test);
  printf("ping2\n");
  
  Region *me = new Region(*r_test);
  r_test->Minus(*res2, *me);
  double r1=me->Area();
  me=new Region(*res2);
  res2->Minus(*r_test,*me);
  double r2=me->Area();
  printf("%s\n", (r1==r2)?"OK":"FAILED");
  delete res2;
  delete r_test;
  delete me;
}


/*
This method tests the method traversed.
  
*/
void testtraversed(){
  testtraversed1();
  testtraversed2();
}

void testMBool(){
  printf("Test testMBool: \n");
  MBool* res =new MBool(10);
  res->Clear();
  res->StartBulkLoad();
  DateTime t1(instanttype);
  t1.Set(2015,3,30,8,01);
  DateTime t2(instanttype);
  t2.Set(2015,3,30,9,02);
  Interval<Instant> iv(t1,t2,false,true);
  UBool ub(iv,(CcBool)true);
  res->MergeAdd(ub);
  
  DateTime t11(instanttype);
  t11.Set(2015,3,31,9,02);
  DateTime t21(instanttype);
  t21.Set(2015,4,30,8,02);
  Interval<Instant> iv2(t11,t21,false,true);
  UBool ub2(iv2,(CcBool)true);
  res->MergeAdd(ub2);
  res->EndBulkLoad();
  
  DateTime test(instanttype);
  test.Set(2015,3,31,9,02);
  printf("(? %s\n",(res->Present(test))?"Wahr":"Falsch");
  
  DateTime test1(instanttype);
  test1.Set(2015,4,30,8,02);
  printf("]? %s\n",(res->Present(test1))?"Wahr":"Falsch");
  
  DateTime test2(instanttype);
  test2.Set(2015,1,30,9,02);
  printf("außerhalb %s\n",(res->Present(test2))?"Wahr":"Falsch");
  
  DateTime test3(instanttype);
  test3.Set(2015,4,8,8,02);
  printf("innerhalb %s\n",(res->Present(test3))?"Wahr":"Falsch");
}


/*
This is the only test method and contains all tests.

*/
void runTestMethod() {
  testMove();
  testLATransform();
  //testRegion();
  testRegionCompare();
  testatinstantNoMove();
  testatinstantLinearMove();
  testatinstantRotate();
  testtraversed();
  testMBool();
}
