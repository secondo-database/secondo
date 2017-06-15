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
        
        ContainerPoint3DTest();
        
        RationalPoint3DExtTest();
        RationalPoint3DExtSetTest1();
        RationalPoint3DExtSetTest2();
        RationalPoint3DExtSetTest3();
        RationalPoint3DExtSetTest4();
        
        RationalPlane3DTest1();
        RationalPlane3DTest2();
        RationalPlane3DTest3();
        
        IntersectionPointTest();
        
        IntersectionSegmentTest1();
        IntersectionSegmentTest2();
        IntersectionSegmentTest3();
        
        IntSegContainerTest1();
        IntSegContainerTest2();
        
        PFaceTest1();
        PFaceTest2();
        PFaceTest3();
        PFaceTest4();
        PFaceTest5();
        PFaceTest6();
        PFaceTest7();
        
        GlobalTimeValuesTest();
     
        SourceUnitTest1();
        SourceUnitTest2();
        SourceUnitTest3();
        
        IntSegContainerTest3();
        IntSegContainerTest4();
        
        SegmentTest();
    
        ResultPfaceFactoryTest1();
        ResultPfaceFactoryTest2();
        ResultPfaceFactoryTest3();
        
        SourceUnitTest4();
        
        
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
/*
1 Test Point3D

*/        
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
/*
2 Test RationalPoint3D

*/        
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
/*
3 Test Test RationalVector3D

*/       
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
/*
4 Test Segment3D

*/        
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
/*
5 Test RationalSegment3D

*/         
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
/*
6 Test Point2D

*/       
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
/*
7 Test RationalPoint2D

*/        
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
/*
8 Test RationalVector2D

*/        
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
/*
9 Test Segment2D

*/        
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
/*
10 Test ContainerPoint3D

*/            
      void ContainerPoint3DTest(){
        ContainerPoint3D container;
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);  
        Point3D point3(3,4,1);
        size_t i1 = container.add(point1);
        size_t i2 = container.add(point2);
        size_t i3 = container.add(point3);
        size_t i4 = container.add(point2);
        assert_("ContainerPoint3DTest", " points index is incoorect.", 
                i1 == 0 && i2 ==1 && i3 == 2 && i4 == 1);
        // cout << container;
        // cout << i1 <<i2 <<i3 << i4<<endl;        
      }// ContainerPoint3DTest
/*
11 Test RationalPoint3DExt

*/        
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
/*
12 Test RationalPoint3DExtSet

*/         
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
/*
13 Test RationalPlane3D

*/        
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
/*
14 Test IntersectionPoint

*/        
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
/*
15 Test IntersectionSegment

*/        
      void IntersectionSegmentTest1(){
        Point3D point1(1,2,0);
        Point3D point2(4,5,3);
        Point2D point3(6,0);
        Point2D point4(7,3);
        IntersectionPoint point5(1,2,0,6);
        IntersectionPoint point6(4,5,3,7);
        Segment3D segment1(point1,point2);
        Segment2D segment2(point3,point4);
        IntersectionSegment segment3(segment1,segment2,LEFT_IS_INNER);
        IntersectionSegment segment4 = segment3;
        IntersectionSegment segment5(point5,point6,LEFT_IS_INNER);
        assert_("IntersectionSegmentTest 1.1", "3D component is incorrect.",
                segment4.getSegment3D() == segment1);
        assert_("IntersectionSegmentTest 1.2", "2D component is incorrect.",
                segment4.getSegment2D() == segment2);
        assert_("IntersectionSegmentTest 1.3", "point on head is incorrect.",
                segment4.getTail() == IntersectionPoint(point1, point3));
        assert_("IntersectionSegmentTest 1.4", "point on tail is incorrect.",
                segment4.getHead() == IntersectionPoint(point2, point4));
        assert_("IntersectionSegmentTest 1.5", "Left area is inner.",
                segment4.getPredicate()== LEFT_IS_INNER);
        assert_("IntersectionSegmentTest 1.6", "Segments are equal.",
                segment3 == segment4);
        assert_("IntersectionSegmentTest 1.7", "Segments are equal.",
                segment3 == segment5);
        assert_("IntersectionSegmentTest 1.8", "Segments arn't equal.",
                !(segment3 == IntersectionSegment()));
        assert_("IntersectionSegmentTest 1.9", "Segment isn't out of range.",
                !(segment5.isOutOfRange(2)));
        assert_("IntersectionSegmentTest 1.10", "Segment is out of range.",
                segment5.isOutOfRange(3));
        assert_("IntersectionSegmentTest 1.11", "Segment is out of range.",
                segment5.isOutOfRange(4));        
        // cout << segment1 << endl; 
        // cout << segment2 << endl; 
        // cout << segment3 << endl; 
        // cout << segment3.getSegment3D() << endl; 
        // cout << segment3.getSegment2D() << endl; 
      }// IntersectionSegmentTest
      
      void IntersectionSegmentTest2(){
        IntersectionPoint point1(0,0,0,6);
        IntersectionPoint point2(0,0,3,7);
        IntersectionPoint point3(0,0,0,5);
        IntersectionPoint point4(0,0,3,6);
        IntersectionPoint point5(0,0,0,7);
        IntersectionPoint point6(0,0,3,7);
        IntersectionPoint point7(0,0,-1,8);
        IntersectionPoint point8(0,0,3,7);   
        IntersectionSegment segment1(point1,point2,UNDEFINED);
        IntersectionSegment segment2(point3,point4,UNDEFINED);
        IntersectionSegment segment3(point5,point6,UNDEFINED);
        IntersectionSegment segment4(point7,point8,UNDEFINED);
        assert_("IntersectionSegmentTest 2.1", "Segment isn't left.",
                !(segment1.isLeftOf(segment2)));
        assert_("IntersectionSegmentTest 2.2", "Segment is left.",
                segment1.isLeftOf(segment3));
        assert_("IntersectionSegmentTest 2.3", "Segment is left.",
                segment1.isLeftOf(segment4)); 
      }// IntersectionSegmentTest
      
      void IntersectionSegmentTest3(){
        IntersectionPoint point1(0,0,0,0);
        IntersectionPoint point2(10,20,5,0);  
        IntersectionSegment segment1(point1,point2,UNDEFINED);
        Point3D point3 = segment1.evaluate(2);  
        Point3D point4 = segment1.evaluate(0); 
        Point3D point5 = segment1.evaluate(5); 
        assert_("IntersectionSegmentTest 3.1", "Points are equal.",
                point3 == Point3D(4,8,2));
        assert_("IntersectionSegmentTest 3.2", "Points are equal.",
                point4 == Point3D(0,0,0));
        assert_("IntersectionSegmentTest 3.3", "Points are equal.",
                point5 == Point3D(10,20,5)); 
      }// IntersectionSegmentTest3   
