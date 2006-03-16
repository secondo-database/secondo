/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] The ~MovingRegionAlgebra~

December 2005, initial version created by Holger M[ue]nx.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January-March 2006, various bugfixes and improvements by Holger M[ue]nx,
with the help of Victor Almeida and Thomas Behr.

[TOC]

1 Introduction

This file contains the class definitions of ~MSegmentData~, 
~RefinementPartition~, 
~TrapeziumSegmentIntersection~, ~URegion~ and ~MRegion~, which
are implemented in ~MovingRegionAlgebra.cpp~. These class definitions have
been moved to this header file to facilitate development work on top of the
~MovingRegionAlgebra~ without modifying the ~MovingRegionAlgebra~ itself.

Please see ~MovingRegionAlgebra.cpp~ for more details on the
~MovingRegionAlgebra~.

*/

#ifndef _MOVING_REGION_ALGEBRA_H_
#define _MOVING_REGION_ALGEBRA_H_

/*
1 Class ~MSegmentData~

The enumeratation ~DegenMode~ specifies how to handle these segments of a
degenerated region unit, which are causing the degeneration (in this case,
we say that the segment is degenerated).

  * ~DGM\_UNKNOWN~: If is not known yet whether the segment is degenerated.
  * ~DGM\_NONE~: Segment is not degenerated.
  * ~DGM\_IGNORE~: Segment is degenerated and can be ignored because it is not
    a border of region area due to its degeneration.
  * ~DGM\_INSIDEABOVE~: Segment is degenerated but is still border of region
    area, which is left or above the segment.
  * ~DGM\_NOTINSIDEABOVE~: Segment is degenerated but is still border of
    region area, which is right or below the segment.

*/
    enum DegenMode { DGM_UNKNOWN,
                     DGM_NONE,
                     DGM_IGNORE,
                     DGM_INSIDEABOVE,
                     DGM_NOTINSIDEABOVE };

class MSegmentData {
/*
The private attributes are used as follows:

  * ~faceno~: The number of the region's face to which the segment belongs.
  * ~cycleno~: The number of the face's cycle, to which the segment belongs.
  * ~segmentno~: The number of the segment in its cycle.
  * ~insideAbove~: ~true~, if the region's interior is left or above of the
    segment. Please note that this is independent of the degeneration status
    of the segment.
  * ~degeneratedInitialInitial~, ~degeneratedFinalFinal~: Specifies whether
    the segment degenerated in the initial or final instant. If so, these
    attributes indicate too whether it is still border of any region area
    even though it is degenerated.
  * ~degeneratedInitialNext~, ~degeneratedFinalNext~: If segments merge into
    a single segments due to degeneration, these attributes are the next
    merged degenerated segments in the region unit's list of segments. The
    last segment will have the value $-1$ in these attributes.
  * ~initialStartX~, ~initialStartY~, ~initialEndX~, ~initialEndY~: The
    coordinates of the segment's start and end points in the initial instant.
  * ~finalStartX~, ~finalStartY~, ~finalEndX~, ~finalEndY~: The
    coordinates of the segment's start and end points in the initial instant.
  * ~pointInitial~, ~pointFinal~: A segment might be reduced to a point in the
    initial or final instant (but not both). This is indicated by these
    values of these two attributes. Please note that reduction of a segment
    to a point and degeneration of a segment are different concepts.

The constructor assures that the segment in initial and final instant is
collinear.

*/

// *hm* Debug only, set attributes to private later again
// private:
public:
    unsigned int faceno;
    unsigned int cycleno;
    unsigned int segmentno;

    bool insideAbove;

    int degeneratedInitialNext;
    int degeneratedFinalNext;
    DegenMode degeneratedInitial;
    DegenMode degeneratedFinal;

    double initialStartX;
    double initialStartY;
    double initialEndX;
    double initialEndY;

    double finalStartX;
    double finalStartY;
    double finalEndX;
    double finalEndY;

    bool pointInitial;
    bool pointFinal;

