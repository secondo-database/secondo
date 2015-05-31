/*
This is the file with the tests.

*/
using namespace std;
#include "FixedMRegiontest.h"
#include "Secondo_Include.h"
#include <vector>
#include "../Spatial/RegionTools.h"
#include "TestTraversed.h"
#include "TestMove.h"
extern NestedList *nl;
extern QueryProcessor *qp;

/*
This method tests the class Move.

*/
void
testMove ()
{
  //Move tests
  printf ("Test Move\n");
  Move *m = new Move (0, 0, 0, 2, 3, 4);
  double *res1 = m->attime (0);
  double *res2 = m->attime (1);
  printf ("m-attime(0):%s\n", ((res1[0] == 0) && (res1[1] == 0) &&
                               (res1[2] == 0)) ? "OK" : "FAILED");
  printf ("m-attime(1):%s\n", ((res2[0] == 2) && (res2[1] == 3) &&
                               (res2[2] == 4)) ? "OK" : "FAILED");
  delete res1;
  delete res2;
  delete m;

  Move *m1 = new Move (0, 0, 0, 3, 3, 3);
  double *res11 = m1->attime (0);
  double *res21 = m1->attime (1);
  printf ("m1-attime(0):%s\n", ((res11[0] == 0) && (res11[1] == 0) &&
                                (res11[2] == 0)) ? "OK" : "FAILED");
  printf ("m1-attime(1):%s\n", ((res21[0] == 3) && (res21[1] == 3) &&
                                (res21[2] == 3)) ? "OK" : "FAILED");
  delete res11;
  delete res21;
  delete m1;

  Move *m2 = new Move (1, 1, 1, 0, 1, 0);
  double *res12 = m2->attime (0);
  double *res22 = m2->attime (1);
  printf ("m2-attime(0):%s\n", ((res12[0] == 1) && (res12[1] == 1) &&
                                (res12[2] == 1)) ? "OK" : "FAILED");
  printf ("m2-attime(1):%s\n", ((res22[0] == 1) && (res22[1] == 2) &&
                                (res22[2] == 1)) ? "OK" : "FAILED");
  delete res12;
  delete res22;
  delete m2;
}

/*
This method tests the class LATransform.

*/
void
testLATransform ()
{
  //LATransform tests
  printf ("Test LATransform\n");
  LATransform *l = new LATransform (1, 1, 0, 0, 0);
  printf ("Translation(1,0):%s\n", ((l->getImgX (1.0, 0.0) == 2) &&
                                    (l->getImgY (1.0, 0.0) ==
                                     1)) ? "OK" : "FAILED");
  printf ("Translation(0,0):%s\n", ((l->getImgX (0.0, 0.0) == 1)
                                    && (l->getImgY (0.0, 0.0) ==
                                        1)) ? "OK" : "FAILED");
  delete l;

  LATransform *l1 = new LATransform (0, 0, 0, 0, M_PI / 2);
  printf ("RotationUrsprungUmPiHalbe:%s\n",
          ((abs (l1->getImgX (1.0, 0.0) - 0.0) < 0.000001)
           && (abs (l1->getImgY (1.0, 0.0) - 1.0) <
               0.000001)) ? "OK" : "FAILED");
  delete l1;

  LATransform *l2 = new LATransform (0, 0, 0, 0, M_PI);
  printf ("RotationUrsprungUmPi:%s\n",
          ((abs (l2->getImgX (1.0, 0.0) + 1.0) < 0.000001)
           && (abs (l2->getImgY (1.0, 0.0)) < 0.000001)) ? "OK" : "FAILED");
  delete l2;

  LATransform *l3 = new LATransform (0, 0, 1, 1, 2 * M_PI);
  printf ("RotationNichtUrsprungUm2Pi:%s\n",
          ((abs (l3->getImgX (0.0, 0.0)) < 0.000001)
           && (abs (l3->getImgY (0.0, 0.0)) < 0.000001)) ? "OK" : "FAILED");
  delete l3;

  LATransform *l4 = new LATransform (0, 0, 1, 1, M_PI);
  printf ("RotationNichtUrsprungUmPi:%s\n",
          ((abs (l4->getImgX (0.0, 0.0) - 2.0) < 0.000001)
           && (abs (l4->getImgY (0.0, 0.0) - 2.0) <
               0.000001)) ? "OK" : "FAILED");
  delete l4;

  LATransform *l5 = new LATransform (1, 1, 0, 0, M_PI);
  printf ("TranslatioinMitRotationUmPi:%s\n",
          ((abs (l5->getImgX (1.0, 0.0) - 0.0) < 0.000001)
           && (abs (l5->getImgY (1.0, 0.0) - 1.0) <
               0.000001)) ? "OK" : "FAILED");
  delete l5;

  LATransform *l6 = new LATransform (1, 1, 0, 0, 2 * M_PI);
  printf ("TranslatioinMitRotationUm2Pi:%s\n",
          ((abs (l6->getImgX (1.0, 0.0) - 2.0) < 0.000001)
           && (abs (l6->getImgY (1.0, 0.0) - 1.0) <
               0.000001)) ? "OK" : "FAILED");
  delete l6;

  LATransform *l7 = new LATransform (1, 1, 1, 1, M_PI);
  printf ("TranslatioinMitRotationUm2Pi:%s\n",
          ((abs (l7->getImgX (2.0, 2.0) - 1.0) < 0.000001)
           && (abs (l7->getImgY (2.0, 2.0) - 1.0) <
               0.000001)) ? "OK" : "FAILED");
  delete l7;
}

