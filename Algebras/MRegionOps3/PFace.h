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
#include <string>

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
4 Enumeration State

Used in the class ~PFace~ to indicate it's current state.

*/
    enum State {
      UNKNOWN,
      ENTIRELY_INSIDE,
      ENTIRELY_OUTSIDE,
      RELEVANT_NOT_CRITICAL,
      RELEVANT_CRITICAL,
      NOT_RELEVANT
    };
    
    enum Indicator {
      LEFT_IS_INNER,
      RIGHT_IS_INNER,
      BORDER
    };
    
    enum Border {
      LEFT,
      RIGHT
    };
    
/*
5 Class RationalPoint3DExt

This datastructure is used in the class ~RationalPointExtSet~.
It simply extends ~Point3D~ with the attribute ~sourceFlag~.

*/
    class RationalPoint3DExt : public RationalPoint3D {
    public:
/*
5.1 Constructors

*/
      RationalPoint3DExt();
      RationalPoint3DExt(const mpq_class& a, const mpq_class& b, 
                         const mpq_class& c, SourceFlag sourceFlag);
/*
5.2 Setter and getter methods

*/      
      void setSourceFlag(SourceFlag flag);
      SourceFlag getSourceFlag()const;
/*
5.3 Operators and Predicates

5.3.1 operator $<$

*/
      bool operator < (const RationalPoint3DExt& point) const;
/*
5.3.2 Operator <<
    
Print the object values to stream.

*/     
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPoint3DExt& point);      
    private:  
/*
5.4 Attributes

5.4.1 sourceFlag

An enum, indicating the ~PFace~, this ~RationalPoint3DExt~ belongs to.
Possible values are:

  * $PFACE\_A$

  * $PFACE\_B$

*/
      SourceFlag sourceFlag;
};

/*
6 Class RationalPoint3DExtSet

This set is used in the class ~PFace~ to compute the intersection segment of
two ~PFaces~.

*/
    class RationalPoint3DExtSet {
    public:
/*
6.1 Constructors

*/
      RationalPoint3DExtSet();
/*
6.2 Operators and Predicates

6.2.1 insert

Inserts point, if point isn't already inserted.

*/
      void insert(const RationalPoint3DExt& point); 
/*
6.2.2 size

Returns the number of points in the set.

*/
      size_t size() const; 
/*
6.2.3 getIntersectionSegment

Returns ~true~, if there is an intersection segment and writes it to result.

*/
      bool getIntersectionSegment(RationalSegment3D& result) const;
/*
6.2.4 Operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPoint3DExtSet& points);
    private:
/*
6.3 Attributes

6.3.1 point

A ~std::set~, using the overloaded operator $<$ for comparison.

*/
      std::set<RationalPoint3DExt> points;
    };  
 
/*
7 Class RationalPlane3D

*/   
    class RationalPlane3D{
    public:
/*
7.1 Constructors

*/      
      RationalPlane3D();
      RationalPlane3D(const RationalPlane3D& plane);
      RationalPlane3D(const PFace& pf);
/*
7.2 Setter and getter methods

*/      
      void set(const RationalVector3D& normalVector,
               const RationalPoint3D& pointOnPlane);
      void set(const RationalPlane3D& plane);     
      RationalPoint3D getPointOnPlane() const;
      RationalVector3D getNormalVector() const;
/*
7.3 Operators and Predicates

7.3.1 distance2ToPlane

*/            
      mpq_class distance2ToPlane(const RationalPoint3D& point) const;
/*
7.3.2 isParallelTo

Returns ~true~, if this ~RationalPlane3D~ is parallel to the 
~RationalPlane3D~ plane.

*/      
      bool isParallelTo(const RationalPlane3D& plane) const;
/*
7.3.2 isCoplanarTo

Returns ~true~, if this ~RationalPlane3D~ is coplanar to the argument.

*/      
      bool isCoplanarTo(const RationalPlane3D& plane)const;
/*
7.3.3 intersection


*/           
      bool intersection(const Segment3D segment, RationalPoint3D& result)const;
      void intersection(const PFace& other, SourceFlag sourceFlag, 
                        RationalPoint3DExtSet& intPointSet)const;
/*
7.3.4 operator =

*/                                     
      RationalPlane3D& operator =(const RationalPlane3D& point);       
/*
7.3.5 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPlane3D& plane);
/*
7.3.6 isLeftAreaInner   

*/       
      bool isLeftAreaInner(const RationalSegment3D segment,
                           const RationalPlane3D other)const;
/*
7.3.7 transform   

*/                              
      Point2D transform(const RationalPoint3D& point) const;
      Segment2D transform(const RationalSegment3D& segment) const;
    private:
/*
7.4 Attributes

*/     
      RationalVector3D  normalVector;
      RationalVector3D  wVector;
      RationalPoint3D   pointOnPlane;
    };