    MSegmentData() { }

/*
This constructor sets the ~faceno~, ~cycleno~, ~segmentno~, ~insideAbove~,
~initialStartX~, ~initialStartY~, ~initialEndX~, ~initialEndY~,
~finalStartX~, ~finalStartY~, ~finalEndX~ and ~finalEndY~ attributes
according to the parameters.

~degeneratedInitialNext~ and ~degeneratedFinalNext~ are initialised with
$-1$, ~degeneratedInitial~ and ~degeneratedFinal~ with $DGM_UNKNOWN$.

~pointInitial~ and ~pointFinal~ are calculated from the parameters and
an exception is thrown if segment is reduced to a point in initial and
final instant.

It is assured that the segment is collinear in initial and final instant
and an exception is thrown otherwise.

*/
    MSegmentData(
        unsigned int fno,
        unsigned int cno,
        unsigned int sno,
        bool ia,
        double isx,
        double isy,
        double iex,
        double iey,
        double fsx,
        double fsy,
        double fex,
        double fey);

/*
Attribute access methods.

*/
    unsigned int GetFaceNo(void) const { return faceno; }
    unsigned int GetCycleNo(void) const { return cycleno; }
    unsigned int GetSegmentNo(void) const { return segmentno; }
    double GetInitialStartX(void) const { return initialStartX; }
    double GetInitialStartY(void) const { return initialStartY; }
    double GetInitialEndX(void) const { return initialEndX; }
    double GetInitialEndY(void) const { return initialEndY; }
    double GetFinalStartX(void) const { return finalStartX; }
    double GetFinalStartY(void) const { return finalStartY; }
    double GetFinalEndX(void) const { return finalEndX; }
    double GetFinalEndY(void) const { return finalEndY; }
    bool GetInsideAbove(void) const { return insideAbove; }

/*
Generate new ~MSegmentData~ instant in ~rDms~ from current instant, where the
original interval ~origIv~ has been restricted to ~restrIv~.

*/
    void restrictToInterval(Interval<Instant> origIv,
                            Interval<Instant> restrIv,
                            MSegmentData& rDms) const;

/*
Since no other classes are accessing the private attributes of
~MSegmentData~, we declare ~URegion~ as friend. This is shorter,
yet somewhat clumsy than creating attribute access functions.

*/
    // *hm* is there a better solution?
    friend class URegion;

};

/*
1 Class ~TrapeziumSegmentIntersection~

Represents an intersection point as used by the ~intersection~ operator.

The enumeration ~TsiType~ denotes whether an intersection will be used
($TSI\_ENTER$ when a point enters a moving region at the intersection
point and $TSI\_LEAVE$ when the point leaves the moving region at the
intersection point) or will be ignored due to degeneration
related special cases ($TSI\_IGNORE$).

*/
enum TsiType { TSI_ENTER, TSI_LEAVE, TSI_IGNORE };

class TrapeziumSegmentIntersection {
public:
/*
~type~ is the type of the intersection as described by above shown
enumeration. ~x~ and ~y~ are the coordinates of the intersection
and ~t~ the time of the intersection.

*/
    TsiType type;

    double x;
    double y;
    double t;

/*
Compares intersections by their time.

*/
    bool operator<(const TrapeziumSegmentIntersection& tsi) const;
};

/*
1 Class template ~RefinementPartition~

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
class RefinementPartition {
private:
/*
Private attributes:

  * ~iv~: Array (vector) of sub-intervals, which has been calculated from the
    unit intervals of the ~Mapping~ instances.

  * ~vur~: Maps intervals in ~iv~ to indices of original units in first
    ~Mapping~ instance. A $-1$ values indicates that interval in ~iv~ is no
    sub-interval of any unit interval in first ~Mapping~ instance.

  * ~vup~: Same as ~vur~ for second mapping instance.

*/
    vector< Interval<Instant>* > iv;
    vector<int> vur;
    vector<int> vup;

/*
~AddUnit()~ is a small helper method to create a new interval from
~start~ and ~end~ instant and ~lc~ and ~rc~ flags and to add these to the
~iv~, ~vur~ and ~vup~ vectors.

*/
    void AddUnits(const int urPos,
                  const int upPos,
                  const Instant& start,
                  const Instant& end,
                  const bool lc,
                  const bool rc);

public:
/*
The constructor creates the refinement partition from the two ~Mapping~
instances ~mr~ and ~mp~.

Runtime is $O(\max(n, m))$ with $n$ and $m$ the numbers of units in
~mr~ and ~mp~.

*/
    RefinementPartition(Mapping1& mr, Mapping2& mp);

