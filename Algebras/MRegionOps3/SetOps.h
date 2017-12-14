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
Mai - November 2017, U. Wiesecke for master thesis.

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
    
    enum PredicateOp {
      INTERSECTS,
      INSIDE
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
6.3.6 isLeftSideInner   

*/       
      bool isLeftSideInner(const RationalSegment3D segment,
                           const RationalPlane3D other)const;
/*
6.3.7 transform

The method transforms a point from the three-dimensional space to the plane 
of the P-Face as a two-dimensional point.

*/                             
      RationalPoint2D transform(const RationalPoint3D& point) const;
      
/*
The method transforms a segment from the three-dimensional space to the plane 
of the P-Face as a two-dimensional segment.

*/    
      RationalSegment2D transform(const RationalSegment3D& segment) const;
/*
The method transforms all segments from three-dimensional space onto the plane 
of the P-Face as two-dimensional segments.

*/   
      void transform(const std::vector<RationalSegment3D>& segment3D,
                     std::vector<RationalSegment2D>& segment2D) const;
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
                          const Predicate& predicate = UNDEFINED);
      IntersectionSegment(const Segment3D& segment3D, 
                          const Segment2D& segment2D, 
                          const Predicate& predicate = UNDEFINED);
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
               const Predicate& predicate);
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
      virtual void first(double t1, double t2, Point3DContainer& points,
                                               SegmentContainer& segments,
                                               bool pFaceIsCritical);
/*
10.2.2 next

next step in the Plane Sweep.

*/          
      virtual void next(double t1, double t2, Point3DContainer& points, 
                                              SegmentContainer& segments,
                                              bool pFaceIsCritical);  
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
      void first(double t1, double t2, Point3DContainer& points,
                                       SegmentContainer& segments,
                                       bool pFaceIsCritical);
/*
11.3.8 next
    
*/  
      void next(double t1, double t2, Point3DContainer& points, 
                                      SegmentContainer& segments,
                                      bool pFaceIsCritical);

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
13.1 Constructor

*/   
      GlobalTimeValues(double scale = 1,
                       double orginalStartTime = 0,
                       double orginalEndTime = 1);
      
      GlobalTimeValues(const Interval<Instant>& orginalInterval);
/*
13.3 Methods, operators and predicates

13.3.1 addTimeValue

*/      
      void addTimeValue(double t);
      
/*
Method addStartAndEndtime

Inserts the start and end time of the time interval

*/    
      void addStartAndEndtime();
      
      void setScaleFactor(double scaleFactor);
      
      Interval<Instant> getOrginalInterval()const;
      
      Interval<Instant> getScaledInterval()const;
      
      double getOrginalStartTime()const;
    
      double getOrginalEndTime()const;
    
      double getScaledStartTime()const;
    
      double getScaledEndTime()const;
         
/*
13.3.2 size

*/      
      size_t size()const; 
      
      std::ostream& print(std::ostream& os, std::string prefix) const;
      
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
      bool scaledFirst(double& t1, double& t2);
      
      bool orginalFirst(double& t1, double& t2);
/*
13.3.2 next

*/        
      bool scaledNext(double& t1, double& t2); 
      
      bool orginalNext(double& t1, double& t2);    
      
      Interval<Instant> createInterval(double start, double end, 
                                       bool rightClosed = false) const;
      
    private:             
      

      double computeOrginalTimeValue(double scaledTimeValue)const;
/*
13.4 Attributes

*/      
      std::set<double, DoubleCompare> time;
      std::set<double, DoubleCompare>::const_iterator timeIter;
      double t1;
      double t2;
      double orginalT1;
      double orginalT2;      
      double orginalStartTime;
      double orginalEndTime;
      double scale;      
    }; // GlobalTimeValues   
