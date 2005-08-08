/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of MovingRegion algebra

[TOC]

1 Introduction

Some nice introduction should be found here.

*/

// code requiring attention is marked with *hm*!! 

/*

Intersection of two trapezoids with parallel sides parallel to (x, y)-plane.

Points 
$A = (a_1, a_2, a_3)$,
$B = (b_1, b_2, a_3)$,
$C = (c_1, c_2, c_3)$,
$D = (d_1, d_2, c_3)$,
$E = (e_1, e_2, e_3)$,
$F = (f_1, f_2, e_3)$,
$G = (g_1, g_2, g_3)$ and
$H = (h_1, h_2, g_3).$

Trapezoids
$T_1 = (A, B, C, D)$ and 
$T_2 = (E, F, G, H)$.

$T_1$ is in plane
$A + (B-A) \cdot X + (C-A) \cdot Y$.
$T_2$ is in plane
$E + (F-E) \cdot S + (G-E) \cdot T$.

Intersection of both planes is the solution of linear systems of equation with 
coefficients

\begin{eqnarray}
\begin{array}{ccc}
\begin{array}{cccc}
b_1-a_1 & c_1-a_1 & f_1-e_1 & g_1-e_1 \\
b_2-a_2 & c_2-a_2 & f_2-e_2 & g_2-e_2 \\
      0 & c_3-a_3 &       0 & g_3-e_3
\end{array} & \left| \begin{array}{c}
e_1-a_1 \\
e_2-a_2 \\
e_3-a_3 
\end{array} \right.
\end{array}
\nonumber \end{eqnarray}

The following cases need to be examined:

\begin{enumerate}
\item
There are no solutions. In this case, the two planes spanned by $T_1$ and 
$T_2$ are parallel and do not touch each other.

\item
The solution is a line. In this case, if the line is both within $T_1$ and
$T_2$, the two trapezoids intersect.

\item
Otherwise, both trapezoids are in the same plane. In this case, it must be
checked if both trapezoids overlap, which is very simple.
\end{enumerate}

*/




/*

The following approach is used to check the validity of the points in
each unit of a moving region:

If $(t_1, t_2, true, true)$ is the interval of the unit,
the moving region is allowed to degenerate in instants $t_1$ and $t_2$. If
the unit's interval is half-open, ie.\ $(t_1, t_2, false, true)$ or 
$(t_1, t_2, true, false)$, the moving region is allowed to generate in 
instants $t_1$ or $t_2$. However, for all all instants $t$ with 
$t_1 < t < t_2$, the moving region must not degenerate. 

Two distinct cases are considered:

\begin{enumerate}
\item
For point intervals $(t, t, true, true)$, the moving region must not
degenerate in instant $t$ and the unit is checked like a spatial region.

\item
For non-point intervals $(t_1, t_2, lc, rc)$ with $t_1 \ne t_2$ and 
$lc, rc \in \{true, false\}$, the value of the unit at instant 
$t = (t2-t1)/2$ is calculated. As the moving region must not degenerate
at instant $t$, this value is checked like a spatial region. If successful,
it is checked that no trapezoid, which is spanned by a moving segment in
the unit, is intersecting or touching in its interior any other moving
segment.
\end{enumerate}

The spatial region in both cases is used to calculate the value of the
~insideLeftOrAbove~ attribute of each segment.

*/


/*
1 Defines and includes

*/

#include <queue>
#include <stdexcept>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "TemporalAlgebra.h"

#include "DateTime.h"
using namespace datetime;

static NestedList* nl;
static QueryProcessor* qp;

const bool MRA_DEBUG = true;

static bool nearlyEqual(double a, double b) {
    return abs(a-b) <= 0.00001;
}

// ************************************************************************

/*
1 Data type ~iregion~

*/

// *hm* Verify list representation

static ListExpr IRegionProperty() {
    if (MRA_DEBUG) cerr << "IRegionProperty() called" << endl;
    
    ListExpr example = nl->TextAtom();
    nl->AppendText(example,
		   "(\"2003-01-10\" (((3 0)(10 1)(3 1))((3.1 0.1)"
		   "(3.1 0.9)(6 0.8))))");
    return 
	nl->TwoElemList(
            nl->FourElemList(
		nl->StringAtom("Signature"),
		nl->StringAtom("Example Type List"),
		nl->StringAtom("List Rep"),
		nl->StringAtom("Example List")),
            nl->FourElemList(
		nl->StringAtom("-> UNIT"),
		nl->StringAtom("(iregion)"),
		nl->StringAtom("(<instant> <region>)"),
		example));
}

static bool CheckIRegion(ListExpr type, ListExpr& errorInfo) {
    if (MRA_DEBUG) cerr << "CheckIRegion() called" << endl;

    return nl->IsEqual(type, "iregion");
}

static TypeConstructor iregion(
    "iregion",
    IRegionProperty,
    OutIntime<CRegion, OutRegion>, 
    InIntime<CRegion, InRegion>,
    0, 0, // SaveToList, RestoreFromList
    CreateIntime<CRegion>,
    DeleteIntime<CRegion>,
    0, 0, // open, save
    CloseIntime<CRegion>,
    CloneIntime<CRegion>,
    CastIntime<CRegion>,
    SizeOfIntime<CRegion>,
    CheckIRegion,
    0, // predef. pers. function for model
    TypeConstructor::DummyInModel,
    TypeConstructor::DummyOutModel,
    TypeConstructor::DummyValueToModel,
    TypeConstructor::DummyValueListToModel );

// ************************************************************************

/*
1 Data type ~uregion~

1.1 Helper class ~MPointData~ 

*/

class MSegmentData {
private:
    unsigned int faceno;
    unsigned int cycleno;
    unsigned int segmentno;

    bool insideLeftOrAbove;
    bool degeneratedInitial;
    bool degeneratedFinal;

    double initialStartX;
    double initialStartY;
    double initialEndX;
    double initialEndY;

    double finalStartX;
    double finalStartY;
    double finalEndX;
    double finalEndY;

public:
    MSegmentData() {
        cerr << "MSegmentData::MSegmentData() #1 called" << endl;
    }

    MSegmentData(unsigned int fno, 
		 unsigned int cno, 
		 unsigned int sno,
		 bool iloa,
		 bool di,
		 bool df,
		 double il,
		 double isx,
		 double isy,
		 double iex,
		 double iey,
		 double fsx,
		 double fsy,
		 double fex,
		 double fey) :
        faceno(fno),
        cycleno(cno),
        segmentno(sno),
	insideLeftOrAbove(iloa),
	degeneratedInitial(di),
	degeneratedFinal(df),
	initialStartX(isx),
	initialStartY(isy),
	initialEndX(iex),
	initialEndY(iey),
	finalStartX(fsx),
	finalStartY(fsy),
	finalEndX(fex),
	finalEndY(fey)  {
	if (MRA_DEBUG)
	    cerr << "MSegmentData::MSegmentData() #2 called "
		 << faceno
		 << " "
		 << cycleno
		 << " "
		 << segmentno
		 << " "
		 << insideLeftOrAbove
		 << " "
		 << degeneratedInitial
		 << " "
		 << degeneratedFinal
		 << " "
		 << initialStartX
		 << " "
		 << initialStartY
		 << " "
		 << initialEndX
		 << " "
		 << initialEndY
		 << " "
		 << finalStartX
		 << " "
		 << finalStartY
		 << " "
		 << finalEndX
		 << " "
		 << finalEndY
		 << endl;

	double idx = (iex-isx)/il;
	double idy = (iey-isy)/il;
	double fdx = (fex-fsx)/il;
	double fdy = (fey-fsy)/il;

	if (!nearlyEqual(idx, fdx) || !nearlyEqual(idy, fdy))
	    throw invalid_argument("initial and final segment not colinear");
    }
};

/*
1.1 Class ~URegion~

*/

class URegion : public SpatialTemporalUnit<CRegion, 3> {
private:
    // *hm* Confirm whether two roles of URegion are really required
    enum { UNDEF, NORMAL, EMBEDDED } role;

    DBArray<MSegmentData>* segments;
    unsigned int segmentsStartPos;
    unsigned int segmentsNum;

public:
    URegion() {
        if (MRA_DEBUG) cerr << "URegion::URegion() #1 called" << endl;
    }

    URegion(Interval<Instant>& interval,
	    DBArray<MSegmentData>* segs,
	    unsigned int pos) : 
        SpatialTemporalUnit<CRegion, 3>(interval),
	role(EMBEDDED),
        segments(segs),
	segmentsStartPos(pos),
	segmentsNum(0) {
        if (MRA_DEBUG) 
            cerr << "URegion::URegion() #2 called" << endl;
    }

    URegion(Interval<Instant>& interval) :
	SpatialTemporalUnit<CRegion, 3>(interval),
	role(NORMAL),
        segments(new DBArray<MSegmentData>(0)),
	segmentsStartPos(0),
	segmentsNum(0) {
        if (MRA_DEBUG) 
            cerr << "URegion::URegion() #2 called" << endl;
    }

    bool AddSegment(unsigned int faceno,
		    unsigned int cycleno,
		    unsigned int segmentno,
		    double intervalLen,
		    ListExpr start,
		    ListExpr end);
    int GetSegmentsNum(void);
    void GetSegment(int pos, MSegmentData& dms);

    void Destroy(void);

    virtual void TemporalFunction(Instant& t, CRegion& result);
    virtual bool Passes(CRegion& val);
    virtual bool At(CRegion& val, TemporalUnit<CRegion>& result);

    virtual const Rectangle<3> BoundingBox() const;

    virtual URegion* Clone();
    virtual void CopyFrom(StandardAttribute* right);
};

void URegion::Destroy(void) {
    if (MRA_DEBUG) cerr << "URegion::Destroy() called" << endl;

    assert(role == NORMAL || role == EMBEDDED);

    if (role == NORMAL) segments->Destroy();
}

URegion* URegion::Clone(void) {
    if (MRA_DEBUG) cerr << "URegion::Clone() called" << endl;

    assert(false);
}

void URegion::CopyFrom(StandardAttribute* right) {
    if (MRA_DEBUG) cerr << "URegion::CopyFrom() called" << endl;

    assert(false);
}