/*
Since the elements of ~iv~ point to dynamically allocated objects, we need
a destructor.

*/
    ~RefinementPartition();

/*
Return the number of intervals in the refinement partition.

*/
    unsigned int Size(void);

/*
Return the interval and indices in original units of position $pos$ in
the refinement partition in the referenced variables ~civ~, ~ur~ and
~up~. Remember that ~ur~ or ~up~ may be $-1$ if interval is no sub-interval
of unit intervals in the respective ~Mapping~ instance.

Runtime is $O(1)$.

*/
    void Get(unsigned int pos,
             Interval<Instant>*& civ,
             int& ur,
             int& up);
};

/*
1 Class ~URegion~

This extension of ~SpatialTemporalUnit<CRegion, 3>~ is fairly simple in its
structure but offers complex methods.

~URegion~ instances can either provide their own storage for moving segments
or they can use moving segments storage in ~MRegion~. This is determined by
the constructor, which is used to create an instance.

*/

class URegion : public SpatialTemporalUnit<CRegion, 3> {
private:
/*
A ~URegion~ instance either maintains its own memory for segments
(~role = NORMAL~) or receives it from its ~MRegion~ instance
(~role = EMBEDDED~).

*/
    enum { NORMAL, EMBEDDED } role;

/*
~segments~ is a pointer to a DBArray, either allocated by the ~URegion~
instance itself or passed by its ~MRegion~ instance. See ~type~ to find
out which is the current case.

~segmentsStartPos~ is the index in ~segments~, where this instances
segments are starting. If the instance allocated its own memory,
~segmentsStartPos~ is $0$.

~segmentsNum~ is the number of segments in the ~URegion~ instance.

*/
    DBArray<MSegmentData> segmentsNormal;
    DBArray<MSegmentData>* segments;
    unsigned int segmentsStartPos;
    unsigned int segmentsNum;

/*
Return the number of segments in ~segments~, which is locate above
~up~ during the interval ~iv~. Before calling this method, it must be
assured that the point does not intersect with one of the segments
during the interval.

*/
    unsigned int Plumbline(const UPoint& up, 
                           const Interval<Instant>& iv) const;

/*
Add new ~UPoint~ unit to ~res~ which reflects that during the
interface $(starttime, endtime, lc, rc)$ the moving point was
from coordinates $(x0, y0)$ to $(x1, y1)$ within this ~URegion~
instance. ~pending~ is used to merge ~UPoint~ instances, if possible.

*/