/*
8 Class IntersectionPoint

*/       
    class IntersectionPoint {
    public:  
/*
8.1 Constructors

*/        
     IntersectionPoint();
     IntersectionPoint(const IntersectionPoint& point);
     IntersectionPoint(const Point3D& point3D, const Point2D& point2D); 
     IntersectionPoint(double x, double y, double z, double w);
/*
8.2 Setter and getter methods

*/        
      void set(const IntersectionPoint& point);
      Point3D getPoint3D() const;
      Point2D getPoint2D() const;
      double getX()const;
      double getY()const;
      double getZ()const;
      double getW()const;
      double getT()const;
      Rectangle<3> getBoundingBox()const;
/*
8.3.1 Operator <<
    
Print the object values to stream.

*/        
      friend std::ostream& operator <<(std::ostream& os, 
                                       const IntersectionPoint& point);
/*
8.3.2 operator =
    
*/
      IntersectionPoint& operator =(const IntersectionPoint& point);  
/*
8.3.3 operator ==
    
*/      
      bool operator ==(const IntersectionPoint& point) const;
    private:  
      double x;
      double y;
      double z;
      double w;
    };
/*
9 Class IntersectionSegment

*/      
    class IntersectionSegment{
    public:
/*
9.1 Constructors

*/         
      IntersectionSegment();
      IntersectionSegment(const IntersectionSegment& segment);
      IntersectionSegment(const IntersectionPoint& tail,
                          const IntersectionPoint& head, 
                          const Indicator indicator);
      IntersectionSegment(const Segment3D& segment3D, 
                          const Segment2D& segment2D, 
                          const Indicator indicator);
/*
9.2 Setter and getter methods

*/        

      Segment3D getSegment3D()const;
      Segment2D getSegment2D()const;
      IntersectionPoint getTail() const;
      IntersectionPoint getHead() const;
      Indicator getIndicator()const;
      
      bool isOrthogonalToTAxis()const;      
      bool isOutOfRange(double t) const;
      bool isLeftOf(const IntersectionSegment& intSeg) const;
      Point3D evaluate(double t) const;
/*
9.3.1 Operator <<
    
Print the object values to stream.

*/       
      friend std::ostream& operator <<(std::ostream& os, 
                                       const IntersectionSegment& segment);
/*
9.3.2 operator =
    
*/
      IntersectionSegment& operator =(const IntersectionSegment& segment);
/*
9.3.3 operator ==
    
*/      
      bool operator ==( const IntersectionSegment& segment) const;
/*
12.3.5 toString

*/      
      static std::string toString(Indicator indicator);      
    private: 
      void set(const IntersectionPoint& tail,const IntersectionPoint& head,
               Indicator indicator);
      void set(const IntersectionSegment& segment);
/*
 12.4 Attributes
 
*/
      IntersectionPoint tail;
      IntersectionPoint head;
      Indicator indicator;  
    };
/*
10 Struct IntSegCompare

This struct implements the ~IntersectionSegment~ order,
used in ~IntSegContainer~.

*/
    struct IntSegCompare {

      bool operator()(const IntersectionSegment* const& s1,
                      const IntersectionSegment* const& s2) const;
    };        