const Rectangle<3> URegion::BoundingBox() const {
    if (MRA_DEBUG) cerr << "URegion::BoundingBox() called" << endl;

    assert(false);
}

// *hm* directly adapted from InRegion(), probably O(n^2) process 
// *hm* understand insideAbove calculation

void URegion::TemporalFunction(Instant& t, CRegion& res) {
    if (MRA_DEBUG) cerr << "URegion::TemporalFunction() called" << endl;

    assert(false);

#ifdef SCHMUH

    assert(t.IsDefined());
    assert(timeInterval.Contains(t));

    Instant t0 = timeInterval.start;
    Instant t1 = timeInterval.end;

    if (MRA_DEBUG) cerr << "URegion::TemporalFunction() trace#1.1" << endl;
    CRegion* cr = new CRegion(0);

    cr->StartBulkLoad();

    // all half segments in one face will have the same fcno
    // all half segments in one cycle will have the same ccno
    // two half segments will have the same edno. edno is only unique 
    // over these pairs within one cycle.
    // two half segments will have the same partnerno. edno is unique
    // over all these pairs.
    int fcno = -1;
    int ccno = -1;
    int edno = -1;
    int partnerno = 0;

    unsigned int pos = 0;

    // this loop goes through all faces...
    while (pos < pointsNum) {
	if (MRA_DEBUG) 
	    cerr << "URegion::TemporalFunction() new face" << endl;

	// isCycle will be true if current cycle is not a hole cycle
	bool isCycle = true;

	fcno++;
	ccno = -1;
	edno = -1;

	// this loop goes through all cycles of the current face
	// *hm* No exit condition for current face -> bug!
	while (pos < pointsNum) {
	    if (MRA_DEBUG) 
		cerr << "URegion::TemporalFunction() new cyle" << endl;

	    ccno++;
	    edno = -1;

	    // cycle must have at least three edges
	    assert(pos+2 < pointsNum);

	    // cyclepoints is used to check whether we have duplicate points
	    // in the cycle
	    Points cyclepoints(0);

	    // *hm* dunno
	    CRegion* rDir = new CRegion(0);

	    rDir->StartBulkLoad();

	    // get first point from DBArray
	    MPointData dmp;
	    points->Get(pointsStartPos+pos, dmp);
	    pos++;

	    if (MRA_DEBUG) 
		cerr << "URegion::TemporalFunction() first vertex pos="
		     << pos
		     << " x=" 
		     << dmp.GetStartX()
		     << " y="
		     << dmp.GetStartY()
		     << endl;

	    // create Point instance with coordinates of first point at 
	    // instant t
	    Point currvertex(true, 
			     (dmp.GetEndX()-dmp.GetStartX())*((t-t0)/(t1-t0))
			     +dmp.GetStartX(),
			     (dmp.GetEndY()-dmp.GetStartY())*((t-t0)/(t1-t0))
			     +dmp.GetStartY());

	    // store first point in cyclepoints
	    cyclepoints.StartBulkLoad();
	    cyclepoints += currvertex;
	    cyclepoints.EndBulkLoad();

	    // since we are adding half segments, we need to remember
	    // the previous point: current half segment connects previous
	    // and current point
	    Point prevvertex = currvertex;

	    // required later to connect first and last point to close
	    // the cycle
	    Point firstvertex = currvertex;

	    // *hm* Not sure. I think, though, that p1 and p2 are a complicated
	    // way to remember the previous point, and that proper usage of 
	    // prevvertex would make these obsolete
	    Point p1 = currvertex;
	    Point p2;

	    // this loop goes through all points of the current cycle
	    // *hm* No exit condition for current cycle -> bug!
	    while (pos < pointsNum) {
		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() new edge" << endl;

		// get current point from DBArray
		points->Get(pointsStartPos+pos, dmp);
		pos++;

		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() current vertex pos="
			 << pos
			 << " x=" 
			 << dmp.GetStartX()
			 << " y="
			 << dmp.GetStartY()
			 << endl;
		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() previous vertex pos="
			 << pos
			 << " x=" 
			 << prevvertex.GetX()
			 << " y="
			 << prevvertex.GetY()
			 << endl;

		// set previously created Point instance to coordinates of 
		// current point at instant t
		currvertex.Set(
		    (dmp.GetEndX()-dmp.GetStartX())*((t-t0)/(t1-t0))
		    +dmp.GetStartX(),
		    (dmp.GetEndY()-dmp.GetStartY())*((t-t0)/(t1-t0))
		    +dmp.GetStartY());

		// check if current point is a duplicate
		assert(!cyclepoints.Contains(currvertex));

		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() trace#2" << endl;

		// add current point to cyclepoints
		cyclepoints.StartBulkLoad();
		cyclepoints += currvertex;
		cyclepoints.EndBulkLoad();

		// *hm* dunno
		p2 = currvertex;

		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() trace#3" << endl;

		// create the half segment, which connects prevvertex
		// and currvertex
		CHalfSegment chs(true, true, prevvertex, currvertex);

		// remember currvertex for next half segment in next iteration
		prevvertex = currvertex;

		// assign fcno, ccno, edno and partnerno as described above
		edno++;

		chs.attr.faceno = fcno;
		chs.attr.cycleno = ccno;
		chs.attr.edgeno = edno;
		chs.attr.partnerno = partnerno;
		partnerno++;

		// *hm* Based on the assumption, that p1 contains the
		// previous point, the next statement will store in
		// chs.attr.insideAbove whether chs contains the two
		// points in swapped order.


// observation for cycles (for whole cycle, exactly the opposite holds):
// 
// cylce clockwise && p1 > p2 => inside is left or above of seg(p1, p2)
// cycle clockwise && p1 < p2 => inside is right or below of seg(p1, p2)
// 
// cycle counterclockwise && p1 < p2 => inside is left or above of seg(p1, p2)
// cycle counterclockwise && p1 > p2 => inside is right or below of seg(p1, p2)
// 
// algorithm:
// 
// calculate insideAbove for each half segment. insideAbove will be true if and
// only if the inside is left or above the half segment
// 
// 1. check for each segment whether p1 < p2 and store true in insideAbove,
//    if so.
// 
// 2. correct insideAbove:
// 
//    (a) if cycle clockwise & p1 < p2 => insideAbove = false
//    (b) if cycle counterclockwise & p1 > p2 => insideAbove = false
// 
// 3. correct insideAbove again:
// 
//    if we are dealing with a hole cycle, invert insideAbove.

		chs.attr.insideAbove = chs.GetLP() == p1;
		if (MRA_DEBUG)
		    cerr << "URegion::TemporalFunction() p1 x="
			 << p1.GetX()
			 << " y="
			 << p1.GetY()
			 << endl;
		if (MRA_DEBUG)
		    cerr << "URegion::TemporalFunction() prevvertex x="
			 << prevvertex.GetX()
			 << " y="
			 << prevvertex.GetY()
			 << endl;
		if (MRA_DEBUG)
		    cerr << "URegion::TemporalFunction() chs.GetLP() x="
			 << chs.GetLP().GetX()
			 << " y="
			 << chs.GetLP().GetY()
			 << endl;

		// hzm *dunno*
		p1 = p2;

		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() trace#4" << endl;

		// *hm* dunno
		assert(cr->insertOK(chs));

		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() trace#5.1" << endl;

		// add half segment to result
		*cr += chs;
		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() trace#5.2" << endl;

		// *hm* dunno
		*rDir += chs;

		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() trace#6" << endl;

		// add other side of half segment
		chs.SetLDP(false);

		*cr += chs;

		if (MRA_DEBUG) 
		    cerr << "URegion::TemporalFunction() trace#7" << endl;
	    }

	    if (MRA_DEBUG) 
		cerr << "URegion::TemporalFunction() trace#8.2" << endl;

	    // create half segment connect first and last point
	    CHalfSegment chs(true, true, firstvertex, currvertex);

	    if (MRA_DEBUG) 
		cerr << "URegion::TemporalFunction() first vertex pos="
		     << pos
		     << " x=" 
		     << chs.GetLP().GetX()
		     << " y="
		     << chs.GetLP().GetY()
		     << endl;
	    if (MRA_DEBUG) 
		cerr << "URegion::TemporalFunction() current vertex pos="
		     << pos
		     << " x=" 
		     << chs.GetRP().GetX()
		     << " y="
		     << chs.GetRP().GetY()
		     << endl;

	    // assign fcno, ccno, edno and partnerno as described above
	    edno++;

	    chs.attr.faceno = fcno;
	    chs.attr.cycleno = ccno;
	    chs.attr.edgeno = edno;
	    chs.attr.partnerno = partnerno;
	    partnerno++;

	    // *hm* dunno
	    chs.attr.insideAbove = chs.GetRP() == firstvertex;

	    // *hm* dunno
	    assert(cr->insertOK(chs));

	    // add half segment to result
	    *cr += chs;

	    // *hm* dunno
	    *rDir += chs;

	    // add other side of half segment
	    chs.SetLDP(false);

	    *cr += chs;

	    rDir->EndBulkLoad();
	    
	    if (MRA_DEBUG) 
		cerr << "URegion::TemporalFunction() trace#8.2" << endl;

	    // direction = true <=> cycle goes clockwise
	    // direction = false <=> cycle goes counterclockwise
	    bool direction = rDir->GetCycleDirection();

	    if (MRA_DEBUG) 
		cerr << "URegion::TemporalFunction() trace#8.3" << endl;

	    int h = cr->Size()-(rDir->Size()*2);
	    while (h < cr->Size()) {
		CHalfSegment chsInsideAbove;
		bool insideAbove;

		cr->Get(h, chsInsideAbove);

		if (direction == chsInsideAbove.attr.insideAbove) 
		    insideAbove = false;
		else
		    insideAbove = true;
		if (!isCycle) insideAbove = !insideAbove;

		chsInsideAbove.attr.insideAbove = insideAbove;
		cr->UpdateAttr(h, chsInsideAbove.attr);

		cr->Get(h+1, chsInsideAbove);
		chsInsideAbove.attr.insideAbove = insideAbove;
		cr->UpdateAttr(h+1, chsInsideAbove.attr);

		h += 2;
	    }

	    delete rDir;

	    // only the first cycle is not hole cycle. since we already
	    // processed the first cycle, all remaining cycles are hole
	    // cycles.
	    isCycle = false;

	    if (MRA_DEBUG) 
		cerr << "URegion::TemporalFunction() trace#9" << endl;
	}
    }

    if (MRA_DEBUG) 
	cerr << "URegion::TemporalFunction() trace#10" << endl;

    // *hm* add marker for end of half segment list
    CHalfSegment chs(false);
    assert(cr->insertOK(chs));

    if (MRA_DEBUG) 
	cerr << "URegion::TemporalFunction() trace#11" << endl;

    cr->EndBulkLoad();

    if (MRA_DEBUG) 
	cerr << "URegion::TemporalFunction() trace#12" << endl;

    // *hm* dunno
    cr->SetPartnerNo();

    if (MRA_DEBUG) 
	cerr << "URegion::TemporalFunction() trace#13" << endl;

    // *hm* UGLY & MEMORY LEAK
    memcpy(&res, cr, sizeof(*cr));

#endif // SCHMUH
}

