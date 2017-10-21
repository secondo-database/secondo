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

Mai - November 2017, U. Wiesecke for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/

#include "Selftest.h"

using namespace std;

namespace temporalalgebra {
  namespace mregionops3 {
    
      Selftest::Selftest(){
        numberOfTestsRun=0;
        numberOfTestsFailed=0;
      }// Konstruktor
    
      Selftest::~Selftest(){
      }// Destruktortor
    
      bool Selftest::run(){
        
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
        
        RationalSegment2DTest1();
        RationalSegment2DTest2();
      
        
        Point3DContainerTest();
        
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
     
        MSegmentTest1();
        MSegmentTest2(); 
        
        CriticalMSegmentTest1();
     
        PFaceTest1();           
        PFaceTest2();
        PFaceTest3();
        PFaceTest4();
        PFaceTest5();      
        PFaceTest6();
        PFaceTest7();
        PFaceTest8();
        PFaceTest9();
        PFaceTest10();       
        PFaceTest11();        
        PFaceTest12();
        PFaceTest13();
      
        GlobalTimeValuesTest1();
        GlobalTimeValuesTest2();
             
        SourceUnitTest1();
        SourceUnitTest2();
        SourceUnitTest3();
        
        IntSegContainerTest3();
        IntSegContainerTest4();
        IntSegContainerTest5();
        
        SegmentTest();
   
//       ResultUnitFactoryTest1();
//        ResultUnitFactoryTest2();
//        ResultUnitFactoryTest3();       
//        ResultUnitFactoryTest4();
//        ResultUnitFactoryTest5();
//        ResultUnitFactoryTest6();
//        ResultUnitFactoryTest7();        
//        ResultUnitFactoryTest8();
        
        SegmentContainerTest1();
     
        SourceUnitTest4();        

        UnitsTest1();
        UnitsTest2();
        UnitsTest3();       
        UnitsTest4();     
        UnitsTest5();        
        UnitsTest6();
        UnitsTest7();
        UnitsTest8();
        UnitsTest9();
        UnitsTest10();

        SourceUnitPairTest1();   
        
        LayerTest1();
        LayerTest2();
        LayerTest3();
        
        LayerContainerTest1();
        LayerContainerTest2();
        LayerContainerTest3();
        LayerContainerTest4();
        LayerContainerTest5();
        LayerContainerTest6();
        LayerContainerTest7();
        LayerContainerTest8();
        LayerContainerTest9();
  
        cerr << endl;
        cerr << numberOfTestsRun << " tests run, ";
        cerr << numberOfTestsFailed << " tests failed." << endl <<endl;  
        return (numberOfTestsFailed==0);
      }// run

      void Selftest::assert_(string test,  string message, bool success){
        numberOfTestsRun++;
        if(!success){
          numberOfTestsFailed++;
          cerr << "Test failed: "<< test << ": "  << message << endl;
        }// if
      }// assert_       
/*
1 Test Point3D

*/        
      void Selftest::Point3DTest1(){
        Point3D point1;
        Point3D point2(0,0,0);
        Point3D point3(1,2,3);
        Point3D point4(point3);
        Point3D point5 = point3;
        assert_("Point3DTest 1.1", "Test points are not equal.", 
                point1 == point2);
        assert_("Point3DTest 1.2", "Test points are not equal.", 
                point3 == point4);
        assert_("Point3DTest 1.3", "3D coordinates are incorect.", 
                point5.getX() == 1 && point5.getY() == 2 &&
                point5.getZ() == 3);
        // cout << point5 <<endl;
      }// Point3DTest1
/*
2 Test RationalPoint3D

*/      
      void Selftest::RationalPoint3DTest1(){
        RationalPoint3D point1;
        RationalPoint3D point2(0,0,0);
        RationalPoint3D point3(1,2,3);
        RationalPoint3D point4(point3);
        RationalPoint3D point5 = point3;
        assert_("RationalPoint3DTest 1.1", "Test points are not equal.",
                point1 == point2);
        assert_("RationalPoint3DTest 1.2", "Test points are not equal.", 
                point3 == point4);
        assert_("RationalPoint3DTest 1.3", "3D coordinates are incorect.",
                point5.getX() == 1 && point5.getY() == 2 && 
                point5.getZ() == 3);
        // cout << point5 <<endl;
      }// Point3DTest1
      
      void Selftest::RationalPoint3DTest2(){
        Point3D point1(1,2,3);
        RationalPoint3D point2(1,2,3);
        assert_("RationalPoint3DTest 2.1", "Transformation incorect.", 
                point1.getR() == point2);
        assert_("RationalPoint3DTest 2.2", "Transformation incorect.", 
                point1 == point2.getD());
        // cout << point5 <<endl;
      }// Point3DTest2     
/*
3 Test Test RationalVector3D

*/  
      void Selftest::RationalVector3DTest1(){
        RationalVector3D vector1;
        RationalVector3D vector2(0,0,0);
        RationalVector3D vector3(1,2,3);
        RationalVector3D vector4(vector3);
        RationalVector3D vector5 = vector3;
        assert_("RationalVector3DTest 1.1", "Vectors are not equal.", 
                vector1 == vector2);
        assert_("RationalVector3DTest 1.2", "Vectors are n0t equal.", 
                vector3 == vector4);
        assert_("RationalVector3DTest 1.3", "3D coordinates are incorect.",
                vector5.getX() == 1 && vector5.getY() == 2 &&
                vector5.getZ() == 3);
        // cout << vector5 << endl;
      }// RationalVector3DTest1
      
      void Selftest::RationalVector3DTest2(){
        RationalVector3D vector1(1,2,3);
        RationalVector3D vector2 = -vector1;
        mpq_class value1 = -1;
        RationalVector3D vector3 = value1 * vector2;
        RationalVector3D vector4 = vector1 * value1;
        assert_("RationalVector3DTest 2.1", "Vector operation is incorrect.",
                vector1 == vector3);
        assert_("RationalVector3DTest 2.2", "Vector operation is incorrect.",
                vector2 == vector4);
        mpq_class        value2  = vector1 * vector2;         
        RationalVector3D vector5 = vector1 ^ vector2;  
        assert_("RationalVector3DTest 2.3", "Scalar product is incorect.", 
                value2 == -14);
        assert_("RationalVector3DTest 2.4", "Cross product is incorrect.", 
                vector5 == RationalVector3D(0,0,0));                
        // cout << vector1 << endl;
        // cout << vector2 << endl;
        // cout << vector3 << endl;
        // cout << vector4 << endl;                
        // cout << value2 << endl;
        // cout << vector5 << endl;                                
      }// RationalVector3DTest2
      