/*
14 class PResultFace

*/ 
    class PResultFace{
    public:  
      PResultFace(); 
      
      PResultFace(const PResultFace& other); 
      
      PResultFace(const Segment3D& left, const Segment3D& right);  
           
      PResultFace(const Segment3D& left,  const Segment3D& right,
                  int faceno, int cycleno, int edgeno, bool leftDomPoint, 
                  bool insideAbove);
      
      PResultFace(const MSegmentData& mSeg, 
                  const GlobalTimeValues& timeValues);
                  
      int getFaceNo() const; 
      int getCycleNo() const;
      int getSegmentNo() const;       
      bool getInsideAbove() const;      
      Point3D getLeftStart() const;
      Point3D getLeftEnd() const;
      Point3D getRightStart() const;
      Point3D getRightEnd() const;
      HalfSegment getMedianHS() const;
      
      Rectangle<2> getBoundingRec()const;
      
      MSegmentData getMSegmentData() const;
      
      bool isLeftDomPoint() const;
      
      void setLeftDomPoint(bool ldp);
      
      void setSegmentNo(int sn);
      
      void copyIndicesFrom(const HalfSegment* hs);
/*
14.1.1 LessByMedianHS

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
according to the ~HalfSegment~ order, specified in the ~SpatialAlgebra~.

*/
      bool lessByMedianHS(const PResultFace& other) const; 
/*
14.1.1 LogicLess

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
similar to ~HalfSegment::LogicCompare~, specified in the ~SpatialAlgebra~.

*/
      bool logicLess(const PResultFace& other) const; 
/*
14.3.3 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
14.3.4 operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const PResultFace& prFace);
/*
14.3.5 operator ==
    
*/     
      bool operator ==(const PResultFace& other)const; 
/*
14.3.6 operator =
    
*/       
      PResultFace& operator =(const PResultFace& other);  
                  

    protected:
      
      void set(const PResultFace& other);

      void set(const Point3D leftStart, const Point3D leftEnd,
               const Point3D rightStart, const Point3D rightEnd);
      
      void createMedianHS();
      
      Rectangle<2> getBoundingRec(const Point3D& point)const; 
/*
14.4 Attributes

*/               
      Point3D leftStart;
      Point3D leftEnd;
      Point3D rightStart;
      Point3D rightEnd;      
      HalfSegment medianHS;  
      bool insideAbove;
      Rectangle<2> boundingRect;  
    }; // PResultFace  
/*

15 Class CriticalPResultFace

*/    
    class CriticalPResultFace {
    public:
      
      CriticalPResultFace();      
      CriticalPResultFace(const CriticalPResultFace& other);      
      CriticalPResultFace(const Segment3D& left, const Segment3D& right, 
                          SourceFlag source, Predicate predicate);   
      
      Point3D getMidPoint()const;
      
      Segment3D getLeft() const;
      
      Segment3D getRight() const;
      
      Predicate getPredicate()const;
      
      PResultFace getPResultFace()const;
      
      bool isPartOfUnitA() const;
      
      bool hasEqualNormalVector(const CriticalPResultFace& other) const;
      
/*
15.3.3 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
15.3.4 operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const CriticalPResultFace& cprFace);
/*
15.3.5 operator ==
    
*/     
      bool operator ==(const CriticalPResultFace& other)const; 
/*
15.3.6 operator =
    
*/       
      CriticalPResultFace& operator =(const CriticalPResultFace& other); 
      
     
      
      bool operator <(const CriticalPResultFace& other) const;
      
    private:
      
      void set(const CriticalPResultFace& other);
      
      void set(const Segment3D& left, const Segment3D& right, 
               SourceFlag source, Predicate predicate);
/*
15.4 Attributes

*/         
      SourceFlag       source;
      Predicate        predicate; 
      Segment3D        left;
      Segment3D        right;
      Point3D          midPoint;       
      RationalVector3D normalVector;
    };
    
