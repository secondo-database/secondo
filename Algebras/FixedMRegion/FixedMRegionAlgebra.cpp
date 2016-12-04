/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

[1] Implementation of the Fixed Moving Region Algebra (FixedMRAlgebra)

September, 2016. Florian Heinz <fh@sysv.de>

[TOC]

1 Overview

This Algebra defines two new datatypes:

1.1 FixedMRegion

The ~fmregion~ represents a moving and rotating region of fixed shape.
Operations are ~atinstant~, ~inside~ (of a moving point), ~interpolate~ (for
creating an ~fmregion~ from two snapshots) and ~traversedarea~, which
calculates the exact area, which is traversed during the time interval.

1.2 CRegion

A ~cregion~ is a generalized region with curved line segments. This is the
resulttype of the ~traversedarea~ operation above, since the areas borders
are not straight lines in general. The operations (point) ~inside~ and
(region) ~intersects~ are defined on it. Additionally, the operation
~cregiontoregion~ creates an approximated classic Region from the cregion.

*/

#include "fmr/fmr_FMRegion.h"
#include "fmr/fmr_MPoint.h"
#include "fmr/fmr_RList.h"
#include "fmr/fmr_MBool.h"

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "SpatialAlgebra.h"
#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"
#include "StandardTypes.h"
#include "RelationAlgebra.h"
#include "ListUtils.h"
#include "FixedMRegionAlgebra.h"


#include <math.h>

using namespace temporalalgebra;
using namespace fmregion;

namespace fmregion {

/*
2 FMRegion type definition

An ~fmregion~ consists of a ~region~ and one or more transformation units
each describing the movement over a time interval.

2.1 List definition

(<region>(<transformation><transformation>[*]))

<region>: See SpatialAlgebra
<transformation>: (<center><v0><v><a0><a><interval>)
<center>: The center point of the rotation relative to the region
<v0>: Initial displacement of the region
<v>: Linear displacement during the time interval
<a0>: Initial rotation of the region
<a>: Rotation of the region during the time interval
<interval>: The time interval consisting of a start and end instant

Example:

(
   ( \_! region !\_
      ( \_! face 1 !\_
         ( \_! main cycle !\_
           (190 -30) 
           (110 -279) 
           (342 -156) 
           (162 -216) ) ) )
        ( \_! transformation units !\_
            ( \_! transformation unit 1 !\_
                (185 -164)    \_! center !\_
                (0 0)         \_! v0     !\_
                (471 482)     \_! v      !\_
                0             \_! a0     !\_
                8             \_! a      !\_
                \_! interval !\_
                ("1970-01-01-01:00:00" "1970-01-01-01:08:20" TRUE FALSE ) ) 
            ( \_! transformation unit 2 !\_
                (112 -274)    \_! center !\_
                (655 519)     \_! v0     !\_
                (638 -517)    \_! v      !\_
                8             \_! a0     !\_
                5             \_! a      !\_
                \_! interval !\_
                ("1970-01-01-01:08:20" "1970-01-01-01:16:40" TRUE TRUE ) ) ) )

2.2 ~FMRegionProperty~

Describes the signature of the type constructor

*/

    ListExpr FMRegionProperty() {
        return (
                nl->TwoElemList(
                nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")
                ),
                nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom(FMRegion::BasicType()),
                nl->StringAtom("((<region>)(<transform1><transform2>...)"),
                nl->StringAtom("<example too long>"),
                nl->StringAtom("Type representing a moving & rotating polygon")
                )
                )
                );
    }

/*
2.3 ~OutFMRegion~

Converts the ~fmregion~ into a list representation.
(See 2.1 for the definition of the list representation)

*/

    ListExpr OutFMRegion(ListExpr typeInfo, Word value) {
        FMRegion *fmr = (FMRegion *) value.addr;

        if (fmr->fmr != NULL) {
            // RList is the interal format of the libfmr
            fmr::RList rl = fmr->fmr->toRList();

            // RList2NL converts RList to a Secondo NestedList
            return RList2NL(rl);
        } else {
            return nl->Empty();
        }
    }

