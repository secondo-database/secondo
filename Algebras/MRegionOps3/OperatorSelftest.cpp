/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]
//[x] [$\times $]
//[->] [$\rightarrow $]

[1] Implementation of the MRegionOps3Algebra

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "OperatorSelftest.h"

using namespace std;

namespace temporalalgebra {
  namespace mregionops3 {
    
    class Selftest{
    public:  
      Selftest(){
        numberOfTestsRun=0;
        numberOfTestsFailed=0;
      }// Konstruktor
    
      ~Selftest(){
      }// Destruktortor
    
      bool run(){
        Point3DTest1();
        Point2DTest1();
        
        RationalPoint3DTest1();
        RationalPoint3DTest2();
        
        RationalVector3DTest1();
        RationalVector3DTest2();
        RationalVector3DTest3();
        
        Segment3DTest();
        RationalSegment3DTest1();
        RationalSegment3DTest2();
        
        Point2DTest1();
        
        RationalPoint2DTest1();
        
        RationalVector2DTest1();
        
        Segment2DTest1();
        Segment2DTest2();
        
        RationalPoint3DExtTest();
        RationalPoint3DExtSetTest1();
        RationalPoint3DExtSetTest2();
        RationalPoint3DExtSetTest3();
        RationalPoint3DExtSetTest4();
        
        RationalPlane3DTest1();
        RationalPlane3DTest2();
        RationalPlane3DTest3();
        
        IntersectionPointTest();
        IntersectionSegmentTest();
        
        IntSegContainerTest1();
        IntSegContainerTest2();
        
        PFaceTesst1();
        PFaceTesst2();
        PFaceTesst3();
        PFaceTesst4();
        PFaceTesst5();
        PFaceTesst6();
        
        
        cerr << endl;
        cerr << numberOfTestsRun << " tests run, ";
        cerr << numberOfTestsFailed << " tests failed." << endl <<endl;  
        return (numberOfTestsFailed==0);
      }// run
    private:
      void assert_(string test,  string message, bool success){
        numberOfTestsRun++;
        if(!success){
          numberOfTestsFailed++;
          cerr << "Test failed: "<< test << ": "  << message << endl;
        }// if
      }// assert_   
      
      void Point3DTest1(){
        Point3D point1;
        Point3D point2(0,0,0);
        Point3D point3(1,2,3);
        Point3D point4(point3);
        Point3D point5 = point3;
        assert_("Point3DTest 1.1", "points aren't equal.", 
                point1 == point2);
        assert_("Point3DTest 1.2", "points aren't equal.", 
                point3 == point4);
        assert_("Point3DTest 1.3", "points aren't equal.", 
                point5.getX() == 1 && point5.getY() == 2 &&
                point5.getZ() == 3);
        // cout << point5 <<endl;
      }// Point3DTest1
      
      void RationalPoint3DTest1(){
        RationalPoint3D point1;
        RationalPoint3D point2(0,0,0);
        RationalPoint3D point3(1,2,3);
        RationalPoint3D point4(point3);
        RationalPoint3D point5 = point3;
        assert_("RationalPoint3DTest 1.1", "points aren't equal.", 
                point1 == point2);
        assert_("RationalPoint3DTest 1.2", "points aren't equal.", 
                point3 == point4);
        assert_("RationalPoint3DTest 1.3", "points aren't equal.",
                point5.getX() == 1 && point5.getY() == 2 && 
                point5.getZ() == 3);
        // cout << point5 <<endl;
      }// Point3DTest1
      
      void RationalPoint3DTest2(){
        Point3D point1(1,2,3);
        RationalPoint3D point2(1,2,3);
        assert_("RationalPoint3DTest 2.1", "points aren't equal.", 
                point1.getR() == point2);
        assert_("RationalPoint3DTest 2.2", "points aren't equal.", 
                point1 == point2.getD());
        // cout << point5 <<endl;
      }// Point3DTest2
      
