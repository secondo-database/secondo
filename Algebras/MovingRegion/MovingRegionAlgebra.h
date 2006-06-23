/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] The ~MovingRegionAlgebra~

December 2005, initial version created by Holger M[ue]nx for bachelor
thesis with Prof. Dr. G[ue]ting, Fachbereich Informatik,
Fernuniversit[ae]t Hagen.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January-June 2006, various bugfixes and improvements by Holger M[ue]nx,
with the help of Victor Almeida and Thomas Behr.

[TOC]

1 Introduction

This file contains the class definitions of ~MSegmentData~, 
~RefinementPartitionOrig~, 
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
1 Supporting classes and class template

1.1 Class ~MSegmentData~

The class ~MSegmentData~ describes a moving segment within a region unit.
Beside start and end points of the segments for the initial and final
instant, the class contains attributes to describe the behaviour during
the initial and final instant and its orientation with the region unit.

1.1.1 Enumeration

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
enum DegenMode { 
    DGM_UNKNOWN,
    DGM_NONE,
    DGM_IGNORE,
    DGM_INSIDEABOVE,
    DGM_NOTINSIDEABOVE };

class MSegmentData {
/*
1.1.1 Private attributes

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

 public:

/*
1.1.1 Constructors

The default constructors does nothing.

*/
    MSegmentData() { }

/*
This constructor sets the ~faceno~, ~cycleno~, ~segmentno~, ~insideAbove~,
~initialStartX~, ~initialStartY~, ~initialEndX~, ~initialEndY~,
~finalStartX~, ~finalStartY~, ~finalEndX~ and ~finalEndY~ attributes
according to the parameters.

~degeneratedInitialNext~ and ~degeneratedFinalNext~ are initialised with
$-1$, ~degeneratedInitial~ and ~degeneratedFinal~ with ~DGM\_UNKNOWN~.

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
1.1.1 Attribute read access methods

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
    bool GetPointInitial(void) const { return pointInitial; }
    bool GetPointFinal(void) const { return pointFinal; }
    DegenMode GetDegeneratedInitial(void) const { 
        return degeneratedInitial; }
    DegenMode GetDegeneratedFinal(void) const { 
        return degeneratedFinal; }
    int GetDegeneratedInitialNext(void) const { 
        return degeneratedInitialNext; }
    int GetDegeneratedFinalNext(void) const { 
        return degeneratedFinalNext; }

/*
1.1.1 Attribute write access methods

*/
    void SetFinalStartX(const double v) { finalStartX = v; }
    void SetFinalStartY(const double v) { finalStartY = v; }
    void SetFinalEndX(const double v) { finalEndX = v; }
    void SetFinalEndY(const double v) { finalEndY = v; }
    void SetInsideAbove(const bool ia) { insideAbove = ia; }
    void SetPointInital(const bool p) { pointInitial = p; }
    void SetPointFinal(const bool p) { pointFinal = p; }
    void SetDegeneratedInitial(const DegenMode dm) { 
        degeneratedInitial = dm; }
    void SetDegeneratedFinal(const DegenMode dm) { 
        degeneratedFinal = dm; }
    void SetDegeneratedInitialNext(const int dn) { 
        degeneratedInitialNext = dn; }
    void SetDegeneratedFinalNext(const int dn) { 
        degeneratedFinalNext = dn; }

/*
1.1.1 Other methods

Generate new ~MSegmentData~ instant in ~rDms~ from current instant, where the
original interval ~origIv~ has been restricted to ~restrIv~.

*/
    void restrictToInterval(
        Interval<Instant> origIv,
        Interval<Instant> restrIv,
        MSegmentData& rDms) const;

/*
Return ~string~ representation of moving segment.

*/
    string ToString(void) const;
};