/*
2.4 ~InFMRegion~

Converts a list representation of an ~fmregion~ into a FMRegion object.
(See 2.1 for the definition of the list representation)

*/
    Word InFMRegion(const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct) {
        FMRegion *fmr = new FMRegion();

        // Convert a Secondo NestedList to the libfmr RList
        fmr::RList rl = NL2RList(instance);
        // Construct a native libfmr fmregion object from the RList
        fmr->fmr = new fmr::FMRegion(rl);

        correct = true;

        return fmr;
    }

/*
2.5 ~CreateFMRegion~

Creates an empty FMRegion instance

*/
    Word CreateFMRegion(const ListExpr typeInfo) {
        FMRegion* fmr = new FMRegion();

        return SetWord(fmr);
    }

/*
2.6 ~DeleteFMRegion~

Deletes an FMRegion object

*/
    void DeleteFMRegion(const ListExpr typeInfo, Word& w) {
        FMRegion* fmr = (FMRegion*) w.addr;

        delete fmr;
    }

/*
2.7 ~CloseFMRegion~

Removes an FMRegion object from memory

*/
    void CloseFMRegion(const ListExpr typeInfo, Word& w) {
        FMRegion* fmr = (FMRegion*) w.addr;
        delete fmr;
    }

/*
2.8 ~CloneFMRegion~

Creates a deep copy of an FMRegion object

*/
    Word CloneFMRegion(const ListExpr typeInfo, const Word& w) {
        return SetWord(((FMRegion*) w.addr)->Clone());
    }

/*
2.9 ~SizeOfFMRegion~

Returns the size of an FMRegion object

*/
    int SizeOfFMRegion() {
        return sizeof (FMRegion);
    }

/*
2.10 ~CastFMRegion~

Casts a pointer to an FMRegion object

*/
    void* CastFMRegion(void* addr) {
        return (new (addr) FMRegion);
    }

/*
2.11 ~CheckFMRegion~

Returns ~true~ iff a type list represents an fmregion object

*/
    bool CheckFMRegion(ListExpr type, ListExpr& errorInfo) {
        return (nl->IsEqual(type, FMRegion::BasicType()));
    }

    TypeConstructor fmregion(
            FMRegion::BasicType(),
            FMRegionProperty,
            OutFMRegion, InFMRegion,
            0, 0,
            CreateFMRegion, DeleteFMRegion,
            0, 0, CloseFMRegion,
            CloneFMRegion, CastFMRegion,
            SizeOfFMRegion, CheckFMRegion
            );

/*
2.12 ~Clone~

Clone this FMRegion object, i.e. creates a deep copy

*/
    FMRegion* FMRegion::Clone() const {
        FMRegion *fmr = new FMRegion();
        if (this->fmr != NULL) {
            fmr->fmr = new fmr::FMRegion();
            *(fmr->fmr) = *(this->fmr);
        }

        return fmr;
    }

/*
2.13 ~FMRegion~ constructor

Creates an empty FMRegion object

*/
    FMRegion::FMRegion() {
        this->fmr = NULL;
    }

/*
2.14 ~FMRegion~ destructor

Destruct an FMRegion object and the underlying libfmr object

*/
    FMRegion::~FMRegion() {
        if (this->fmr)
            delete this->fmr;
    }