      void RationalVector3DTest1(){
        RationalVector3D vector1;
        RationalVector3D vector2(0,0,0);
        RationalVector3D vector3(1,2,3);
        RationalVector3D vector4(vector3);
        RationalVector3D vector5 = vector3;
        assert_("RationalVector3DTest 1.1", "vectors aren't equal.", 
                vector1 == vector2);
        assert_("RationalVector3DTest 1.2", "vectors aren't equal.", 
                vector3 == vector4);
        assert_("RationalVector3DTest 1.3", "vectors aren't equal.", 
                vector5.getX() == 1 && vector5.getY() == 2 &&
                vector5.getZ() == 3);
        // cout << vector5 << endl;
      }// RationalVector3DTest1
      
      void RationalVector3DTest2(){
        RationalVector3D vector1(1,2,3);
        RationalVector3D vector2 = -vector1;
        mpq_class value1 = -1;
        RationalVector3D vector3 = value1 * vector2;
        RationalVector3D vector4 = vector1 * value1;
        assert_("RationalVector3DTest 2.1", "vectors aren't equal.", 
                vector1 == vector3);
        assert_("RationalVector3DTest 2.2", "vectors aren't equal.", 
                vector2 == vector4);
        mpq_class        value2  = vector1 * vector2;         
        RationalVector3D vector5 = vector1 ^ vector2;  
        assert_("RationalVector3DTest 2.3", "values aren't equal.", 
                value2 == -14);
        assert_("RationalVector3DTest 2.4", "vectors aren't equal.", 
                vector5 == RationalVector3D(0,0,0));                
        // cout << vector1 << endl;
        // cout << vector2 << endl;
        // cout << vector3 << endl;
        // cout << vector4 << endl;                
        // cout << value2 << endl;
        // cout << vector5 << endl;                                
      }// RationalVector3DTest2
      
       void RationalVector3DTest3(){
        Point3D point1(1,2,3);
        Point3D point2(10,11,9);
        RationalVector3D vector1 = point2.getR() - point1.getR();
        RationalVector3D vector2(9,9,6);
        Point3D point3 = (point1.getR() + vector1).getD();
        Point3D point4 = (point2.getR() - vector1).getD();
        assert_("RationalVector3DTest 3.1", "vectors aren't equal.", 
                vector1 == vector2);
        assert_("RationalVector3DTest 3.2", "points aren't equal.", 
                point2 == point3);
        assert_("RationalVector3DTest 3.3", "points aren't equal.", 
                point1 == point4);    
        // cout << vector1 << endl;
        // cout << point3 << endl;
        // cout << point4 << endl;       
      }// Point3DTest1
      
      void Segment3DTest(){
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);  
        Point3D point3(3,4,1);
        Segment3D segment1(point1,point2);
        Segment3D segment2(point1,point3);
        Segment3D segment3 = segment1;
        assert_("Segment3DTest 1.1", "points aren't equal.", 
                 segment1.getTail() == point1);
        assert_("Segment3DTest 1.2", "points aren't equal.", 
                 segment1.getHead() == point2); 
        assert_("Segment3DTest 1.3", "segments are equal.", 
                 !(segment1 == segment2)); 
        assert_("Segment3DTest 1.4", "segments aren't equal.", 
                 segment1 == segment3); 
        // cout << segment1 << endl; ;
      }// Segment3DTest
      
      void RationalSegment3DTest1(){
        RationalPoint3D point1(1,2,0);
        RationalPoint3D point2(3,4,0);  
        RationalPoint3D point3(3,4,1);
        RationalSegment3D segment1(point1,point2);
        RationalSegment3D segment2(point1,point3);
        RationalSegment3D segment3 = segment1;
        assert_("RationalSegment3DTest 1.1", "points aren't equal.", 
                 segment1.getTail() == point1);
        assert_("RationalSegment3DTest 1.2", "points aren't equal.", 
                 segment1.getHead() == point2); 
        assert_("RationalSegment3DTest 1.3", "segments are equal.", 
                 !(segment1 == segment2)); 
        assert_("RationalSegment3DTest 1.4", "segments aren't equal.", 
                 segment1 == segment3); 
        // cout << segment1 << endl; ;
      }// RationalSegment3DTest1
      
