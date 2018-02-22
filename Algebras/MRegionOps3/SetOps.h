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

April - November 2008, M. H[oe]ger for bachelor thesis.
Mai - November 2017, U. Wiesecke for master thesis.

[TOC]

1 Introduction

This file essentially contains the definitions of several classes which
provides the core functionality of the three set operators
~intersection~, ~union~ and ~minus~ with the signature \\
mregion [x] mregion [->] mregion \\
used in the ~MovingRegionAlgebra~.

2 Defines and Includes

*/
#include "PointVectorSegment.h"
#include "NumericUtil.h"
#include "DateTime.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/MovingRegion/MovingRegionAlgebra.h"

#include <gmp.h>
#include <gmpxx.h>
#include <set>
#include <vector>
#include <list>
#include <string>

#ifndef SETOPS_H
#define SETOPS_H


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

Used in the class ~PFace~ to indicate the border.

*/        
    enum Border {
      LEFT,
      RIGHT
    };
/*
3.4 Enumeration SetOp

Indicates the kind of set operation.

*/    
    enum SetOp {
      INTERSECTION,
      UNION,
      MINUS
    };
/*
3.5 Enumeration SetOp

Indicates the kind of predicate operation.

*/    
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

The class implements operations for planes in the (x, y, t)-Space.
Here, methods and operators were realized that support the intersection 
of two P-Faces.

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

The method determines the intersection of a segment in the (x, y, t)-
Space with the plane of a p-face.

*/           
      bool intersection(const Segment3D segment, RationalPoint3D& result)const;
/*
The method determines the intersections of another P-Face in the (x, y, t)-
space with the plane of a P-Face.

*/      
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

The method determines, for a directed intersection segment, whether the left 
side of the segment is inside the plane of the other P-Face.

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

6.4.1 Setter methods

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

The class implements the representation of a point in the (x, y, t, w)-space.
The coordinates of the point include its position in the (x, y, t)-space and 
the local coordinates that the point has on the surface of a P-Face in the 
(w, t)-Space. The specified point belongs to a intersection segment associated 
with a P-Face. The position of the point in the (w, t) coordinate system is 
needed for sorting the intersection segments and in the (x, y, t) coordinate 
system for performing the plane sweep.

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

The class implements the representation of a intersection segment with start 
and end points in (x, y, t, w) space. The coordinate components for the 
surface always refer to a certain P-Face. A intersection segment also has 
a predicate that specifies more information about the intersection segment.

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

Returns ~true~, if this is parallel to the xy-plane.

*/      
      bool isOrthogonalToTAxis()const;  
/*
8.3.6 isOutOfRange

Returns ~true~, if this is out of the t-value.

*/       
      bool isOutOfRange(double t) const;
/*      
8.3.7 isLeftOf

Returns ~true~, if this is left of intSeg.

*/      
      bool isLeftOf(const IntersectionSegment& intSeg) const;
      
/*
8.3.8 evaluate

Returns the point (in xyt-coords) on this segment with t-coord t.

*/       
      Point3D evaluate(double t) const;     
    private: 
/*
8.4 Private methods

8.4.1 Setter methods

*/       
      void set(const IntersectionPoint& tail,const IntersectionPoint& head,
               const Predicate& predicate);
      void set(const IntersectionSegment& segment);
/*
8.5 Attributes
 
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

The class defines an interface for methods of the class Selftest.

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

This class implemented a container object in which intersection segments
are stored. The saved intersection segments are sorted.
Same intersection segments are not recorded multiple times. 
The class provides all the methods needed for the plane sweep to 
form the segments of the PResultFace.

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

The method determines all segments in decomposed form for the first step 
during the plane sweep.
    
*/       
      void first(double t1, double t2, Point3DContainer& points,
                                       SegmentContainer& segments,
                                       bool pFaceIsCritical);
/*
11.3.8 next

The method determines all segments in decomposed form for all further steps 
during the plane sweep.
    
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

The structure supports the class GlobalTimeValues in sorting the 
time instants with structure changes.

*/    
    struct DoubleCompare {
      bool operator()(const double& d1, const double& d2) const;
    };