/*
This method creates a region.

*/
void
testRegion ()
{
  printf ("Test Region\n");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *result = new Region (*p1, *p2, *p3);

  HalfSegment hs;
  for (int i = 0; i < 2; i++)
    {
      result->Get (i, hs);
      const Point l = hs.GetLeftPoint ();
      printf ("P%f=(%f,%f)\n", (double) i, l.GetX (), l.GetY ());
      Point r = hs.GetRightPoint ();
      printf ("P%f=(%f,%f)\n", (double) i, r.GetX (), r.GetY ());
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
void
testRegionCompare ()
{
  printf ("Test RegionCompare\n");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *result = new Region (*p1, *p2, *p3);
  p1 = new Point (true, 0.0, 0.0);
  p2 = new Point (true, 1.0, 0.0);
  p3 = new Point (true, 0.0, 1.0);
  Region *res2 = new Region (*p1, *p2, *p3);
  p1 = new Point (true, 0.0, 0.0);
  p2 = new Point (true, 0.0, 1.0);
  p3 = new Point (true, 1.0, 0.0);
  Region *res3 = new Region (*p1, *p2, *p3);
  printf ("Same Order: %s\n", (*result == *res2) ? "OK" : "FAILED");
  printf ("Different Order : %s\n", (*result == *res3) ? "OK" : "FAILED");
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
void
testatinstantNoMove ()
{
  printf ("Test atinstantNoMove: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *result = new Region (*p1, *p2, *p3);
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *result, 0, 0, 0, 0, 0, 0);
  Region *res2 = new Region(0);
  fmr.atinstant (2, *res2);
  printf ("%s\n", (*result == *res2) ? "OK" : "FAILED");
  delete p1;
  delete p2;
  delete p3;
  delete res2;
}

void
printHS (HalfSegment hs)
{
  printf ("((%f, %f) - (%f, %f))\n",
          hs.GetLeftPoint ().GetX (),
          hs.GetLeftPoint ().GetY (),
          hs.GetRightPoint ().GetX (), hs.GetRightPoint ().GetY ());
}

int
gotPoint (const Point & p, const Point * list, int size)
{
  printf("Point: %d, %f\n", p.GetX(), p.GetY());
  for (int i = 0; i < size; i++)
    {
      if (p.Compare (list + i) == 0)
        return i;
    }
  return -1;
}

bool
checkRegionPoints (Region * r, const Point * list, int size)
{
  bool *res = new bool[size];
  for (int i = 0; i < size; i++)
    res[i] = false;
  for (int i = 0; i < r->Size (); i++)
    {
      HalfSegment hs;
      r->Get (i, hs);
      int tmp;
      tmp = gotPoint (hs.GetLeftPoint (), list, size);
      if (tmp == -1)
        return false;
      res[tmp] = true;
      tmp = gotPoint (hs.GetRightPoint (), list, size);
      if (tmp == -1)
        return false;
      res[tmp] = true;
    }
  bool ret = true;
  for (int i = 0; i < size; i++)
    ret &= res[i];
  delete res;
  return ret;
}

/*
This method tests the method atinstant with a linear movement.

*/
void
testatinstantLinearMove ()
{
  printf ("Test atinstantLinearMove: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *result = new Region (*p1, *p2, *p3);
  Point expected[] = { Point (true, 2.0, 4.0), Point (true, 3.0, 4.0),
    Point (true, 2.0, 5.0)
  };

  FixedMRegion fmr = FixedMRegion (0, 0, 0, *result, 0, 0, 0,
                                   1, 2, 0);
  Region *res = new Region(0);
  fmr.atinstant (2, *res);
  printf ("%s\n", (checkRegionPoints (res, expected, 3)) ? "OK" : "FAILED");
  delete p1;
  delete p2;
  delete p3;
  delete res;
}

void
testatinstantRotate ()
{
  printf ("Test atinstantRotate: ");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 0.0, 1.0);
  Region *result = new Region (*p1, *p2, *p3);
  Point expected[] = { Point (true, 1.0, 0.0), Point (true, 0.0, 1.0),
    Point (true, 1.0, 1.0)
  };

  FixedMRegion fmr = FixedMRegion (1, 0.5, 0.5, *result, 0, 0, 0,
                                   0, 0, M_PI);
  Region *res = new Region(0);
  fmr.atinstant (1, *res);
  printf ("%s\n", (checkRegionPoints (res, expected, 3)) ? "OK" : "FAILED");
  delete p1;
  delete p2;
  delete p3;
  delete res;
}


void
testMBool ()
{
  printf ("Test testMBool: \n");
  MBool *res = new MBool (10);
  res->Clear ();
  res->StartBulkLoad ();
  DateTime t1 (instanttype);
  t1.Set (2015, 3, 30, 8, 01);
  DateTime t2 (instanttype);
  t2.Set (2015, 3, 30, 9, 02);
  Interval < Instant > iv (t1, t2, false, true);
  UBool ub (iv, (CcBool) true);
  res->MergeAdd (ub);

  DateTime t11 (instanttype);
  t11.Set (2015, 3, 31, 9, 02);
  DateTime t21 (instanttype);
  t21.Set (2015, 4, 30, 8, 02);
  Interval < Instant > iv2 (t11, t21, false, true);
  UBool ub2 (iv2, (CcBool) true);
  res->MergeAdd (ub2);
  res->EndBulkLoad ();

  DateTime test (instanttype);
  test.Set (2015, 3, 31, 9, 02);
  printf ("(? %s\n", (res->Present (test)) ? "Wahr" : "Falsch");

  DateTime test1 (instanttype);
  test1.Set (2015, 4, 30, 8, 02);
  printf ("]? %s\n", (res->Present (test1)) ? "Wahr" : "Falsch");

  DateTime test2 (instanttype);
  test2.Set (2015, 1, 30, 9, 02);
  printf ("außerhalb %s\n", (res->Present (test2)) ? "Wahr" : "Falsch");

  DateTime test3 (instanttype);
  test3.Set (2015, 4, 8, 8, 02);
  printf ("innerhalb %s\n", (res->Present (test3)) ? "Wahr" : "Falsch");
}

void
testQuatro ()
{
  printf ("Test testQuatro: \n");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 1.0, 1.0);
  Point *p4 = new Point (true, 0.0, 1.0);

  Region *t1 = new Region (*p1, *p2, *p3);
  Region *t2 = new Region (*p4, *p3, *p1);

//  Region *tmp1= new Region (*p1, *p2, *p4);
//  Region *tmp2=new Region(0);
//  tmp1->Union(*t1, *tmp2);

  //Point expected[]= {Point(true, 1.0, 0.0), Point(true, 0.0, 1.0), 
  //  Point(true, 1.0, 1.0)};

  //printf("%s\n", (checkRegionPoints(res, expected, 3))?"OK":"FAILED");

  Region *v1 = new Region (0);
  //t1->Union(*t2, *v1);
  RobustPlaneSweep::robustUnion (*t1, *t2, *v1);
  delete p1;
  delete p2;
  delete p3;
  delete p4;
  delete t1;
  delete t2;

  printf ("Test testQuatro6: \n");
  double m = v1->Area ();
  printf ("Flächeninhalt kleines Viereck: %f\n", m);

  Point *p5 = new Point (true, 0.0, 2.0);
  Point *p6 = new Point (true, 1.0, 2.0);
  Point *p7 = new Point (true, 1.0, 1.0);
  Point *p8 = new Point (true, 0.0, 1.0);

  Region *t3 = new Region (*p5, *p6, *p7);
  Region *t4 = new Region (*p5, *p7, *p8);

  //Point expected[]= {Point(true, 1.0, 0.0), Point(true, 0.0, 1.0), 
  //  Point(true, 1.0, 1.0)};

  //printf("%s\n", (checkRegionPoints(res, expected, 3))?"OK":"FAILED");

  Region *v2 = new Region (0);
  //t3->Union(*t4, *v2);
  RobustPlaneSweep::robustUnion (*t3, *t4, *v2);
  delete p7;
  delete p8;
  delete p5;
  delete p6;
  delete t3;
  delete t4;

  printf ("Test testQuatro7: \n");
  double m2 = v2->Area ();
  printf ("Flächeninhalt zweites kleines Viereck: %f\n", m2);

  Region *res = new Region (0);
  //v1->Union(*v2, *res);
  RobustPlaneSweep::robustUnion (*v1, *v2, *res);
  printf ("Test testQuatro12: \n");
  double m3 = res->Area ();
  printf ("Flächeninhalt großes Viereck: %f\n", m3);
  delete res;
}

void
testTriangleQuatro ()
{
  printf ("Test testTriangleQuatro: \n");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 0.5, 0.0);
  Point *p3 = new Point (true, 1.0, 0.0);
  Point *p4 = new Point (true, 1.0, 1.0);
  Region *res = new Region (0);
  printf ("Test testTriangleQuatro2: \n");
  res->StartBulkLoad ();
  printf ("Test testTriangleQuatro3: \n");
  HalfSegment h1 (true, *p1, *p2);
  HalfSegment h2 (true, *p3, *p4);
  HalfSegment h3 (true, *p4, *p1);
  HalfSegment h4 (true, *p2, *p3);
  printf ("Test testTriangleQuatro4: \n");
  *res += h1;
  *res += h2;
  *res += h3;
  *res += h4;
  printf ("Test testTriangleQuatro5: \n");
  res->EndBulkLoad ();
  printf ("Test testTriangleQuatro6: \n");
  double m = res->Area ();
  printf ("Flächeninhalt kleines Viereck: %f\n", m);
}

void
testCycle ()
{
  vector < Point > v;           //Vektor für den Polygonzug
  v.push_back (Point (true, 0.0, 0.0));
  //Die einzelnen Punkte hinzufügen

  v.push_back (Point (true, 0.0, 1.0)); //immer im Uhrzeigersinn
  v.push_back (Point (true, 1.0, 1.0));
  //gegen den Uhrzeigersinn würde es ein Loch

  v.push_back (Point (true, 1.0, 0.0));
  v.push_back (Point (true, 0.0, 0.0)); //Den ersten zum Schluss nochmal
  vector < vector < Point > >vv;        //Einen Vektor von Vektoren erzeugen
  vv.push_back (v);
  //Die einzelnen Polygonzüge hinzufügen (hier nur einer)
  Region *res = buildRegion (vv);       //Region erstellen
  printf ("Cycle Area: %f\n", res->Area ());
  delete res;
}



void
testUnion ()
{
  printf ("Test Union bei Dreiecken mit einer gleichen Kante: \n");
  Point *p1 = new Point (true, 0.0, 0.0);
  Point *p2 = new Point (true, 1.0, 0.0);
  Point *p3 = new Point (true, 1.0, 1.0);
  Point *p4 = new Point (true, 0.0, 1.0);
  Region *t1 = new Region (*p1, *p3, *p2);
  Region *t2 = new Region (*p4, *p3, *p1);
  Region *v1 = new Region (0);
  //t1->Union(*t2, *v1);
  RobustPlaneSweep::robustUnion (*t1, *t2, *v1);
  delete p1;
  delete p2;
  delete p3;
  delete p4;
  delete t1;
  delete t2;

  printf ("Test Union von überlappenden Dreiecken: \n");
  Point *p5 = new Point (true, 0.0, 2.0);
  Point *p6 = new Point (true, 1.0, 2.0);
  Point *p7 = new Point (true, 1.0, 1.0);
  Point *p8 = new Point (true, 0.0, 1.0);

  Region *t3 = new Region (*p5, *p6, *p7);
  Region *t4 = new Region (*p6, *p7, *p8);
  Region *v2 = new Region (0);
  //t3->Union(*t4, *v2);
  RobustPlaneSweep::robustUnion (*t3, *t4, *v2);
  delete p7;
  delete p8;
  delete p5;
  delete p6;
  delete t3;
  delete t4;

  printf ("Test Union von berührenden Dreiecken: \n");
  Point *p01 = new Point (true, 0.0, 0.0);
  Point *p02 = new Point (true, 1.0, 0.0);
  Point *p03 = new Point (true, 1.0, 1.0);
  Point *p04 = new Point (true, 0.0, 2.0);
  Point *p05 = new Point (true, 1.0, 2.0);

  Region *t5 = new Region (*p01, *p02, *p03);
  Region *t6 = new Region (*p03, *p04, *p05);
  Region *v3 = new Region (0);
  //t5->Union(*t6, *v3);
  RobustPlaneSweep::robustUnion (*t5, *t6, *v3);
  delete p01;
  delete p02;
  delete p03;
  delete p04;
  delete p05;
  delete t5;
  delete t6;
}


void
testconcaveQuadruple ()
{
  vector < Point > v;           //Vektor für den Polygonzug
  v.push_back (Point (true, 0.0, 0.0));
  //Die einzelnen Punkte hinzufügen

  v.push_back (Point (true, -0.5, -0.5));       //immer im Uhrzeigersinn
  v.push_back (Point (true, -0.5, 0.5));
  //gegen den Uhrzeigersinn würde es ein Loch

  v.push_back (Point (true, 1.0, 0.0));
  v.push_back (Point (true, 0.0, 0.0)); //Den ersten zum Schluss nochmal
  vector < vector < Point > >vv;        //Einen Vektor von Vektoren erzeugen
  vv.push_back (v);
  //Die einzelnen Polygonzüge hinzufügen (hier nur einer)
  Region *res = buildRegion2 (vv);      //Region erstellen
  printf ("Area should be 0.5 but is: %f\n", res->Area ());
  delete res;
}

void
testMPoint ()
{
  printf ("Test testMPoint: \n");
  MPoint *res = new MPoint (0);
  res->Clear ();
  res->StartBulkLoad ();
  DateTime t1 (instanttype);
  t1.Set (2015, 3, 30, 8, 01);
  DateTime t2 (instanttype);
  t2.Set (2015, 3, 30, 9, 02);
  Interval < Instant > iv (t1, t2, false, true);
  UPoint ub (iv, 0, 0, 1, 0);
  res->MergeAdd (ub);

  DateTime t11 (instanttype);
  t11.Set (2015, 3, 31, 9, 02);
  DateTime t21 (instanttype);
  t21.Set (2015, 4, 30, 8, 02);
  Interval < Instant > iv2 (t11, t21, false, true);
  UPoint ub2 (iv2, 2, 1, 3, 2);
  res->MergeAdd (ub2);
  res->EndBulkLoad ();

  for (int i = 0; i < res->GetNoComponents (); i++)
    {
      UPoint unit2;
      res->Get (i, unit2);

    }
  //Point *p1 = new Point (true, 0.0, 0.0);
  //Point *p2 = new Point (true, 1.0, 0.0);
  //Point *p3 = new Point (true, 1.0, 1.0);
  //Point *p4 = new Point( true, 0.0, 1.0);

  //Region *tv1 = new Region (*p1, *p2, *p3);

  //Region *v1 = new Region (0);
  //t5->Union(*t6, *v3);
//  RobustPlaneSweep::robustUnion(*tv1,*res,*v1);


//  DateTime test(instanttype);
//  test.Set(2015,3,31,9,02);
//  printf("(? %s\n",(res->Present(test))?"Wahr":"Falsch");

//  DateTime test1(instanttype);
//  test1.Set(2015,4,30,8,02);
//  printf("]? %s\n",(res->Present(test1))?"Wahr":"Falsch");

//  DateTime test2(instanttype);
//  test2.Set(2015,1,30,9,02);
//  printf("außerhalb %s\n",(res->Present(test2))?"Wahr":"Falsch");

//  DateTime test3(instanttype);
//  test3.Set(2015,4,8,8,02);
//  printf("innerhalb %s\n",(res->Present(test3))?"Wahr":"Falsch");
}

void
testMRegionConstruction ()
{
  MRegion *mr;
  {
    Region r (Point (0.0, 0.0), Point (1.0, 0.0), Point (1.0, 1.0));
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
    //Stillstehende MovingRegion erzeugen
    mr = new MRegion (rock, r);
  }

  MPoint *res = new MPoint (0);
  res->Clear ();
  res->StartBulkLoad ();
  DateTime t1 (instanttype);
  t1.Set (2015, 3, 30, 8, 01);
  DateTime t2 (instanttype);
  t2.Set (2015, 3, 30, 9, 02);
  Interval < Instant > iv (t1, t2, false, true);
  UPoint ub (iv, 0, 0, 1, 0);
  res->MergeAdd (ub);

  DateTime t11 (instanttype);
  t11.Set (2015, 3, 31, 9, 02);
  DateTime t21 (instanttype);
  t21.Set (2015, 4, 30, 8, 02);
  Interval < Instant > iv2 (t11, t21, false, true);
  UPoint ub2 (iv2, 2, 1, 3, 2);
  res->MergeAdd (ub2);
  res->EndBulkLoad ();

  for (int i = 0; i < res->GetNoComponents (); i++)
    {
      UPoint unit2;
      res->Get (i, unit2);
      Point p0 = unit2.p0;
      Point p1 = unit2.p1;
      Interval < Instant > ii = unit2.getTimeInterval ();
      Instant start = ii.start;
      Instant end = ii.end;
      bool lc = ii.lc;
      bool rc = ii.rc;
      lc=rc;
    }
  MBool res1 (0);
  MPoint res2 (0);

  mr->Intersection (*res, res2);
  mr->Inside (*res, res1);
//  DateTime test(instanttype);
//  test.Set(2015,3,31,9,02);
//  printf("(? %s\n",(res->Present(test))?"Wahr":"Falsch");

//  DateTime test1(instanttype);
//  test1.Set(2015,4,30,8,02);
//  printf("]? %s\n",(res->Present(test1))?"Wahr":"Falsch");

//  DateTime test2(instanttype);
//  test2.Set(2015,1,30,9,02);
//  printf("außerhalb %s\n",(res->Present(test2))?"Wahr":"Falsch");

//  DateTime test3(instanttype);
//  test3.Set(2015,4,8,8,02);
//  printf("innerhalb %s\n",(res->Present(test3))?"Wahr":"Falsch");

}

bool
checkTransform (LATransform * t, double x, double y)
{
  double tmpx, tmpy;
  double fx, fy;
  tmpx = t->getImgX (x, y);
  tmpy = t->getImgY (x, y);
  fx = t->getOrigX (tmpx, tmpy);
  fy = t->getOrigY (tmpx, tmpy);
  printf ("(%f, %f) -> (%f, %f) -> (%f, %f) ", x, y, tmpx, tmpy, fx, fy);
  return (fabs (x - fx) < 0.000001) && ((fabs (y - fy) < 0.000001));
}

void
testLATransformInv ()
{
  //LATransform tests
  bool res;
  printf ("Test LATransform\n");
  LATransform *l = new LATransform (1, 1, 0, 0, 0);
  printf ("Test1: ");
  res = checkTransform (l, 1.0, 0.0);
  printf ("%s\n", (res) ? "OK" : "FAILED");
  delete l;

  l = new LATransform (0, 0, 0, 0, M_PI / 2);
  printf ("Test1: ");
  res = checkTransform (l, 1.0, 0.0);
  printf ("%s\n", (res) ? "OK" : "FAILED");
  delete l;

  l = new LATransform (0, 0, 1, 1, 2 * M_PI);
  printf ("Test2: ");
  res = checkTransform (l, 0.0, 0.0);
  printf ("%s\n", (res) ? "OK" : "FAILED");
  delete l;

  l = new LATransform (0, 0, 1, 1, M_PI);
  printf ("Test3: ");
  res = checkTransform (l, 0.0, 0.0);
  printf ("%s\n", (res) ? "OK" : "FAILED");
  delete l;

  l = new LATransform (0, 0, 1, 1, M_PI / 4);
  printf ("Test4: ");
  res = checkTransform (l, 4.0, 5.0);
  printf ("%s\n", (res) ? "OK" : "FAILED");
  delete l;
}


void testInside1()
{
  //inside test 
  //innerhalb+außerhalb
  printf ("Test inside inner- und außerhalb: ");

  DateTime t_start(0.0);
  DateTime t_end(1.0);

  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion(*rbig,
                                  Point(true, 0, 0), 0.0, t_start,
                                  Point(true, 0, 1), 0.0, t_end,
                                  Point(true, 0, 0));

  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, false, true);
  UPoint ub (iv, 0, 0.5, 2, 1.5);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MBool expected (0);
  expected.Clear();
  expected.StartBulkLoad();
  Interval < Instant > ive (t_start, DateTime(0.5), false, true);
  UBool ube (ive, (CcBool) true);
  expected.MergeAdd (ube);
  expected.EndBulkLoad ();
  
  MBool result=fmr.inside(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      //result.Get(i, up);
      //printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
      //up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
      //up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }
}

void testInside2()
{
  //inside test 
  //außerhalb
  printf ("Test inside kein Schnitt: ");
    
  DateTime t_start(0.0);
  DateTime t_end(1.0);

  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion(*rbig,
                                  Point(true, 0, 0), 0.0, t_start,
                                  Point(true, 0, 1), 0.0, t_end,
                                  Point(true, 0, 0));
  
  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, false, true);
  UPoint ub (iv, 0, -0.5, 2, -1.5);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MBool expected (0);
  expected.Clear();
  expected.StartBulkLoad();
  expected.EndBulkLoad ();
  
  MBool result=fmr.inside(res);
  

  if (result==expected) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      //result.Get(i, up);
      //printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
      //up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
      //up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }
}

