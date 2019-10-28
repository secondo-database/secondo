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

June, 2018. Florian Heinz <fh@sysv.de>

[TOC]

1 Overview

This Algebra defines a new datatype:

1.1 PMRegion

The ~pmregion~ represents a polyhedral moving region.
Operations are ~atinstant~, ~inside~ (of a moving point), ~traversedarea~,
which calculates the exact area, which is traversed during the time interval,
~area~ and ~perimeter~, which calculates an mreal value for the requested
parameter, the set operations ~union~, ~intersection~ and ~minus~ as well as
~intersects~. Furthermore, ~pmreg2mreg~ and ~mreg2pmreg~ can be used to
convert the pmregion to a mregion and vice versa.

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "Algebras/MovingRegion/MovingRegionAlgebra.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "ListUtils.h"

#include "PMRegionAlgebra.h"


#include <math.h>

using namespace temporalalgebra;
using namespace pmregion;
using namespace pmr;


namespace temporalalgebra {
    Word InUReal( const ListExpr typeInfo, const ListExpr instance,
		  const int errorPos, ListExpr& errorInfo, bool& correct );
    Word InMRegion(const ListExpr typeInfo,
		    const ListExpr instance,
		    const int errorPos,
		    ListExpr& errorInfo,
		    bool& correct);
    ListExpr OutMRegion(ListExpr typeInfo, Word instance);
}

namespace pmregion {

/*
   2 PMRegion type definition

   An ~pmregion~ is defined by one or more polyhedra.

   2.1 List definition

   ((<Point3D>*)(<Triangle>*))

   <Point3D>: (x y z) with x, y, z : coordinate (real value)
   <Triangle>: (idx1 idx2 idx3) with idxn being the index of
   the point in the first list.

Example:
( 
(  __! Vertices !__
( 1392 438 0 ) 
( 1683 758 1000 ) 
( 1098 917 1000 ) 
( 1392 438 1000 ) ) 
(  __! Surfaces !__
( 1 2 3 ) 
( 3 2 0 ) 
( 0 2 1 ) 
( 3 0 1 ) ) ) 


2.2 ~PMRegionProperty~

Describes the signature of the type constructor

*/

ListExpr PMRegionProperty() {
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
                  nl->StringAtom(PMRegion::BasicType()),
                  nl->StringAtom("((<points>)(<faces>))"),
                  nl->StringAtom("<example too long>"),
                  nl->StringAtom("Type representing a polyhedral moving region")
                    )
                )
           );
}

/*
   2.3 ~OutPMRegion~

   Converts the ~pmregion~ into a list representation.
   (See 2.1 for the definition of the list representation)

*/

ListExpr OutPMRegion(ListExpr typeInfo, Word value) {
    PMRegion *pmr = (PMRegion *) value.addr;

    // RList is the interal format of the libpmregion
    RList rl = pmr->pmr->toRList();

    // RList2NL converts RList to a Secondo NestedList
    return RList2NL(rl.items[4]);
}

/*
   2.4 ~InPMRegion~

   Converts a list representation of a ~pmregion~ into a PMRegion object.
   (See 2.1 for the definition of the list representation)

*/
Word InPMRegion(const ListExpr typeInfo, const ListExpr instance,
        const int errorPos, ListExpr& errorInfo, bool& correct) {
    PMRegion *pmr = new PMRegion();

    // Convert a Secondo NestedList to the libpmregion RList
    RList rl = NL2RList(instance);
    // Construct a native libpmregion object from the RList
    pmr->pmr = new pmr::PMRegion();
    *pmr->pmr = pmr::PMRegion::fromRList(rl.obj("pmregion", "pmregion"));

    correct = true;

    return pmr;
}

/*
   2.5 ~CreatePMRegion~

   Creates an empty PMRegion instance

*/
Word CreatePMRegion(const ListExpr typeInfo) {
    PMRegion* pmr = new PMRegion();

    return SetWord(pmr);
}

/*
   2.6 ~DeletePMRegion~

   Deletes a PMRegion object

*/
void DeletePMRegion(const ListExpr typeInfo, Word& w) {
    PMRegion* pmr = (PMRegion*) w.addr;

    delete pmr;
}