      void RationalSegment3DTest2(){
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);  
        Segment3D segment1(point1,point2);
        RationalSegment3D segment2(point1.getR(),point2.getR());        
        assert_("RationalSegment3DTest 2.1", "segments aren't equal.", 
                 segment1.getR() == segment2);
        assert_("RationalSegment3DTest 2.2", "segments aren't equal.", 
                 segment1 == segment2.getD()); 
        //cout << segment1 << endl; 
        //cout << segment2 << endl;
      }// RationalSegment3DTest2
      
      void Point2DTest1(){
        Point2D point1;
        Point2D point2(0,0);
        Point2D point3(1,2);
        Point2D point4(point3);
        Point2D point5 = point3;
        assert_("Point2DTest 1.1", "points aren't equal.", 
                point1 == point2);
        assert_("Point2DTest 1.2", "points aren't equal.", 
                point3 == point4);
        assert_("Point2DTest 1.3", "points aren't equal.", 
                point5.getX() == 1 &&  point5.getY() == 2);
        // cout << point5 <<endl;
      }// Point3DTest1
      
      void RationalPoint2DTest1(){
        RationalPoint2D point1;
        RationalPoint2D point2(0,0);
        RationalPoint2D point3(1,2);
        RationalPoint2D point4(point3);
        RationalPoint2D point5 = point3;
        assert_("RationalPoint2DTest 1.1", "points aren't equal.", 
                point1 == point2);
        assert_("RationalPoint2DTest 1.2", "points aren't equal.", 
                point3 == point4);
        assert_("RationalPoint2DTest 1.3", "points aren't equal.", 
                point5.getX() == 1 &&  point5.getY() == 2);
        // cout << point5 <<endl;
      }// PationalPoint2DTest1
      
      void RationalPoint2DTest2(){
        Point2D point1(1,2);
        RationalPoint2D point2(1,2);
        assert_("RationalPoint2DTest 2.1", "points aren't equal.", 
                point1.getR() == point2);
        assert_("RationalPoint2DTest 2.2", "points aren't equal.", 
                point1 == point2.getD());
        // cout << point5 <<endl;
      }// Point2DTest2
      
      void RationalVector2DTest1(){
        RationalVector2D point1;
        RationalVector2D point2(0,0);
        RationalVector2D point3(1,2);
        RationalVector2D point4(point3);
        RationalVector2D point5 = point3;
        assert_("RationalVector2DTest 1.1", "vectors aren't equal.", 
                point1 == point2);
        assert_("RationalVector2DTest 1.2", "vectors aren't equal.", 
                point3 == point4);
        assert_("RationalVector2DTest 1.3", "vectors aren't equal.", 
                point5.getX() == 1 &&  point5.getY() == 2);
        // cout << point5 <<endl;
      }// RationalVector2DTest1
      
      void Segment2DTest1(){
        Point2D point1(1,2);
        Point2D point2(3,4);  
        Point2D point3(3,4);
        Segment2D segment1(point1,point2);
        Segment2D segment2(point1,point3);
        assert_("Segment2DTest 1.1", "points aren't equal.", 
                segment1.getTail() == point1);
        assert_("Segment2DTest 1.2", "points aren't equal.", 
                segment1.getHead() == point2); 
        // cout << segment1 << endl; ;
      }// Segment3DTest1

      void Segment2DTest2(){
        Point2D point1(1,1);
        Point2D point2(3,4.5);  
        Point2D point3(3,2);
        Point2D point4(1,3);
        Point2D point5(3,6);  
        Point2D point6(0,0);
        Point2D point7(1,0);  
        Point2D point8(1,1);
        Segment2D segment1(point1,point2);
        assert_("Segment2DTest 2.1", "point isn't left.", 
                !segment1.isLeft(point3));
        assert_("Segment2DTest 2.2", "point is left.", 
                segment1.isLeft(point4));
        assert_("Segment2DTest 2.3", "point is left.", 
                segment1.isLeft(point5));
        assert_("Segment2DTest 2.4", "point is left.", 
                segment1.isLeft(point6));
        assert_("Segment2DTest 2.5", "point isn't left.", 
                !segment1.isLeft(point7));
        assert_("Segment2DTest 2.6", "point isn't left.", 
                !segment1.isLeft(point8));
        // cout << segment1 << endl; ;
      }// Segment3DTest
      
