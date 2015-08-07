/*
This class is a FixedMRegion.

*/
#include "TestMove.h"



void testInside1()
{
  //inside test 
  //innerhalb+außerhalb
  printf ("Test inside inner- und außerhalb: ");

  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  DateTime t_start(0.0);
  DateTime t_end(1.0);
  
  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, false, true);
  UPoint ub (iv, 0, 1.0, 2, 1.0);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MBool expected (0);
  expected.Clear();
  expected.StartBulkLoad();
  DateTime tt(0.5);
  Interval < Instant > ive (t_start, tt, false, false);
  UBool ube (ive, (CcBool) true);
  expected.MergeAdd (ube);
  Interval<Instant> ive2(tt, t_end, true, true);
  UBool ube2(ive2, (CcBool) false);
  expected.MergeAdd (ube2);
  expected.EndBulkLoad ();
  
  MBool result=fmr.inside(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
    printf("Components:%d\n", result.GetNoComponents());
    for (int i=0; i<result.GetNoComponents(); i++) {
      UBool up;
      result.Get(i, up);
      printf("%d: %s %f-%f\n", i, (up.constValue==(CcBool)false)?"false":"true",
      up.getTimeInterval().start.ToDouble(),
      up.getTimeInterval().end.ToDouble());
      cout << up.constValue << "\n";
    }
  }  
}

void testInside2()
{
  //inside test 
  //außerhalb
  printf ("Test inside kein Schnitt: ");
    
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  DateTime t_start(0.0);
  DateTime t_end(1.0);
  
  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, true, true);
  UPoint ub (iv, 0, -0.6, 2, -1.4);
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
    printf("FAILED\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UBool up;
      result.Get(i, up);
      printf("%d: %s %f-%f\n", i, (up.constValue==(CcBool)true)?"true":"false", 
      up.getTimeInterval().start.ToDouble(),
      up.getTimeInterval().end.ToDouble());
    }
  }
}

void testInside3()
{
  //inside test 
  //innerhalb
  printf ("Test inside nur innerhalb: ");
  
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  DateTime t_start(0.0);
  DateTime t_end(1.0);
  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, true, true);
  UPoint ub (iv, 0, 0.5, 1.0, 1.5);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MBool expected (0);
  expected.Clear();
  expected.StartBulkLoad();
  Interval < Instant > ive (t_start, t_end, true, true);
  UBool ube (ive, (CcBool) true);
  expected.MergeAdd (ube);
  expected.EndBulkLoad ();
  
  MBool result=fmr.inside(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UBool up;
      result.Get(i, up);
      printf("%d: %s %f-%f\n", i, (up.constValue==(CcBool)true)?"true":"false", 
      up.getTimeInterval().start.ToDouble(),
      up.getTimeInterval().end.ToDouble());
    }
  }
}