/*
13 class GlobalTimeValues

All times at which structural changes take place are recorded in an object
of this class. These time values are retrieved during the plan sweep 
to determine the segments for the result P-Fcae.

This class is also used to support scaling of the t-axis using appropriate
methods. There are methods for this which, in addition to the scaled time 
values, can also determine and process the original time values.

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
13.2 Getter and setter methods

*/        
      void setScaleFactor(double scaleFactor);
      Interval<Instant> getOrginalInterval()const;        
      Interval<Instant> getScaledInterval()const;        
      double getOrginalStartTime()const;     
      double getOrginalEndTime()const;     
      double getScaledStartTime()const;       
      double getScaledEndTime()const;
/*
13.3 Methods, operators and predicates

13.3.1 Method addTimeValue

*/      
      void addTimeValue(double t);
      
/*
13.2.2 addStartAndEndtime

Inserts the start and end time of the time interval

*/    
      void addStartAndEndtime();        
/*
13.3.3 size

*/      
      size_t size()const; 
/*
13.3.4 print 

*/          
      std::ostream& print(std::ostream& os, std::string prefix) const;
      
/*
13.3.5 Operator <<

*/       
      friend std::ostream& operator <<(std::ostream& os, 
                                       const GlobalTimeValues& timeValues);
/*
13.3.6 Operator ==

*/                                        
      bool operator ==(const GlobalTimeValues& other)const; 
/*
13.3.7 scaledFirst

*/        
      bool scaledFirst(double& t1, double& t2);
/*
13.3.8 orginalFirst 

*/          
      bool orginalFirst(double& t1, double& t2);
/*
13.3.9 scaledNext

*/        
      bool scaledNext(double& t1, double& t2); 
/*
13.3.10 orginalNext 

*/          
      bool orginalNext(double& t1, double& t2);    
/*
13.3.11 createInterval 

*/          
      Interval<Instant> createInterval(double start, double end, 
                                       bool lc = true, bool rc = false) const;
    private:             
/*
13.4 Private methods

13.4.1 computeOrginalTimeValue 

*/         

      double computeOrginalTimeValue(double scaledTimeValue)const;