void testInside3()
{
  //inside test 
  //innerhalb
  printf ("Test inside nur innerhalb: ");
  
  DateTime t_start(0.0);
  DateTime t_end(1.0);

  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion(*rbig,
                                  Point(true, 0, 0), 0.0, t_start,
                                  Point(true, 0, 1), 0.0, t_end,
                                  Point(true, 0, 0));

  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, false, true);
  UPoint ub (iv, 0, 0.5, 1.0, 1.5);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MBool expected (0);
  expected.Clear();
  expected.StartBulkLoad();
  Interval < Instant > ive (t_start, t_end, false, true);
  UBool ube (ive, (CcBool) true);
  expected.MergeAdd (ube);
  expected.EndBulkLoad ();
  
  MBool result=fmr.inside(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      //result.Get(i, up);
      //printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
      //up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
      //up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }
}
/*
void testInside4()
{
  //inside test 
  //mehrfacher Wechsel
  printf ("Test inside wechselt: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, rbig, 0, 0, 0,
    0, 1, 0);
  
  MPoint *res = new MPoint (0);
  res->Clear ();
  res->StartBulkLoad ();
  
  DateTime t1 (instanttype);
  t1.Set (2015, 3, 30, 8, 01);
  DateTime t2 (instanttype);
  t2.Set (2015, 3, 30, 9, 00);
  Interval < Instant > iv (t1, t2, false, true);
  UPoint ub (iv, 0, 0.5, 1, 1.5);
  res->MergeAdd (ub);

  DateTime t3 (instanttype);
  t3.Set (2015, 3, 30, 9, 01);
  DateTime t4 (instanttype);
  t4.Set (2015, 3, 30, 10, 00);
  Interval < Instant > iv2 (t3, t4, false, true);
  UPoint ub2 (iv2, 0, -1.0, 1, -1.5);
  res->MergeAdd (ub2);
  
  DateTime t5 (instanttype);
  t5.Set (2015, 3, 30, 10, 01);
  DateTime t6 (instanttype);
  t6.Set (2015, 3, 30, 11, 00);
  Interval < Instant > iv3 (t5, t6, false, true);
  UPoint ub3 (iv3, 0, 0.5, 1, 1.5);
  res->MergeAdd (ub3);
  
  DateTime t7 (instanttype);
  t7.Set (2015, 3, 30, 11, 01);
  DateTime t8 (instanttype);
  t8.Set (2015, 3, 30, 12, 00);
  Interval < Instant > iv4 (t7, t8, false, true);
  UPoint ub4 (iv4, 0, -0.5, 1, -1.5);
  res->MergeAdd (ub4);
  
  res->EndBulkLoad ();


  MBool result=fmr.inside(*res);
  printf("innerhalb %s\n",(result->true(ub))?"Wahr":"Falsch");
  
}
*/
void testInside(){
  printf ("Test inside\n");
  testInside1();
  testInside2();
  testInside3();
  //testInside4();
}