/*
3 CRegion

A cregion is a new region type, which supports not only straight,
but also curved line segments of different types. The design is
held extensible to easily support other types of curves. Otherwise,
it is similar to a ~region~, which means, it consists of several faces
each optionally containing holes.

3.1 List definition

cregion: (<cface>[*])
cface:   (<cycle><holecycle>[*])
cycle:   (<rcurve><rcurve>[*])

In contrast to a conventional ~region~ a cycle can consist of a single
curved line segment here.

rcurve: (<xoff> <yoff> <angle> <type> <param>[*])
xoff:  x offset of the startpoint
yoff:  y offset of the startpoint
angle: a rotation angle
type:  Type of curve
param: one or more optional parameters, which are type specific

Currently, three types of curves are defined. The parametric
functions expect ~t~ in the range of [0;1]. The result is
rotated by ~angle~ and then translated by (~xoff~,~yoff~).

T : A trochoidal line segment
Parameters: <a> <b> <toff> <rotation>
Parametric functions:
fx(t) = a[*]t[*]rot - b[*](sin(t[*]rot + toff) - sin(toff))
fy(t) = b[*](cos(t[*]rot + toff) - cos(toff))

R : A ravdoidal line segment
Parameters: <hp> <cd> <toff> <rotation>
Parametric functions:
fx(t) = hp[*](2[*]t[*]rot - sin(2[*](t[*]rot + toff)) + sin(2[*]toff)) +
        cd[*]              (cos     (t[*]rot + toff)  - cos(    toff))
fy(t) = hp[*](cos(2[*](t[*]rot + toff)) - cos(2[*]toff)) +
        cd[*](sin     (t[*]rot + toff)  - sin(    toff))

S : A straight line segment
Parameters: <xd> <yd>
Parametric functions:
fx(t) = xd [*] t
fy(t) = yd [*] t

T : A trochoidal line segment
Parameters: <a> <b> <toff> <rotation>
Parametric functions:
fx(t) = a [*] t [*] rot - b [*] (sin(t [*] rot + toff) - sin(toff))
fy(t) = b [*] (cos(t [*] rot + toff) - cos(toff))

R : A ravdoidal line segment
Parameters: <hp> <cd> <toff> <rotation>
Parametric functions:
fx(t) = hp[*](2[*]t[*]rot - sin(2[*](t[*]rot + toff)) + sin(2[*]toff)) +
        cd[*]              (cos     (t[*]rot + toff)  - cos(    toff))
fy(t) = hp[*](cos(2[*](t[*]rot + toff)) - cos(2[*]toff)) +
        cd[*](sin     (t[*]rot + toff)  - sin(    toff))

S : A straight line segment
Parameters: <xd> <yd>
Parametric functions:
fx(t) = xd[*]t
fy(t) = yd[*]t
T : A trochoidal line segment
Parameters: <a> <b> <toff> <rotation>
Parametric functions:
fx(t) = a[*]t[*]rot - b[*](sin(t[*]rot + toff) - sin(toff))
fy(t) = b[*](cos(t[*]rot + toff) - cos(toff))

R : A ravdoidal line segment
Parameters: <hp> <cd> <toff> <rotation>
Parametric functions:
fx(t) = hp[*](2[*]t[*]rot - sin(2[*](t[*]rot + toff)) + sin(2[*]toff)) +
        cd[*]              (cos     (t[*]rot + toff)  - cos(    toff))
fy(t) = hp[*](cos(2[*](t[*]rot + toff)) - cos(2[*]toff)) +
        cd[*](sin     (t[*]rot + toff)  - sin(    toff))

S : A straight line segment
Parameters: <xd> <yd>
Parametric functions:
fx(t) = xd[*]t
fy(t) = yd[*]t

A point on the curve at time ~t~ is then calculated as:
Point(fx(t), fy(t)).rotate(<angle>) + Point(<xoff>, <yoff>)


3.2 ~CRegionProperty~

Describes the signature of the type constructor.

*/

    ListExpr CRegionProperty() {
        return (
                nl->TwoElemList(
                nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")
                ),
                nl->FiveElemList(
                nl->StringAtom("-> Data"),
                nl->StringAtom(CRegion::BasicType()),
                nl->StringAtom("()"),
                nl->StringAtom("()"),
                nl->StringAtom("A region with curved edges")
                )
                )
                );
    }

/*
3.3 ~OutCRegion~

Converts a ~cregion~ to its nested list representation.

*/

    ListExpr OutCRegion(ListExpr typeInfo, Word value) {
        CRegion *creg = (CRegion *) value.addr;

        // Convert the libfmr region object to RList
        fmr::RList rl = creg->reg->toRList();

        // and then the RList to Secondo nested list representation
        return RList2NL(rl);
    }

/*
3.4 ~InCRegion~

Converts a ~cregion~ to its nested list representation.

*/

    Word InCRegion(const ListExpr typeInfo, const ListExpr instance,
            const int errorPos, ListExpr& errorInfo, bool& correct) {
        CRegion *creg = new CRegion();

        // Convert the cregion nested list representation to a libfmr RList
        fmr::RList rl = NL2RList(instance);
        // and create an object for it.
        creg->reg = new fmr::CRegion(rl);

        correct = true;

        return creg;
    }