/*
13.5 Attributes

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

This class implements the representation of the result P-Face. In contrast to
the P-Faces, the result  P-Faces are not stored in decomposed form (segments, 
points). Objects of these classes are formed from the layers of P-Faces if 
they belong to the result. This class provides methods that support the 
creation of the results region unit.

*/ 
    class PResultFace{
    public: 
/*      
14.1 Constructors    

*/
      PResultFace(); 
      PResultFace(const PResultFace& other); 
      PResultFace(const Segment3D& left, const Segment3D& right);     
      PResultFace(const Segment3D& left,  const Segment3D& right,
                  int faceno, int cycleno, int edgeno, bool leftDomPoint, 
                  bool insideAbove);
      PResultFace(const MSegmentData& mSeg, 
                  const GlobalTimeValues& timeValues);
/*
14.2 Getter and setter methods

*/                    
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
/*
14.3 Methods, operators and predicates

14.3.1 copyIndicesFrom

*/       
      void copyIndicesFrom(const HalfSegment* hs);
/*
14.3.2 LessByMedianHS

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
according to the ~HalfSegment~ order, specified in the ~SpatialAlgebra~.

*/
      bool lessByMedianHS(const PResultFace& other) const; 
/*
14.3.3 LogicLess

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
similar to ~HalfSegment::LogicCompare~, specified in the ~SpatialAlgebra~.

*/
      bool logicLess(const PResultFace& other) const; 
/*
14.3.4 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
14.3.5 operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const PResultFace& prFace);
/*
14.3.6 operator ==
    
*/     
      bool operator ==(const PResultFace& other)const; 
/*
14.3.7 operator =
    
*/       
      PResultFace& operator =(const PResultFace& other);  
                  
    protected:
/*
14.4 Private methods

14.4.1 Setter Methods

*/      
      void set(const PResultFace& other);
      void set(const Point3D leftStart, const Point3D leftEnd,
               const Point3D rightStart, const Point3D rightEnd);
/*
14.4.2 createMedianHS
    
*/       
      void createMedianHS();
/*
14.4.3 getBoundingRec
    
*/       
      Rectangle<2> getBoundingRec(const Point3D& point)const; 
/*
14.5 Attributes

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

When intersecting two coplanar P-Paces, the resulting result P-faces are 
stored as critical result P-faces. For objects of this class, after acquiring
all critical result P-Faces, another processing step is necessary. 
All necessary methods for this processing step are implemented in the class.

*/    
    class CriticalPResultFace {
    public:
/*      
15.1 Constructors    

*/      
      CriticalPResultFace();      
      CriticalPResultFace(const CriticalPResultFace& other);      
      CriticalPResultFace(const Segment3D& left, const Segment3D& right, 
                          SourceFlag source, Predicate predicate);   
/*
15.2 Getter methods

*/         
      Point3D getMidPoint()const;
      Segment3D getLeft() const;
      Segment3D getRight() const;
      Predicate getPredicate()const;
      PResultFace getPResultFace()const;
/*
15.3 Methods, operators and predicates

15.3.1 isPartOfUnitA

*/    
      bool isPartOfUnitA() const;
/*
15.3.2 createMedianHS
    
*/        
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
/*
15.3.7 print
    
*/       
      bool operator <(const CriticalPResultFace& other) const;
      
    private:
/*
15.4 Private methods

15.4.1 Setter Methods 

*/        
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

In order to be able to generate a result region unit, result P-Faces and 
critical result P-Faces are formed from all layers that exist in the same 
time interval and assigned to an object of this type. After the critical 
result P-Faces have been processed, moving segments may be generated 
from the result P-Faces of the object to produce a result region unit.
The class provides methods for all these functions.

*/     
    class ResultUnit{
    public:
/*      
16.1 Constructors    

*/
      ResultUnit();
      ResultUnit(double orginalStartTime,double orginalEndTime);
      ResultUnit(const ResultUnit& other);
/*
16.2 Methods, operators and predicates

16.2.1 addPResultFace

*/         
      void addPResultFace(PResultFace& prFace, bool completely);
      void addPResultFace(const CriticalPResultFace& cprFace);
      void addCPResultFace(const CriticalPResultFace& cprFace);
/*
16.2.2 getTimeInterval
    
*/       
      Interval<Instant> getTimeInterval() const;
/*
15.2.3 size
    
*/ 
      size_t size();
/*
16.2.4 print
    
*/    
      std::ostream& print(std::ostream& os, std::string prefix)const; 
/*
16.2.5 operator <<

*/
      friend std::ostream& operator <<(std::ostream& os, 
                                       const ResultUnit& unit);
/*
16.2.6 operator ==
    
*/     
      bool operator ==(const ResultUnit& unit) const; 
/*
16.2.7 operator =
    
*/       
      ResultUnit& operator = (const ResultUnit& unit);  
/*
16.2.4 finalize
    
*/ 
      void finalize();
/*
16.2.4 convertToURegionEmb
    
*/       
      URegionEmb* convertToURegionEmb(DbArray<MSegmentData>* segments) const;
/*
16.2.4 evaluateCriticalMSegmens
    
*/       
      void evaluateCriticalMSegmens(SetOp setOp);
            
    private:
/*
16.3 Private methods

16.3.1 set 

*/       
      void set(const ResultUnit& other);
/*
16.3.2 less
    
*/       
      static bool less(const PResultFace& prf1, const PResultFace& prf2);
/*
16.3.3 logicLess
    
*/ 
      static bool logicLess(const PResultFace& prf1, const PResultFace& prf2); 
/*
16.3.4 copyCriticalMSegmens
    
*/       
      void copyCriticalMSegmens(const CriticalPResultFace& cprf, SetOp setOp);
/*
16.4 Attributes

*/         
      std::vector<PResultFace> prFaces;
      std::vector<CriticalPResultFace> mCSegments;                
      double orginalStartTime;
      double orginalEndTime;      
    };// ResultUnit        
/*
17 class Layer

The segments that are created in the sweep plan belong to different result units
depending on the time interval in which they are defined. An object of the class 
Layer represents within a P-Face a layer that records all segments of a time 
interval. Using methods of this class, the determination and passing on of 
predicates is realized. From the stored segments, the result P-Faces or the 
critical result P-Faces are finally formed. For all these functions, the class 
provides appropriate methods.

*/        
    class Layer {
    public:
/*
17.1 Constructor

*/
      Layer(bool iscritical = false);
      Layer(const Layer& layer); 
/*
17.3.1 Method addOrthSegment 
 
Adds an orthogonal segment to the layer

*/
      void addOrthSegment(const Segment& segment);
/*
17.3.4 Method addNonOrthSegment

Adds a non-orthogonal segment to layer

*/
      void addNonOrthSegment(const Segment& segment);  
/*
17.3.5 Method setPredicateFromPredeccor

Sets the predicate of the segment at the left boundary according 
to the information of the predecessor      

*/
      void setPredicateFromPredecessor(Predicate predicate);      
/*
17.3.6 Method setPredicateFromSuccessor

Sets the predicate of the segment at the left boundary according 
to the information of the successor

*/      
      void setPredicateFromSuccessor(Predicate predicate);
/* 
17.3.7 Method getPredicateForPredecessor     

Returns the predicate for the left segment of the successor

*/
      Predicate getPredicateForPredecessor()const;
/*  
17.3.8 Method getPredicateForSuccessor
 
Returns the predicate for the left segment of the predecessor
 
*/
      Predicate getPredicateForSuccessor()const;
/*
17.3.9 Method getBorderPredicate

Returns the predicates for the left and right border

*/
      void getBorderPredicates(Predicate& left, Predicate& right)const;
/*
17.3.10 Method setBorderPrediacte

sets the predicates for the left and right border

*/
      void setBorderPrediactes(Predicate left, Predicate right); 
/*
17.3.11 Method evaluate

Determines the predicates of all segments of a layer with the help 
of the intersegment segments

*/
      bool evaluate();
/*
17.3.12 Method print

Output the content into a stream

*/
      std::ostream& print(std::ostream& os,std::string prefix)const;
/*
17.3.13 Operator <<

*/      
      friend std::ostream& operator <<(std::ostream& os, const Layer& layer);
/*
17.3.14 Operator ==

Comparison operator, compare two objects for equality.

*/        
      bool operator ==(const Layer& layer)const;   
/*
17.3.15 Operator ==

Assignment operator

*/            
      Layer& operator =(const Layer& other);  
/*
17.3.16 Method intersects 
 
The method determines whether an intersection is within a layer. The result is
returned via the reference predicate. If an evaluation of the layer is possible,
the return value is "true"     

*/
      bool intersects(bool& predicate)const;
/*
17.3.17 Method inside

The method determines whether a layer is within the other region unit. 
The result is returned via the reference predicate. If an evaluation of the 
layer is possible, the return value is "true"

*/      
      bool inside(bool& predicate)const;
/*
17.3.18 Method getResultUnit
 
 In the result unit, the moving segments of the layer are created and stored 
 which correspond to the desired predicate. Critical moving segments are 
 recorded seperately in the results unit.
 
*/
      void getResultUnit(Predicate soughtPredicate, bool reverse, 
                         const Point3DContainer& points, ResultUnit& unit,
                         SourceFlag source)const;        

    private:
/*
17.4.1 Method getBorderPredicate

Returns the predicate of the area bounded by the segment

*/
      Predicate getBorderPredicate(const Segment& segment, Border border)const;
/*
17.4.2 Method getAreaPredicate

Returns the predicate of the surface bounded by the left and right segments

*/
      Predicate getAreaPredicate(size_t left, size_t right, size_t orthogonal, 
                                 bool orthSegmentExist)const;      
/*
17.4.3 Method print

Outputs a vector into a stream

*/      
      void print(std::ostream& os, std::vector<size_t> values)const;
/*
17.4.4 Method set

Applies the attributes of another object of the same class

*/            
      void set(const Layer& layer);    
/*
17.5 Attributes

*/      
      SegmentContainer     segments;
      std::vector<size_t>  orthSegments;
      std::vector<size_t>  nonOrthSegments;
      size_t               touchAbove;
      size_t               touchBelow;
      bool                 isCritical;      
    }; // class Layer
    
/*
18 class LayerContainer

Within a P-Face, a layer of segments is generated for each time interval used 
in the plane sweep. All layers within a P-Face are stored in an object of the
class LayerContainer. The class provides appropriate methods for identifying 
and passing predicates between layers and beyond the boundaries of the P-Face.

*/          
    class LayerContainer {
    public:
/*
18.1 Constructor

*/      
      LayerContainer(size_t size = 0, bool isCritcal = false);
      LayerContainer(const LayerContainer& other);
      LayerContainer(Point3DContainer& points,
                     GlobalTimeValues &timeValues,
                     PlaneSweepAccess &access,
                     bool isCritcal);
/*
18.3.1 Method addOrthSegment 
 
Add the orthogonal segment to the specified layer

*/
      void addOrthSegment(size_t layer, const Segment& segment);
/*
18.3.2 Method addNonOrthSegment

Add the non-orthogonal segment to the specified layer

*/
      void addNonOrthSegment(size_t layer, const Segment& segment); 
/*
18.3.3 Method print

Output the content into a stream

*/
      std::ostream& print(std::ostream& os,std::string prefix)const;
/*
18.3.4 Operator <<

*/      
      friend std::ostream& operator <<(std::ostream& os, 
                                       const LayerContainer& layerContainer);
/*
18.3.5 Method evaluate

In all layers of a P-Face, the predicates of the intersegment segments or 
boundaries are evaluated. If all layers of the P-PFace could be evaluated, 
the return value is "true".

*/
      bool evaluate();
/*
18.3.6 Method getBorderPredicates

Returns the predicate for the left and right boundary of the layers 
of a P-Faces.

*/
      void getBorderPredicates(Predicate& left, Predicate& right)const;
/*
18.3.7 Operator ==

Comparison operator, compare two objects for equality.

*/      
      bool operator ==(const LayerContainer& other)const;
/*
18.3.8 Operator =

Assignment operator

*/
      LayerContainer& operator =(const LayerContainer& other);  
/*
18.3.9 Method intersects

*/    
      bool intersects(std::vector<bool>& predicate)const;
/*
18.3.10 Method inside

*/      
      bool inside(std::vector<bool>& predicate)const; 
 
/*
18.3.11 Method getResultUnit
 
In the result unit, the moving segments of the given layer are created 
and stored, which correspond to the desired predicate. Critical moving 
segments are recorded seperately in the results unit.
 
*/      
      void getResultUnit(size_t layer, Predicate soughtPredicate,
      bool reverse, const Point3DContainer& points, 
      ResultUnit& unit, SourceFlag source)const;
      
    private:
/*
18.4 Private methods

18.4.1 Method set

Applies the attributes of another object of the same class

*/      
      void set(const LayerContainer& other);      
/*
18.5 Attributes

*/      
      std::vector<Layer> layers;  
    
    };// LayerContainer
        
/*
19 Class PFace

Objects of this class and the components contained in them represent the main
processing objects of the implemented algorithms. For this purpose, the cuts 
between P-Faces are determined and the generated intersection segments are 
assigned to the relevant P-faces. In the next step, layers with segments of 
the result P-Faces are formed from the intersection segments during the plane 
sweep. The determination and passing on of the predicates in the layers and 
beyond the P-Faces are also organized at the level of the P-Faces. For all these
functions, appropriate methods are implemented in the class.

*/    
    class PFace: public PResultFace, public PlaneSweepAccess {
    friend class Selftest;    
    public:
/*
19.1 Constructors

*/
      PFace(size_t left, size_t right,const Point3DContainer& points, 
            const SegmentContainer& segments);
      PFace(const MSegmentData& mSeg, const GlobalTimeValues& timeValues,
            Point3DContainer& points, 
            SegmentContainer& segments);    
      PFace(const PFace& pf); 
/*
19.2 Setter and Getter methods

*/
      void set(const PFace& pf);
      void setState(State state);
      State getState() const;
/*
19.3 Methods, operators and predicates

19.3.1 existsIntSegs

*/
      bool existsIntSegs()const;
/*
19.3.2 print

*/        
      std::ostream& print(std::ostream& os, std::string prefix)const;     
/*
19.3.3 Operator <<
    
Print the object values to stream.

*/            
      friend std::ostream& operator <<(std::ostream& os, const PFace& pf);      
/*
19.3.4 intersectionOnPlane

*/
      bool intersectionOnPlane(PFace& other, 
                               const RationalPlane3D& planeSelf,
                               GlobalTimeValues &timeValues);
/*
19.3.5 intersection

Computes the intersection of this ~PFace~ with other. 

*/
      bool intersection(PFace& other,GlobalTimeValues &timeValues);
/*
19.3.6 toString

*/      
      static std::string toString(State state);
/*
19.3.7 addIntSeg

*/       
      void addIntSeg(const IntersectionSegment& seg);     
      void addIntSeg(const RationalPlane3D &planeSelf, 
                     const RationalPlane3D &planeOther,
                     const RationalSegment3D &intSeg,
                     GlobalTimeValues &timeValues);
/*
19.3.8 addBorder

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
19.3.9 setBorderPredicate
 
The method sets the predicate of the border segment of the P-Face 
to the specified value.
 
*/
      void setBorderPredicate(SegmentContainer& segment, Predicate predicate);
/*
19.3.10 Operator =

*/  
      PFace& operator =(const PFace& pf);
/*
19.3.11 Operator ==

*/        
      bool operator ==(const PFace& pf)const;
/*
19.3.12 first
    
*/    
      void first(double t1, double t2, Point3DContainer& points,
                                       SegmentContainer& segments,
                                       bool pFaceIsCritical);
/*
19.3.13 next
    
*/     
      void next(double t1, double t2, Point3DContainer& points, 
                                      SegmentContainer& segments,
                                      bool pFaceIsCritical);
/*
19.3.14 finalize
    
*/       
      bool finalize(Point3DContainer& points, SegmentContainer& segments, 
                    GlobalTimeValues& timeValues);
/*
19.3.15 intersects
    
*/       
      bool intersects(Point3DContainer& points, GlobalTimeValues& timeValues,
                      std::vector<bool>& predicate);
/*
19.3.16 inside
    
*/       
      bool inside(Point3DContainer& points, GlobalTimeValues& timeValues,
                  std::vector<bool>& predicate);
/*
19.3.17 getResultUnit
    
*/       
      void getResultUnit(size_t slide, Predicate predicate, bool reverse, 
                         const Point3DContainer& points, ResultUnit& unit,
                         SourceFlag source);
    private:  
/*
19.4 Private methods

19.4.1 Method getBorderSegments

The method determines all or only the segments for the left and right edges 
of a P-Face, depending on the boolean variable "all".

*/
      void getBorderSegments(bool all, std::vector<RationalSegment3D>& borders);
/*
19.4.2 Method intersection

The method calculates the segment of the intersection for the edge segments
of a P-Face and a segment. All segments are exists on the plane of the 
P-Face. If a section segment exists, the return value is "true".

*/      
      bool intersection(const std::vector<RationalSegment2D>& borders, 
                        const RationalSegment2D& segment, 
                        RationalSegment2D& result);
/*
19.4.3 Method map

The method transfers the points of a segment in the plane to a segment
in the (x, y, t) space. The points of the intersection must lie on the 
original segment of the plane. The segment on the plane must be an 
transformation of the segment in space.

*/        
      RationalSegment3D map(const RationalSegment3D& orginal3D, 
                            const RationalSegment2D& orginal2D, 
                            const RationalSegment2D& intersection);
/*
19.4.4 createBorder

Creates a segment for the boder of a P-Face. Whether it is the left or 
right edge is defined by the value of "border".

*/        
      IntersectionSegment createBorder(const RationalPlane3D &planeSelf,
                                       Border border, Predicate predicate);
/*
19.5 Attributes

*/      
      IntSegContainer    intSegs;
      LayerContainer     layers;
      State              state;
      size_t             left;
      size_t             right;  
    };// class PFace  
    
/*
20 Class FaceCycleInfo

In order to ensure the functionality of the algorithm, information about faces
and cycles of the input region units in the source units must be acquired. 
Objects of this class ensure that intersection information is generated for at
least one P-Face of each face and cycle of the region unit. The class provides 
suitable methods for the necessary functions.

*/
    class FaceCycleInfo {
    public:
/*
20.1 Constructor

*/      
      FaceCycleInfo();
/*
20.2 Setter and Getter methods

*/      
      void   setFirstIndex(size_t firstIndex);
      void   setTouch();
      bool   getTouch() const;
      size_t getFirstIndex() const;  
    private:
/*
20.3 Attributes

*/        
      size_t firstIndex;
      bool   touch;
    };
    
/*
21 class SourceUnit

The input region units are mapped as objects of this class to perform the 
desired operations. The moving segments of the input region units are 
stored as P-Faces in the objects. This class provides methods for 
bundling the described algorithms for the P-Faces at the next higher 
level of abstraction and providing appropriate functions for the set 
operations.

*/        
    class SourceUnit{
    public:
/*
21.1 Constructors and Destructor

*/       
      SourceUnit();
      SourceUnit(const SourceUnit& other);     
      ~SourceUnit();
/*
21.2 Methods, operators and predicates

21.2.1 addPFace
    
*/      
      void addPFace(const Segment& left, const Segment& right, 
                    const Point3DContainer& points);
/*
21.2.2 addMSegmentData
    
*/  
      void addMSegmentData(const MSegmentData& mSeg, 
                           const GlobalTimeValues& timeValues,
                           Point3DContainer& points);
/*
21.2.3 isEmpty
    
*/  
      bool isEmpty()const;
/*
21.2.4 intersect
    
*/  
      bool intersect(const SourceUnit& other)const;
/*
21.2.5 addToResultUnit
    
*/  
      void addToResultUnit(ResultUnit& result)const;
      
/*
21.2.6 intersection
    
*/       
      void intersection(SourceUnit& other, GlobalTimeValues& timeValues); 
/*
21.2.7 intersectionFast
    
*/  
      void intersectionFast(SourceUnit& other, GlobalTimeValues& timeValues);
/*
21.2.8 finalize
    
*/  
      bool finalize(Point3DContainer& points, GlobalTimeValues& timeValues,
                    Predicate predicateconst, SourceUnit& other);
/*
21.2.9 intersects
    
*/  
      void intersects(Point3DContainer& points, GlobalTimeValues& timeValues,
                      SourceUnit& other, std::vector<bool>& predicate);
/*
21.2.10 inside
    
*/  
      void inside(Point3DContainer& points, GlobalTimeValues& timeValues,
                  SourceUnit& other, std::vector<bool>& predicate);
/*
21.2.11 getResultUnit
    
*/  
      void getResultUnit(size_t slide, Predicate predicate,bool reverse, 
                         const Point3DContainer& points, ResultUnit& unit,
                         SourceFlag source);
/*
21.2.12 print
    
*/  
      std::ostream& print(std::ostream& os, std::string prefix)const;
/*
21.2.13 Operator <<
    
Print the object values to stream.

*/          
      friend std::ostream& operator <<(std::ostream& os, 
                                       const SourceUnit& unit);
/*
21.2.14 Operator ==  

*/       
      bool operator ==(const SourceUnit& unit)const; 
/*
21.2.15 operator =
    
*/        
      SourceUnit& operator =(const SourceUnit& unit);
/*
21.2.16 createTestRegion
    
*/        
      void createTestRegion();
/*
21.2.17 isInside
    
*/        
      bool isInside(const PFace* pFace);
/*
21.2.18 createFaceCycleEntry
    
*/              
      void createFaceCycleEntry(const PFace* pf, size_t index);
/*
21.2.19 touchFaceCycleEntry
    
*/        
      void touchFaceCycleEntry(const PFace* pf);
/*
21.2.20 checkFaceCycleEntrys
    
*/        
      void checkFaceCycleEntrys(SourceUnit& other);
/*
21.2.21 printFaceCycleEntrys
    
*/        
      void printFaceCycleEntrys();
/*
21.2.22 reSort
    
*/  
      void reSort();      
           
    private: 
/*      
21.3 Private methods  

21.3.1 lessByMedianHS

*/
      static bool lessByMedianHS(const PFace* pf1, const PFace *pf2);
/*
21.3.2 logicLess
    
*/               
      static bool logicLess(const PFace* pf1, const PFace *pf2);
/*
21.3.3 set
    
*/       
      void set(const SourceUnit& other);
/*
21.4 Attributes

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
22 Class SourceUnitPair

In this class, processing is performed for set operations from the two source
units to the result unit. For this purpose, an object of the class has two 
source units, from which one or more result units are calculated. The class 
provides suitable methods for the set operations to be performed, which 
exclusively relate to the internal representation. Methods are also provided 
for storing or reading out region units into the internal data structure.

*/ 
    class SourceUnitPair {
    public:
/*
22.1 Constructors

*/        
      SourceUnitPair(double orginalStartTime = 0,double orginalEndTime = 1, 
                     double scale = 1);
      SourceUnitPair(const Interval<Instant>& orginalInterval);
/*
22.2 Methods, operators and predicates

22.2.1 setScaleFactor
    
*/          
      void setScaleFactor(double scale);
/*

22.2.2 addPFace
    
*/                   
      void addPFace(SourceFlag flag, Segment3D& left, Segment3D& right);
/*

22.2.3 addMSegmentData
    
*/                      
      void addMSegmentData(const MSegmentData& mSeg, SourceFlag sourceFlag);
/*

22.2.4 operate
    
*/                      
      bool operate(SetOp setOp);
/*

22.2.5 predicate
    
*/                     
      bool predicate(PredicateOp predicateOp);
/*

22.2.6 print
    
*/ 
      std::ostream& print(std::ostream& os, std::string prefix)const;
/*

22.2.7 operator <<
    
*/ 
      friend std::ostream& operator <<(std::ostream& os, 
                                       const SourceUnitPair& unitPair);
/*

22.2.8 countResultUnits
    
*/ 
      size_t countResultUnits()const;
/*

22.2.9 getResultUnit
    
*/ 
      ResultUnit getResultUnit(size_t slide)const;
/*

22.2.10 createResultMRegion
    
*/ 
      void createResultMRegion( MRegion* resMRegion);
/*

22.2.11 createResultMBool
    
*/ 
      void createResultMBool(MBool* resMBool, bool lc, bool rc );
/*

22.2.12 createSourceUnit
    
*/ 
      void createSourceUnit(const Interval<Instant>& interval, MRegion* mregion,
                            SourceFlag sourceFlag);

    private:      
/*
22.3 Attributes

*/
      SourceUnit unitA;
      SourceUnit unitB;
      std::vector<ResultUnit> result;
      std::vector<bool> predicates;      
      GlobalTimeValues timeValues;        
      Point3DContainer points; 
    };
/*
23 class SetOperator

In this class, the processing for set operations from moving input regions to
moving result regions is realized. For this purpose, the refinement partition 
first generates pairs of source region units defined over the same time
interval. An object of the SourceUnitPair class organizes the processing of 
the two source units until the result is several result units. From these, 
region units are formed and added to the moving region of the result.

*/
    class SetOperator {
    public:  
/*
23.1 Constructors

*/  
      SetOperator(MRegion* const _mRegionA, MRegion* const _mRegionB, 
                  MRegion* const _mRegionResult);
/*
23.2 Method operate
    
*/        
      void operate(SetOp setOp);    

    private: 
/*
23.3 Attributes

*/
      MRegion* const mRegionA;
      MRegion* const mRegionB;
      MRegion* const mRegionResult;
    }; // class SetOperator

/*
24 class PredicateOperator

In this class, the processing of the predicate operations from moving input 
regions to a moving Boolean value is realized.

*/
    class PredicateOperator {
    public:  
/*
24.1 Constructors

*/   
      PredicateOperator(MRegion* const _mRegionA, MRegion* const _mRegionB, 
                        MBool* const  _mBool);
/*
23.2 Method operate
    
*/      
      void operate(PredicateOp predicateOp);    

    private: 
/*
24.4 Attributes

*/
      MRegion* const mRegionA;
      MRegion* const mRegionB;
      MBool*   const mBool;
    }; // class PredicateOperator    

  } // end of namespace mregionops3
} // end of namespace temporalalgebra
#endif 
// SETOPS_H