      void RationalPoint3DExtTest(){
        RationalPoint3DExt point1(1,2,3,PFACE_A);
        RationalPoint3DExt point2(5,6,7,PFACE_B);
        assert_("RationalPoint3DExtTest 1", "point is from pface A.", 
                point1.getSourceFlag() == PFACE_A);
        assert_("RationalPoint3DExtTest 1", "point is from pface B.", 
                point2.getSourceFlag() == PFACE_B);
        assert_("RationalPoint3DExtTest 1", "point 1 isn't lower point 1.", 
                !(point1 < point1));
        assert_("RationalPoint3DExtTest 1", "point 1 is lower point 2.", 
                point1 < point2);
        assert_("RationalPoint3DExtTest 1", "point 2 isn't lower point 1.", 
                !(point2 < point1));
        // cout << point1 << endl;
        // cout << point2 << endl;        
      }// RationalPoint3DExtTest
      
      void RationalPoint3DExtSetTest1(){
        RationalPoint3DExt point1(1,1,0,PFACE_A);
        RationalPoint3DExt point2(2,2,0,PFACE_A);
        RationalPoint3DExt point3(1,1,0,PFACE_B);
        RationalPoint3DExt point4(2,2,0,PFACE_B);        
        RationalPoint3DExtSet points;      
        points.insert(point1);
        points.insert(point2);
        points.insert(point3);
        points.insert(point4);
        RationalSegment3D segment1;  
        RationalSegment3D segment2(Point3D(1,1,0),Point3D(2,2,0));
        bool result = points.getIntersectionSegment(segment1);
        assert_("RationalPoint3DExt 1.1", "it exist a intersection.", 
                result);
        assert_("RationalPoint3DExt 1.1", "intersection segment is incorrect.",
                segment1 == segment2); 
        // cout << segment1 << endl;
        // cout << points << endl; 
      }// PointExtSetTest1
      
      void RationalPoint3DExtSetTest2(){
        RationalPoint3DExt point1(1,1,0,PFACE_A);
        RationalPoint3DExt point2(3,3,0,PFACE_A);
        RationalPoint3DExt point3(2,2,0,PFACE_B);
        RationalPoint3DExt point4(4,4,0,PFACE_B);        
        RationalPoint3DExtSet points;      
        points.insert(point1);
        points.insert(point2);
        points.insert(point3);
        points.insert(point4);
        RationalSegment3D segment1;  
        RationalSegment3D segment2(Point3D(2,2,0),Point3D(3,3,0));
        bool result = points.getIntersectionSegment(segment1);
        assert_("RationalPoint3DExtSetTest 2.1", "it exist a intersection.",
                result);
        assert_("RationalPoint3DExtSetTest 2.1", 
                "intersection segment is incorrect.", segment1 == segment2);
        // cout << segment1 << endl;
        // cout << points << endl; 
      }// PointExtSetTest2
      
       void RationalPoint3DExtSetTest3(){
        RationalPoint3DExt point1(1,1,0,PFACE_A);
        RationalPoint3DExt point2(3,3,0,PFACE_B);
        RationalPoint3DExt point3(2,2,0,PFACE_A);
        RationalPoint3DExt point4(4,4,0,PFACE_B);        
        RationalPoint3DExtSet points;      
        points.insert(point1);
        points.insert(point2);
        points.insert(point3);
        points.insert(point4);
        RationalSegment3D segment1;  
        bool result = points.getIntersectionSegment(segment1);
        assert_("RationalPoint3DExtSetTest 3", "it exist no intersection",
                !result);
        // cout << segment1 << endl;
        // cout << points << endl; 
      }// PointExtSetTest3

