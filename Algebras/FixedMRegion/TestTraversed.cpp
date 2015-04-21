/*
This class is a FixedMRegion.

*/
#include "TestTraversed.h"

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

bool createTestObject(double x=0.0, double y=0.0, 
  double alpha=M_PI/2, double area=1.0){
  double min1a[]={0.0, 0.0};
  double max1a[]={1.0, 0.001};
  Region *r_tmp = new Region(Rectangle<2>(true,min1a, max1a));  
  FixedMRegion fmr = FixedMRegion(1.0, 0.0, 0.0, r_tmp, 0.0, 0.0, 0.0, 
    x, y, alpha);
  Region * r = fmr.traversed(0.0, 1.0, 1.0);
  double a = r->Area();
  printf("%f\n", a);
  bool res = (a>=area-0.002) && (a<=area+0.002);
  return res;
}


void testtraversedCase0(){
  //alle Punkte auf einer Geraden
  printf("Test testtraversedCase0: ");
  bool res = createTestObject(1.0, 0.0, 0.0, 0.0);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase1(){
  //parallel
  printf("Test testtraversedCase1: ");
  bool res = createTestObject(0.0, 1.0, 0.0, 1.002);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase2(){
  //beide außerhalb
  printf("Test testtraversedCase2: ");
  bool res = createTestObject(1.0, 1.0, M_PI/2, 1.002);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase3(){
  //P0 + außerhalb
  printf("Test testtraversedCase3: ");
  bool res = createTestObject(0.0, 1.0, M_PI/2, 1.002);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase4(){
  //P0 + P0'
  printf("Test testtraversedCase4: ");
  bool res = createTestObject(0.0, 0.0, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase5(){
  //P0 + innerhalb
  printf("Test testtraversedCase5: ");
  bool res = createTestObject(0.0, -0.5, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase6(){
  //beide innerhalb
  printf("Test testtraversedCase6: ");
  bool res = createTestObject(0.5, -0.5, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase7(){
  //P0 + P1'
  printf("Test testtraversedCase7: ");
  bool res = createTestObject(0.0, -1.0, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase8(){
  //P1 + außerhalb
  printf("Test testtraversedCase8: ");
  bool res = createTestObject(1.0, 1.0, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}
}

void testtraversedCase9(){
  //P1 + P0'
  printf("Test testtraversedCase9: ");
  bool res = createTestObject(1.0, 0.001, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase10(){
  //P1 + P1'
  printf("Test testtraversedCase10: ");
  bool res = createTestObject(1.0, -1.0, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase11(){
  //P1 + innerhalb
  printf("Test testtraversedCase11: ");
  bool res = createTestObject(1.0, -0.5, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase12(){
  //außerhalb + P0'
  printf("Test testtraversedCase12: ");
  bool res = createTestObject(-1.0, 0.0, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase13(){
  //außerhalb + innerhalb
  printf("Test testtraversedCase13: ");
  bool res = createTestObject(-0.5, -0.5, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase14(){
  //außerhalb + P1'
  printf("Test testtraversedCase14: ");
  bool res = createTestObject(-1.0, -0.5, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase15(){
  //innerhalb + P0'
  printf("Test testtraversedCase15: ");
  bool res = createTestObject(0.5, 0.0, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase16(){
  //innerhalb + außerhalb
  printf("Test testtraversedCase16: ");
  bool res = createTestObject(0.5, -0.5, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCase17(){
  //innerhalb + P1'
  printf("Test testtraversedCase17: ");
  bool res = createTestObject(0.5, 1.0, M_PI/2, M_PI/4);
  printf("%s\n", (res==true)?"OK":"FAILED");
}

void testtraversedCaseA(){
  //entartet
  printf("Test testtraversedCaseA: \n");
  testtraversedCase0();
}
void testtraversedCaseB(){
  //kein Schnittpunkt
  printf("Test testtraversedCaseB: \n");
  testtraversedCase1();
}
void testtraversedCaseC(){
  //2 von 4 Punkten sind identisch
  printf("Test testtraversedCaseC: \n");
  testtraversedCase4();
  testtraversedCase7();
  testtraversedCase9();
  testtraversedCase10();
}
void testtraversedCaseD(){
  //Viereck
  printf("Test testtraversedCaseD: \n");
  testtraversedCase2();
  testtraversedCase13();
  testtraversedCase16();
}
void testtraversedCaseE(){
  //Dreieck
  printf("Test testtraversedCaseE: \n");
  testtraversedCase3();
  testtraversedCase8();
  testtraversedCase12();
  testtraversedCase14();
}
void testtraversedCaseF(){
  //2 Dreiecke
  printf("Test testtraversedCaseF: \n");
  testtraversedCase6();
  testtraversedCase15();
}
void testtraversedCaseG(){
  //Dreieck + Gerade
  printf("Test testtraversedCaseG: \n");
  testtraversedCase5();
  testtraversedCase11();
  testtraversedCase17();
}

void testtraversedNew(){
  printf("Test testtraversedNew: \n");
  testtraversedCaseA();
  testtraversedCaseB();
  testtraversedCaseC();
  testtraversedCaseD();
  testtraversedCaseE();
  testtraversedCaseF();
  testtraversedCaseG();
}

/*
This is the only test method and contains all tests.

*/
void runTestTraversedMethod() {
  testtraversedNew();
  testtraversed();
}