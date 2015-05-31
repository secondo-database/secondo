/*
This class is a FixedMRegion.

*/
#include "TestMove.h"


void
testmoveSetGet()
{
  printf ("Test testmoveSetGet: \n");
  Point3 p = Point3(true, 1.0,2.0,3.0);
  printf ("Test testmoveSetGetX: ");
  double x = p.GetX();
  printf ("%s\n", (x == 1.0) ? "OK" : "FAILED");
  
  printf ("Test testmoveSetGetY: ");
  double y = p.GetY();
  printf ("%s\n", (y == 2.0) ? "OK" : "FAILED");
  
  printf ("Test testmoveSetGetAlpha: ");
  double alpha = p.GetAlpha();
  printf ("%s\n", (alpha == 3.0) ? "OK" : "FAILED");
  
  Point3 p2 = Point3(true, 4.4,5.5,6.6);
  p.Set(p2);
  printf ("Test testmoveSetGetMinX: ");
   x = p.getMinX();
  printf ("%s\n", (x == 4.4) ? "OK" : "FAILED");
  
  printf ("Test testmoveSetGetMinY: ");
   y = p.getMinY();
  printf ("%s\n", (y == 5.5) ? "OK" : "FAILED");
  
  printf ("Test testmoveSetGetMinAlpha: ");
   alpha = p.getMinAlpha();
  printf ("%s\n", (alpha == 6.6) ? "OK" : "FAILED");
 
  p.Set(0.5, 1.5, 2.5);
  printf ("Test testmoveSetGetMaX: ");
   x = p.getMaxX();
  printf ("%s\n", (x == 0.5) ? "OK" : "FAILED");
  
  printf ("Test testmoveSetGetMaxY: ");
   y = p.getMaxY();
  printf ("%s\n", (y == 1.5) ? "OK" : "FAILED");
  
  printf ("Test testmoveSetGetMaxAlpha: ");
   alpha = p.getMaxAlpha();
  printf ("%s\n", (alpha == 2.5) ? "OK" : "FAILED");
  
  Point3 pc = Point3(p2);
  printf ("Test testmoveCopy==: ");
  bool res = (p2==pc);
  printf ("%s\n", (res == true) ? "OK" : "FAILED");
  printf ("Test testmoveCopy!=: ");
  Point3 pd = Point3(p);
  res = (pd==pc);
  printf ("%s\n", (res == false) ? "OK" : "FAILED");  
  
}

void
testmoveOperators()
{
  printf ("Test testmoveOperators: \n");
  Point3 p = Point3(true, 1.0,2.0,3.0);
  Point3 q = Point3(true, 1.0,1.0,1.0);
  Point3 s = Point3(true, 2.0,2.0,2.0);
  Point3 t = Point3(true, 0.0,0.0,0.0);  
  
  printf ("Test testmoveAlmostEqual: ");
  bool b = AlmostEqual(p,q);
  printf ("%s\n", (b == false) ? "OK" : "FAILED");
  
  printf ("Test testmove=: ");
  Point3 r = Point3(p);
  r=q;
  printf ("%s\n", (r == q) ? "OK" : "FAILED");
  
  printf ("Test testmove<=: ");
  printf ("%s\n", (q<=p) ? "OK" : "FAILED");
  
  printf ("Test testmove<: ");
  printf ("%s\n", (q<p) ? "OK" : "FAILED");

  printf ("Test testmove>=: ");
  printf ("%s\n", (p>=q) ? "OK" : "FAILED");

  printf ("Test testmove>: ");
  printf ("%s\n", (p>q) ? "OK" : "FAILED");

  printf ("Test testmove+: ");
  r=q+q;
  printf ("%s\n", ((r==s)) ? "OK" : "FAILED");
  
  printf ("Test testmove-: ");
  r=s-q;
  printf ("%s\n", ((r==q)) ? "OK" : "FAILED");  
  
  printf ("Test testmove*: ");
  r=q*2;
  printf ("%s\n", ((r==s)) ? "OK" : "FAILED");
  
  printf ("Test testmoveMidPointTo1: ");
  Point3 v = Point3(true, 1.5, 1.5, 1.5);
  r= q.MidpointTo(s);
  printf ("%s\n", ((r==v)) ? "OK" : "FAILED");
  
  printf ("Test testmoveMidPointTo1: ");
  v = Point3(true, 1.2, 1.2, 1.2);
  r= q.MidpointTo(s,0.2);
  printf ("%s\n", ((r==v)) ? "OK" : "FAILED");  
}



void
testmoveNew ()
{
  printf ("Test testmoveNew: \n");
  testmoveSetGet();
  testmoveOperators();
}
/*
This is the only test method and contains all tests.

*/
void
runTestMoveMethod ()
{
  testmoveNew ();
//  testtraversed();
}