/*
16 class ResultUnit

*/     
    class ResultUnit{
    public:
      ResultUnit();

      ResultUnit(double orginalStartTime,double orginalEndTime);
      
      ResultUnit(const ResultUnit& other);
      
      void addPResultFace(PResultFace& prFace, bool completely);
      
      void addPResultFace(const CriticalPResultFace& cprFace);
      
      void addCPResultFace(const CriticalPResultFace& cprFace);
      
      Interval<Instant> getTimeInterval() const;

      size_t size();
/*
16.3.3 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
16.3.4 operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const ResultUnit& unit);
/*
16.3.5 operator ==
    
*/     
      bool operator ==(const ResultUnit& unit) const; 
/*
16.3.6 operator =
    
*/       
      ResultUnit& operator = (const ResultUnit& unit);  

      void finalize();
      
      URegionEmb* convertToURegionEmb(DbArray<MSegmentData>* segments) const;
      
      void evaluateCriticalMSegmens(SetOp setOp);
            
    private:
      void set(const ResultUnit& other);
      
      static bool less(const PResultFace& prf1, const PResultFace& prf2);

      static bool logicLess(const PResultFace& prf1, const PResultFace& prf2); 
      
      void copyCriticalMSegmens(const CriticalPResultFace& cprf, SetOp setOp);
/*
16.4 Attributes

*/         
      std::vector<PResultFace> prFaces;
      std::vector<CriticalPResultFace> mCSegments;                
      double orginalStartTime;
      double orginalEndTime;      
    };// ResultUnit        
    
    class Layer {
    public:
/*
Constructor

*/
      Layer(bool iscritical = false);
      
      Layer(const Layer& layer); 
/*
Method addOrthSegment 
 
Adds an orthogonal segment to the layer

*/
      void addOrthSegment(const Segment& segment);
/*
Method addNonOrthSegment

Adds a non-orthogonal segment to layer

*/
      void addNonOrthSegment(const Segment& segment);  
/*
Method setPredicateFromPredeccor

Sets the predicate of the segment at the left boundary according 
to the information of the predecessor      

*/
      void setPredicateFromPredecessor(Predicate predicate);      
/*
Method setPredicateFromSuccessor

Sets the predicate of the segment at the left boundary according 
to the information of the successor

*/      
      void setPredicateFromSuccessor(Predicate predicate);
/* 
Method getPredicateForPredecessor     

Returns the predicate for the left segment of the successor

*/
      Predicate getPredicateForPredecessor()const;
/*  
Method getPredicateForSuccessor
 
Returns the predicate for the left segment of the predecessor
 
*/
      Predicate getPredicateForSuccessor()const;
/*
Method getBorderPredicate

Returns the predicates for the left and right border

*/
      void getBorderPredicates(Predicate& left, Predicate& right)const;
/*
Method setBorderPrediacte

sets the predicates for the left and right border

*/
      void setBorderPrediactes(Predicate left, Predicate right); 
/*
Method evaluate

Determines the predicates of all segments of a layer with the help 
of the intersegment segments

*/
      bool evaluate();

/*
Method print

Output the content into a stream

*/
      std::ostream& print(std::ostream& os,std::string prefix)const;
/*
Operator <<

*/      
      friend std::ostream& operator <<(std::ostream& os, const Layer& layer);
/*
Operator ==

Comparison operator, compare two objects for equality.

*/        
      bool operator ==(const Layer& layer)const;   
/*
Operator ==

Assignment operator

*/            
      Layer& operator =(const Layer& other);  
/*
Method intersects 
 
The method determines whether an intersection is within a layer. The result is
returned via the reference predicate. If an evaluation of the layer is possible,
the return value is "true"     

*/
      bool intersects(bool& predicate)const;
/*
Method inside

The method determines whether a layer is within the other region unit. 
The result is returned via the reference predicate. If an evaluation of the 
layer is possible, the return value is "true"

*/      
      bool inside(bool& predicate)const;
/*
 Method getResultUnit
 
 In the result unit, the moving segments of the layer are created and stored 
 which correspond to the desired predicate. Critical moving segments are 
 recorded seperately in the results unit.
 
*/
      void getResultUnit(Predicate soughtPredicate, bool reverse, 
                         const Point3DContainer& points, ResultUnit& unit,
                         SourceFlag source)const;        

    private:
/*
Method getBorderPredicate

Returns the predicate of the area bounded by the segment

*/
      Predicate getBorderPredicate(const Segment& segment, Border border)const;
/*
Method getAreaPredicate

Returns the predicate of the surface bounded by the left and right segments

*/
      Predicate getAreaPredicate(size_t left, size_t right, size_t orthogonal, 
                                 bool orthSegmentExist)const;      
/*
Method print

Outputs a vector into a stream

*/      
      void print(std::ostream& os, std::vector<size_t> values)const;
/*
Method set

Applies the attributes of another object of the same class

*/            
      void set(const Layer& layer);    
/*
Attributes

*/      
      // Attribute
      SegmentContainer     segments;
      std::vector<size_t>  orthSegments;
      std::vector<size_t>  nonOrthSegments;
      size_t               touchAbove;
      size_t               touchBelow;
      bool                 isCritical;      
    }; // class Layer
    
    
    
    class LayerContainer {
    public:
/*
Constructor

*/      
      LayerContainer(size_t size = 0, bool isCritcal = false);
      
      LayerContainer(const LayerContainer& other);
      
      LayerContainer(Point3DContainer& points,
                     GlobalTimeValues &timeValues,
                     PlaneSweepAccess &access,
                     bool isCritcal);
/*
Method addOrthSegment 
 
Add the orthogonal segment to the specified layer

*/
      void addOrthSegment(size_t layer, const Segment& segment);
/*
Method addNonOrthSegment

Add the non-orthogonal segment to the specified layer

*/
      void addNonOrthSegment(size_t layer, const Segment& segment); 
/*
Method print

Output the content into a stream

*/
      std::ostream& print(std::ostream& os,std::string prefix)const;
/*
Operator <<

*/      
      friend std::ostream& operator <<(std::ostream& os, 
                                       const LayerContainer& layerContainer);
/*
Method evaluate

In all layers of a P-Face, the predicates of the intersegment segments or 
boundaries are evaluated. If all layers of the P-PFace could be evaluated, 
the return value is "true".

*/
      bool evaluate();
/*
Method getBorderPredicates

Returns the predicate for the left and right boundary of the layers 
of a P-Faces.

*/
      void getBorderPredicates(Predicate& left, Predicate& right)const;
/*
Operator ==

Comparison operator, compare two objects for equality.


*/      
      bool operator ==(const LayerContainer& other)const;
/*
Operator =

Assignment operator

*/
      LayerContainer& operator =(const LayerContainer& other);  
/*
Method intersects

*/    
      bool intersects(std::vector<bool>& predicate)const;
/*
Method inside

*/      
      bool inside(std::vector<bool>& predicate)const; 
 
/*
Method getResultUnit
 
In the result unit, the moving segments of the given layer are created 
and stored, which correspond to the desired predicate. Critical moving 
segments are recorded seperately in the results unit.
 
*/      
      void getResultUnit(size_t layer, Predicate soughtPredicate,
        bool reverse, const Point3DContainer& points, 
        ResultUnit& unit, SourceFlag source)const;
      
    private:
/*
Method set

Applies the attributes of another object of the same class

*/      
      void set(const LayerContainer& other);      
/*
Attributes

*/      
      std::vector<Layer> layers;  
    
    };// LayerContainer
        