       void Selftest::RationalVector3DTest3(){
        Point3D point1(1,2,3);
        Point3D point2(10,11,9);
        RationalVector3D vector1 = point2.getR() - point1.getR();
        RationalVector3D vector2(9,9,6);
        Point3D point3 = (point1.getR() + vector1).getD();
        Point3D point4 = (point2.getR() - vector1).getD();
        assert_("RationalVector3DTest 3.1", "Vector operation is incorrect.", 
                vector1 == vector2);
        assert_("RationalVector3DTest 3.2", "Vector operation is incorrect.",
                point2 == point3);
        assert_("RationalVector3DTest 3.3", "Vector operation is incorrect.", 
                point1 == point4);    
        // cout << vector1 << endl;
        // cout << point3 << endl;
        // cout << point4 << endl;       
      }// Point3DTest1      
/*
4 Test Segment3D

*/
      void Selftest::Segment3DTest(){
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);  
        Point3D point3(3,4,1);
        Segment3D segment1(point1,point2);
        Segment3D segment2(point1,point3);
        Segment3D segment3 = segment1;
        assert_("Segment3DTest 1.1", "Segment point is  aren't equal.", 
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
      void Selftest::RationalSegment3DTest1(){
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
      
      void Selftest::RationalSegment3DTest2(){
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
      void Selftest::Point2DTest1(){
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
      void Selftest::RationalPoint2DTest1(){
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
      
      void Selftest::RationalPoint2DTest2(){
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
      void Selftest::RationalVector2DTest1(){
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
      void Selftest::Segment2DTest1(){
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

      void Selftest::Segment2DTest2(){
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
9 Test RationalSegment2D

*/ 
      void Selftest::RationalSegment2DTest1(){
        Point2D point1(1,2);
        Point2D point2(3,4);  
        RationalPoint2D point3(3,4);
        RationalSegment2D segment1(point1,point2);
        RationalSegment2D segment2(point1.getR(),point3);
        assert_("RationalSegment2DTest 1.1", "points aren't equal.", 
                segment1.getTail() == point1);
        assert_("RationalSegment2DTest 1.2", "points aren't equal.", 
                segment1.getHead() == point2); 
        // cout << segment1 << endl;
      }// RationalSegment2DTest1

      void Selftest::RationalSegment2DTest2(){
        Point2D point1(1,1);
        Point2D point2(3,4.5);  
        Point2D point3(3,2);
        Point2D point4(1,3);
        RationalSegment2D segment1(point1,point2);
        RationalSegment2D segment2(point3,point4);
        RationalPoint2D point5;
        bool result = segment1.intersection(segment2,point5); 
        assert_("RationalSegment2DTest 2.1", "segments intersect.", 
                result);
        assert_("RationalSegment2DTest 2.1", "points arn't equal.", 
                point5 == RationalPoint2D(1.888888889,2.555555556));
        // cout << point5 <<endl;
      }// RationalSegment2DTest2 
      
/*
10 Test Point3DContainer

*/   
      void Selftest::Point3DContainerTest(){
        Point3DContainer container;
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);  
        Point3D point3(3,4,1);
        size_t i1 = container.add(point1);
        size_t i2 = container.add(point2);
        size_t i3 = container.add(point3);
        size_t i4 = container.add(point2);
        assert_("Point3DContainerTest", " points index is incoorect.", 
                i1 == 0 && i2 ==1 && i3 == 2 && i4 == 1);
        // cout << container;
        // cout << i1 <<i2 <<i3 << i4<<endl;        
      }// Point3DContainerTest     
/*
11 Test RationalPoint3DExt

*/
      void Selftest::RationalPoint3DExtTest(){
        RationalPoint3DExt point1(1,2,3,UNIT_A);
        RationalPoint3DExt point2(5,6,7,UNIT_B);
        assert_("RationalPoint3DExtTest 1", "point is from pface A.", 
                point1.getSourceFlag() == UNIT_A);
        assert_("RationalPoint3DExtTest 1", "point is from pface B.", 
                point2.getSourceFlag() == UNIT_B);
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
      void Selftest::RationalPoint3DExtSetTest1(){
        RationalPoint3DExt point1(1,1,0,UNIT_A);
        RationalPoint3DExt point2(2,2,0,UNIT_A);
        RationalPoint3DExt point3(1,1,0,UNIT_B);
        RationalPoint3DExt point4(2,2,0,UNIT_B);        
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
      
      void Selftest::RationalPoint3DExtSetTest2(){
        RationalPoint3DExt point1(1,1,0,UNIT_A);
        RationalPoint3DExt point2(3,3,0,UNIT_A);
        RationalPoint3DExt point3(2,2,0,UNIT_B);
        RationalPoint3DExt point4(4,4,0,UNIT_B);        
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
      
       void Selftest::RationalPoint3DExtSetTest3(){
        RationalPoint3DExt point1(1,1,0,UNIT_A);
        RationalPoint3DExt point2(3,3,0,UNIT_B);
        RationalPoint3DExt point3(2,2,0,UNIT_A);
        RationalPoint3DExt point4(4,4,0,UNIT_B);        
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

      void Selftest::RationalPoint3DExtSetTest4(){
        RationalPoint3DExt point1(1,1,0,UNIT_A);
        RationalPoint3DExt point2(2,2,0,UNIT_A);
        RationalPoint3DExt point3(4,4,0,UNIT_A);        
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
      void Selftest::RationalPlane3DTest1(){
        Point3DContainer points;
        SegmentContainer segments;
        // points for pfaces 
        points.add( Point3D(1,2,0));
        points.add( Point3D(3,4,0));         
        points.add( Point3D(1,2,3));
        points.add( Point3D(3,4,3));
        // segments for pface
        segments.add( Segment(0,2));
        segments.add( Segment(1,3));
        // test points
        Point3D point1(1,2,0);
        Point3D point2(3,4,3);
        // points from segment1 
        Point3D point3(2,2,0);
        Point3D point4(1,3,3);
        // points from segment 2
        Point3D point5(2,2,3);
        Point3D point6(1,3,0);
        // results
        RationalPoint3D point7;
        // planes
        RationalPlane3D plane1(PFace(0,1, points, segments));
        RationalPlane3D plane2(PFace(1,0, points, segments));
        // intersection point
        RationalPoint3D point8(1.5,2.5,1.5);  
        // distance check
        assert_("RationalPlane3DTest 1.1", "distance to plane is zero.", 
                plane1.distance2ToPlane(point2) == 0.0); 
        assert_("RationalPlane3DTest 1.2", "distance to plane is zero.", 
                plane2.distance2ToPlane(point1) == 0.0);  
        assert_("RationalPlane3DTest 1.3", "distance to plane is incorrect.", 
                plane1.distance2ToPlane(point4) == 0.5); 
        assert_("RationalPlane3DTest 1.4", "distance to plane is incorrect.", 
                plane2.distance2ToPlane(point6) == 0.5);  
        // intersection
        bool result = plane1.intersection(Segment3D(point3,point4),
                                          point7);
        assert_("RationalPlane3DTest 1.5", "segment intersect plane.",result);
        assert_("RationalPlane3DTest 1.6", "intersection point is incorrect.",
                point7 == point8);
        result = plane1.intersection(Segment3D(point5,point6),
                                     point7);
        assert_("RationalPlane3DTest 1.7", "segment intersect plane.",result);
        assert_("RationalPlane3DTest 1.8", "intersection point is incorrect.",
                point7 == point8);
      }// RationalPlane3DTest1
  
      void Selftest::RationalPlane3DTest2(){
        Point3DContainer points;
        SegmentContainer segments;
        // points for pface 1 
        points.add( Point3D(1,2,0));// 0
        points.add( Point3D(3,4,0));         
        points.add( Point3D(1,2,3));
        points.add( Point3D(3,4,3)); 
        // points for pface 2
        points.add( Point3D(2,2,0));// 4
        points.add( Point3D(4,4,0));         
        points.add( Point3D(2,2,3));
        points.add( Point3D(4,4,3));
        // points for pface 3 
        points.add( Point3D(0,1,0));// 8
        points.add( Point3D(5,6,0));         
        points.add( Point3D(0,1,3));
        points.add( Point3D(5,6,3));
        // segments for pface 1
        segments.add( Segment(0,2));
        segments.add( Segment(1,3));
        // segments for pface 2
        segments.add( Segment(4,6));
        segments.add( Segment(5,7));
        // segments for pface 3
        segments.add( Segment(8,10));
        segments.add( Segment(9,11));
        // segments for pface 4
        segments.add( Segment(4,2));
        segments.add( Segment(5,3));
        // planes for pfaces
        RationalPlane3D plane1(PFace(0, 1, points,segments));
        RationalPlane3D plane2(PFace(2, 3, points,segments));
        RationalPlane3D plane3(PFace(4, 5, points,segments));
        RationalPlane3D plane4(PFace(6, 7, points,segments));
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
     
      void Selftest::RationalPlane3DTest3(){
        Point3DContainer points;
        SegmentContainer segments;
        // points for pface 1 
        points.add( Point3D(1,2,0));// 0
        points.add( Point3D(3,4,0));         
        points.add( Point3D(1,2,3));
        points.add( Point3D(3,4,3)); 
        // segments for pface 1
        segments.add( Segment(0,2));
        segments.add( Segment(1,3));
        // points for transformation 
        Point3D point1(1,2,0);
        Point3D point2(3,4,0);         
        Point3D point3(1,2,3);
        Point3D point4(3,4,3);
        // pfaces
        RationalPlane3D plane1(PFace(0, 1, points,segments));
        // transformation
        Point2D point5 = plane1.transform(point1).getD();
        Point2D point6 = plane1.transform(point2).getD();
        Point2D point7 = plane1.transform(point3).getD();
        Point2D point8 = plane1.transform(point4).getD();
        // result
        Point2D point9(2.12132034,0);
        Point2D point10(4.94974747,0);
        Point2D point11(2.12132034,3);
        Point2D point12(4.94974747,3);
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
      void Selftest::IntersectionPointTest(){
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
      void Selftest::IntersectionSegmentTest1(){
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
        assert_("IntersectionSegmentTest 1.7", "Segments are equal.",
                segment3 == segment4);
        assert_("IntersectionSegmentTest 1.8", "Segments are equal.",
                segment3 == segment5);
        assert_("IntersectionSegmentTest 1.9", "Segments arn't equal.",
                !(segment3 == IntersectionSegment()));
        assert_("IntersectionSegmentTest 1.10", "Segment isn't out of range.",
                !(segment5.isOutOfRange(2)));
        assert_("IntersectionSegmentTest 1.11", "Segment is out of range.",
                segment5.isOutOfRange(3));
        assert_("IntersectionSegmentTest 1.12", "Segment is out of range.",
                segment5.isOutOfRange(4));        
        // cout << segment1 << endl; 
        // cout << segment2 << endl; 
        // cout << segment3 << endl; 
        // cout << segment3.getSegment3D() << endl; 
        // cout << segment3.getSegment2D() << endl; 
      }// IntersectionSegmentTest
      
      void Selftest::IntersectionSegmentTest2(){
        IntersectionPoint point1(0,0,0,6);
        IntersectionPoint point2(0,0,3,7);
        IntersectionPoint point3(0,0,0,5);
        IntersectionPoint point4(0,0,3,6);
        IntersectionPoint point5(0,0,0,7);
        IntersectionPoint point6(0,0,3,7);
        IntersectionPoint point7(0,0,-1,8);
        IntersectionPoint point8(0,0,3,7);   
        IntersectionSegment segment1(point1,point2);
        IntersectionSegment segment2(point3,point4);
        IntersectionSegment segment3(point5,point6);
        IntersectionSegment segment4(point7,point8);
        assert_("IntersectionSegmentTest 2.1", "Segment isn't left.",
                !(segment1.isLeftOf(segment2)));
        assert_("IntersectionSegmentTest 2.2", "Segment is left.",
                segment1.isLeftOf(segment3));
        assert_("IntersectionSegmentTest 2.3", "Segment is left.",
                segment1.isLeftOf(segment4)); 
      }// IntersectionSegmentTest
      
      void Selftest::IntersectionSegmentTest3(){
        IntersectionPoint point1(0,0,0,0);
        IntersectionPoint point2(10,20,5,0);  
        IntersectionSegment segment1(point1,point2);
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
      void Selftest::IntSegContainerTest1(){
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
        container1.addIntSeg(IntersectionSegment(
          IntersectionPoint(0,0,0,0),
          IntersectionPoint(0,0,1,1),LEFT_IS_INNER));
        assert_("IntSegContainerTest 1.3", 
                "intersection segments are incorect.",
                container1 == container2);
        // cout << container1;
        // cout << container2;
      }// IntSegContainerTest1
      
      void Selftest::IntSegContainerTest2(){
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
16 Test MSegment

*/      
      void Selftest::MSegmentTest1(){
        Point3D point0(1,1,0);
        Point3D point1(1,2,5);
        Point3D point2(1,5,0);
        Point3D point3(1,4,5);
        Segment3D segment0(point0,point1);
        Segment3D segment1(point2,point3);
        MSegment msegment0;
        MSegment msegment1(segment0, segment1);
        MSegment msegment2 = msegment1;
        MSegment msegment3(msegment1);
        assert_("MSegmentTest1 1.1", 
                " MSegments are different.",
                (!(msegment0==msegment1)));
        assert_("MSegmentTest1 1.2", 
                " MSegments are equal.",
                msegment1==msegment2);
        assert_("MSegmentTest1 1.3", 
                " MSegments are equal.",
                msegment1==msegment3);
        // cout << msegment0 << endl;
        // cout << msegment1 << endl;
        // cout << msegment2 << endl;
        // cout << msegment3 << endl;
      }// MSegmentTest

      void Selftest::MSegmentTest2(){
        ResultUnit result1;
        Segment3D segment0(Point3D (4.666666667, 1.666666667, 0), 
                           Point3D (4.2, 2.6, 1));            
        Segment3D segment1(Point3D (4.666666667, 1.666666667, 0), 
                           Point3D (3.5, 4, 1));          
        Segment3D segment2(Point3D (4.666666667, 1.666666667, 0), 
                           Point3D (2.8, 2.6, 1));       
        Segment3D segment3(Point3D (4.666666667, 1.666666667, 0), 
                           Point3D (4.2, 2.6, 1));
        Segment3D segment4(Point3D (4.666666667, 1.666666667, 0), 
                           Point3D (3.5, 4, 1));
        Segment3D segment5(Point3D (4.666666667, 1.666666667, 0), 
                           Point3D (2.8, 2.6, 1));
        MSegment mSegment0(segment0,segment1);
        MSegment mSegment1(segment2,segment3);
        MSegment mSegment2(segment4,segment5);       
        result1.addMSegment(mSegment0,false);
        result1.addMSegment(mSegment1,false);
        result1.addMSegment(mSegment2,false);
        // cout << result1; 
        result1.finalize();
        // result
        ResultUnit result2;
        MSegment mSegment3(segment0,segment1,0,0,0,true,false);
        MSegment mSegment4(segment2,segment3,0,0,1,true,true);
        MSegment mSegment5(segment4,segment5,0,0,2,true,false); 
        // Reihenfolge der Segmente werden getauscht
        result2.addMSegment(mSegment4,true);
        result2.addMSegment(mSegment5,true);
        result2.addMSegment(mSegment3,true);
        assert_("MSegmentTest 2.1", "Msegments don't equal.",
                result1 == result2); 
        // cout << result1;
        // cout << result2;
      }// MSegmentTest2  
      
/*
16 Test CriticalMSegment

*/       
      void Selftest::CriticalMSegmentTest1(){
        Segment3D segment0( Point3D(6,1,0), Point3D(0,4,5));
        Segment3D segment1( Point3D(9,1,0), Point3D(3,4,5));
        CriticalMSegment cmSegment0;        
        CriticalMSegment cmSegment1(segment0,segment1,UNIT_B,INNER);
        CriticalMSegment cmSegment2 = cmSegment1;
        CriticalMSegment cmSegment3(segment1,segment0,UNIT_A,INNER);
        Point3D point = cmSegment2.getMidPoint();
        assert_("CriticalMSegmentTest 1.1",
                "Critical MSegments don't equal.",
                !(cmSegment0 == cmSegment1)); 
        assert_("CriticalMSegmentTest 1.2", 
                "Critical MSegments does equal.",
                cmSegment1 == cmSegment2); 
        assert_("CriticalMSegmentTest 1.3", 
                "Critical MSegments don't equal.",
                !(cmSegment2 == cmSegment3)); 
        assert_("CriticalMSegmentTest 1.4", 
                "Mid points does equal.",
                point == Point3D(4.5,2.5,2.5)); 
        assert_("CriticalMSegmentTest 1.5", 
                "Critical MSegments is not a part of unit A.",
                !(cmSegment1.isPartOfUnitA())); 
        assert_("CriticalMSegmentTest 1.6", 
                "Critical MSegments is not a part of unit A.",
                cmSegment3.isPartOfUnitA()); 
        assert_("CriticalMSegmentTest 1.7", 
                "Critical MSegments does have equal normalvectors.",
                cmSegment1.hasEqualNormalVector(cmSegment2)); 
        assert_("CriticalMSegmentTest 1.8", 
                "Critical MSegments don't have equal normalvectors.",
                !(cmSegment1.hasEqualNormalVector(cmSegment3))); 
        // cout << cmSegment0;
        // cout << cmSegment1;
        // cout << cmSegment2;
        // cout << cmSegment3;
        // cout << point << endl;
      }// CriticalMSegmentTest1
      
/*
17 Test PFace

*/
      void Selftest::PFaceTest1(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues;
        timeValues.setScaleFactor(3);
        // points for pface 1 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(4,3,0));         
        points.add(Point3D(1,2,3));
        points.add(Point3D(3,4,3)); 
        // points for pface 2 
        points.add(Point3D(2.5,1,0));// 4
        points.add(Point3D(5,1,0));         
        points.add(Point3D(2.5,4.5,3));
        points.add(Point3D(5,4.5,3)); 
        // segments for pface 1
        segments.add(Segment(0,2));
        segments.add(Segment(1,3));
        // segments for pface 2
        segments.add(Segment(4,6));
        segments.add(Segment(5,7)); 
        // Result points
        IntersectionPoint point9(2.5, 2.16666667, 1.0, 3.29983165);
        IntersectionPoint point10(3.2, 3.8, 2.4, 4.94974747);
        IntersectionPoint point11(2.5, 2.16666667, 1, 2.5);
        IntersectionPoint point12(3.2, 3.8, 2.4, 3.2);
        // pfaces  
        PFace pf1(0,1,points,segments);
        PFace pf3(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        PFace pf4(2,3,points,segments);
        bool result = pf1.intersection(pf2,timeValues);    
        pf3.addIntSeg(IntersectionSegment(point9,point10,RIGHT_IS_INNER));
        pf3.setState(RELEVANT);
        pf4.addIntSeg(IntersectionSegment(point11,point12,LEFT_IS_INNER));
        pf4.setState(RELEVANT);
        assert_("PFaceTest 1.1", "pfaces does intersect.",result);
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
     
      void Selftest::PFaceTest2(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(3);
        // points for pface 1 
        points.add(Point3D(11,12,0));
        points.add(Point3D(13,14,0));         
        points.add(Point3D(11,12,3));
        points.add(Point3D(13,14,3));
        // points for pface 2
        points.add(Point3D(2,1,0));
        points.add(Point3D(4,3,0));         
        points.add(Point3D(2,1,3));
        points.add(Point3D(4,3,3));
        // segments for pface 1
        segments.add(Segment(0,2));
        segments.add(Segment(1,3));
        // segments for pface 2
        segments.add(Segment(4,6));
        segments.add(Segment(5,7));  
        // pfaces
        PFace pf1(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        bool result = pf1.intersection(pf2,timeValues);
        assert_("PFaceTest 2.1", "pfaces does not intersect.",!result);
      }// PFaceTest2
     
      void Selftest::PFaceTest3(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(3);
        // points for pface 1 
        points.add(Point3D(1,2,0));
        points.add(Point3D(3,4,0));         
        points.add(Point3D(1,2,3));
        points.add(Point3D(3,4,3));
        // points for pface 2
        points.add(Point3D(2,1,0));
        points.add(Point3D(4,3,0));         
        points.add(Point3D(2,1,3));
        points.add(Point3D(4,3,3));
        // segments for pface 1
        segments.add(Segment(0,2));
        segments.add(Segment(1,3));
        // segments for pface 2
        segments.add(Segment(4,6));
        segments.add(Segment(5,7));  
        // pfaces
        PFace pf1(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        bool result = pf1.intersection(pf2,timeValues);
        assert_("PFaceTest 3.1", "pfaces does not intersect.",!result);
      }// PFaceTest3
      
      void Selftest::PFaceTest4(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(3);
        // points for pface 1 
        points.add(Point3D(1,2,0));
        points.add(Point3D(3,4,0));         
        points.add(Point3D(1,2,3));
        points.add(Point3D(3,4,3));
        // points for pface 2
        points.add(Point3D(2,3,0));
        points.add(Point3D(4,5,0));         
        points.add(Point3D(2,3,3));
        points.add(Point3D(4,5,3));
        // segments for pface 1
        segments.add(Segment(0,2));
        segments.add(Segment(1,3));
        // segments for pface 2
        segments.add(Segment(4,6));
        segments.add(Segment(5,7));  
        // pfaces
        PFace pf1(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        bool result = pf1.intersection(pf2,timeValues);
        assert_("PFaceTest 4.1", "pfaces does not intersect.",!result);
      }// PFaceTest4
      
      void Selftest::PFaceTest5(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(3);
        // points for pface 1 
        points.add(Point3D(1,2,0));
        points.add(Point3D(3,4,0));         
        points.add(Point3D(1,2,3));
        points.add(Point3D(3,4,3));
        // points for pface 2
        points.add(Point3D(3,2,0));
        points.add(Point3D(1,4,0));         
        points.add(Point3D(3,2,3));
        points.add(Point3D(1,4,3));
        // segments for pface 1
        segments.add( Segment(0,2));
        segments.add( Segment(1,3));
        // segments for pface 2
        segments.add( Segment(4,6));
        segments.add( Segment(5,7));  
        // result     
        IntersectionPoint point9(2, 3, 0, 3.53553391);
        IntersectionPoint point10(2, 3, 3, 3.53553391);
        IntersectionPoint point11(2, 3, 0, 0.707106781);
        IntersectionPoint point12(2, 3, 3, 0.707106781); 
        PFace pf1(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        PFace pf3(0,1,points,segments);
        PFace pf4(2,3,points,segments);      
        bool result = pf1.intersection(pf2,timeValues);    
        pf3.addIntSeg(IntersectionSegment(point9,point10,LEFT_IS_INNER));
        pf3.setState(RELEVANT);
        pf4.addIntSeg(IntersectionSegment(point11,point12,RIGHT_IS_INNER));
        pf4.setState(RELEVANT);
        assert_("PFaceTest 5.1", "pfaces does intersect.",result);
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
    
      void Selftest::PFaceTest6(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(3);
        // points for pface 1 
        points.add(Point3D(1,2,0));
        points.add(Point3D(3,4,0));         
        points.add(Point3D(1,2,3));
        points.add(Point3D(3,4,3));
        // points for pface 2
        points.add(Point3D(1,0,0));
        points.add(Point3D(-1,2,0));         
        points.add(Point3D(1,0,3));
        points.add(Point3D(-1,2,3));
        // segments for pface 1
        segments.add(Segment(0,2));
        segments.add(Segment(1,3));
        // segments for pface 2
        segments.add(Segment(4,6));
        segments.add(Segment(5,7));  
        // pfaces
        PFace pf1(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        bool result = pf1.intersection(pf2,timeValues);
        assert_("PFaceTest 6.1", "pfaces not intersect.",!result);
      }// PFaceTest4
      
      void Selftest::PFaceTest7(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(3);
        // points for pface 1 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(4,3,0));         
        points.add(Point3D(1,2,3));
        points.add(Point3D(3,4,3)); 
        // points for pface 2 
        points.add(Point3D(2.5,1,0));
        points.add(Point3D(5,1,0));         
        points.add(Point3D(2.5,4.5,3));
        points.add(Point3D(5,4.5,3));
        // segments for pface 1
        segments.add(Segment(0,2));
        segments.add(Segment(1,3));
        // segments for pface 2
        segments.add(Segment(4,6));
        segments.add(Segment(5,7));  
        // Result points
        IntersectionPoint point9(2.5, 2.16666667, 1.0, 3.29983165);
        IntersectionPoint point10(3.2, 3.8, 2.4, 4.94974747);
        IntersectionPoint point11(2.5, 2.16666667, 1, 2.5);
        IntersectionPoint point12(3.2, 3.8, 2.4, 3.2);
        // pfaces
        PFace pf1(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        PFace pf3(0,1,points,segments);
        PFace pf4(2,3,points,segments);
        bool result = pf1.intersection(pf2,timeValues);
        pf3.addIntSeg(IntersectionSegment(point9,point10,RIGHT_IS_INNER));
        pf3.setState(RELEVANT);
        pf4.addIntSeg(IntersectionSegment(point11,point12,LEFT_IS_INNER));
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
      
      void Selftest::PFaceTest8(){
        // Spezieller Test für die Fehlersuche beim Verschneiden von 
        // pFaces un der Bestimmung des Schnittsegments        
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(5);
        // points for pface 1 
        points.add(Point3D(3.5,4,0));// 0
        points.add(Point3D(3.5,4,5));         
        points.add(Point3D(5,1,0));
        points.add(Point3D(5,1,5)); 
        // points for pface 2 
        points.add(Point3D(6,1,0));// 4
        points.add(Point3D(3,2.5,5));         
        points.add(Point3D(7.5,4,0));
        points.add(Point3D(4.5,5.5,5)); 
        // Points for pface 3
        points.add(Point3D(9,1,0)); // 8
        points.add(Point3D(6,2.5,5));
        // segments 
        segments.add(Segment(0,1));
        segments.add(Segment(2,3));
        segments.add(Segment(4,5));
        segments.add(Segment(6,7));         
        segments.add(Segment(8,9));
        // pfaces  
        PFace pf1(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        PFace pf3(2,4,points,segments);
        // Intersection
        pf1.intersection(pf2,timeValues);         
        pf1.intersection(pf3,timeValues);
        // Result
        PFace pf4(0,1,points,segments);
        PFace pf5(2,3,points,segments);
        PFace pf6(2,4,points,segments);
        pf4.addIntSeg(IntersectionSegment (
          IntersectionPoint (4.66666667, 1.66666667, 2.22222222, 0.596284794), 
          IntersectionPoint (3.625, 3.75, 5, -1.73295268), 
          LEFT_IS_INNER));
        pf4.addIntSeg(IntersectionSegment (
          IntersectionPoint (4.66666667, 1.66666667, 2.22222222, 0.596284794), 
          IntersectionPoint (4.25, 2.5, 5, -0.335410197),
          LEFT_IS_INNER));
        pf4.setState(RELEVANT);
        pf5.addIntSeg(IntersectionSegment (
          IntersectionPoint (4.66666667, 1.66666667, 2.22222222, 3.57770876), 
          IntersectionPoint (3.625, 3.75, 5, 4.97525125), 
          RIGHT_IS_INNER));
        pf5.setState(RELEVANT);
        pf6.addIntSeg(IntersectionSegment (
          IntersectionPoint (4.66666667, 1.66666667, 2.22222222, 4.66666667), 
          IntersectionPoint (4.25, 2.5, 5, 4.25), 
          RIGHT_IS_INNER));
        pf6.setState(RELEVANT);
        assert_("PFaceTest 8.1", "pface don't equal.",
                pf1 == pf4); 
        assert_("PFaceTest 8.2", "pface don't equal.",
                pf2 == pf5); 
        assert_("PFaceTest 8.3", "pface don't equal.",
                pf3 == pf6);         
        // cout << setprecision(9);
        // cout << pf1 << endl;
        // cout << pf2 << endl; 
        // cout << pf3 << endl;        
        // cout << pf4 << endl;
        // cout << pf5 << endl; 
        // cout << pf6 << endl;
      }// PFaceTest8  
      
      void Selftest::PFaceTest9(){        
        Point3DContainer points;
        SegmentContainer segments1,segments2;
        GlobalTimeValues timeValues(6);
        // points for pface 1
        Point3D point0(2.8, 2.6, 0);
        Point3D point1(3.333333333, 3.666666667, 5.133333333);
        Point3D point2(2, 1, 0);
        Point3D point3(2, 1, 5.133333333);
        // points for pface 2
        Point3D point4(2.8, 2.6, 0);
        Point3D point5(0.666666667, 3.666666667, 5.133333333);
        Point3D point6(4.8, 2.6, 0);
        Point3D point7(2.666666667, 3.666666667, 5.133333333);
        // points in container
        size_t i0 = points.add(point0);
        size_t i1 = points.add(point1);
        size_t i2 = points.add(point2);
        size_t i3 = points.add(point3);
        size_t i4 = points.add(point4);
        size_t i5 = points.add(point5);
        size_t i6 = points.add(point6);
        size_t i7 = points.add(point7);
        // segments for pface 1
        segments1.add(Segment(i0,i1));
        segments1.add(Segment(i2,i3)); 
        segments2.add(Segment(i4,i5));
        segments2.add(Segment(i6,i7)); 
        // pface
        PFace pf0(0,1,points,segments1);
        PFace pf1(0,1,points,segments2);
        pf0.intersection(pf1,timeValues); 
        pf0.addBorder(timeValues);
        pf1.addBorder(timeValues);
        // Build result factory
        LayerContainer layer1(points,timeValues,pf0,false);
        LayerContainer layer2(points,timeValues,pf1,false);
        layer1.evaluate();
        layer2.evaluate();        
        LayerContainer layer3(2),layer4(2);
        layer3.addNonOrthSegment(0,Segment(0,7,LEFT_IS_INNER));
        layer3.addNonOrthSegment(0,Segment(2,8,OUTER));
        layer3.addNonOrthSegment(1,Segment(7,1,OUTER));
        layer3.addNonOrthSegment(1,Segment(8,3,OUTER));        
        layer4.addNonOrthSegment(0,Segment(0,9,OUTER));
        layer4.addNonOrthSegment(0,Segment(0,7,RIGHT_IS_INNER));
        layer4.addNonOrthSegment(0,Segment(5,7,INNER));
        layer4.addNonOrthSegment(1,Segment(9,4,OUTER));
        layer4.addNonOrthSegment(1,Segment(7,6,OUTER));
        assert_("PFaceTest 9.1", "factorys are equal.",
                layer1 == layer3);
        assert_("PFaceTest 9.2", "factorys are equal.",
                layer2 == layer4);
        // cout << points;
        // cout << segments;
        // cout << pf0;
        // cout << pf1;
        // cout << layer1;
        // cout << layer2;
      }// PFaceTest9
      
      void Selftest::PFaceTest10() {  
        Point3DContainer points;
        SegmentContainer segments1,segments2;
        GlobalTimeValues timeValues(5);
        // points for pface 0 
        size_t i0 = points.add(Point3D(6,1,0));// 0
        size_t i1 = points.add(Point3D(0,4,5));
        size_t i2 = points.add(Point3D(9,1,0));
        size_t i3 = points.add(Point3D(3,4,5)); 
        // points for pface 1 
        size_t i4 = points.add(Point3D(6,1,0));// 6
        size_t i5 = points.add(Point3D(0,4,5));      
        size_t i6 = points.add(Point3D(8,1,0));  
        size_t i7 = points.add(Point3D(2,4,5));
        // segments for pface 0
        segments1.add(Segment(i0,i1));
        segments1.add(Segment(i2,i3));
        // segments for pface 1
        segments2.add(Segment(i4,i5));
        segments2.add(Segment(i6,i7)); 
        // create pface
        PFace pf0(0,1,points,segments1);
        PFace pf1(0,1,points,segments2);
        PFace pf2(pf0);
        PFace pf3(pf1);
        // create plane for intersection
        RationalPlane3D plane0(pf0);
        RationalPlane3D plane1(pf1);
        // intersection on plane
        pf0.intersectionOnPlane(pf1,plane0,timeValues);   
        pf1.intersectionOnPlane(pf0,plane1,timeValues);   
        pf2.addIntSeg(IntersectionSegment (
          IntersectionPoint (8, 1, 0, 8), 
          IntersectionPoint (2, 4, 5, 2), 
          NO_INTERSECT));
        pf2.setState(CRITICAL);
        pf3.setState(CRITICAL); 
        assert_("PFaceTest 10.1", "PFaces don't equal.",
                pf0 == pf2);
        assert_("PFaceTest 10.2", "PFaces don't equal.",
                pf1 == pf3);
        // cout << pf0;
        // cout << pf1;
        // cout << pf2;
        // cout << pf3;        
      }// PFaceTest10
      
      void Selftest::PFaceTest11() {  
        Point3DContainer points;
        SegmentContainer segments1,segments2;
        GlobalTimeValues timeValues(5);
        // points for pface 0 
        size_t i0 = points.add(Point3D(1.2, 3.4, 0));// 0
        size_t i1 = points.add(Point3D(0.666666667, 3.666666667, 4.333333333));
        size_t i2 = points.add(Point3D(3.2, 3.4, 0));
        size_t i3 = points.add(Point3D(2.666666667, 3.666666667, 4.333333333)); 
        size_t i4 = points.add(Point3D(1.2, 3.4, 0));// 1
        size_t i5 = points.add(Point3D(0.666666667, 3.666666667, 4.333333333));
        size_t i6 = points.add(Point3D(3.2, 3.4, 0));
        size_t i7 = points.add(Point3D(3.333333333, 3.666666667, 4.333333333)); 
        // segments for pface 0
        segments1.add(Segment(i0,i1));
        segments1.add(Segment(i2,i3));
        // segments for pface 1
        segments2.add(Segment(i4,i5));
        segments2.add(Segment(i6,i7)); 
        // create pface
        PFace pf0(0,1,points,segments1);
        PFace pf1(0,1,points,segments2);
        PFace pf2(pf0);
        PFace pf3(pf1);
        // create plane for intersection
        RationalPlane3D plane0(pf0);
        RationalPlane3D plane1(pf1);
        // intersection on plane
        pf0.intersectionOnPlane(pf1,plane0,timeValues);   
        pf1.intersectionOnPlane(pf0,plane1,timeValues);   
        pf3.addIntSeg(IntersectionSegment (
          IntersectionPoint(3.2, 3.4, 0, 3.2),
          IntersectionPoint(2.666666667, 3.666666667, 4.333333333, 2.666666667),
          NO_INTERSECT));
        pf2.setState(CRITICAL);
        pf3.setState(CRITICAL); 
        assert_("PFaceTest 11.1", "PFaces don't equal.",
                pf0 == pf2);
        assert_("PFaceTest 11.2", "PFaces don't equal.",
                pf1 == pf3);
        // cout << pf0;
        // cout << pf1;
        // cout << pf2;
        // cout << pf3;        
      }// PFaceTest11
      
      void Selftest::PFaceTest12() {        
        Point3DContainer points;
        SegmentContainer segments1,segments2,segments3;
        GlobalTimeValues timeValues(5);
        // points for pface 0 and 2 
        size_t  i0 = points.add(Point3D(6.21621622222, 1.89189188889, 0));// 0
        size_t  i1 = points.add(Point3D(5.6, 2.2, 2.61621622222));
        size_t  i2 = points.add(Point3D(5.21621622222, 4.39189188889, 0));
        size_t  i3 = points.add(Point3D(4.6, 4.7, 2.61621622222)); 
        // points for pface 1 
        size_t  i4 = points.add(Point3D(4.98648648649, 1.89189189189, 0));// 1
        size_t  i5 = points.add(Point3D(5.5, 2.2, 2.61621622222));
        size_t  i6 = points.add(Point3D(7.21621621622, 1.89189189189, 0));
        size_t  i7 = points.add(Point3D(6.6, 2.2, 2.61621622222)); 
        // points for pface 3 
        size_t  i8 = points.add(Point3D(4.98648648649, 1.891891892, 0));// 3
        size_t  i9 = points.add(Point3D(5.5, 2.2, 2.61621622222));
        size_t i10 = points.add(Point3D(7.21621621622, 1.89189189189, 0));
        size_t i11 = points.add(Point3D(6.6, 2.2, 2.61621622222)); 
        // segments for pface 0
        segments1.add(Segment(i0,i1));
        segments1.add(Segment(i2,i3));
        // segments for pface 1
        segments2.add(Segment(i4,i5));
        segments2.add(Segment(i6,i7)); 
        // segments for pface 2
        segments3.add(Segment(i8,i9));
        segments3.add(Segment(i10,i11));         
        // create pface
        PFace pf0(0,1,points,segments1);
        PFace pf1(0,1,points,segments2);
        PFace pf2(pf0);
        PFace pf3(0,1,points,segments3);
        // intersection
        pf0.intersection(pf1,timeValues);
        pf2.intersection(pf3,timeValues);
        assert_("PFaceTest 12.1", "PFaces don't equal.",
                pf0 == pf2);
        assert_("PFaceTest 12.2", "PFaces don't equal.",
                pf1 == pf3);
        // cout << setprecision(12) << points;
        // cout << setprecision(12) << pf0;
        // cout << setprecision(12) << pf1;
        // cout << setprecision(12) << pf2;
        // cout << setprecision(12) << pf3;
      }// PFaceTest12
      
      
/*
18 Test GlobalTimeValues

*/ 
      void Selftest::GlobalTimeValuesTest1(){
        GlobalTimeValues timeValues1,timeValues2,timeValues3;
        timeValues1.addTimeValue(1);
        timeValues1.addTimeValue(0);
        timeValues1.addTimeValue(1);
        timeValues1.addTimeValue(0.5);
        timeValues1.addTimeValue(0.7);
        timeValues1.addTimeValue(0.3);
        timeValues1.addTimeValue(0.5);
        assert_("GlobalTimeValuesTest 1.1", "time values are not equal.",
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
        assert_("GlobalTimeValuesTest 1.2", "time values are equal.",
                timeValues1 == timeValues2);
        assert_("GlobalTimeValuesTest 1.3", "time values are not equal.",
                !(timeValues1 == timeValues3));
        // cout << timeValues1;
        // cout << timeValues2;
        double t1,t2;
        double t3,t4;
        bool result = true;
        if (timeValues1.scaledFirst(t1,t2) && 
            timeValues2.scaledFirst(t3,t4)){
          if(t1 != t3 && t2 != t4) result = false;
          // cout << "time:=" << t1 << "," << t2 << endl;
          while(timeValues1.scaledNext(t1,t2) && 
                timeValues2.scaledNext(t3,t4)){
            if(t1 != t3 && t2 != t4) result = false;
            // cout << "time:=" << t1 << "," << t2 << endl;       
          }// while
        }// if
        assert_("GlobalTimeValuesTest 1.4", "time values are equal.",
                result);
      }// GlobalTimeValuesTest1 
      
      void Selftest::GlobalTimeValuesTest2(){
        GlobalTimeValues timeValues(100, 1001, 1002);
        timeValues.addTimeValue(100);
        timeValues.addTimeValue(0);
        timeValues.addTimeValue(25);
        assert_("GlobalTimeValuesTest 2.1", 
                "Orginal start time is not equal.",
                timeValues.getOrginalStartTime() == 1001);
        assert_("GlobalTimeValuesTest 2.2", 
                "Orginal end time is not equal.",
                timeValues.getOrginalEndTime() == 1002);
        assert_("GlobalTimeValuesTest 2.3", 
                "Orginal scaled time is not equal.",
                timeValues.getScaledStartTime() == 0);
        assert_("GlobalTimeValuesTest 2.4", 
                "Orginal scaled time is not equal.",
                timeValues.getScaledEndTime() == 100);
        double t1,t2;
        timeValues.orginalFirst(t1,t2);
        assert_("GlobalTimeValuesTest 2.5", 
                "value is no correct.",
                t1 == 1001); 
        assert_("GlobalTimeValuesTest 2.6", 
                "value is no correct.",
                t2 == 1001.25); 
        // cout << t1 << endl;
        // cout << t2 << endl;       
        timeValues.orginalNext(t1, t2);
        assert_("GlobalTimeValuesTest 2.7", 
                "value is no correct.",
                t1 == 1001.25); 
        assert_("GlobalTimeValuesTest 2.8", 
                "value is no correct.",
                t2 == 1002);  
        // cout << t1 << endl;
        // cout << t2 << endl;  
        // cout << timeValues;
      }// GlobalTimeValuesTest2
      
/*       
16 Test IntSegContainer 2

*/            
      void Selftest::IntSegContainerTest3(){
        GlobalTimeValues timeValues(5);
        IntSegContainer container;
        IntersectionSegment segment1(
          IntersectionPoint(3.5, 4, 0, 2.01246118), 
          IntersectionPoint(3.5, 4, 5, 2.01246118));
        IntersectionSegment segment2(
          IntersectionPoint(5, 1, 0, -1.34164079), 
          IntersectionPoint(5, 1, 5, -1.34164079));
        IntersectionSegment segment3(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -0.596284794),
          IntersectionPoint(3.5, 4, 2.56944444, 2.01246118),
          LEFT_IS_INNER);
        IntersectionSegment segment4(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -0.596284794),
          IntersectionPoint(4, 3, 3.33333333, 0.894427191),             
          RIGHT_IS_INNER);
        IntersectionSegment segment5(
          IntersectionPoint(4, 3, 3.33333333, 0.894427191),
          IntersectionPoint(3.5, 4, 3.4375, 2.01246118), 
          RIGHT_IS_INNER);                
        container.addIntSeg(segment1); 
        container.addIntSeg(segment2);
        container.addIntSeg(segment3);
        container.addIntSeg(segment4);
        container.addIntSeg(segment5);
        timeValues.addTimeValue(0);
        timeValues.addTimeValue(1.11111111);
        timeValues.addTimeValue(2.56944444); 
        timeValues.addTimeValue(2.66666667); 
        timeValues.addTimeValue(3.33333333);
        timeValues.addTimeValue(3.4375);
        timeValues.addTimeValue(4);
        timeValues.addTimeValue(5);
        Point3DContainer points1,points2;
        SegmentContainer segments1,segments2;            
        double t1,t2;
        timeValues.scaledFirst(t1,t2);  
        container.first(t1,t2,points1,segments1,false); 
        points2.add(Point3D(5,1,0));
        points2.add(Point3D(5,1,1.11111111));
        points2.add(Point3D(3.5,4,0));
        points2.add(Point3D(3.5,4,1.11111111));
        segments2.add(Segment(0,1));
        segments2.add(Segment(2,3));
        // cout << setprecision(9);
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 3.1", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 3.1", "result segments are incorect.",
                segments1 == segments2);    
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D(5,1,2.56944444)); 
        points2.add(Point3D(4.66666667, 1.66666667, 1.11111111));
        points2.add(Point3D(4.22916667, 2.54166667, 2.56944444));
        points2.add(Point3D(3.5,4,2.56944444));       
        segments2.add(Segment(1, 4));
        segments2.add(Segment(5, 6, RIGHT_IS_INNER));
        segments2.add(Segment(5, 7, LEFT_IS_INNER));
        segments2.add(Segment(3, 7));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 3.2", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 3.2", "result segments are incorect.",
                segments1 == segments2);         
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D(5, 1, 2.66666667));
        points2.add(Point3D(4.2, 2.6, 2.66666667));
        points2.add(Point3D(3.5, 4, 2.66666667));
        segments2.add(Segment(4, 8));
        segments2.add(Segment(6, 9, RIGHT_IS_INNER));
        segments2.add(Segment(7, 10));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 3.3", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 3.3", "result segments are incorect.",
                segments1 == segments2);         
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();
        container.next(t1,t2,points1,segments1,false);
        points2.add(Point3D(5, 1, 3.33333333));
        points2.add(Point3D(4, 3, 3.33333333));
        points2.add(Point3D(3.5, 4, 3.33333333));
        segments2.add(Segment(8, 11));
        segments2.add(Segment(9, 12, RIGHT_IS_INNER));
        segments2.add(Segment(10, 13));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 3.4", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 3.4", "result segments are incorect.",
                segments1 == segments2);       
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D(5, 1, 3.4375));
        points2.add(Point3D(3.5, 4, 3.4375));
        segments2.add(Segment(11, 14));
        segments2.add(Segment(12, 15, RIGHT_IS_INNER));
        segments2.add(Segment(13, 15));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 3.5", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 3.5", "result segments are incorect.",
                segments1 == segments2);     
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D(5, 1, 4));
        points2.add(Point3D(3.5, 4, 4));
        segments2.add(Segment(14, 16));
        segments2.add(Segment(15, 17));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 3.6", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 3.6", "result segments are incorect.",
                segments1 == segments2);              
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D(5, 1, 5));
        points2.add(Point3D(3.5, 4, 5));
        segments2.add(Segment(16, 18));
        segments2.add(Segment(17, 19));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 3.7", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 3.7", "result segments are incorect.",
                segments1 == segments2);        
        t1 = t2;
        segments1.clear();
        segments2.clear();
        container.next(t1,t2,points1,segments1,false); 
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 3.8", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 3.8", "result segments are incorect.",
                segments1 == segments2); 
        // cout << setprecision(2);
        // cout << container << endl;
        // cout << timeValues << endl;
      }// IntSegContainerTest3
      
      void Selftest::IntSegContainerTest4(){
        GlobalTimeValues timeValues(5);
        IntSegContainer container;
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
        container.addIntSeg(segment1);
        container.addIntSeg(segment2);
        container.addIntSeg(segment3);
        container.addIntSeg(segment4);
        container.addIntSeg(segment5);  
        timeValues.addTimeValue(0);
        timeValues.addTimeValue(1.11111111);
        timeValues.addTimeValue(2.66666667); 
        timeValues.addTimeValue(4.44444444);
        timeValues.addTimeValue(4.66666667);
        timeValues.addTimeValue(5);        
        Point3DContainer points1,points2;
        SegmentContainer segments1,segments2;       
        double t1,t2;
        timeValues.scaledFirst(t1,t2);  
        container.first(t1,t2,points1,segments1,false); 
        points2.add(Point3D(2,1,0));
        points2.add(Point3D(2,1,1.11111111));
        points2.add(Point3D(3.5,4,0));
        points2.add(Point3D(3.5,4,1.11111111));
        segments2.add(Segment(0,1));
        segments2.add(Segment(2,3));
        // cout << setprecision(9);
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 4.1", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 4.1", "result segments are incorect.",
                segments1 == segments2);   
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();     
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D (2, 1, 2.66666667));
        points2.add(Point3D (3.5, 4, 2.66666667));
        segments2.add(Segment(1,4,UNDEFINED));
        segments2.add(Segment (3, 5));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 4.2", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 4.2", "result segments are incorect.",
                segments1 == segments2);    
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();     
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D (2, 1, 4.44444444));
        points2.add(Point3D (2.8, 2.6, 2.66666667));
        points2.add(Point3D (3.33333333, 3.66666666, 4.44444444));
        points2.add(Point3D (3.5, 4, 4.44444444));
        segments2.add(Segment (4, 6));
        segments2.add(Segment (7, 8, RIGHT_IS_INNER));
        segments2.add(Segment (7, 5, LEFT_IS_INNER));
        segments2.add(Segment (5, 9));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 4.3", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 4.3", "result segments are incorect.",
                segments1 == segments2);
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();     
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D (2, 1, 4.66666667));
        points2.add(Point3D (3.4, 3.8, 4.66666667));
        points2.add(Point3D (3.5, 4, 4.66666667));
        segments2.add(Segment (6, 10));
        segments2.add(Segment (8, 11, RIGHT_IS_INNER));
        segments2.add(Segment (9, 11, LEFT_IS_INNER));
        segments2.add(Segment (9, 12));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 4.4", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 4.4", "result segments are incorect.",
                segments1 == segments2);
        timeValues.scaledNext(t1,t2);
        segments1.clear();
        segments2.clear();     
        container.next(t1,t2,points1,segments1,false); 
        points2.add(Point3D (2, 1, 5));
        points2.add(Point3D (3.5, 4, 5));
        segments2.add(Segment (10, 13));
        segments2.add(Segment (12, 14));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 4.5", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 4.5", "result segments are incorect.",
                segments1 == segments2);     
        t1 = t2;
        segments1.clear();
        segments2.clear();     
        container.next(t1,t2,points1,segments1,false); 
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 4.6", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 4.6", "result segments are incorect.",
                segments1 == segments2);         
         // cout << setprecision(2);
         // cout << container << endl;
         // cout << timeValues << endl;
      }// IntSegContainerTest4
      
      void Selftest::IntSegContainerTest5(){
        GlobalTimeValues timeValues(5);
        IntSegContainer container;
        IntersectionSegment segment1(
          IntersectionPoint(2, 1, 0, 1.78885438), 
          IntersectionPoint(2, 1, 5, 1.78885438), 
          UNDEFINED);
        IntersectionSegment segment2(
          IntersectionPoint(3.5, 4, 0, 5.14295635), 
          IntersectionPoint(3.5, 4, 5, 5.14295635), 
          RIGHT_IS_INNER);
        IntersectionSegment segment3(
          IntersectionPoint(2, 1, 0, 1.78885438), 
          IntersectionPoint(3.5, 4, 0, 5.14295635), 
          UNDEFINED);
        IntersectionSegment segment4(
          IntersectionPoint(2, 1, 5, 1.78885438), 
          IntersectionPoint(3.5, 4, 5, 5.14295635), 
          UNDEFINED);
        IntersectionSegment segment5(
          IntersectionPoint(2, 1, 0, 1.78885438), 
          IntersectionPoint(2, 1, 5, 1.78885438), 
          LEFT_IS_INNER);
        IntersectionSegment segment6(
          IntersectionPoint(3.5, 4, 0, 5.14295635), 
          IntersectionPoint(3.5, 4, 5, 5.14295635), 
          UNDEFINED);
        container.addIntSeg(segment1);
        container.addIntSeg(segment2);
        container.addIntSeg(segment3);
        container.addIntSeg(segment4);
        container.addIntSeg(segment5);
        container.addIntSeg(segment6);
        timeValues.addTimeValue(0);
        timeValues.addTimeValue(5);
        Point3DContainer points1,points2;
        SegmentContainer segments1,segments2;       
        double t1,t2;
        timeValues.scaledFirst(t1,t2);  
        container.first(t1,t2,points1,segments1,false); 
        points2.add(Point3D(2,1,0));
        points2.add(Point3D(2,1,5));
        points2.add(Point3D(3.5,4,0));
        points2.add(Point3D(3.5,4,5));
        segments2.add(Segment(0,1,LEFT_IS_INNER));
        segments2.add(Segment(0,2));
        segments2.add(Segment(2,3,RIGHT_IS_INNER));
        // cout << setprecision(9);
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;  
        assert_("IntSegContainerTest 5.1", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 5.1", "result segments are incorect.",
                segments1 == segments2); 
         t1 = t2;
        segments1.clear();
        segments2.clear();     
        container.next(t1,t2,points1,segments1,false); 
        segments2.add(Segment(1,3));
        // cout << points1;
        // cout << points2;
        // cout << segments1;
        // cout << segments2;
        assert_("IntSegContainerTest 5.2", "result points are incorect.",
                points1 == points2);
        assert_("IntSegContainerTest 5.2", "result segments are incorect.",
                segments1 == segments2); 
      }// IntSegContainerTest5      
/*
17 Test SegmentTest

*/
      void Selftest::SegmentTest(){
        Segment segment1;
        Segment segment2(3,7,INNER);
        Segment segment3(5,6,OUTER);
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
                segment3.getPredicate()== OUTER);  
        // cout << segment1 << endl;
        // cout << segment2 << endl;
        // cout << segment3 << endl;
        // cout << segment4 << endl;
      }// SegmentTest   
/*
18 Test ResultUnitFactoryTest

*/ 
/*
      void Selftest::ResultUnitFactoryTest1(){
        ResultUnitFactory factory1(4),factory2(4);
        assert_("ResultUnitFactoryTest 1.1", "factorys are equal.",
                 factory1 == factory2);
        factory1.addNonOrthogonalEdges(0,Segment(0,1));
        factory1.addNonOrthogonalEdges(0,Segment(2,3));
        factory1.addNonOrthogonalEdges(1,Segment(4,5,LEFT_IS_INNER));
        factory1.addNonOrthogonalEdges(2,Segment(5,6));
        factory1.addOrthogonalEdges(0,Segment(7,8,LEFT_IS_INNER));
        factory1.addOrthogonalEdges(3,Segment(9,10,LEFT_IS_INNER));
        factory1.setTouchsOnLeftBorder(1,2);
        assert_("ResultUnitFactoryTest 1.2", "factorys aren't equal.",
                (!(factory1 == factory2)));
        // result
        factory2.addNonOrthogonalEdges(0,Segment(0,1));
        factory2.addNonOrthogonalEdges(0,Segment(2,3));
        factory2.addNonOrthogonalEdges(1,Segment(4,5,LEFT_IS_INNER));
        factory2.addNonOrthogonalEdges(2,Segment(5,6));
        factory2.addOrthogonalEdges(0,Segment(7,8,LEFT_IS_INNER));
        factory2.addOrthogonalEdges(3,Segment(9,10,LEFT_IS_INNER));
        factory2.setTouchsOnLeftBorder(1,2); 
        assert_("ResultUnitFactoryTest 1.3", "factorys are equal.",
                 factory1 == factory2);
        // cout << factory1 << endl;
        // cout << factory2 << endl;
      }// ResultUnitFactoryTest1
*/ 

/*
      void Selftest::ResultUnitFactoryTest2(){
        GlobalTimeValues timeValues(5);
        IntSegContainer container;
        Point3DContainer points1,points2;
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(2, 1, 0, 1.78885438), 
          IntersectionPoint(2, 1, 5, 1.78885438)));
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, 5.14295635), 
          IntersectionPoint (3.5, 4, 5, 5.14295635)));
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 4.91934955), 
          RIGHT_IS_INNER));
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, 3.57770876), 
          IntersectionPoint(3.5, 4, 2.66666667, 5.14295635), 
          LEFT_IS_INNER));
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, 5.14295635), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 4.91934955), 
          LEFT_IS_INNER));  
        timeValues.addTimeValue(0);
        timeValues.addTimeValue(1.11111111);
        timeValues.addTimeValue(2.66666667); 
        timeValues.addTimeValue(4.44444444);
        timeValues.addTimeValue(4.66666667);
        timeValues.addTimeValue(5); 
        // Ergebnissatz bestimmen
        ResultUnitFactory factory1(points1,timeValues,container,false);
        ResultUnitFactory factory2(6);
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
        assert_("ResultUnitFactoryTest 2.1", "points are equal.",
                 points1 == points2);
        factory2.addNonOrthogonalEdges(0,Segment(0,1));
        factory2.addNonOrthogonalEdges(0,Segment(2,3));
        factory2.addNonOrthogonalEdges(1,Segment (1, 4));
        factory2.addNonOrthogonalEdges(1,Segment (3, 5));
        factory2.addNonOrthogonalEdges(2,Segment (4, 6));
        factory2.addNonOrthogonalEdges(2, Segment (7, 8, RIGHT_IS_INNER));
        factory2.addOrthogonalEdges(2,Segment (7, 5, LEFT_IS_INNER));
        factory2.addNonOrthogonalEdges(2,Segment (5, 9));
        factory2.addNonOrthogonalEdges(3,Segment (6, 10));
        factory2.addNonOrthogonalEdges(3,Segment (8, 11, RIGHT_IS_INNER));
        factory2.addNonOrthogonalEdges(3,Segment (9, 11, LEFT_IS_INNER));
        factory2.addNonOrthogonalEdges(3,Segment (9, 12));
        factory2.addNonOrthogonalEdges(4,Segment (10, 13));
        factory2.addNonOrthogonalEdges(4,Segment (12, 14));
        assert_("ResultUnitFactoryTest 2.2", "factorys are equal.",
                 factory1 == factory2);                 
        // cout << setprecision(9);
        // cout << container << endl;
        // cout << timeValues << endl;
        // cout << points1 << endl;
        // cout << points2 << endl;
        // cout << factory1 << endl;
        // cout << factory2 << endl;
      }// ResultUnitFactoryTest2
*/
/*
      void Selftest::ResultUnitFactoryTest3(){
        Point3DContainer points;
        SegmentContainer segments;
        Point3DContainer points1,points2,points3;
        GlobalTimeValues timeValues1(5);
        GlobalTimeValues timeValues2(5);
        // points for unit 1 
        points1.add(Point3D(2,1,0));// 0
        points1.add(Point3D(5,1,0));         
        points1.add(Point3D(3.5,4,0));
        points1.add(Point3D(2,1,5));
        points1.add(Point3D(5,1,5));         
        points1.add(Point3D(3.5,4,5));
        // points for unit 2 
        points1.add(Point3D(6,1,0));// 6
        points1.add(Point3D(9,1,0));         
        points1.add(Point3D(7.5,4,0));
        points1.add(Point3D(0,4,5));
        points1.add(Point3D(3,4,5));         
        points1.add(Point3D(1.5,7,5));
        // segments for pface 1
        segments.add(Segment(0,3));
        segments.add(Segment(1,4));
        segments.add(Segment(2,5));
        // segments for pface 2
        segments.add(Segment(6,9));
        segments.add(Segment(7,10)); 
        segments.add(Segment(8,11));        
        // add pfaces to unit 1 
        PFace pf0(0,1,points1,segments);
        PFace pf1(1,2,points1,segments);
        PFace pf2(2,0,points1,segments);
        // add pfaces to unit 2
        PFace pf3(3,4,points1,segments);
        PFace pf4(4,5,points1,segments);
        PFace pf5(5,3,points1,segments);        
        // intersection
        pf0.intersection(pf3,timeValues1);
        pf0.intersection(pf4,timeValues1);
        pf0.intersection(pf5,timeValues1);
        pf0.addBorder(timeValues1);
        pf1.intersection(pf3,timeValues1);
        pf1.intersection(pf4,timeValues1);
        pf1.intersection(pf5,timeValues1);
        pf1.addBorder(timeValues1);
        pf2.intersection(pf3,timeValues1);
        pf2.intersection(pf4,timeValues1);
        pf2.intersection(pf5,timeValues1);
        pf2.addBorder(timeValues1);
        pf3.addBorder(timeValues1);
        pf4.addBorder(timeValues1);
        pf5.addBorder(timeValues1);
        ResultUnitFactory factory0(points2,timeValues1,pf0,false);
        ResultUnitFactory factory1(points2,timeValues1,pf1,false);
        ResultUnitFactory factory2(points2,timeValues1,pf2,false);
        // Results
        ResultUnitFactory factory3(6), factory4(6),factory5(6);        
        points3.add(Point3D(2, 1, 0));
        points3.add(Point3D(2, 1, 1.11111111));
        points3.add(Point3D(5, 1, 0));
        points3.add(Point3D(5, 1, 1.11111111));
        points3.add(Point3D(2, 1, 2.66666667));
        points3.add(Point3D(5, 1, 2.66666667));
        points3.add(Point3D(2, 1, 4.44444444));
        points3.add(Point3D(5, 1, 4.44444444));
        points3.add(Point3D(2, 1, 4.66666667));
        points3.add(Point3D(5, 1, 4.66666667));
        points3.add(Point3D(2, 1, 5));
        points3.add(Point3D(5, 1, 5));
        points3.add(Point3D(3.5, 4, 0));
        points3.add(Point3D(3.5, 4, 1.11111111));
        points3.add(Point3D(4.66666667, 1.66666667, 1.11111111));
        points3.add(Point3D(4.2, 2.6, 2.66666667));
        points3.add(Point3D(3.5, 4, 2.66666667));
        points3.add(Point3D(3.66666667, 3.66666667, 4.44444444));
        points3.add(Point3D(3.5, 4, 4.44444444));
        points3.add(Point3D(3.5, 4, 4.66666667));
        points3.add(Point3D(3.5, 4, 5));
        points3.add(Point3D(2.8, 2.6, 2.66666667));
        points3.add(Point3D(3.33333333, 3.66666667, 4.44444444));
        points3.add(Point3D(3.4, 3.8, 4.66666667));
        assert_("ResultUnitFactoryTest 3.1", "points are equal.",
                points2 == points3);
        // cout << setprecision(9);
        // cout << points2 << endl;
        // cout << points3 << endl;
        factory3.addNonOrthogonalEdges(0,Segment(0, 1)); 
        factory3.addNonOrthogonalEdges(0,Segment(2, 3));
        factory3.addNonOrthogonalEdges(1,Segment(1, 4));
        factory3.addNonOrthogonalEdges(1,Segment(3, 5));
        factory3.addNonOrthogonalEdges(2,Segment(4, 6));
        factory3.addNonOrthogonalEdges(2,Segment(5, 7));
        factory3.addNonOrthogonalEdges(3,Segment(6, 8)); 
        factory3.addNonOrthogonalEdges(3,Segment(7, 9));
        factory3.addNonOrthogonalEdges(4,Segment(8, 10)); 
        factory3.addNonOrthogonalEdges(4,Segment(9, 11));        
        factory4.addNonOrthogonalEdges(0,Segment(2, 3));
        factory4.addNonOrthogonalEdges(0,Segment(12, 13));
        factory4.addNonOrthogonalEdges(1,Segment(3, 5));
        factory4.addNonOrthogonalEdges(1,Segment(14, 15, RIGHT_IS_INNER));
        factory4.addNonOrthogonalEdges(1,Segment(14, 16, LEFT_IS_INNER));
        factory4.addNonOrthogonalEdges(1,Segment(13, 16));
        factory4.addNonOrthogonalEdges(2,Segment(5, 7));
        factory4.addNonOrthogonalEdges(2,Segment(15, 17, RIGHT_IS_INNER));
        factory4.addNonOrthogonalEdges(2,Segment(16, 18));
        factory4.addNonOrthogonalEdges(3,Segment(7, 9));
        factory4.addOrthogonalEdges(   3,Segment(17, 18, RIGHT_IS_INNER));
        factory4.addNonOrthogonalEdges(3,Segment(18, 19));
        factory4.addNonOrthogonalEdges(4,Segment(9, 11));        
        factory4.addNonOrthogonalEdges(4,Segment(19, 20));  
        factory5.addNonOrthogonalEdges(0,Segment(12, 13));
        factory5.addNonOrthogonalEdges(0,Segment(0, 1));
        factory5.addNonOrthogonalEdges(1,Segment(13, 16));
        factory5.addNonOrthogonalEdges(1,Segment(1, 4));
        factory5.addNonOrthogonalEdges(2,Segment(16, 18));
        factory5.addOrthogonalEdges(2,   Segment(16, 21, LEFT_IS_INNER));
        factory5.addNonOrthogonalEdges(2,Segment(21, 22, LEFT_IS_INNER));
        factory5.addNonOrthogonalEdges(2,Segment(4, 6));
        factory5.addNonOrthogonalEdges(3,Segment(18, 19));
        factory5.addNonOrthogonalEdges(3,Segment(18, 23, RIGHT_IS_INNER));
        factory5.addNonOrthogonalEdges(3,Segment(22, 23, LEFT_IS_INNER));
        factory5.addNonOrthogonalEdges(3,Segment(6, 8));
        factory5.addNonOrthogonalEdges(4,Segment(19, 20));
        factory5.addNonOrthogonalEdges(4,Segment(8, 10));
        assert_("ResultUnitFactoryTest 3.2", 
                " Content in Factory  is incorrect.",
                factory0 == factory3);
        // cout << factory0 << endl;
        // cout << factory3 << endl;
        assert_("ResultUnitFactoryTest 3.3", 
                " Content in Factory is incorrect.",
                 factory1 == factory4);;
        // cout << factory1 << endl;
        // cout << factory4 << endl;
        assert_("ResultUnitFactoryTest 3.4", 
                " Content in Factory is incorrect.",
                 factory2 == factory5);
        // cout << factory2 << endl;
        // cout << factory5 << endl;
      }// ResultUnitFactoryTest3
*/      
/*
      void Selftest::ResultUnitFactoryTest4(){
        ResultUnitFactory factory1(6),factory2(6);
        factory1.addNonOrthogonalEdges(0,Segment(14, 2)); 
        factory1.addNonOrthogonalEdges(0,Segment(0, 2, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(0,Segment(1, 3, LEFT_IS_INNER));
        factory1.addNonOrthogonalEdges(0,Segment(15, 16));
        factory1.addOrthogonalEdges(0,   Segment(0, 1, LEFT_IS_INNER));
        factory1.addNonOrthogonalEdges(1,Segment(2, 17));
        factory1.addNonOrthogonalEdges(1,Segment(2, 4, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(1,Segment(3, 6, LEFT_IS_INNER));       
        factory1.addNonOrthogonalEdges(1,Segment(16, 6));
        factory1.addNonOrthogonalEdges(2,Segment(17, 18));
        factory1.addNonOrthogonalEdges(2,Segment(4, 7, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(2,Segment(5, 8, LEFT_IS_INNER));
        factory1.addNonOrthogonalEdges(2,Segment(5, 9, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(2,Segment(6, 9));
        factory1.addNonOrthogonalEdges(3,Segment(18, 10));
        factory1.addNonOrthogonalEdges(3,Segment(7, 10, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(3,Segment(8, 11, LEFT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(3,Segment(9, 19));
        factory1.addNonOrthogonalEdges(4,Segment(10, 12)); 
        factory1.addNonOrthogonalEdges(4,Segment(11, 13, LEFT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(4,Segment(19, 20));
        factory1.addOrthogonalEdges(5,   Segment(12, 13, RIGHT_IS_INNER));
        factory1.evaluate();
        // result
        factory2.addNonOrthogonalEdges(0, Segment (14, 2, OUTER)); 
        factory2.addNonOrthogonalEdges(0, Segment (0, 2, RIGHT_IS_INNER));
        factory2.addNonOrthogonalEdges(0, Segment (1, 3, LEFT_IS_INNER));
        factory2.addNonOrthogonalEdges(0, Segment (15, 16, OUTER));
        factory2.addOrthogonalEdges(0, Segment (0, 1, LEFT_IS_INNER));
        factory2.addNonOrthogonalEdges(1, Segment (2, 17, OUTER));
        factory2.addNonOrthogonalEdges(1, Segment (2, 4, RIGHT_IS_INNER));
        factory2.addNonOrthogonalEdges(1, Segment (3, 6, LEFT_IS_INNER));       
        factory2.addNonOrthogonalEdges(1, Segment (16, 6, OUTER));
        factory2.addNonOrthogonalEdges(2, Segment (17, 18, OUTER));
        factory2.addNonOrthogonalEdges(2, Segment (4, 7, RIGHT_IS_INNER));
        factory2.addNonOrthogonalEdges(2, Segment (5, 8, LEFT_IS_INNER));
        factory2.addNonOrthogonalEdges(2, Segment (5, 9, RIGHT_IS_INNER));
        factory2.addNonOrthogonalEdges(2, Segment (6, 9, INNER));
        factory2.addNonOrthogonalEdges(3, Segment (18, 10, OUTER));
        factory2.addNonOrthogonalEdges(3, Segment (7, 10, RIGHT_IS_INNER));
        factory2.addNonOrthogonalEdges(3, Segment (8, 11, LEFT_IS_INNER)); 
        factory2.addNonOrthogonalEdges(3, Segment (9, 19, OUTER));
        factory2.addNonOrthogonalEdges(4, Segment (10, 12, INNER)); 
        factory2.addNonOrthogonalEdges(4, Segment (11, 13, LEFT_IS_INNER)); 
        factory2.addNonOrthogonalEdges(4, Segment (19, 20, OUTER));
        factory2.addOrthogonalEdges(5, Segment (12, 13, RIGHT_IS_INNER));
        factory2.setTouchsOnLeftBorder(1,2);
        factory2.setTouchsOnLeftBorder(4,1);
        Predicate left, right;
        factory1.getBorderPredicates(left,right);
        assert_("ResultUnitFactoryTest 4.1",
                " Content in Factory is incorrect.",
                 factory1 == factory2);
        assert_("ResultUnitFactoryTest 4.2",
                " Border predicates from Factory are incorrect.",
                 left == INTERSECT && right == INTERSECT);
        // cout << factory1 << endl;
        // cout << factory2 << endl;     
      }// ResultUnitFactoryTest4
*/
 
/* 
      void Selftest::ResultUnitFactoryTest5(){
        ResultUnitFactory factory1(4),factory2(4);
        factory1.addNonOrthogonalEdges(0, Segment (0, 3)); 
        factory1.addNonOrthogonalEdges(0, Segment (1, 4));
        factory1.addNonOrthogonalEdges(0, Segment (2, 5));
        factory1.addNonOrthogonalEdges(1, Segment (3, 6));
        factory1.addNonOrthogonalEdges(1, Segment (4, 7));
        factory1.addNonOrthogonalEdges(1, Segment (5, 8, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(1, Segment (5, 9));
        factory1.addNonOrthogonalEdges(2, Segment (6, 10));
        factory1.addNonOrthogonalEdges(2, Segment (6, 11));
        factory1.addNonOrthogonalEdges(2, Segment (7, 11));
        factory1.addOrthogonalEdges(2, Segment (8, 9, RIGHT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(2, Segment (9, 12));
        factory1.evaluate();
        // result
        factory2.addNonOrthogonalEdges(0, Segment (0, 3, OUTER)); 
        factory2.addNonOrthogonalEdges(0, Segment (1, 4, OUTER));
        factory2.addNonOrthogonalEdges(0, Segment (2, 5, OUTER));
        factory2.addNonOrthogonalEdges(1, Segment (3, 6, OUTER));
        factory2.addNonOrthogonalEdges(1, Segment (4, 7, OUTER));
        factory2.addNonOrthogonalEdges(1, Segment (5, 8, RIGHT_IS_INNER));
        factory2.addNonOrthogonalEdges(1, Segment (5, 9, INNER));
        factory2.addNonOrthogonalEdges(2, Segment (6, 10, OUTER));
        factory2.addNonOrthogonalEdges(2, Segment (6, 11, OUTER));
        factory2.addNonOrthogonalEdges(2, Segment (7, 11, OUTER));
        factory2.addOrthogonalEdges(2, Segment (8, 9, RIGHT_IS_INNER)); 
        factory2.addNonOrthogonalEdges(2, Segment (9, 12, OUTER));
        Predicate left, right;
        factory1.getBorderPredicates(left,right);
        assert_("ResultUnitFactoryTest 5.1",
                " Content in Factory is incorrect.",
                 factory1 == factory2);
        assert_("ResultUnitFactoryTest 5.2",
                " Border predicates from Factory are incorrect.",
                 left == OUTER && right == INTERSECT);
        // cout << factory1 << endl;
        // cout << factory2 << endl; 
        // cout << "Predicate on left border:=" << toString(left) << endl;
        // cout << "Predicate on right border:=" << toString(right) << endl;
      }// ResultUnitFactoryTest5  
*/

/*
      void Selftest::ResultUnitFactoryTest8(){
        ResultUnitFactory factory1(4),factory2(4);
        factory1.addNonOrthogonalEdges(0, Segment (0, 9,OUTER)); 
        factory1.addNonOrthogonalEdges(0, Segment (3, 12));
        factory1.addNonOrthogonalEdges(1, Segment (4, 13));
        factory1.addNonOrthogonalEdges(1, Segment (5, 14,NO_INTERSECT));
        factory1.addNonOrthogonalEdges(1, Segment (6, 15));
        factory1.addNonOrthogonalEdges(2, Segment (7, 16));
        factory1.addNonOrthogonalEdges(2, Segment (8, 17));
        factory1.evaluate();
        factory2.addNonOrthogonalEdges(0, Segment (0, 9,OUTER)); 
        factory2.addNonOrthogonalEdges(0, Segment (3, 12,OUTER));
        factory2.addNonOrthogonalEdges(1, Segment (4, 13,OUTER));
        factory2.addNonOrthogonalEdges(1, Segment (5, 14,OUTER));
        factory2.addNonOrthogonalEdges(1, Segment (6, 15,OUTER));
        factory2.addNonOrthogonalEdges(2, Segment (7, 16,OUTER));
        factory2.addNonOrthogonalEdges(2, Segment (8, 17,OUTER));
        assert_("ResultUnitFactoryTest 8",
                " Content in Factory is incorrect.",
                 factory1 == factory2);
        // cout << factory1;
        // cout << factory2;
      }// ResultUnitFactoryTest8
*/      
      
/*
19 Test SegmentContainer

*/
      void Selftest::SegmentContainerTest1(){
        SegmentContainer container1,container2,container3;
        container1.add(Segment (0, 3));
        container1.add(Segment (5, 8, RIGHT_IS_INNER));
        container1.add(Segment (8, 9, LEFT_IS_INNER));
        container3 = container1;
        container1.add(Segment (0, 3, LEFT_IS_INNER));
        container1.add(Segment (8, 9, LEFT_IS_INNER));
        
        container2.add(Segment (0, 3, LEFT_IS_INNER));
        container2.add(Segment (5, 8, RIGHT_IS_INNER));
        container2.add(Segment (8, 9, LEFT_IS_INNER));
        assert_("SegmentContainerTest 1.1", 
                " Container of segments are not different.",
                 container1 == container2);
        assert_("SegmentContainerTest 1.2", 
                " Container of segments not different.",
                 !(container1 == container3));
        container3.set(0, LEFT_IS_INNER);
        assert_("SegmentContainerTest 1.3", 
                " Container of segments are not different.",
                 container1 == container3);
        container3 = container1;
        SegmentContainer container4(container1);
        assert_("SegmentContainerTest 1.4", 
                " Container of segments are not different.",
                 container1 == container3);
        assert_("SegmentContainerTest 1.5", 
                " Container of segments are not different.",
                 container1 == container4);
        // cout << container1;
        // cout << container2;
        // cout << container3;
        // cout << container4;        
      }// SegmentContainerTest1        
/*
19 Test SourceUnit

*/  
      void Selftest::SourceUnitTest1(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues1(3);
        GlobalTimeValues timeValues2(3);
        // points for pface 1 
        points.add(Point3D(1,2,0));// 0
        points.add(Point3D(3,4,0));         
        points.add(Point3D(1,2,3));
        points.add(Point3D(3,4,3)); 
        // points for pface 2 
        points.add(Point3D(3,2,0));// 4
        points.add(Point3D(1,4,0));         
        points.add(Point3D(3,2,3));
        points.add(Point3D(1,4,3));
        // segments for pface 1
        segments.add(Segment(0,2));
        segments.add(Segment(1,3));
        // segments for pface 2
        segments.add(Segment(4,6));
        segments.add(Segment(5,7));              
        // result
        PFace pf1(0,1,points,segments);
        PFace pf2(2,3,points,segments);
        pf1.intersection(pf2,timeValues1);
        pf1.addBorder(timeValues1);
        pf2.addBorder(timeValues1);
        PFace pf3(0,1,points,segments);
        PFace pf4(2,3,points,segments);
        // Intersection points
        IntersectionPoint point9(3, 4, 0, 4.94974747);
        IntersectionPoint point10(3, 4, 3, 4.94974747);
        IntersectionPoint point11(2, 3, 0, 3.53553391);
        IntersectionPoint point12(2, 3, 3, 3.53553391);
        IntersectionPoint point13(1, 2, 0, 2.12132034);
        IntersectionPoint point14(1, 2, 3, 2.12132034);
        IntersectionPoint point15(1, 4, 0, 2.12132034);
        IntersectionPoint point16(1, 4, 3, 2.12132034);
        IntersectionPoint point17(2, 3, 0, 0.707106781);
        IntersectionPoint point18(2, 3, 3, 0.707106781);
        IntersectionPoint point19(3, 2, 0, -0.707106781);
        IntersectionPoint point20(3, 2, 3, -0.707106781);
        // Intersection segments
        pf3.addIntSeg(
          IntersectionSegment(point9,point10));
        pf3.addIntSeg(
          IntersectionSegment(point11,point12,LEFT_IS_INNER));
        pf3.addIntSeg(  
          IntersectionSegment(point13,point14));
        pf4.addIntSeg(
          IntersectionSegment(point15,point16));
        pf4.addIntSeg( 
          IntersectionSegment(point17,point18,RIGHT_IS_INNER));
        pf4.addIntSeg(   
          IntersectionSegment(point19,point20));
        // state 
        pf3.setState(RELEVANT);
        pf4.setState(RELEVANT);
        // global time values
        timeValues2.addTimeValue(0);
        timeValues2.addTimeValue(3);
        assert_("SourceUnitTest 1.1", "time values are equal.",
                timeValues1 == timeValues2);
        assert_("SourceUnitTest 1.2", "source units are equal.",
                pf1 == pf3);
        assert_("SourceUnitTest 1.3", "source units are equal.",
                pf2 == pf4);
        assert_("SourceUnitTest 1.4", "time values are equal.",
                timeValues1 == timeValues2);
        // cout << setprecision(9);
        // cout << pf1 << endl;
        // cout << pf2 << endl;
        // cout << pf3 << endl;
        //  cout << pf4 << endl;
        // cout << timeValues1 << endl;
      }// SourceUnitTest1
    
      void Selftest::SourceUnitTest2(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues1(5);
        GlobalTimeValues timeValues2(5);
        // points for unit 1 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3.5,4,0));
        points.add(Point3D(2,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3.5,4,5));
        // points for unit 2 
        points.add(Point3D(6,1,0));// 6
        points.add(Point3D(8,1,0));         
        points.add(Point3D(7,3.5,0));
        points.add(Point3D(0,4,5));
        points.add(Point3D(2,4,5));         
        points.add(Point3D(1,6.5,5));
        // segments for pface 1
        segments.add(Segment(0,3));
        segments.add(Segment(1,4));
        segments.add(Segment(2,5));
        // segments for pface 2
        segments.add(Segment(6,9));
        segments.add(Segment(7,10)); 
        segments.add(Segment(8,11));  
        // pfaces for unit 1 
        PFace pf0(0,1,points,segments);
        PFace pf1(1,2,points,segments);
        PFace pf2(2,0,points,segments);
        // pfaces for unit 2
        PFace pf3(3,4,points,segments);
        PFace pf4(4,5,points,segments);
        PFace pf5(5,3,points,segments);
        // intersection
        pf0.intersection(pf4,timeValues1);
        pf0.intersection(pf5,timeValues1);
        pf0.addBorder(timeValues1);
        pf1.intersection(pf3,timeValues1);
        pf1.intersection(pf4,timeValues1);
        pf1.intersection(pf5,timeValues1);
        pf1.addBorder(timeValues1);
        pf2.intersection(pf3,timeValues1);
        pf2.intersection(pf4,timeValues1);
        pf2.intersection(pf5,timeValues1);
        pf2.addBorder(timeValues1);
        pf3.addBorder(timeValues1);
        pf4.addBorder(timeValues1);
        pf5.addBorder(timeValues1);
        // pfaces from result unit 3
        PFace pf6(0, 1, points,segments);
        pf6.addIntSeg(IntersectionSegment(
          IntersectionPoint(2, 1, 0, 2), 
          IntersectionPoint(2, 1, 5, 2)));
        pf6.addIntSeg(IntersectionSegment(
          IntersectionPoint(5, 1, 0, 5), 
          IntersectionPoint(5, 1, 5, 5)));        
        pf6.setState(UNKNOWN);
        PFace pf7(1, 2, points,segments);
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, 2.01246118), 
          IntersectionPoint(3.5, 4, 5, 2.01246118)));
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(5, 1, 0, -1.34164079), 
          IntersectionPoint(5, 1, 5, -1.34164079)));
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -0.596284794),
          IntersectionPoint(3.5, 4, 2.56944444, 2.01246118),
          LEFT_IS_INNER));
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -0.596284794),
          IntersectionPoint(4, 3, 3.33333333, 0.894427191),             
          RIGHT_IS_INNER));
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(4, 3, 3.33333333, 0.894427191),
          IntersectionPoint(3.5, 4, 3.4375, 2.01246118), 
          RIGHT_IS_INNER));
        pf7.setState(RELEVANT);
        PFace pf8(2, 0, points,segments);
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(2, 1, 0, -1.78885438), 
          IntersectionPoint(2, 1, 5, -1.78885438)));
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, -5.14295635), 
          IntersectionPoint(3.5, 4, 5, -5.14295635)));
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 2.56944444, -5.14295635),
          IntersectionPoint(2.8, 2.6, 2.66666667, -3.57770876), 
          LEFT_IS_INNER)); 
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, -3.57770876), 
          IntersectionPoint(3.2, 3.4, 4, -4.47213595), 
          LEFT_IS_INNER));
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 3.4375, -5.14295635), 
          IntersectionPoint(3.2, 3.4, 4, -4.47213595), 
          RIGHT_IS_INNER));
        pf8.setState(RELEVANT);
        // pfaces from result unit 4
        PFace pf9(3, 4, points,segments);
        pf9.addIntSeg(IntersectionSegment(
          IntersectionPoint(8, 1, 0, 8),
          IntersectionPoint(2, 4, 5, 2)));
        pf9.addIntSeg(IntersectionSegment(
          IntersectionPoint(6, 1, 0, 6),
          IntersectionPoint(0, 4, 5, 0)));
        pf9.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, 4.66666667),
          IntersectionPoint(4, 3, 3.33333333, 4), 
          LEFT_IS_INNER));
        pf9.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, 2.8), 
          IntersectionPoint(3.2, 3.4, 4, 3.2),
          RIGHT_IS_INNER));
        pf9.setState(RELEVANT);
        PFace pf10(4, 5, points,segments);
        pf10.addIntSeg(IntersectionSegment (
          IntersectionPoint(7, 3.5, 0, 0.649933684), 
          IntersectionPoint(1, 6.5, 5, 5.66370781)));
        pf10.addIntSeg(IntersectionSegment (
          IntersectionPoint(8, 1, 0, -2.04264872),
          IntersectionPoint(2, 4, 5, 2.97112541)));
        pf10.addIntSeg(IntersectionSegment (
          IntersectionPoint(4, 3, 3.33333333, 1.29986737), 
          IntersectionPoint(3.5, 4, 3.4375, 2.4140394), 
          LEFT_IS_INNER));
        pf10.addIntSeg(IntersectionSegment (
          IntersectionPoint(3.5, 4, 3.4375, 2.4140394), 
          IntersectionPoint(3.2, 3.4, 4, 1.96837058), 
          LEFT_IS_INNER));     
        pf10.setState(RELEVANT);
        PFace pf11(5, 3, points,segments);
        pf11.addIntSeg(IntersectionSegment (
          IntersectionPoint(6, 1, 0, -3.15682075), 
          IntersectionPoint(0, 4, 5, -3.71390676)));
        pf11.addIntSeg(IntersectionSegment (
          IntersectionPoint(7, 3.5, 0, -5.84940315), 
          IntersectionPoint(1, 6.5, 5, -6.40648917)));
        pf11.addIntSeg(IntersectionSegment (
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -3.28061764), 
          IntersectionPoint(3.5, 4, 2.56944444, -5.01377413), 
          RIGHT_IS_INNER));
        pf11.addIntSeg(IntersectionSegment (
          IntersectionPoint(3.5, 4, 2.56944444, -5.01377413), 
          IntersectionPoint(2.8, 2.6, 2.66666667, -3.45393329), 
          RIGHT_IS_INNER));
        pf11.setState(RELEVANT);
        timeValues2.addTimeValue(0);
        timeValues2.addTimeValue(1.11111111);
        timeValues2.addTimeValue(2.56944444); 
        timeValues2.addTimeValue(2.66666667); 
        timeValues2.addTimeValue(3.33333333);
        timeValues2.addTimeValue(3.4375);
        timeValues2.addTimeValue(4);
        timeValues2.addTimeValue(5);
        assert_("SourceUnitTest 2.1", "pface does not equal.",
                pf0 == pf6);
        assert_("SourceUnitTest 2.2", "pface does not equal.",
                pf1 == pf7);
        assert_("SourceUnitTest 2.3", "pface does not equal.",
                pf2 == pf8);
        assert_("SourceUnitTest 2.4", "pface does not equal.",
                pf3 == pf9);
        assert_("SourceUnitTest 2.5", "pface does not equal.",
                pf4 == pf10);
        assert_("SourceUnitTest 2.6", "pface does not equal.",
                pf5 == pf11);
        assert_("SourceUnitTest 2.7", "time values are equal.",
                timeValues1 == timeValues2);
        // cout << setprecision(2);
        // cout << pf0 << endl;
        // cout << pf1 << endl;
        // cout << pf2 << endl;
        // cout << pf3 << endl;
        // cout << pf4 << endl;
        // cout << pf5 << endl;
        // cout << timeValues1 << endl;
      }// SourceUnitTest2
   
     void Selftest::SourceUnitTest3(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues1(5);
        GlobalTimeValues timeValues2(5);
        // points for unit 1 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3.5,4,0));
        points.add(Point3D(2,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3.5,4,5));
        // points for unit 2 
        points.add(Point3D(6,1,0));// 6
        points.add(Point3D(9,1,0));         
        points.add(Point3D(7.5,4,0));
        points.add(Point3D(0,4,5));
        points.add(Point3D(3,4,5));         
        points.add(Point3D(1.5,7,5));
        // segments for pface 1
        segments.add(Segment(0,3,UNDEFINED));
        segments.add(Segment(1,4,UNDEFINED));
        segments.add(Segment(2,5,UNDEFINED));
        // segments for pface 2
        segments.add(Segment(6,9,UNDEFINED));
        segments.add(Segment(7,10,UNDEFINED)); 
        segments.add(Segment(8,11,UNDEFINED)); 
        // add pfaces to unit 1 
        PFace pf0(0,1,points,segments);
        PFace pf1(1,2,points,segments);
        PFace pf2(2,0,points,segments);
        // add pfaces to unit 2
        PFace pf3(3,4,points,segments);
        PFace pf4(4,5,points,segments);
        PFace pf5(5,3,points,segments);
        // intersection
        pf0.intersection(pf3,timeValues1);
        pf0.intersection(pf4,timeValues1);
        pf0.intersection(pf5,timeValues1);
        pf0.addBorder(timeValues1);
        pf1.intersection(pf3,timeValues1);
        pf1.intersection(pf4,timeValues1);
        pf1.intersection(pf5,timeValues1);
        pf1.addBorder(timeValues1);
        pf2.intersection(pf3,timeValues1);
        pf2.intersection(pf4,timeValues1);
        pf2.intersection(pf5,timeValues1);
        pf2.addBorder(timeValues1);
        pf3.addBorder(timeValues1);
        pf4.addBorder(timeValues1);
        pf5.addBorder(timeValues1);
        PFace pf6(0,1,points,segments);
        pf6.addIntSeg(IntersectionSegment(
          IntersectionPoint (2, 1, 0, 2), 
          IntersectionPoint (2, 1, 5, 2)));
        pf6.addIntSeg(IntersectionSegment(
          IntersectionPoint (5, 1, 0, 5), 
          IntersectionPoint (5, 1, 5, 5)));
        pf6.setState(UNKNOWN);
        PFace pf7(1,2,points,segments);
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, 2.01246118), 
          IntersectionPoint(3.5, 4, 5, 2.01246118)));
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(5, 1, 0, -1.34164079), 
          IntersectionPoint(5, 1, 5, -1.34164079)));
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -0.596284794),
          IntersectionPoint(3.5, 4, 2.66666667, 2.01246118), 
          LEFT_IS_INNER));
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -0.596284794), 
          IntersectionPoint(3.66666667, 3.66666667, 4.44444444, 1.63978318), 
          RIGHT_IS_INNER));
        pf7.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, 2.01246118), 
          IntersectionPoint(3.66666667, 3.66666667, 4.44444444, 1.63978318), 
          LEFT_IS_INNER));
        pf7.setState(RELEVANT);
        PFace pf8(2,0,points,segments);
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(2, 1, 0, -1.78885438), 
          IntersectionPoint(2, 1, 5, -1.78885438)));
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 0, -5.14295635), 
          IntersectionPoint (3.5, 4, 5, -5.14295635)));
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 2.66666667, -5.14295635),
          IntersectionPoint(2.8, 2.6, 2.66666667, -3.57770876),           
          LEFT_IS_INNER));
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8, 2.6, 2.66666667, -3.57770876), 
          IntersectionPoint(3.4, 3.8, 4.66666667, -4.91934955), 
          LEFT_IS_INNER));
        pf8.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, -5.14295635), 
          IntersectionPoint(3.4, 3.8, 4.66666667, -4.91934955), 
          RIGHT_IS_INNER));
        pf8.setState(RELEVANT);
        // pfaces from result unit 4
        PFace pf9(3,4,points,segments);
        pf9.addIntSeg(IntersectionSegment(
          IntersectionPoint(9, 1, 0, 9), 
          IntersectionPoint(3, 4, 5, 3)));
        pf9.addIntSeg(IntersectionSegment(
          IntersectionPoint(6, 1, 0, 6), 
          IntersectionPoint(0, 4, 5, 0)));
        pf9.addIntSeg(IntersectionSegment(
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, 4.66666667),
          IntersectionPoint(3.66666667, 3.66666667, 4.44444444, 3.66666667), 
          LEFT_IS_INNER));
        pf9.addIntSeg(IntersectionSegment (
          IntersectionPoint(2.8, 2.6, 2.66666667, 2.8), 
          IntersectionPoint(3.4, 3.8, 4.66666667, 3.4), 
          RIGHT_IS_INNER));
        pf9.setState(RELEVANT);
        PFace pf10(4,5,points,segments);
        pf10.addIntSeg(IntersectionSegment(
          IntersectionPoint(7.5, 4, 0, 0.223606798),
          IntersectionPoint(1.5, 7, 5, 5.59016994)));
        pf10.addIntSeg(IntersectionSegment(
          IntersectionPoint(9, 1, 0, -3.13049517), 
          IntersectionPoint(3, 4, 5, 2.23606798)));
        pf10.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.66666667, 3.66666667, 4.44444444, 1.63978318),
          IntersectionPoint(3.5, 4, 4.44444444, 2.01246118),
          LEFT_IS_INNER));
        pf10.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5, 4, 4.44444444, 2.01246118),
          IntersectionPoint(3.4, 3.8, 4.66666667, 1.8782971),
          LEFT_IS_INNER));       
        pf10.setState(RELEVANT);
        PFace pf11(5,3,points,segments);
        pf11.addIntSeg(IntersectionSegment(
          IntersectionPoint(6, 1, 0, -3.57770876), 
          IntersectionPoint(0, 4, 5, -3.57770876)));
        pf11.addIntSeg(IntersectionSegment(
          IntersectionPoint(7.5, 4, 0, -6.93181073), 
          IntersectionPoint(1.5, 7, 5, -6.93181073)));
        pf11.addIntSeg(IntersectionSegment (
          IntersectionPoint(4.66666667, 1.66666667, 1.11111111, -3.57770876), 
          IntersectionPoint(3.5, 4, 2.66666667, -5.14295635), 
          RIGHT_IS_INNER));
        pf11.addIntSeg(IntersectionSegment (
          IntersectionPoint(2.8, 2.6, 2.66666667, -3.57770876), 
          IntersectionPoint(3.5, 4, 2.66666667, -5.14295635), 
          LEFT_IS_INNER));
        pf11.setState(RELEVANT);
        timeValues2.addTimeValue(0);
        timeValues2.addTimeValue(1.11111111);
        timeValues2.addTimeValue(2.66666667); 
        timeValues2.addTimeValue(4.44444444);
        timeValues2.addTimeValue(4.66666667);
        timeValues2.addTimeValue(5); 
        assert_("SourceUnitTest 3.1", "pface does not equal.",
                pf0 == pf6);
        assert_("SourceUnitTest 3.2", "pface does not equal.",
                pf1 == pf7);
        assert_("SourceUnitTest 3.3", "pface does not equal.",
                pf2 == pf8);
        assert_("SourceUnitTest 3.4", "pface does not equal.",
                pf3 == pf9);
        assert_("SourceUnitTest 3.5", "pface does not equal.",
                pf4 == pf10);
        assert_("SourceUnitTest 3.6", "pface does not equal.",
                pf5 == pf11);
        assert_("SourceUnitTest 3.7", "time values are equal.",
                timeValues1 == timeValues2);
        // cout << setprecision(9);
        // cout << pf0 << endl;
        // cout << pf1 << endl;
        // cout << pf2 << endl;
        // cout << pf3 << endl;
        // cout << pf4 << endl;
        // cout << pf5 << endl;
      }// SourceUnitTest2

      void Selftest::SourceUnitTest4(){
        Point3DContainer points;
        // points for unit 0 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3.5,4,0));
        points.add(Point3D(2,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3.5,4,5));
        // segments for unit 0
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // Build unit 0
        SourceUnit unit0, unit1,unit3;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        unit1 = unit0;
        SourceUnit unit2(unit0);
        assert_("UnitTest 1.1", 
                " Units are equal.",
                 unit0 == unit1);
        assert_("UnitTest 1.2", 
                " Units are equal.",
                 unit0 == unit2);
        assert_("UnitTest 1.3", 
                " Units are not equal.",
                (!( unit0 == unit3)));
        // cout << unit0 <<endl;       
      }// SourceUnittest4   