      void RationalPoint3DExtSetTest4(){
        RationalPoint3DExt point1(1,1,0,PFACE_A);
        RationalPoint3DExt point2(2,2,0,PFACE_A);
        RationalPoint3DExt point3(4,4,0,PFACE_A);        
        RationalPoint3DExtSet points;      
        points.insert(point1);
        points.insert(point2);
        points.insert(point3);
        RationalSegment3D segment1;  
        bool result = points.getIntersectionSegment(segment1);
        assert_("RationalPoint3DExtSetTest 4", "it exist no intersection",
                !result);   
        // cout << segment1 << endl;
        // cout << points << endl; 
      }// PointExtSetTest4
      
      void RationalPlane3DTest1(){
        // points from pfaces 
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // points from segment1 
        Point3D point5(2,2,0);
        Point3D point6(1,3,3);
        // points from segment 2
        Point3D point7(2,2,3);
        Point3D point8(1,3,0);
        // results
        RationalPoint3D point9,point10;
        // planes
        RationalPlane3D plane1(PFace(point1,point2,point3,point4));
        RationalPlane3D plane2(PFace(point3,point4,point1,point2));
        // intersection point
        RationalPoint3D iPoint(1.5,2.5,1.5);  
        // distance check
        assert_("RationalPlane3DTest 1.1", "distance to plane is zero.", 
                plane1.distance2ToPlane(point4) == 0.0); 
        assert_("RationalPlane3DTest 1.2", "distance to plane is zero.", 
                plane2.distance2ToPlane(point1) == 0.0);  
        assert_("RationalPlane3DTest 1.3", "distance to plane is incorrect.", 
                plane1.distance2ToPlane(point5) == 0.5); 
        assert_("RationalPlane3DTest 1.4", "distance to plane is incorrect.", 
                plane2.distance2ToPlane(point8) == 0.5);  
        // intersection
        bool result = plane1.intersection(Segment3D(point5,point6),
                                          point9);
        assert_("RationalPlane3DTest 1.5", "segment intersect plane.",result);
        assert_("RationalPlane3DTest 1.6", "intersection point is incorrect.",
                point9 == iPoint);
        result = plane1.intersection(Segment3D(point7,point8),
                                     point10);
        assert_("RationalPlane3DTest 1.7", "segment intersect plane.",result);
        assert_("RationalPlane3DTest 1.8", "intersection point is incorrect.",
                point10 == iPoint);
      }// RationalPlane3DTest1
      
      void RationalPlane3DTest2(){
         // points from pface 1 
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // points from pface 2
        Point3D point5(2,2,0);
        Point3D point6(4,4,0);         
        Point3D point7(2,2,3);
        Point3D point8(4,4,3);
        // points form pface 3
        Point3D point9(0,1,0);
        Point3D point10(5,6,0);         
        Point3D point11(0,1,3);
        Point3D point12(5,6,3);
        // points form pface 4
        Point3D point13(2,2,0);
        Point3D point14(4,4,0);         
        Point3D point15(1,2,3);
        Point3D point16(3,4,3);
        // pfaces
        RationalPlane3D plane1(PFace(point1,point2,point3,point4));
        RationalPlane3D plane2(PFace(point5,point6,point7,point8));
        RationalPlane3D plane3(PFace(point9,point10,point11,point12));
        RationalPlane3D plane4(PFace(point13,point14,point15,point16));
        bool result1, result2;
        result1 = plane1.isParallelTo(plane2);
        result2 = plane1.isCoplanarTo(plane2); 
        assert_("RationalPlane3DTest 2.1", "pface is parallel.",result1);
        assert_("RationalPlane3DTest 2.2", "pface isn't coplanar.",!result2);
        result1 = plane1.isParallelTo(plane3);   
        result2 = plane1.isCoplanarTo(plane3);
        assert_("RationalPlane3DTest 2.3", "pface is parallel.",result1);
        assert_("RationalPlane3DTest 2.4", "pface is coplanar.",result2);
        result1 = plane1.isParallelTo(plane4); 
        assert_("RationalPlane3DTest 2.5", "pface isn't parallel.",!result1);
      }// RationalPlane3DTest2  
      
