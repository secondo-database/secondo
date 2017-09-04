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

[1] Implementation of the MRegionOpsAlgebra

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/
#include <iostream>
#include <gmp.h>
#include <gmpxx.h>
#include <vector>
#include <string>
#include <memory>
#include "MMRTree.h"
#include "TemporalAlgebra.h"
#include "NumericUtil.h"

#ifndef POINTVECTORSEGMENT_H
#define POINTVECTORSEGMENT_H

namespace temporalalgebra {
  namespace mregionops3 {    
/*
2.2 Forward declarations

*/ 
    class RationalPoint3D;
    class RationalVector3D;
    class RationalSegment3D;
    class RationalPoint2D;
    class RationalVector2D;
    
/*
2.3 Enumeration Predicate

*/    
    enum Predicate { 
      UNDEFINED,
      TEST,
      LEFT_IS_INNER,
      RIGHT_IS_INNER,
      INSIDE,
      OUTSIDE,
      INTERSECT
    };
    
    std::string toString(Predicate predicate); 
/*
3 Class Point3D

This class provides a point in the euclidian space.
It's components are represented by three double values.

*/
    class Point3D{
    public:
/*
3.1 Constructors

*/
      Point3D();
      Point3D(const Point3D& point);
      Point3D(const RationalPoint3D& point);
      Point3D(double x, double y, double z);
/*
3.2 Getter methods

*/      
      double getX() const; 
      double getY() const;
      double getZ() const;
/*
3.2.1 getR()

Return the object with double representaion in rational representation

*/      
      RationalPoint3D getR() const;
/*
3.2.2 getBoundigBox()

Return the bounding box for the point object.

*/
      
      Rectangle<3> getBoundingBox() const;
/*
3.3 Operators and Predicates
    
3.3.1 Operators for comparison.

*/
      bool operator !=(const Point3D& point) const;
      bool operator ==(const Point3D& point) const;
      bool operator < (const Point3D& point) const;
/*
3.3.2 operator =
    
*/
      Point3D& operator =(const Point3D& point);   
/*
3.3.3 Operator <<
    
Overloaded output operators.

*/         
      friend std::ostream& operator <<(std::ostream& os, const Point3D& point);
    private:  
/*
3.4 Private methods

3.4.1 setter methods

*/    
      void set(const Point3D& point);
      void set(const RationalPoint3D& point);
      void set(double x, double y, double z);
/*
3.5 Attributes

*/      
      double x;
      double y;
      double z;     
    };// class Point3D
/*       
4 Class RationalPoint3D

This class provides a point in the euclidian space.
It's components are represented by three rational values.

*/    
    class RationalPoint3D {
    public:  
/*
4.1 Constructors

*/
      RationalPoint3D();
      RationalPoint3D(const Point3D& point);
      RationalPoint3D(const RationalPoint3D& point);
      RationalPoint3D(mpq_class x, mpq_class y, mpq_class z);    
/*
4.2 Getter methods

*/
      mpq_class getX() const; 
      mpq_class getY() const;
      mpq_class getZ() const;
/*
4.2.1 getD()

Return the object with rational representaion in double representation

*/         
      Point3D getD()const;  
/*
4.3 Methods, operators and predicates
    
4.3.1 Operators for comparison.

*/
      bool operator !=(const RationalPoint3D& point) const;
      bool operator ==(const RationalPoint3D& point) const; 
/*
4.3.2 operator =   

*/
      RationalPoint3D& operator =(const RationalPoint3D& point);  
/*
4.3.3 Operator <<
    
Overloaded output operators.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPoint3D& point);
/*
4.3.4 operator +
    
Returns the translation of this along vector.

*/
      RationalPoint3D  operator +(const RationalVector3D& vector) const;
/*
4.3.5 operator -
    
Returns the translation of this along -vector.

*/
      RationalPoint3D  operator -(const RationalVector3D& vector) const;
/*
4.3.6 operator -
    
Returns the Vector3D pointing from point to this.

*/
      RationalVector3D operator -(const RationalPoint3D& point) const;
/*
4.3.7 distance
    
Returns the distance between this and point as double value.

*/ 
      double distance(const RationalPoint3D& point)const;
/*
4.3.8 distance2
    
Returns the quadratic distance between this and point as rational value.

*/
      mpq_class distance2(const RationalPoint3D& point)const;
    protected:  
/*
4.4 Private methods

4.4.1 Setter methods

*/  
      void set(const Point3D& point);
      void set(const RationalPoint3D& point);
      void set(mpq_class x, mpq_class y, mpq_class z);
/*
4.5 Attributes

*/       
      mpq_class x;
      mpq_class y;
      mpq_class z;
    };// RationalPoint3D
    
/*
5 Class RationalVector3D

This class provides a spatial vector of dimension 3.
It's components are represented by three rational values.

*/
    class RationalVector3D{
    public: 
/*
5.1 Constructors

*/
      RationalVector3D();
      RationalVector3D(const RationalVector3D& vector);  
      RationalVector3D(const mpq_class& x, const mpq_class& y, 
                       const mpq_class& z);    
/*
5.2 Getter methods

*/  
      mpq_class getX() const; 
      mpq_class getY() const;
      mpq_class getZ() const; 
/*
5.3 Methods, operators and predicates
 
5.3.1 Operators for comparison.

*/
      bool operator !=(const RationalVector3D& vector) const;
      bool operator ==(const RationalVector3D& vector) const;   
/* 
5.3.2 Operator =

*/
      RationalVector3D& operator =(const RationalVector3D& vector);
/*
5.3.3 Operator <<
    
Overloaded output operators.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalVector3D& vector);
/*      
5.3.4 Operator [*]

Returns the scalar multplication of vector and value.

*/
      friend RationalVector3D operator *(const mpq_class& value, 
                                         const RationalVector3D& vector);   
      friend RationalVector3D operator *(const RationalVector3D& vector, 
                                         const mpq_class& value);
/*
5.3.5 Operator -
    
Returns the negative of this vector.

*/ 
      RationalVector3D operator -() const;
/*
5.3.6 Operator [*] 

Returns the dot product of this and vector.

*/
      mpq_class operator *(const RationalVector3D& vector) const; 
/*
5.3.7 Operator power
    
Returns the cross product of this and vector.

*/
      RationalVector3D operator ^(const RationalVector3D& vector) const;
/*
5.3.8 normalize
    
Normalize this vector to length one.

*/
      void normalize();
/*
5.3.9 length
    
Returns the length of this vector.

*/
      double length() const;   
/*
5.3.10 length2
    
Returns the quadratic length of this vector.

*/
      mpq_class length2() const;
    