/*
   2.7 ~ClosePMRegion~

   Removes a PMRegion object from memory

*/
void ClosePMRegion(const ListExpr typeInfo, Word& w) {
    PMRegion* pmr = (PMRegion*) w.addr;
    delete pmr;
}

/*
   2.8 ~ClonePMRegion~

   Creates a deep copy of a PMRegion object

*/
Word ClonePMRegion(const ListExpr typeInfo, const Word& w) {
    return SetWord(((PMRegion*) w.addr)->Clone());
}

/*
   2.9 ~SizeOfPMRegion~

   Returns the size of a PMRegion object

*/
int SizeOfPMRegion() {
    return sizeof (PMRegion);
}

/*
   2.10 ~CastPMRegion~

   Casts a pointer to an PMRegion object

*/
void* CastPMRegion(void* addr) {
    return (new (addr) PMRegion);
}

/*
   2.11 ~CheckPMRegion~

   Returns ~true~ iff a type list represents an pmregion object

*/
bool CheckPMRegion(ListExpr type, ListExpr& errorInfo) {
    return (nl->IsEqual(type, PMRegion::BasicType()));
}

TypeConstructor pmregion(
        PMRegion::BasicType(),
        PMRegionProperty,
        OutPMRegion, InPMRegion,
        0, 0,
        CreatePMRegion, DeletePMRegion,
        0, 0, ClosePMRegion,
        ClonePMRegion, CastPMRegion,
        SizeOfPMRegion, CheckPMRegion
        );

/*
   2.12 ~Clone~

   Clone this PMRegion object, i.e. creates a deep copy

*/
PMRegion* PMRegion::Clone() const {
    PMRegion *pmr = new PMRegion();
    if (this->pmr != NULL) {
        pmr->pmr = new pmr::PMRegion();
        *(pmr->pmr) = *(this->pmr);
    }

    return pmr;
}

/*
   2.13 ~PMRegion~ constructor

   Creates an empty PMRegion object

*/
PMRegion::PMRegion() {
    this->pmr = NULL;
}

/*
   2.14 ~PMRegion~ destructor

   Destruct a PMRegion object and the underlying libpmregion object

*/
PMRegion::~PMRegion() {
    if (this->pmr)
        delete this->pmr;
}