    void RestrictedIntersectionAddUPoint(MPoint& res,
                                         double starttime,
                                         double endtime,
                                         bool lc,
                                         bool rc,
                                         double x0,
                                         double y0,
                                         double x1,
                                         double y1,
                                         UPoint*& pending) const;

/*
Collect 'normal' intersections between ~UPoint~ unit ~rUp~ and moving segment
~rDms~, which occured in interval ~iv~ at point $(ip1x, ip1y)$ at the time
$ip1t$. The intersections are written to the vector ~vtsi~.

*/
    void RestrictedIntersectionFindNormal(
        const Interval<Instant>& iv,
        const UPoint& rUp,
        MSegmentData& rDms,
        bool& ip1present,
        double& ip1x,
        double& ip1y,
        double& ip1t,
        vector<TrapeziumSegmentIntersection>& vtsi) const;

/*
Handle intersection line between ~UPoint~ unit ~rUp~ and a moving segment,
which lies in the same plane as the moving segment and occured in the
intervall ~iv~. $(ip1x, ip1y)$ is the
first intersection point, $(ip2y, ip2y)$ is the second intersection point,
~ip1present~ and ~ip2present~ denote whether both points are present
(they may be reduced to a single point) The intersections are written to
the vector ~vtsi~.

*/
    void RestrictedIntersectionFindInPlane(
        const Interval<Instant>& iv,
        const UPoint& rUp,
        bool& ip1present,
        double& ip1x,
        double& ip1y,
        double& ip1t,
        bool& ip2present,
        double& ip2x,
        double& ip2y,
        double& ip2t,
        vector<TrapeziumSegmentIntersection>& vtsi) const;

/*
Find all intersections between the ~UPoint~ unit ~up~ and the segments
of this ~URegion~ instance during the interval ~iv~. Write intersections
to vector ~vtsi~.

*/
    void RestrictedIntersectionFind(
        const UPoint& up,
        const Interval<Instant>& iv,
        vector<TrapeziumSegmentIntersection>& vtsi) const;

/*
For the intersections between ~up~ and this ~URegion~ instance's segments
stored in ~vtsi~,
match two pairs of intersection points, which have been previously calculated
by ~RestrictedIntersectionFind()~, so that each pair contains an intersection
point where the ~URegion~ instance is being entered and an intersection point
where it is left. From each pair, construct a ~UPoint~ unit and add it to
~res~. ~pending~ is used to merge ~UPoint~ instances, if possible.

*/
    bool RestrictedIntersectionProcess(
        const UPoint& up,
        const Interval<Instant>& iv,
        vector<TrapeziumSegmentIntersection>& vtsi,
        MPoint& res,
        UPoint*& pending) const;

/*
Handle intersections in ~vtsi~ with degenerated segments.

*/
    void RestrictedIntersectionFix(
        vector<TrapeziumSegmentIntersection>& vtsi) const;

public:
    URegion() { }

/*
Constructor, which allocated its own storage for segments.

*/
    URegion(const Interval<Instant>& interval);

/*
Constructor, which receives pointer to storage for segments from
~MRegion~ instance. The ~URegion~ instance will store it segments
starting at ~pos~ in ~segs~.

*/
    URegion(const Interval<Instant>& interval,
            DBArray<MSegmentData>* segs,
            unsigned int pos);

/*
Constructor, which receives storage for its segments from ~MRegion~
instance and creates a constant unit for the specied interval from the
provided ~CRegion~ instance.
The ~URegion~ instance will store it segments
starting at ~pos~ in ~segs~.

*/
    URegion(const Interval<Instant>& interval,
            const CRegion& region,
            DBArray<MSegmentData>* segs,
            unsigned int pos);

/*
Returns ~true~ if instance does not have its moving segments storage, ie.
uses a ~MRegion~ instance's storage.

*/
    bool IsEmbedded(void);


/*
Required for ~OpenURegion()~.
Sets ~segments~ to address of ~segmentsNormal~ after instance has been created.

*/
    void SetMSegmentData(void);

/*
Required for ~OpenMRegion()~. Set the storage for segments after the
instance has been created.

*/
    void SetMSegmentData(DBArray<MSegmentData>* s);

/*
Adds a segment to this ~URegion~ instance.

~cr~ is a ~CRegion~ instance, which is used to check whether the ~URegion~
instance represents a valid region in the middle of its interval, when no
degeneration can occur. All segments added to the ~URegion~ instance are
added as non-moving segments to ~cr~ too.

~rDir~ is used to check the direction of a cylce, which is required to
finalise the ~insideAbove~ attributes of the segments.

Both ~cr~ and ~rDir~ are checked outside the methods of ~URegion~, they
are just filled by ~AddSegment()~.

~faceno~, ~cycleno~, ~segmentno~ and ~pointno~ are the obvious indices
in the ~URegion~ unit's segment.

~intervalLen~ is the length of the interval covered by this ~URegion~
instance.

~start~ and ~end~ contain the initial and final positions of the segment
in list representation, which will be added to the ~URegion~ instance

~AddSegment()~ performs numerous calculations and checks. See the method
description below for details.

*/
    bool AddSegment(CRegion& cr,
                    CRegion& rDir,
                    unsigned int faceno,
                    unsigned int cycleno,
                    unsigned int segmentno,
                    unsigned int partnerno,
                    double intervalLen,
                    ListExpr start,
                    ListExpr end);

/*
Set the value of the ~insideAbove~ attribut of segment as position ~pos~ to
the value ~insideAbove~.

*/

