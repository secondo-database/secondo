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
      RationalPoint3D get()const;
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
4 Class Segment3D

This class provides an oriented segment in the euclidian space.
It's start- and endpoint is represented by a Point3D each.

*/
    class Segment3D {
    public:
/*
4.1 Constructors

*/       
      Segment3D();
      Segment3D(const Point3D& tail, const Point3D& head);
/*
4.2 Getter methods.

*/
      Point3D getHead() const; 
      Point3D getTail() const; 
/*
4.3 Operators and Predicates
        
4.3.1 isOrthogonalToZAxis

Returns ~true~, if this is parallel to the xy-plane.

*/   
      bool isOrthogonalToZAxis() const;
/*
4.3.2 length
        
Returns the length of this segment.

*/       
      double length() const; 
/*
4.3.3 length2
            
Returns the quadratic length of this segment.

*/       
      mpq_class length2() const;
/*
4.3.4 Operator <<
    
Print the object values to stream.

*/        
      friend std::ostream& operator <<(std::ostream& os, 
                                       const Segment3D& segment);
/*      
4.3.5 Operator for comparison.

*/
      bool operator ==(const Segment3D& segment) const;       
    private:
      Point3D tail;
      Point3D head;
    };     
/*
5 Class RationalPoint3D

This class provides a point in the euclidian space.
It's components are represented by three values.

*/    
    class RationalPoint3D {
    public:  
/*
5.1 Constructors

*/
      RationalPoint3D();
      RationalPoint3D(const Point3D& point);
      RationalPoint3D(const RationalPoint3D& point);
      RationalPoint3D(mpq_class x, mpq_class y, mpq_class z);    
/*
5.2 Getter and setter methods

*/
      void set(const Point3D& point);
      void set(const RationalPoint3D& point);
      void set(mpq_class x, mpq_class y, mpq_class z);
      mpq_class getX() const; 
      mpq_class getY() const;
      mpq_class getZ() const;
      Point3D get()const;  
/*
5.3 Operators and Predicates
    
5.3.1 Operators for comparison.

*/
      bool operator !=(const RationalPoint3D& point) const;
      bool operator ==(const RationalPoint3D& point) const;   
/*
5.3.2 operator =   

*/
      RationalPoint3D& operator =(const RationalPoint3D& point);  
/*
5.3.3 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPoint3D& point);
/*
5.3.4 operator +
    
Returns the translation of this along vector.

*/
      RationalPoint3D  operator +(const RationalVector3D& vector) const;
/*
5.3.5 operator -
    
Returns the translation of this along -vector.

*/
      RationalPoint3D  operator -(const RationalVector3D& vector) const;
/*
5.3.6 operator -
    
Returns the Vector3D pointing from point to this.

*/
      RationalVector3D operator -(const RationalPoint3D& point) const;
 
/*
5.3.7 distance
    
Returns the distance between this and point.

*/ 
      double distance(const RationalPoint3D& point)const;
/*
5.3.8 distance2
    
Returns the quadratic distance between this and point.

*/
      mpq_class distance2(const RationalPoint3D& point)const;
    protected:  
      mpq_class x;
      mpq_class y;
      mpq_class z;
    };
    
/*
6 Class RationalVector3D

This class provides a spatial vector of dimension 3.
It's components are represented by three values.

*/
    class RationalVector3D{
    public: 
/*
6.1 Constructors

*/
      RationalVector3D();
      RationalVector3D(const RationalVector3D& vector);  
      RationalVector3D(const mpq_class& x, const mpq_class& y, 
                       const mpq_class& z);    
/*
6.2 Getter and setter methods

*/  
      void set(const RationalVector3D& vector);  
      void set(const mpq_class& x, const mpq_class& y, const mpq_class& z);
      mpq_class getX() const; 
      mpq_class getY() const;
      mpq_class getZ() const; 
 
/*
6.3 Operators and Predicates
 
6.3.1 Operators for comparison.

*/
      bool operator !=(const RationalVector3D& vector) const;
      bool operator ==(const RationalVector3D& vector) const;   
/*
    
6.3.2 Operator =

*/
      RationalVector3D& operator =(const RationalVector3D& vector);
/*
6.3.3 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalVector3D& vector);
/*      
6.3.4 Operator [*]

Returns the scalar multplication of vector and value.

*/
      friend RationalVector3D operator *(const mpq_class& value, 
                                         const RationalVector3D& vector);   
      friend RationalVector3D operator *(const RationalVector3D& vector, 
                                         const mpq_class& value);
/*
6.3.5 Operator -
    
Returns the negative of this vector.

*/ 
      RationalVector3D operator -() const;
/*
6.3.6 Operator [*] 

Returns the dot product of this and w.

*/
      mpq_class operator *(const RationalVector3D& vector) const; 
/*
6.3.7 Operator power
    
Returns the cross product of this and w.

*/
      RationalVector3D operator ^(const RationalVector3D& vector) const;
/*
6.3.8 normalize
    
Normalize this vector to length one.

*/
      void normalize();
/*
6.3.9 length
    
Returns the length of this vector.

*/
      double length() const;   
/*
6.3.10 length2
    
Returns the quadratic length of this vector.

*/
      mpq_class length2() const;
    
    private:
      mpq_class x;
      mpq_class y;
      mpq_class z;
    };// class RationalVector3D 
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// POINTVECTOR_H 