/*
1.1 Class ~TrapeziumSegmentIntersection~

Represents an intersection point as used by the ~intersection~ operator.

1.1.1 Enumeration

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
1.1.1 Public attributes:

  * ~type~ is the type of the intersection as described by above shown
    enumeration. 

  * ~x~ and ~y~ are the coordinates of the intersection and ~t~ the 
    time of the intersection.

Due to the simplicity of this class, no private attributes and attribute
access methods have been implemented.

*/
    TsiType type;

    double x;
    double y;
    double t;

/*
1.1.1 Operators

Compares intersections by their time.

*/
    bool operator<(const TrapeziumSegmentIntersection& tsi) const;
};

/*
1.1 Class template ~RefinementPartitionOrig~

The class template is called ~RefinementPartitionOrig~ and not
~RefinementPartition~ because J[ue]rgen Schmidt decided to implement
his own version of ~RefinementPartition~ in his ~TemporalLiftedAlgebra~,
resulting in conflicts. The postfix ~Orig~ indicates who was here first!
:-)

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
class RefinementPartitionOrig {
private:
/*
1.1.1 Private attributes:

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
1.1.1 Private methods

~AddUnit()~ is a small helper method to create a new interval from
~start~ and ~end~ instant and ~lc~ and ~rc~ flags and to add these to the
~iv~, ~vur~ and ~vup~ vectors.

*/
    void AddUnits(
        const int urPos,
        const int upPos,
        const Instant& start,
        const Instant& end,
        const bool lc,
        const bool rc);

public:
/*
1.1.1 Constructor and destructor

The constructor creates the refinement partition from the two ~Mapping~
instances ~mr~ and ~mp~.

Runtime is $O(\max(n, m))$ with $n$ and $m$ the numbers of units in
~mr~ and ~mp~.

*/
    RefinementPartitionOrig(Mapping1& mr, Mapping2& mp);

/*
Since the elements of ~iv~ point to dynamically allocated objects, we need
a destructor.

*/
    ~RefinementPartitionOrig();

/*
1.1.1 Methods

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
    void Get(
        unsigned int pos,
        Interval<Instant>*& civ,
        int& ur,
        int& up);
};

/*
1 Class ~IRegion~

The code for this data type is fairly simple because it can mostly rely on the
instantiation of the class template ~Intime~ with class ~CRegion~. Just a 
number of specialized constructors and methods must be overwritten to assure
that the ~DBArray~ in ~CRegion~ is properly handled. ~Intime~ itself does
not care about whether its ~value~ attribute is of a class with ~DBArray~
attributes or not.

*/

class IRegion : public Intime<CRegion> {
public:
/*
1.1 Constructors

The default constructor does nothing.

*/
    IRegion(void) {}

/*
Create a new ~IRegion~ instance without setting attribute default values.
The ~value~ attribute is properly initialized with an empty region, though,
to assure that its ~DBArray~ is properly initialized.

*/
    IRegion(bool dummy);

/*
Create a new ~IRegion~ instance and initialize it with the value of ~ir~.

*/
    IRegion(const IRegion& ir);

/*
1.1 Methods for algebra integration

Clone this ~IRegion~ instance and return a pointer to the new instance.

*/
    IRegion* Clone(void) const;

/*
~DBArray~ access methods. These do use the ~value~ attributes ~DBArray~.

*/
    int NumOfFLOBs(void) const;
    FLOB* GetFLOB(const int i);
};

/*
1 Class ~URegionEmb~

~URegion~ and ~MRegion~ use the common class ~URegionEmb~ due to the following
situation: 

Moving regions contain a variable number of region units, and each
region unit contains a variable number of moving segments. In theory, this
requires a two-level data structure of DBArrays, which is not 
supported by SECONDO. Instead, a moving region object of class ~MRegion~
contains two ~DBArray~ instances. The first ~DBArray~ instance contains all
region units, represented as objects of class ~URegionEmb~. The second
~DBArray~ contains the moving segments of all regions units in a moving
region. Each ~URegionEmb~ object knows the start index and the total number
of its moving segments in the second ~DBArray~ instance. Methods of 
~URegionEmb~, which need to access segments, receive the second ~DBArray~ 
instance as explicit parameter.

Since region units, which are not contained in moving regions, need to be
available as SECONDO datatype too, the class ~URegion~ has been implemented.
It has a ~DBArray~ to store moving segments and an instance of ~URegionEmb~
as attributes. This facilitates reusing the ~URegionEmb~ functionality for
standalone region unit values without duplication of code.

*/

