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
#include "DateTime.h"
#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"

#include <gmp.h>
#include <gmpxx.h>
#include <set>
#include <vector>
#include <list>
#include <string>

#ifndef SETOPS_H
#define SETOPS_H

/*
2 Enumeration SetOp

Indicates the kind of set operation.

*/
namespace temporalalgebra {
  namespace mregionops3 {
    class PFace;
/*
3 Enumeration 

3.1 SourceFlag

Indicates the source unit of a ~RationalPoint3DExt~.

*/
    enum SourceFlag {
      UNIT_A,
      UNIT_B
    };
/*
3.2 Enumeration State

Used in the class ~PFace~ to indicate it's current state.

*/
    enum State {
      UNKNOWN,
      RELEVANT,
      CRITICAL,
      NOT_RELEVANT
    };
/*
3.3 Enumeration Border

*/        
    enum Border {
      LEFT,
      RIGHT
    };
    
    enum SetOp {
      INTERSECTION,
      UNION,
      MINUS
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
5.2 Methods, operators and predicates

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
      bool getIntersectionSegment(RationalSegment3D& result) const;
/*
5.2.4 Operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPoint3DExtSet& points);
/*
5.2.5 print

*/      
      std::ostream& print(std::ostream& os, std::string prefix)const;
      
    private:
/*
5.3 Attributes

5.3.1 points

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
6.2 Getter methods

*/      
      RationalPoint3D getPointOnPlane() const;
      RationalVector3D getNormalVector() const;
/*
6.3 Methods, operators and predicates

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
6.3.3 isCoplanarTo

Returns ~true~, if this ~RationalPlane3D~ is coplanar to the argument.

*/      
      bool isCoplanarTo(const RationalPlane3D& plane)const;
/*
6.3.4 intersection


*/           
      bool intersection(const Segment3D segment, RationalPoint3D& result)const;
      void intersection(const PFace& other, SourceFlag sourceFlag, 
                        RationalPoint3DExtSet& intPointSet)const;
/*
7.3.4 operator =

*/                                     
      RationalPlane3D& operator =(const RationalPlane3D& point);       
/*
6.3.5 Operator <<
    
Print the object values to stream.

*/         
      friend std::ostream& operator <<(std::ostream& os, 
                                       const RationalPlane3D& plane);
/*
6.3.6 isLeftAreaInner   

*/       
      bool isLeftAreaInner(const RationalSegment3D segment,
                           const RationalPlane3D other)const;
/*
6.3.7 transform   

*/                              
      Point2D transform(const RationalPoint3D& point) const;
      Segment2D transform(const RationalSegment3D& segment) const;
    private:
/*
6.4 Private methods

6.4.1 set

*/       
      void set(const RationalVector3D& normalVector,
               const RationalPoint3D& pointOnPlane);
      void set(const RationalPlane3D& plane);       
/*
6.5 Attributes

*/     
      RationalVector3D  normalVector;
      RationalVector3D  wVector;
      RationalPoint3D   pointOnPlane;
    };
/*
7 Class IntersectionPoint

*/       
    class IntersectionPoint {
    public:  
/*
7.1 Constructors

*/        
     IntersectionPoint();
     IntersectionPoint(const IntersectionPoint& point);
     IntersectionPoint(const Point3D& point3D, const Point2D& point2D); 
     IntersectionPoint(double x, double y, double z, double w);
/*
7.2 Getter methods

*/        
      Point3D getPoint3D() const;
      Point2D getPoint2D() const;
      double getX()const;
      double getY()const;
      double getZ()const;
      double getW()const;
      double getT()const;
      Rectangle<3> getBoundingBox()const;
/*
7.3 Methods, operators and predicates

7.3.1 Operator <<
    
Print the object values to stream.

*/        
      friend std::ostream& operator <<(std::ostream& os, 
                                       const IntersectionPoint& point);
/*
7.3.2 operator =
    
*/
      IntersectionPoint& operator =(const IntersectionPoint& point);  
/*
7.3.3 operator ==
    
*/      
      bool operator ==(const IntersectionPoint& point) const;
    private:  
/*
7.4 Private methods

7.4.1 set

*/        
      void set(const IntersectionPoint& point);
/*
7.5 Attributes

*/       
      double x;
      double y;
      double z;
      double w;
    };
/*
8 Class IntersectionSegment

*/      
    class IntersectionSegment{
    public:
/*
8.1 Constructors

*/         
      IntersectionSegment();
      IntersectionSegment(const IntersectionSegment& segment);
      IntersectionSegment(const IntersectionPoint& tail,
                          const IntersectionPoint& head, 
                          const Predicate predicate);
      IntersectionSegment(const Segment3D& segment3D, 
                          const Segment2D& segment2D, 
                          const Predicate predicate);
/*
8.2 Getter methods

*/        
      Segment3D getSegment3D()const;
      Segment2D getSegment2D()const;
      IntersectionPoint getTail() const;
      IntersectionPoint getHead() const;
      Predicate getPredicate()const;
/*
8.3 Methods, operators and predicates

8.3.1 Operator <<
    
Print the object values to stream.

*/       
      friend std::ostream& operator <<(std::ostream& os, 
                                       const IntersectionSegment& segment);
/*
8.3.2 operator =
    
*/
      IntersectionSegment& operator =(const IntersectionSegment& segment);
/*
8.3.3 operator ==
    
*/      
      bool operator ==( const IntersectionSegment& segment) const;
/*
8.3.5 isOrthogonalToTAxis

*/      
      bool isOrthogonalToTAxis()const;  
/*
8.3.6 isOutOfRange

*/       
      bool isOutOfRange(double t) const;
/*      
8.3.7 isLeftOf

*/      
      bool isLeftOf(const IntersectionSegment& intSeg) const;
/*
8.3.8 evaluate

*/       
      Point3D evaluate(double t) const;     
    private: 
/*
8.4 Private methods

8.4.1 set

*/       
      void set(const IntersectionPoint& tail,const IntersectionPoint& head,
               Predicate predicate);
      void set(const IntersectionSegment& segment);
/*
8.6 Attributes
 
*/
      IntersectionPoint tail;
      IntersectionPoint head;
      Predicate predicate;  
    };

/*
9 Struct IntSegCompare

This struct implements the ~IntersectionSegment~ order,
used in ~IntSegContainer~.

*/
    struct IntSegCompare {