void testIntersection1()
{
  //intersection test 
  //innerhalb+außerhalb
  printf ("Test intersection inner- und außerhalb: ");

  DateTime t_start(0.0);
  DateTime t_end(1.0);

  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion(*rbig,
                                  Point(true, 0, 0), 0.0, t_start,
                                  Point(true, 0, 1), 0.0, t_end,
                                  Point(true, 0, 0));

  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, false, true);
  UPoint ub (iv, 0, 1, 0, 0);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MPoint expected (0);
  expected.Clear();
  expected.StartBulkLoad();
  Interval < Instant > ive (t_start, DateTime(0.5), false, true);
  UPoint ube (ive, 0, 0.5, 1, 1.0);
  expected.MergeAdd (ube);
  expected.EndBulkLoad ();
  
  MPoint result=fmr.intersection(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      result.Get(i, up);
      printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
             up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
             up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }
}

void testIntersection2()
{
  //intersection test 
  //außerhalb
  printf ("Test intersection kein Schnitt: ");
  DateTime t_start(0.0);
  DateTime t_end(1.0);

  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
   FixedMRegion fmr = FixedMRegion(*rbig,
                                  Point(true, 0, 0), 0.0, t_start,
                                  Point(true, 0, 1), 0.0, t_end,
                                  Point(true, 0, 0));
  
  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, false, true);
  UPoint ub (iv, 0, -0.5, 1, -1.5);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MPoint expected (0);
  expected.Clear();
  expected.StartBulkLoad();

  expected.EndBulkLoad ();
  
  MPoint result=fmr.intersection(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      result.Get(i, up);
      printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
             up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
             up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }
}