/*
   3 ~Operators~

   Some operators for handling ~pmregion~s

   3.1 ~atinstant~

   Calculates the projection of a ~pmregion~ to an ~iregion~ for a given instant.

Signature: pmregion x instant -> iregion
Example: query pmregion1 atinstant [const instant value "2000-01-01-01:00"]

3.1.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr atinstanttypemap(ListExpr args) {
    std::string err = "pmregion x instant expected";
    int len = nl->ListLength(args);
    if (len != 2) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    if (!Instant::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (second arg wrong)");
    }
    return nl->SymbolAtom(temporalalgebra::IRegion::BasicType());
}

/*
   3.1.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int atinstantvalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr = static_cast<PMRegion*> (args[0].addr);
    Instant *it = static_cast<Instant*> (args[1].addr);

    // Convert secondo instant to ms since unix epoch
    double instant = (it->ToDouble()+10959)*86400000;

    // Calculate the projected region in libpmregion
    RList region = pmr->pmr->atinstant(instant);
    // and convert the result to a Secondo nested list
    ListExpr regle = RList2NL(region.items[4]);

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
"  (<text>pmregion x instant -> iregion</text--->"
"<text>_ atinstant _</text--->"
"<text>Shows the pmregion at some instant</text--->"
"<text>pmregion atinstant instant1</text---> ) )";

Operator atinstant("atinstant",
        atinstantspec,
        atinstantvalmap,
        Operator::SimpleSelect,
        atinstanttypemap
        );


/*
   3.2 ~perimeter~

   Calculates the perimeter of a ~pmregion~ to an ~mreal~ value

Signature: pmregion -> mreal
Example: query perimeter(pmregion1)

3.2.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr perimetertypemap(ListExpr args) {
    std::string err = "pmregion expected";
    int len = nl->ListLength(args);
    if (len != 1) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    return nl->SymbolAtom(temporalalgebra::MReal::BasicType());
}

/*
   3.2.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int perimetervalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr = static_cast<PMRegion*> (args[0].addr);

    // Calculate the perimeter in libpmregion
    pmr::MReal mreal = pmr->pmr->perimeter();
    // and convert the result to a Secondo nested list
    ListExpr le = RList2NL(mreal.rl.items[4]);

    // Create an MReal from the result and return it
    bool correct;
    ListExpr errorInfo;
    result = InMapping<temporalalgebra::MReal, UReal, InUReal>
        (nl->Empty(), le, 0, errorInfo, correct);

    return 0;
}

static const std::string perimeterspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion -> mreal</text--->"
"<text>perimeter ( _ )</text--->"
"<text>Calculates the perimeter of a pmregion</text--->"
"<text>perimeter pmregion</text---> ) )";

Operator perimeter("perimeter",
        perimeterspec,
        perimetervalmap,
        Operator::SimpleSelect,
        perimetertypemap
        );



/*
   3.3 ~area~

   Calculates the area of a ~pmregion~ to an ~iregion~ for a given instant.

Signature: pmregion -> mreal
Example: query area(pmregion)

3.3.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr areatypemap(ListExpr args) {
    std::string err = "pmregion expected";
    int len = nl->ListLength(args);
    if (len != 1) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    return nl->SymbolAtom(temporalalgebra::MReal::BasicType());
}

/*
   3.3.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int areavalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr = static_cast<PMRegion*> (args[0].addr);

    // Calculate the area in libpmregion
    pmr::MReal mreal = pmr->pmr->area();
    // and convert the result to a Secondo nested list
    ListExpr le = RList2NL(mreal.rl.items[4]);

    // Create an MReal from the result and return it
    bool correct;
    ListExpr errorInfo;
    result = InMapping<temporalalgebra::MReal, UReal, InUReal>
        (nl->Empty(), le, 0, errorInfo, correct);

    return 0;
}

static const std::string areaspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion -> mreal</text--->"
"<text>perimeter ( _ )</text--->"
"<text>Calculates the area of a pmregion</text--->"
"<text>perimeter pmregion</text---> ) )";

Operator area("area",
        areaspec,
        areavalmap,
        Operator::SimpleSelect,
        areatypemap
        );


/*
   3.4 ~traversedarea~

   Calculates the traversed area of a ~pmregion~

Signature: pmregion -> region
Example: query traversedarea(pmregion1)

3.4.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr traversedareatypemap(ListExpr args) {
    std::string err = "pmregion expected";
    int len = nl->ListLength(args);
    if (len != 1) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    return nl->SymbolAtom(Region::BasicType());
}

/*
   3.4.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int traversedareavalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr = static_cast<PMRegion*> (args[0].addr);

    // Calculate the traversed area in libpmregion
    RList region = pmr->pmr->traversedarea();
    // and convert the result to a Secondo nested list
    ListExpr le = RList2NL(region.items[4]);

    // Create an region from the result and return it
    bool correct;
    ListExpr errorInfo;
    result = InRegion(nl->Empty(), le, 0, errorInfo, correct);

    return 0;
}

static const std::string traversedareaspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion -> region</text--->"
"<text>traversedarea ( _ )</text--->"
"<text>Calculates the traversed area of a pmregion</text--->"
"<text>traversedarea pmregion</text---> ) )";

Operator traversedarea("traversedarea",
        traversedareaspec,
        traversedareavalmap,
        Operator::SimpleSelect,
        traversedareatypemap
        );

#if CGAL_VERSION_NR >= 1041400000
/*
   3.4 ~coverduration~

   Calculates the coverduration of a ~pmregion~

Signature: pmregion -> pmregion
Example: query coverduration(pmregion1)

3.4.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr coverdurationtypemap(ListExpr args) {
    std::string err = "pmregion expected";
    int len = nl->ListLength(args);
    if (len != 1) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    return nl->SymbolAtom(PMRegion::BasicType());
}

/*
   3.4.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int coverdurationvalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr1 = static_cast<PMRegion*> (args[0].addr);
    PMRegion *pmr = new PMRegion();
    pmr->pmr = new pmr::PMRegion();


    // Calculate the coverduration in libpmregion
    *pmr->pmr = pmr1->pmr->coverduration2();
    result = pmr;

    return 0;
}

static const std::string coverdurationspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion -> pmregion</text--->"
"<text>coverduration ( _ )</text--->"
"<text>Calculates the coverduration of a pmregion</text--->"
"<text>coverduration pmregion</text---> ) )";

Operator coverduration("coverduration",
        coverdurationspec,
        coverdurationvalmap,
        Operator::SimpleSelect,
        coverdurationtypemap
        );

#endif

/*
   3.5 ~inside~

   Calculates the times, when a moving point is inside the pmregion.

Signature: mpoint x pmregion -> mbool
Example: query mpoint1 inside pmregion1

3.5.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr mpointinsidetypemap(ListExpr args) {
    std::string err = "mpoint x pmregion expected";
    int len = nl->ListLength(args);
    if (len != 2) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!temporalalgebra::MPoint::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    if (!PMRegion::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (second arg wrong)");
    }
    return nl->SymbolAtom(temporalalgebra::MBool::BasicType());
}

/*
   3.5.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int mpointinsidevalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    RList mprl = NL2RList(OutMapping<MPoint, UPoint, OutUPoint>
            (nl->Empty(), args[0])).obj("mpoint", "mpoint");
    PMRegion *pmr = static_cast<PMRegion*> (args[1].addr);

    // Calculate the inside operation in libpmregion
    pmr::MBool mbool = pmr->pmr->mpointinside(mprl);
    // and convert the result to a Secondo nested list
    ListExpr le = RList2NL(mbool.rl.items[4]);

    // Create an MBool from the result and return it
    bool correct;
    ListExpr errorInfo;
    result = InMapping<temporalalgebra::MBool, UBool,
           InConstTemporalUnit<CcBool, InCcBool> >
               (nl->Empty(), le, 0, errorInfo, correct);

    return 0;
}

static const std::string mpointinsidespec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>mpoint x pmregion -> region</text--->"
"<text>_ inside _</text--->"
"<text>Calculates times when mpoint is inside the pmregion</text--->"
"<text>mpoint1 inside pmregion1</text---> ) )";

Operator mpointinside("inside",
        mpointinsidespec,
        mpointinsidevalmap,
        Operator::SimpleSelect,
        mpointinsidetypemap
        );



/*
   3.6 ~union~

   Calculates the union of two ~pmregion~s

Signature: pmregion x pmregion -> pmregion
Example: query pmregion1 union pmregion2

3.6.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr uniontypemap(ListExpr args) {
    std::string err = "pmregion x pmregion expected";
    int len = nl->ListLength(args);
    if (len != 2) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    if (!PMRegion::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (second arg wrong)");
    }
    return nl->SymbolAtom(PMRegion::BasicType());
}

/*
   3.6.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int unionvalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr1 = static_cast<PMRegion*> (args[0].addr);
    PMRegion *pmr2 = static_cast<PMRegion*> (args[1].addr);

    // Create the result object
    PMRegion *pmr = new PMRegion();
    pmr->pmr = new pmr::PMRegion();
    // Calculate the union of the two pmregions
    *pmr->pmr = *pmr1->pmr + *pmr2->pmr;

    result = pmr;

    return 0;
}

static const std::string unionspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion x pmregion -> pmregion</text--->"
"<text>_ union _</text--->"
"<text>Calculates the union of two pmregions</text--->"
"<text>pmreg1 union pmreg2</text---> ) )";

Operator do_union("union",
        unionspec,
        unionvalmap,
        Operator::SimpleSelect,
        uniontypemap
        );


/*
   3.7 ~minus~

   Calculates the difference of two ~pmregion~s

Signature: pmregion x pmregion -> pmregion
Example: query pmregion1 minus pmregion2

3.7.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr minustypemap(ListExpr args) {
    std::string err = "pmregion x pmregion expected";
    int len = nl->ListLength(args);
    if (len != 2) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    if (!PMRegion::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (second arg wrong)");
    }
    return nl->SymbolAtom(PMRegion::BasicType());
}

/*
   3.7.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int minusvalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr1 = static_cast<PMRegion*> (args[0].addr);
    PMRegion *pmr2 = static_cast<PMRegion*> (args[1].addr);

    // Create the result object
    PMRegion *pmr = new PMRegion();
    pmr->pmr = new pmr::PMRegion();
    // Calculate the difference of the two pmregions
    *pmr->pmr = *pmr1->pmr - *pmr2->pmr;

    result = pmr;

    return 0;
}

static const std::string minusspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion x pmregion -> pmregion</text--->"
"<text>_ union _</text--->"
"<text>Calculates the difference of two pmregions</text--->"
"<text>pmreg1 minus pmreg2</text---> ) )";

Operator do_minus("minus",
        minusspec,
        minusvalmap,
        Operator::SimpleSelect,
        minustypemap
        );


/*
   3.8 ~intersection~

   Calculates the union of two ~pmregion~s

Signature: pmregion x pmregion -> pmregion
Example: query intersection(pmregion1, pmregion2)

3.8.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr intersectiontypemap(ListExpr args) {
    std::string err = "pmregion x pmregion expected";
    int len = nl->ListLength(args);
    if (len != 2) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    if (!PMRegion::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (second arg wrong)");
    }
    return nl->SymbolAtom(PMRegion::BasicType());
}

/*
   3.8.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int intersectionvalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr1 = static_cast<PMRegion*> (args[0].addr);
    PMRegion *pmr2 = static_cast<PMRegion*> (args[1].addr);

    // Create the result object
    PMRegion *pmr = new PMRegion();
    pmr->pmr = new pmr::PMRegion();
    // Calculate the intersection of the two pmregions
    *pmr->pmr = *pmr1->pmr * *pmr2->pmr;

    result = pmr;

    return 0;
}

static const std::string intersectionspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion x pmregion -> pmregion</text--->"
"<text>_ union _</text--->"
"<text>Calculates the intersection of two pmregions</text--->"
"<text>intersection(pmreg1, pmreg2)</text---> ) )";

Operator do_intersection("intersection",
        intersectionspec,
        intersectionvalmap,
        Operator::SimpleSelect,
        intersectiontypemap
        );


/*
   3.9 ~intersects~

   Calculates the times, during which two ~pmregion~s intersect.

Signature: pmregion x pmregion -> mbool
Example: query pmregion1 intersects pmregion2

3.9.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr intersectstypemap(ListExpr args) {
    std::string err = "pmregion x pmregion expected";
    int len = nl->ListLength(args);
    if (len != 2) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    if (!PMRegion::checkType(nl->Second(args))) {
        return listutils::typeError(err + " (second arg wrong)");
    }
    return nl->SymbolAtom(temporalalgebra::MBool::BasicType());
}

/*
   3.9.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int intersectsvalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr1 = static_cast<PMRegion*> (args[0].addr);
    PMRegion *pmr2 = static_cast<PMRegion*> (args[1].addr);

    // Perform the "intersects" operation
    pmr::MBool mbool = pmr1->pmr->intersects(*(pmr2->pmr));

    // Convert the result to a Secondo nested list
    ListExpr le = RList2NL(mbool.rl.items[4]);

    // Create an MBool from the result
    bool correct;
    ListExpr errorInfo;
    result = InMapping<temporalalgebra::MBool, UBool,
           InConstTemporalUnit<CcBool, InCcBool> >
               (nl->Empty(), le, 0, errorInfo, correct);

    return 0;
}

static const std::string intersectsspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion x pmregion -> pmregion</text--->"
"<text>_ union _</text--->"
"<text>Calculates the intersection of two pmregions</text--->"
"<text>pmreg1 intersects pmreg2</text---> ) )";

Operator intersects("intersects",
        intersectsspec,
        intersectsvalmap,
        Operator::SimpleSelect,
        intersectstypemap
        );

/*
   3.10 ~pmreg2mreg~

   Converts a pmregion to an mregion

Signature: pmregion -> mregion
Example: query pmreg2mreg(pmregion1)

3.10.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr pmreg2mregtypemap(ListExpr args) {
    std::string err = "pmregion expected";
    int len = nl->ListLength(args);
    if (len != 1) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    return nl->SymbolAtom(temporalalgebra::MRegion::BasicType());
}

/*
   3.10.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int pmreg2mregvalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr = static_cast<PMRegion*> (args[0].addr);

    // Perform the conversion to an mregion
    RList mregion = pmr->pmr->toMRegion();

    // Create a Secondo nested list from the result
    ListExpr le = RList2NL(mregion.items[4]);

    // Create an mregion object from the nested list
    bool correct;
    ListExpr errorInfo;
    result = InMRegion(nl->Empty(), le, 0, errorInfo, correct);

    return 0;
}

static const std::string pmreg2mregspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion -> mregion</text--->"
"<text>pmreg2mreg(_)</text--->"
"<text>Converts a pmregion to an mregion</text--->"
"<text>pmreg2mreg(pmreg1)</text---> ) )";

Operator pmreg2mreg("pmreg2mreg",
        pmreg2mregspec,
        pmreg2mregvalmap,
        Operator::SimpleSelect,
        pmreg2mregtypemap
        );


/*
   3.11 ~mreg2pmreg~

   Converts an mregion to a pmregion

Signature: mregion -> pmregion
Example: query mreg2pmreg(pmregion1)

3.11.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr mreg2pmregtypemap(ListExpr args) {
    std::string err = "mregion expected";
    int len = nl->ListLength(args);
    if (len != 1) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!temporalalgebra::MRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    return nl->SymbolAtom(PMRegion::BasicType());
}

/*
   3.11.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int mreg2pmregvalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    MRegion *mr = static_cast<MRegion*> (args[0].addr);

    // Convert the mregion object to a libpmregion RList
    RList mregion = NL2RList(OutMRegion(nl->Empty(), mr));

    // Create the result object
    PMRegion *pmr = new PMRegion();
    pmr->pmr = new pmr::PMRegion();
    // Create a pmregion from the mregion
    *pmr->pmr = pmr::PMRegion::fromMRegion(
            mregion.obj("mregion", "mregion"));

    result = pmr;

    return 0;
}

static const std::string mreg2pmregspec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>mregion -> pmregion</text--->"
"<text>mreg2pmreg(_)</text--->"
"<text>Converts an mregion to a pmregion</text--->"
"<text>mreg2pmreg(mreg1)</text---> ) )";

Operator mreg2pmreg("mreg2pmreg",
        mreg2pmregspec,
        mreg2pmregvalmap,
        Operator::SimpleSelect,
        mreg2pmregtypemap
        );

/*
   3.12 ~translate~

   Translates a pmregion

Signature: pmregion x real x real x real -> pmregion
Example: query pmregion1 translate [100,100,0]

3.11.1 ~Type mapping~

Maps the source types to the result type. Only one variant is supported here.

*/
ListExpr translatetypemap(ListExpr args) {
    std::string err = "mregion expected";
    int len = nl->ListLength(args);
    if (len != 2) {
        return listutils::typeError(err + " (wrong number of arguments)");
    }
    if (!PMRegion::checkType(nl->First(args))) {
        return listutils::typeError(err + " (first arg wrong)");
    }
    ListExpr coords = nl->Second(args);
    if (nl->ListLength(coords) != 3) {
        return listutils::typeError(err + " (need three coordinates)");
    }
    if (!nl->IsEqual(nl->First(coords), CcReal::BasicType()) ||
            !nl->IsEqual(nl->Second(coords), CcReal::BasicType()) ||
            !nl->IsEqual(nl->Third(coords), CcReal::BasicType())) {
        return listutils::typeError(err + " (wrong type of coordinates)");
    }

    return nl->SymbolAtom(PMRegion::BasicType());
}