      bool operator()(const IntersectionSegment* const& s1,
                      const IntersectionSegment* const& s2) const;
    };    
/*
10 Class PlaneSweepAccess

*/     
    class PlaneSweepAccess {
    public:
/*
10.1 Destructor

*/         
      virtual ~PlaneSweepAccess();
/*
10.2 Methods and Operators

10.2.1 first

first step in the Plane Sweep.

*/       
      virtual void first(double t1, double t2, ContainerPoint3D& points,
                                       ContainerSegment& segments);
/*
10.2.2 next

next step in the Plane Sweep.

*/          
      virtual void next(double t1, double t2, ContainerPoint3D& points, 
                                      ContainerSegment& segments);  
    };// PlaneSweepAccess 
/*    
11 Class IntSegContainer

This class is used by the class ~PFace~ and provides essentially
an ordered set of ~IntersectionSegments~

*/
    class IntSegContainer: public PlaneSweepAccess{
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
/*
11.3 Methods and Operators

11.3.1 addIntSeg

Adds seg to the set of ~IntersectionSegments~.

*/  
      void addIntSeg(const IntersectionSegment& seg);
/*
11.3.2 size

Adds seg to the set of ~IntersectionSegments~.

*/        
      size_t size()const;   
/*
11.3.3 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
11.3.4 operator <<
    
*/        
      friend std::ostream& operator <<(std::ostream& os, 
                                       const IntSegContainer& container);
/*
11.3.5 operator ==
    
*/     
      bool operator ==(const IntSegContainer& container)const; 
/*
11.3.6 operator =
    
*/       
      IntSegContainer& operator =(const IntSegContainer& container);  
/*
11.3.7 first
    
*/       
      void first(double t1, double t2, ContainerPoint3D& points,
                                       ContainerSegment& segments);
/*
11.3.8 next
    
*/  
      void next(double t1, double t2, ContainerPoint3D& points, 
                                      ContainerSegment& segments);