    void SetSegmentInsideAbove(int pos, bool insideAbove);

/*
Get number of segments, get specific segment, write specific segment.

*/
    int GetSegmentsNum(void) const;
    void GetSegment(int pos, const MSegmentData*& dms) const;
    void PutSegment(int pos, const MSegmentData& dms);

/*
Calculate ~MPoint~ instance ~res~ from the intersection ~up~ and this
~URegion~ unit, restricted to interval ~iv~ instead of ~up~'s or this
instances full intervals. ~pending~ is used to try to merge multiple
units.

*/
    void RestrictedIntersection(const UPoint& up,
                                const Interval<Instant>& iv,
                                MPoint& res,
                                UPoint*& pending) const;

/*
Destroy the ~URegion~ unit. If it has its own segments memory, free it.

*/
    void Destroy(void);

/*
Returns the ~CRegion~ value of this ~URegion~ unit at instant ~t~
in ~result~.

*/
    virtual void TemporalFunction(const Instant& t, CRegion& result) const;

/*
~At()~, ~Passes()~ and ~BoundingBox()~ are not yet implemented. Stubs
required for to make this class non-abstract.

*/
    virtual bool At(const CRegion& val, TemporalUnit<CRegion>& result) const;
    virtual bool Passes(const CRegion& val) const;

    virtual const Rectangle<3> BoundingBox() const;

/*
~DBArray~ access. These make sense on ~URegion~ instances with own segments
storage only, and will run into failed assertions for other instances.

*/
    int NumOfFLOBs() const;
    FLOB *GetFLOB(const int i);

/*
Required for Algebra integration. Not implemented either.

*/
    virtual URegion* Clone() const;
    virtual void CopyFrom(const StandardAttribute* right);
};

/*
1 Class ~MRegion~

Represents a moving region. It contains an array of segments, which is
references by its ~URegion~ units, which do not have its own storage for
segments.

*/

class MRegion : public Mapping<URegion, CRegion> {
private:
/*
The array with the segments.

*/
    DBArray<MSegmentData> msegmentdata;

/*
Calculates the intersection between this ~mp~ instance and ~mp~ based
on the refinement partition ~rp~. The result goes into ~res~.

*/
    void IntersectionRP(
        MPoint& mp,
        MPoint& res,
        RefinementPartition<MRegion, MPoint, URegion, UPoint>& rp);

/*
Add a ~UBool~ unit to ~res~ for interval $(starttime, endtime, lc, rc)$
with value ~value~. ~pending~ is used to merge units to reduce their
number. ~prev~ and ~prev\_c~ are used to assure continuous coverage.

*/
    void InsideAddUBool(MBool& res,
                        double starttime,
                        double endtime,
                        bool lc,
                        bool rc,
                        bool value,
                        double& prev,
                        bool& prev_c,
                        UBool*& pending);

public:
    MRegion() { }

/*
Create ~MRegion()~ instance, which is prepared for ~n~ units.

*/
    MRegion(const int n);

/*
Create ~MRegion()~ instance, determine its units by the units in ~mp~ and
set each unit to the constant value of ~r~.

*/
    MRegion(MPoint& mp, CRegion& r);

/*
~DBArray~ access.

*/
    int NumOfFLOBs() const;
    FLOB *GetFLOB(const int i);

/*
Calculate intersection between ~mp~ and this ~MRegion~ isntance and
return result in ~res~.

*/
    void Intersection(MPoint& mp, MPoint& res);

/*
Check when ~mp~ is inside this ~MRegion~ instance and create ~res~
accordingly.

*/
    void Inside(MPoint& mp, MBool& res);

/*
Calculate region traversed by ~MRegion~ instant in ~res~.

*/
    void Traversed(CRegion& res);

/*
Get ~CRegion~ value ~result~ at instant ~t~.

*/
    void AtInstant(Instant& t, Intime<CRegion>& result);

/*
Get ~CRegion~ value ~result~ at initial and final instants.

*/
    void Initial(Intime<CRegion>& result);
    void Final(Intime<CRegion>& result);

/*
Friend access for ~InMRegion()~ makes live easier.

*/
    friend Word InMRegion(const ListExpr typeInfo,
                          const ListExpr instance,
                          const int errorPos,
                          ListExpr& errorInfo,
                          bool& correct);

/*
Get ~URegion~ unit ~i~ from this ~MRegion~ instance and return it in ~ur~.

*/
    void Get(const int i, URegion& ur);

/*
For unit testing only.

*/
    int Unittest2(int pos);

};

#endif // _MOVING_REGION_ALGEBRA_H_