bool URegion::Passes(CRegion& val) {
    if (MRA_DEBUG) cerr << "URegion::Passes() called" << endl;

    assert(false);
}

bool URegion::At(CRegion& val, TemporalUnit<CRegion>& result) {
    if (MRA_DEBUG) cerr << "URegion::At() called" << endl;

    assert(false);
}

bool URegion::AddSegment(unsigned int faceno,
			 unsigned int cycleno,
			 unsigned int segmentno,
			 double intervalLen,
			 ListExpr start, 
			 ListExpr end) {
    if (MRA_DEBUG) cerr << "URegion::AddSegment() called" << endl;

    assert(role == NORMAL || role == EMBEDDED);

    if (nl->ListLength(start) != 4
	|| !nl->IsAtom(nl->First(start))
	|| nl->AtomType(nl->First(start)) != RealType
	|| !nl->IsAtom(nl->Second(start))
	|| nl->AtomType(nl->Second(start)) != RealType
	|| !nl->IsAtom(nl->Third(start))
	|| nl->AtomType(nl->Third(start)) != RealType
	|| !nl->IsAtom(nl->Fourth(start))
	|| nl->AtomType(nl->Fourth(start)) != RealType) {
	cerr << "start point "
	     << nl->ToString(start)
	     << " not in format (<real> <real> real> <real>)" 
	     << endl;
	return false;
    }

    if (nl->ListLength(end) != 4
	|| !nl->IsAtom(nl->First(end))
	|| nl->AtomType(nl->First(end)) != RealType
	|| !nl->IsAtom(nl->Second(end))
	|| nl->AtomType(nl->Second(end)) != RealType
	|| !nl->IsAtom(nl->Third(end))
	|| nl->AtomType(nl->Third(end)) != RealType
	|| !nl->IsAtom(nl->Fourth(end))
	|| nl->AtomType(nl->Fourth(end)) != RealType) {
	cerr << "end point " 
	     << nl->ToString(end) 
	     << " not in format (<real> <real> real> <real>)" 
	     << endl;
	return false;
    }

    try {
	MSegmentData dms(faceno, 
			 cycleno, 
			 segmentno, 
			 false,
			 false,
			 false,
			 intervalLen,
			 nl->RealValue(nl->First(start)), 
			 nl->RealValue(nl->Second(start)), 
			 nl->RealValue(nl->Third(start)), 
			 nl->RealValue(nl->Fourth(start)), 
			 nl->RealValue(nl->First(end)), 
			 nl->RealValue(nl->Second(end)), 
			 nl->RealValue(nl->Third(end)), 
			 nl->RealValue(nl->Fourth(end)));

	segments->Resize(segmentsStartPos+segmentsNum+1);
	segments->Put(segmentsStartPos+segmentsNum, dms);
	segmentsNum++;
    } catch (invalid_argument& e) {
	cerr << "checking segment " 
	     << nl->ToString(start) 
	     << " - "
	     << nl->ToString(end) 
	     << " failed: " 
	     << e.what()
	     << endl;
	return false;
    }

    return true;
}

int URegion::GetSegmentsNum(void) {
    if (MRA_DEBUG) cerr << "URegion::GetSegmentsNum() called" << endl;

    assert(role == NORMAL || role == EMBEDDED);

    return segmentsNum;
}

void URegion::GetSegment(int pos, MSegmentData& dms) {
    if (MRA_DEBUG) cerr << "URegion::GetSegment() called, pos=" << pos << endl;

    assert(role == NORMAL || role == EMBEDDED);

    segments->Get(segmentsStartPos+pos, dms);
}

/*
1.1 Algebra integration

*/

// *hm* Verify list representation

static ListExpr URegionProperty() {
    if (MRA_DEBUG) cerr << "URegionProperty() called" << endl;

    ListExpr listrep = nl->TextAtom();
    nl->AppendText(listrep,
		   "(<interval> <face>*), where <interval> is "
		   "(<real> <real> <bool> <bool>) and where "
		   "<face> is (<outercycle> <holecycle>*), "
                   "where <outercycle> and <holecycle> are "
		   "(<real> <real> <real> <real>), representing "
		   "start X, start Y, end X and end Y values.");
    ListExpr example = nl->TextAtom();
    nl->AppendText(example,
		   "((0.0 10.0 TRUE FALSE) (((3 0)(10 1)(3 1))((3.1 0.1)"
		   "(3.1 0.9)(6 0.8))))");
    ListExpr remarks = nl->TextAtom();
    nl->AppendText(remarks,
		   "All <holecycle> must be completely within "
		   "corresponding <outercylce>.");

    return 
	nl->TwoElemList(
            nl->FiveElemList(
		nl->StringAtom("Signature"),
		nl->StringAtom("Example Type List"),
		nl->StringAtom("List Rep"),
		nl->StringAtom("Example List"),
		nl->StringAtom("Remarks")),
            nl->FiveElemList(
		nl->StringAtom("-> UNIT"),
		nl->StringAtom("(uregion)"),
		listrep,
		example,
		remarks));
}

static bool CheckURegion(ListExpr type, ListExpr& errorInfo) {
    if (MRA_DEBUG) cerr << "CheckURegion() called" << endl;

    return nl->IsEqual(type, "uregion");
}

static ListExpr OutURegion(ListExpr typeInfo, Word value) {
    if (MRA_DEBUG) cerr << "OutURegion() called" << endl;

    assert(false);

    return 0;

#ifdef SCHMUH
    URegion* ur = (URegion*) value.addr;

    ListExpr res = 
        nl->OneElemList(
            nl->FourElemList(
                OutDateTime(nl->TheEmptyList(), 
                            SetWord(&ur->timeInterval.start)),
                OutDateTime(nl->TheEmptyList(), 
                            SetWord(&ur->timeInterval.end)),
                nl->BoolAtom(ur->timeInterval.lc),
                nl->BoolAtom(ur->timeInterval.rc)));
    ListExpr resLastElem = res;

    int num = ur->GetPointsNum();
    if (MRA_DEBUG) cerr << "OutURegion() #points=" << num << endl;

    ListExpr face = nl->TheEmptyList();
    ListExpr faceLastElem = face;
    ListExpr cycle = nl->TheEmptyList();
    ListExpr cycleLastElem = cycle;

    MPointData nextDmp;
    if (num > 0) ur->GetPoint(0, nextDmp);

    for (int i = 0; i < num; i++) {
        if (MRA_DEBUG) cerr << "OutURegion() point #" << i << endl;

        MPointData dmp = nextDmp;
        if (i != num-1) ur->GetPoint(i+1, nextDmp);

        if (MRA_DEBUG) 
            cerr << "OutURegion() point is ("
                 << dmp.GetFaceNo()
                 << ", "
                 << dmp.GetCycleNo()
                 << ", "
                 << dmp.GetStartX()
                 << ", "
                 << dmp.GetStartY()
                 << ", "
                 << dmp.GetEndX()
                 << ", "
                 << dmp.GetEndY()
                 << ")"
                 << endl;

        ListExpr p = 
            nl->FourElemList(
                nl->RealAtom(dmp.GetStartX()),
                nl->RealAtom(dmp.GetStartY()),
                nl->RealAtom(dmp.GetEndX()),
                nl->RealAtom(dmp.GetEndY()));

        if (cycle == nl->TheEmptyList()) {
            if (MRA_DEBUG) cerr << "OutURegion() new cycle" << endl;
            cycle = nl->OneElemList(p);
            cycleLastElem = cycle;
        } else {
            if (MRA_DEBUG) cerr << "OutURegion() existing cycle" << endl;
            cycleLastElem = nl->Append(cycleLastElem, p);
        }

        if (i == num-1 || dmp.GetCycleNo() != nextDmp.GetCycleNo()) {
            if (MRA_DEBUG) cerr << "OutURegion() end of cycle" << endl;

            if (face == nl->TheEmptyList()) {
                if (MRA_DEBUG) cerr << "OutURegion() new face" << endl;
                face = nl->OneElemList(cycle);
                faceLastElem = face;
            } else {
                if (MRA_DEBUG) cerr << "OutURegion() existing face" << endl;
                faceLastElem = nl->Append(faceLastElem, cycle);
            }

            if (i == num-1 || dmp.GetFaceNo() != nextDmp.GetFaceNo()) {
                if (MRA_DEBUG) cerr << "OutURegion() end of face" << endl;
                resLastElem = nl->Append(resLastElem, face);
                
                face = nl->TheEmptyList();
                faceLastElem = face;
            }

            cycle = nl->TheEmptyList();
            cycleLastElem = cycle;
        }
    }

    return res;
#endif // SCHMUH
}

// *hm* No verification on data done yet