    private:
/*
11.4 Private methods

11.4.1 hasMoreSegsToInsert

*/        
      bool hasMoreSegsToInsert(double t) const; 
/*
11.4.2 set
    
*/                 
      void set(const IntSegContainer& container);
/*
11.5 Attributes

11.5.1 intSegs, intSegIter

A ~std::set~ to store the ~IntersectionSegments~ using the order
provided by ~IntSegCompare~ and a suitable iterator.

*/
      std::set<IntersectionSegment*, IntSegCompare> intSegs;
      std::set<IntersectionSegment*, IntSegCompare>::iterator intSegIter;
/*
11.5.2 orthogonal, active

A ~std::list~ to store the active ~IntersectionSegments~ and the a
orthogonal ~IntersectionSegments~during the plane-sweep.

*/
      std::list<IntersectionSegment*> orthogonal;
      std::list<IntersectionSegment*> active;   
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
11.1 Constructor

*/   
      GlobalTimeValues();
/*
13.3 Methods, operators and predicates

13.3.1 addTimeValue

*/      
      void addTimeValue(double t);
/*
13.3.2 size

*/      
      size_t size()const; 
      
/*
13.3.3 Operator <<

*/       
      friend std::ostream& operator <<(std::ostream& os, 
                                       const GlobalTimeValues& timeValues);
/*
13.3.4 Operator ==

*/                                        
      bool operator ==(const GlobalTimeValues& other)const; 
/*
13.3.2 first

*/        
      bool first(double& t);
/*
13.3.2 next

*/        
      bool next(double& t);           
    private: 
/*
13.4 Attributes

*/      
      std::set<double, DoubleCompare> time;
      std::set<double, DoubleCompare>::const_iterator timeIter;
      double t1;
      double t2;
    }; // GlobalTimeValues   
/*
4 class MSegment

*/ 
    class MSegment{
    public:  
      MSegment(); 
      
      MSegment(const MSegment& msegment); 
      
      MSegment(const Segment3D& left, const Segment3D& right);  
           
      MSegment(const Segment3D& left,  const Segment3D& right,
               int faceno, int cycleno, int edgeno, bool leftDomPoint, 
               bool insideAbove);
      
      MSegment(const MSegmentData& mSeg, const Interval<Instant>& interval);
                  
      int getFaceNo() const; 
      int getCycleNo() const;
      int getSegmentNo() const;       
      bool getInsideAbove() const;      
      Point3D getLeftStart() const;
      Point3D getLeftEnd() const;
      Point3D getRightStart() const;
      Point3D getRightEnd() const;
      HalfSegment getMedianHS() const;
      
      bool isLeftDomPoint() const;
      
      void setLeftDomPoint(bool ldp);
      void setSegmentNo(int sn);
      
      void copyIndicesFrom(const HalfSegment* hs);
/*
1.1.1 LessByMedianHS

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
according to the ~HalfSegment~ order, specified in the ~SpatialAlgebra~.

*/
      bool lessByMedianHS(const MSegment& other) const; 
/*
1.1.1 LogicLess

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
similar to ~HalfSegment::LogicCompare~, specified in the ~SpatialAlgebra~.

*/
      bool logicLess(const MSegment& other) const; 
/*
11.3.3 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
11.3.4 operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const MSegment& mSegment);
/*
11.3.5 operator ==
    
*/     
      bool operator ==(const MSegment& mSegment)const; 
/*
11.3.6 operator =
    
*/       
      MSegment& operator =(const MSegment& mSegment);  
                  

    protected:
      
      void set(const MSegment& msegment);

      void set(const Point3D leftStart, const Point3D leftEnd,
               const Point3D rightStart, const Point3D rightEnd);
      
      void createMedianHS();
      
      Point3D leftStart;
      Point3D leftEnd;
      Point3D rightStart;
      Point3D rightEnd;
      
      HalfSegment medianHS;  
      bool insideAbove;
    }; // MSegment
    
    
    class CriticalMSegment {
    public:
      
      CriticalMSegment();      
      CriticalMSegment(const CriticalMSegment& cmsegment);      
      CriticalMSegment(const Segment3D& left, const Segment3D& right, 
                       SourceFlag source, Predicate predicate);   
      
      Point3D getMidPoint()const;
      
      Predicate getPredicate()const;
      
      MSegment getMSegment()const;
      
      bool isPartOfUnitA() const;
      
      bool hasEqualNormalVector(const CriticalMSegment& other) const;
      
/*
11.3.3 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
11.3.4 operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const CriticalMSegment& cmSegment);
/*
11.3.5 operator ==
    
*/     
      bool operator ==(const CriticalMSegment& cmSegment)const; 
/*
11.3.6 operator =
    
*/       
      CriticalMSegment& operator =(const CriticalMSegment& cmSegment); 
      
     
      