/*
18 Class PFace

*/    
    class PFace: public PResultFace, public PlaneSweepAccess {
    friend class Selftest;    
    public:
/*
18.1 Constructors

*/
      PFace(size_t left, size_t right,const Point3DContainer& points, 
            const SegmentContainer& segments);
      
      PFace(const MSegmentData& mSeg, const GlobalTimeValues& timeValues,
            Point3DContainer& points, 
            SegmentContainer& segments);
            
      PFace(const PFace& pf); 
/*
18.2 Setter and Getter methods

*/
      void set(const PFace& pf);
      void setState(State state);
      State getState() const;
/*
18.3 Methods, operators and predicates

18.3.1 existsIntSegs

*/
      bool existsIntSegs()const;
/*
18.3.2 print

*/        
      std::ostream& print(std::ostream& os, std::string prefix)const;     
/*
18.3.3 Operator <<
    
Print the object values to stream.

*/            
      friend std::ostream& operator <<(std::ostream& os, const PFace& pf);      
/*
18.3.4 intersection

Computes the intersection of this ~PFace~ with pf. 

*/
      bool intersectionOnPlane(PFace& other, 
                               const RationalPlane3D& planeSelf,
                               GlobalTimeValues &timeValues);

      bool intersection(PFace& other,GlobalTimeValues &timeValues);
/*
18.3.5 toString

*/      
      static std::string toString(State state);
/*
18.3.6 addIntSeg

*/       
      // für den Selbsttest
      void addIntSeg(const IntersectionSegment& seg);
      
      void addIntSeg(const RationalPlane3D &planeSelf, 
                     const RationalPlane3D &planeOther,
                     const RationalSegment3D &intSeg,
                     GlobalTimeValues &timeValues);
/*
18.3.7 Method addBorder

The method adds the left and right border of the P-Face to the 
intersection segments.Te predicate "Undefined" is used.

*/        
      void addBorder(const RationalPlane3D &plane,
                     Predicate predicate);
/*
The method adds the left and right border of the P-Face to the 
intersection segments. The predicates used are set to the predicates 
of the border segments of the P-Face.

*/    
      void addBorder(const SegmentContainer& segments, 
                     Predicate predicate);
/*
 Method setBorderPredicate
 
 The method sets the predicate of the border segment sof the P-Face 
 to the specified value.
 
 */
      void setBorderPredicate(SegmentContainer& segment, Predicate predicate);
/*
18.3.8 Operator =

*/  
      PFace& operator =(const PFace& pf);
/*
18.3.9 Operator ==

*/        
      bool operator ==(const PFace& pf)const;
/*
18.3.10 first
    
*/    
      void first(double t1, double t2, Point3DContainer& points,
                                       SegmentContainer& segments,
                                       bool pFaceIsCritical);
/*
18.3.11 next
    
*/     
      void next(double t1, double t2, Point3DContainer& points, 
                                      SegmentContainer& segments,
                                      bool pFaceIsCritical);
      
      bool finalize(Point3DContainer& points, SegmentContainer& segments, 
                    GlobalTimeValues& timeValues);
      
      bool intersects(Point3DContainer& points, GlobalTimeValues& timeValues,
                      std::vector<bool>& predicate);
      
      bool inside(Point3DContainer& points, GlobalTimeValues& timeValues,
                  std::vector<bool>& predicate);
      
      void getResultUnit(size_t slide, Predicate predicate, bool reverse, 
                         const Point3DContainer& points, ResultUnit& unit,
                         SourceFlag source);
    private:  
/*
Method getBorderSegments

The method determines all or only the segments for the left and right edges 
of a P-Face, depending on the boolean variable "all".

*/
      void getBorderSegments(bool all, std::vector<RationalSegment3D>& borders);
/*
Method intersection

The method calculates the segment of the intersection for the edge segments
of a P-Face and a segment. All segments are exists on the plane of the 
P-Face. If a section segment exists, the return value is "true".

*/      
      bool intersection(const std::vector<RationalSegment2D>& borders, 
                        const RationalSegment2D& segment, 
                        RationalSegment2D& result);
/*
Method map

The method transfers the points of a segment in the plane to a segment
in the (x, y, t) space. The points of the intersection must lie on the 
original segment of the plane. The segment on the plane must be an 
transformation of the segment in space.

*/        
      RationalSegment3D map(const RationalSegment3D& orginal3D, 
                            const RationalSegment2D& orginal2D, 
                            const RationalSegment2D& intersection);
/*
18.4.3 createBorder

Creates a segment for the boder of a P-Face. Whether it is the left or 
right edge is defined by the value of "border".

*/        
      IntersectionSegment createBorder(const RationalPlane3D &planeSelf,
                                       Border border, Predicate predicate);
/*
18.5 Attributes

*/      
      IntSegContainer    intSegs;
      LayerContainer     layers;
      State              state;
      size_t             left;
      size_t             right;  
    };// class PFace  
    