void testIntersection3()
{
  //intersection test 
  //innerhalb
  printf ("Test intersection nur innerhalb: ");
    DateTime t_start(0.0);
  DateTime t_end(1.0);

  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
   FixedMRegion fmr = FixedMRegion(*rbig,
                                  Point(true, 0, 0), 0.0, t_start,
                                  Point(true, 0, 1), 0.0, t_end,
                                  Point(true, 0, 0));
  
  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, false, true);
  UPoint ub (iv, 0, 0.5, 1, 1.5);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MPoint expected (0);
  expected.Clear();
  expected.StartBulkLoad();
  Interval < Instant > ive (t_start, t_end, false, true);
  UPoint ube (ive, 0, 0.5, 1, 1.5);
  expected.MergeAdd (ube);
  expected.EndBulkLoad ();
  
  MPoint result=fmr.intersection(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("Failed\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      result.Get(i, up);
      printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
             up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
             up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }
}
/*
void testIntersection4()
{
  //intersection test 
  //mehrfacher Wechsel
  printf ("Test intersection wechselt: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, rbig, 0, 0, 0,
    0, 1, 0);
  
  MPoint *res = new MPoint (0);
  res->Clear ();
  res->StartBulkLoad ();
  
  DateTime t1 (instanttype);
  t1.Set (2015, 3, 30, 8, 01);
  DateTime t2 (instanttype);
  t2.Set (2015, 3, 30, 9, 00);
  Interval < Instant > iv (t1, t2, false, true);
  UPoint ub (iv, 0, 0.5, 1, 1.5);
  res->MergeAdd (ub);

  DateTime t3 (instanttype);
  t3.Set (2015, 3, 30, 9, 01);
  DateTime t4 (instanttype);
  t4.Set (2015, 3, 30, 10, 00);
  Interval < Instant > iv2 (t3, t4, false, true);
  UPoint ub2 (iv2, 0, -1.0, 1, -1.5);
  res->MergeAdd (ub2);
  
  DateTime t5 (instanttype);
  t5.Set (2015, 3, 30, 10, 01);
  DateTime t6 (instanttype);
  t6.Set (2015, 3, 30, 11, 00);
  Interval < Instant > iv3 (t5, t6, false, true);
  UPoint ub3 (iv3, 0, 0.5, 1, 1.5);
  res->MergeAdd (ub3);
  
  DateTime t7 (instanttype);
  t7.Set (2015, 3, 30, 11, 01);
  DateTime t8 (instanttype);
  t8.Set (2015, 3, 30, 12, 00);
  Interval < Instant > iv4 (t7, t8, false, true);
  UPoint ub4 (iv4, 0, -0.5, 1, -1.5);
  res->MergeAdd (ub4);
  
  res->EndBulkLoad ();


  MPoint result=fmr.intersection(*res);
  printf("innerhalb %s\n",(result->true(ub))?"Wahr":"Falsch");
  
}
*/