      bool operator <(const CriticalMSegment& cmSegment) const;
      
    private:
      
      void set(const CriticalMSegment& cmsegment);
      
      void set(const Segment3D& left, const Segment3D& right, 
               SourceFlag source, Predicate predicate);
      
      SourceFlag       source;
      Predicate        predicate; 
      Segment3D        left;
      Segment3D        right;
      Point3D          midPoint;       
      RationalVector3D normalVector;
    };
    
/*
4 class ResultUnit

*/     
    class ResultUnit{
    public:
      ResultUnit();

      ResultUnit(double startTime,double endTime);
      
      ResultUnit(const ResultUnit& other);
      
      void addMSegment(MSegment& mSegment, bool completely );
      
      void addMSegment(const CriticalMSegment& mSegment);
      
      void addCMSegment(const CriticalMSegment& mSegment);
      
      Interval<Instant> getTimeInterval() const;

      size_t size();
/*
11.3.3 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
11.3.4 operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const ResultUnit& unit);
/*
11.3.5 operator ==
    
*/     
      bool operator ==(const ResultUnit& unit)const; 
/*
11.3.6 operator =
    
*/       
      ResultUnit& operator =(const ResultUnit& unit);  

      void finalize();
      
      URegionEmb* convertToURegionEmb(DbArray<MSegmentData>* segments)const;
      
//     Region* createTestRegion();
      
      void evaluateCriticalMSegmens(SetOp setOp);
            
    private:
      void set(const ResultUnit& other);
      
      static bool less(const MSegment& ms1, const MSegment& ms2);

      static bool logicLess(const MSegment& ms1, const MSegment& ms2); 
      
      void copyCriticalMSegmens(const CriticalMSegment& cmSeg0, SetOp setOp);
      
      std::vector<MSegment> mSegments;
      std::vector<CriticalMSegment> mCSegments;
      
      
      int index;
      double startTime;
      double endTime;
      
    };// ResultUnit        
/*
4 class ResultPfaceFactory

*/      
    class ResultPfaceFactory {
    public:  
/*
4.1 Constructor

*/        
      ResultPfaceFactory(const ResultPfaceFactory& other);
       
      ResultPfaceFactory();
       
      ResultPfaceFactory(size_t size);
 
      ResultPfaceFactory(ContainerPoint3D& points,
                         GlobalTimeValues &timeValues,
                         PlaneSweepAccess &access);    
/*
4.3 Methods, Operators and Predicates

*/
      ResultPfaceFactory& operator =(const ResultPfaceFactory& other);

      void addNonOrthogonalEdges(size_t slide, const Segment& segment);
      
      void addOrthogonalEdges(size_t slide,const Segment& segment);
      
      void setTouchsOnLeftBorder(size_t slide, size_t value);
      
      bool operator ==(const ResultPfaceFactory& factory)const;
      
      void getBorderPredicates(Predicate& left, Predicate& right)const;
   
      void evaluate();
      
      void getResultUnit(size_t slide, Predicate predicate, bool reverse, 
                         const ContainerPoint3D& points, ResultUnit& unit, 
                         State pfState, SourceFlag source);
/*
4.3.2 operator <<

*/     
      friend std::ostream& operator <<(std::ostream& os, 
                                       const ResultPfaceFactory& unit);
/*
4.3.3 print

*/     
      std::ostream& print(std::ostream& os,std::string prefix)const;

    private: 
      
      void inittialize(size_t size);
      
      void set(      std::vector<std::list<size_t>>& edges1,
               const std::vector<std::list<size_t>>& edges2);
       
      void set(const ResultPfaceFactory& other);
      
      void print(std::ostream& os,std::string prefix,
                 std::vector<std::list<size_t>> edges)const;
                 
      bool iSEqual(const std::vector<std::list<size_t>>& edges1,
                   const std::vector<std::list<size_t>>& edges2)const;
      
      void setPredicate(size_t index, Predicate& predicate);
      
      Predicate createPredicate(const Predicate source,
                                const Border border)const;
      
      void checkPredicate(const Segment& segment, const Border border,
                          Predicate& result)const;
      
      void evaluate(size_t i);
                          