/*
19 Class FaceCycleInfo

*/
    class FaceCycleInfo {
    public:
      FaceCycleInfo();
      
      void   setFirstIndex(size_t firstIndex);
      void   setTouch();
      bool   getTouch() const;
      size_t getFirstIndex() const;
      
    private:
/*
19.5 Attributes

*/        
      size_t firstIndex;
      bool   touch;
    };
    
/*
20 class SourceUnit

*/        
    class SourceUnit{
    public:
/*
20.1 Constructor

*/       
      SourceUnit();
      SourceUnit(const SourceUnit& other);
/*
20.1 Destructor

*/       
      ~SourceUnit();
/*
20.2 Methods, operators and predicates

20.2.3 addPFace
    
*/      
      void addPFace(const Segment& left, const Segment& right, 
                    const Point3DContainer& points);
      
      void addMSegmentData(const MSegmentData& mSeg, 
                           const GlobalTimeValues& timeValues,
                           Point3DContainer& points);
      
      bool isEmpty()const;
      
      bool intersect(const SourceUnit& other)const;

      void addToResultUnit(ResultUnit& result)const;
      
/*
20.2.4 intersection
    
*/       
      void intersection(SourceUnit& other, GlobalTimeValues& timeValues); 
      
      void intersectionFast(SourceUnit& other, GlobalTimeValues& timeValues);
      
      bool finalize(Point3DContainer& points, GlobalTimeValues& timeValues,
                    Predicate predicateconst, SourceUnit& other);
      
      void intersects(Point3DContainer& points, GlobalTimeValues& timeValues,
                      SourceUnit& other, std::vector<bool>& predicate);
      
      void inside(Point3DContainer& points, GlobalTimeValues& timeValues,
                  SourceUnit& other, std::vector<bool>& predicate);
      
      void getResultUnit(size_t slide, Predicate predicate,bool reverse, 
                         const Point3DContainer& points, ResultUnit& unit,
                         SourceFlag source);
      
      std::ostream& print(std::ostream& os, std::string prefix)const;
/*
20.2.5 Operator <<
    
Print the object values to stream.

*/          
      friend std::ostream& operator <<(std::ostream& os, 
                                       const SourceUnit& unit);
/*
20.2.6 Operator ==  

*/       
      bool operator ==(const SourceUnit& unit)const; 
      
      SourceUnit& operator =(const SourceUnit& unit);
      
      void createTestRegion();
      
      bool isInside(const PFace* pFace);
            
      void createFaceCycleEntry(const PFace* pf, size_t index);
      
      void touchFaceCycleEntry(const PFace* pf);
      
      void checkFaceCycleEntrys(SourceUnit& other);
      
      void printFaceCycleEntrys();

      void reSort();      
           
    private: 
      
      static bool lessByMedianHS(const PFace* pf1, const PFace *pf2);
             
      static bool logicLess(const PFace* pf1, const PFace *pf2);
      
      void set(const SourceUnit& other);
/*
20.3 Attributes

*/       
      std::vector<PFace*> pFaces;
      mmrtree::RtreeT<2, size_t> pFaceTree; 
      SegmentContainer segments;
      std::vector<size_t> itersectedPFace;
      Region testRegion;
      bool   testRegionDefined;      
      std::vector<std::vector<FaceCycleInfo>> faceCycleInfo;            
    };// class SourceUnit 