static Word InURegionEmbedded(const ListExpr typeInfo, 
			      const ListExpr instance,
			      const int errorPos, 
			      ListExpr& errorInfo, 
			      bool& correct,
			      DBArray<MSegmentData>* segments,
			      unsigned int segmentsStartPos) {
    if (MRA_DEBUG) cerr << "InURegionEmbedded() called" << endl;

    if (nl->ListLength(instance) == 0) {
	cerr << "uregion not in format (<interval> <face>*)" << endl;
        correct = false;
        return SetWord(Address(0));
    }

    if (MRA_DEBUG) 
        cerr << "InURegion() (<interval> <face>*) found" << endl;

    ListExpr interval = nl->First(instance);

    if (nl->ListLength(interval) != 4
        || !nl->IsAtom(nl->First(interval))
        || (nl->AtomType(nl->First(interval)) != StringType
            && nl->AtomType(nl->First(interval)) != RealType)
        || !nl->IsAtom(nl->Second(interval))
        || (nl->AtomType(nl->Second(interval)) != StringType
            && nl->AtomType(nl->Second(interval)) != RealType)
        || !nl->IsAtom(nl->Third(interval))
        || nl->AtomType(nl->Third(interval)) != BoolType
        || !nl->IsAtom(nl->Fourth(interval))
        || nl->AtomType(nl->Fourth(interval)) != BoolType) {
	cerr << "uregion interval not in format (<real> <real> <bool> <bool>)" 
	     << endl;
        correct = false;
        return SetWord(Address(0));
    }

    Instant *start = 
        (Instant *) InInstant(nl->TheEmptyList(), 
                              nl->First(interval),
                              errorPos, 
                              errorInfo, 
                              correct).addr;
    if (!correct) {
        cerr << "uregion interval invalid start time" << endl;
        correct = false;
        return SetWord(Address(0));
    }

    Instant *end = 
        (Instant *) InInstant(nl->TheEmptyList(), 
                              nl->Second(interval),
                              errorPos, 
                              errorInfo, 
                              correct ).addr;
    if (!correct) {
        cerr << "uregion interval invalid end time" << endl;
        correct = false;
        delete start;
        return SetWord(Address(0));
    }

    Interval<Instant> tinterval(*start, 
                                *end,
                                nl->BoolValue(nl->Third(interval)),
                                nl->BoolValue(nl->Fourth(interval)));

    double intervalLen = end-start;

    delete start;
    delete end;

    URegion* uregion =
	segments == 0 
	? new URegion(tinterval)
	: new URegion(tinterval, segments, segmentsStartPos);

    unsigned int faceno = 0;
    ListExpr faces = nl->Rest(instance);

    if (nl->ListLength(faces) == 0) {
	cerr << "uregion should contain at least one face" << endl;
	delete uregion;
        correct = false;
        return SetWord(Address(0));
    }

    while (!nl->IsEmpty(faces)) {
        if (MRA_DEBUG) cerr << "InURegion() face #" << faceno << endl;

        ListExpr cycles = nl->First(faces);

	if (nl->ListLength(cycles) == 0) {
	    cerr << "uregion face should contain at least one cycle" << endl;
	    delete uregion;
	    correct = false;
	    return SetWord(Address(0));
	}

        unsigned int cycleno = 0;
        unsigned int pointno = 0;

        while (!nl->IsEmpty(cycles)) {
	    if (MRA_DEBUG) cerr << "InURegion()   cycle #" << cycleno << endl;

            ListExpr cyclepoints = nl->First(cycles);

	    if (nl->ListLength(cyclepoints) < 3) {
		cerr << "uregion cycle should contain at least three points" 
		     << endl;
		delete uregion;
		correct = false;
		return SetWord(Address(0));
	    }
	    
	    ListExpr firstPoint = nl->First(cyclepoints);
	    ListExpr prevPoint = 0;

            while (!nl->IsEmpty(cyclepoints)) {
		if (MRA_DEBUG) 
		    cerr << "InURegion()     point #" << pointno << endl;

                ListExpr point = nl->First(cyclepoints);

		if (prevPoint != 0 
		    && !uregion->AddSegment(faceno, 
					    cycleno, 
					    pointno, 
					    intervalLen,
					    prevPoint, 
					    point)) {
		    cerr << "uregion's segment checks failed" << endl;
		    delete uregion;
		    correct = false;
		    return SetWord(Address(0));
		}

		prevPoint = point;
                cyclepoints = nl->Rest(cyclepoints);
                pointno++;
	    }

	    if (!uregion->AddSegment(faceno,
				     cycleno,
				     pointno,
				     intervalLen,
				     prevPoint, 
				     firstPoint)) {
		cerr << "uregion's segment checks failed" << endl;
		delete uregion;
		correct = false;
		return SetWord(Address(0));
	    }

            cycles = nl->Rest(cycles);
            cycleno++;
	}

        faces = nl->Rest(faces);
	faceno++;
    }

    assert(false);

    return SetWord(0);

#ifdef SCHMUH

    unsigned int faceno = 0;
    ListExpr faces = nl->Rest(instance);
    while (!nl->IsEmpty(faces)) {
        if (MRA_DEBUG) cerr << "InURegion() next face" << endl;

        ListExpr cycles = nl->First(faces);
        unsigned int cycleno = 0;
        unsigned int pointno = 0;
        while (!nl->IsEmpty(cycles)) {
            if (MRA_DEBUG) cerr << "InURegion() next cycle" << endl;

            ListExpr cyclepoints = nl->First(cycles);
            while (!nl->IsEmpty(cyclepoints)) {
                if (MRA_DEBUG) cerr << "InURegion() next point" << endl;

                ListExpr point = nl->First(cyclepoints);

                if (nl->ListLength(point) != 4
                    || !nl->IsAtom(nl->First(point))
                    || nl->AtomType(nl->First(point)) != RealType
                    || !nl->IsAtom(nl->Second(point))
                    || nl->AtomType(nl->Second(point)) != RealType
                    || !nl->IsAtom(nl->Third(point))
                    || nl->AtomType(nl->Third(point)) != RealType
                    || !nl->IsAtom(nl->Fourth(point))
                    || nl->AtomType(nl->Fourth(point)) != RealType) {
                    if (MRA_DEBUG) 
                        cerr << "InURegion() "
                             << "point not in format "
                             << "(<real> <real> <real> <real>)" 
                             << endl;
                    correct = false;
                    delete uregion;
                    return SetWord(Address(0));
                }

                uregion->AddPoint(pointno, 
                                  faceno,
                                  cycleno,
                                  nl->RealValue(nl->First(point)),
                                  nl->RealValue(nl->Second(point)),
                                  nl->RealValue(nl->Third(point)),
                                  nl->RealValue(nl->Fourth(point)));

                cyclepoints = nl->Rest(cyclepoints);
                pointno++;
            }

            cycles = nl->Rest(cycles);
            cycleno++;
        }

        faces = nl->Rest(faces);
    }

    int num = uregion->GetPointsNum();
    if (MRA_DEBUG) cerr << "InURegion() #points=" << num << endl;

    if (MRA_DEBUG) cerr << "InURegion() success" << endl;

    correct = true;
    return SetWord(Address(uregion));
#endif // SCHMUH
}

static Word InURegion(const ListExpr typeInfo, 
		      const ListExpr instance,
		      const int errorPos, 
		      ListExpr& errorInfo, 
		      bool& correct) {
    if (MRA_DEBUG) cerr << "InURegion() called" << endl;

    return 
	InURegionEmbedded(typeInfo, 
			  instance, 
			  errorPos, 
			  errorInfo, 
			  correct, 
			  0, 
			  0);
}

static Word CreateURegion(const ListExpr typeInfo) {
    if (MRA_DEBUG) cerr << "CreateURegion() called" << endl;

    assert(false);
}

static void DeleteURegion(Word& w) {
    if (MRA_DEBUG) cerr << "DeleteURegion() called" << endl;

    URegion *ur = (URegion *)w.addr;
    delete ur;
    w.addr = 0;
}

static void CloseURegion(Word& w) {
    if (MRA_DEBUG) cerr << "CloseURegion() called" << endl;

    delete (URegion *) w.addr;
    w.addr = 0;
}

static Word CloneURegion(const Word& w) {
    if (MRA_DEBUG) cerr << "CloneURegion() called" << endl;

    assert(false);
}

static void* CastURegion(void* addr) {
    if (MRA_DEBUG) cerr << "CastURegion() called" << endl;

    assert(false);
}

static int SizeOfURegion() {
    if (MRA_DEBUG) cerr << "SizeOfURegion() called" << endl;

    assert(false);
}

static TypeConstructor uregion(
    "uregion",
    URegionProperty,
    OutURegion, 
    InURegion,
    0, 0, // SaveToList, RestoreFromList
    CreateURegion,
    DeleteURegion,
    0, 0, // open, save
    CloseURegion,
    CloneURegion,
    CastURegion,
    SizeOfURegion,
    CheckURegion,
    0, // predef. pers. function for model
    TypeConstructor::DummyInModel,
    TypeConstructor::DummyOutModel,
    TypeConstructor::DummyValueToModel,
    TypeConstructor::DummyValueListToModel );

// ************************************************************************

/*
1 ~mregion~

1.1 Helper class ~PlaneSweepStopPoint~

*/

class PlaneSweepStopPoint {
private:
    double x;
    double y;
    double partnerX;
    double partnerY;
    bool insideLeftOrAbove;

public:
    PlaneSweepStopPoint(double xx, 
			double yy, 
			double xp, 
			double yp, 
			bool iloa) : 
	x(xx), y(yy), partnerX(xp), partnerY(yp), insideLeftOrAbove(iloa) {}

    double GetX() { return x; }
    double GetY() { return y; }
    double GetPartnerX() { return partnerX; }
    double GetPartnerY() { return partnerY; }
    bool GetInsideLeftOrAbove() { return insideLeftOrAbove; }

    bool operator<(const PlaneSweepStopPoint& p) const {
	return x > p.x || (x == p.x && y > p.y);
    }
};

/*
1.1 Class ~MRegion~

*/

class MRegion : public Mapping<URegion, CRegion> {
private: 
    DBArray<MSegmentData> msegmentdata;

    void AddRefinementUnits(vector<URegion*>& vur,
			    vector<UPoint*>& vup,
			    URegion& ur,
			    UPoint& up,
			    Instant& start,
			    Instant& end,
			    bool lc,
			    bool rc);
    void RefinementPartition(MPoint& mp,
			     vector<URegion*>& vur,
			     vector<UPoint*>& vup);

public:
    MRegion() {
        if (MRA_DEBUG) cerr << "MRegion::MRegion(int) called" << endl;
    }