/*
   3.11.2 ~Value mapping~

   Maps a result value to the given arguments.

*/
int translatevalmap(Word *args, Word& result,
        int message, Word& local, Supplier s) {
    result = qp->ResultStorage(s);

    PMRegion *pmr = static_cast<PMRegion*> (args[0].addr);


    Word t;
    Supplier son = qp->GetSupplier( args[1].addr, 0 );
    qp->Request( son, t );
    const CcReal *tx = ((CcReal *)t.addr);
    son = qp->GetSupplier( args[1].addr, 1 );
    qp->Request( son, t );
    const CcReal *ty = ((CcReal *)t.addr);
    son = qp->GetSupplier( args[1].addr, 2 );
    qp->Request( son, t );
    const CcReal *tz = ((CcReal *)t.addr);



    // Create the result object
    PMRegion *pmr2 = new PMRegion();
    pmr2->pmr = new pmr::PMRegion();
    // Create a pmregion from the mregion
    *pmr2->pmr = *pmr->pmr;
    pmr2->pmr->translate(tx->GetRealval(), 
            ty->GetRealval(),
            tz->GetRealval());

    result = pmr2;

    return 0;
}

static const std::string translatespec =
"( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
"  (<text>pmregion x real x real x real -> pmregion</text--->"
"<text>_ translate [ _, _, _ ]</text--->"
"<text>translate the object by the given vector.</text--->"
"<text>query pmregion1 translate [100, 200, 0]</text--->"
") )";