/*
16 Test IntSegContainer

*/       
      void IntSegContainerTest1(){
        IntSegContainer container1,container2;
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,0,0),
          IntersectionPoint(0,0,1,1),LEFT_IS_INNER));
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,2,2),
          IntersectionPoint(0,0,3,3),RIGHT_IS_INNER)); 
        container2.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,2,2),
          IntersectionPoint(0,0,3,3),RIGHT_IS_INNER));
        assert_("IntSegContainerTest 1.1", 
                "intersection segments are incorect.",
                !(container1 == container2));
        container2.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,0,0),
          IntersectionPoint(0,0,1,1),LEFT_IS_INNER));
        assert_("IntSegContainerTest 1.2", 
                "intersection segments are incorect.",
                container1 == container2);
        // cout << container1;
        // cout << container2;
      }// IntSegContainerTest1
      
      void IntSegContainerTest2(){
        IntSegContainer container1,container2,container3;
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,0,0),
          IntersectionPoint(0,0,1,1),LEFT_IS_INNER));
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,2,2),
          IntersectionPoint(0,0,3,3),RIGHT_IS_INNER));
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,1.5,2),
          IntersectionPoint(0,0,1.5,3),LEFT_IS_INNER));
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,1.5,2),
          IntersectionPoint(0,0,3,3),RIGHT_IS_INNER));
        container2.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,1.5,2),
          IntersectionPoint(0,0,3,3),RIGHT_IS_INNER));
        container2.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,1.5,2),
          IntersectionPoint(0,0,1.5,3),LEFT_IS_INNER));
        container2.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,2,2),
          IntersectionPoint(0,0,3,3),RIGHT_IS_INNER));
        container2.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,0,0),
          IntersectionPoint(0,0,1,1),LEFT_IS_INNER));
        assert_("IntSegContainerTest 2.1", 
                "intersection segments are incorect.",
                container1 == container2);
        assert_("IntSegContainerTest 2.2", 
                "intersection container are incorect.",
                !(container1 == container3));
        container3 = container2;
        assert_("IntSegContainerTest 2.3", 
                "intersection segments are incorect.",
                container1 == container3);
        // cout << container1;
        // cout << container2;
      }// IntSegContainerTest2   