    MRegion(const int n) :
        Mapping<URegion, CRegion>(n),
	msegmentdata(0) {
        if (MRA_DEBUG) cerr << "MRegion::MRegion(int) called" << endl;
    }

    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

    void Traversed(void);
    void Intersection(MPoint& mp);

    friend Word InMRegion(const ListExpr typeInfo, 
			  const ListExpr instance,
			  const int errorPos, 
			  ListExpr& errorInfo, 
			  bool& correct);
};

int MRegion::NumOfFLOBs() {
    if (MRA_DEBUG) cerr << "MRegion::NumOfFLOBs() called" << endl;

    assert(false);
    return 2;
}

FLOB* MRegion::GetFLOB(const int i) {
    if (MRA_DEBUG) cerr << "MRegion::GetFLOB() called" << endl;

    assert(false);
    assert(i == 0 || i == 1);

    return i == 0 ? Mapping<URegion, CRegion>::GetFLOB(0) : &msegmentdata;
}

static void addSegment(priority_queue<PlaneSweepStopPoint>& pq,
		       double startX, 
		       double startY, 
		       double endX, 
		       double endY, 
		       bool insideLeftOrAbove) {
    cerr << "segment: (" 
	 << startX 
	 << ", " 
	 << startY 
	 << ")-(" 
	 << endX 
	 << ", " 
	 << endY 
	 << ") insideAbove=" 
	 << insideLeftOrAbove 
	 << endl;

    PlaneSweepStopPoint sp1(startX, startY, endX, endY, insideLeftOrAbove);
    PlaneSweepStopPoint sp2(endX, endY, startX, startY, insideLeftOrAbove);

    pq.push(sp1);
    pq.push(sp2);
}


// *hm* no holes covered

void MRegion::Traversed(void) {
    if (MRA_DEBUG) cerr << "MRegion::Traversed() called" << endl;

    assert(false);

#ifdef SCHMUH
    priority_queue<PlaneSweepStopPoint> pq;

    for (int i = 0; i < GetNoComponents(); i++) {
	if (MRA_DEBUG) cerr << "MRegion::Traversed() unit #" << i << endl;

	URegion ur;
	Get(i, ur);

	// *hm* how to handle uregions, which are degenerated during initial
	// instant?

	CRegion cr;
	ur.TemporalFunction(ur.timeInterval.start, cr);

	for (int j = 0; j < cr.Size(); j++) {
	    CHalfSegment chs;
	    cr.Get(j, chs);

	    if (!chs.GetLDP()) continue;

	    if (MRA_DEBUG) 
		cerr << "MRegion::Traversed() chs #" << j << endl;

	    if (MRA_DEBUG) 
		cerr << "MRgion::Traversed()   start ("
		     << chs.GetLP().GetX()
		     << ", "
		     << chs.GetLP().GetY()
		     << ")-("
		     << chs.GetRP().GetX()
		     << ", "
		     << chs.GetRP().GetY()
		     << ") LDP="
		     << chs.GetLDP()
		     << " insideLeftOrAbove="
		     << chs.GetAttr().insideAbove
		     << " partnerno="
		     << chs.GetAttr().partnerno
		     << " edgeno="
		     << chs.GetAttr().edgeno
		     << endl;

	    addSegment(pq,
		       chs.GetLP().GetX(),
		       chs.GetLP().GetY(),
		       chs.GetRP().GetX(),
		       chs.GetRP().GetY(),
		       chs.GetAttr().insideAbove);

	    MPointData dmp1;
	    ur.GetPoint(chs.GetAttr().edgeno, dmp1);

	    MPointData dmp2;
	    if (chs.GetAttr().edgeno+1 == ur.GetPointsNum()) 
		ur.GetPoint(0, dmp2);
	    else
		ur.GetPoint(chs.GetAttr().edgeno+1, dmp2);

	    if (dmp1.GetEndX() > dmp2.GetEndX()
		|| (dmp1.GetEndX() == dmp2.GetEndX()
		    && dmp1.GetEndY() == dmp2.GetEndY())) {
		if (MRA_DEBUG) 
		    cerr << "MRegion::Traversed()   swapping" << endl;

		MPointData dummy = dmp1;
		dmp1 = dmp2;
		dmp2 = dummy;
	    }

	    if (MRA_DEBUG) 
		cerr << "MRegion::Traversed()   end ("
		     << dmp1.GetEndX()
		     << ", "
		     << dmp1.GetEndY()
		     << ")-("
		     << dmp2.GetEndX()
		     << ", "
		     << dmp2.GetEndY()
		     << ")"
		     << endl;

	    if (chs.GetLP().GetY() == chs.GetRP().GetY()) {
		if (MRA_DEBUG) 
		    cerr << "MRegion::Traversed()   horizontal" << endl;

		assert(dmp1.GetEndY() == dmp2.GetEndY());

		if (chs.GetLP().GetY() < dmp1.GetEndY()) {
		    addSegment(pq,
			       chs.GetLP().GetX(),
			       chs.GetLP().GetY(),
			       chs.GetRP().GetX(),
			       chs.GetRP().GetY(),
			       true);
		    addSegment(pq,
			       dmp1.GetEndX(),
			       dmp1.GetEndY(),
			       dmp2.GetEndX(),
			       dmp2.GetEndY(),
			       false);
		    addSegment(pq,
			       chs.GetLP().GetX(),
			       chs.GetLP().GetY(),
			       dmp1.GetEndX(),
			       dmp1.GetEndY(),
			       chs.GetLP().GetX() > dmp1.GetEndX());
		    addSegment(pq,
			       chs.GetRP().GetX(),
			       chs.GetRP().GetY(),
			       dmp2.GetEndX(),
			       dmp2.GetEndY(),
			       chs.GetRP().GetX() <= dmp2.GetEndX());
		} else if (chs.GetLP().GetY() > dmp1.GetEndY()) {
		    addSegment(pq,
			       chs.GetLP().GetX(),
			       chs.GetLP().GetY(),
			       chs.GetRP().GetX(),
			       chs.GetRP().GetY(),
			       false);
		    addSegment(pq,
			       dmp1.GetEndX(),
			       dmp1.GetEndY(),
			       dmp2.GetEndX(),
			       dmp2.GetEndY(),
			       true);
		    addSegment(pq,
			       chs.GetLP().GetX(),
			       chs.GetLP().GetY(),
			       dmp1.GetEndX(),
			       dmp1.GetEndY(),
			       chs.GetLP().GetX() < dmp1.GetEndX());
		    addSegment(pq,
			       chs.GetRP().GetX(),
			       chs.GetRP().GetY(),
			       dmp2.GetEndX(),
			       dmp2.GetEndY(),
			       chs.GetRP().GetX() >= dmp2.GetEndX());
		} else
		    // *hm* what to do here?
		    assert(false);
	    } else {
		if (MRA_DEBUG) 
		    cerr << "MRegion::Traversed()   other" << endl;
// Algorithm:
// 
// 1. Calculate X coordinate x1, where extension of chs touches X axis.
// 2. Calculate X coordinate x2, where extension of dmp1-dmp2 touches X axis.
// 3. If x1 < x2: 
//      insideLeftOrAbove(chs) = !(ascending(chs) || vertical(chs))
//      insideLeftOrAbove(dmp1-dmp2) = ascending(dmp1-dmp2) || vertical(dmp1-dmp2)
// 
//    If x1 > x2: 
//      insideLeftOrAbove(chs) = ascending(chs) || vertical(chs)
//      insideLeftOrAbove(dmp1-dmp2) = !(ascending(dmp1-dmp2) || vertical(dmp1-dmp2))
//      If x1 = x2: ignore this area?
// 
// Line (x1, y1, x2, y2),
// y1+(y2-y1)*t0 = 0 <=> t0 = y1 / (y1-y2),
// extension of line hits X axis in point (x1+(x2-x1)*t0, 0).

		double start_t0 = 
		    chs.GetLP().GetY()/(chs.GetLP().GetY()-chs.GetRP().GetY());
		double start_x0 = 
		    chs.GetLP().GetX()
		    +(chs.GetRP().GetX()-chs.GetLP().GetX())*start_t0;

		if (MRA_DEBUG) 
		    cerr << "MRegion::Traversed()   start_x0=" 
			 << start_x0 
			 << endl;

		double end_t0 = 
		    dmp1.GetEndY()/(dmp1.GetEndY()-dmp2.GetEndY());
		double end_x0 = 
		    dmp1.GetEndX()+(dmp2.GetEndX()-dmp1.GetEndX())*end_t0;

		if (MRA_DEBUG) 
		    cerr << "MRegion::Traversed()   end_x0=" 
			 << end_x0 
			 << endl;

		if (start_x0 < end_x0) {
		    addSegment(pq,
			       chs.GetLP().GetX(),
			       chs.GetLP().GetY(),
			       chs.GetRP().GetX(),
			       chs.GetRP().GetY(),
			       !(chs.GetRP().GetY() > chs.GetLP().GetY()
				 || chs.GetRP().GetX() == chs.GetLP().GetX()));
		    addSegment(pq,
			       dmp1.GetEndX(),
			       dmp1.GetEndY(),
			       dmp2.GetEndX(),
			       dmp2.GetEndY(),
			       dmp2.GetEndY() > dmp1.GetEndY()
			       || dmp1.GetEndX() == dmp2.GetEndX());
		    addSegment(pq,
			       chs.GetLP().GetX(),
			       chs.GetLP().GetY(),
			       dmp1.GetEndX(),
			       dmp1.GetEndY(),
			       chs.GetLP().GetY() < chs.GetRP().GetY());
		    addSegment(pq,
			       chs.GetRP().GetX(),
			       chs.GetRP().GetY(),
			       dmp2.GetEndX(),
			       dmp2.GetEndY(),
			       chs.GetLP().GetY() > chs.GetRP().GetY());
		} else if (start_x0 > end_x0) {
		    addSegment(pq,
			       chs.GetLP().GetX(),
			       chs.GetLP().GetY(),
			       chs.GetRP().GetX(),
			       chs.GetRP().GetY(),
			       chs.GetRP().GetY() > chs.GetLP().GetY()
				 || chs.GetRP().GetX() == chs.GetLP().GetX());
		    addSegment(pq,
			       dmp1.GetEndX(),
			       dmp1.GetEndY(),
			       dmp2.GetEndX(),
			       dmp2.GetEndY(),
			       !(dmp2.GetEndY() > dmp1.GetEndY()
				 || dmp1.GetEndX() == dmp2.GetEndX()));
		    addSegment(pq,
			       chs.GetLP().GetX(),
			       chs.GetLP().GetY(),
			       dmp1.GetEndX(),
			       dmp1.GetEndY(),
			       chs.GetLP().GetY() < chs.GetRP().GetY());
		    addSegment(pq,
			       chs.GetRP().GetX(),
			       chs.GetRP().GetY(),
			       dmp2.GetEndX(),
			       dmp2.GetEndY(),
			       chs.GetLP().GetY() > chs.GetRP().GetY());
		} else
		    // *hm* what to do here?
		    assert(false);

	    }
	}
    }


    while (!pq.empty()) {
	PlaneSweepStopPoint sp = pq.top();

	if (MRA_DEBUG) 
	    cerr << "MRegion::Traversed() stop point (" 
		 << sp.GetX()
		 << ", "
		 << sp.GetY()
		 << ")-("
		 << sp.GetPartnerX()
		 << ", "
		 << sp.GetPartnerY()
		 << ") insideLeftOrAbove="
		 << sp.GetInsideLeftOrAbove()
		 << endl;

	pq.pop();
    }

    assert(false);
#endif // SCHMUH
}