/*
3.5 ~CreateCRegion~

Creates an empty ~cregion~ instance.

*/
    Word CreateCRegion(const ListExpr typeInfo) {
        CRegion* creg = new CRegion();

        return SetWord(creg);
    }

/*
3.6 ~DeleteCRegion~

Removes a ~cregion~ instance.

*/
    void DeleteCRegion(const ListExpr typeInfo, Word& w) {
        CRegion* reg = (CRegion*) w.addr;

        delete reg;
    }

/*
3.7 ~CloseCRegion~

Removes a ~cregion~ instance from memory.

*/
    void CloseCRegion(const ListExpr typeInfo, Word& w) {
        CRegion* creg = (CRegion*) w.addr;

        delete creg;
    }

/*
3.7 ~CloneCRegion~

Creates a deep copy of a ~cregion~ object.

*/
    Word CloneCRegion(const ListExpr typeInfo, const Word& w) {
        return SetWord(((CRegion*) w.addr)->Clone());
    }

/*
3.8 ~SizeOfCRegion~

Returns the size of a ~cregion~ object

*/
    int SizeOfCRegion() {
        return sizeof (CRegion);
    }

/*
3.9 ~CastCRegion~

Casts a pointer to a ~cregion~ object

*/
    void* CastCRegion(void* addr) {
        return (new (addr) CRegion);
    }

/*
3.10 ~CheckCRegion~

Checks if a type represents a ~cregion~ object

*/
    bool CheckCRegion(ListExpr type, ListExpr& errorInfo) {
        return (nl->IsEqual(type, CRegion::BasicType()));
    }

    TypeConstructor cregion(
            CRegion::BasicType(),
            CRegionProperty,
            OutCRegion, InCRegion,
            0, 0,
            CreateCRegion, DeleteCRegion,
            0, 0, CloseCRegion,
            CloneCRegion, CastCRegion,
            SizeOfCRegion, CheckCRegion
            );

/*
3.11 ~CRegion::Clone~

Creates a clone (deep copy) of itself.

*/
    CRegion* CRegion::Clone() const {
        CRegion *creg = new CRegion();
        creg->reg = new fmr::CRegion();
        *(creg->reg) = *(this->reg);

        return creg;
    }

/*
3.12 ~CRegion()~

The default constructor.

*/
    CRegion::CRegion() {
        this->reg = NULL;
    }

/*
3.13 ~CRegion destructor~

The destructor. Also deletes the underlying libfmr object.

*/
    CRegion::~CRegion() {
        if (this->reg)
            delete this->reg;
    }

/*
4 ~Operators~

Some operators for handling ~fmregion~ and ~cregion~ objects.

4.1 ~atinstant~

Calculates the projection of an ~fmregion~ to an ~iregion~ for a given instant.

Signature: fmregion x instant -> iregion
Example: query fmregion1 atinstant [const instant value "2000-01-01-01:00"]

4.1.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
    ListExpr atinstanttypemap(ListExpr args) {
        std::string err = "fmregion x instant expected";
        int len = nl->ListLength(args);
        if (len != 2) {
            return listutils::typeError(err + " (wrong number of arguments)");
        }
        if (!FMRegion::checkType(nl->First(args))) {
            return listutils::typeError(err + " (first arg wrong)");
        }
        if (!Instant::checkType(nl->Second(args))) {
            return listutils::typeError(err + " (second arg wrong)");
        }
        return nl->SymbolAtom(temporalalgebra::IRegion::BasicType());
    }

/*
4.1.2 ~Value mapping~

Maps a result value to the given arguments.

*/
    int atinstantvalmap(Word *args, Word& result,
            int message, Word& local, Supplier s) {
        result = qp->ResultStorage(s);

        FMRegion *fmr = static_cast<FMRegion*> (args[0].addr);
        Instant *it = static_cast<Instant*> (args[1].addr);

        // Calculate the projected region in libfmr
        fmr::Region fmrreg = fmr->fmr->atinstant(it->GetAllMilliSeconds());
        // and convert the result to a Secondo nested list
        ListExpr regle = RList2NL(fmrreg.toRList());

        // Create an IRegion from the result and return it
        bool correct;
        ListExpr errorInfo;
        Word regp = InRegion(nl->Empty(), regle, 0, errorInfo, correct);
        Region *reg = static_cast<Region*> (regp.addr);
        result.setAddr(new temporalalgebra::IRegion(*it, *reg));

        return 0;
    }

    static const std::string atinstantspec =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
            "  (<text>fmregion x instant -> iregion</text--->"
            "<text>_ atinstant _</text--->"
            "<text>Shows the fmregion at some instant</text--->"
            "<text>fmregion atinstant instant1</text---> ) )";

    Operator atinstant("atinstant",
            atinstantspec,
            atinstantvalmap,
            Operator::SimpleSelect,
            atinstanttypemap
            );