/*
20 Test Units

*/       
      void Selftest::UnitsTest1(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(5);
        // points for unit 1 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3.5,4,0));
        
        points.add(Point3D(2,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3.5,4,5));
        // points for unit 2 
        points.add(Point3D(6,1,0));// 6
        points.add(Point3D(9,1,0));         
        points.add(Point3D(7.5,4,0));
        points.add(Point3D(0,4,5));
        points.add(Point3D(3,4,5));         
        points.add(Point3D(1.5,7,5));
        // segments for pfaces 0, 1, 2
        segments.add(Segment(0,3));
        segments.add(Segment(1,4));
        segments.add(Segment(2,5));
        // segments for pfaces 3, 4, 5
        segments.add(Segment(6,9));
        segments.add(Segment(7,10)); 
        segments.add(Segment(8,11));        
        // add pfaces to unit 1 
        PFace pf0(0,1,points,segments);
        PFace pf1(1,2,points,segments);
        PFace pf2(2,0,points,segments);
        // add pfaces to unit 2
        PFace pf3(3,4,points,segments);
        PFace pf4(4,5,points,segments);
        PFace pf5(5,3,points,segments);   
        // intersection
        pf0.intersection(pf3,timeValues);
        pf0.intersection(pf4,timeValues);
        pf0.intersection(pf5,timeValues);
        // pf0.addBorder(timeValues);
        pf1.intersection(pf3,timeValues);
        pf1.intersection(pf4,timeValues);
        pf1.intersection(pf5,timeValues);
        pf2.intersection(pf3,timeValues);
        pf2.intersection(pf4,timeValues);
        pf2.intersection(pf5,timeValues);
        pf1.addBorder(timeValues);
        pf2.addBorder(timeValues);
        pf3.addBorder(timeValues);
        pf4.addBorder(timeValues);
        pf5.addBorder(timeValues);
        // result from pface 1
        // pface with Intersection
        pf1.finalize(points,segments,timeValues);
        pf2.finalize(points,segments,timeValues);
        // pface without intersection
        pf0.addBorder(timeValues,segments,INNER);
        pf0.finalize(points,segments,timeValues);  
        // result from pface 2
        // pface with Intersection
        pf3.finalize(points,segments,timeValues);
        pf4.finalize(points,segments,timeValues);
        pf5.finalize(points,segments,timeValues);
        vector<ResultUnit> units = 
          vector<ResultUnit>(timeValues.size()-1, ResultUnit());
        for(size_t i = 0; i < timeValues.size()-1; i++){  
          pf0.getResultUnit(i,INNER,false,points,units[i],UNIT_A);
          pf1.getResultUnit(i,INNER,false,points,units[i],UNIT_A);
          pf2.getResultUnit(i,INNER,false,points,units[i],UNIT_A);
          pf3.getResultUnit(i,INNER,false,points,units[i],UNIT_B);
          pf4.getResultUnit(i,INNER,false,points,units[i],UNIT_B);
          pf5.getResultUnit(i,INNER,false,points,units[i],UNIT_B);
        }// for
        vector<ResultUnit> result = vector<ResultUnit>(5,ResultUnit());        
        Segment3D segment0(Point3D(4.666666667, 1.666666667, 1.111111111), 
                           Point3D(4.2, 2.6, 2.666666667));            
        Segment3D segment1(Point3D(4.666666667, 1.666666667, 1.111111111), 
                           Point3D(3.5, 4, 2.666666667));          
        Segment3D segment2(Point3D(4.666666667, 1.666666667, 1.111111111), 
                           Point3D(2.8, 2.6, 2.666666667));       
        Segment3D segment3(Point3D(4.666666667, 1.666666667, 1.111111111), 
                           Point3D(4.2, 2.6, 2.666666667));
        Segment3D segment4(Point3D(4.666666667, 1.666666667, 1.111111111), 
                           Point3D(3.5, 4, 2.666666667));
        Segment3D segment5(Point3D(4.666666667, 1.666666667, 1.111111111), 
                           Point3D(2.8, 2.6, 2.666666667));
        MSegment mSegment0(segment0,segment1);
        MSegment mSegment1(segment2,segment3);
        MSegment mSegment2(segment4,segment5);       
        result[1].addMSegment(mSegment0,false);
        result[1].addMSegment(mSegment1,false);
        result[1].addMSegment(mSegment2,false);   
        Segment3D segment6(Point3D(4.2, 2.6, 2.666666667), 
                           Point3D(3.666666667, 3.666666667, 4.444444444));  
        Segment3D segment7(Point3D(3.5, 4, 2.666666667), 
                           Point3D(3.5, 4, 4.444444444));  
        Segment3D segment8(Point3D(3.5, 4, 2.666666667), 
                           Point3D(3.5, 4, 4.444444444)); 
        Segment3D segment9(Point3D(2.8, 2.6, 2.666666667), 
                           Point3D(3.333333333, 3.666666667, 4.444444444));  
        Segment3D segment10(Point3D(2.8, 2.6, 2.666666667), 
                            Point3D(3.333333333, 3.666666667, 4.444444444));   
        Segment3D segment11(Point3D(4.2, 2.6, 2.666666667), 
                            Point3D(3.666666667, 3.666666667, 4.444444444));
        MSegment mSegment3(segment6,segment7);
        MSegment mSegment4(segment8,segment9);
        MSegment mSegment5(segment10,segment11);  
        result[2].addMSegment(mSegment3,false);
        result[2].addMSegment(mSegment4,false);
        result[2].addMSegment(mSegment5,false);       
        Segment3D segment12(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment13(Point3D(3.333333333, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));   
        Segment3D segment14(Point3D(3.333333333, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment15(Point3D(3.666666667, 3.666666667, 4.44444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment16(Point3D(3.666666667, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667)); 
        Segment3D segment17(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        MSegment mSegment6(segment12,segment13);
        MSegment mSegment7(segment14,segment15);
        MSegment mSegment8(segment16,segment17);  
        result[3].addMSegment(mSegment6,false);
        result[3].addMSegment(mSegment7,false);
        result[3].addMSegment(mSegment8,false);   
        assert_("UnitsTest 1.1", 
                " size of units vectors are different.",
                 units.size() == result.size());
        for(size_t i = 0; i < timeValues.size()-1; i++){ 
          // cout << result[i];
          // cout << units[i];
          assert_("UnitsTest 1.2", 
                  " Unit are different.",
                  units[i] == result[i]);
        }// for
        // cout << points << endl;
        // cout << segments << endl;
        // cout << pf0 << endl;
        // cout << pf1 << endl;
        // cout << pf2 << endl;
        // cout << pf3 << endl;
        // cout << pf4 << endl;
        // cout << pf5 << endl;
      }// UnitsTest1
      
      void Selftest::UnitsTest2(){
        GlobalTimeValues timeValues(5);
        Point3DContainer points;
        SegmentContainer segments;
        // points for unit 0 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3.5,4,0));
        points.add(Point3D(2,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3.5,4,5));
        // points for unit 0 
        points.add(Point3D(12,1,0));// 6
        points.add(Point3D(15,1,0));         
        points.add(Point3D(13.5,4,0));
        points.add(Point3D(12,1,5));
        points.add(Point3D(15,1,5));         
        points.add(Point3D(13.5,4,5));
        // points for unit 1
        points.add(Point3D(6,1,0));// 12
        points.add(Point3D(9,1,0));         
        points.add(Point3D(7.5,4,0));
        points.add(Point3D(0,4,5));
        points.add(Point3D(3,4,5));         
        points.add(Point3D(1.5,7,5));
        // segments for pfaces 0, 1, 2
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // segments for pfaces 3, 4, 5
        Segment segment3(6,9);
        Segment segment4(7,10);
        Segment segment5(8,11);
        // segments for Pfaces 6, 7, 8
        Segment segment6(12,15);
        Segment segment7(13,16);
        Segment segment8(14,17);
        SourceUnit unit0;  
        // Object 0
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Object 1
        unit0.addPFace(segment3,segment4,points);
        unit0.addPFace(segment4,segment5,points);
        unit0.addPFace(segment5,segment3,points);       
        // Object 2
        SourceUnit unit1;  
        unit1.addPFace(segment6,segment7,points);
        unit1.addPFace(segment7,segment8,points);
        unit1.addPFace(segment8,segment6,points); 
        unit0.reSort();
        unit1.reSort();                
        // Intersection
        unit0.intersection(unit1, timeValues);        
        // cout << unit0;
        // cout << unit1;
        // unit0.printFaceCycleEntrys();
        // unit1.printFaceCycleEntrys();        
        // Finalize
        unit0.finalize(points,timeValues,OUTER,unit1);
        unit1.finalize(points,timeValues,INNER,unit0);
        // get result Units
        vector<ResultUnit> units = vector<ResultUnit>(
          timeValues.size()-1, ResultUnit());
        for(size_t i = 0; i < timeValues.size()-1; i++){  
          unit0.getResultUnit(i,OUTER,false,points,units[i],UNIT_A);
          unit1.getResultUnit(i,INNER,true,points,units[i],UNIT_B);
          units[i].evaluateCriticalMSegmens(MINUS);
          units[i].finalize();  
        }// for        
        // cout << unit0;
        // cout << unit1;
        // unit0.printFaceCycleEntrys();
        // unit1.printFaceCycleEntrys();  
        ResultUnit result0 = units[0];
        ResultUnit result1;                
        Segment3D segment10(Point3D(2, 1, 0), 
                            Point3D(2, 1, 1.111111111));
        Segment3D segment11(Point3D(5, 1, 0), 
                            Point3D(5, 1, 1.111111111));
        MSegment mSegment0(segment10,segment11,0,0,0,true,true);
        result1.addMSegment(mSegment0,true);
        Segment3D segment12(Point3D(3.5, 4, 0),
                            Point3D(3.5, 4, 1.111111111));
        Segment3D segment13(Point3D(2, 1, 0), 
                            Point3D(2, 1, 1.111111111));
        MSegment mSegment1(segment12,segment13,0,0,1,true,false);
        result1.addMSegment(mSegment1,true);
        Segment3D segment14(Point3D(5, 1, 0),
                            Point3D(5, 1, 1.111111111));
        Segment3D segment15(Point3D(3.5, 4, 0), 
                            Point3D(3.5, 4, 1.111111111));
        MSegment mSegment2(segment14,segment15,0,0,2,true,false);
        result1.addMSegment(mSegment2,true);
        Segment3D segment16(Point3D(12, 1, 0),
                            Point3D(12, 1, 1.111111111));
        Segment3D segment17(Point3D(15, 1, 0),
                            Point3D(15, 1, 1.111111111));
        MSegment mSegment3(segment16,segment17,1,0,0,true,true);
        result1.addMSegment(mSegment3,true);
        Segment3D segment18(Point3D(13.5, 4, 0),
                            Point3D(13.5, 4, 1.111111111));
        Segment3D segment19(Point3D(12, 1, 0), 
                            Point3D(12, 1, 1.111111111));
        MSegment mSegment4(segment18,segment19,1,0,1,true,false);
        result1.addMSegment(mSegment4,true);
        Segment3D segment20(Point3D(15, 1, 0),
                            Point3D(15, 1, 1.111111111));
        Segment3D segment21(Point3D(13.5, 4, 0), 
                            Point3D(13.5, 4, 1.111111111));
        MSegment mSegment5(segment20,segment21,1,0,2,true,false);
        result1.addMSegment(mSegment5,true);
        assert_("UnitsTest 2.1", "ResultUnits don't equal.",
                result0 == result1); 
        // cout << result0;
        // cout << result1;
        ResultUnit result2 = units[1];
        ResultUnit result3;         
        Segment3D segment22(Point3D(2, 1, 1.111111111), 
                            Point3D(2, 1, 2.666666667));
        Segment3D segment23(Point3D(5, 1, 1.111111111), 
                            Point3D(5, 1, 2.666666667));
        MSegment mSegment6(segment22,segment23,0,0,0,true,true);
        result3.addMSegment(mSegment6,true);
        Segment3D segment24(Point3D(3.5, 4, 1.111111111), 
                            Point3D(3.5, 4, 2.666666667));
        Segment3D segment25(Point3D(2, 1, 1.111111111), 
                            Point3D(2, 1, 2.666666667));
        MSegment mSegment7(segment24,segment25,0,0,1,true,false);
        result3.addMSegment(mSegment7,true);
        Segment3D segment26(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(3.5, 4, 2.666666667));
        Segment3D segment27(Point3D(3.5, 4, 1.111111111), 
                            Point3D (3.5, 4, 2.666666667));
        MSegment mSegment8(segment26,segment27,0,0,2,true,false);
        result3.addMSegment(mSegment8,true);
        Segment3D segment28(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(2.8, 2.6, 2.666666667));
        Segment3D segment29(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(3.5, 4, 2.666666667));
        MSegment mSegment9(segment28,segment29,0,0,3,true,true);
        result3.addMSegment(mSegment9,true);
        Segment3D segment30(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(4.2, 2.6, 2.666666667));
        Segment3D segment31(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(2.8, 2.6, 2.666666667));
        MSegment mSegment10(segment30,segment31,0,0,4,true,false);
        result3.addMSegment(mSegment10,true);
        Segment3D segment32(Point3D(5, 1, 1.111111111), 
                            Point3D(5, 1, 2.666666667));
        Segment3D segment33(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D (4.2, 2.6, 2.666666667));
        MSegment mSegment11(segment32,segment33,0,0,5,true,false);
        result3.addMSegment(mSegment11,true);
        Segment3D segment34(Point3D(12, 1, 1.111111111), 
                            Point3D(12, 1, 2.66666667));
        Segment3D segment35(Point3D(15, 1, 1.111111111), 
                            Point3D(15, 1, 2.666666667));
        MSegment mSegment12(segment34,segment35,1,0,0,true,true);
        result3.addMSegment(mSegment12,true);
        Segment3D segment36(Point3D(13.5, 4, 1.111111111), 
                            Point3D(13.5, 4, 2.666666667));
        Segment3D segment37(Point3D(12, 1, 1.111111111), 
                            Point3D(12, 1, 2.666666667));
        MSegment mSegment13(segment36,segment37,1,0,1,true,false);
        result3.addMSegment(mSegment13,true);
        Segment3D segment38(Point3D(15, 1, 1.111111111), 
                            Point3D(15, 1, 2.666666667));
        Segment3D segment39(Point3D(13.5, 4, 1.111111111), 
                            Point3D(13.5, 4, 2.666666667));
        MSegment mSegment14(segment38,segment39,1,0,2,true,false);
        result3.addMSegment(mSegment14,true);
        assert_("UnitsTest 2.2", "ResultUnits don't equal.",
                result2 == result3); 
        // cout << result2;
        // cout << result3;
        ResultUnit result4 = units[2];
        ResultUnit result5; 
        Segment3D segment40(Point3D(2, 1, 2.666666667), 
                            Point3D(2, 1, 4.444444444));
        Segment3D segment41(Point3D(5, 1, 2.666666667), 
                            Point3D(5, 1, 4.444444444));        
        MSegment mSegment15(segment40,segment41,0,0,0,true,true);
        result5.addMSegment(mSegment15,true);        
        Segment3D segment42(Point3D(2.8, 2.6, 2.666666667), 
                            Point3D(3.333333333, 3.666666667, 4.444444444));
        Segment3D segment43(Point3D(2, 1, 2.666666667), 
                            Point3D(2, 1, 4.444444444));
        MSegment mSegment16(segment42,segment43,0,0,1,true,false);
        result5.addMSegment(mSegment16,true); 
        Segment3D segment44(Point3D(4.2, 2.6, 2.666666667), 
                            Point3D(3.666666667, 3.666666667, 4.444444444));
        Segment3D segment45(Point3D(2.8, 2.6, 2.666666667), 
                            Point3D(3.333333333, 3.666666667, 4.444444444));
        MSegment mSegment17(segment44,segment45,0,0,2,true,false);
        result5.addMSegment(mSegment17,true); 
        Segment3D segment46(Point3D(5, 1, 2.666666667), 
                            Point3D(5, 1, 4.444444444));
        Segment3D segment47(Point3D(4.2, 2.6, 2.666666667), 
                            Point3D(3.666666667, 3.666666667, 4.444444444));
        MSegment mSegment18(segment46,segment47,0,0,3,true,false);
        result5.addMSegment(mSegment18,true); 
        Segment3D segment48(Point3D(12, 1, 2.666666667), 
                            Point3D(12, 1, 4.444444444));
        Segment3D segment49(Point3D(15, 1, 2.666666667), 
                            Point3D(15, 1, 4.444444444));
        MSegment mSegment19(segment48,segment49,1,0,0,true,true);
        result5.addMSegment(mSegment19,true);
        Segment3D segment50(Point3D(13.5, 4, 2.666666667), 
                            Point3D(13.5, 4, 4.444444444));
        Segment3D segment51(Point3D(12, 1, 2.666666667), 
                            Point3D(12, 1, 4.444444444));
        MSegment mSegment20(segment50,segment51,1,0,1,true,false);
        result5.addMSegment(mSegment20,true);
        Segment3D segment52(Point3D(15, 1, 2.666666667), 
                            Point3D(15, 1, 4.444444444));
        Segment3D segment53(Point3D(13.5, 4, 2.666666667), 
                            Point3D(13.5, 4, 4.444444444));
        MSegment mSegment21(segment52,segment53,1,0,2,true,false);
        result5.addMSegment(mSegment21,true);
        assert_("UnitsTest 2.3", "ResultUnits don't equal.",
                result4 == result5); 
        // cout << result4;
        // cout << result5;        
        ResultUnit result6 = units[3];
        ResultUnit result7; 
        Segment3D segment54(Point3D(2, 1, 4.444444444), 
                            Point3D(2, 1, 4.666666667));
        Segment3D segment55(Point3D(5, 1, 4.444444444), 
                            Point3D(5, 1, 4.666666667));
        MSegment mSegment22(segment54,segment55,0,0,0,true,true);
        result7.addMSegment(mSegment22,true);
        Segment3D segment56(Point3D(3.333333333, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment57(Point3D(2, 1, 4.444444444),
                            Point3D(2, 1, 4.666666667));
        MSegment mSegment23(segment56,segment57,0,0,1,true,false);
        result7.addMSegment(mSegment23,true);
        Segment3D segment58(Point3D(3.666666667, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment59(Point3D(3.333333333, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        MSegment mSegment24(segment58,segment59,0,0,2,true,false);
        result7.addMSegment(mSegment24,true);
        Segment3D segment60(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment61(Point3D(3.666666667, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        MSegment mSegment25(segment60,segment61,0,0,3,true,true);
        result7.addMSegment(mSegment25,true);
        Segment3D segment62(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.5, 4, 4.666666667));
        Segment3D segment63(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        MSegment mSegment26(segment62,segment63,0,0,4,true,false);
        result7.addMSegment(mSegment26,true);
        Segment3D segment64(Point3D(5, 1, 4.444444444), 
                            Point3D(5, 1, 4.666666667));
        Segment3D segment65(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.5, 4, 4.666666667));
        MSegment mSegment27(segment64,segment65,0,0,5,true,false);
        result7.addMSegment(mSegment27,true);
        Segment3D segment66(Point3D(12, 1, 4.444444444), 
                            Point3D(12, 1, 4.66666667));
        Segment3D segment67(Point3D(15, 1, 4.444444444), 
                            Point3D(15, 1, 4.666666667));
        MSegment mSegment28(segment66,segment67,1,0,0,true,true);
        result7.addMSegment(mSegment28,true);
        Segment3D segment68(Point3D(13.5, 4, 4.444444444), 
                            Point3D(13.5, 4, 4.666666667));
        Segment3D segment69(Point3D(12, 1, 4.444444444), 
                            Point3D(12, 1, 4.666666667));
        MSegment mSegment29(segment68,segment69,1,0,1,true,false);
        result7.addMSegment(mSegment29,true);
        Segment3D segment70(Point3D(15, 1, 4.444444444), 
                            Point3D(15, 1, 4.666666667));
        Segment3D segment71(Point3D(13.5, 4, 4.444444444), 
                            Point3D(13.5, 4, 4.666666667));
        MSegment mSegment30(segment70,segment71,1,0,2,true,false);
        result7.addMSegment(mSegment30,true);
        assert_("UnitsTest 2.4", "ResultUnits don't equal.",
                result6 == result7); 
        // cout << result6;
        // cout << result7;
        ResultUnit result8 = units[4];
        ResultUnit result9; 
        Segment3D segment72(Point3D(2, 1, 4.666666667), 
                            Point3D(2, 1, 5));
        Segment3D segment73(Point3D(5, 1, 4.666666667), 
                            Point3D(5, 1, 5));
        MSegment mSegment31(segment72,segment73,0,0,0,true,true);
        result9.addMSegment(mSegment31,true);
        Segment3D segment74(Point3D(3.5, 4, 4.66666667), 
                            Point3D(3.5, 4, 5));
        Segment3D segment75(Point3D(2, 1, 4.666666667), 
                            Point3D(2, 1, 5));
        MSegment mSegment32(segment74,segment75,0,0,1,true,false);
        result9.addMSegment(mSegment32,true);
        Segment3D segment76(Point3D(5, 1, 4.666666667), 
                            Point3D(5, 1, 5));
        Segment3D segment77(Point3D(3.5, 4, 4.666666667), 
                            Point3D(3.5, 4, 5));
        MSegment mSegment33(segment76,segment77,0,0,2,true,false);
        result9.addMSegment(mSegment33,true);
        Segment3D segment78(Point3D(12, 1, 4.666666667), 
                            Point3D(12, 1, 5));
        Segment3D segment79(Point3D(15, 1, 4.666666667), 
                            Point3D(15, 1, 5));
        MSegment mSegment34(segment78,segment79,1,0,0,true,true);
        result9.addMSegment(mSegment34,true);
        Segment3D segment80(Point3D(13.5, 4, 4.666666667), 
                            Point3D(13.5, 4, 5));
        Segment3D segment81(Point3D(12, 1, 4.666666667), 
                            Point3D(12, 1, 5));
        MSegment mSegment35(segment80,segment81,1,0,1,true,false);
        result9.addMSegment(mSegment35,true);
        Segment3D segment82(Point3D(15, 1, 4.666666667), 
                            Point3D(15, 1, 5));
        Segment3D segment83(Point3D(13.5, 4, 4.666666667), 
                            Point3D(13.5, 4, 5));
        MSegment mSegment36(segment82,segment83,1,0,2,true,false);
        result9.addMSegment(mSegment36,true);
        assert_("UnitsTest 2.5", "ResultUnits don't equal.",
                result8 == result9); 
        // cout << result8;
        // cout << result9;      
      }// ResultUnitTest2   
     
      void Selftest::UnitsTest3(){
        Point3DContainer points;
        GlobalTimeValues timeValues(5);
        // points for unit 0 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3.5,4,0));
        points.add(Point3D(2,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3.5,4,5));
        // points for unit 1
        points.add(Point3D(6,1,0));// 6
        points.add(Point3D(9,1,0));         
        points.add(Point3D(7.5,4,0));
        points.add(Point3D(0,4,5));
        points.add(Point3D(3,4,5));         
        points.add(Point3D(1.5,7,5));
        // segments for pfaces 0, 1, 2
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // segments for pfaces 3, 4, 5
        Segment segment3(6,9);
        Segment segment4(7,10); 
        Segment segment5(8,11);   
        // Build unit 0
        SourceUnit unit0;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Build unit 1
        SourceUnit unit1;        
        unit1.addPFace(segment3,segment4,points);
        unit1.addPFace(segment4,segment5,points);
        unit1.addPFace(segment5,segment3,points);
        // Intersection
        unit0.intersection(unit1,timeValues);
        // Finalize
        unit0.finalize(points,timeValues,INNER,unit1);
        unit1.finalize(points,timeValues,INNER,unit0);
        // get result Units
        vector<ResultUnit> units = vector<ResultUnit>(timeValues.size()-1,
                                                      ResultUnit());
        for(size_t i = 0; i < timeValues.size()-1; i++){  
           unit0.getResultUnit(i,INNER,false,points,units[i],UNIT_A);
           unit1.getResultUnit(i,INNER,false,points,units[i],UNIT_B);
        }// for
        vector<ResultUnit> result = vector<ResultUnit>(5,ResultUnit());        
        Segment3D segment10(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(4.2, 2.6, 2.666666667));            
        Segment3D segment11(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(3.5, 4, 2.666666667));          
        Segment3D segment12(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(2.8, 2.6, 2.666666667));       
        Segment3D segment13(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(4.2, 2.6, 2.666666667));
        Segment3D segment14(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(3.5, 4, 2.666666667));
        Segment3D segment15(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(2.8, 2.6, 2.666666667));
        MSegment mSegment0(segment10,segment11);
        MSegment mSegment1(segment12,segment13);
        MSegment mSegment2(segment14,segment15);       
        result[1].addMSegment(mSegment0,false);
        result[1].addMSegment(mSegment1,false);
        result[1].addMSegment(mSegment2,false);   
        Segment3D segment16(Point3D(4.2, 2.6, 2.666666667), 
                            Point3D(3.666666667, 3.666666667, 4.444444444));  
        Segment3D segment17(Point3D(3.5, 4, 2.666666667), 
                            Point3D(3.5, 4, 4.444444444));  
        Segment3D segment18(Point3D(3.5, 4, 2.666666667), 
                            Point3D(3.5, 4, 4.444444444)); 
        Segment3D segment19(Point3D(2.8, 2.6, 2.666666667), 
                            Point3D(3.333333333, 3.666666667, 4.444444444));  
        Segment3D segment20(Point3D(2.8, 2.6, 2.666666667), 
                            Point3D(3.333333333, 3.666666667, 4.444444444));   
        Segment3D segment21(Point3D(4.2, 2.6, 2.666666667), 
                            Point3D(3.666666667, 3.666666667, 4.444444444));
        MSegment mSegment3(segment16,segment17);
        MSegment mSegment4(segment18,segment19);
        MSegment mSegment5(segment20,segment21);  
        result[2].addMSegment(mSegment3,false);
        result[2].addMSegment(mSegment4,false);
        result[2].addMSegment(mSegment5,false);       
        Segment3D segment22(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment23(Point3D(3.333333333, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));   
        Segment3D segment24(Point3D(3.333333333, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment25(Point3D(3.666666667, 3.666666667, 4.44444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment26(Point3D(3.666666667, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667)); 
        Segment3D segment27(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        MSegment mSegment6(segment22,segment23);
        MSegment mSegment7(segment24,segment25);
        MSegment mSegment8(segment26,segment27);  
        result[3].addMSegment(mSegment6,false);
        result[3].addMSegment(mSegment7,false);
        result[3].addMSegment(mSegment8,false);        
        assert_("UnitsTest 3.1", 
                " size of units vectors are different.",
                 units.size() == result.size());
        for(size_t i = 0; i < result.size(); i++){ 
        assert_("UnitTest 3.2", 
                " Unit are different.",
                 units[i] == result[i]);
         // cout << result[i];
         // cout << units[i];
        }// for
        // cout << unit0 << endl;
        // cout << unit1 << endl;       
      }// UnitTest3
      
      void Selftest::UnitsTest4(){
        Point3DContainer points;
        GlobalTimeValues timeValues(5);
        // points for unit 0 
        size_t i0 = points.add(Point3D(6,1,0));// 0
        size_t i1 = points.add(Point3D(9,1,0));         
        size_t i2 = points.add(Point3D(7.5,4,0));
        size_t i3 = points.add(Point3D(0,4,5));
        size_t i4 = points.add(Point3D(3,4,5));         
        size_t i5 = points.add(Point3D(1.5,7,5));
        // points for unit 2 
        size_t i6 = points.add(Point3D(6,1,0));// 6
        size_t i7 = points.add(Point3D(8,1,0));        
        size_t i8 = points.add(Point3D(7,3.5,0));
        size_t i9 = points.add(Point3D(0,4,5));
        size_t i10= points.add(Point3D(2,4,5));         
        size_t i11= points.add(Point3D(1,6.5,5));
        // segments for pfaces 0, 1, 2
        Segment segment0(i0,i3);
        Segment segment1(i1,i4);
        Segment segment2(i2,i5);
        // segments for pfaces 3, 4, 5
        Segment segment3(i6,i9);
        Segment segment4(i7,i10); 
        Segment segment5(i8,i11);   
        // Build unit 0
        SourceUnit unit0;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Build unit 1
        SourceUnit unit1;        
        unit1.addPFace(segment3,segment4,points);
        unit1.addPFace(segment4,segment5,points);
        unit1.addPFace(segment5,segment3,points);
        // Intersection
        unit0.intersection(unit1,timeValues);
        // Finalize
        // cout << unit0;
        // cout << unit1;        
        unit0.finalize(points,timeValues,OUTER,unit1);
        // cout << unit0;
        unit1.finalize(points,timeValues,INNER,unit0);
        // cout << points;
        // cout << unit1;
        // get result Units
        vector<ResultUnit> units = vector<ResultUnit>(timeValues.size()-1,
                                                       ResultUnit());
        for(size_t i = 0; i < timeValues.size()-1; i++){  
          unit0.getResultUnit(i,OUTER,false,points, units[i],UNIT_A);
          unit1.getResultUnit(i,INNER,true,points, units[i],UNIT_B);
          // cout << units[i];
          units[i].evaluateCriticalMSegmens(MINUS);
          units[i].finalize();  
        }// for
        ResultUnit result;
        Segment3D segment10(Point3D(7.1111111111, 3.2222222222, 0), 
                            Point3D(1.1111111111, 6.2222222222, 5));
        Segment3D segment11(Point3D(8, 1, 0), Point3D(2, 4, 5));           
        Segment3D segment12(Point3D(7.5, 4, 0), Point3D(1.5, 7, 5));        
        Segment3D segment13(Point3D(7.1111111111, 3.2222222222, 0), 
                            Point3D(1.1111111111, 6.2222222222, 5));         
        Segment3D segment14(Point3D(9, 1, 0), Point3D(3, 4, 5));        
        Segment3D segment15(Point3D(7.5, 4, 0), Point3D(1.5, 7, 5));
        Segment3D segment16(Point3D(8, 1, 0), Point3D(2, 4, 5));
        Segment3D segment17(Point3D(9, 1, 0), Point3D(3, 4, 5));
        MSegment mSegment0(segment10,segment11,0,0,0,true,true);
        MSegment mSegment1(segment12,segment13,0,0,1,true,false);
        MSegment mSegment2(segment14,segment15,0,0,2,true,false); 
        MSegment mSegment3(segment16,segment17,0,0,3,true,true);
        result.addMSegment(mSegment0,true);
        result.addMSegment(mSegment1,true);
        result.addMSegment(mSegment2,true);
        result.addMSegment(mSegment3,true);
        assert_("UnitsTest 4", "ResultUnits don't equal.",
                result == units[0]); 
        // cout << unit0;
        // cout << unit1;
        // cout << units[0];
        // cout << result; 
      }// UnitsTest4
      
      void Selftest::UnitsTest5(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(5);
        // points for pface 0, 1, 2 
        points.add(Point3D(1,1,0));
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3,5,0));
        points.add(Point3D(1,1,5));
        points.add(Point3D(1,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3,5,5));
        // points for pface 3, 4, 5
        points.add(Point3D(3,1,0));
        points.add(Point3D(4,3,0));         
        points.add(Point3D(2,3,0));
        points.add(Point3D(3,1,5));
        points.add(Point3D(4,3,5));         
        points.add(Point3D(2,3,5));
        // segments for pfaces 0, 1, 2
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // segments for pfaces 3, 4, 5
        Segment segment3(6,9);
        Segment segment4(7,10); 
        Segment segment5(8,11);   
        // Build unit 0
        SourceUnit unit0;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Build unit 1
        SourceUnit unit1;        
        unit1.addPFace(segment3,segment4,points);
        unit1.addPFace(segment4,segment5,points);
        unit1.addPFace(segment5,segment3,points);
        // Intersection
        unit0.intersection(unit1,timeValues);        
        // Finalize
        // cout << unit0;
        // cout << unit1; 
        unit0.finalize(points,timeValues,OUTER,unit1);
        // cout << unit0;
        unit1.finalize(points,timeValues,OUTER,unit0);                
        // cout << unit1;  
        // cout << timeValues;
        vector<ResultUnit> units = vector<ResultUnit>(
          timeValues.size()-1, ResultUnit());
        for(size_t i = 0; i < timeValues.size()-1; i++){  
          unit0.getResultUnit(i,OUTER,false,points,units[i],UNIT_A);
          unit1.getResultUnit(i,OUTER,false,points,units[i],UNIT_B);
          units[i].evaluateCriticalMSegmens(UNION);
          units[i].finalize();  
          // cout << units[i];
        }// for          
        ResultUnit result0 = units[0];
        ResultUnit result1; 
        MSegment mSegment0(Segment3D(Point3D(1, 1, 0), Point3D(1, 1, 5)),
                           Segment3D(Point3D(3, 1, 0), Point3D(3, 1, 5)), 
                           0,0,0,true, true);
        result1.addMSegment(mSegment0,true);
        MSegment mSegment1(Segment3D(Point3D(2, 3, 0), Point3D(2, 3, 5)),
                           Segment3D(Point3D(1, 1, 0), Point3D(1, 1, 5)),
                           0,0,1,true, false);
        result1.addMSegment(mSegment1,true);
        MSegment mSegment2(Segment3D(Point3D(3, 5, 0), Point3D(3, 5, 5)),
                           Segment3D(Point3D(2, 3, 0), Point3D(2, 3, 5)),
                           0,0,2,true, false);
        result1.addMSegment(mSegment2,true);
        MSegment mSegment3(Segment3D(Point3D(4, 3, 0), Point3D(4, 3, 5)),
                           Segment3D(Point3D(3, 5, 0), Point3D(3, 5, 5)),
                           0,0,3,true, false);
        result1.addMSegment(mSegment3,true);
        MSegment mSegment4(Segment3D(Point3D(5, 1, 0), Point3D(5, 1, 5)),
                           Segment3D(Point3D(4, 3, 0), Point3D(4, 3, 5)),
                           0,0,4,true, false);
        result1.addMSegment(mSegment4,true);
        MSegment mSegment5(Segment3D(Point3D(3, 1, 0), Point3D(3, 1, 5)),
                           Segment3D(Point3D(5, 1, 0), Point3D(5, 1, 5)),
                           0,0,5,true,true);
        result1.addMSegment(mSegment5,true);
        assert_("UnitsTest 5", "ResultUnits don't be equal.",
                result0 == result1); 
        // cout << result0;
        // cout << result1;      
      }// UnitsTest5
      
      void Selftest::UnitsTest6(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(5);
        // points for pface 0, 1, 2 
        points.add(Point3D(1,1,0));
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3,5,0));
        points.add(Point3D(1,1,5));
        points.add(Point3D(1,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3,5,5));
        // points for pface 3, 4, 5
        points.add(Point3D(3,1,0));
        points.add(Point3D(4,3,0));         
        points.add(Point3D(2,3,0));
        points.add(Point3D(3,1,5));
        points.add(Point3D(4,3,5));         
        points.add(Point3D(2,3,5));
        // segments for pfaces 0, 1, 2
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // segments for pfaces 3, 4, 5
        Segment segment3(6,9);
        Segment segment4(7,10); 
        Segment segment5(8,11);   
        // Build unit 0
        SourceUnit unit0;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Build unit 1
        SourceUnit unit1;        
        unit1.addPFace(segment0,segment1,points);
        unit1.addPFace(segment1,segment2,points);
        unit1.addPFace(segment2,segment0,points);
        // Intersection
        unit0.intersection(unit1, timeValues);        
        // Finalize
        unit0.finalize(points,timeValues,OUTER,unit1);
        unit1.finalize(points,timeValues,OUTER,unit0);
        // cout << unit0;
        // cout << unit1;  
        // cout << timeValues;
        vector<ResultUnit> units = vector<ResultUnit>(
          timeValues.size()-1, ResultUnit());
        for(size_t i = 0; i < timeValues.size()-1; i++){  
          unit0.getResultUnit(i,OUTER,false,points,units[i],UNIT_A);
          unit1.getResultUnit(i,OUTER,false,points,units[i],UNIT_B);
          units[i].evaluateCriticalMSegmens(UNION);
          units[i].finalize();  
          // cout << units[i];
        }// for  
        ResultUnit result0 = units[0];
        ResultUnit result1; 
        MSegment mSegment0(Segment3D(Point3D (1, 1, 0), Point3D(1, 1, 5)),
                           Segment3D(Point3D (5, 1, 0), Point3D(5, 1, 5)),
                           0,0,0,true, true);
        result1.addMSegment(mSegment0,true);
        MSegment mSegment1(Segment3D(Point3D (3, 5, 0), Point3D(3, 5, 5)),
                           Segment3D(Point3D (1, 1, 0), Point3D(1, 1, 5)),
                           0,0,1,true, false);
        result1.addMSegment(mSegment1,true);                           
        MSegment mSegment2(Segment3D(Point3D (5, 1, 0), Point3D(5, 1, 5)),
                           Segment3D(Point3D (3, 5, 0), Point3D(3, 5, 5)),
                           0,0,2,true, false);
        result1.addMSegment(mSegment2,true);                     
        assert_("UnitsTest 6", "ResultUnits don't be equal.",
                result0 == result1); 
        // cout << result0;
        // cout << result1; 
      }// UnitsTest6
      
      void Selftest::UnitsTest7(){
        Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(5);
        // points for pface 0, 1, 2 
        points.add(Point3D(1,1,0));
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3,5,0));
        points.add(Point3D(1,1,5));
        points.add(Point3D(1,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3,5,5));
        // points for pface 3, 4, 5
        points.add(Point3D(3,1,0));
        points.add(Point3D(4,3,0));         
        points.add(Point3D(2,3,0));
        points.add(Point3D(3,1,5));
        points.add(Point3D(4,3,5));         
        points.add(Point3D(2,3,5));
        // segments for pfaces 0, 1, 2
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // segments for pfaces 3, 4, 5
        Segment segment3(6,9);
        Segment segment4(7,10); 
        Segment segment5(8,11);   
        // Build unit 0
        SourceUnit unit0;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Build unit 1
        SourceUnit unit1;        
        unit1.addPFace(segment5,segment4,points);
        unit1.addPFace(segment4,segment2,points);
        unit1.addPFace(segment2,segment5,points);
        // Intersection
        unit0.intersection(unit1,timeValues);  
        // cout << unit0;
        // cout << unit1;  
        // cout << timeValues;
        // Finalize
        unit0.finalize(points,timeValues,OUTER,unit1);
        unit1.finalize(points,timeValues,OUTER,unit0);
        // cout << unit0;
        // cout << unit1;  
        // cout << timeValues;
        vector<ResultUnit> units = vector<ResultUnit>(
          timeValues.size()-1, ResultUnit());
        for(size_t i = 0; i < timeValues.size()-1; i++){  
          unit0.getResultUnit(i,OUTER,false,points,units[i],UNIT_A);
          unit1.getResultUnit(i,OUTER,false,points,units[i],UNIT_B);
          units[i].evaluateCriticalMSegmens(UNION);
          units[i].finalize();  
          // cout << units[i];
        }// for  
        ResultUnit result0 = units[0];
        ResultUnit result1; 
        MSegment mSegment0(Segment3D(Point3D(1, 1, 0), Point3D(1, 1, 5)),
                           Segment3D(Point3D(5, 1, 0), Point3D(5, 1, 5)),
                           0,0,0,true, true);
        result1.addMSegment(mSegment0,true);
        MSegment mSegment1(Segment3D(Point3D(2, 3, 0), Point3D(2, 3, 5)),
                           Segment3D(Point3D(1, 1, 0), Point3D(1, 1, 5)),
                           0,0,1,true, false);
        result1.addMSegment(mSegment1,true);                    
        MSegment mSegment2(Segment3D(Point3D(3, 5, 0), Point3D(3, 5, 5)),
                           Segment3D(Point3D(2, 3, 0), Point3D(2, 3, 5)),
                           0,0,2,true, false);
        result1.addMSegment(mSegment2,true);
        MSegment mSegment3(Segment3D(Point3D(4, 3, 0), Point3D(4, 3, 5)),
                           Segment3D(Point3D(3, 5, 0), Point3D(3, 5, 5)),
                           0,0,3,true, false);
        result1.addMSegment(mSegment3,true);          
        MSegment mSegment4(Segment3D(Point3D(5, 1, 0), Point3D(5, 1, 5)),
                           Segment3D(Point3D(4, 3, 0), Point3D(4, 3, 5)),
                           0,0,4,true, false);
        result1.addMSegment(mSegment4,true);
        assert_("UnitsTest 7", "ResultUnits don't be equal.",
                result0 == result1); 
        // cout << result0;
        // cout << result1;        
      }// UnitsTest7
      
       void Selftest::UnitsTest8() {
         Point3DContainer points;
        SegmentContainer segments;
        GlobalTimeValues timeValues(5);
        // points for pface 0, 1, 2 
        points.add(Point3D(1,1,0));
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3,5,0));        
        points.add(Point3D(1,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3,5,5));
        // points for pface 3, 4, 5        
        points.add(Point3D(3,3,0));
        points.add(Point3D(3,3,5)); 
        // segments for pfaces 0, 1, 2
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // segments for pfaces 3, 4, 5
        Segment segment3(6,7);  
        // Build unit 0
        SourceUnit unit0;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Build unit 1
        SourceUnit unit1;        
        unit1.addPFace(segment0,segment1,points);
        unit1.addPFace(segment1,segment3,points);
        unit1.addPFace(segment3,segment0,points);
        // Intersection
        unit0.reSort();
        unit1.reSort(); 
        unit0.intersection(unit1,timeValues);         
        // cout << unit0;
        // cout << unit1;
        // unit0.printFaceCycleEntrys();
        // unit1.printFaceCycleEntrys(); 
        // Finalize
        unit0.finalize(points,timeValues,OUTER,unit1);
        unit1.finalize(points,timeValues,INNER,unit0);  
        // cout << unit0;
        // cout << unit1; 
        vector<ResultUnit> units = vector<ResultUnit>(
        timeValues.size()-1, ResultUnit());
        for(size_t i = 0; i < timeValues.size()-1; i++){  
          unit0.getResultUnit(i,OUTER,false,points,units[i],UNIT_A);
          unit1.getResultUnit(i,INNER,true,points,units[i],UNIT_B);
          units[i].evaluateCriticalMSegmens(MINUS);
          units[i].finalize();  
          // cout << units[i];
        }// for  
        ResultUnit result0 = units[0];
        ResultUnit result1; 
        MSegment mSegment0(Segment3D(Point3D(1, 1, 0), Point3D(1, 1, 5)),
                           Segment3D(Point3D(3, 3, 0), Point3D(3, 3, 5)),
                           0,0,0,true, true);
        result1.addMSegment(mSegment0,true);
        MSegment mSegment1(Segment3D(Point3D(3, 5, 0), Point3D(3, 5, 5)),
                           Segment3D(Point3D(1, 1, 0), Point3D(1, 1, 5)),
                           0,0,1,true, false);
        result1.addMSegment(mSegment1,true);                    
        MSegment mSegment2(Segment3D(Point3D(5, 1, 0), Point3D(5, 1, 5)),
                           Segment3D(Point3D(3, 5, 0), Point3D(3, 5, 5)),
                           0,0,2,true, false);
        result1.addMSegment(mSegment2,true);
        MSegment mSegment3(Segment3D(Point3D(3, 3, 0), Point3D(3, 3, 5)),
                           Segment3D(Point3D(5, 1, 0), Point3D(5, 1, 5)),
                           0,0,3,true, true);
        result1.addMSegment(mSegment3,true);          
        assert_("UnitsTest 8", "ResultUnits don't be equal.",
                result0 == result1);
      }// UnitsTest8
      
/*
 Test UnitPair
 
 */
      void Selftest::SourceUnitPairTest1(){
        SourceUnitPair unitPair(0,5,5); 
        Segment3D segment0(Point3D(2,1,0), Point3D(2,1,5)); 
        Segment3D segment1(Point3D(5,1,0), Point3D(5,1,5));
        Segment3D segment2(Point3D(3.5,4,0), Point3D(3.5,4,5));
        Segment3D segment3(Point3D(6,1,0), Point3D(0,4,5));
        Segment3D segment4(Point3D(9,1,0),  Point3D(3,4,5));
        Segment3D segment5(Point3D(7.5,4,0), Point3D(1.5,7,5));
        unitPair.addPFace(UNIT_A,segment0,segment1);
        unitPair.addPFace(UNIT_A,segment1,segment2);
        unitPair.addPFace(UNIT_A,segment2,segment0);
        unitPair.addPFace(UNIT_B,segment3,segment4);
        unitPair.addPFace(UNIT_B,segment4,segment5);
        unitPair.addPFace(UNIT_B,segment5,segment3);
        unitPair.operate(INTERSECTION);
        // cout << unitPair;
        // result
        ResultUnit result1 =  unitPair.getResultUnit(0);
        ResultUnit result2(0,1.111111111);
        assert_("SourceUnitPairTest 1.1", "ResultUnits don't equal.",
                result1 == result2); 
        // cout << result1;
        // cout << result2;
        ResultUnit result3 =  unitPair.getResultUnit(1);
        ResultUnit result4(1.111111111,2.666666667);
        
        Segment3D segment10(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(4.2, 2.6, 2.666666667));            
        Segment3D segment11(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(3.5, 4, 2.666666667));          
        Segment3D segment12(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(2.8, 2.6, 2.666666667));       
        Segment3D segment13(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(4.2, 2.6, 2.666666667));
        Segment3D segment14(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(3.5, 4, 2.666666667));
        Segment3D segment15(Point3D(4.666666667, 1.666666667, 1.111111111), 
                            Point3D(2.8, 2.6, 2.666666667));
        MSegment mSegment2(segment10,segment11,0,0,2,true,false);
        MSegment mSegment0(segment12,segment13,0,0,0,true,true);
        MSegment mSegment1(segment14,segment15,0,0,1,true,false);       
        result4.addMSegment(mSegment0,true);
        result4.addMSegment(mSegment1,true);
        result4.addMSegment(mSegment2,true);
        assert_("SourceUnitPairTest 1.2", "ResultUnits don't equal.",
                result3 == result4); 
        // cout << result3;
        // cout << result4;
        ResultUnit result5 =  unitPair.getResultUnit(2);
        ResultUnit result6(2.666666667,4.444444444);
        Segment3D segment16(Point3D(4.2, 2.6, 2.666666667), 
                            Point3D(3.666666667, 3.666666667, 4.444444444));  
        Segment3D segment17(Point3D(3.5, 4, 2.666666667), 
                            Point3D(3.5, 4, 4.444444444));  
        Segment3D segment18(Point3D(3.5, 4, 2.666666667), 
                            Point3D(3.5, 4, 4.444444444)); 
        Segment3D segment19(Point3D(2.8, 2.6, 2.666666667), 
                            Point3D(3.333333333, 3.666666667, 4.444444444));  
        Segment3D segment20(Point3D(2.8, 2.6, 2.666666667), 
                            Point3D(3.333333333, 3.666666667, 4.444444444));   
        Segment3D segment21(Point3D(4.2, 2.6, 2.666666667), 
                            Point3D(3.666666667, 3.666666667, 4.444444444));
        MSegment mSegment5(segment16,segment17,0,0,0,true,false);
        MSegment mSegment4(segment18,segment19,0,0,1,true,false);
        MSegment mSegment3(segment20,segment21,0,0,2,true,true);  
        result6.addMSegment(mSegment3,true);
        result6.addMSegment(mSegment4,true);
        result6.addMSegment(mSegment5,true); 
        assert_("SourceUnitPairTest 1.3", "ResultUnits don't equal.",
                result5 == result6); 
        // cout << result5;
        // cout << result6;
        ResultUnit result7 =  unitPair.getResultUnit(3);
        ResultUnit result8(4.444444444,4.666666667);
        Segment3D segment22(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment23(Point3D(3.333333333, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));   
        Segment3D segment24(Point3D(3.333333333, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment25(Point3D(3.666666667, 3.666666667, 4.44444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        Segment3D segment26(Point3D(3.666666667, 3.666666667, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667)); 
        Segment3D segment27(Point3D(3.5, 4, 4.444444444), 
                            Point3D(3.4, 3.8, 4.666666667));
        MSegment mSegment6(segment22,segment23,0,0,0,true,false);
        MSegment mSegment7(segment24,segment25,0,0,1,true,true);
        MSegment mSegment8(segment26,segment27,0,0,2,true,false);  
        result8.addMSegment(mSegment7,true);
        result8.addMSegment(mSegment6,true);
        result8.addMSegment(mSegment8,true);
        assert_("SourceUnitPairTest 1.4", "ResultUnits don't equal.",
                result7 == result8);  
        // cout << result7;
        // cout << result8;
        ResultUnit result9 =  unitPair.getResultUnit(4);
        ResultUnit result10(4.666666667,5);
        assert_("SourceUnitPairTest 1.5", "ResultUnits don't equal.",
                result9 == result10); 
        // cout << result9;
        // cout << result10;
      }// SourceUnitPairTest1
/*      
      void Selftest::ResultUnitFactoryTest6() {
        ResultUnitFactory factory1(10),factory2(2);
        std::vector<bool> predicate1(9),predicate2(9); 
        std::vector<bool> predicate3(1),predicate4(1);   
        // Outside
        factory1.addNonOrthogonalEdges(0,Segment(0, 1, LEFT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(0,Segment(2, 3, RIGHT_IS_INNER));  
        // Outside
        factory1.addNonOrthogonalEdges(1,Segment(4, 5, LEFT_IS_INNER));
        factory1.addNonOrthogonalEdges(1,Segment(6, 7));
        // Inside
        factory1.addNonOrthogonalEdges(2,Segment(8, 9, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(2,Segment(10, 11));
        // Inside
        factory1.addNonOrthogonalEdges(3,Segment(12, 13));
        factory1.addNonOrthogonalEdges(3,Segment(14, 15, LEFT_IS_INNER));
        // Outside
        factory1.addNonOrthogonalEdges(4,Segment(16, 17, INTERSECT));
        factory1.addNonOrthogonalEdges(4,Segment(18, 19, RIGHT_IS_INNER));
        // three segments
        factory1.addNonOrthogonalEdges(5,Segment(20, 21, LEFT_IS_INNER));
        factory1.addNonOrthogonalEdges(5,Segment(20, 23, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(5,Segment(22, 23));
        // three segments
        factory1.addNonOrthogonalEdges(6,Segment(24, 25, RIGHT_IS_INNER));
        factory1.addNonOrthogonalEdges(6,Segment(24, 27, INTERSECT));
        factory1.addNonOrthogonalEdges(6,Segment(26, 27));
        // Inside
        factory1.addNonOrthogonalEdges(7,Segment(28, 29, INTERSECT));
        factory1.addNonOrthogonalEdges(7,Segment(30, 31, INNER));
        // Outside
        factory1.addNonOrthogonalEdges(8,Segment(32, 33));
        factory1.addNonOrthogonalEdges(8,Segment(33, 34, OUTER));
        // Undefined
        factory2.addNonOrthogonalEdges(0,Segment(0, 1)); 
        factory2.addNonOrthogonalEdges(0,Segment(2, 3)); 
        // cout << factory1;
        std::vector<bool> predicate5 = {false, false, true, true, false, 
                                        true, true, true, false};
        bool result = factory1.intersects(predicate1);
        assert_("ResultUnitFactoryTest 6.1", 
                "An intersection result is available.",
                (result)); 
        result = true;      
        for(size_t i = 0; i < predicate1.size(); i++){
          if(predicate1[i] != predicate5[i])  result = false;
        //  if(predicate1[i]){
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", true" <<endl;
        //  }// if
        //  else {
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", false" <<endl;
        //  }// else          
        }// for
        assert_("ResultUnitFactoryTest 6.2", 
                "An intersection result isn't equal.",
                (result));  
        result = factory2.intersects(predicate3);
        assert_("ResultUnitFactoryTest 6.3", 
                "An intersection result isn't available.",
                (!result)); 
        // Tests for inside
        std::vector<bool> predicate6 = {false, false, true, true, false, 
                                        false, false, true, false};
        result = factory1.inside(predicate2);
        assert_("ResultUnitFactoryTest 6.4", 
                "An intersection result is available.",
                (result)); 
        result = true;      
        for(size_t i = 0; i < predicate2.size(); i++){
          if(predicate2[i] != predicate6[i])  result = false;
        //  if(predicate2[i]){
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", true" <<endl;
        //  }// if
        //  else {
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", false" <<endl;
        //   }// else          
        }// for
        assert_("ResultUnitFactoryTest 6.5", 
                "An intersection result isn't equal.",
                (result));  
        result = factory2.inside(predicate4);
        assert_("ResultUnitFactoryTest 6.6", 
                "An intersection result isn't available.",
                (!result));  
      }// ResultUnitFactoryTest6
*/
/*
      void Selftest::ResultUnitFactoryTest7() {
        ResultUnitFactory factory1(10,true),factory2(2,true);
        std::vector<bool> predicate1(9),predicate2(9); 
        std::vector<bool> predicate3(1),predicate4(1);   
        // Inside
        factory1.addNonOrthogonalEdges(0,Segment(0, 1, RIGHT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(0,Segment(2, 3, INTERSECT));
        // Outside
        factory1.addNonOrthogonalEdges(1,Segment(4, 5, LEFT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(1,Segment(6, 7, INTERSECT));
        // Inside
        factory1.addNonOrthogonalEdges(2,Segment(8, 9, INTERSECT));        
        factory1.addNonOrthogonalEdges(2,Segment(10, 11, LEFT_IS_INNER)); 
        // Outside
        factory1.addNonOrthogonalEdges(3,Segment(12, 13, INTERSECT));
        factory1.addNonOrthogonalEdges(3,Segment(14, 15, RIGHT_IS_INNER));
        // Outside/Inside
        factory1.addNonOrthogonalEdges(4,Segment(16, 17)); 
        factory1.addNonOrthogonalEdges(4,Segment(18, 19, RIGHT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(4,Segment(20, 21, INTERSECT));
        // Inside/Outside
        factory1.addNonOrthogonalEdges(5,Segment(22, 23, INTERSECT));
        factory1.addNonOrthogonalEdges(5,Segment(24, 25, LEFT_IS_INNER));
        factory1.addNonOrthogonalEdges(5,Segment(26, 27));
        // Outside/Inside
        factory1.addNonOrthogonalEdges(6,Segment(28, 29)); 
        factory1.addNonOrthogonalEdges(6,Segment(30, 31, NO_INTERSECT)); 
        factory1.addNonOrthogonalEdges(6,Segment(32, 33, RIGHT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(6,Segment(34, 35, NO_INTERSECT)); 
        factory1.addNonOrthogonalEdges(6,Segment(36, 37, INTERSECT));
        // Inside/Outside
        factory1.addNonOrthogonalEdges(7,Segment(38, 39, INTERSECT)); 
        factory1.addNonOrthogonalEdges(7,Segment(40, 41, NO_INTERSECT)); 
        factory1.addNonOrthogonalEdges(7,Segment(42, 43, LEFT_IS_INNER)); 
        factory1.addNonOrthogonalEdges(7,Segment(44, 45, NO_INTERSECT)); 
        factory1.addNonOrthogonalEdges(7,Segment(46, 47));
        // Outside
        factory1.addNonOrthogonalEdges(8,Segment(48, 49, INTERSECT)); 
        factory1.addNonOrthogonalEdges(8,Segment(50, 51, NO_INTERSECT)); 
        factory1.addNonOrthogonalEdges(8,Segment(52, 53, NO_INTERSECT)); 
        factory1.addNonOrthogonalEdges(8,Segment(54, 55, RIGHT_IS_INNER));
        // Undefined
        factory2.addNonOrthogonalEdges(0,Segment(0, 1, INTERSECT)); 
        factory2.addNonOrthogonalEdges(0,Segment(2, 3, INTERSECT)); 
        // cout << factory1;        
        std::vector<bool> predicate5 = {true, false, true, false, true, 
                                        true, true, true, false};
        bool result = factory1.intersects(predicate1);
        assert_("ResultUnitFactoryTest 7.1", 
                "An intersection result is available.",
                (result));
        result = true;      
        for(size_t i = 0; i < predicate1.size(); i++){
          if(predicate1[i] != predicate5[i])  result = false;
        //   if(predicate1[i]){
        //      cout << "Predikat intersects for slide " << i;
        //      cout << ", true" <<endl;
        //   }// if
        //   else {
        //      cout << "Predikat intersects for slide " << i;
        //      cout << ", false" <<endl;
        //   }// else          
        }// for
        assert_("ResultUnitFactoryTest 7.2", 
                "An intersection result isn't equal.",
                (result));
        std::vector<bool> predicate6 = {true, false, true, false, false, 
                                        false, false, false, false};
        result = factory2.intersects(predicate3);        
        assert_("ResultUnitFactoryTest 7.3", 
                "An intersection result isn't available.",
                (!result)); 
        // Test for inside
        result = factory1.inside(predicate2);
        assert_("ResultUnitFactoryTest 7.4", 
                "An inside result is available.",
                (result));
        result = true;      
        for(size_t i = 0; i < predicate2.size(); i++){
          if(predicate2[i] != predicate6[i])  result = false;
        //  if(predicate2[i]){
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", true" <<endl;
        //  }// if
        //  else {
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", false" <<endl;
        //  }// else          
        }// for
        assert_("ResultUnitFactoryTest 7.5", 
                "An intersection result isn't equal.",
                (result));
        result = factory2.inside(predicate4);
        assert_("ResultUnitFactoryTest 7.6", 
                "An inside result isn't available.",
                (!result));   
      }// ResultUnitFactoryTest7
*/      

      void Selftest::UnitsTest9(){
        Point3DContainer points;
        GlobalTimeValues timeValues(5);
        std::vector<bool> predicates1,predicates2;
        // points for unit 0 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3.5,4,0));
        points.add(Point3D(2,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3.5,4,5));
        // points for unit 1
        points.add(Point3D(6,1,0));// 6
        points.add(Point3D(9,1,0));         
        points.add(Point3D(7.5,4,0));
        points.add(Point3D(0,4,5));
        points.add(Point3D(3,4,5));         
        points.add(Point3D(1.5,7,5));
        // segments for pfaces 0, 1, 2
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // segments for pfaces 3, 4, 5
        Segment segment3(6,9);
        Segment segment4(7,10); 
        Segment segment5(8,11);   
        // Build unit 0
        SourceUnit unit0;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Build unit 1
        SourceUnit unit1;        
        unit1.addPFace(segment3,segment4,points);
        unit1.addPFace(segment4,segment5,points);
        unit1.addPFace(segment5,segment3,points);
        // Intersection
        unit0.intersection(unit1,timeValues);
        unit0.reSort();
        unit1.reSort(); 
        // Prädikat intersects bestimmen
        unit0.intersects(points,timeValues,unit1,predicates1);
        // cout << unit0;
        std::vector<bool> predicates3 = {false, true, true, true, false};
        bool result = true;      
        for(size_t i = 0; i < predicates1.size(); i++){
          if(predicates1[i] != predicates3[i])  result = false; 
          // if(predicates1[i]){
          //   cout << "Predikat intersects for slide " << i;
          //   cout << ", true" <<endl;
          // }// if
          // else {
          //   cout << "Predikat intersects for slide " << i;
          //   cout << ", false" <<endl;
          // }// else          
        }// for          
        assert_("UnitsTest 9.1", 
                "An intersection result isn't equal.",
                (result)); 
        unit0.inside(points,timeValues,unit1,predicates2);
        std::vector<bool> predicates4 = {false, false, false, false, false};
        result = true;      
        for(size_t i = 0; i < predicates2.size(); i++){
          if(predicates2[i] != predicates4[i])  result = false; 
          // if(predicates2[i]){
          //   cout << "Predikat inside for slide " << i;
          //  cout << ", true" <<endl;
          // }// if
          // else {
          //   cout << "Predikat inside for slide " << i;
          //   cout << ", false" <<endl;
          // }// else          
        }// for  
        assert_("UnitsTest 9.2", 
                "An intersection result isn't equal.",
                (result));
      }// UnitsTest9
      
      void Selftest::UnitsTest10(){
        Point3DContainer points;
        GlobalTimeValues timeValues(5);
        std::vector<bool> predicates1,predicates2;
                // points for unit 1
        points.add(Point3D(6,1,0));// 6
        points.add(Point3D(7,1,0));         
        points.add(Point3D(6.5,2,0));
        points.add(Point3D(0,1,5));        
        points.add(Point3D(1,1,5));
        points.add(Point3D(0.5,2,5)); 
        // points for unit 0 
        points.add(Point3D(2,1,0));// 0
        points.add(Point3D(5,1,0));         
        points.add(Point3D(3.5,4,0));
        points.add(Point3D(2,1,5));
        points.add(Point3D(5,1,5));         
        points.add(Point3D(3.5,4,5));

        // segments for pfaces 0, 1, 2
        Segment segment0(0,3);
        Segment segment1(1,4);
        Segment segment2(2,5);
        // segments for pfaces 3, 4, 5
        Segment segment3(6,9);
        Segment segment4(7,10); 
        Segment segment5(8,11);   
        // Build unit 0
        SourceUnit unit0;        
        unit0.addPFace(segment0,segment1,points);
        unit0.addPFace(segment1,segment2,points);
        unit0.addPFace(segment2,segment0,points);
        // Build unit 1
        SourceUnit unit1;        
        unit1.addPFace(segment3,segment4,points);
        unit1.addPFace(segment4,segment5,points);
        unit1.addPFace(segment5,segment3,points);
        // Intersection      
        unit0.intersection(unit1,timeValues);
        // cout << unit0 << endl;
        // cout << unit1 << endl;        
        // Prädikat intersects bestimmen
        unit0.intersects(points,timeValues,unit1,predicates1);
        // cout << unit0;       
        std::vector<bool> predicates3 = {false, true, true, true, false};      
        bool result = true;      
        for(size_t i = 0; i < predicates1.size(); i++){
          if(predicates1[i] != predicates3[i])  result = false; 
          // if(predicates1[i]){
          //   cout << "Predikat intersects for slide " << i;
          //   cout << ", true" <<endl;
          // }// if
          // else {
          //   cout << "Predikat intersects for slide " << i;
          //   cout << ", false" <<endl;
          // }// else          
        }// for        
        assert_("UnitsTest 10.1", 
                "An intersection result isn't equal.",
                (result));               
        unit0.inside(points,timeValues,unit1,predicates2);
        std::vector<bool> predicates4 = {false, false, true, false, false};
        result = true;      
        for(size_t i = 0; i < predicates2.size(); i++){
          if(predicates2[i] != predicates4[i])  result = false; 
          // if(predicates2[i]){
          //   cout << "Predikat inside for slide " << i;
          //   cout << ", true" <<endl;
          // }// if
          // else {
          //   cout << "Predikat inside for slide " << i;
          //   cout << ", false" <<endl;
          // }// else          
        }// for  
        assert_("UnitsTest 10.2", 
                "An intersection result isn't equal.",
                (result));
      }// UnitsTest10
      

     void Selftest::LayerTest1(){
       Layer layer1, layer2, layer3, layer4, layer5, layer6, layer7;       
       layer1.addNonOrthSegment(Segment(0, 1));
       layer1.addNonOrthSegment(Segment(2, 3, LEFT_IS_INNER));
       layer1.addNonOrthSegment(Segment(4, 5, RIGHT_IS_INNER));
       layer1.addOrthSegment(Segment(0, 6, LEFT_IS_INNER));
       layer1.addOrthSegment(Segment(1, 7, RIGHT_IS_INNER));
       // Result layer
       layer4.addNonOrthSegment(Segment(0, 1, INNER));
       layer4.addNonOrthSegment(Segment(2, 3, LEFT_IS_INNER));
       layer4.addNonOrthSegment(Segment(4, 5, RIGHT_IS_INNER));
       layer4.addOrthSegment(Segment(0, 6, LEFT_IS_INNER));
       layer4.addOrthSegment(Segment(1, 7, RIGHT_IS_INNER));
       bool result = layer1.evaluate();
       assert_("LayerTest 1.1", 
               "Evaluation of the layer is not complete.",
               result);       
       assert_("LayerTest 1.2", 
               "The layers are different.",
               layer1 == layer4);       
       // cout << layer1;
       // cout << layer4;       
       layer2.addNonOrthSegment(Segment(0, 1));
       layer2.addNonOrthSegment(Segment(2, 3));
       layer2.addNonOrthSegment(Segment(4, 5, RIGHT_IS_INNER));
       layer2.addNonOrthSegment(Segment(6, 7));
       layer2.addNonOrthSegment(Segment(8, 9));
       // Result layer
       layer5.addNonOrthSegment(Segment(0, 1, OUTER));
       layer5.addNonOrthSegment(Segment(2, 3, OUTER));
       layer5.addNonOrthSegment(Segment(4, 5, RIGHT_IS_INNER));
       layer5.addNonOrthSegment(Segment(6, 7, INNER));
       layer5.addNonOrthSegment(Segment(8, 9, INNER));
       result = layer2.evaluate();
       assert_("LayerTest 1.3", 
               "Evaluation of the layer is not complete.",
               result);   
       assert_("LayerTest 1.4", 
               "The layers are different.",
               layer2 == layer5);
       // cout << layer2;
       // cout << layer5;       
       layer3.addNonOrthSegment(Segment(0, 1));
       layer3.addOrthSegment(Segment(0, 2, LEFT_IS_INNER));
       layer3.addOrthSegment(Segment(1, 3, RIGHT_IS_INNER));
       layer3.addNonOrthSegment(Segment(2, 3));
       // Result layer
       layer6.addNonOrthSegment(Segment(0, 1, INNER));
       layer6.addOrthSegment(Segment(0, 2, LEFT_IS_INNER));
       layer6.addOrthSegment(Segment(1, 3, RIGHT_IS_INNER));
       layer6.addNonOrthSegment(Segment(2, 3, INNER));
       result = layer3.evaluate();
       assert_("LayerTest 1.5", 
               "Evaluation of the layer is not complete.",
               result);   
       assert_("LayerTest 1.6", 
               "The layers are different.",
               layer3 == layer6);
       // cout << layer3;
       // cout << layer6;    
       layer7.addNonOrthSegment(Segment(0, 1));
       layer7.addNonOrthSegment(Segment(2, 3));
       result = layer7.evaluate();
       assert_("LayerTest 1.7",
               "Evaluation of the layer is not complete.",
               (!result)); 
     }// LayerTest1
     
     void Selftest::LayerTest2(){
       Layer layer1;      
       layer1.addNonOrthSegment(Segment(0, 1));
       layer1.addNonOrthSegment(Segment(2, 3, LEFT_IS_INNER));
       layer1.addNonOrthSegment(Segment(4, 5, RIGHT_IS_INNER));
       layer1.addOrthSegment(Segment(0, 6, LEFT_IS_INNER));
       layer1.addOrthSegment(Segment(1, 7, RIGHT_IS_INNER));
       Layer layer2 = layer1;
       Layer layer3(layer1);
       assert_("LayerTest 2", 
               "The layers are different.",
               layer2 == layer2);    
     }// LayerTest2
     
     void Selftest::LayerTest3(){
       Layer layer1, layer2, layer3;      
       layer1.addNonOrthSegment(Segment(0, 1));
       layer1.addNonOrthSegment(Segment(0, 2));
       layer1.addOrthSegment(Segment(0, 3, LEFT_IS_INNER));       
       layer1.addNonOrthSegment(Segment(3, 4));
       bool result = layer1.evaluate();
       Predicate predecessor = layer1.getPredicateForPredecessor();
       Predicate successor   = layer1.getPredicateForSuccessor();
       assert_("LayerTest 3.1", 
               "Evaluation of the layer is not complete.",
               result);  
       assert_("LayerTest 3.2", 
               "Predecessor predicate is wrong.",
               predecessor == OUTER);
       assert_("LayerTest 3.3", 
               "Successor predicate is wrong.",
               successor == INNER);
       layer2.addNonOrthSegment(Segment(5, 6));
       layer2.addNonOrthSegment(Segment(7, 8));
       layer2.addNonOrthSegment(Segment(9, 10));
       layer2.setPredicateFromSuccessor(successor);
       layer2.evaluate();   
       // Result layer
       layer3.addNonOrthSegment(Segment(5, 6, INNER));
       layer3.addNonOrthSegment(Segment(7, 8, INNER));
       layer3.addNonOrthSegment(Segment(9, 10, INNER));
       assert_("LayerTest 3.4", 
               "The layers are different.",
               layer2 == layer2);        
       // cout << layer1;
       // cout << "Predeccesor predicate:=" << toString(predecessor) << endl;
       // cout << "Successor predicate:=" << toString(successor) << endl;
       // cout << layer2;    
     }// LayerTest2

     void Selftest::LayerContainerTest1(){
        LayerContainer layerContainer1(4), layerContainer2;
        assert_("LayerContainerTest 1.1", 
                "The layer containers are different.",
                (!(layerContainer1 == layerContainer2)));        
        layerContainer1.addNonOrthSegment(0,Segment(0,1));
        layerContainer1.addNonOrthSegment(0,Segment(2,3));
        layerContainer1.addNonOrthSegment(1,Segment(4,5,LEFT_IS_INNER));
        layerContainer1.addNonOrthSegment(2,Segment(5,6));
        layerContainer1.addOrthSegment(0,Segment(7,8,LEFT_IS_INNER));
        layerContainer1.addOrthSegment(3,Segment(9,10,LEFT_IS_INNER));
        layerContainer2 = layerContainer1;
        LayerContainer layerContainer3(layerContainer1);
        assert_("LayerContainerTest 1.2", 
                "The layer containers are equal.",
                (layerContainer1 == layerContainer2));
        assert_("LayerContainerTest 1.3", 
                "The layer containers are equal.",
                (layerContainer1 == layerContainer3));
        // cout << layerContainer1 << endl;
        // cout << layerContainer2 << endl
        // cout << layerContainer2 << endl
     }// LayerContainerTest1
     
     void Selftest::LayerContainerTest2(){
        LayerContainer layerContainer1(4), layerContainer2(4);
        layerContainer1.addNonOrthSegment(0,Segment(0,1));
        layerContainer1.addNonOrthSegment(0,Segment(2,3));        
        layerContainer1.addNonOrthSegment(1,Segment(1,4,LEFT_IS_INNER));
        layerContainer1.addOrthSegment(1,Segment(1,3,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(1,Segment(3,5));        
        layerContainer1.addNonOrthSegment(2,Segment(4,6,LEFT_IS_INNER));
        layerContainer1.addNonOrthSegment(2,Segment(5,7)); 
        layerContainer1.addNonOrthSegment(3,Segment(6,8));
        layerContainer1.addNonOrthSegment(3,Segment(7,9));
        // Result layer conatiner
        layerContainer2.addNonOrthSegment(0,Segment(0,1,INNER));
        layerContainer2.addNonOrthSegment(0,Segment(2,3,INNER));        
        layerContainer2.addNonOrthSegment(1,Segment(1,4,LEFT_IS_INNER));
        layerContainer2.addOrthSegment(1,Segment(1,3,RIGHT_IS_INNER));
        layerContainer2.addNonOrthSegment(1,Segment(3,5,OUTER));        
        layerContainer2.addNonOrthSegment(2,Segment(4,6,LEFT_IS_INNER));
        layerContainer2.addNonOrthSegment(2,Segment(5,7,OUTER));
        layerContainer2.addNonOrthSegment(3,Segment(6,8,OUTER));
        layerContainer2.addNonOrthSegment(3,Segment(7,9,OUTER));        
        // cout << layerContainer1;        
        bool result = layerContainer1.evaluate();
        assert_("LayerContainerTest 2.1", 
                "Evaluation of the layer conatainer is not complete.",
                result); 
        assert_("LayerContainerTest 2.2", 
                "The layer containers are equal.",
               layerContainer1 == layerContainer2);
        Predicate left, right;
        layerContainer1.getBorderPredicates(left, right);
        assert_("LayerContainerTest 2.3", 
                "The predicates at the border are not correct..",
               (left == INTERSECT && right == INTERSECT));
        // cout << layerContainer1;
        // cout << layerContainer2;
        // cout << "Left border:=" << toString(left) << endl;
        // cout << "Right border:=" << toString(right) << endl;
     }// LayerContainerTest2
     
     void Selftest::LayerContainerTest3(){
        LayerContainer layerContainer1(3), layerContainer2(3);
        layerContainer1.addNonOrthSegment(0,Segment(0,3)); 
        layerContainer1.addNonOrthSegment(0,Segment(1,4));
        layerContainer1.addNonOrthSegment(0,Segment(2,5));
        layerContainer1.addNonOrthSegment(1,Segment(3,6));
        layerContainer1.addNonOrthSegment(1,Segment(4,7));
        layerContainer1.addNonOrthSegment(1,Segment(5,8, RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(1,Segment(5,9));
        layerContainer1.addNonOrthSegment(2,Segment(6,10));
        layerContainer1.addNonOrthSegment(2,Segment(6,11));
        layerContainer1.addNonOrthSegment(2,Segment(7,11));
        layerContainer1.addOrthSegment(2,Segment(8,9,RIGHT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(2,Segment(9,12));
        bool result = layerContainer1.evaluate();
        // result
        layerContainer2.addNonOrthSegment(0,Segment(0,3,OUTER)); 
        layerContainer2.addNonOrthSegment(0,Segment(1,4,OUTER));
        layerContainer2.addNonOrthSegment(0,Segment(2,5,OUTER));
        layerContainer2.addNonOrthSegment(1,Segment(3,6,OUTER));
        layerContainer2.addNonOrthSegment(1,Segment(4,7,OUTER));
        layerContainer2.addNonOrthSegment(1,Segment(5,8,RIGHT_IS_INNER));
        layerContainer2.addNonOrthSegment(1,Segment(5,9,INNER));
        layerContainer2.addNonOrthSegment(2,Segment(6,10,OUTER));
        layerContainer2.addNonOrthSegment(2,Segment(6,11,OUTER));
        layerContainer2.addNonOrthSegment(2,Segment(7,11,OUTER));
        layerContainer2.addOrthSegment(2,Segment(8, 9,RIGHT_IS_INNER)); 
        layerContainer2.addNonOrthSegment(2,Segment(9,12,OUTER));
        Predicate left, right;
        layerContainer1.getBorderPredicates(left,right); 
        assert_("LayerContainerTest 3.1", 
                "Evaluation of the layer conatainer is not complete.",
                result); 
        assert_("LayerContainerTest 3.2", 
                "The layer containers are equal.",
               layerContainer1 == layerContainer2);
        assert_("LayerContainerTest 3.3", 
                "The predicates at the border are not correct..",
               (left == OUTER && right == INTERSECT));
        // cout << layerContainer1 << endl;
        // cout << layerContainer2 << endl; 
        // cout << "Predicate on left border:=" << toString(left) << endl;
        // cout << "Predicate on right border:=" << toString(right) << endl;
      }// LayerContainerTest3  
      
      void Selftest::LayerContainerTest4(){
        LayerContainer layerContainer1(5), layerContainer2(5);
        layerContainer1.addNonOrthSegment(0,Segment(14,2)); 
        layerContainer1.addNonOrthSegment(0,Segment(0,2,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(0,Segment(1,3,LEFT_IS_INNER));
        layerContainer1.addNonOrthSegment(0,Segment(15,16));
        layerContainer1.addOrthSegment(0,Segment(0,1,LEFT_IS_INNER));        
        layerContainer1.addNonOrthSegment(1,Segment(2,17));
        layerContainer1.addNonOrthSegment(1,Segment(2,4,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(1,Segment(3,6,LEFT_IS_INNER));       
        layerContainer1.addNonOrthSegment(1,Segment(16,6));        
        layerContainer1.addNonOrthSegment(2,Segment(17,18));
        layerContainer1.addNonOrthSegment(2,Segment(4,7,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(2,Segment(5,8,LEFT_IS_INNER));
        layerContainer1.addNonOrthSegment(2,Segment(5,9,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(2,Segment(6,9));
        layerContainer1.addNonOrthSegment(3,Segment(18,10));
        layerContainer1.addNonOrthSegment(3,Segment(7,10,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(3,Segment(8,11,LEFT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(3,Segment(9,19));
        layerContainer1.addNonOrthSegment(4,Segment(10,12)); 
        layerContainer1.addNonOrthSegment(4,Segment(11,13, LEFT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(4,Segment(19,20));
        bool result = layerContainer1.evaluate();
        // result
        layerContainer2.addNonOrthSegment(0,Segment(14,2,OUTER)); 
        layerContainer2.addNonOrthSegment(0,Segment(0,2,RIGHT_IS_INNER));
        layerContainer2.addNonOrthSegment(0,Segment(1,3,LEFT_IS_INNER));
        layerContainer2.addNonOrthSegment(0,Segment(15,16,OUTER));
        layerContainer2.addOrthSegment(0,Segment(0,1,LEFT_IS_INNER));
        layerContainer2.addNonOrthSegment(1,Segment(2,17,OUTER));
        layerContainer2.addNonOrthSegment(1,Segment(2,4,RIGHT_IS_INNER));
        layerContainer2.addNonOrthSegment(1,Segment(3,6,LEFT_IS_INNER));       
        layerContainer2.addNonOrthSegment(1,Segment(16,6,OUTER));
        layerContainer2.addNonOrthSegment(2,Segment(17,18,OUTER));
        layerContainer2.addNonOrthSegment(2,Segment(4,7,RIGHT_IS_INNER));
        layerContainer2.addNonOrthSegment(2,Segment(5,8,LEFT_IS_INNER));
        layerContainer2.addNonOrthSegment(2,Segment(5,9,RIGHT_IS_INNER));
        layerContainer2.addNonOrthSegment(2,Segment(6,9,INNER));
        layerContainer2.addNonOrthSegment(3,Segment(18,10,OUTER));
        layerContainer2.addNonOrthSegment(3,Segment(7,10,RIGHT_IS_INNER));
        layerContainer2.addNonOrthSegment(3,Segment(8,11,LEFT_IS_INNER)); 
        layerContainer2.addNonOrthSegment(3,Segment(9,19,OUTER));
        layerContainer2.addNonOrthSegment(4,Segment(10,12,INNER)); 
        layerContainer2.addNonOrthSegment(4,Segment(11,13,LEFT_IS_INNER)); 
        layerContainer2.addNonOrthSegment(4,Segment(19,20,OUTER));
        Predicate left, right;
        layerContainer1.getBorderPredicates(left,right);
        assert_("LayerContainerTest 4.1", 
                "Evaluation of the layer conatainer is not complete.",
                result); 
         assert_("LayerContainerTest 4.2", 
                "The layer containers are equal.",
               layerContainer1 == layerContainer2);
        assert_("LayerContainerTest 4.3", 
                "The predicates at the border are not correct..",
               (left == INTERSECT && right == INTERSECT));
        // cout << layerContainer1 << endl;
        // cout << layerContainer2 << endl;  
        // cout << "Predicate on left border:=" << toString(left) << endl;
        // cout << "Predicate on right border:=" << toString(right) << endl;
      }// LayerContainerTest4
      
      void Selftest::LayerContainerTest5(){
        LayerContainer layerContainer1(3), layerContainer2(3);
        layerContainer1.addNonOrthSegment(0, Segment (0, 9,OUTER)); 
        layerContainer1.addNonOrthSegment(0, Segment (3, 12));
        layerContainer1.addNonOrthSegment(1, Segment (4, 13));
        layerContainer1.addNonOrthSegment(1, Segment (5, 14,NO_INTERSECT));
        layerContainer1.addNonOrthSegment(1, Segment (6, 15));
        layerContainer1.addNonOrthSegment(2, Segment (7, 16));
        layerContainer1.addNonOrthSegment(2, Segment (8, 17));
        bool result = layerContainer1.evaluate();
        layerContainer2.addNonOrthSegment(0, Segment (0, 9,OUTER)); 
        layerContainer2.addNonOrthSegment(0, Segment (3, 12,OUTER));
        layerContainer2.addNonOrthSegment(1, Segment (4, 13,OUTER));
        layerContainer2.addNonOrthSegment(1, Segment (5, 14,OUTER));
        layerContainer2.addNonOrthSegment(1, Segment (6, 15,OUTER));
        layerContainer2.addNonOrthSegment(2, Segment (7, 16,OUTER));
        layerContainer2.addNonOrthSegment(2, Segment (8, 17,OUTER));
        Predicate left, right;
        layerContainer1.getBorderPredicates(left,right);
        assert_("LayerContainerTest 5.1", 
                "Evaluation of the layer conatainer is not complete.",
                result); 
        assert_("LayerContainerTest 5.2", 
                "The layer containers are equal.",
               layerContainer1 == layerContainer2);
        assert_("LayerContainerTest 5.3", 
                "The predicates at the border are not correct..",
               (left == OUTER && right == OUTER));
        // cout << layerContainer1 << endl;
        // cout << layerContainer2 << endl;  
        // cout << "Predicate on left border:=" << toString(left) << endl;
        // cout << "Predicate on right border:=" << toString(right) << endl;
      }// LayerContainerTest5
      
      void Selftest::LayerContainerTest6(){
        GlobalTimeValues timeValues(5);
        IntSegContainer container;
        Point3DContainer points1,points2;
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(2,1,0,1.78885438), 
          IntersectionPoint(2,1,5,1.78885438)));
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5,4,0,5.14295635), 
          IntersectionPoint(3.5,4,5,5.14295635)));
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8,2.6,2.66666667,3.57770876), 
          IntersectionPoint(3.4,3.8,4.66666667,4.91934955), 
          RIGHT_IS_INNER));
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(2.8,2.6,2.66666667,3.57770876), 
          IntersectionPoint(3.5,4,2.66666667,5.14295635), 
          LEFT_IS_INNER));
        container.addIntSeg(IntersectionSegment(
          IntersectionPoint(3.5,4,4.44444444, 5.14295635), 
          IntersectionPoint(3.4,3.8,4.66666667,4.91934955), 
          LEFT_IS_INNER));  
        timeValues.addTimeValue(0);
        timeValues.addTimeValue(1.11111111);
        timeValues.addTimeValue(2.66666667); 
        timeValues.addTimeValue(4.44444444);
        timeValues.addTimeValue(4.66666667);
        timeValues.addTimeValue(5); 
        // Determine the result
        LayerContainer layerContainer1(points1,timeValues,container,false);
        LayerContainer layerContainer2(5),layerContainer3(5);
        points2.add(Point3D(2,1,0));
        points2.add(Point3D(2,1,1.11111111));
        points2.add(Point3D(3.5,4,0));
        points2.add(Point3D(3.5,4,1.11111111));
        points2.add(Point3D(2,1,2.66666667));
        points2.add(Point3D(3.5,4,2.66666667));
        points2.add(Point3D(2,1,4.44444444));
        points2.add(Point3D(2.8,2.6,2.66666667));
        points2.add(Point3D(3.33333333, 3.66666666, 4.44444444));
        points2.add(Point3D(3.5,4, 4.44444444));
        points2.add(Point3D(2,1,4.66666667));
        points2.add(Point3D(3.4,3.8,4.66666667));
        points2.add(Point3D(3.5,4,4.66666667));
        points2.add(Point3D(2,1,5));
        points2.add(Point3D(3.5,4,5));
        assert_("LayerContainerTest 6.1","points are equal.",
                 points1 == points2);
        layerContainer2.addNonOrthSegment(0,Segment(0,1));
        layerContainer2.addNonOrthSegment(0,Segment(2,3));
        layerContainer2.addNonOrthSegment(1,Segment(1,4));
        layerContainer2.addNonOrthSegment(1,Segment(3,5));
        layerContainer2.addNonOrthSegment(2,Segment(4,6));
        layerContainer2.addNonOrthSegment(2,Segment(7,8,RIGHT_IS_INNER));
        layerContainer2.addOrthSegment(2,Segment(7,5,LEFT_IS_INNER));
        layerContainer2.addNonOrthSegment(2,Segment(5,9));
        layerContainer2.addNonOrthSegment(3,Segment(6,10));
        layerContainer2.addNonOrthSegment(3,Segment(8,11,RIGHT_IS_INNER));
        layerContainer2.addNonOrthSegment(3,Segment(9,11,LEFT_IS_INNER));
        layerContainer2.addNonOrthSegment(3,Segment(9,12));
        layerContainer2.addNonOrthSegment(4,Segment(10,13));
        layerContainer2.addNonOrthSegment(4,Segment(12,14));
        assert_("LayerContainerTest 6.2", 
                "The layer containers are equal.",
                 layerContainer1 == layerContainer2);  
        // cout << setprecision(9);
        // cout << container << endl;
        // cout << timeValues << endl;
        // cout << points1 << endl;
        // cout << points2 << endl;
        // cout << layerContainer1 << endl;
        // cout << layerContainer2 << endl;
        layerContainer3.addNonOrthSegment(0,Segment(0,1,OUTER));
        layerContainer3.addNonOrthSegment(0,Segment(2,3,OUTER));
        layerContainer3.addNonOrthSegment(1,Segment(1,4,OUTER));
        layerContainer3.addNonOrthSegment(1,Segment(3,5,OUTER));
        layerContainer3.addNonOrthSegment(2,Segment(4,6,OUTER));
        layerContainer3.addNonOrthSegment(2,Segment(7,8,RIGHT_IS_INNER));
        layerContainer3.addOrthSegment(2,Segment(7,5,LEFT_IS_INNER));
        layerContainer3.addNonOrthSegment(2,Segment(5,9,INNER));
        layerContainer3.addNonOrthSegment(3,Segment(6,10,OUTER));
        layerContainer3.addNonOrthSegment(3,Segment(8,11,RIGHT_IS_INNER));
        layerContainer3.addNonOrthSegment(3,Segment(9,11,LEFT_IS_INNER));
        layerContainer3.addNonOrthSegment(3,Segment(9,12,OUTER));
        layerContainer3.addNonOrthSegment(4,Segment(10,13,OUTER));
        layerContainer3.addNonOrthSegment(4,Segment(12,14,OUTER));
        bool result = layerContainer1.evaluate();
        Predicate left, right;
        layerContainer1.getBorderPredicates(left,right);
        assert_("LayerContainerTest 6.3", 
                "Evaluation of the layer conatainer is not complete.",
                result); 
        assert_("LayerContainerTest 6.4", 
                "The layer containers are equal.",
               layerContainer1 == layerContainer3);
        assert_("LayerContainerTest 6.5", 
                "The predicates at the border are not correct..",
               (left == OUTER && right == INTERSECT));
        // cout << layerContainer1 << endl;
        // cout << layerContainer3 << endl;  
        // cout << "Predicate on left border:=" << toString(left) << endl;
        // cout << "Predicate on right border:=" << toString(right) << endl;
      }// LayerContainerTest6
      
      void Selftest::LayerContainerTest7(){
        Point3DContainer points;
        SegmentContainer segments;
        Point3DContainer points1,points2,points3;
        GlobalTimeValues timeValues1(5);
        GlobalTimeValues timeValues2(5);
        // points for unit 1 
        points1.add(Point3D(2,1,0));// 0
        points1.add(Point3D(5,1,0));         
        points1.add(Point3D(3.5,4,0));
        points1.add(Point3D(2,1,5));
        points1.add(Point3D(5,1,5));         
        points1.add(Point3D(3.5,4,5));
        // points for unit 2 
        points1.add(Point3D(6,1,0));// 6
        points1.add(Point3D(9,1,0));         
        points1.add(Point3D(7.5,4,0));
        points1.add(Point3D(0,4,5));
        points1.add(Point3D(3,4,5));         
        points1.add(Point3D(1.5,7,5));
        // segments for pface 1
        segments.add(Segment(0,3));
        segments.add(Segment(1,4));
        segments.add(Segment(2,5));
        // segments for pface 2
        segments.add(Segment(6,9));
        segments.add(Segment(7,10)); 
        segments.add(Segment(8,11));        
        // add pfaces to unit 1 
        PFace pf0(0,1,points1,segments);
        PFace pf1(1,2,points1,segments);
        PFace pf2(2,0,points1,segments);
        // add pfaces to unit 2
        PFace pf3(3,4,points1,segments);
        PFace pf4(4,5,points1,segments);
        PFace pf5(5,3,points1,segments);        
        // intersection
        pf0.intersection(pf3,timeValues1);
        pf0.intersection(pf4,timeValues1);
        pf0.intersection(pf5,timeValues1);
        pf0.addBorder(timeValues1);
        pf1.intersection(pf3,timeValues1);
        pf1.intersection(pf4,timeValues1);
        pf1.intersection(pf5,timeValues1);
        pf1.addBorder(timeValues1);
        pf2.intersection(pf3,timeValues1);
        pf2.intersection(pf4,timeValues1);
        pf2.intersection(pf5,timeValues1);
        pf2.addBorder(timeValues1);
        pf3.addBorder(timeValues1);
        pf4.addBorder(timeValues1);
        pf5.addBorder(timeValues1);
        LayerContainer layerContainer0(points2,timeValues1,pf0,false);
        LayerContainer layerContainer1(points2,timeValues1,pf1,false);
        LayerContainer layerContainer2(points2,timeValues1,pf2,false);
        LayerContainer layerContainer3(5),layerContainer4(5),layerContainer5(5);
        // Results       
        points3.add(Point3D(2,1,0));
        points3.add(Point3D(2,1,1.11111111));
        points3.add(Point3D(5,1,0));
        points3.add(Point3D(5,1,1.11111111));
        points3.add(Point3D(2,1,2.66666667));
        points3.add(Point3D(5,1,2.66666667));
        points3.add(Point3D(2,1,4.44444444));
        points3.add(Point3D(5,1,4.44444444));
        points3.add(Point3D(2,1,4.66666667));
        points3.add(Point3D(5,1,4.66666667));
        points3.add(Point3D(2,1,5));
        points3.add(Point3D(5,1,5));
        points3.add(Point3D(3.5,4,0));
        points3.add(Point3D(3.5,4,1.11111111));
        points3.add(Point3D(4.66666667,1.66666667,1.11111111));
        points3.add(Point3D(4.2,2.6,2.66666667));
        points3.add(Point3D(3.5,4,2.66666667));
        points3.add(Point3D(3.66666667, 3.66666667,4.44444444));
        points3.add(Point3D(3.5,4,4.44444444));
        points3.add(Point3D(3.5,4,4.66666667));
        points3.add(Point3D(3.5,4,5));
        points3.add(Point3D(2.8,2.6,2.66666667));
        points3.add(Point3D(3.33333333,3.66666667,4.44444444));
        points3.add(Point3D(3.4,3.8,4.66666667));
        assert_("LayerContainerTest 7.1", "points are equal.",
                points2 == points3);
        // cout << setprecision(9);
        // cout << points2 << endl;
        // cout << points3 << endl;
        layerContainer3.addNonOrthSegment(0,Segment(0,1)); 
        layerContainer3.addNonOrthSegment(0,Segment(2,3));
        layerContainer3.addNonOrthSegment(1,Segment(1,4));
        layerContainer3.addNonOrthSegment(1,Segment(3,5));
        layerContainer3.addNonOrthSegment(2,Segment(4,6));
        layerContainer3.addNonOrthSegment(2,Segment(5,7));
        layerContainer3.addNonOrthSegment(3,Segment(6,8)); 
        layerContainer3.addNonOrthSegment(3,Segment(7,9));
        layerContainer3.addNonOrthSegment(4,Segment(8,10)); 
        layerContainer3.addNonOrthSegment(4,Segment(9,11));
        layerContainer4.addNonOrthSegment(0,Segment(2,3));
        layerContainer4.addNonOrthSegment(0,Segment(12,13));
        layerContainer4.addNonOrthSegment(1,Segment(3,5));
        layerContainer4.addNonOrthSegment(1,Segment(14,15,RIGHT_IS_INNER));
        layerContainer4.addNonOrthSegment(1,Segment(14,16,LEFT_IS_INNER));
        layerContainer4.addNonOrthSegment(1,Segment(13,16));
        layerContainer4.addNonOrthSegment(2,Segment(5,7));
        layerContainer4.addNonOrthSegment(2,Segment(15,17,RIGHT_IS_INNER));
        layerContainer4.addNonOrthSegment(2,Segment(16,18));
        layerContainer4.addNonOrthSegment(3,Segment(7,9));
        layerContainer4.addOrthSegment(3,Segment(17,18,RIGHT_IS_INNER));
        layerContainer4.addNonOrthSegment(3,Segment(18,19));
        layerContainer4.addNonOrthSegment(4,Segment(9,11));        
        layerContainer4.addNonOrthSegment(4,Segment(19,20));  
        layerContainer5.addNonOrthSegment(0,Segment(12,13));
        layerContainer5.addNonOrthSegment(0,Segment(0,1));
        layerContainer5.addNonOrthSegment(1,Segment(13,16));
        layerContainer5.addNonOrthSegment(1,Segment(1,4));
        layerContainer5.addNonOrthSegment(2,Segment(16,18));
        layerContainer5.addOrthSegment(2,Segment(16,21,LEFT_IS_INNER));
        layerContainer5.addNonOrthSegment(2,Segment(21,22,LEFT_IS_INNER));
        layerContainer5.addNonOrthSegment(2,Segment(4,6));
        layerContainer5.addNonOrthSegment(3,Segment(18,19));
        layerContainer5.addNonOrthSegment(3,Segment(18,23,RIGHT_IS_INNER));
        layerContainer5.addNonOrthSegment(3,Segment(22,23,LEFT_IS_INNER));
        layerContainer5.addNonOrthSegment(3,Segment(6,8));
        layerContainer5.addNonOrthSegment(4,Segment(19,20));
        layerContainer5.addNonOrthSegment(4,Segment(8,10));
        assert_("LayerContainerTest 7.2", 
                " The layer containers are equal",
                layerContainer0 == layerContainer3);
        // cout << layerContainer0 << endl;
        // cout << layerContainer3 << endl;
        assert_("LayerContainerTest 7.3", 
                " The layer containers are equal",
                layerContainer1 == layerContainer4);;
        // cout << layerContainer1 << endl;
        // cout << layerContainer4 << endl;
        assert_("LayerContainerTest 7.4", 
                " The layer containers are equal",
                layerContainer2 == layerContainer5);
        // cout << layerContainer2 << endl;
        // cout << layerContainer5 << endl;
      }// LayerContainerTest7
      
      void Selftest::LayerContainerTest8() {
        LayerContainer layerContainer1(9), layerContainer2(1);
        std::vector<bool> predicate1(9),predicate2(9); 
        std::vector<bool> predicate3(1),predicate4(1);   
        // Outside
        layerContainer1.addNonOrthSegment(0,Segment(0,1,LEFT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(0,Segment(2,3,RIGHT_IS_INNER));  
        // Outside
        layerContainer1.addNonOrthSegment(1,Segment(4,5,LEFT_IS_INNER));
        layerContainer1.addNonOrthSegment(1,Segment(6,7));
        // Inside
        layerContainer1.addNonOrthSegment(2,Segment(8,9,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(2,Segment(10,11));
        // Inside
        layerContainer1.addNonOrthSegment(3,Segment(12,13));
        layerContainer1.addNonOrthSegment(3,Segment(14,15,LEFT_IS_INNER));
        // Outside
        layerContainer1.addNonOrthSegment(4,Segment(16,17,INTERSECT));
        layerContainer1.addNonOrthSegment(4,Segment(18,19,RIGHT_IS_INNER));
        // three segments
        layerContainer1.addNonOrthSegment(5,Segment(20,21,LEFT_IS_INNER));
        layerContainer1.addNonOrthSegment(5,Segment(20,23,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(5,Segment(22,23));
        // three segments
        layerContainer1.addNonOrthSegment(6,Segment(24,25,RIGHT_IS_INNER));
        layerContainer1.addNonOrthSegment(6,Segment(24,27,INTERSECT));
        layerContainer1.addNonOrthSegment(6,Segment(26,27));
        // Inside
        layerContainer1.addNonOrthSegment(7,Segment(28,29,INTERSECT));
        layerContainer1.addNonOrthSegment(7,Segment(30,31,INNER));
        // Outside
        layerContainer1.addNonOrthSegment(8,Segment(32,33));
        layerContainer1.addNonOrthSegment(8,Segment(33,34,OUTER));
        // Undefined
        layerContainer2.addNonOrthSegment(0,Segment(0,1)); 
        layerContainer2.addNonOrthSegment(0,Segment(2,3)); 
        // cout << layerContainer1;
        std::vector<bool> predicate5 = {false, false, true, true, false, 
                                        true, true, true, false};
        bool result = layerContainer1.intersects(predicate1);
        assert_("LayerContainerTest 8.1", 
                "A intersection result are available.",
                (result)); 
        result = true;      
        for(size_t i = 0; i < predicate1.size(); i++){
          if(predicate1[i] != predicate5[i])  result = false;
        //  if(predicate1[i]){
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", true" <<endl;
        //  }// if
        //  else {
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", false" <<endl;
        //   }// else          
        }// for
        assert_("LayerContainerTest 8.2", 
                "A intersection result is not the same.",
                (result));  
        result = layerContainer2.intersects(predicate3);
        assert_("LayerContainerTest 8.3", 
                "A intersection result isn't available.",
                (!result)); 
        // Tests for inside
        std::vector<bool> predicate6 = {false, false, true, true, false, 
                                        false, false, true, false};
        result = layerContainer1.inside(predicate2);
        assert_("LayerContainerTest 8.4", 
                "A intersection result is available.",
                (result)); 
        result = true;      
        for(size_t i = 0; i < predicate2.size(); i++){
          if(predicate2[i] != predicate6[i])  result = false;
        //  if(predicate2[i]){
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", true" <<endl;
        //  }// if
        //  else {
        //    cout << "Predikat intersects for slide " << i;
        //    cout << ", false" <<endl;
        //   }// else          
        }// for
        assert_("LayerContainerTest 8.5", 
                "A intersection result is not the same.",
                (result));  
        result = layerContainer2.inside(predicate4);
        assert_("LayerContainerTest 8.6", 
                "An intersection result isn't available.",
                (!result));  
      }// LayerContainerTest8
       
      void Selftest::LayerContainerTest9() {
         LayerContainer layerContainer1(9,true),layerContainer2(9,true);
        std::vector<bool> predicate1(9),predicate2(9); 
        std::vector<bool> predicate3(1),predicate4(1);   
        // Inside
        layerContainer1.addNonOrthSegment(0,Segment(0,1,RIGHT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(0,Segment(2,3,INTERSECT));
        // Outside
        layerContainer1.addNonOrthSegment(1,Segment(4,5,LEFT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(1,Segment(6,7,INTERSECT));
        // Inside
        layerContainer1.addNonOrthSegment(2,Segment(8,9,INTERSECT));        
        layerContainer1.addNonOrthSegment(2,Segment(10,11,LEFT_IS_INNER)); 
        // Outside
        layerContainer1.addNonOrthSegment(3,Segment(12,13,INTERSECT));
        layerContainer1.addNonOrthSegment(3,Segment(14,15,RIGHT_IS_INNER));
        // Outside/Inside
        layerContainer1.addNonOrthSegment(4,Segment(16,17)); 
        layerContainer1.addNonOrthSegment(4,Segment(18,19,RIGHT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(4,Segment(20,21,INTERSECT));
        // Inside/Outside
        layerContainer1.addNonOrthSegment(5,Segment(22,23,INTERSECT));
        layerContainer1.addNonOrthSegment(5,Segment(24,25,LEFT_IS_INNER));
        layerContainer1.addNonOrthSegment(5,Segment(26,27));
        // Outside/Inside
        layerContainer1.addNonOrthSegment(6,Segment(28,29)); 
        layerContainer1.addNonOrthSegment(6,Segment(30,31,NO_INTERSECT)); 
        layerContainer1.addNonOrthSegment(6,Segment(32,33,RIGHT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(6,Segment(34,35,NO_INTERSECT)); 
        layerContainer1.addNonOrthSegment(6,Segment(36,37,INTERSECT));
        // Inside/Outside
        layerContainer1.addNonOrthSegment(7,Segment(38,39,INTERSECT)); 
        layerContainer1.addNonOrthSegment(7,Segment(40,41,NO_INTERSECT)); 
        layerContainer1.addNonOrthSegment(7,Segment(42,43,LEFT_IS_INNER)); 
        layerContainer1.addNonOrthSegment(7,Segment(44,45,NO_INTERSECT)); 
        layerContainer1.addNonOrthSegment(7,Segment(46,47));
        // Outside
        layerContainer1.addNonOrthSegment(8,Segment(48,49,INTERSECT)); 
        layerContainer1.addNonOrthSegment(8,Segment(50,51,NO_INTERSECT)); 
        layerContainer1.addNonOrthSegment(8,Segment(52,53,NO_INTERSECT)); 
        layerContainer1.addNonOrthSegment(8,Segment(54,55,RIGHT_IS_INNER));
        // Undefined
        layerContainer2.addNonOrthSegment(0,Segment(0,1,INTERSECT)); 
        layerContainer2.addNonOrthSegment(0,Segment(2,3,INTERSECT)); 
        // cout << factory1;        
        std::vector<bool> predicate5 = {true, false, true, false, true, 
                                        true, true, true, false};
        bool result = layerContainer1.intersects(predicate1);
        assert_("LayerContainerTest 9.1", 
                "A intersection result is available.",
                (result));
        result = true;      
        for(size_t i = 0; i < predicate1.size(); i++){
          if(predicate1[i] != predicate5[i])  result = false;
        //   if(predicate1[i]){
        //      cout << "Intersect predicate for layer " << i;
        //      cout << ", true" <<endl;
        //   }// if
        //   else {
        //      cout << "Intersect predicate for layer " << i;
        //      cout << ", false" <<endl;
        //   }// else          
        }// for
        assert_("LayerContainerTest 9.2", 
                "A intersection result isn't equal.",
                (result));
        std::vector<bool> predicate6 = {true, false, true, false, false, 
                                        false, false, false, false};
        result = layerContainer2.intersects(predicate3);        
        assert_("LayerContainerTest 9.3", 
                "A intersection result isn't available.",
                (!result)); 
        // Test for inside
        result = layerContainer1.inside(predicate2);
        assert_("LayerContainerTest 9.4", 
                "A inside result is available.",
                (result));
        result = true;      
        for(size_t i = 0; i < predicate2.size(); i++){
          if(predicate2[i] != predicate6[i])  result = false;
        //  if(predicate2[i]){
        //    cout << "Inside predicate for layer " << i;
        //    cout << ", true" <<endl;
        //  }// if
        //  else {
        //    cout << "Inside predicate for layer " << i;
        //    cout << ", false" <<endl;
        //  }// else          
        }// for
        assert_("ResultUnitFactoryTest 7.5", 
                "An intersection result isn't equal.",
                (result));
        result = layerContainer2.inside(predicate4);
        assert_("ResultUnitFactoryTest 7.6", 
                "An inside result isn't available.",
                (!result));   
      }// LayerContainerTest9
      
      
      void Selftest::PFaceTest13(){
        Point3DContainer points1;
        SegmentContainer segments1,segments2;
        GlobalTimeValues timeValues(2.24250325);
        // Points for pface 0
        size_t i0 = points1.add(
          Point3D(5.61538461111, 4.19230769444, 0));
        size_t i1 = points1.add(
          Point3D(5.37288136111, 4.31355931944, 2.24250325));
        size_t i2 = points1.add(
          Point3D(4.61538461111, 1.69230769444, 0));
        size_t i3 = points1.add(
          Point3D(4.37288136111, 1.81355931944, 2.24250325));
        // Points for pface 1
        size_t i4 = points1.add(
          Point3D(5.28846153846, 3.375, 0));
        size_t i5 = points1.add(
          Point3D(5.33898305085, 4.22881355932, 2.24250325));
        size_t i6 = points1.add(
          Point3D(5.28846153846, 4.26923076923, 0));
        size_t i7 = points1.add(
          Point3D(5.33898305085, 4.22881355932, 2.24250325));
        // Points for pface 2
        size_t i8 = points1.add(
          Point3D(5.61538461111, 4.19230769444, 0));
        size_t i9 = points1.add(
          Point3D(5.37288136111, 4.31355931944, 2.24250325));
        size_t i10 = points1.add(
          Point3D(5.28846153846, 3.375, 0));
        size_t i11 = points1.add(
          Point3D(5.33898305085, 4.22881355932, 2.24250325));
        // segments for pface 0
        size_t is0 = segments1.add(Segment(i0,i1));
        size_t is1 = segments1.add(Segment(i2,i3));
        // segments for pface 1
        size_t is2 = segments2.add(Segment(i4,i5));
        size_t is3 = segments2.add(Segment(i6,i7));    
        // segments for pface 1
        size_t is4 = segments2.add(Segment(i8,i9));
        size_t is5 = segments2.add(Segment(i10,i11));   
        // create pface
        PFace pf0(is0,is1,points1,segments1);
        PFace pf1(is2,is3,points1,segments2);
        PFace pf2(is4,is5,points1,segments2);
        PFace pf3 = pf0;
        PFace pf4 = pf1;
        PFace pf5 = pf2;
        // intersection
        pf0.intersection(pf1,timeValues);
        pf0.intersection(pf2,timeValues);
        // result  
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(5.2884615, 3.375, 0, -5.0976941), 
          IntersectionPoint(5.3389831, 4.2288136, 2.2425032, -5.9092033),
          NO_INTERSECT));
        pf3.addIntSeg(IntersectionSegment(
          IntersectionPoint(5.2884615, 3.375, 0, -5.0976942), 
          IntersectionPoint(5.3389831, 4.2288135, 2.2425032, -5.9092033),
          RIGHT_IS_INNER));
        pf3.setState(CRITICAL);
        pf4.addIntSeg(IntersectionSegment(
          IntersectionPoint(5.2884615, 3.375, 0, 3.375), 
          IntersectionPoint(5.3389831, 4.2288135, 2.2425032, 4.2288135),
          LEFT_IS_INNER));
        pf4.setState(RELEVANT);
        pf5.setState(CRITICAL);
        assert_("PFaceTest 13.1", 
                "the P-faces are the same.",
                (pf0 == pf3));
        assert_("PFaceTest 13.2", 
                "the P-faces are the same.",
                (pf1 == pf4));
        assert_("PFaceTest 13.3", 
                "the P-faces are the same.",
                (pf2 == pf5));
        // cout << setprecision(9);
        // cout << points1;
        // cout << segments1;
        // cout << segments2;
        // cout << pf0;
        // cout << pf1;
        // cout << pf2;
      }// PFaceTest13
     
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