void MRegion::AddRefinementUnits(vector<URegion*>& vur,
				 vector<UPoint*>& vup,
				 URegion& ur,
				 UPoint& up,
				 Instant& start,
				 Instant& end,
				 bool lc,
				 bool rc) {
    if (MRA_DEBUG) cerr << "MRegion::AddRefinementUnits() called" << endl;

    assert(false);

#ifdef SCHMUH

    if (MRA_DEBUG) 
	cerr << "MRegion::AddRefinementUnits() start="
	     << start.ToDouble()
	     << " end="
	     << end.ToDouble()
	     << " lc="
	     << lc
	     << " rc="
	     << rc
	     << endl;

    Interval<Instant> iv(start, end, lc, rc);

    URegion* rur = new URegion(iv);

    double t0 = 
	(start.ToDouble()-ur.timeInterval.start.ToDouble())/
	(ur.timeInterval.end.ToDouble()-ur.timeInterval.start.ToDouble());
    double t1 = 
	(end.ToDouble()-ur.timeInterval.start.ToDouble())/
	(ur.timeInterval.end.ToDouble()-ur.timeInterval.start.ToDouble());

    if (MRA_DEBUG) 
	cerr << "MRegion::AddRefinementUnits() URegion t0="
	     << t0
	     << " t1="
	     << t1
	     << endl;

    for (int i = 0; i < ur.GetPointsNum(); i++) {
	if (MRA_DEBUG) 
	    cerr << "MRegion::AddRefinementUnits() point #" << i << endl;

	MPointData dmp;
	ur.GetPoint(i, dmp);

	rur->AddPoint(
	    i,
	    dmp.GetFaceNo(),
	    dmp.GetCycleNo(),
	    dmp.GetStartX()+(dmp.GetEndX()-dmp.GetStartX())*t0,
	    dmp.GetStartY()+(dmp.GetEndY()-dmp.GetStartY())*t0,
	    dmp.GetStartX()+(dmp.GetEndX()-dmp.GetStartX())*t1,
	    dmp.GetStartY()+(dmp.GetEndY()-dmp.GetStartY())*t1);
    }

    vur.push_back(rur);

    t0 = (start.ToDouble()-up.timeInterval.start.ToDouble())/
	 (up.timeInterval.end.ToDouble()-up.timeInterval.start.ToDouble());
    t1 = (end.ToDouble()-up.timeInterval.start.ToDouble())/
	 (up.timeInterval.end.ToDouble()-up.timeInterval.start.ToDouble());

    if (MRA_DEBUG) 
	cerr << "MRegion::AddRefinementUnits() UPoint t0="
	     << t0
	     << " t1="
	     << t1
	     << endl;

    vup.push_back(
	new UPoint(
	    iv,
	    up.p0.GetX()+(up.p1.GetX()-up.p0.GetX())*t0,
	    up.p0.GetY()+(up.p1.GetY()-up.p0.GetY())*t0,
	    up.p0.GetX()+(up.p1.GetX()-up.p0.GetX())*t1,
	    up.p0.GetY()+(up.p1.GetY()-up.p0.GetY())*t1));
#endif // SCHMUH
}

void MRegion::RefinementPartition(MPoint& mp,
				  vector<URegion*>& vur,
				  vector<UPoint*>& vup) {
    if (MRA_DEBUG) cerr << "MRegion::RefinementPartition() called" << endl;

    int mrUnit = 0;
    int mpUnit = 0;

    URegion ur;
    UPoint up;

    Get(0, ur);
    mp.Get(0, up);

    while (mrUnit < GetNoComponents() && mpUnit < mp.GetNoComponents()) {
	if (MRA_DEBUG) 
	    cerr << "MRegion::Intersection() mrUnit=" 
		 << mrUnit 
		 << " mpUnit=" 
		 << mpUnit 
		 << endl;

	if (ur.timeInterval.start.Compare(&up.timeInterval.start) == 0
	    && ur.timeInterval.end.Compare(&up.timeInterval.end) == 0) {
	    // case 1

	    if (MRA_DEBUG) {
		cerr << "MRegion::Intersection()   ur: |-----|" << endl;
		cerr << "MRegion::Intersection()   up: |-----|" << endl;
	    }

	    AddRefinementUnits(
		vur,
		vup,
		ur,
		up,
		ur.timeInterval.start,
		ur.timeInterval.end,
		ur.timeInterval.lc && up.timeInterval.lc,
		ur.timeInterval.rc && up.timeInterval.rc);

	    if (++mrUnit < GetNoComponents()) Get(mrUnit, ur);
	    if (++mpUnit < mp.GetNoComponents()) mp.Get(mpUnit, up);
	} else if (ur.timeInterval.Inside(up.timeInterval)) {
	    // case 2

	    if (MRA_DEBUG) {
		cerr << "MRegion::Intersection()   ur:  |---|" << endl;
		cerr << "MRegion::Intersection()   up: |-----|" << endl;
	    }

	    AddRefinementUnits(
		vur,
		vup,
		ur,
		up,
		ur.timeInterval.start,
		ur.timeInterval.end,
		ur.timeInterval.lc,
		ur.timeInterval.rc);

	    if (++mrUnit < GetNoComponents()) Get(mrUnit, ur);
	} else if (up.timeInterval.Inside(ur.timeInterval)) {
	    // case 3

	    if (MRA_DEBUG) {
		cerr << "MRegion::Intersection()   ur: |-----|" << endl;
		cerr << "MRegion::Intersection()   up:  |---|" << endl;
	    }

	    AddRefinementUnits(
		vur,
		vup,
		ur,
		up,
		up.timeInterval.start,
		up.timeInterval.end,
		up.timeInterval.lc,
		up.timeInterval.rc);

	    if (++mpUnit < mp.GetNoComponents()) mp.Get(mpUnit, up);
	} else if (ur.timeInterval.Intersects(up.timeInterval)) {
	    if (MRA_DEBUG) 
		cerr << "MRegion::Intersection()   intersect" << endl;

	    if (ur.timeInterval.start.Compare(&up.timeInterval.end) == 0 
		&& ur.timeInterval.lc 
		&& up.timeInterval.rc) {
		// case 4.1

		if (MRA_DEBUG) {
		    cerr << "MRegion::Intersection()   ur:     [---|" << endl;
		    cerr << "MRegion::Intersection()   up: |---]" << endl;
		}

		AddRefinementUnits(
		    vur,
		    vup,
		    ur,
		    up,
		    ur.timeInterval.start,
		    ur.timeInterval.start,
		    true,
		    true);

		if (++mpUnit < mp.GetNoComponents()) mp.Get(mpUnit, up);
	    } else if (ur.timeInterval.end.Compare(&up.timeInterval.start) == 0
		       && ur.timeInterval.rc
		       && up.timeInterval.lc) {
		// case 4.2

		if (MRA_DEBUG) {
		    cerr << "MRegion::Intersection()   ur: |---]" << endl;
		    cerr << "MRegion::Intersection()   up:     [---|" << endl;
		}

		AddRefinementUnits(
		    vur,
		    vup,
		    ur,
		    up,
		    ur.timeInterval.end,
		    ur.timeInterval.end,
		    true,
		    true);

		if (++mrUnit < GetNoComponents()) Get(mrUnit, ur);
	    } else if (ur.timeInterval.start.Compare(
			   &up.timeInterval.start) < 0) {
		// case 4.3

		if (MRA_DEBUG) {
		    cerr << "MRegion::Intersection()   ur: |----|" << endl;
		    cerr << "MRegion::Intersection()   up:    |----|" << endl;
		}

		AddRefinementUnits(
		    vur,
		    vup,
		    ur,
		    up,
		    up.timeInterval.start,
		    ur.timeInterval.end,
		    up.timeInterval.lc,
		    ur.timeInterval.rc);

		if (++mrUnit < GetNoComponents()) Get(mrUnit, ur);
	    } else if (ur.timeInterval.start.Compare(
			   &up.timeInterval.start) == 0) {
		// If the following assertion would not hold, we had 
		// case 2 or 3
		assert(!ur.timeInterval.lc || !up.timeInterval.rc);

		if (ur.timeInterval.end.Compare(&up.timeInterval.end) < 0) {
		    // case 4.4.1

		    if (MRA_DEBUG) {
			cerr << "MRegion::Intersection()   ur: )---|" 
			     << endl;
			cerr << "MRegion::Intersection()   up: (------|" 
			     << endl;
		    }

		    AddRefinementUnits(
			vur,
			vup,
			ur,
			up,
			ur.timeInterval.start,
			ur.timeInterval.end,
			ur.timeInterval.lc && up.timeInterval.lc,
			ur.timeInterval.rc);

		    if (++mrUnit < GetNoComponents()) Get(mrUnit, ur);
		} else {
		    // case 4.4.2

		    // If the following assertion would not hold, we had
		    // case 2 or 3 again.
		    assert(ur.timeInterval.end.Compare(
			       &up.timeInterval.end) > 0);

		    if (MRA_DEBUG) {
			cerr << "MRegion::Intersection()   ur: )------|" 
			     << endl;
			cerr << "MRegion::Intersection()   up: (---|" 
			     << endl;
		    }

		    AddRefinementUnits(
			vur,
			vup,
			ur,
			up,
			ur.timeInterval.start,
			up.timeInterval.end,
			ur.timeInterval.lc && up.timeInterval.lc,
			up.timeInterval.rc);

		    if (++mpUnit < mp.GetNoComponents()) mp.Get(mpUnit, up);
		}
	    } else {
		// case 4.5

		if (MRA_DEBUG) {
		    cerr << "MRegion::Intersection()   ur:    |----|" << endl;
		    cerr << "MRegion::Intersection()   up: |----|" << endl;
		}

		AddRefinementUnits(
		    vur,
		    vup,
		    ur,
		    up,
		    ur.timeInterval.start,
		    up.timeInterval.end,
		    ur.timeInterval.lc,
		    up.timeInterval.rc);

		if (++mpUnit < mp.GetNoComponents()) mp.Get(mpUnit, up);
	    }
	} else if (ur.timeInterval.start.Compare(
		       &up.timeInterval.start) <= 0) {
	    // case 5

	    if (MRA_DEBUG) {
		cerr << "MRegion::Intersection()   ur: |--|" << endl;
		cerr << "MRegion::Intersection()   up:      |--|" << endl;
	    }
	    
	    if (++mrUnit < GetNoComponents()) Get(mrUnit, ur);
	} else {
	    // case 6

	    if (MRA_DEBUG) {
		cerr << "MRegion::Intersection()   ur:      |--|" << endl;
		cerr << "MRegion::Intersection()   up: |--|" << endl;
	    }

	    if (++mpUnit < mp.GetNoComponents()) mp.Get(mpUnit, up);
	}
    }
}