class URegionEmb {
private:
/*
1.1 Private attributes

  * ~segmentsStartPos~ is the index in moving segments array, where this 
    instances segments are starting.

  * ~segmentsNum~ is the number of segments in the ~URegion~ instance.

  * ~bbox~ contains the bounding box of the region unit.

*/
    unsigned int segmentsStartPos;
    unsigned int segmentsNum;

    Rectangle<3> bbox;

/*
1.1 Helper methods for ~RestrictedIntersection()~

Return the number of segments in ~segments~, which is locate above
~up~ during the interval ~iv~. Before calling this method, it must be
assured that the point does not intersect with one of the segments
during the interval.

*/
    unsigned int Plumbline(
        const DBArray<MSegmentData>* segments,
        const UPoint& up, 
        const Interval<Instant>& iv) const;


/*
Add new ~UPoint~ unit to ~res~ which reflects that during the
interface $(starttime, endtime, lc, rc)$ the moving point was
from coordinates $(x0, y0)$ to $(x1, y1)$ within this ~URegionEmb~
instance. ~pending~ is used to merge ~UPoint~ instances, if possible,
but only if ~merge~ is ~true~.

*/
    void RestrictedIntersectionAddUPoint(
        MPoint& res,
        double starttime,
        double endtime,
        bool lc,
        bool rc,
        double x0,
        double y0,
        double x1,
        double y1,
        UPoint*& pending,
        bool merge) const;

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
For the intersections between ~up~ and this ~URegionEmb~ instance's segments
stored in ~vtsi~,
match two pairs of intersection points, which have been previously calculated
by ~RestrictedIntersectionFind()~, so that each pair contains an intersection
point where the ~URegionEmb~ instance is being entered and an intersection 
point where it is left. From each pair, construct a ~UPoint~ unit and add it to
~res~. ~pending~ is used to merge ~UPoint~ instances, if possible, but only
if ~merge~ is ~true~.

*/
    bool RestrictedIntersectionProcess(
        const UPoint& up,
        const Interval<Instant>& iv,
        vector<TrapeziumSegmentIntersection>& vtsi,
        MPoint& res,
        UPoint*& pending,
        bool merge) const;

/*
Handle intersections in ~vtsi~ with degenerated segments.

*/
    void RestrictedIntersectionFix(
        vector<TrapeziumSegmentIntersection>& vtsi) const;

/*
Find all intersections between the ~UPoint~ unit ~up~ and the segments
of this ~URegionEmb~ instance during the interval ~iv~. Write intersections
to vector ~vtsi~.

*/
    void RestrictedIntersectionFind(
        const DBArray<MSegmentData>* segments,
        const UPoint& up,
        const Interval<Instant>& iv,
        vector<TrapeziumSegmentIntersection>& vtsi) const;

public:
/*
1.1 Public attributes

~timeInterval~ contains the interval of the unit. While a private 
attribute with proper public access methods would be more clean, we
leave this attribute public to mimic a ~Unit~'s behaviour.

*/
    Interval<Instant> timeInterval;

/*
1.1 Constructors

The default constructor does nothing.

*/
    URegionEmb() { }

/*
Constructor, which creates an empty unit for the specified interval.
The ~URegionEmb~ instance will store it segments starting at ~pos~
in the moving segments array.

*/
    URegionEmb(
        const Interval<Instant>& tiv,
        unsigned int pos);

/*
Constructor, which creates a constant unit for the specified interval from the
provided ~CRegion~ instance. The ~URegionEmb~ instance will store it segments
starting at ~pos~ in ~segments~.

*/
    URegionEmb(
        DBArray<MSegmentData>* segments,
        const Interval<Instant>& iv,
        const CRegion& region,
        unsigned int pos);

/*
1.1 Moving segments access methods

Get number of segments, get specified segment, write specified segment.

*/
    int GetSegmentsNum(void) const;
    void GetSegment(
        const DBArray<MSegmentData>* segments,
        int pos, 
        const MSegmentData*& dms) const;
    void PutSegment(
        DBArray<MSegmentData>* segments,
        int pos, 
        const MSegmentData& dms);

/*
Adds a segment to this ~URegionEmb~ instance.

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
    bool AddSegment(
        DBArray<MSegmentData>* segments,
        CRegion& cr,
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
the value ~insideAbove~. This is required by function ~InURegionEmb()~.

*/
    void SetSegmentInsideAbove(
        DBArray<MSegmentData>* segments,
        int pos, 
        bool insideAbove);

/*
1.1 Methods for database operators

Calculate ~MPoint~ instance ~res~ from the intersection ~up~ and this
~URegion~ unit, restricted to interval ~iv~ instead of ~up~'s or this
instances full intervals. ~pending~ is used to to merge multiple
units but only if ~merge~ is ~true~.

*/
    void RestrictedIntersection(
        const DBArray<MSegmentData>* segments,
        const UPoint& up,
        const Interval<Instant>& iv,
        MPoint& res,
        UPoint*& pending,
        bool merge) const;

/*
Returns the ~CRegion~ value of this ~URegionEmb~ unit at instant ~t~
in ~result~.

*/
    void TemporalFunction(
        const DBArray<MSegmentData>* segments,
        const Instant& t, 
        CRegion& result) const;

/*
Return the bounding box of the region unit. This is an $O(1)$ operation
since the bounding box is calculated when the region unit is created.

*/
    const Rectangle<3> BoundingBox() const;

/*
Sets the bounding box.

Use it carefully to avoid inconsistencies.

*/
    void SetBBox(Rectangle<3> box) { bbox = box; }

/*
1.1 Methods to act as unit in ~Mapping~ template.

*/
    bool IsValid(void) const;