/*
4.2 ~inside~

Calculates the times, when a moving point is inside an ~fmregion~. The result
is an ~mbool~ object, which is defined at all times, when both source objects
are defined. The ~mbool~ is *true*, when the moving point is ~inside~ the
fmregion and *false* at all other times.

Signature: mpoint x fmregion -> mbool
Example: query mpoint1 inside fmregion1

4.2.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
    ListExpr insidetypemap(ListExpr args) {
        std::string err = "mpoint x fmregion expected";
        int len = nl->ListLength(args);
        if (len != 2) {
            return listutils::typeError(err + " (wrong number of arguments)");
        }
        if (!temporalalgebra::MPoint::checkType(nl->First(args))) {
            return listutils::typeError(err + " (wrong first arg)");
        }
        if (!FMRegion::checkType(nl->Second(args))) {
            return listutils::typeError(err + " (wrong second arg)");
        }
        return nl->SymbolAtom(temporalalgebra::MBool::BasicType());
    }

/*
4.2.2 ~Value mapping~

Maps a result value to the given arguments.

*/
    int insidevalmap(Word *args, Word& result,
            int message, Word& local, Supplier s) {
        result = qp->ResultStorage(s);

        FMRegion *fmr = static_cast<FMRegion*> (args[1].addr);

        // Convert the Secondo nested list representation of the moving point
        // to a libfmr MPoint object
        fmr::RList mprl = NL2RList(OutMapping<MPoint, UPoint,
                OutUPoint>(nl->Empty(), args[0]));
        fmr::MPoint mp(mprl);

        // Calculate the times, when the moving point is inside the fmregion
        // and store the result in an mbool object
        fmr::MBool mb = mp.inside(*fmr->fmr);

        // Convert the libfmr mbool to a Secondo mbool and return it
        ListExpr le = RList2NL(mb.toRList());
        bool correct;
        ListExpr errorInfo;
        result = InMapping<MBool, UBool, InConstTemporalUnit<CcBool, InCcBool> >
                                       (nl->Empty(), le, 0, errorInfo, correct);

        return 0;
    }

    static const std::string insidespec =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
            "  (<text>mpoint x fmregion -> mbool</text--->"
            "<text>_ inside _</text--->"
            "<text>calculate times when mpoint is inside the fmregion</text--->"
            "<text>mpoint1 inside fmregion1</text---> ) )";

    Operator inside("inside",
            insidespec,
            insidevalmap,
            Operator::SimpleSelect,
            insidetypemap
            );