      void RationalPlane3DTest3(){
         // points from pface 1 
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // pfaces
        RationalPlane3D plane1(PFace(point1,point2,point3,point4));
        // transformation
        Point2D point5 = plane1.transform(point1);
        Point2D point6 = plane1.transform(point2);
        Point2D point7 = plane1.transform(point3);
        Point2D point8 = plane1.transform(point4);
        // result
        Point2D point9(-2.12132034,0);
        Point2D point10(-4.94974747,0);
        Point2D point11(-2.12132034,3);
        Point2D point12(-4.94974747,3);
        assert_("RationalPlane3DTest 3.1", "transformation is incorrect.",
                point5 == point9);
        assert_("RationalPlane3DTest 3.2", "transformation is incorrect.",
                point6 == point10);
        assert_("RationalPlane3DTest 3.3", "transformation is incorrect.",
                point7 == point11);
        assert_("RationalPlane3DTest 3.4", "transformation is incorrect.",
                point8 == point12);        
        // cout << setprecision(9);
        // cout << point1 << ", " << point5 << endl;
        // cout << point2 << ", " << point6 << endl;
        // cout << point3 << ", " << point7 << endl;
        // cout << point4 << ", " << point8 << endl;
      }// RationalPlane3DTest3 
      
      void IntersectionPointTest(){
        Point3D point1(1,2,3);
        Point2D point2(4,3);       
        IntersectionPoint point3(point1,point2);
        IntersectionPoint point4 = point3;
        IntersectionPoint point5(1,2,3,4);
        assert_("IntersectionPointTest 1.1", "3D component is incorrect.",
                point4.getPoint3D() == point1);
        assert_("IntersectionPointTest 1.2", "2D component is incorrect.",
                point4.getPoint2D() == point2);
        assert_("IntersectionPointTest 1.3", "3D component is incorrect.",
                point4.getX() == 1 &&  point4.getY() == 2 &&
                point4.getZ() == 3);
        assert_("IntersectionPointTest 1.4", "2D component is incorrect.",
                point4.getW() == 4 && point4.getT() == 3 );
        assert_("IntersectionPointTest 1.5", "points are equal.",
                point4 == point3);
        assert_("IntersectionPointTest 1.6", "points are equal.",
                point4 == point5);
        // cout << point4 <<endl;
      }// IntersectionPointTest
      
      void IntersectionSegmentTest(){
        Point3D point1(1,2,0);
        Point3D point2(4,5,3);
        Point2D point3(6,0);
        Point2D point4(7,3);
        IntersectionPoint point5(1,2,0,6);
        IntersectionPoint point6(4,5,3,7);
        Segment3D segment1(point1,point2);
        Segment2D segment2(point3,point4);
        IntersectionSegment segment3(segment1,segment2,true);
        IntersectionSegment segment4 = segment3;
        IntersectionSegment segment5(point5,point6,true);
        assert_("IntersectionSegmentTest 1.1", "3D component is incorrect.",
                segment4.getSegment3D() == segment1);
        assert_("IntersectionSegmentTest 1.2", "2D component is incorrect.",
                segment4.getSegment2D() == segment2);
        assert_("IntersectionSegmentTest 1.3", "point on head is incorrect.",
                segment4.getTail() == IntersectionPoint(point1, point3));
        assert_("IntersectionSegmentTest 1.4", "point on tail is incorrect.",
                segment4.getHead() == IntersectionPoint(point2, point4));
        assert_("IntersectionSegmentTest 1.5", "Left area is inner.",
                segment4.isleftAreaInner());
        assert_("IntersectionSegmentTest 1.6", "Segments are equal.",
                segment3 == segment4);
        assert_("IntersectionSegmentTest 1.7", "Segments are equal.",
                segment3 == segment5);
        assert_("IntersectionSegmentTest 1.8", "Segments arn't equal.",
                !(segment3 == IntersectionSegment()));
        // cout << segment1 << endl; 
        // cout << segment2 << endl; 
        // cout << segment3 << endl; 
        // cout << segment3.getSegment3D() << endl; 
        // cout << segment3.getSegment2D() << endl; 
      }// IntersectionSegmentTest
      