    bool Disjoint(const URegionEmb& ur) const;
    bool TU_Adjacent(const URegionEmb& ur) const;
    bool operator==(const URegionEmb& ur) const;
    bool Before(const URegionEmb& ur) const;

/*
This method is not implemented since it is not required by the current
version of the algebra but is required by the ~Mapping~ template.. Calls 
will run into a failed assertion.

*/
    bool Compare(const URegionEmb* ur) const;
};

/*
1 Class ~URegion~

Instances of class ~URegion~ represent SECONDO ~uregion~ objects.

*/

class URegion : public SpatialTemporalUnit<CRegion, 3> {
private:
/*
1.1 Private attributes

  * ~segments~ contains the moving segments of this region unit.

  *  ~uremb~ is an instance of ~URegionEmb~, which will be used to provide most
     of the functionality. Please note that the ~uremb~ is using ~segments~ 
     through a pointer to ~segments~, which is explicitely passed to ~uremb~'s
     methods.

*/
    DBArray<MSegmentData> segments;
    URegionEmb uremb;

public:
/*
1.1 Constructors

The default constructor does nothing.

*/
    URegion() { }

/*
Constructor, which creates an region unit with ~segments~ prepared for ~n~
elements (use ~0~ for ~n~ to creatw an empty region unit).

*/
    URegion(unsigned int n);

/*
1.1 Attribute access method

Set and get the ~uremb~ attribute. Required for function ~InURegion()~.

*/
    void SetEmbedded(const URegionEmb* p) { uremb = *p; }
    URegionEmb* GetEmbedded(void) { return &uremb; }

/*
1.1 Methods for database operators

Returns the ~CRegion~ value of this ~URegion~ unit at instant ~t~
in ~result~.

*/
    virtual void TemporalFunction(const Instant& t, 
                                  CRegion& result) const;

/*
~At()~ and ~Passes()~ are not yet implemented. Stubs
required to make this class non-abstract.

*/
    virtual bool At(const CRegion& val, 
                    TemporalUnit<CRegion>& result) const;
    virtual bool Passes(const CRegion& val) const;

/*
Return the bounding box of the region unit. This is an $O(1)$ operation
since the bounding box is calculated when the region unit is created.

*/
    virtual const Rectangle<3> BoundingBox() const;

/*
1.1 Methods for algebra integration

~DBArray~ access. These make sense on ~URegion~ instances with own segments
storage only, and will run into failed assertions for other instances.

*/
    int NumOfFLOBs(void) const;
    FLOB *GetFLOB(const int i);

/*
Clone ~URegion~ instance. Please note that resulting instance will have
its own moving segments storage, even if the cloned instance has not.

*/
    virtual URegion* Clone(void) const;

/*
Copy ~URegion~ instance. Please note that resulting instance will have
its own moving segments storage, even if the copied instance has not.

*/
    virtual void CopyFrom(const StandardAttribute* right);
};