/*
4.3 ~fmrinterpolate~

Try to create a ~fmregion~ from two snapshots. The shape of the region must
be identical, i.e. the second region must be a translated and rotated version
of the first one.

Signature: region x instant x region x instant -> fmregion
Example: query fmrinterpolate(region1, instant1, region2, instant2)

4.3.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
    ListExpr fmrinterpolatetypemap(ListExpr args) {
        std::string err = "region x instant x region x instant expected";
        int len = nl->ListLength(args);
        if (len != 4) {
            return listutils::typeError(err + " (wrong number of arguments)");
        }
        if (!Region::checkType(nl->First(args))) {
            return listutils::typeError(err + " (wrong first arg)");
        }
        if (!Instant::checkType(nl->Second(args))) {
            return listutils::typeError(err + " (wrong second arg)");
        }
        if (!Region::checkType(nl->Third(args))) {
            return listutils::typeError(err + " (wrong third arg)");
        }
        if (!Instant::checkType(nl->Fourth(args))) {
            return listutils::typeError(err + " (wrong fourth arg)");
        }
        return nl->SymbolAtom(FMRegion::BasicType());
    }

/*
4.3.2 ~Value mapping~

Maps a result value to the given arguments.

*/
    int fmrinterpolatevalmap(Word *args, Word& result,
            int message, Word& local, Supplier s) {
        result = qp->ResultStorage(s);

        // Convert the Secondo source regions to libfmr region objects
        fmr::RList sregrl = NL2RList(OutRegion(nl->Empty(), args[0]));
        fmr::Region sreg(sregrl);
        fmr::RList dregrl = NL2RList(OutRegion(nl->Empty(), args[2]));
        fmr::Region dreg(dregrl);

        Instant* ti1 = static_cast<Instant*> (args[1].addr);
        Instant* ti2 = static_cast<Instant*> (args[3].addr);

        // Create the interpolation interval
        fmr::Interval iv(ti1->GetAllMilliSeconds(), ti2->GetAllMilliSeconds(),
                true, true);
        // Calculate the interpolation
        fmr::FMRegion fm = sreg.interpolate(dreg, iv);

        // Convert the libfmr fmregion to a Secondo fmregion object and return
        bool correct;
        ListExpr errorInfo;
        ListExpr fmle = RList2NL(fm.toRList());
        result = InFMRegion(nl->Empty(), fmle, 0, errorInfo, correct);

        return 0;
    }

    static const std::string fmrinterpolatespec =
     "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
     "  (<text>region x instant x region x instant -> fmregion</text--->"
     "<text>fmrinterpolate( _ , _ , _ , _ )</text--->"
     "<text>interpolates two regions to an fmregion</text--->"
     "<text>fmrinterpolate(region1, instant1, region2, instant2)</text---> ) )";

    Operator fmrinterpolate("fmrinterpolate",
            fmrinterpolatespec,
            fmrinterpolatevalmap,
            Operator::SimpleSelect,
            fmrinterpolatetypemap
            );

/*
4.4 ~traversedarea~

An ~fmregion~ traverses a certain area during its movement. This operator
calculates this area and returns a corresponding ~cregion~ object.

Signature: fmregion -> cregion
Example: query traversedarea(fmregion1)

4.4.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
    ListExpr traversedareatypemap(ListExpr args) {
        std::string err = "fmregion expected";
        int len = nl->ListLength(args);
        if (len != 1) {
            return listutils::typeError(err + " (wrong number of arguments)");
        }
        if (!FMRegion::checkType(nl->First(args))) {
            return listutils::typeError(err + " (wrong first arg)");
        }
        return nl->SymbolAtom(CRegion::BasicType());
    }

/*
4.4.2 ~Value mapping~

Maps a result value to the given arguments.

*/
    int traversedareavalmap(Word *args, Word& result,
                                         int message, Word& local, Supplier s) {
        result = qp->ResultStorage(s);

        FMRegion *fmr = static_cast<FMRegion*> (args[0].addr);

        // Calculate the traversed area and store it in a libfmr region2 object
        fmr::CRegion creg = fmr->fmr->traversedArea();
        bool correct;
        ListExpr errorInfo;
        // Convert the libfmr region2 to a Secondo cregion and return it
        result = InCRegion(nl->Empty(), RList2NL(creg.toRList()), 0,
                errorInfo, correct);

        return 0;
    }

    static const std::string traversedareaspec =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
            "  (<text>fmregion -> cregion</text--->"
            "<text>traversedarea( _ )</text--->"
            "<text>calculates the traversed area of an fmregion</text--->"
            "<text>traversedarea(fmregion1)</text---> ) )";

    Operator traversedarea("traversedarea",
            traversedareaspec,
            traversedareavalmap,
            Operator::SimpleSelect,
            traversedareatypemap
            );