    private:
/*
5.4 Private methods

5.4.1 Setter methods

*/    
      void set(const RationalVector3D& vector);  
      void set(const mpq_class& x, const mpq_class& y, const mpq_class& z);
/*
5.5 Attributes

*/        
      mpq_class x;
      mpq_class y;
      mpq_class z;
    };// class RationalVector3D 
/*
6 Class Segment3D

This class provides an oriented segment in the euclidian space.
It's start- and endpoint is represented by a Point3D each.

*/
    class Segment3D {
    public:
/*
6.1 Constructors

*/       
      Segment3D();
      Segment3D(const Segment3D& segment);
      Segment3D(const RationalSegment3D& segment);
      Segment3D(const Point3D& tail, const Point3D& head);
/*
6.2 Getter methods.

*/
      Point3D getHead() const; 
      Point3D getTail() const; 
/*
6.2.1 getR()

Return the object with doube representaion in rational representation

*/       
      RationalSegment3D getR() const;
/*
6.3 Operators and predicates
        
6.3.1 Operator <<
    
Overloaded output operators.

*/    
      friend std::ostream& operator <<(std::ostream& os, 
                                       const Segment3D& segment);
/*      
6.3.2 Operator for comparison.

*/
      bool operator ==(const Segment3D& segment) const; 
/*
6.3.3 operator =
    
*/
      Segment3D& operator =(const Segment3D& segment);         
    private:
/*
6.4 Private methods

6.4.1 Setter methods

*/
      void set(const Segment3D& segment);
/*
6.5 Attributes

*/        
      Point3D tail;
      Point3D head;
    };// Segment3D    
/*
7 Class RationalSegment3D

This class provides an oriented segment in the euclidian space.
It's start- and endpoint is represented by a RationalPoint3D each.

*/
    class RationalSegment3D {
    public:
/*
7.1 Constructors

*/       
      RationalSegment3D();
      RationalSegment3D(const RationalSegment3D& segment);
      RationalSegment3D(const Segment3D& segment);
      RationalSegment3D(const RationalPoint3D& tail, 
                        const RationalPoint3D& head);
/*
7.2 Getter methods.

*/
      RationalPoint3D getHead() const; 
      RationalPoint3D getTail() const; 
/*
4.2.1 getD()

Return the object with rational representaion in double representation

*/       
      Segment3D getD() const;
/*
7.3 Operators and predicates

7.3.1 Operator <<
    
Overloaded output operators.

*/        
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalSegment3D& segment);
/*      
7.3.2 Operator for comparison.

*/
      bool operator ==(const RationalSegment3D& segment) const; 

/*
7.3.3 operator =   

*/
      RationalSegment3D& operator =(const RationalSegment3D& point);      
    private:
/*
7.4 Private methods

7.4.1 Setter methods

*/     
      void set(const RationalSegment3D& segment);
/*
7.5 Attributes

*/        
      RationalPoint3D tail;
      RationalPoint3D head;
    };// RationalSegment3D
    
/*
8 Class Point2D

This class provides a point in the euclidian plane.
It's components are represented by two double values.

*/    
    class Point2D{
    public:
/*
8.1 Constructors

*/
      Point2D();
      Point2D(const Point2D& point);
      Point2D(const RationalPoint2D& point);
      Point2D(double x, double y);
/*
8.2 Getter methods

*/
      double getX() const; 
      double getY() const;
      RationalPoint2D getR()const;
/*
8.3 Operators and predicates

8.3.1 Operator for comparison.

*/
      bool operator ==(const Point2D& point) const;  
/*
8.3.2 operator =
    
*/
      Point2D& operator =(const Point2D& point);   
/*
8.3.3 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, const Point2D& point);
      
      bool operator <(const Point2D& point)const;
      
    private:  
/*
8.4 Private methods

8.4.1 set

*/    
      void set(const Point2D& point);
      void set(double x, double y);
/*
8.5 Attributes

*/       
      double x;
      double y;
    };// class Point2D
    
/*
9 Class RationalPoint2D

This class provides a point in the euclidian plane.
It's components are represented by two rational values.

*/    
    class RationalPoint2D{
    public:
/*
9.1 Constructors

*/
      RationalPoint2D();
      RationalPoint2D(const RationalPoint2D& point);
      RationalPoint2D(const Point2D& point);
      RationalPoint2D(const mpq_class& x, const mpq_class& y);
/*
9.2 Getter methods

*/
      mpq_class getX() const; 
      mpq_class getY() const;  
      Point2D getD()const;
/*
9.3 Operators and predicates

9.3.1 Operator for comparison.

*/
      bool operator ==(const RationalPoint2D& point) const;  
/*
9.3.2 operator =
    
*/
      RationalPoint2D& operator =(const RationalPoint2D& point);   
/*
9.3.3 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPoint2D& point);
/*
9.3.4 operator -
    
Returns the Vector2D pointing from p to this.

*/       
      RationalVector2D operator -(const RationalPoint2D& point) const;
      