      void IntSegContainerTest1(){
        IntSegContainer container1,container2;
        container1.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,0,0),IntersectionPoint(0,0,1,1),true));
        container1.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,2,2),IntersectionPoint(0,0,3,3),false)); 
        container2.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,2,2),IntersectionPoint(0,0,3,3),false));
        assert_("IntSegContainerTest 1.1", 
                "intersection segments are incorect.",
                !(container1 == container2));
        container2.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,0,0),IntersectionPoint(0,0,1,1),true));
        assert_("IntSegContainerTest 1.2", 
                "intersection segments are incorect.",
                container1 == container2);
        // cout << container1;
        // cout << container2;
      }// IntSegContainerTest1
      
      void IntSegContainerTest2(){
        IntSegContainer container1,container2;
        container1.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,0,0),IntersectionPoint(0,0,1,1),true));
        container1.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,2,2),IntersectionPoint(0,0,3,3),false));
        container1.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,1.5,2),IntersectionPoint(0,0,1.5,3),true));
        container1.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,1.5,2),IntersectionPoint(0,0,3,3),false));
        container2.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,1.5,2),IntersectionPoint(0,0,3,3),false));
        container2.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,1.5,2),IntersectionPoint(0,0,1.5,3),true));
        container2.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,2,2),IntersectionPoint(0,0,3,3),false));
        container2.addIntSeg(new IntersectionSegment(
          IntersectionPoint(0,0,0,0),IntersectionPoint(0,0,1,1),true));
        assert_("IntSegContainerTest 2.1", 
                "intersection segments are incorect.",
                container1 == container2);
        // cout << container1;
        // cout << container2;
      }// IntSegContainerTest2
      
      void PFaceTesst1(){
        // points from pface 1 
        Point3D point1(2,1,0);
        Point3D point2(4,3,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // points from pface 2
        Point3D point5(2.5,1,0);
        Point3D point6(5,1,0);         
        Point3D point7(2.5,4.5,3);
        Point3D point8(5,4.5,3);
        // Result points
        IntersectionPoint point9(2.5, 2.16666667, 1.0, -3.29983165);
        IntersectionPoint point10(3.2, 3.8, 2.4, -4.94974747);
        IntersectionPoint point11(2.5, 2.16666667, 1, -2.5);
        IntersectionPoint point12(3.2, 3.8, 2.4, -3.2);
        IntSegContainer intSegContainer1, intSegContainer2;
        intSegContainer1.addIntSeg(
          new IntersectionSegment(point9,point10,false));
        intSegContainer2.addIntSeg(
          new IntersectionSegment(point11,point12,true));
        // pfaces
        PFace pf1(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);
        assert_("PFaceTesst 1.1", "pfaces intersect.",result);
        assert_("PFaceTesst 1.2", "intersection segment is incorrect.",
                pf1.intSegContainer == intSegContainer1);
        assert_("PFaceTesst 1.3", "intersection segment is incorrect.",
                pf2.intSegContainer == intSegContainer2);
        // cout << setprecision(9);
        // cout << pf1 << endl;
        // cout << pf2 << endl;
      }// PFaceTesst1
      
      void PFaceTesst2(){
        // points from pface 1 
        Point3D point1(11,12,0);
        Point3D point2(13,14,0);         
        Point3D point3(11,12,3);
        Point3D point4(13,14,3);
        // points from pface 2
        Point3D point5(2,1,0);
        Point3D point6(4,3,0);         
        Point3D point7(2,1,3);
        Point3D point8(4,3,3);
        // pfaces
        PFace pf1(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);
        assert_("PFaceTesst 2.1", "pfaces not intersect.",!result);
      }// PFaceTesst2
      
      void PFaceTesst3(){
        // points from pface 1 
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // points from pface 2
        Point3D point5(2,1,0);
        Point3D point6(4,3,0);         
        Point3D point7(2,1,3);
        Point3D point8(4,3,3);
        // pfaces
        PFace pf1(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);
        assert_("PFaceTesst 3.1", "pfaces not intersect.",!result);
      }// PFaceTesst3
      
      void PFaceTesst4(){
        // points from pface 1 
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // points from pface 2
        Point3D point5(2,3,0);
        Point3D point6(4,5,0);         
        Point3D point7(2,3,3);
        Point3D point8(4,5,3);
        // pfaces
        PFace pf1(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);
        assert_("PFaceTesst 4.1", "pfaces not intersect.",!result);
      }// PFaceTesst4
      
      void PFaceTesst5(){
        // points from pface 1 
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // points from pface 2
        Point3D point5(3,2,0);
        Point3D point6(1,4,0);         
        Point3D point7(3,2,3);
        Point3D point8(1,4,3);
        // result     
        IntersectionPoint point9(2, 3, 0, -3.53553391);
        IntersectionPoint point10(2, 3, 3, -3.53553391);
        IntersectionPoint point11(2, 3, 0, -0.707106781);
        IntersectionPoint point12(2, 3, 3, -0.707106781);
        IntSegContainer intSegContainer1, intSegContainer2;
        intSegContainer1.addIntSeg(
          new IntersectionSegment(point9,point10,true));
        intSegContainer2.addIntSeg(
          new IntersectionSegment(point11,point12,false));
        // pfaces
        PFace pf1(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);
        assert_("PFaceTesst 5.1", "pfaces intersect.",result);
        assert_("PFaceTesst 5.2", "intersection segment is incorrect.",
                pf1.intSegContainer == intSegContainer1);
        assert_("PFaceTesst 5.3", "intersection segment is incorrect.",
                pf2.intSegContainer == intSegContainer2);
        // cout << setprecision(9);
        // cout << pf1 << endl;
        // cout << pf2 << endl;
      }// PFaceTesst5    
      
      void PFaceTesst6(){
        // points from pface 1 
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // points from pface 2
        Point3D point5(1,0,0);
        Point3D point6(-1,2,0);         
        Point3D point7(1,0,3);
        Point3D point8(-1,2,3);
        // pfaces
        PFace pf1(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);
        assert_("PFaceTesst 6.1", "pfaces not intersect.",!result);
      }// PFaceTesst4
       
      

      
      
      int numberOfTestsRun;
      int numberOfTestsFailed; 
           
    }; // class Selftest
    
    // TypeMapping für den Operator 'selftest'
    ListExpr selftestTM(ListExpr args){
      string err = " no paramters expected";
      if(!nl->HasLength(args,0)){ 
        return listutils::typeError(err);
      }// if
      return listutils::basicSymbol<CcBool>();
    }// TypeMapping 
    
    // ValueMapping für den Operator 'selftest'
    int selftestVM(Word* args, Word& result, int message, Word& local, 
                  Supplier s){ 
      Selftest test;
      bool res;
      // Selbstest ausführen
      try {
        res = test.run();
      }// try
      catch (NumericFailure e){
        cerr << endl << "!!! "<< e.what()<< endl << endl;
      }// catch
      result = qp->ResultStorage(s);
      CcBool* b = (CcBool*) result.addr;    
      b->Set(true,res);
      return 0;
    }// ValueMapping
    
    // Angaben zum Operator 'selftest'
    OperatorSpec selftestSpec(
      "selftest->bool", 
      "selftest ()", 
      "Selftest for Spatial3d2 ", 
      "query selftest()"
    );

    // Operator zusammensetzen
    Operator* getSelftestPtr(){
      return new Operator(
        "selftest",
        selftestSpec.getStr(),
        selftestVM,
        Operator::SimpleSelect,
        selftestTM
      );
    }// getSelftestPtr  
    
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