/*
4.5 ~inside~

Test if a point is inside a ~cregion~

Signature: point x cregion -> bool
Example: query point([100, 100]) inside cregion1

4.5.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
    ListExpr insidecregiontypemap(ListExpr args) {
        std::string err = "point x cregion expected";
        int len = nl->ListLength(args);
        if (len != 2) {
            return listutils::typeError(err + " (wrong number of arguments)");
        }
        if (!Point::checkType(nl->First(args))) {
            return listutils::typeError(err + " (wrong first arg)");
        }
        if (!CRegion::checkType(nl->Second(args))) {
            return listutils::typeError(err + " (wrong second arg)");
        }
        return nl->SymbolAtom(CcBool::BasicType());
    }

/*
4.5.2 ~Value mapping~

Maps a result value to the given arguments.

*/
    int insidecregionvalmap(Word *args, Word& result, int message,
            Word& local, Supplier s) {
        result = qp->ResultStorage(s);

        Point *p = static_cast<Point*> (args[0].addr);
        CRegion *cr = static_cast<CRegion*> (args[1].addr);

        result = new CcBool(true,
                cr->reg->inside(fmr::Point(p->GetX(), p->GetY())));

        return 0;
    }


    static const std::string insidecregionspec =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
            "  (<text>point x cregion -> bool</text--->"
            "<text>_ inside _</text--->"
            "<text>Test if point is inside a cregion</text--->"
            "<text>point1 inside cregion1</text---> ) )";

    Operator insidecregion("inside",
            insidecregionspec,
            insidecregionvalmap,
            Operator::SimpleSelect,
            insidecregiontypemap
            );

/*
4.6 ~intersects~

Test if a ~cregion~ intersects or overlaps with a ~region~

Signature: cregion x region -> bool
Example: query cregion1 intersects region1

4.6.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
    ListExpr intersectstypemap(ListExpr args) {
        std::string err = "cregion x region expected";
        int len = nl->ListLength(args);
        if (len != 2) {
            return listutils::typeError(err + " (wrong number of arguments)");
        }
        if (!CRegion::checkType(nl->First(args))) {
            return listutils::typeError(err + " (wrong first arg)");
        }
        if (!Region::checkType(nl->Second(args))) {
            return listutils::typeError(err + " (wrong second arg)");
        }
        return nl->SymbolAtom(CcBool::BasicType());
    }

/*
4.6.2 ~Value mapping~

Maps a result value to the given arguments.

*/
    int intersectsvalmap(Word *args, Word& result, int message,
            Word& local, Supplier s) {
        result = qp->ResultStorage(s);

        CRegion *creg = static_cast<CRegion*> (args[0].addr);
        fmr::RList regrl = NL2RList(OutRegion(nl->Empty(), args[1]));
        fmr::Region reg(regrl);

        result = new CcBool(true, creg->reg->intersects(reg));

        return 0;
    }

    static const std::string intersectsspec =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
            "  (<text>cregion x region -> bool</text--->"
            "<text>_ intersects _</text--->"
            "<text>Test if a region intersects a cregion</text--->"
            "<text>cregion1 intersects region1</text---> ) )";

    Operator intersects("intersects",
            intersectsspec,
            intersectsvalmap,
            Operator::SimpleSelect,
            intersectstypemap
            );

    
/*
4.7 ~cregiontoregion~

Converts a ~cregion~ into a ~region~ by approximating the curved border with
a specified amount of straight line segments.

Signature: cregion x int -> region
Example: query cregiontoregion(cregion1, 100);

4.7.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
    ListExpr cregiontoregiontypemap(ListExpr args) {
        std::string err = "cregion x int expected";
        int len = nl->ListLength(args);
        if (len != 2) {
            return listutils::typeError(err + " (wrong number of arguments)");
        }
        if (!CRegion::checkType(nl->First(args))) {
            return listutils::typeError(err + " (wrong first arg)");
        }
        if (!CcInt::checkType(nl->Second(args))) {
            return listutils::typeError(err + " (wrong second arg)");
        }
        return nl->SymbolAtom(Region::BasicType());
    }

/*
4.7.2 ~Value mapping~

Maps a result value to the given arguments.

*/
    int cregiontoregionvalmap(Word *args, Word& result, int message,
            Word& local, Supplier s) {
        result = qp->ResultStorage(s);

        // First, convert the arguments
        CRegion *creg = static_cast<CRegion*> (args[0].addr);
        CcInt *nrsegs = static_cast<CcInt*> (args[1].addr);
        // And create the approximated region...
        fmr::Region fmrreg = creg->reg->toRegion(nrsegs->GetIntval());
        
        // Now, union all faces of this region, since the faces may have
        // overlapped in the cregion (and therefore also in the approximated
        // region). This is necessary, since there is no union operation
        // implemented on a cregion yet.
        Region *res = NULL;
        for (unsigned int i = 0; i < fmrreg.faces.size(); i++) {
            fmr::Region tmp;
            tmp.faces.push_back(fmrreg.faces[i]);
            
            ListExpr regle = RList2NL(tmp.toRList());
            bool correct;
            ListExpr errorInfo;
            Word regp = InRegion(nl->Empty(), regle, 0, errorInfo, correct);
            if (!correct) {
                continue;
            }
            Region *reg = static_cast<Region*> (regp.addr);
            if (!res) {
                res = reg;
            } else {
		Region res2(0);
                res->Union(*reg, res2, NULL);
                delete reg;
		*res = res2;
            }
        }
        
        result.setAddr(res);

        return 0;
    }


    static const std::string cregiontoregionspec =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
            "  (<text>cregion x int -> region</text--->"
            "<text>cregiontoregion(_, _)</text--->"
            "<text>Convert a cregion to an approximated region</text--->"
            "<text>cregiontoregion(cregion1, 100)</text---> ) )";

    Operator cregiontoregion("cregiontoregion",
             cregiontoregionspec,
             cregiontoregionvalmap,
             Operator::SimpleSelect,
             cregiontoregiontypemap
            );