    private:  
/*
9.4 Private methods

9.4.1 set

*/    
      void set(const RationalPoint2D& point);
      void set(const mpq_class& x, const mpq_class& y);
/*
9.5 Attributes

*/         
      mpq_class x;
      mpq_class y;
    };// class RationalPoint2D
    
/*
10 Class RationalVector2D

This class provides a vector in the euclidian plane.
It's components are represented by two rational values.

*/    
    class RationalVector2D{
    public:
/*
10.1 Constructors

*/
      RationalVector2D();
      RationalVector2D(const RationalVector2D& vector);
      RationalVector2D(const mpq_class& x, const mpq_class& y);
/*
10.2 Getter and setter methods

*/
      mpq_class getX() const; 
      mpq_class getY() const;  
/*
10.3 Methodes, operators and predicates

10.3.1 Operator for comparison.

*/
      bool operator ==(const RationalVector2D& vector) const;  
/*
10.3.2 operator =
    
*/
      RationalVector2D& operator =(const RationalVector2D& vector);
/*
10.3.3 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalVector2D& vector);
/*
10.3.4 normalize
    
Normalize this vector to length one.

*/         
      void normalize();
/*
10.3.5 operator $|$
    
Returns the perp product of this and vector: a scalar.

*/   
      mpq_class operator |(const RationalVector2D& vector) const; 
      
      double length() const;
    private:  
/*
10.4 Private methods

10.4.1 set

*/    
      void set(const RationalVector2D& vector);
      void set(const mpq_class& x, const mpq_class& y);
/*
10.5 Attributes

*/        
      mpq_class x;
      mpq_class y;
    };// class RationalVector2D
/*
11 Class Segment2D

This class provides an oriented segment in the euclidian plane.
It's start- and endpoint is represented by a Point2D each.

*/
    class RationalSegment2D;
    class Segment2D {
    public:
/*
11.1 Constructors

*/       
      Segment2D();
      Segment2D(const Point2D& tail, const Point2D& head);
      Segment2D(const RationalSegment2D& segment);
/*
11.2 Getter methods.

*/
      Point2D getHead() const; 
      Point2D getTail() const; 
      RationalSegment2D getR()const;
/*
11.3 Methods, operators and predicates
        
11.3.1 Operator <<

Print the object values to stream.

*/        
      friend std::ostream& operator <<(std::ostream& os, 
                                       const Segment2D& segment);   
/*      
11.3.2 Operator for comparison.

*/
      bool operator ==(const Segment2D& segment) const; 
/*      
11.3.2 whichSide.

*/      
      double whichSide(const Point2D& point)const;
/*      
11.3.3 isLeft

*/      
      bool isLeft(const Point2D& point) const;   
      
