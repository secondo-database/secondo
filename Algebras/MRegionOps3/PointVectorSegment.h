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
3.2 Getter and setter methods

*/
      void set(const Point3D& point);
      void set(const RationalPoint3D& point);
      void set(double x, double y, double z);
      double getX() const; 
      double getY() const;
      double getZ() const;
      RationalPoint3D getR()const;
      Rectangle<3> getBoundingBox()const;
/*
3.3 Operators and Predicates
    
3.3.1 Operators for comparison.

*/
      bool operator !=(const Point3D& point) const;
      bool operator ==(const Point3D& point) const;   
/*
3.3.2 operator =
    
*/
      Point3D& operator =(const Point3D& point);   
/*
3.3.3 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, const Point3D& point);
    private:          
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
4.2 Getter and setter methods

*/
      void set(const Point3D& point);
      void set(const RationalPoint3D& point);
      void set(mpq_class x, mpq_class y, mpq_class z);
      mpq_class getX() const; 
      mpq_class getY() const;
      mpq_class getZ() const;
      Point3D getD()const;  
/*
4.3 Operators and Predicates
    
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
    
Print the object values to stream.

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
    
Returns the distance between this and point.

*/ 
      double distance(const RationalPoint3D& point)const;
/*
4.3.8 distance2
    
Returns the quadratic distance between this and point.

*/
      mpq_class distance2(const RationalPoint3D& point)const;
    protected:  
      mpq_class x;
      mpq_class y;
      mpq_class z;
    };
    
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
5.2 Getter and setter methods

*/  
      void set(const RationalVector3D& vector);  
      void set(const mpq_class& x, const mpq_class& y, const mpq_class& z);
      mpq_class getX() const; 
      mpq_class getY() const;
      mpq_class getZ() const; 
/*
5.3 Operators and Predicates
 
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
    
Print the object values to stream.

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

Returns the dot product of this and w.

*/
      mpq_class operator *(const RationalVector3D& vector) const; 
/*
5.3.7 Operator power
    
Returns the cross product of this and w.

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
      void set(const Segment3D& segment);
      Point3D getHead() const; 
      Point3D getTail() const; 
      RationalSegment3D getR() const;
/*
6.3 Operators and Predicates
        
6.3.1 Operator <<
    
Print the object values to stream.

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
    protected:
      Point3D tail;
      Point3D head;
    };    
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
      void set(const RationalSegment3D& segment);
      RationalPoint3D getHead() const; 
      RationalPoint3D getTail() const; 
      Segment3D getD() const;

/*
7.3.1 Operator <<
    
Print the object values to stream.

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
      RationalPoint3D tail;
      RationalPoint3D head;
    };
    
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
8.2 Getter and setter methods

*/
      void set(const Point2D& point);
      void set(double x, double y);
      double getX() const; 
      double getY() const;
      RationalPoint2D getR()const;
/*
8.3 Operators and Predicates

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
    private:          
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
9.2 Getter and setter methods

*/
      void set(const RationalPoint2D& point);
      void set(const mpq_class& x, const mpq_class& y);
      mpq_class getX() const; 
      mpq_class getY() const;  
      Point2D getD()const;
/*
9.3 Operators and Predicates

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
      void set(const RationalVector2D& vector);
      void set(const mpq_class& x, const mpq_class& y);
      mpq_class getX() const; 
      mpq_class getY() const;  
/*
10.3 Operators and Predicates

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
    private:          
      mpq_class x;
      mpq_class y;
    };// class RationalVector2D
/*
11 Class Segment2D

This class provides an oriented segment in the euclidian plane.
It's start- and endpoint is represented by a Point2D each.

*/
    class Segment2D {
    public:
/*
11.1 Constructors

*/       
      Segment2D();
      Segment2D(const Point2D& tail, const Point2D& head);
/*
11.2 Getter methods.

*/
      Point2D getHead() const; 
      Point2D getTail() const; 
/*
11.3 Operators and Predicates
        
11.3.1 Operator <<

Print the object values to stream.

*/        
      friend std::ostream& operator <<(std::ostream& os, 
                                       const Segment2D& segment);   
/*      
11.3.2 Operator for comparison.

*/
      bool operator ==(const Segment2D& segment) const; 
      
      double whichSide(const Point2D& point)const;
/*      
11.3.3 isLeft

*/      
      bool isLeft(const Point2D& point) const;    
    protected:
      Point2D tail;
      Point2D head;
    };   
    
    template <typename T> class Container{
    public:
      // Konstruktor
      Container();
      Container(const Container<T>& points);
      // Destruktor
      ~Container();      
      size_t          add(const T& points);
      T               get(const size_t index)const;
      size_t          size()const;
      std::ostream& print(std::ostream& os, std::string prefix)const;
      bool operator == (const Container<T>& points)const;
      Container<T>& operator = (const Container<T>& points);
    private:
      void set(const Container<T>& points);
      
      std::vector<T> points; 
      mmrtree::RtreeT<3, size_t> pointsTree;
    };
    
    template <typename T> std::ostream& operator<<(
      std::ostream& os, const Container<T>& container); 
    
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// POINTVECTOR_H 