      void setSlidePredicates(Predicate predicate,size_t i, size_t touch);
/*
4.4 Attributes

*/    
      ContainerSegment segments;
      std::vector<std::list<size_t>> nonOrthogonalEdges;
      std::vector<std::list<size_t>> orthogonalEdges;
      std::vector<size_t> touchsOnLeftBorder;
    }; // ResultPfaceFactory 
 
/*
14 Class PFace

*/    
    class PFace: public MSegment, public PlaneSweepAccess {
    friend class Selftest;    
    public:
/*
14.1 Constructors

*/
      PFace(size_t left, size_t right,const ContainerPoint3D& points, 
            const ContainerSegment& segments, size_t index = -1);
      
      PFace(const MSegmentData& mSeg, const Interval<Instant>& interval,
            ContainerPoint3D& points, ContainerSegment& segments);
            
      PFace(const PFace& pf); 
/*
14.2 Setter and Getter methods

*/
      void    set(const PFace& pf);
      void    setState(State state);
/*      Point3D getLeftStart() const; 
      Point3D getLeftEnd() const;
      Point3D getRightStart() const;       
      Point3D getRightEnd() const;*/ 
      State   getState() const;
      Rectangle<2> getBoundingRec()const;
/*
14.3 Methods, operators and predicates

14.3.1 existsIntSegs

*/
      bool existsIntSegs()const;
/*
14.3.2 print

*/        
      std::ostream& print(std::ostream& os, std::string prefix)const;
//      std::ostream& printShort(std::ostream& os, std::string prefix)const;
      
/*
14.3.3 Operator <<
    
Print the object values to stream.

*/            
      friend std::ostream& operator <<(std::ostream& os, const PFace& pf);      
/*
14.3.4 intersection

Computes the intersection of this ~PFace~ with pf. 

*/
      bool intersection(PFace& other,GlobalTimeValues &timeValues);
/*
14.3.5 toString

*/      
      static std::string toString(State state);
/*
14.3.6 addIntSeg

*/       
      // für den Selbsttest
      void addIntSeg(const IntersectionSegment& seg);
      
      void addIntSeg(const RationalPlane3D &planeSelf, 
                     const RationalPlane3D &planeOther,
                     const RationalSegment3D &intSeg,
                     GlobalTimeValues &timeValues);
/*
14.3.7 addBorder

*/        
      void addBorder(const RationalPlane3D &plane,
                     GlobalTimeValues &timeValues,
                     Predicate predicate);
      
      void addBorder(GlobalTimeValues &timeValues);
      
      void addBorder(GlobalTimeValues &timeValues, 
                     const ContainerSegment& segments, 
                     Predicate predicate);
      
      void setBorderPredicate(ContainerSegment& segment, Predicate predicate);
/*
14.3.8 Operator =

*/  
      PFace& operator =(const PFace& pf);
/*
14.3.9 Operator ==

*/        
      bool operator ==(const PFace& pf)const;
/*
14.3.10 first
    
*/    
      void first(double t1, double t2, ContainerPoint3D& points,
                                       ContainerSegment& segments);
/*
14.3.11 next
    
*/     
      void next(double t1, double t2, ContainerPoint3D& points, 
                                      ContainerSegment& segments);
      
      bool finalize(ContainerPoint3D& points, ContainerSegment& segments, 
                    GlobalTimeValues& timeValues);
      
      void getResultUnit(size_t slide, Predicate predicate, bool reverse, 
                         const ContainerPoint3D& points, ResultUnit& unit,
                         SourceFlag source);
           
      
            
//       HalfSegment getMedianHS() const;
    private:  
/*
14.4 Private methods

14.4.1 getBoundingRec

*/
      Rectangle<2> getBoundingRec(const Point3D& point)const;    
/*
14.4.3 createBorder

*/        
      IntersectionSegment createBorder(const RationalPlane3D &planeSelf,
                                       Border border, Predicate predicate);
/*
14.5 Attributes

*/      
      IntSegContainer    intSegs;
      ResultPfaceFactory factory;
      State              state;
      Rectangle<2>       boundingRect;   
//       Point3D            leftStart;
//       Point3D            leftEnd;
//       Point3D            rightStart;
//       Point3D            rightEnd;
      size_t             left;
      size_t             right;  
//    int                index;
    };// class PFace  
    

    class FaceCycleInfo {
    public:
      FaceCycleInfo();
      
      void   setFirstIndex(size_t firstIndex);
      void   setTouch();
      bool   getTouch() const;
      size_t getFirstIndex() const;
      