Operator translate("translate",
        translatespec,
        translatevalmap,
        Operator::SimpleSelect,
        translatetypemap
        );


/*
   4 ~PMRegionAlgebra~

   Instantiation of the FixedMRegionAlgebra.
   Adds the types ~pmregion~
   Adds the operators ~atinstant~, ~perimeter~, ~area~, ~traversedarea~
   ~inside~, ~union~, ~minus~, ~intersection~, ~intersects~
   ~pmreg2mreg~ and ~mreg2pmreg~

*/
PMRegionAlgebra::PMRegionAlgebra() : Algebra() {
    AddTypeConstructor(&pmregion);
    AddOperator(&atinstant);
    AddOperator(&translate);
    AddOperator(&perimeter);
    AddOperator(&area);
    AddOperator(&traversedarea);
    AddOperator(&mpointinside);
    AddOperator(&do_union);
    AddOperator(&do_minus);
    AddOperator(&do_intersection);
    AddOperator(&intersects);
    AddOperator(&pmreg2mreg);
    AddOperator(&mreg2pmreg);
#if CGAL_VERSION_NR >= 1041400000
    AddOperator(&coverduration);
#endif
}

extern "C"
Algebra *
InitializePMRegionAlgebra(NestedList *nlRef, QueryProcessor *qpRef) {
    nl = nlRef;
    qp = qpRef;
    return new PMRegionAlgebra();
}
}