    protected:
/*
11.4 Attributes

*/       
      Point2D tail;
      Point2D head;
    };// Segment2D   
    
/*
11 Class RationalSegment2D

This class provides an oriented segment in the euclidian plane.
It's start- and endpoint is represented by a RationalPoint2D each.

*/
    class RationalSegment2D {
    public:
/*
11.1 Constructors

*/       
      RationalSegment2D();
      RationalSegment2D(const RationalPoint2D& tail, 
                        const RationalPoint2D& head);
      RationalSegment2D(const Segment2D& segment);
/*
11.2 Getter methods.

*/
      RationalPoint2D getHead() const; 
      RationalPoint2D getTail() const; 
      
      Segment2D getD()const; 
      
/*
11.3 Methods, operators and predicates
        
11.3.1 Operator <<

Print the object values to stream.

*/        
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalSegment2D& segment);   
/*      
11.3.2 Operator for comparison.

*/
      bool operator ==(const RationalSegment2D& segment) const; 
 
      bool intersection(const RationalSegment2D& other, 
                        RationalPoint2D& intersectionPoint);
      
    protected:
/*
11.4 Attributes

*/       
      RationalPoint2D tail;
      RationalPoint2D head;
    };// RationalSegment2D 
    
/*
12 Class Point3DContainer

This class provides a containervector for Point3d values.

*/      
    class Point3DContainer{
    public:
/*
12.1 Constructors

*/    
      Point3DContainer();
      Point3DContainer(const Point3DContainer& points);
/*
12.2 Getter methods.

*/     
      Point3D  get(size_t index)const;
      size_t   size()const;
/*
12.3 Methods, Operators and predicates
        
12.3.1 Operator <<

Print the object values to stream.

*/            
      friend std::ostream& operator<<( std::ostream& os, 
                                       const Point3DContainer& container); 
/*      
12.3.2 Operator for comparison.

*/      
      bool operator == (const Point3DContainer& points)const;
/*
12.3.3 operator =
    
*/      
      Point3DContainer& operator = (const Point3DContainer& points);
/*
12.3.4 add.

*/       
      size_t add(const Point3D& point);
/*
12.3.5 print.

*/       
      std::ostream& print(std::ostream& os, std::string prefix)const;           
    private:
/*
12.4 Private methods

12.4.1 set

*/      
      void set(const Point3DContainer& points);
/*
12.5 Attributes

*/        
      std::vector<Point3D> points; 
      mmrtree::RtreeT<3, size_t> pointsTree;
    };// Point3DContainer
    
/*
3 class Segment

*/     
    class Segment {
    public:
/*
3.1 Constructor

*/        
      Segment ();
      Segment (size_t head, size_t tail, 
               const Predicate& predicate = UNDEFINED);
      Segment (const Segment& segment);
/*
3.2 Getter methods

*/     
      void setPredicate(Predicate predicate);
      size_t getHead()const;
      size_t getTail()const;
      Predicate getPredicate() const;
/*
3.3 Methods, Operators and Predicates

3.3.1 Operator <<
    
Print the object values to stream.

*/      
      friend std::ostream& operator <<(std::ostream& os, 
                                       const Segment& segment);
/*      
3.3.2 Operator for comparison.

*/
      bool operator ==(const Segment& segment) const; 
/*
3.3.3 operator =
    
*/
      Segment& operator =(const Segment& segment);     
    private:
/*
3.4 Private methods

3.4.1 set

*/        
      void set(const Segment& segment);  
/*
3.5 Attributes

*/
      size_t head;
      size_t tail;
      Predicate predicate;
    };// Segment     
    
    class SegmentContainer {
    public:  
      // Konstruktor
      SegmentContainer();
      
      SegmentContainer(const SegmentContainer& other);
      
      size_t size()const;

      size_t add(const Segment& segment, bool pFaceIsCritical = false);
      
      void clear();
      
      void set(size_t index, Predicate predicate);
      
      Segment   get(size_t index)const;
      
      std::ostream& print(std::ostream& os, std::string prefix)const;
      
      friend std::ostream& operator<<( std::ostream& os, 
                                       const SegmentContainer& container); 
      
      bool operator == (const SegmentContainer& segments)const;
      
      SegmentContainer& operator = (const SegmentContainer& segments);
    private:  
      
      size_t getHash(const Segment& segment)const;
      
      void set(const SegmentContainer& other);
      
      const size_t buckets = 47;
      
      std::vector<std::list<size_t>> segmentBuckets;
      std::vector<Segment> segments; 
    }; //SegmentContainer

  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// POINTVECTOR_H 