    private:
      size_t firstIndex;
      bool   touch;
    };
    
    
/*
5 class SourceUnit

*/        
    class SourceUnit{
    public:
/*
5.1 Constructor

*/       
      SourceUnit();
      SourceUnit(const SourceUnit& other);
/*
5.1 Destructor

*/       
      ~SourceUnit();
/*
5.2 Methods, operators and predicates

5.2.3 addPFace
    
*/        

      void addPFace(const Segment& left, const Segment& right, 
                    const ContainerPoint3D& points);
      
      void addMSegmentData(const MSegmentData& mSeg, 
                           const Interval<Instant>& interval,
                           ContainerPoint3D& points);
      
      bool isEmpty()const;
      
      bool intersect(const SourceUnit& other)const;
      
      bool createResultUnit(ResultUnit& result)const;
      
      void addToResultUnit(ResultUnit& result)const;
      
/*
5.2.4 intersection
    
*/       
      bool intersection(SourceUnit& other, GlobalTimeValues& timeValues); 
      
      bool finalize(ContainerPoint3D& points, GlobalTimeValues& timeValues,
                    Predicate predicate);
      
      void getResultUnit(size_t slide, Predicate predicate,bool reverse, 
                         const ContainerPoint3D& points, ResultUnit& unit,
                         SourceFlag source);
      
      std::ostream& print(std::ostream& os, std::string prefix)const;
/*
5.2.5 Operator <<
    
Print the object values to stream.

*/          
      friend std::ostream& operator <<(std::ostream& os, 
                                       const SourceUnit& unit);
/*
5.2.6 Operator ==  

*/       
      bool operator ==(const SourceUnit& unit)const; 
      
      SourceUnit& operator =(const SourceUnit& unit);
      
      void createTestRegion();
      
      bool isInside(const PFace* pFace)const;
      
      
      void createFaceCycleEntry(const PFace* pf, size_t index);
      
      void touchFaceCycleEntry(const PFace* pf);
      
      void checkFaceCycleEntrys(SourceUnit& other);
      
      void printFaceCycleEntrys();
      
      
      
      
      // wird nur im Rahmen des Selbsttestes verwendet
      void reSort();      
           
    private: 
      
      static bool lessByMedianHS(const PFace* pf1, const PFace *pf2);
             
      static bool logicLess(const PFace* pf1, const PFace *pf2);
      
        
      void set(const SourceUnit& other);
/*
5.3 Attributes

*/       
      std::vector<size_t> itersectedPFace;
      std::vector<PFace*> pFaces;
      ContainerSegment segments;
      mmrtree::RtreeT<2, size_t> pFaceTree;
      
      Region testRegion;
      bool   testRegionDefined;
      
      std::vector<std::vector<FaceCycleInfo>> faceCycleInfo;
      
    };// class SourceUnit 
/*
class SourceUnitPair

*/
    
    class SourceUnitPair {
    public:
      SourceUnitPair();
      
      void addPFace(SourceFlag flag, Segment3D& left, Segment3D& right);
      
      void addMSegmentData(const MSegmentData& mSeg, 
                           const Interval<Instant>& interval,
                           SourceFlag sourceFlag);
      
      bool operate(SetOp setOp);
      
      std::ostream& print(std::ostream& os, std::string prefix)const;
      
      friend std::ostream& operator <<(std::ostream& os, 
                                       const SourceUnitPair& unitPair);
      
      size_t countResultUnits()const;
      
      ResultUnit getResultUnit(size_t slide)const;
      
      void createResultMRegion(MRegion* resMRegion);

    private:      

      SourceUnit unitA;
      SourceUnit unitB;
      std::vector<ResultUnit> result;
      GlobalTimeValues timeValues;  
      ContainerPoint3D points; 
    };

    class SetOperator {
    public:  
      SetOperator(MRegion* const _mRegionA, MRegion* const _mRegionB, 
                  MRegion* const _mRegionResult):
          mRegionA(_mRegionA), 
          mRegionB(_mRegionB), 
          mRegionResult(_mRegionResult) {
      }// Konstruktor
      
      void operate(SetOp setOp);    

    private:
      
      void createSourceUnit(const Interval<Instant>& interval, 
                            MRegion* mregion,
                            SourceFlag sourceFlag, 
                            SourceUnitPair& unitPair);      
      
      MRegion* const mRegionA;
      MRegion* const mRegionB;
      MRegion* const mRegionResult;
    }; // class SetOperator

  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// SETOPS_H