/*
5 ~FixedMRegionAlgebra~

Instantiation of the FixedMRegionAlgebra.
Adds the types ~fmregion~ and ~cregion~
Adds the operators ~atinstant~, ~inside~, ~fmrinterpolate~, ~traversedarea~

*/
    FixedMRegionAlgebra::FixedMRegionAlgebra() : Algebra() {
        AddTypeConstructor(&fmregion);
        AddTypeConstructor(&cregion);
        AddOperator(&atinstant);
        AddOperator(&inside);
        AddOperator(&insidecregion);
        AddOperator(&fmrinterpolate);
        AddOperator(&traversedarea);
        AddOperator(&intersects);
        AddOperator(&cregiontoregion);
    }

    extern "C"
    Algebra *
    InitializeFixedMRegionAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
        nl = nlRef;
        qp = qpRef;
        return new FixedMRegionAlgebra();
    }
}
/*
6 Conversion functions

These functions convert between Secondo NestedLists and libfmr RLists.
Supported types are: List, String, Double, Bool and Symbol, which are all
existing RList types. NestedList Integers are converted to double.
Unknown types are dropped.

6.1 ~RList2NL~

Convert a libfmr RList to a Secondo NestedList.

*/
ListExpr RList2NL(fmr::RList r) {
    ListExpr ret = nl->Empty();
    ListExpr cur = ret;

    switch (r.getType()) {
        case fmr::NL_LIST:
            for (int i = 0; i < r.size(); i++) {
                if (i == 0)
                    ret = cur = nl->OneElemList(RList2NL(r[i]));
                else
                    cur = nl->Append(cur, RList2NL(r[i]));
            }
            break;
        case fmr::NL_STRING:
            ret = nl->StringAtom(r.getString());
            break;
        case fmr::NL_DOUBLE:
            ret = nl->RealAtom(r.getNr());
            break;
        case fmr::NL_BOOL:
            ret = nl->BoolAtom(r.getBool());
            break;
        case fmr::NL_SYM:
            ret = nl->SymbolAtom(r.getSym());
            break;
    }

    return ret;
}
/*
6.2 ~NL2RList~

Convert a Secondo NestedList to a libfmr RList.

*/
fmr::RList NL2RList(ListExpr l) {
    fmr::RList ret;

    while (l != nl->Empty()) {
        ListExpr i = nl->First(l);
        if (nl->IsNodeType(NoAtom, i)) {
            ret.append(NL2RList(i));
        } else if (nl->IsNodeType(IntType, i)) {
            // Convert integer to double, since there is no separate
            // RList type for it
            ret.append((double) nl->IntValue(i));
        } else if (nl->IsNodeType(RealType, i)) {
            ret.append(nl->RealValue(i));
        } else if (nl->IsNodeType(BoolType, i)) {
            ret.append(nl->BoolValue(i));
        } else if (nl->IsNodeType(StringType, i)) {
            ret.append(nl->StringValue(i));
        } else if (nl->IsNodeType(SymbolType, i)) {
            ret.appendSym(nl->SymbolValue(i));
        }
        l = nl->Rest(l);
    }

    return ret;
}