void MRegion::Intersection(MPoint& mp) {
    if (MRA_DEBUG) cerr << "MRegion::Intersection() called" << endl;

    vector<URegion*> vur;
    vector<UPoint*> vup;

    RefinementPartition(mp, vur, vup);

    for (unsigned int i = 0; i < vur.size(); i++) {
	delete vur[i];
	delete vup[i];
    }

    assert(false);
}

/*
1.1 Algebra integration

*/

// *hm* Verify list representation

static ListExpr MRegionProperty() {
    if (MRA_DEBUG) cerr << "MRegionProperty() called" << endl;

    ListExpr listrep = nl->TextAtom();
    nl->AppendText(listrep,
		   "(<interval> <uregion>*), where <interval> is "
		   "(<real> <real> <bool> <bool>) and where "
		   "<uregion> is uregion list representation.");
    ListExpr example = nl->TextAtom();
    nl->AppendText(example,
		   "(((0.0 10.0 TRUE FALSE) (((3 0)(10 1)(3 1))((3.1 0.1)"
		   "(3.1 0.9)(6 0.8))))((10.0 20.0 TRUE FALSE) "
		   "(((0.0 0.0 -1.0 -1.0) (1.0 0.0 2.0 -1.0) "
		   "(1.0 1.0 2.0 2.0) (0.0 1.0 -1.0 2.0)))) "
		   "((20.0 30.0 TRUE FALSE) (((4.0 4.0 3.0 3.0) "
		   "(5.0 4.0 6.0 3.0) (5.0 5.0 6.0 6.0) "
		   "(4.0 5.0 3.0 6.0)))))");

    return 
	nl->TwoElemList(
            nl->FourElemList(
		nl->StringAtom("Signature"),
		nl->StringAtom("Example Type List"),
		nl->StringAtom("List Rep"),
		nl->StringAtom("Example List")),
            nl->FourElemList(
		nl->StringAtom("-> MAPPING"),
		nl->StringAtom("(mregion)"),
		listrep,
		example));
}

static bool CheckMRegion(ListExpr type, ListExpr& errorInfo) {
    if (MRA_DEBUG) cerr << "CheckMRegion() called" << endl;

    return nl->IsEqual(type, "mregion");
}

static ListExpr OutMRegion(ListExpr typeInfo, Word value) {
    if (MRA_DEBUG) cerr << "OutMRegion() called" << endl;

    MRegion* mr = (MRegion*) value.addr;

    if (mr->IsEmpty()) return (nl->TheEmptyList());

    assert(mr->IsOrdered());

    ListExpr l = nl->TheEmptyList();
    ListExpr lastElem;
    ListExpr unitList;

    for(int i = 0; i < mr->GetNoComponents(); i++) {
	URegion ur;
	mr->Get(i, ur);
	unitList = OutURegion(nl->TheEmptyList(), SetWord(&ur));
	if (l == nl->TheEmptyList()) {
	    l = nl->Cons(unitList, nl->TheEmptyList());
	    lastElem = l;
	} else
	    lastElem = nl->Append(lastElem, unitList);
    }
    return l;
}

Word InMRegion(const ListExpr typeInfo, 
		      const ListExpr instance,
		      const int errorPos, 
		      ListExpr& errorInfo, 
		      bool& correct) {
    if (MRA_DEBUG) cerr << "InMRegion() called" << endl;

    MRegion* mr = new MRegion(0);

    mr->StartBulkLoad();

    ListExpr rest = instance;
    while(!nl->IsEmpty(rest)) {
	ListExpr first = nl->First(rest);
	rest = nl->Rest(rest);

	URegion* ur = 
	    (URegion*) InURegionEmbedded(
		nl->TheEmptyList(), 
		first,
		errorPos, 
		errorInfo,
		correct,
		&mr->msegmentdata,
		mr->msegmentdata.Size()).addr;
	if(correct == false) return SetWord(Address(0));
	mr->Add(*ur);
	delete ur;
    }

    mr->EndBulkLoad(true);

    if (mr->IsValid()) {
	correct = true;
	return SetWord(mr);
    } else {
	correct = false;
	mr->Destroy();
	delete mr;
	return SetWord(0);
    }
}

static TypeConstructor mregion(
    "mregion",
    MRegionProperty,
    OutMRegion, 
    InMRegion,
    0, 0, // SaveToList, RestoreFromList
    CreateMapping<MRegion>, DeleteMapping<MRegion>,
    0, 0, // open, save
    CloseMapping<MRegion>, CloneMapping<MRegion>,
    CastMapping<MRegion>,
    SizeOfMapping<MRegion>,
    CheckMRegion,
    0, // predef. pers. function for model
    TypeConstructor::DummyInModel,
    TypeConstructor::DummyOutModel,
    TypeConstructor::DummyValueToModel,
    TypeConstructor::DummyValueListToModel);

// ************************************************************************

/*
1 Operators

1.1 Dummy model mapping

*/

static Word NoModelMapping(ArgVector arg, Supplier opTreeNode) {
    if (MRA_DEBUG) cerr << "NoModelMapping() called" << endl;

    return SetWord(Address(0));
}

static ModelMapping nomodelmap[] = { NoModelMapping };

/*
1.1 Type mapping functions

*/

// used by intersection

static ListExpr MPointMRegionToMPointTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "MPointMRegionToMPointTypeMap() called" << endl;

    cerr << nl->SymbolValue(nl->First(args)) << endl;
    cerr << nl->SymbolValue(nl->Second(args)) << endl;

    if (nl->ListLength(args) == 2 
	&& nl->IsEqual(nl->First(args), "mpoint")
	&& nl->IsEqual(nl->Second(args), "mregion"))
	return nl->SymbolAtom("mpoint");
    else
	return nl->SymbolAtom("typeerror");
}

// used by inside

static ListExpr MPointMRegionToMBoolTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "MPointMRegionToMBoolTypeMap() called" << endl;

    if (nl->ListLength(args) == 2 
	&& nl->IsEqual(nl->First(args), "mpoint")
	&& nl->IsEqual(nl->Second(args), "mregion"))
	return nl->SymbolAtom("mbool");
    else
	return nl->SymbolAtom("typeerror");
}

// used by atinstant

static ListExpr MRegionInstantToIRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "MRegionInstantToIRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 2 
	&& nl->IsEqual(nl->First(args), "mregion")
	&& nl->IsEqual(nl->Second(args), "instant"))
	return nl->SymbolAtom("iregion");
    else
	return nl->SymbolAtom("typeerror");
}

// used by traversed

static ListExpr MRegionToRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "MRegionToRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 1 
	&& nl->IsEqual(nl->First(args), "mregion"))
	return nl->SymbolAtom("region");
    else
	return nl->SymbolAtom("typeerror");
}

// used by initial, final

static ListExpr MRegionToIRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "MRegionToIRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 1 
	&& nl->IsEqual(nl->First(args), "mregion"))
	return nl->SymbolAtom("iregion");
    else
	return nl->SymbolAtom("typeerror");
}

// used by deftime

static ListExpr MRegionToPeriodsTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "MRegionToPeriodsTypeMap() called" << endl;

    if (nl->ListLength(args) == 1 
	&& nl->IsEqual(nl->First(args), "mregion"))
	return nl->SymbolAtom("periods");
    else
	return nl->SymbolAtom("typeerror");
}

// used by inst

static ListExpr IRegionToInstantTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "IRegionToInstantTypeMap() called" << endl;

    if (nl->ListLength(args) == 1 
	&& nl->IsEqual(nl->First(args), "iregion"))
	return nl->SymbolAtom("instant");
    else
	return nl->SymbolAtom("typeerror");
}

// used by val

static ListExpr IRegionToRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "IRegionToRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 1 
	&& nl->IsEqual(nl->First(args), "iregion"))
	return nl->SymbolAtom("region");
    else
	return nl->SymbolAtom("typeerror");
}

// used by present