/*
   5 Conversion functions

   These functions convert between Secondo NestedLists and libpmregion RLists.
   Supported types are: List, String, Double, Bool and Symbol, which are all
   existing RList types. NestedList Integers are converted to double.
   Unknown types are dropped.

   5.1 ~RList2NL~

   Convert a libpmregion RList to a Secondo NestedList.

*/
ListExpr RList2NL(RList r) {
    ListExpr ret = nl->Empty();
    ListExpr cur = ret;

    switch (r.getType()) {
        case NL_LIST:
            for (unsigned int i = 0; i < r.items.size(); i++) {
                if (i == 0)
                    ret = cur = nl->OneElemList(RList2NL(r.items[i]));
                else
                    cur = nl->Append(cur, RList2NL(r.items[i]));
            }
            break;
        case NL_STRING:
            ret = nl->StringAtom(r.getString());
            break;
        case NL_DOUBLE:
            ret = nl->RealAtom(r.getNr());
            break;
        case NL_BOOL:
            ret = nl->BoolAtom(r.getBool());
            break;
        case NL_SYM:
            ret = nl->SymbolAtom(r.getSym());
            break;
    }

    return ret;
}
/*
   5.2 ~NL2RList~

   Convert a Secondo NestedList to a libpmregion RList.

*/
RList NL2RList(ListExpr l) {
    RList ret;

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
            ret.appendsym(nl->SymbolValue(i));
        }
        l = nl->Rest(l);
    }

    return ret;
}