/*
11 Class IntSegContainer

This class is used by the class ~PFace~ and provides essentially
an ordered set of ~IntersectionSegments~

*/
    class IntSegContainer {
    public: 
/*
11.1 Constructor

*/     
      IntSegContainer();
      IntSegContainer(const IntSegContainer& container);
/*
11.2 Destructor

*/
      ~IntSegContainer();
      void set(const IntSegContainer& container);
/*
11.3 Operators

11.3.1 AddIntSeg
s
Adds seg to the set of ~IntersectionSegments~.

*/  
      void addIntSeg(const IntersectionSegment& seg);
      size_t size()const;
     
/*
11.3.2 operator =
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
      friend std::ostream& operator <<(std::ostream& os, 
                                       const IntSegContainer& container);
/*
11.3.3 operator ==
    
*/     
      bool operator ==(const IntSegContainer& container)const; 
      IntSegContainer& operator =(const IntSegContainer& container);  
      
      void first(double t,std::vector<IntersectionSegment>& result);
      void next(double t,std::vector<IntersectionSegment>& result);
      
 //     void updateTimeLevel(double _t);
      
    private:
      
      bool hasMoreSegsToInsert() const; 
/*
11.4 Attributes

11.4.1 intSegs, intSegIter

A ~std::set~ to store the ~IntersectionSegments~ using the order
provided by ~IntSegCompare~ and a suitable iterator.

*/
      std::set<IntersectionSegment*, IntSegCompare> intSegs;
      std::set<IntersectionSegment*, IntSegCompare>::iterator intSegIter;
      
/*
1.1.1 active, activeIter

A ~std::list~ to store the active ~IntersectionSegments~ during the plane-sweep
and a suitable iterator.

*/
      std::list<IntersectionSegment*> active;
      std::list<IntersectionSegment*>::iterator activeIter;
      
      double t;
//      bool firstTimeLevel;
      
    };   
/*
12 struct DoubleCompare

*/    
    struct DoubleCompare {
      bool operator()(const double& d1, const double& d2) const;
    };
/*
13 class GlobalTimeValues

*/     
    class GlobalTimeValues{
    public:  
/*
13.3 Operators and Predicates

13.3.1 addTimeValue

*/      
      void addTimeValue(double t);
/*
13.3.2 Operator <<

*/       
      friend std::ostream& operator <<(std::ostream& os, 
                                       GlobalTimeValues& timeValues);
/*
13.3.3 Operator ==

*/                                        
      bool operator ==(const GlobalTimeValues& other)const; 
      
      bool first(double& t);
      
      bool next(double& t);
            
    private: 
      std::set<double, DoubleCompare> time;
      std::set<double, DoubleCompare>::const_iterator timeIter;
      double t1;
      double t2;
    };
/*
14 Class PFace

*/    
    class PFace {
    friend class Selftest;    
    public:
/*
14.1 Constructors

*/
      PFace(const Point3D& a, const Point3D& b, const Point3D& c, 
            const Point3D& d);
      PFace(const PFace& pf);     
/*
14.2 Setter and Getter methods

*/
      void    set(const PFace& pf);
      void    setState(State state);
      Point3D getA() const; 
      Point3D getB() const;
      Point3D getC() const;       
      Point3D getD() const; 
      State   getState() const;
      Rectangle<2> getBoundingRec()const;
/*
14.3 Operators and Predicates

14.3.1 existsIntSegs

*/
      bool existsIntSegs()const;
/*
14.3.2 print

*/        
      std::ostream& print(std::ostream& os, std::string prefix)const;
/*
14.3.3 Operator <<
    
Print the object values to stream.

*/            
      friend std::ostream& operator <<(std::ostream& os, const PFace& pf);      
/*
14.3.4 intersection

Computes the intersection of this ~PFace~ with pf. 

*/
      bool intersection(PFace& other);
/*
14.3.5 toString

*/      
      static std::string toString(State state);
/*
14.3.6 addIntSeg

*/       
      void addIntSeg(const IntersectionSegment& seg);
      void addIntSeg(const RationalPlane3D &planeSelf, 
                     const RationalPlane3D &planeOther,
                     const RationalSegment3D &intSeg,
                     GlobalTimeValues &timeValues);
/*
14.3.7 addBorder

*/        
      void addBorder(const RationalPlane3D &plane,
                     GlobalTimeValues &timeValues);
/*
14.3.8 Operator =

*/  
      PFace& operator =(const PFace& pf);
/*
14.3.9 Operator ==

*/        
      bool operator ==(const PFace& pf)const;
    private:    
      Rectangle<2> getBoundingRec(const Point3D& point)const;   
      
      IntersectionSegment createIntSeg(const RationalPlane3D& planeSelf, 
                                       const RationalPlane3D& planeOther,
                                       const RationalSegment3D& intSeg); 
      IntersectionSegment createBorder(const RationalPlane3D &planeSelf,
                                       Border border);
/*
14.4 Attributes

*/      
      IntSegContainer   intSegContainer;
      State             state;
      Rectangle<2>      boundingRect;  
      Point3D           a;
      Point3D           b;
      Point3D           c;
      Point3D           d;            
    };// class PFace   
  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// PFACE_H 