/*
17 Test PFace

*/        
      void PFaceTest1(){
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
        // pfaces
        PFace pf1(point1,point2,point3,point4);
        PFace pf3(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        PFace pf4(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);    
        pf3.addIntSeg(IntersectionSegment(point9,point10,LEFT_IS_INNER));
        pf3.setState(RELEVANT);
        pf4.addIntSeg(IntersectionSegment(point11,point12,RIGHT_IS_INNER));
        pf4.setState(RELEVANT);
        assert_("PFaceTest 1.1", "pfaces intersect.",result);
        assert_("PFaceTest 1.2", "intersection segment is incorrect.",
                pf1 == pf3);
        assert_("PFaceTest 1.3", "intersection segment is incorrect.",
                pf2 == pf4);
        // cout << setprecision(9);
        // cout << pf1 << endl;
        // cout << pf3 << endl;
        // cout << pf2 << endl; 
        // cout << pf4 << endl;
      }// PFaceTest1
      
      void PFaceTest2(){
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
        assert_("PFaceTest 2.1", "pfaces not intersect.",!result);
      }// PFaceTest2
      
      void PFaceTest3(){
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
        assert_("PFaceTest 3.1", "pfaces not intersect.",!result);
      }// PFaceTest3
      
      void PFaceTest4(){
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
        assert_("PFaceTest 4.1", "pfaces not intersect.",!result);
      }// PFaceTest4
      
      void PFaceTest5(){
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
        
        PFace pf1(point1,point2,point3,point4);
        PFace pf3(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        PFace pf4(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);    
        pf3.addIntSeg(IntersectionSegment(point9,point10,RIGHT_IS_INNER));
        pf3.setState(RELEVANT);
        pf4.addIntSeg(IntersectionSegment(point11,point12,LEFT_IS_INNER));
        pf4.setState(RELEVANT);
        assert_("PFaceTest 5.1", "pfaces intersect.",result);
        assert_("PFaceTest 5.2", "intersection segment is incorrect.",
                pf1 == pf3);
        assert_("PFaceTest 5.3", "intersection segment is incorrect.",
                pf2 == pf4);
        // cout << setprecision(9);
        // cout << pf1 << endl;
        // cout << pf2 << endl;
        // cout << pf3 << endl;
        // cout << pf4 << endl;
      }// PFaceTest5    
      
      void PFaceTest6(){
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
        assert_("PFaceTest 6.1", "pfaces not intersect.",!result);
      }// PFaceTest4
      
      void PFaceTest7(){
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
        // pfaces
        PFace pf1(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        bool result = pf1.intersection(pf2);
        PFace pf3(point1,point2,point3,point4);
        PFace pf4(point5,point6,point7,point8); 
        pf3.addIntSeg(IntersectionSegment(point9,point10,LEFT_IS_INNER));
        pf3.setState(RELEVANT);
        pf4.addIntSeg(IntersectionSegment(point11,point12,RIGHT_IS_INNER));
        pf4.setState(RELEVANT);       
        assert_("PFaceTest 7.1", "pfaces intersect.",result);
        assert_("PFaceTest 7.2", "intersection segment is incorrect.",
                pf1 == pf3);
        assert_("PFaceTest 7.3", "intersection segment is incorrect.",
                pf2 == pf4);
        // cout << setprecision(9);
        // cout << pf1 << endl;
        // cout << pf2 << endl;
        // cout << pf3 << endl;
        // cout << pf4 << endl;        
      }// PFaceTest7     
/*
18 Test GlobalTimeValues

*/        
      void GlobalTimeValuesTest(){
        GlobalTimeValues timeValues1,timeValues2,timeValues3;
        timeValues1.addTimeValue(1);
        timeValues1.addTimeValue(0);
        timeValues1.addTimeValue(1);
        timeValues1.addTimeValue(0.5);
        timeValues1.addTimeValue(0.7);
        timeValues1.addTimeValue(0.3);
        timeValues1.addTimeValue(0.5);
        assert_("GlobalTimeValuesTest 1", "time values are not equal.",
                !(timeValues1 == timeValues2));
        timeValues2.addTimeValue(0);
        timeValues2.addTimeValue(0.3);
        timeValues2.addTimeValue(0.5);
        timeValues2.addTimeValue(0.7);
        timeValues2.addTimeValue(1);
        timeValues3.addTimeValue(0);
        timeValues3.addTimeValue(0.3);
        timeValues3.addTimeValue(0.5);
        timeValues3.addTimeValue(0.8);
        timeValues3.addTimeValue(1);
        assert_("GlobalTimeValuesTest 2", "time values are equal.",
                timeValues1 == timeValues2);
        assert_("GlobalTimeValuesTest 3", "time values are not equal.",
                !(timeValues1 == timeValues3));
        // cout << timeValues1 << endl;
        // cout << timeValues2 << endl;
        double value1,value2;
        bool result = true;
        if (timeValues1.first(value1) && timeValues2.first(value2)){
          if(value1 != value2) result = false;
          // cout << "time:=" << value1<< endl;
          while(timeValues1.next(value1) && timeValues2.next(value2)){
            if(value1 != value2) result = false;
            // cout << "time:=" << value1 << endl;            
          }// while
        }// if
        assert_("GlobalTimeValuesTest 4", "time values are equal.",
                result);
      }// GlobalTimeValuesTest 
/*
19 Test SourceUnit

*/       
      void SourceUnitTest1(){
        SourceUnit sourceUnit1,sourceUnit2,sourceUnit3,sourceUnit4;
        GlobalTimeValues timeValues1,timeValues2;
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
        sourceUnit1.addPFace(point1,point2,point3,point4);
        sourceUnit2.addPFace(point5,point6,point7,point8);       
        sourceUnit1.intersection(sourceUnit2,timeValues1);
        // result
        PFace pf1(point1,point2,point3,point4);
        PFace pf2(point5,point6,point7,point8);
        // Intersection points
        IntersectionPoint point9(3, 4, 0, -4.94974747);
        IntersectionPoint point10(3, 4, 3, -4.94974747);
        IntersectionPoint point11(2, 3, 0, -3.53553391);
        IntersectionPoint point12(2, 3, 3, -3.53553391);
        IntersectionPoint point13(1, 2, 0, -2.12132034);
        IntersectionPoint point14(1, 2, 3, -2.12132034);
        IntersectionPoint point15(1, 4, 0, -2.12132034);
        IntersectionPoint point16(1, 4, 3, -2.12132034);
        IntersectionPoint point17(2, 3, 0, -0.707106781);
        IntersectionPoint point18(2, 3, 3, -0.707106781);
        IntersectionPoint point19(3, 2, 0, 0.707106781);
        IntersectionPoint point20(3, 2, 3, 0.707106781);
        // Intersection segments
        pf1.addIntSeg(
          IntersectionSegment(point9,point10,UNDEFINED));
        pf1.addIntSeg(
          IntersectionSegment(point11,point12,RIGHT_IS_INNER));
        pf1.addIntSeg(  
          IntersectionSegment(point13,point14,UNDEFINED));
        pf2.addIntSeg(
          IntersectionSegment(point15,point16,UNDEFINED));
        pf2.addIntSeg( 
          IntersectionSegment(point17,point18,LEFT_IS_INNER));
        pf2.addIntSeg(   
          IntersectionSegment(point19,point20,UNDEFINED));
        // state 
        pf1.setState(RELEVANT);
        pf2.setState(RELEVANT);
        sourceUnit3.addPFace(pf1);
        sourceUnit4.addPFace(pf2); 
        // global time values
        timeValues2.addTimeValue(0);
        timeValues2.addTimeValue(3);
        assert_("GlobalTimeValuesTest 2", "time values are equal.",
                timeValues1 == timeValues2);
        assert_("SourceUnitTest1 1.1", "source units are equal.",
                sourceUnit1 == sourceUnit3);
        assert_("SourceUnitTest1 1.2", "source units are equal.",
                sourceUnit2 == sourceUnit4);
        assert_("SourceUnitTest1 1.3", "time values are equal.",
                timeValues1 == timeValues2);
        // cout << setprecision(9);
        // cout << sourceUnit1 << endl;
        // cout << sourceUnit2 << endl;
        // cout << sourceUnit3 << endl;
        // cout << sourceUnit4 << endl;
        // cout << timeValues1 << endl;
      }// SourceUnitTest1
      
      void SourceUnitTest2(){
        SourceUnit sourceUnit1,sourceUnit2, sourceUnit3, sourceUnit4;
        GlobalTimeValues timeValues1,timeValues2;
        // points from unit 1 
        Point3D point0(2,1,0);
        Point3D point1(5,1,0);         
        Point3D point2(3.5,4,0);
        Point3D point3(2,1,5);
        Point3D point4(5,1,5);         
        Point3D point5(3.5,4,5);
        // points from pface 2
        Point3D point6(6,1,0);
        Point3D point7(8,1,0);         
        Point3D point8(7,3.5,0);
        Point3D point9(0,4,5);
        Point3D point10(2,4,5);         
        Point3D point11(1,6.5,5);
        // add pfaces to unit 1 
        sourceUnit1.addPFace(point4,point3,point1,point0);
        sourceUnit1.addPFace(point5,point4,point2,point1);
        sourceUnit1.addPFace(point3,point5,point0,point2);
        // add pfaces to unit 2
        sourceUnit2.addPFace(point10,point9,point7,point6);
        sourceUnit2.addPFace(point11,point10,point8,point7);
        sourceUnit2.addPFace(point9,point11,point6,point8);
        // intersection
        sourceUnit1.intersection(sourceUnit2,timeValues1);  
        // pfaces from result unit 3
        PFace pf1(point4,point3,point1,point0);
        pf1.setState(UNKNOWN);
        PFace pf2(point5,point4,point2,point1);
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, -2.01246118), 
          IntersectionPoint(3.5, 4, 5, -2.01246118), 
          UNDEFINED));
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(5, 1, 0, 1.34164079), 
          IntersectionPoint(5, 1, 5, 1.34164079), 
          UNDEFINED));
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, 0.596284794),
          IntersectionPoint(3.5, 4, 2.56944444, -2.01246118),
          RIGHT_IS_INNER));
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, 0.596284794),
          IntersectionPoint(4, 3, 3.33333333, -0.894427191),             
          LEFT_IS_INNER));
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(4, 3, 3.33333333, -0.894427191),
          IntersectionPoint(3.5, 4, 3.4375, -2.01246118), 
          LEFT_IS_INNER));
        pf2.setState(RELEVANT);
        PFace pf3(point3,point5,point0,point2);
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(2, 1, 0, 1.78885438), 
          IntersectionPoint(2, 1, 5, 1.78885438), 
          UNDEFINED));
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, 5.14295635), 
          IntersectionPoint(3.5, 4, 5, 5.14295635), 
          UNDEFINED));
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 2.56944444, 5.14295635),
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          RIGHT_IS_INNER)); 
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.2, 3.4, 4, 4.47213595), 
          RIGHT_IS_INNER));
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 3.4375, 5.14295635), 
          IntersectionPoint(3.2, 3.4, 4, 4.47213595), 
          LEFT_IS_INNER));
        pf3.setState(RELEVANT);
        sourceUnit3.addPFace(pf1);
        sourceUnit3.addPFace(pf2);
        sourceUnit3.addPFace(pf3);
        // pfaces from result unit 4
        PFace pf4(point10,point9,point7,point6);
        pf4.addIntSeg(IntersectionSegment(
          IntersectionPoint(8, 1, 0, -8),
          IntersectionPoint(2, 4, 5, -2), 
          UNDEFINED));
        pf4.addIntSeg(IntersectionSegment(
          IntersectionPoint(6, 1, 0, -6),
          IntersectionPoint(0, 4, 5, 0),  
          UNDEFINED));
        pf4.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -4.66666667),
          IntersectionPoint(4, 3, 3.33333333, -4), 
          RIGHT_IS_INNER));
        pf4.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, -2.8), 
          IntersectionPoint(3.2, 3.4, 4, -3.2),
          LEFT_IS_INNER));
        pf4.setState(RELEVANT);
        PFace pf5(point11,point10,point8,point7);
        pf5.addIntSeg(IntersectionSegment (
          IntersectionPoint(7, 3.5, 0, -0.649933684), 
          IntersectionPoint(1, 6.5, 5, -5.66370781), 
          UNDEFINED));
        pf5.addIntSeg(IntersectionSegment (
          IntersectionPoint(8, 1, 0, 2.04264872),
          IntersectionPoint(2, 4, 5, -2.97112541), 
          UNDEFINED));
        pf5.addIntSeg(IntersectionSegment (
          IntersectionPoint(4, 3, 3.33333333, -1.29986737), 
          IntersectionPoint(3.5, 4, 3.4375, -2.4140394), 
          RIGHT_IS_INNER));
        pf5.addIntSeg(IntersectionSegment (
          IntersectionPoint(3.5, 4, 3.4375, -2.4140394), 
          IntersectionPoint(3.2, 3.4, 4, -1.96837058), 
          RIGHT_IS_INNER));     
        pf5.setState(RELEVANT);
        PFace pf6(point9,point11,point6,point8);
        pf6.addIntSeg(IntersectionSegment (
          IntersectionPoint(6, 1, 0, 3.15682075), 
          IntersectionPoint(0, 4, 5, 3.71390676), 
          UNDEFINED));
        pf6.addIntSeg(IntersectionSegment (
          IntersectionPoint(7, 3.5, 0, 5.84940315), 
          IntersectionPoint(1, 6.5, 5, 6.40648917), 
          UNDEFINED));
        pf6.addIntSeg(IntersectionSegment (
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, 3.28061764), 
          IntersectionPoint(3.5, 4, 2.56944444, 5.01377413), 
          LEFT_IS_INNER));
        pf6.addIntSeg(IntersectionSegment (
          IntersectionPoint(3.5, 4, 2.56944444, 5.01377413), 
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.45393329), 
          LEFT_IS_INNER));
        pf6.setState(RELEVANT);
        sourceUnit4.addPFace(pf4);
        sourceUnit4.addPFace(pf5);
        sourceUnit4.addPFace(pf6);
        timeValues2.addTimeValue(0);
        timeValues2.addTimeValue(1.11111111);
        timeValues2.addTimeValue(2.56944444); 
        timeValues2.addTimeValue(2.66666667); 
        timeValues2.addTimeValue(3.33333333);
        timeValues2.addTimeValue(3.4375);
        timeValues2.addTimeValue(4);
        timeValues2.addTimeValue(5);
        assert_("SourceUnitTest1 2.1", "source units are equal.",
                sourceUnit1 == sourceUnit3);
        assert_("SourceUnitTest1 2.2", "source units are equal.",
                sourceUnit2 == sourceUnit4);
        assert_("SourceUnitTest1 2.3", "time values are equal.",
                timeValues1 == timeValues2);
        // cout << setprecision(2);
        // cout << sourceUnit1 << endl;
        // cout << sourceUnit2 << endl;
        // cout << sourceUnit3 << endl;
        // cout << sourceUnit4 << endl;
        // cout << timeValues1 << endl;
      }// SourceUnitTest2
     
     void SourceUnitTest3(){
        SourceUnit sourceUnit1,sourceUnit2, sourceUnit3, sourceUnit4;
        GlobalTimeValues timeValues1,timeValues2;
        // points from unit 1 
        Point3D point0(2,1,0);
        Point3D point1(5,1,0);         
        Point3D point2(3.5,4,0);
        Point3D point3(2,1,5);
        Point3D point4(5,1,5);         
        Point3D point5(3.5,4,5);
        // points from pface 2
        Point3D point6(6,1,0);
        Point3D point7(9,1,0);         
        Point3D point8(7.5,4,0);
        Point3D point9(0,4,5);
        Point3D point10(3,4,5);         
        Point3D point11(1.5,7,5);
        // add pfaces to unit 1 
        sourceUnit1.addPFace(point4,point3,point1,point0);
        sourceUnit1.addPFace(point5,point4,point2,point1);
        sourceUnit1.addPFace(point3,point5,point0,point2);
        // add pfaces to unit 2
        sourceUnit2.addPFace(point10,point9,point7,point6);
        sourceUnit2.addPFace(point11,point10,point8,point7);
        sourceUnit2.addPFace(point9,point11,point6,point8);
        // intersection
        sourceUnit1.intersection(sourceUnit2,timeValues1); 
        // intersection
        PFace pf1(point4,point3,point1,point0);
        pf1.setState(UNKNOWN);
        PFace pf2(point5,point4,point2,point1);
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, -2.01246118), 
          IntersectionPoint(3.5, 4, 5, -2.01246118), 
          UNDEFINED));
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(5, 1, 0, 1.34164079), 
          IntersectionPoint(5, 1, 5, 1.34164079), 
          UNDEFINED));
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, 0.596284794),
          IntersectionPoint(3.5, 4, 2.66666667, -2.01246118), 
          RIGHT_IS_INNER));
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, 0.596284794), 
          IntersectionPoint(3.66666667, 3.66666667, 4.44444444, -1.63978318), 
          LEFT_IS_INNER));
        pf2.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, -2.01246118), 
          IntersectionPoint(3.66666667, 3.66666667, 4.44444444, -1.63978318), 
          RIGHT_IS_INNER));
        pf2.setState(RELEVANT);
        PFace pf3(point3,point5,point0,point2);
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(2, 1, 0, 1.78885438), 
          IntersectionPoint(2, 1, 5, 1.78885438), 
          UNDEFINED));
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, 5.14295635), 
          IntersectionPoint (3.5, 4, 5, 5.14295635), 
          UNDEFINED));
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 4.91934955), 
          RIGHT_IS_INNER));
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.5, 4, 2.66666667, 5.14295635), 
          LEFT_IS_INNER));
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, 5.14295635), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 4.91934955), 
          LEFT_IS_INNER));
        pf3.setState(RELEVANT);
        sourceUnit3.addPFace(pf1);
        sourceUnit3.addPFace(pf2);
        sourceUnit3.addPFace(pf3);
        // pfaces from result unit 4
        PFace pf4(point10,point9,point7,point6);
        pf4.addIntSeg(IntersectionSegment(
          IntersectionPoint(9, 1, 0, -9), 
          IntersectionPoint(3, 4, 5, -3), 
          UNDEFINED));
        pf4.addIntSeg(IntersectionSegment(
          IntersectionPoint(6, 1, 0, -6), 
          IntersectionPoint(0, 4, 5, 0), 
          UNDEFINED));
        pf4.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -4.66666667),
          IntersectionPoint(3.66666667, 3.66666667, 4.44444444, -3.66666667), 
          RIGHT_IS_INNER));
        pf4.addIntSeg(IntersectionSegment (
          IntersectionPoint(2.8, 2.6, 2.66666667, -2.8), 
          IntersectionPoint(3.4, 3.8, 4.66666667, -3.4), 
          LEFT_IS_INNER));
        pf4.setState(RELEVANT);
        PFace pf5(point11,point10,point8,point7);
        pf5.addIntSeg(IntersectionSegment(
          IntersectionPoint(7.5, 4, 0, -0.223606798),
          IntersectionPoint(1.5, 7, 5, -5.59016994), 
          UNDEFINED));
        pf5.addIntSeg(IntersectionSegment(
          IntersectionPoint(9, 1, 0, 3.13049517), 
          IntersectionPoint(3, 4, 5, -2.23606798), 
          UNDEFINED));
        pf5.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, -2.01246118), 
          IntersectionPoint(3.4, 3.8, 4.66666667, -1.8782971), 
          RIGHT_IS_INNER));
        pf5.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, -2.01246118), 
          IntersectionPoint(3.66666667, 3.66666667, 4.44444444, -1.63978318), 
          LEFT_IS_INNER));
        pf5.setState(RELEVANT);
        PFace pf6(point9,point11,point6,point8);
        pf6.addIntSeg(IntersectionSegment(
          IntersectionPoint(6, 1, 0, 3.57770876), 
          IntersectionPoint(0, 4, 5, 3.57770876), 
          UNDEFINED));
        pf6.addIntSeg(IntersectionSegment(
          IntersectionPoint(7.5, 4, 0, 6.93181073), 
          IntersectionPoint(1.5, 7, 5, 6.93181073), 
          UNDEFINED));
        pf6.addIntSeg(IntersectionSegment (
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, 3.57770876), 
          IntersectionPoint(3.5, 4, 2.66666667, 5.14295635), 
          LEFT_IS_INNER));
        pf6.addIntSeg(IntersectionSegment (
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.5, 4, 2.66666667, 5.14295635), 
          RIGHT_IS_INNER));
        pf6.setState(RELEVANT);
        sourceUnit4.addPFace(pf4);
        sourceUnit4.addPFace(pf5);
        sourceUnit4.addPFace(pf6);
        timeValues2.addTimeValue(0);
        timeValues2.addTimeValue(1.11111111);
        timeValues2.addTimeValue(2.66666667); 
        timeValues2.addTimeValue(4.44444444);
        timeValues2.addTimeValue(4.66666667);
        timeValues2.addTimeValue(5);      
        assert_("SourceUnitTest1 3.1", "source units are equal.",
                sourceUnit1 == sourceUnit3);
        assert_("SourceUnitTest1 3.2", "source units are equal.",
                sourceUnit2 == sourceUnit4);
        assert_("SourceUnitTest1 2.3", "time values are equal.",
                timeValues1 == timeValues2);
        // cout << setprecision(9);
        // cout << sourceUnit1 << endl;
        // cout << sourceUnit2 << endl;
        // cout << sourceUnit3 << endl;
        // cout << sourceUnit4 << endl;
        // cout << timeValues1 << endl;
      }// SourceUnitTest2
