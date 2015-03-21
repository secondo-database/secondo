/*
This is the file with the tests.

*/
using namespace std;
#include FixedMRegiontest.h";
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
  
  Move *m1 = new Move(0,0,0,3,3,3);
  double *res11=m1->attime(0);
  double *res21=m1->attime(1);
  printf("m1-attime(0):%s\n",((res11[0]==0)&&(res11[1]==0)&&
  (res11[2]==0))?"OK":"FAILED");
  printf("m1-attime(1):%s\n",((res21[0]==3)&&(res21[1]==3)&&
  (res21[2]==3))?"OK":"FAILED");
  delete res11;
  delete res21;
  
  Move *m2 = new Move(1,1,1,0,1,0);
  double *res12=m2->attime(0);
  double *res22=m2->attime(1);
  printf("m2-attime(0):%s\n",((res12[0]==1)&&(res12[1]==1)&&
  (res12[2]==1))?"OK":"FAILED");
  printf("m2-attime(1):%s\n",((res22[0]==1)&&(res22[1]==2)&&
  (res22[2]==1))?"OK":"FAILED");
  delete res12;
  delete res22;
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
This method tests the class LATransform.

*/
void testRegion(){
  //Region tests
  printf("Test Region\n");
  Point *p1 = new Point( true, 0.0, 0.0);
  Point *p2 = new Point( true, 1.0, 0.0);
  Point *p3 = new Point( true, 0.0, 1.0);
  Region *result= new Region(*p1, *p2, *p3);
  
  //result->Clear();
  //if( !IsDefined() ) {
  //  result->SetDefined( false );
  //  return;
  //}
  //result->SetDefined( true );
  //assert( IsOrdered() );
  HalfSegment hs;
  //result->StartBulkLoad();
  for( int i = 0; i <2; i++ )
  {
    result->Get( i, hs );
    //hs.Translate( x, y );
    const Point l=hs.GetLeftPoint();
    printf("P%f=(%f,%f)\n",(double)i,l.GetX(),l.GetY());
    Point r=hs.GetRightPoint();
    printf("P%f=(%f,%f)\n",(double)i,r.GetX(),r.GetY());
    //result += hs;
  }
  //result->SetNoComponents( NoComponents() );
  //result->EndBulkLoad( false, false, false, false );
}

/*
This is the only test method and contains all tests.

*/
void runTestMethod() {
  testMove();
  testLATransform();
  testRegion();
}