void testInside4()
{
  //inside test 
  //mehrfacher Wechsel
  printf ("Test inside wechselt: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  DateTime t_start(0.0);
  DateTime t_end(1.0);
  
  MPoint res (0);
  res.Clear ();
  res.StartBulkLoad ();
  
  DateTime t1 (instanttype);
  t1.Set (2015, 3, 30, 8, 01);
  DateTime t2 (instanttype);
  t2.Set (2015, 3, 30, 9, 00);
  Interval < Instant > iv (t1, t2, true, true);
  UPoint ub (iv, 0, 0.5, 1, 1.5);
  res.MergeAdd (ub);

  DateTime t3 (instanttype);
  t3.Set (2015, 3, 30, 9, 01);
  DateTime t4 (instanttype);
  t4.Set (2015, 3, 30, 10, 00);
  Interval < Instant > iv2 (t3, t4, false, true);
  UPoint ub2 (iv2, 0, -1.0, 1, -1.5);
  res.MergeAdd (ub2);
  
  DateTime t5 (instanttype);
  t5.Set (2015, 3, 30, 10, 01);
  DateTime t6 (instanttype);
  t6.Set (2015, 3, 30, 11, 00);
  Interval < Instant > iv3 (t5, t6, false, true);
  UPoint ub3 (iv3, 0, 0.5, 1, 1.5);
  res.MergeAdd (ub3);
  
  DateTime t7 (instanttype);
  t7.Set (2015, 3, 30, 11, 01);
  DateTime t8 (instanttype);
  t8.Set (2015, 3, 30, 12, 00);
  Interval < Instant > iv4 (t7, t8, false, true);
  UPoint ub4 (iv4, 0, -0.5, 1, -1.5);
  res.MergeAdd (ub4);
  
  res.EndBulkLoad ();


  MBool result=fmr.inside(res);
  //printf("innerhalb %s\n",(result->true(ub))?"Wahr":"Falsch");
  printf("FAILED\n");
}

void testInside(){
  printf ("Test inside\n");
//  testInside1();
  testInside2();
  testInside3();
  testInside4();
}




void testIntersection1()
{
  //intersection test 
  //innerhalb+außerhalb
  printf ("Test intersection inner- und außerhalb: ");

  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  DateTime t_start(0.0);
  DateTime t_end(1.0);
  
  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, true, true);
  UPoint ub (iv, 0, 1, 0, 0);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MPoint expected (0);
  expected.Clear();
  expected.StartBulkLoad();
  Interval < Instant > ive (t_start, DateTime(0.5), false, true);
  UPoint ube (ive, 0, 1, 0, 0.5);
  expected.MergeAdd (ube);
  expected.EndBulkLoad ();
  
  //printf("FAILED\n");
  MPoint result=fmr.intersection(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      result.Get(i, up);
  //    printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
  //        up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
  //       up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }
  
}

void testIntersection2()
{
  //intersection test 
  //außerhalb
  printf ("Test intersection kein Schnitt: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  DateTime t_start(0.0);
  DateTime t_end(1.0);
  MPoint res (0);
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, true, true);
  UPoint ub (iv, 0, -0.5, 1, -1.5);
  res.MergeAdd (ub);
  res.EndBulkLoad ();

  MPoint expected (0);
  expected.Clear();
  expected.StartBulkLoad();

  expected.EndBulkLoad ();
    printf("FAILED\n");
  /*
  MPoint result=fmr.intersection(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      result.Get(i, up);
      printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
             up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
             up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }*/
}

void testIntersection3()
{
  //intersection test 
  //innerhalb
  printf ("Test intersection nur innerhalb: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  DateTime t_start(0.0);
  DateTime t_end(1.0);
  MPoint res (0);
  
  res.Clear();
  res.StartBulkLoad();
  Interval < Instant > iv (t_start, t_end, true, true);
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
  printf("FAILED\n");
  /*
  MPoint result=fmr.intersection(res);
  
  if (result==expected) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
    
    for (int i=0; i<result.GetNoComponents(); i++) {
      UPoint up;
      result.Get(i, up);
      printf("%d: (%f, %f) @ %f - (%f, %f) @ %f\n", i,
             up.p0.GetX(), up.p0.GetY(), up.getTimeInterval().start.ToDouble(),
             up.p1.GetX(), up.p1.GetY(), up.getTimeInterval().end.ToDouble());
    }
  }*/
}

void testIntersection4()


{
  //intersection test 
  //mehrfacher Wechsel
  printf ("Test intersection wechselt: ");
  double min[] = { 0.0, 0.0 };
  double max[] = { 1.0, 1.0 };
  Region *rbig = new Region (Rectangle < 2 > (true, min, max));
  FixedMRegion fmr = FixedMRegion (0, 0, 0, *rbig, 0, 0, 0,
    0, 1, 0);
  DateTime t_start(0.0);
  DateTime t_end(1.0);
  
  MPoint *res = new MPoint (0);
  res->Clear ();
  res->StartBulkLoad ();
  
  DateTime t1 (instanttype);
  t1.Set (2015, 3, 30, 8, 01);
  DateTime t2 (instanttype);
  t2.Set (2015, 3, 30, 9, 00);
  Interval < Instant > iv (t1, t2, true, true);
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
  printf("FAILED\n");
/*
  MPoint result=fmr.intersection(*res);
  printf("innerhalb %s\n",(result->true(ub))?"Wahr":"Falsch");
  */
}




void testIntersection()
{
  //intersection tests
  printf ("Test intersection\n");
  testIntersection1();
  testIntersection2();
  testIntersection3();
  testIntersection4();
}



/*
This is the only test method and contains all tests.

*/
void
runtestInsideIntersection ()
{
  testInside();
  testIntersection();

}