/*
21 Class SourceUnitPair

*/ 
    class SourceUnitPair {
    public:
      SourceUnitPair(double orginalStartTime = 0,double orginalEndTime = 1, 
                     double scale = 1);
      
      SourceUnitPair(const Interval<Instant>& orginalInterval);
      
      void setScaleFactor(double scale);
                  
      void addPFace(SourceFlag flag, Segment3D& left, Segment3D& right);
      
      void addMSegmentData(const MSegmentData& mSeg, SourceFlag sourceFlag);
      
      bool operate(SetOp setOp);
      
      bool predicate(PredicateOp predicateOp);
      
      std::ostream& print(std::ostream& os, std::string prefix)const;
      
      friend std::ostream& operator <<(std::ostream& os, 
                                       const SourceUnitPair& unitPair);
      
      size_t countResultUnits()const;
      
      ResultUnit getResultUnit(size_t slide)const;
      
      void createResultMRegion( MRegion* resMRegion);
      
      void createResultMBool(MBool* resMBool, bool last );
      
      void createSourceUnit(const Interval<Instant>& interval, MRegion* mregion,
                            SourceFlag sourceFlag);

    private:      
/*
21.4 Attributes

*/
      SourceUnit unitA;
      SourceUnit unitB;
      std::vector<ResultUnit> result;
      std::vector<bool> predicates;      
      GlobalTimeValues timeValues;        
      Point3DContainer points; 
    };
/*
22 class SetOperator

*/
    class SetOperator {
    public:  

      SetOperator(MRegion* const _mRegionA, MRegion* const _mRegionB, 
                  MRegion* const _mRegionResult);
      
      void operate(SetOp setOp);    

    private: 
/*
22.4 Attributes

*/
      MRegion* const mRegionA;
      MRegion* const mRegionB;
      MRegion* const mRegionResult;
    }; // class SetOperator

/*
23 class SetOperator

*/
    class PredicateOperator {
    public:  

      PredicateOperator(MRegion* const _mRegionA, MRegion* const _mRegionB, 
                        MBool* const  _mBool);
      
      void operate(PredicateOp predicateOp);    

    private: 
/*
22.4 Attributes

*/
      MRegion* const mRegionA;
      MRegion* const mRegionB;
      MBool*   const mBool;
    }; // class PredicateOperator    

  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// SETOPS_H