/*
16 Test IntSegContainer 2

*/   
      bool compareList(const list<IntersectionSegment>& list1,  
                       const list<IntersectionSegment>& list2){
        if (list1.size() != list2.size()) return false;
        list<IntersectionSegment>::const_iterator iter1, iter2;
        for( iter1  = list1.begin(), iter2 = list2.begin(); 
             iter1 != list1.end();
             iter1++,iter2++){
          if(!(*iter1 == *iter2)) return false;
        }// for
        return true;
      }// compareList       
      
      void printList(const list<IntersectionSegment>& list){       
        std::list<IntersectionSegment>::const_iterator iter;
        for( iter = list.begin(); iter != list.end(); iter++){
          cout << *iter << endl;
        }// for
      }// printList        
          
      void IntSegContainerTest3(){
        GlobalTimeValues timeValues1;
        IntSegContainer container1;
        IntersectionSegment segment1(
          IntersectionPoint (3.5, 4, 0, -2.01246118), 
          IntersectionPoint (3.5, 4, 5, -2.01246118), 
          UNDEFINED);
        IntersectionSegment segment2(
          IntersectionPoint (5, 1, 0, 1.34164079), 
          IntersectionPoint (5, 1, 5, 1.34164079), 
          UNDEFINED);
        IntersectionSegment segment3(
          IntersectionPoint (4.66666667, 1.66666667, 1.11111111, 0.596284794),
          IntersectionPoint (3.5, 4, 2.56944444, -2.01246118),           
          RIGHT_IS_INNER);
        IntersectionSegment segment4(
          IntersectionPoint (4.66666667, 1.66666667, 1.11111111, 0.596284794),
          IntersectionPoint (4, 3, 3.33333333, -0.894427191),
          LEFT_IS_INNER);  
        IntersectionSegment segment5(
          IntersectionPoint (4, 3, 3.33333333, -0.894427191),
          IntersectionPoint (3.5, 4, 3.4375, -2.01246118), 
          LEFT_IS_INNER);
        container1.addIntSeg(segment1); 
        container1.addIntSeg(segment2);
        container1.addIntSeg(segment3);
        container1.addIntSeg(segment4);
        container1.addIntSeg(segment5);
        timeValues1.addTimeValue(0);
        timeValues1.addTimeValue(1.11111111);
        timeValues1.addTimeValue(2.56944444); 
        timeValues1.addTimeValue(2.66666667); 
        timeValues1.addTimeValue(3.33333333);
        timeValues1.addTimeValue(3.4375);
        timeValues1.addTimeValue(4);
        timeValues1.addTimeValue(5);
        list<IntersectionSegment> result1,resultOrthogonal1;
        list<IntersectionSegment> result2,resultOrthogonal2;        
        double t1,t2;
        timeValues1.first(t1);                     
        container1.first(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);    
        result2.push_back(segment1);
        result2.push_back(segment2);
        assert_("SourceUnitTest1 4.1", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 4.1", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);
        result2.clear();
        result2.push_back(segment1);
        result2.push_back(segment3);
        result2.push_back(segment4);
        result2.push_back(segment2);
        assert_("SourceUnitTest1 4.2", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 4.3", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);
        result2.clear();
        result2.push_back(segment1);
        result2.push_back(segment4);
        result2.push_back(segment2);
        assert_("SourceUnitTest1 4.3", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 4.3", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);
        assert_("SourceUnitTest1 4.4", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 4.4", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);        
        result2.clear();
        result2.push_back(segment1);
        result2.push_back(segment5);
        result2.push_back(segment2);
        assert_("SourceUnitTest1 4.5", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 4.5", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);    
        result2.clear();
        result2.push_back(segment1);
        result2.push_back(segment2);
        assert_("SourceUnitTest1 4.6", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 4.6", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);         
        assert_("SourceUnitTest1 4.7", "result lists are equal.",            
                compareList(result1,result2)); 
        assert_("SourceUnitTest1 4.7", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        // cout << setprecision(2);
        // cout << container1 << endl;
        // cout << timeValues1 << endl;
      }// IntSegContainerTest3
      
      void IntSegContainerTest4(){
        GlobalTimeValues timeValues1;
        IntSegContainer container1;
        IntersectionSegment segment1(
          IntersectionPoint(2, 1, 0, 1.78885438), 
          IntersectionPoint(2, 1, 5, 1.78885438), 
          UNDEFINED);
        IntersectionSegment segment2(
          IntersectionPoint(3.5, 4, 0, 5.14295635), 
          IntersectionPoint(3.5, 4, 5, 5.14295635), 
          UNDEFINED);
        IntersectionSegment segment3(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 4.91934955), 
          RIGHT_IS_INNER);
        IntersectionSegment segment4(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.5, 4, 2.66666667, 5.14295635), 
          LEFT_IS_INNER);
        IntersectionSegment segment5(
          IntersectionPoint(3.5, 4, 4.44444444, 5.14295635), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 4.91934955), 
          LEFT_IS_INNER); 
        container1.addIntSeg(segment1);
        container1.addIntSeg(segment2);
        container1.addIntSeg(segment3);
        container1.addIntSeg(segment4);
        container1.addIntSeg(segment5);  
        timeValues1.addTimeValue(0);
        timeValues1.addTimeValue(1.11111111);
        timeValues1.addTimeValue(2.66666667); 
        timeValues1.addTimeValue(4.44444444);
        timeValues1.addTimeValue(4.66666667);
        timeValues1.addTimeValue(5); 
        list<IntersectionSegment> result1,resultOrthogonal1;
        list<IntersectionSegment> result2,resultOrthogonal2;        
        double t1,t2;      
        timeValues1.first(t1);                     
        container1.first(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);    
        result2.push_back(segment1);
        result2.push_back(segment2);
        assert_("SourceUnitTest1 5.1", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 5.1", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);
        assert_("SourceUnitTest1 5.2", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 5.2", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);
        result2.clear();
        result2.push_back(segment1);
        result2.push_back(segment3);
        result2.push_back(segment2);
        resultOrthogonal2.push_back(segment4);
        assert_("SourceUnitTest1 5.3", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 5.3", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        // printList(resultOrthogonal1);
        // printList(resultOrthogonal2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);
        result2.clear();
        resultOrthogonal2.clear();
        result2.push_back(segment1);
        result2.push_back(segment3);
        result2.push_back(segment5);
        result2.push_back(segment2);
        assert_("SourceUnitTest1 5.4", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 5.4", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        // printList(resultOrthogonal1);
        // printList(resultOrthogonal2);
        t1 = t2;
        container1.next(t1,result1,resultOrthogonal1);
        timeValues1.next(t2);
        result2.clear();
        result2.push_back(segment1);
        result2.push_back(segment2);
        assert_("SourceUnitTest1 5.5", "result lists are equal.",
                compareList(result1,result2));
        assert_("SourceUnitTest1 5.5", "orthogonal result lists are equal.",
                compareList(resultOrthogonal1,resultOrthogonal2));
        // printList(result1);
        // printList(result2);
        // printList(resultOrthogonal1);
        // printList(resultOrthogonal2);
        // cout << setprecision(2);
        // cout << container1 << endl;
        // cout << timeValues1 << endl;
      }// IntSegContainerTest4
/*
17 Test SegmentTest

*/           
      void SegmentTest(){
        Segment segment1;
        Segment segment2(3,7,INSIDE);
        Segment segment3(5,6,OUTSIDE);
        Segment segment4(segment3);
        segment1 = segment2;
        
        assert_("SegmentTest 1.1", "segments are equal.",
                segment1 == segment2);
        assert_("SegmentTest 1.2", "segments are equal.",
                segment3 == segment4);
        assert_("SegmentTest 1.3", "segment arn't equal.",
                !(segment1 == segment3));
        assert_("SegmentTest 1.4", "value are equal.",
                segment3.getHead()== 6);
        assert_("SegmentTest 1.5", "value are equal.",
                segment3.getTail()== 5);   
        assert_("SegmentTest 1.6", "predicate are equal.",
                segment3.getPredicate()== OUTSIDE);  
        // cout << segment1 << endl;
        // cout << segment2 << endl;
        // cout << segment3 << endl;
        // cout << segment4 << endl;
      }// SegmentTest
/*
18 Test ResultPfaceFactory

*/       
      void ResultPfaceFactoryTest1(){
        ResultPfaceFactory factory1(4),factory2(4);
        assert_("ResultPfaceFactoryTest 1.1", "factorys are equal.",
                 factory1 == factory2);
        factory1.addEdge(0,Segment(0,1,UNDEFINED));
        factory1.addEdge(0,Segment(2,3,UNDEFINED));
        factory1.addEdge(1,Segment(4,5,LEFT_IS_INNER));
        factory1.addEdge(2,Segment(5,6,UNDEFINED));
        factory1.addOrthogonal(0,Segment(7,8,LEFT_IS_INNER));
        factory1.addOrthogonal(3,Segment(9,10,LEFT_IS_INNER));
        factory1.setTouch(1,2);
        assert_("ResultPfaceFactoryTest 1.2", "factorys aren't equal.",
                (!(factory1 == factory2)));
        // result
        factory2.addEdge(0,Segment(0,1,UNDEFINED));
        factory2.addEdge(0,Segment(2,3,UNDEFINED));
        factory2.addEdge(1,Segment(4,5,LEFT_IS_INNER));
        factory2.addEdge(2,Segment(5,6,UNDEFINED));
        factory2.addOrthogonal(0,Segment(7,8,LEFT_IS_INNER));
        factory2.addOrthogonal(3,Segment(9,10,LEFT_IS_INNER));
        factory2.setTouch(1,2); 
        assert_("ResultPfaceFactoryTest 1.3", "factorys are equal.",
                 factory1 == factory2);
        // cout << factory1 << endl;
        // cout << factory2 << endl;
      }// ResultPfaceFactoryTest1
      
      void ResultPfaceFactoryTest2(){
        ResultPfaceFactory factory1(4),factory2(4);
        factory1.addEdge(0,Segment(0,1,UNDEFINED));
        factory1.addEdge(1,Segment(2,3,UNDEFINED));
        factory1.addEdge(1,Segment(2,3,LEFT_IS_INNER));
        factory1.addEdge(2,Segment(4,5,RIGHT_IS_INNER));
        factory1.addEdge(2,Segment(4,5,UNDEFINED));
        factory1.addEdge(2,Segment(4,5,RIGHT_IS_INNER));
        // result
        factory2.addEdge(0,Segment(0,1,UNDEFINED));
        factory2.addEdge(1,Segment(2,3,LEFT_IS_INNER));
        factory2.addEdge(2,Segment(4,5,RIGHT_IS_INNER));
        assert_("ResultPfaceFactoryTest 2.1", "factorys are equal.",
                 factory1 == factory2);
        // cout << factory1 << endl;
        // cout << factory2 << endl;
      }// ResultPfaceFactoryTest2
      
      void ResultPfaceFactoryTest3(){
        GlobalTimeValues timeValues1;
        IntSegContainer container1;
        ContainerPoint3D points1,points2;
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(2, 1, 0, 1.78885438), 
          IntersectionPoint(2, 1, 5, 1.78885438), 
          UNDEFINED));
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, 5.14295635), 
          IntersectionPoint (3.5, 4, 5, 5.14295635), 
          UNDEFINED));
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 4.91934955), 
          RIGHT_IS_INNER));
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.5, 4, 2.66666667, 5.14295635), 
          LEFT_IS_INNER));
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, 5.14295635), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 4.91934955), 
          LEFT_IS_INNER));  
        timeValues1.addTimeValue(0);
        timeValues1.addTimeValue(1.11111111);
        timeValues1.addTimeValue(2.66666667); 
        timeValues1.addTimeValue(4.44444444);
        timeValues1.addTimeValue(4.66666667);
        timeValues1.addTimeValue(5); 
        // Ergebnissatz bestimmen
        ResultPfaceFactory factory1(points1,timeValues1,container1);
        ResultPfaceFactory factory2(6);
        points2.add(Point3D (2, 1, 0));
        points2.add(Point3D (2, 1, 1.11111111));
        points2.add(Point3D (3.5, 4, 0));
        points2.add(Point3D (3.5, 4, 1.11111111));
        points2.add(Point3D (2, 1, 2.66666667));
        points2.add(Point3D (3.5, 4, 2.66666667));
        points2.add(Point3D (2, 1, 4.44444444));
        points2.add(Point3D (2.8, 2.6, 2.66666667));
        points2.add(Point3D (3.33333333, 3.66666666, 4.44444444));
        points2.add(Point3D (3.5, 4, 4.44444444));
        points2.add(Point3D (2, 1, 4.66666667));
        points2.add(Point3D (3.4, 3.8, 4.66666667));
        points2.add(Point3D (3.5, 4, 4.66666667));
        points2.add(Point3D (2, 1, 5));
        points2.add(Point3D (3.5, 4, 5));
        assert_("ResultPfaceFactoryTest 3.1", "points are equal.",
                 points1 == points2);
        factory2.addEdge(0, Segment(0,1,UNDEFINED));
        factory2.addEdge(0, Segment(2,3,UNDEFINED));
        factory2.addEdge(1, Segment (1, 4, UNDEFINED));
        factory2.addEdge(1, Segment (3, 5, UNDEFINED));
        factory2.addEdge(2, Segment (4, 6, UNDEFINED));
        factory2.addEdge(2, Segment (7, 8, RIGHT_IS_INNER));
        factory2.addEdge(2, Segment (5, 9, UNDEFINED));
        factory2.addEdge(3, Segment (6, 10, UNDEFINED));
        factory2.addEdge(3, Segment (8, 11, RIGHT_IS_INNER));
        factory2.addEdge(3, Segment (9, 11, LEFT_IS_INNER));
        factory2.addEdge(3, Segment (9, 12, UNDEFINED));
        factory2.addEdge(4, Segment (10, 13, UNDEFINED));
        factory2.addEdge(4, Segment (12, 14, UNDEFINED));
        factory2.addOrthogonal(2, Segment (7, 5, LEFT_IS_INNER));
        assert_("ResultPfaceFactoryTest 3.2", "factorys are equal.",
                 factory1 == factory2);
        // cout << setprecision(9);
        // cout << container1 << endl;
        // cout << timeValues1 << endl;
        // cout << points1 << endl;
        // cout << points2 << endl;
        // cout << factory1 << endl;
        // cout << factory2 << endl;
      }// ResultPfaceFactoryTest1
      
      void SourceUnitTest4(){
        SourceUnit sourceUnit1,sourceUnit2;
        GlobalTimeValues timeValues;
        // points from unit 1 
        Point3D point0(2,1,0);
        Point3D point1(5,1,0);         
        Point3D point2(3.5,4,0);
        Point3D point3(2,1,5);
        Point3D point4(5,1,5);         
        Point3D point5(3.5,4,5);
        // points from pface 2
        Point3D point6(6,1,0);
        Point3D point7(9,1,0);         
        Point3D point8(7.5,4,0);
        Point3D point9(0,4,5);
        Point3D point10(3,4,5);         
        Point3D point11(1.5,7,5);
        // add pfaces to unit 1 
        sourceUnit1.addPFace(point4,point3,point1,point0);
        sourceUnit1.addPFace(point5,point4,point2,point1);
        sourceUnit1.addPFace(point3,point5,point0,point2);
        // add pfaces to unit 2
        sourceUnit2.addPFace(point10,point9,point7,point6);
        sourceUnit2.addPFace(point11,point10,point8,point7);
        sourceUnit2.addPFace(point9,point11,point6,point8);
        // intersection
        sourceUnit1.intersection(sourceUnit2,timeValues); 
        ContainerPoint3D points;
        sourceUnit1.createResultPfaces(points,timeValues);
        cout << points;
        
        // cout << setprecision(9);
        // cout << sourceUnit1 << endl;
        // cout << sourceUnit2 << endl;
        // cout << timeValues << endl;
      }// SourceUnitTest4
      
      
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
