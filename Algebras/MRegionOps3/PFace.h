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

#include "PointVectorSegment.h"
#include "NumericUtil.h"
#include "TemporalAlgebra.h"
#include <gmp.h>
#include <gmpxx.h>
#include <set>
#include <vector>

#ifndef PFACE_H
#define PFACE_H

namespace temporalalgebra {
  namespace mregionops3 {
    
    class PFace;
/*
3 Enumeration SourceFlag

Indicates the source unit of a ~RationalPoint3DExt~.

*/
    enum SourceFlag {
        PFACE_A,
        PFACE_B
    };
/*
4 Class RationalPoint3DExt

This datastructure is used in the class ~RationalPointExtSet~.
It simply extends ~Point3D~ with the attribute ~sourceFlag~.

*/
    class RationalPoint3DExt : public RationalPoint3D {
    public:
/*
4.1 Constructors

*/
      RationalPoint3DExt();
      RationalPoint3DExt(const mpq_class& a, const mpq_class& b, 
                         const mpq_class& c, SourceFlag sourceFlag);
/*
4.2 Setter and getter methods

*/      
      void setSourceFlag(SourceFlag flag);
      SourceFlag getSourceFlag()const;
/*
4.3 Operators and Predicates

4.3.1 operator $<$

*/
      bool operator < (const RationalPoint3DExt& point) const;
/*
4.3.2 Operator <<
    
Print the object values to stream.

*/     
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPoint3DExt& point);      
    private:  
/*
4.4 Attributes

4.4.1 sourceFlag

An enum, indicating the ~PFace~, this ~RationalPoint3DExt~ belongs to.
Possible values are:

  * $PFACE\_A$

  * $PFACE\_B$

*/
      SourceFlag sourceFlag;
};

/*
5 Class RationalPoint3DExtSet

This set is used in the class ~PFace~ to compute the intersection segment of
two ~PFaces~.

*/
    class RationalPoint3DExtSet {
    public:
/*
5.1 Constructors

*/
      RationalPoint3DExtSet();
/*
5.2 Operators and Predicates

5.2.1 insert

Inserts point, if point isn't already inserted.

*/
      void insert(const RationalPoint3DExt& point); 
/*
5.2.2 size

Returns the number of points in the set.

*/
      size_t size() const; 
/*
5.2.3 getIntersectionSegment

Returns ~true~, if there is an intersection segment and writes it to result.

*/
      bool getIntersectionSegment(Segment3D& result) const;
/*
5.2.4 Operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPoint3DExtSet& points);
    private:
/*
5.3 Attributes

5.3.1 point

A ~std::set~, using the overloaded operator $<$ for comparison.

*/
      std::set<RationalPoint3DExt> points;
    };  
 
/*
6 Class RationalPlane3D

*/   
    class RationalPlane3D{
    public:
/*
6.1 Constructors

*/      
      RationalPlane3D();
      RationalPlane3D(const RationalPlane3D& plane);
      RationalPlane3D(const PFace& pf);
/*
6.2 Setter and getter methods

*/      
      void set(const RationalVector3D& normalVector,
               const RationalPoint3D& pointOnPlane);
      void set(const RationalPlane3D& plane);     
      RationalPoint3D getPointOnPlane() const;
      RationalVector3D getNormalVector() const;
/*
6.3 Operators and Predicates

6.3.1 distance2ToPlane

*/            
      mpq_class distance2ToPlane(const RationalPoint3D& point) const;
/*
6.3.2 isParallelTo

Returns ~true~, if this ~RationalPlane3D~ is parallel to the 
~RationalPlane3D~ plane.

*/      
      bool isParallelTo(const RationalPlane3D& plane) const;
/*
6.3.2 isCoplanarTo

Returns ~true~, if this ~RationalPlane3D~ is coplanar to the argument.

*/      
      bool isCoplanarTo(const RationalPlane3D& plane)const;
/*
6.3.3 intersection


*/           
      bool intersection(const Segment3D segment, RationalPoint3D& result)const;
      void intersection(const PFace& other, SourceFlag sourceFlag, 
                        RationalPoint3DExtSet& intPointSet)const;
/*
6.3.4 operator =

*/                                     
      RationalPlane3D& operator =(const RationalPlane3D& point);       
/*
6.3.5 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPlane3D& plane);
    private:
/*
6.4 Attributes

*/     
      RationalVector3D  normalVector;
      RationalPoint3D   pointOnPlane;
    };
/*
7 Class PFace

*/    
    class PFace {
    friend class Selftest;    
    public:
/*
7.1 Constructors

*/
      PFace(const Point3D& a, const Point3D& b, const Point3D& c, 
            const Point3D& d);
/*
7.2 Getter methods

*/
      Point3D getA() const; 
      Point3D getB() const;
      Point3D getC() const;       
      Point3D getD() const; 
/*
7.3 Operators and Predicates

7.3.3 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, const PFace& pf);      
/*
7.3.4 intersection

Computes the intersection of this ~PFace~ with pf. 

*/
      bool intersection(const PFace& other,Segment3D& intSeg)const;       
    private:                                 
/*
7.4 Attributes

*/      
      Rectangle<2> boundingRect;  
      Point3D      a;
      Point3D      b;
      Point3D      c;
      Point3D      d;            
    };// calss PFace   
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// PFACE_H 