void testIntersection()
{
  //intersection tests
  printf ("Test intersection\n");
  testIntersection1();
//  testIntersection2();
//  testIntersection3();
//  testIntersection4();
}

void FMRTest::testgenerateListOfRegionPoints(){
  printf ("Test generateListOfRegionPoints: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  vector<Point> v;
  v.push_back(Point(0.0,0.0));
  v.push_back(Point(1.0,0.0));
  v.push_back(Point(0.0,1.0));
  v.push_back(Point(1.0,1.0));
  vector<Point> res = fmr.generateListOfRegionPoints(*rbig);
  
  if (res==v) {
    printf("OK\n");
  } else {
    printf("Failed\n");
  }
}

void FMRTest::testgetOneDistance(){
  printf ("Test getOneDistance: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  Point p1 =  Point(true, 0.0,0.0);
  Point p2 = Point(true, 1.0,0.0);
     //printf("p2.x= %f\n", p2.GetX());
  double d = fmr.getOneDistance(p1,p2);
  //printf("%f\n", d);
  if (d==1.0) {
    printf("OK\n");
  } else {
    printf("Failed\n");
  }
}

void FMRTest::testgetDistancesForPoint(){
  printf ("Test getDistancesForPoint: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  vector<Point> v;
  v.push_back(Point(true, 0.0,0.0));
  v.push_back(Point(true, 1.0,0.0));
  v.push_back(Point(true, 0.0,1.0));
  v.push_back(Point(true, -2.0,0.0));
  vector<double> d = fmr.getDistancesForPoint(0, v);
  vector<double> res;
  res.push_back(0.0);
  res.push_back(1.0);
  res.push_back(1.0);
  res.push_back(2.0);
  
  if (res==d) {
    printf("OK\n");
  } else {
    printf("Failed\n");
  }
}

void FMRTest::testgenerateDistancesMatrix(){
  printf ("Test generateDistancesMatrix: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  vector<Point> v;
  v.push_back(Point(true, 0.0,0.0));
  v.push_back(Point(true, 1.0,0.0));
  vector<vector<double> > calc = fmr.generateDistancesMatrix(v);
  vector<double> res1;
  res1.push_back(0.0);
  res1.push_back(1.0);
  vector<double> res2;
  res2.push_back(1.0);
  res2.push_back(0.0);
  vector<vector<double> > result(0);
  result.push_back(res1);
  result.push_back(res2);
  
  if (result==calc) {
    printf("OK\n");
  } else {
    printf("Failed\n");
  }
}

void FMRTest::testidentifyPoint(){
  printf ("Test identifyPoint: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  vector<double> v;
  v.push_back(0.0);
  v.push_back(1.0);
  vector<double> v2;
  v2.push_back(1.0);
  v2.push_back(0.0);
  vector<vector<double> > vv(0);
  vv.push_back(v);
  vv.push_back(v2);
  int calc = fmr.identifyPoint(vv, v);
    
  if (calc==0) {
    printf("OK\n");
  } else {
    printf("Failed\n");
  }
}

void FMRTest::testidentifyPoints(){
  printf ("Test identifyPoints: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  vector<double> v;
  v.push_back(0.0);
  v.push_back(1.0);
  vector<double> v2;
  v2.push_back(1.0);
  v2.push_back(0.0);
  vector<vector<double> > vv(0);
  vv.push_back(v);
  vv.push_back(v2);
  vector<vector<double> > vv2(0);
  vv2.push_back(v2);
  vv2.push_back(v);
  vector<int> res(0);
  res.push_back(1);
  res.push_back(0);
  vector<int> result = fmr.identifyPoints(vv,vv2);
  //FIXME: Wird die Reihenfolge beachtet?
  if (result==res) {
    printf("OK\n");
  } else {
    printf("Failed\n");
  }
}
  

void testskeleton(){
  printf ("Test intersection\n");
  FMRTest t;
  t.testgenerateListOfRegionPoints();
  t.testgetOneDistance();
  t.testgetDistancesForPoint();
  t.testgenerateDistancesMatrix();
  t.testidentifyPoint();
  t.testidentifyPoints();
}

/*
This is the only test method and contains all tests.

*/
void
runTestMethod ()
{
//  testMove();
//  testLATransform();
//  testRegion();
//  testRegionCompare();
//  testatinstantNoMove();
//  testatinstantLinearMove(); //FIXME
//  testatinstantRotate();
//  testMBool();
//  testQuatro();
//  testCycle();
//  testTriangleQuatro(); //DEPRECATED
//  testUnion();
  runTestTraversedMethod();
  runTestMoveMethod();
  testconcaveQuadruple();
  testMPoint();
  testLATransformInv ();
  testInside();
  testIntersection();
//  testskeleton();
}
