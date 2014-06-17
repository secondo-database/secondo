/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
Faculty of Mathematics and Computer Science,
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


[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

1 class declarations

class SetOperator2 - startpoint Operate()
class SourceUnitPair2
class SourceUnit2

2 Defines and Includes

*/

#ifndef SETOPS2_H_
#define SETOPS2_H_

#include "MovingRegion2Algebra.h"
#include "PointVector.h"
#include "Segment.h"
#include "Refinement3.h"
#include "IntersectionSegment.h"
#include "IntSegContainer.h"
#include "Point3DExt.h"
#include "ResultUnitFactory.h"
#include "Region2Algebra.h"

namespace mregionops2 {

/*

1 Forward declarations

*/

class SourceUnit2;
class SourceUnitPair2;
class IntSegContainer;
class MSegment;
class MSegmentCritical;


/*
1.1 $DECIDE\_BY\_ENTIRELY\_IN\_OUT$

Enables an optimization during the construction of a result unit.
See the class ~ResultUnitFactory~ for details.

*/

const bool DECIDE_BY_ENTIRELY_IN_OUT = true;


/*

1 Enumeration SetOp

Indicates the kind of set operation.

*/

enum SetOp {

    INTERSECTION,
    UNION,
    MINUS
};

/*

1 Class SetOperator2

This class provides the three set operations
~intersection~, ~union~ and ~minus~ with the signature
mregion2 [x] mregion2 [->] mregion2.

*/

class SetOperator2 {

public:

/*

1.1 Constructor

The constructor takes three pointers to ~MRegion2~ instances,
according to the signature a [x] b [->] res.

*/

    SetOperator2(MRegion2* const _a,
                 MRegion2* const _b,
                 MRegion2* const _res) :
                 a(_a), b(_b), res(_res) {
    }

/*

1.1 Operators

1.1.1 Intersection

Performs the operation $a \cap b$ and writes the result to res.

*/

    void Intersection();

/*

1.1.1 Union

Performs the operation $a \cup b$ and writes the result to res.

*/

    void Union();

/*

1.1.1 Minus

Performs the operation $a \setminus b$ and writes the result to res.

*/

    void Minus();

private:

/*

1.1 Private methods

*/

    void Operate(const SetOp op);

/*

1.1 Attributes

*/

    MRegion2* const a;
    MRegion2* const b;
    MRegion2* const res;
};

/*

1 Class SourceUnit2

This class represents a single unit of a ~MRegion2~ A or B,
which are the arguments of the set operation.

*/

class SourceUnit2 {

    friend class SourceUnitPair2;

public:

/*

1.1 Constructor

Note that the interval of the parameter ~uRegion~ might be changed!

*/

    SourceUnit2(const bool _isUnitA,
                MRegion2* const _mRegion,
                precTimeInterval _interval,
                const int _pos,
                SourceUnitPair2* const _parent);

/*

1.1 Destructor

*/
  
  ~SourceUnit2();

/*
1.1 Getter and setter methods

1.1.1 SetPartner

Sets a pointer the other ~SourceUnit~ instance, which
is the other argument of the set operation.

*/

    inline void SetPartner(SourceUnit2* _partner) {

        partner = _partner;
    }

/*
1.1.1 GetPartner

Returns a pointer the other ~SourceUnit~ instance, which
is the other argument of the set operation.

*/

    inline SourceUnit2* GetPartner() const {

        return partner;
    }

/*
1.1.1 GetParent

Returns a pointer the parent object.

*/

    inline SourceUnitPair2* GetParent() const {

        return parent;
    }


/*
1.1 Getter and setter methods

1.1.1 IsEmpty

Returns ~true~, if this ~SourceUnit~ represents an empty unit.

*/

    inline bool IsEmpty() const {
      return (pos == -1);
    }

    inline mpq_class GetStartTime(){
      return interval.start;
    }

    inline mpq_class GetEndTime(){
      return interval.end;
    }

/*
1.1.1 IsUnitA/B

Returns ~true~, if this ~SourceUnit~ represents the first/second argument
of the set operation.

*/

    inline bool IsUnitA() const {
     return isUnitA;
    }

    inline bool IsUnitB() const {
     return !isUnitA;
    }


/*
1.1 Getter and setter methods

1.1.1 GetOperation

Returns an enum to indicate the current set operation:
INTERSECTION, UNION or MINUS.

*/

    const SetOp GetOperation() const;

/*
1.1 Operators and Predicates

1.1.1 AddGlobalTimeValue

Adds a relevant timevalue, which marks the border of a result unit.
This value is passed to the ~ResultUnitFactory~.

*/

   void AddGlobalTimeValue(mpq_class t);

/*
1.1.1 AddToMRegion

Adds this unit to target.

*/

    void AddToMRegion(MRegion2* const target) const;

/*
1.1.1 IsEntirelyOutside

Returns ~true~, if pFace is entirely outside of this ~SourceUnit~.

Precondition: pFace is already known as entirely inside or entirely outside!

*/

    bool IsEntirelyOutside(const PFace* pFace);

/*
1.1.1 IsOutside

Returns ~true~, if ~p~ is outside of this ~SourceUnit~.

Note: We create a ~TestRegion~ ~tr~ by evaluating this ~SourceUnit~
at time ~p.t~ and check, if the point ~(p.x; p.y)~ is outside of ~tr~.
For the sake of efficency, we cache ~tr~ for later use.

*/

    bool IsOutside(const Point3D& p);

/*
1.1.1 IsOnBorder

Returns ~true~, if ~p~ is on the border of this ~SourceUnit~.

Note: We create a ~TestRegion~ ~tr~ by evaluating this ~SourceUnit~
at time ~p.t~ and check, if the point ~(p.x; p.y)~ is on the border of ~tr~.
For the sake of efficency, we cache ~tr~ for later use.

*/

    bool IsOnBorder(const Point3D& p);

private:


    void NormalizeTimeInterval();
    void ComputeBoundingRect();
    const Region GetTestRegion(const double t);
/*
1.1 Attributes

1.1.1 isUnitA

A flag, indicating which argument of the set operation
this ~SourceUnit~ represents: A or B.

*/

    const bool isUnitA;

/*
1.1.1 isEmpty

A flag, indicating that this ~SourceUnit~ is empty.

*/

    const int pos;

/*

1.1.1 uRegion

A pointer to the corresponding ~URegionEmb~.

Note that the interval of uRegion might be changed!

*/

    MRegion2* const mRegion;
    precTimeInterval interval;
/*

1.1.1 array

A pointer to the corresponding ~DBArray<MSegmentData>~.

1.1.1 partner

A pointer to  the other ~SourceUnit~ instance, which
is the other argument of the set operation.

*/

    SourceUnit2* partner;

/*

1.1.1 parent
A pointer the parent object.

*/

    SourceUnitPair2* const parent;

/*

1.1.1 pFaces
A ~std::vector~ to store all ~PFaces~.

*/

    vector<PFace*> pFaces;

/*

1.1.1 pFacesReduced

A ~std::vector~ to store a reduced set of ~PFaces~, relevant for the
intersection process. This optimization can be done in advance by using
the projection bounding rectangles of both ~SourceUnits~.

*/

    vector<PFace*> pFacesReduced;

/*

1.1 Operators and Predicates
1.1.1 CreatePFaces

Creates a new ~PFace~ for each ~MSegmentData~ of this unit.

*/

    void CreatePFaces();

/*
1.1.1 CollectRelevantPFaces

Performs a linear scan over the list of all ~PFaces~,
collecting those which are relevant for the result and
pass them to the ~ResultUnitFactory~.

*/

    void CollectRelevantPFaces(ResultUnitFactory* receiver);



/*

1.1 Methods for debugging

*/

    void PrintPFaces();

/*

1.1.1 pFace

A pointer to the ~PFace~, this instance belongs to.

*/

    const PFace* pFace;

// new  
 void restrictSegtoInterval(MSegmentData2 sourceSeg, 
                            precTimeInterval sourceInt, 
                            precTimeInterval destInt, 
                            MSegmentData2* destSeg);

};


/*
1 Class SourceUnitPair2

This class is essentially a container for both ~SourceUnits~ A and B,
creates and initialise the ~ResultUnitFactory~ and runs the main-loop
of the set operation by calling the method ~Operate~.

*/

class SourceUnitPair2 {

public:

/*

1.1 Constructor

The constructor takes essentially two pointers to ~URegionEmb2~ instances,
one pointer to the global result ~MRegion2~ and an enum to indicate the
kind of set operation.

*/

    SourceUnitPair2(MRegion2* const _unitA,
                    int _aPos, 
                    precTimeInterval _interval,
                    MRegion2* const _unitB,
                    int _bPos,
                    const SetOp _operation,
                    MRegion2* const resultMRegion);
/*

1.1 Destructor

*/

//   ~SourceUnitPair2();

/*

1.1 Operators

1.1.1 Operate

This method runs the main-loop of the set operation by calling
the following methods:

  1 Step ComputeIntSegs
  2 Step CollectRelevantPFaces
  3 Step ConstructResultUnits

*/

    void Operate();

/*
1.1 Getter and setter methods

1.1.1 GetOperation

Returns an enum to indicate the current set operation:
INTERSECTION, UNION or MINUS.

*/

    inline const SetOp GetOperation() const {
        return op;
    }

/*
1.1.1 GetOverlapRect

Returns the intersection of the projection bounding rectangles of
both ~SourceUnits~, A and B.

*/

    inline Rectangle<2> GetOverlapRect() const {
        return overlapRect;
    }
/*
1.1.1 HasOverlappingBoundingRect

Returns ~true~, if the intersection of the projection bounding rectangles of
both ~SourceUnits~ is not empty.

*/

    inline bool HasOverlappingBoundingRect() const {
	return overlapRect.IsDefined();
    
    }

private:

/*

1.1 Methods for debugging

*/

    void PrintPFaces();
    void ToVrmlFile(bool a, bool b, bool res);

/*
1.1 Private methods

1.1.1 CreatePFaces
Calls ~SourceUnit2::CreatePFaces~ on both instances.

*/
    void CreatePFaces();
//    void CreatePFaces(precTimeInterval interval);

/*

1.1.1 ComputeIntSegs

Intersects each ~PFace~ from ~SourceUnit A~ with each ~PFace~ from
~SourceUnit B~. The result might be an ~IntersectionSegment~ pair.

*/

    void ComputeIntSegs();

/*

1.1.1 CollectRelevantPFaces

Performs a linear scan over all ~PFaces~ of both ~SourceUnits~,
collecting those which are relevant for the result and
pass them to the ~ResultUnitFactory~.

*/

    void CollectRelevantPFaces();

/*
1.1.1 ConstructResultUnits

Runs the construction of all result units by calling
~ResultUnitFactory::Start~.

*/

    void ConstructResultUnits();

/*
1.1.1 IsEntirelyOutside

Returns ~true~, if pFace is entirely outside of this ~SourceUnit~.

Precondition: pFace is already known as entirely inside or entirely outside!

*/

    bool IsEntirelyOutside(const PFace* pFace);

/*
1.1.1 ComputeOverlapRect

Computes the intersection of the projection bounding rectangles of
both ~SourceUnits~.

*/

    void ComputeOverlapRect();

/*
1.1 Attributes

*/
    SourceUnit2 unitA;
    int aPos;
    precTimeInterval interval;
    SourceUnit2 unitB;
    int bPos;
    const SetOp op;
    MRegion2* const resultMRegion;
    Rectangle<2> overlapRect;  
    ResultUnitFactory resultUnitFactory;

};
/*
1 Class MSegment

This class represents a moving segment, which is constructed by
the ~ResultUnitFactory~.

*/

class MSegment {

public:

/*
1.1 Constructor

*/

    MSegment(const Segment3D& initial,
             const Segment3D& median,
             const Segment3D& final,
             const PFace* const pFace);

/*
1.1 Getter and setter methods

1.1.1 GetPFace

Returns a pointer to the ~PFace~, this ~MSegment~ comes from.

*/

    inline const PFace* GetPFace() const {

        return pFace;
    }

/*
1.1.1 GetInitial/GetFinal

Returns the initial/final state of this ~MSegment~.
This segment might be degenerated to a point.

*/

    inline const Segment2D& GetInitial() const {

        return initial;
    }

    inline const Segment2D& GetFinal() const {

        return final;
    }

/*
1.1.1 GetMedian

Returns the median state of this ~MSegment~.
This segment can never degenerate.

*/

    inline const Segment2D& GetMedian() const {

        return median;
    }

/*
1.1.1 GetMedianHS

Returns the median state of this ~MSegment~ as ~HalfSegment~.
This segment can never degenerate.

*/

    inline const HalfSegment& GetMedianHS() const {

        return medianHS;
    }

/*
1.1.1 GetFaceNo/GetCycleNo/GetSegmentNo

Returns the faceno/cycleno/edgeno of this ~MSegment~.

*/

    inline int GetFaceNo() const {

        return medianHS.attr.faceno;
    }

    inline int GetCycleNo() const {

        return medianHS.attr.cycleno;
    }

    inline int GetSegmentNo() const {

        return medianHS.attr.edgeno;
    }

/*
1.1.1 GetInsideAbove

Returns the flag insideAbove of this ~MSegment~.

*/

    inline int GetInsideAbove() const {

        return insideAbove;
    }

/*

1.1.1 IsLeftDomPoint

Returns ~true~, if this ~MSegment~ is a left one.
Note: This is indicated by the median ~HalfSegment~.

*/

    inline bool IsLeftDomPoint() const {

        return medianHS.IsLeftDomPoint();
    }

/*
1.1.1 SetLeftDomPoint

Marks this ~MSegment~ as left.

*/

    inline void SetLeftDomPoint(bool ldp) {

        medianHS.SetLeftDomPoint(ldp);
    }

/*
1.1.1 SetSegmentNo

Sets the segmentno to sn.

*/

    inline void SetSegmentNo(int sn) {

        medianHS.attr.edgeno = sn;
    }

/*
1.1 Operators and Predicates

1.1.1 CopyIndicesFrom

Copies the faceno/cycleno/edgeno from hs to this.

*/

    inline void CopyIndicesFrom(const HalfSegment* hs) {

        medianHS.attr.faceno = hs->attr.faceno;
        medianHS.attr.cycleno = hs->attr.cycleno;
        medianHS.attr.edgeno = hs->attr.edgeno;
        //medianHS.attr.insideAbove = hs->attr.insideAbove;
    }

/*
1.1.1 IsParallel

Returns ~true~, if this ~MSegment~ is parallel to ms.

*/

    bool IsParallel(const MSegment& ms) const;

/*
1.1.1 LessByMedianHS

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
according to the ~HalfSegment~ order, specified in the ~SpatialAlgebra~.

*/

    inline bool LessByMedianHS(const MSegment& ms) const {

        return GetMedianHS() < ms.GetMedianHS();
    }

/*
1.1.1 LogicLess

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
similar to ~HalfSegment::LogicCompare~, specified in the ~SpatialAlgebra~.

*/

    inline bool LogicLess(const MSegment& ms) const {

        if (IsLeftDomPoint() != ms.IsLeftDomPoint())
            return IsLeftDomPoint() > ms.IsLeftDomPoint();

        return GetMedianHS().LogicCompare(ms.GetMedianHS()) == -1;
    }

/*
1.1 Methods for debugging

*/

    void Print() const;

private:

/*
1.1 Attributes

1.1.1 initial/median/final

The initial/median/final state as ~Segment2D~.

*/

    Segment2D initial;
    Segment2D median;
    Segment2D final;

/*
1.1.1 medianHS

The median state as ~HalfSegment~.

*/

    HalfSegment medianHS;

/*
1.1.1 insideAbove

The flag insideAbove.

*/

    bool insideAbove;

/*
1.1.1 pFace

A pointer to the ~PFace~, this ~MSegment~ comes from.

*/

    const PFace* pFace;
};

/*
1 Class MSegmentCritical

This class extends the class ~MSegment~ with the attribute ~midPoint~,
which is needed to handle critical ~MSegments~.
A pair of critical ~MSegments~ is constructed as a result of the
intersection of two overlapping coplanar PFaces.

*/

class MSegmentCritical : public MSegment {

public:

/*
1.1 Constructor

*/

    MSegmentCritical(const Segment3D& _initial,
                     const Segment3D& _median,
                     const Segment3D& _final,
                     const Point3D& _midPoint,
                     const PFace* const _pFace) :

      MSegment(_initial, _median, _final, _pFace),
      midPoint(_midPoint) {

    }

/*
1.1 Getter and setter methods

1.1.1 GetMidPoint

Returns the midpoint of this ~MSegmentCritical~, which is equal to
the midpoint of the median ~HalfSegment~.

*/

    inline const Point2D& GetMidpoint() const {

        return midPoint;
    }

/*
1.1 Operators and Predicates

1.1.1 IsPartOfUnitA

Returns ~true~, if the ~Face~ of this belongs to ~SourceUnit~ A.

*/

    bool IsPartOfUnitA() const;

/*
1.1.1 HasEqualNormalVector

Returns ~true~, if the normal vectors of this' and pf' ~Face~ are equal.

*/

    bool HasEqualNormalVector(const MSegmentCritical& msc) const;

/*
1.1.1 Operator $<$

Returns ~true~, if the midPoint of this is lower as the midpoint of msc.

*/

    bool operator <(const MSegmentCritical& msc) const {

        return midPoint < msc.midPoint;
    }

private:

/*
1.1 Attributes

1.1.1 midPoint

The midpoint of this ~MSegmentCritical~, which is equal to
the midpoint of the median ~HalfSegment~.

*/

    Point2D midPoint;
};


}

#endif /*SETOPS2_H_*/