static ListExpr PresentTypeMap(ListExpr args) {
    if (MRA_DEBUG) cerr << "PresentTypeMap() called" << endl;

    if (nl->ListLength(args) == 2 
	&& nl->IsEqual(nl->First(args), "mregion")
	&& (nl->IsEqual(nl->Second(args), "instant")
	    || nl->IsEqual(nl->Second(args), "periods"))) 
	return nl->SymbolAtom("bool");
    else
	return nl->SymbolAtom("typeerror");
}

/*
1.1 Selection functions

*/

// used by intersection, inside

static int MPointMRegionSelect(ListExpr args) {
    if (MRA_DEBUG) cerr << "MPointMRegionSelect() called" << endl;

    if (nl->ListLength(args) == 2
	&& nl->SymbolValue(nl->First(args)) == "mpoint"
	&& nl->SymbolValue(nl->Second(args)) == "mregion")
	return 0;
    else
	return -1;
}

// used by initial, final, deftime, traversed

static int MRegionSelect(ListExpr args) {
    if (MRA_DEBUG) cerr << "MRegionSelect() called" << endl;

    if (nl->ListLength(args) == 1
	&& nl->SymbolValue(nl->First(args)) == "mregion")
	return 0;
    else
	return -1;
}

// used by atinstant

static int MRegionInstantSelect(ListExpr args) {
    if (MRA_DEBUG) cerr << "MRegionInstantSelect() called" << endl;

    if (nl->ListLength(args) == 2
	&& nl->SymbolValue(nl->First(args)) == "mregion"
	&& nl->SymbolValue(nl->Second(args)) == "instant")
	return 0;
    else
	return -1;
}

// used by inst, val

static int IRegionSelect(ListExpr args) {
    if (MRA_DEBUG) cerr << "IRegionSelect() called" << endl;

    if (MRA_DEBUG) 
	cerr << "IRegionSelect() len=" << nl->ListLength(args) << endl;
    if (MRA_DEBUG) 
	cerr << "IRegionSelect() symbolvalue(first)=" 
	     << nl->SymbolValue(nl->First(args)) 
	     << endl;

    if (nl->ListLength(args) == 1
	&& nl->SymbolValue(nl->First(args)) == "iregion")
	return 0;
    else
	return -1;
}

static int PresentSelect(ListExpr args) {
    if (MRA_DEBUG) cerr << "PresentSelect() called" << endl;

    if (nl->ListLength(args) == 2
	&& nl->SymbolValue(nl->First(args)) == "mregion"
	&& nl->SymbolValue(nl->Second(args)) == "instant")
	return 0;
    else if (nl->ListLength(args) == 2
	&& nl->SymbolValue(nl->First(args)) == "mregion"
	&& nl->SymbolValue(nl->Second(args)) == "periods")
	return 1;
    else
	return -1;
}

/*
1.1 Value mapping functions

*/

static int TraversedValueMap(Word* args, 
			     Word& result, 
			     int message, 
			     Word& local, 
			     Supplier s) {
    if (MRA_DEBUG) cerr << "TraversedValueMap() called" << endl;

    ((MRegion*) args[0].addr)->Traversed();
    
    assert(false);
}


static int IntersectionValueMap(Word* args, 
				Word& result, 
				int message, 
				Word& local, 
				Supplier s) {
    if (MRA_DEBUG) cerr << "IntersectionValuMap() called" << endl;

    ((MRegion*) args[1].addr)->Intersection(* (MPoint*) args[0].addr);

    assert(false);
}

static int InsideValueMap(Word* args, 
			  Word& result, 
			  int message, 
			  Word& local, 
			  Supplier s) {
    if (MRA_DEBUG) cerr << "InsideValuMap() called" << endl;

    assert(false);
}

/*
1.1 Value mapping arrays

*/

// *hm* remove value mapping array from non-overloaded operators

static ValueMapping atinstantvaluemap[] = 
    { MappingAtInstant<MRegion, CRegion> };

static ValueMapping initialvaluemap[] = 
    { MappingInitial<MRegion, URegion, CRegion> };

static ValueMapping finalvaluemap[] = 
    { MappingFinal<MRegion, URegion, CRegion> };

static ValueMapping instvaluemap[] = 
    { IntimeInst<CRegion> };

static ValueMapping valvaluemap[] = 
    { IntimeVal<CRegion> };

static ValueMapping deftimevaluemap[] = 
    { MappingDefTime<MRegion> };

static ValueMapping presentvaluemap[] = 
    { MappingPresent_i<MRegion>,
      MappingPresent_p<MRegion> };

static ValueMapping traversedvaluemap[] = 
    { TraversedValueMap };

static ValueMapping intersectionvaluemap[] = 
    { IntersectionValueMap };

static ValueMapping insidevaluemap[] = 
    { InsideValueMap };

/*
1.1 Operator specifications

*/

static const string atinstantspec  = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mregion instant) -> iregion</text--->"
    "    <text>_ atinstant _ </text--->"
    "    <text>Get the iregion value corresponding to the instant.</text--->"
    "    <text>mregion1 atinstant instant1</text---> ) )";

static const string initialspec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> iregion</text--->"
    "    <text>initial( _ )</text--->"
    "    <text>Get the iregion value corresponding to the initial instant."
    "    </text--->"
    "    <text>initial( mregion1 )</text---> ) )";

static const string finalspec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> iregion</text--->"
    "    <text>final( _ )</text--->"
    "    <text>Get the iregion value corresponding to the final instant."
    "    </text--->"
    "    <text>final( mregion1 )</text---> ) )";

static const string instspec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>iregion -> instant</text--->"
    "    <text>inst ( _ )</text--->"
    "    <text>iregion time instant.</text--->"
    "    <text>inst ( iregion1 )</text---> ) )";

static const string valspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>iregion -> region</text--->"
    "    <text>val ( _ )</text--->"
    "    <text>Intime value.</text--->"
    "    <text>val ( iregion1 )</text---> ) )";

static const string deftimespec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> periods</text--->"
    "    <text>deftime( _ )</text--->"
    "    <text>Get the defined time of the mregion object.</text--->"
    "    <text>deftime( mregion1 )</text---> ) )";

static const string presentspec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mregion instant) -> bool, (mregion periods) -> bool</text--->"
    "    <text>_ present _ </text--->"
    "    <text>Whether the object is present at the given instant or"
    "    period.</text--->"
    "    <text>mregion1 present instant1</text---> ) )";

static const string traversedspec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> region</text--->"
    "    <text>traversed( _ )</text--->"
    "    <text>Projection of a moving region into the plane.</text--->"
    "    <text>traversed(mregion1)</text---> ) )";

static const string intersectionspec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mpoint mregion) -> mpoint</text--->"
    "    <text>_ intersection _</text--->"
    "    <text>Intersection between mpoint and mregion.</text--->"
    "    <text>mpoint1 intersection mregion1</text---> ) )";

static const string insidespec = 
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mpoint mregion) -> mbool</text--->"
    "    <text>_ intersection _</text--->"
    "    <text>Calculates if and when mpoint is inside mregion.</text--->"
    "    <text>mpoint1 inside mregion1</text---> ) )";

/*
1.1 Operator creation

*/

static Operator atinstant("atinstant",
			  atinstantspec,
			  1,
			  atinstantvaluemap,
			  nomodelmap,
			  MRegionInstantSelect,
			  MRegionInstantToIRegionTypeMap);

static Operator initial("initial",
			initialspec,
			1,
			initialvaluemap,
			nomodelmap,
			MRegionSelect,
			MRegionToIRegionTypeMap);

static Operator final("final",
		      finalspec,
		      1,
		      finalvaluemap,
		      nomodelmap,
		      MRegionSelect,
		      MRegionToIRegionTypeMap);

static Operator inst("inst",
		     instspec,
		     1,
		     instvaluemap,
		     nomodelmap,
		     IRegionSelect,
		     IRegionToInstantTypeMap);

static Operator val("val",
		    valspec,
		    1,
		    valvaluemap,
		    nomodelmap,
		    IRegionSelect,
		    IRegionToRegionTypeMap);

static Operator deftime("deftime",
			deftimespec,
			1,
			deftimevaluemap,
			nomodelmap,
			MRegionSelect,
			MRegionToPeriodsTypeMap);

static Operator present("present",
			presentspec,
			2,
			presentvaluemap,
			nomodelmap,
			PresentSelect,
			PresentTypeMap);

static Operator traversed("traversed",
			  traversedspec,
			  1,
			  traversedvaluemap,
			  nomodelmap,
			  MRegionSelect,
			  MRegionToRegionTypeMap);

static Operator intersection("intersection",
			     intersectionspec,
			     1,
			     intersectionvaluemap,
			     nomodelmap,
			     MPointMRegionSelect,
			     MPointMRegionToMPointTypeMap);

static Operator inside("inside",
		       insidespec,
		       1,
		       insidevaluemap,
		       nomodelmap,
		       MPointMRegionSelect,
		       MPointMRegionToMBoolTypeMap);

// ************************************************************************

/*
1 Algebra creation

*/

class MovingRegionAlgebra : public Algebra {
public:
    MovingRegionAlgebra() : Algebra() {
	AddTypeConstructor(&iregion);
	AddTypeConstructor(&uregion);
	AddTypeConstructor(&mregion);

	iregion.AssociateKind("TEMPORAL");
	iregion.AssociateKind("DATA");

	uregion.AssociateKind("TEMPORAL");
	uregion.AssociateKind("DATA");
	uregion.AssociateKind("SPATIAL3D");

	mregion.AssociateKind("TEMPORAL");
	mregion.AssociateKind("DATA");

	AddOperator(&atinstant);
	AddOperator(&initial);
	AddOperator(&final);
	AddOperator(&inst);
	AddOperator(&val);
	AddOperator(&deftime);
	AddOperator(&present);
	AddOperator(&traversed);
	AddOperator(&intersection);
	AddOperator(&inside);
    }
    ~MovingRegionAlgebra() {}
};

static MovingRegionAlgebra movingRegionAlgebra;

extern "C"
Algebra* InitializeMovingRegionAlgebra(NestedList* nlRef, 
				      QueryProcessor *qpRef) {
    nl = nlRef;
    qp = qpRef;
    return &movingRegionAlgebra;
}