/*
1 Class ~MRegion~

Represents a moving region. It contains an array of segments, which is
references by its ~URegion~ units, which do not have its own storage for
segments.

*/

class MRegion : public Mapping<URegionEmb, CRegion> {
private:
/*
1.1 Private attributes

  * The array with the segments.

*/
    DBArray<MSegmentData> msegmentdata;

/*
1.1 Private methods

Calculates the intersection between this ~mp~ instance and ~mp~ based
on the refinement partition ~rp~. The result goes into ~res~. If ~merge~
is ~true~, the resulting ~URegion~ instances are merged, if possible.

*/
    void IntersectionRP(
        MPoint& mp,
        MPoint& res,
        RefinementPartitionOrig<MRegion, 
                                MPoint, 
                                URegionEmb, 
                                UPoint>& rp,
        bool merge);

/*
Add a ~UBool~ unit to ~res~ for interval $(starttime, endtime, lc, rc)$
with value ~value~. ~pending~ is used to merge adjacent units with same
value to reduce the total number of units.
~prev~ and ~prev\_c~ are used to assure continuous coverage.

*/
    void InsideAddUBool(
        MBool& res,
        double starttime,
        double endtime,
        bool lc,
        bool rc,
        bool value,
        double& prev,
        bool& prev_c,
        UBool*& pending);

public:
/*
1.1 Constructors

The default constructor does nothing.

*/
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
Constructs a contiunues moving region from the parameters. ~dummy~ is not
used.

*/
    MRegion(MPoint& mp, CRegion& r,int dummy);

/*
1.1 Attribute access methods

Get ~URegion~ unit ~i~ from this ~MRegion~ instance and return it in ~ur~.

*/
    void Get(const int i, const URegionEmb*& ur) const;

/*
Allow read-only access to ~msegmentdata~.

*/
    const DBArray<MSegmentData>* GetMSegmentData(void);

/*
1.1 Methods for database operators

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
#ifdef MRA_TRAVERSED
    void Traversed(CRegion& res);
#endif // MRA_TRAVERSED

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
1.1 Methods for algebra integration

~DBArray~ access.

*/
    int NumOfFLOBs(void) const;
    FLOB *GetFLOB(const int i);

/*
Clone ~MRegion~ instance.

*/
    MRegion* Clone(void) const;

/*
Copy ~MRegion~ instance.

*/
    void CopyFrom(const StandardAttribute* right);

/*
1.1 Unit testing

*/
#ifdef MRA_UNITTEST
    bool Unittest2(int pos);
#endif // MRA_UNITTEST

/*
1.1 Other declarations

Friend access for ~InMRegion()~ and ~OpenMRegion()~ makes live easier.

*/
    friend Word InMRegion(
        const ListExpr typeInfo,
        const ListExpr instance,
        const int errorPos,
        ListExpr& errorInfo,
        bool& correct);
};

#endif // _MOVING_REGION_ALGEBRA_H_
