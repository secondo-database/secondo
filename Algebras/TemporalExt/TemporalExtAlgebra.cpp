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

0 TODO


1 Includes and Initialization

Place for initialization of pointer variables, constants and namespaces and
inclusion of header files concerning Secondo.

*/
#include <set>
#include <time.h>
#include <vector>
#include <map>
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "TemporalAlgebra.h"
#include "MovingRegionAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

#include "DateTime.h"
using namespace datetime;

#include "TemporalExtAlgebra.h"

/*

2 Type definitions, Auxiliary Functions

*/
struct USegments
{
    int unitnr;
    vector<MSegmentData> sgms;
};

struct GroupOfIntervals
{
    int unit_nr;
    Interval<Instant> str_inst;
};

void MinMaxValueFunction(const UReal* utemp, double& minimum, double& maximum)
{
    /*cout << endl;
    cout << "==========> Starting MinMaxValueFunction()" << endl;*/
    double t0, t1, t0_value, t1_value, a, b, c;
    double t_extrem, t_extrem_value;
    //double minimum, maximum;
    bool lh = utemp->timeInterval.lc;
    bool rh = utemp->timeInterval.rc;
    bool conv_conc = true;


    t0 = utemp->timeInterval.start.ToDouble();
    t1 = utemp->timeInterval.end.ToDouble();

    a = utemp->a;
    b = utemp->b;
    c = utemp->c;

    t0_value = a * pow( t0, 2 ) + b * t0 + c;
    t1_value = a * pow( t1, 2 ) + b * t1 + c;

    if( utemp->a != 0 )
    {
        t_extrem = - b / ( 2 * a );
        if( (!lh && !rh && ( t_extrem <= t0 || t_extrem >= t1 ) ) ||
            (lh && rh && ( t_extrem < t0 || t_extrem > t1) ) ||
            (!lh && rh && ( t_extrem <= t0 || t_extrem > t1 ) ) ||
            (lh && !rh && ( t_extrem < t0 && t_extrem >= t1 ) )
        )
        conv_conc = false;
    }
    else
        conv_conc = false;

    if(conv_conc)
    {

        t_extrem_value = a * pow( t_extrem, 2 ) + b * t_extrem + c;
        if(t0_value < t_extrem_value)
        {
        /* The parabola is concave */
        //cout << "--->CONCAVE!" << endl;
        maximum = t_extrem;
        if(t0_value < t1_value)
            minimum = t0;
        else
            minimum = t1;
        }
        else
        {
        /* The parabola is convex */
        //cout << "--->CONVEX!" << endl;
        minimum = t_extrem;
        if(t0_value < t1_value)
            maximum = t1;
        else
            maximum = t0;
        }
    }
    else
    {
        //cout << "--->PIECE OF CURVE OR A LINEAR ECUATION!" << endl;
        if(t0_value < t1_value)
        {
        maximum = t1;
        minimum = t0;
        //cout << "---> Curve goes up!!" << endl;
        }
        else
        {
        maximum = t0;
        minimum = t1;
        //cout << "---> Curve goes down!!" << endl;
        }
    }

    /*cout << "---> maximum: " << maximum << endl;
    cout << "---> minimum: " << minimum << endl;
    cout << "==========> Ending MinMaxValueFunction()" << endl;
    cout << endl;*/
}

/*
2.1 Auxiliary Funcions

2.1.1 Aux. Function ~CheckURealDerivable~


*/
bool CheckURealDerivable(const UReal* unit)
{
    UReal* tmp_unit = (UReal*)unit;
    return tmp_unit->r;
}

/*
3.1 Type Constructor ~istring~

Type ~istring~ represents an (instant, value)-pair of strings.

The list representation of an ~istring~ is

----    ( t string-value )
----

For example:

----    ( (instant 1.0) "My String" )
----

3.1.1 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeStringProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(istring) "),
                             nl->StringAtom("(instant string-value)"),
                             nl->StringAtom("((instant 0.5) My String)"))));
}

/*
3.1.2 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckIntimeString( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "istring" ));
}

/*
3.1.3 Creation of the type constructor ~istring~

*/
TypeConstructor intimestring(
        "istring",  //name
        IntimeStringProperty,   //property function describing signature
        OutIntime<CcString, OutCcString>,
        InIntime<CcString, InCcString>,     //Out and In functions
        0,
        0,  //SaveToList and RestoreFromList functions
        CreateIntime<CcString>,
        DeleteIntime<CcString>, //object creation and deletion
        0,
        0,  // object open and save
        CloseIntime<CcString>,
        CloneIntime<CcString>,  //object close and clone
        CastIntime<CcString>,   //cast function
        SizeOfIntime<CcString>, //sizeof function
        CheckIntimeString       //kind checking function
);

/*
3.2 Type Constructor ~ustring~

Type ~ustring~ represents an (tinterval, stringvalue)-pair.

3.2.1 List Representation

The list representation of an ~ustring~ is

----    ( timeinterval string-value )
----

For example:

----    ( ( (instant 6.37)  (instant 9.9)   TRUE FALSE)   "My String" )
----

3.2.2 function Describing the Signature of the Type Constructor

*/
ListExpr
UStringProperty()
{
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
    nl->FourElemList(nl->StringAtom("-> UNIT"),
                     nl->StringAtom("(ustring) "),
                     nl->StringAtom("(timeInterval string) "),
                     nl->StringAtom("((i1 i2 FALSE FALSE) My String)"))));
}

/*
3.2.3 Kind Checking Function

*/
bool
CheckUString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "ustring" ));
}

/*
3.2.4 Creation of the type constructor ~uint~

*/
TypeConstructor unitstring(
        "ustring",  //name
        UStringProperty,    //property function describing signature
        OutConstTemporalUnit<CcString, OutCcString>,
        InConstTemporalUnit<CcString, InCcString>,  //Out and In functions
        0,
        0,  //SaveToList and RestoreFromList functions
        CreateConstTemporalUnit<CcString>,
        DeleteConstTemporalUnit<CcString>,  //object creation and deletion
        0,
        0,  // object open and save
        CloseConstTemporalUnit<CcString>,
        CloneConstTemporalUnit<CcString>,   //object close and clone
        CastConstTemporalUnit<CcString>,    //cast function
        SizeOfConstTemporalUnit<CcString>,  //sizeof function
        CheckUString    //kind checking function
);

/*
3.3 Type Constructor ~mstring~

Type ~mstring~ represents a moving string.

3.3.1 List Representation

The list representation of a ~mstring~ is

----    ( u1 ... un )
----

,where u1, ..., un are units of type ~ustring~.

For example:

----    (
          ( (instant 6.37)  (instant 9.9)   TRUE FALSE) "String 1" )
          ( (instant 11.4)  (instant 13.9)  FALSE FALSE) "String 2" )
        )
----

3.3.2 function Describing the Signature of the Type Constructor

*/
ListExpr
MStringProperty()
{
    return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
    nl->FourElemList(nl->StringAtom("-> MAPPING"),
                     nl->StringAtom("(mstring) "),
                     nl->StringAtom("( u1 ... un)"),
                     nl->StringAtom("(((i1 i2 TRUE TRUE) My String) ...)"))));
}

/*
3.3.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckMString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "mstring" ));
}

/*
3.3.4 Creation of the type constructor ~mstring~

*/
TypeConstructor movingstring(
    "mstring",  //name
    MStringProperty,    //property function describing signature
    //Out and In functions
    OutMapping<MString, UString, OutConstTemporalUnit<CcString, OutCcString> >,
    InMapping<MString, UString, InConstTemporalUnit<CcString, InCcString> >,
    0,
    0,  //SaveToList and RestoreFromList functions
    CreateMapping<MString>,
    DeleteMapping<MString>,     //object creation and deletion
    0,
    0,  // object open and save
    CloseMapping<MString>,
    CloneMapping<MString>,  //object close and clone
    CastMapping<MString>,   //cast function
    SizeOfMapping<MString>, //sizeof function
    CheckMString    //kind checking function
);

/*
3.4 Type Constructor ~rbool~

Type ~rbool~ represents a range bool.

3.4.1 List Representation

The list representation of a ~rbool~ is

----    ( (bb1 eb1 lc1 rc1) (bb2 eb2 lc2 rc2) ... (bbn ebn lcn rcn) )
----


For example:

----    (
          ( (TRUE FALSE TRUE FALSE)  (FALSE FALSE TRUE TRUE) )
        )
----

3.4.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeBoolProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
    " e.g. (TRUE TRUE TRUE FALSE) means the range [TRUE, TRUE[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rbool) "),
        nl->StringAtom("((bb1 eb1 lci rci) ... (bbn ebn lci rci))"),
        nl->StringAtom("((TRUE TRUE TRUE FALSE) (FALSE FALSE TRUE TRUE))"),
                             remarkslist)));
}

/*
3.3.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckRBool( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "rbool" ));
}

/*
3.3.4 Creation of the type constructor ~rbool~

*/
TypeConstructor rangebool(
    "rbool",  //name
     RangeBoolProperty,   //property function describing signature
     OutRange<CcBool, OutCcBool>,
     InRange<CcBool, InCcBool>,                 //Out and In functions
     0,            0,  //SaveToList and RestoreFromList functions
     CreateRange<CcBool>,DeleteRange<CcBool>,   //object creation and deletion
     OpenRange<CcBool>,  SaveRange<CcBool>,     // object open and save
     CloseRange<CcBool>, CloneRange<CcBool>,    //object close and clone
     CastRange<CcBool>,                        //cast function
     SizeOfRange<CcBool>,                      //sizeof function
     CheckRBool                          //kind checking function
);

/*
3.5 Type Constructor ~rstring~

Type ~rstring~ represents a range string.

3.5.1 List Representation

The list representation of a ~rstring~ is

----    ( (bs1 es1 lc1 rc1) (bs2 es2 lc2 rc2) ... (bsn esn lcn rcn) )
----


For example:

----    (
          ( ("First string" "Second string" TRUE FALSE)
          ("New York" "Washington" TRUE TRUE) )
        )
----

3.5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeStringProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,
  "lci means left closed interval, rci respectively right closed interval,"
    " e.g. (String1 String2 TRUE FALSE) means the range [String1, String2[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("-> RANGE"),
                             nl->StringAtom("(rstring) "),
        nl->StringAtom("((bs1 es1 lci rci) ... (bsn esn lci rci))"),
        nl->StringAtom("((String1 String2 TRUE FALSE))"), remarkslist)));
}

/*
3.3.3 Kind Checking Function

This function checks whether the type constructor is applied correctly.

*/
bool
CheckRString( ListExpr type, ListExpr& errorInfo )
{
    return (nl->IsEqual( type, "rstring" ));
}

/*
3.3.4 Creation of the type constructor ~rstring~

*/
TypeConstructor rangestring(
    "rstring",  //name
     RangeStringProperty,   //property function describing signature
     OutRange<CcString, OutCcString>,
     InRange<CcString, InCcString>,                 //Out and In functions
     0,            0,  //SaveToList and RestoreFromList functions
     CreateRange<CcString>,DeleteRange<CcString>, //obj. creation and deletion
     OpenRange<CcString>,  SaveRange<CcString>,     // object open and save
     CloseRange<CcString>, CloneRange<CcString>,    //object close and clone
     CastRange<CcString>,                        //cast function
     SizeOfRange<CcString>,                      //sizeof function
     CheckRString                          //kind checking function
);

/*
4 Operators

4.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

4.1.1 Type mapping function ~MovingInstantTypeMapIntime~

It is for the operator ~atinstant~.

*/
ListExpr
MovingInstantExtTypeMapIntime( ListExpr args )
{
    if ( nl->ListLength( args ) == 2 )
    {
        ListExpr arg1 = nl->First( args ),
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg2, "instant" ) )
        {
            if( nl->IsEqual( arg1, "mstring" ) )
                return nl->SymbolAtom( "istring" );
        }
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.2 Type mapping function ~MovingPeriodsTypeMapMoving~

It is for the operator ~atperiods~.

*/
ListExpr
MovingPeriodsExtTypeMapMoving( ListExpr args )
{
    if ( nl->ListLength( args ) == 2 )
    {
        ListExpr arg1 = nl->First( args ),
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg2, "periods" ) )
        {
            if( nl->IsEqual( arg1, "mstring" ) )
                return nl->SymbolAtom( "mstring" );

            if( nl->IsEqual( arg1, "movingregion" ) )
                return nl->SymbolAtom( "movingregion" );
        }
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.3 Type mapping function ~MovingTypeMapeIntime~

It is for the operators ~initial~ and ~final~.

*/
ListExpr
MovingExtTypeMapIntime( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mstring" ) )
            return nl->SymbolAtom( "istring" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.4 Type mapping function ~MovingInstantPeriodsTypeMapBool~

It is for the operator ~present~.

*/
ListExpr
MovingInstantPeriodsExtTypeMapBool( ListExpr args )
{
    if ( nl->ListLength( args ) == 2 )
    {
        ListExpr arg1 = nl->First( args ),
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg2, "instant" ) ||
          nl->IsEqual( arg2, "periods" ) )
        {
            if( nl->IsEqual( arg1, "mstring" ))
                return nl->SymbolAtom( "bool" );
        }
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.5 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~at~.

*/
ListExpr
MovingBaseExtTypeMapMoving( ListExpr args )
{
    cout << "----->Type Mapping Function: MovingBaseTypeMapMoving" << endl;
    ListExpr arg1, arg2;
    if( nl->ListLength( args ) == 2 )
    {
        arg1 = nl->First( args );
        arg2 = nl->Second( args );

        if( nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "bool" ) )
            return nl->SymbolAtom( "mbool" );

        if( nl->IsEqual( arg1, "mint" ) &&
            ( nl->IsEqual( arg2, "int" ) ||
            nl->IsEqual( arg2, "rint" ) ) )
            return nl->SymbolAtom( "mint" );

// VTA - This operator is not yet implemented for the type of ~mreal~
        if( nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "real" ) )
            return nl->SymbolAtom( "mreal" );

        if( nl->IsEqual( arg1, "mstring" ) && nl->IsEqual( arg2, "string" ) )
            return nl->SymbolAtom( "mstring" );

    }
    cout << "---------->Return NICHT OK!!" << endl;
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.6 Type mapping function ~MovingBaseTypeMapBool~

It is for the operator ~passes~.

*/
ListExpr
MovingBaseExtTypeMapBool( ListExpr args )
{
    ListExpr arg1, arg2;
    if( nl->ListLength( args ) == 2 )
    {
        arg1 = nl->First( args );
        arg2 = nl->Second( args );

        if( (nl->IsEqual( arg1, "mbool" ) && nl->IsEqual( arg2, "bool" )) ||
            (nl->IsEqual( arg1, "mint" ) && nl->IsEqual( arg2, "int" )) ||
// VTA - This operator is not yet implemented for the type of ~mreal~
            (nl->IsEqual( arg1, "mreal" ) && nl->IsEqual( arg2, "real" )) ||
            (nl->IsEqual( arg1, "mstring" ) && nl->IsEqual( arg2, "string" )) ||
            (nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "region" )))
          return nl->SymbolAtom( "bool" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.7 Type mapping function ~MovingTypeMapRange~

It is for the operator ~deftime~.

*/
ListExpr
MovingExtTypeMapPeriods( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mstring" ))

            return nl->SymbolAtom( "periods" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.8 Type mapping function ~IntimeTypeMapInstant~

It is for the operator ~inst~.

*/
ListExpr
IntimeExtTypeMapInstant( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "istring" ))
            return nl->SymbolAtom( "instant" );

        if( nl->IsEqual( arg1, "intimeregion" ))
            return nl->SymbolAtom( "instant" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.9 Type mapping function ~IntimeTypeMapBase~

It is for the operator ~val~.

*/
ListExpr
IntimeExtTypeMapBase( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "istring" ) )
            return nl->SymbolAtom( "string" );

        if( nl->IsEqual( arg1, "intimeregion" ) )
            return nl->SymbolAtom( "region" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.10 Type mapping function ~MovingRExtTypeMapMovingR~

It is for the operator ~derivative~.

*/
ListExpr
MovingRExtTypeMapMovingR( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mreal" ) )
            return nl->SymbolAtom( "mreal" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.11 Type mapping function ~MovingRExtTypeMapBool~

It is for the operator ~derivable~.

*/
ListExpr
MovingRExtTypeMapBool( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mreal" ) )
            return nl->SymbolAtom( "mbool" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.12 Type mapping function ~MovingPointExtTypeMapMReal~

It is for the operator ~speed~.

*/
ListExpr
MovingPointExtTypeMapMReal( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mpoint" ) )
            return nl->SymbolAtom( "mreal" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.13 Type mapping function RangeRangevaluesExtTypeMapRange

It is for the operator ~rangevalues~.

*/
ListExpr
RangeRangevaluesExtTypeMapRange( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mbool" ) )
            return nl->SymbolAtom( "rbool" );

        if( nl->IsEqual( arg1, "mint" ) )
            return nl->SymbolAtom( "rint" );

        if( nl->IsEqual( arg1, "mstring" ) )
            return nl->SymbolAtom( "rstring" );

        if( nl->IsEqual( arg1, "mreal" ) )
            return nl->SymbolAtom( "rreal" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.14 Type mapping function MovingSANExtTypeMap

It is for the operators ~sometimes~, ~always~ and ~never~.

*/
ListExpr
MovingSANExtTypeMap( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mbool" ) )
            return nl->SymbolAtom( "bool" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.1.15 Type mapping function ~MPointExtTypeMapMPoint~

It is for the operator ~velocity~.

*/
ListExpr
MPointExtTypeMapMPoint( ListExpr args )
{
    if ( nl->ListLength( args ) == 1 )
    {
        ListExpr arg1 = nl->First( args );

        if( nl->IsEqual( arg1, "mpoint" ) )
            return nl->SymbolAtom( "mpoint" );
    }
    return nl->SymbolAtom( "typeerror" );
}

/*
4.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

4.2.1 Selection function ~MovingSimpleSelect~

Is used for the ~deftime~, ~initial~, ~final~, ~inst~, ~val~,
~atinstant~,
~atperiods~  operations.

*/
int
MovingExtSimpleSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "mstring" )
        return 0;

    return -1; // This point should never be reached
}

/*
4.2.2 Selection function ~MovingInstantPeriodsSelect~

Is used for the ~present~ operations.

*/
int
MovingExtInstantPeriodsSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args ),
    arg2 = nl->Second( args );

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "instant" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "periods" )
        return 1;

    return -1; // This point should never be reached
}

/*
4.2.3 Selection function MovingBaseRangeSelect

Is used for the ~at~ operations.

*/
int
MovingExtBaseRangeSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args ),
    arg2 = nl->Second( args );

    if( nl->SymbolValue( arg1 ) == "mbool" &&
        nl->SymbolValue( arg2 ) == "bool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "int" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "real" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "string" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "rint" )
        return 4;

  /*if( nl->SymbolValue( arg1 ) == "mreal" &&
    nl->SymbolValue( arg2 ) == "rreal"
)
    return 6;*/

    return -1; // This point should never be reached
}

/*
4.2.4 Selection function ~MovingBaseSelect~

Is used for the ~passes~ operations.

*/
int
MovingExtBaseSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args ),
    arg2 = nl->Second( args );

    if( nl->SymbolValue( arg1 ) == "mbool" &&
        nl->SymbolValue( arg2 ) == "bool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "int" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "real" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "string" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "region" )
        return 4;

    return -1; // This point should never be reached
}

/*
4.2.5 Selection function ~IntimeSimpleSelect~

Is used for the ~inst~ and ~val~ operations.

*/
int
IntimeExtSimpleSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "istring" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "intimeregion" )
        return 1;

    return -1; // This point should never be reached
}

/*
4.2.6 Selection function RangeRangevaluesExtBaseSelect

Is used for the ~rangevalues~ operations.

*/
int
RangeRangevaluesExtBaseSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "mbool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mstring" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mreal" )
        return 3;

    return -1; // This point should never be reached
}

/*
4.2.7 Selection function ~MovingPeriodsSelect~

Is used for the ~atperiods~  operation.

*/
int
MovingPeriodsSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "mstring" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "movingregion" )
        return 1;

    return -1; // This point should never be reached
}

/*
4.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

4.3.1 Value mapping functions of operator ~atinstant~

*/
template <class Mapping, class Alpha>
int MappingAtInstantExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );
    Intime<Alpha>* pResult = (Intime<Alpha>*)result.addr;

    ((Mapping*)args[0].addr)->AtInstant( *((Instant*)args[1].addr), *pResult );

    return 0;
}

/*
4.3.2 Value mapping functions of operator ~atperiods~

*/
template <class Mapping>
int MappingAtPeriodsExt( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s )
{
    result = qp->ResultStorage( s );
    ((Mapping*)args[0].addr)->AtPeriods( *((Periods*)args[1].addr),
    *((Mapping*)result.addr) );
    return 0;
}

int MappingAtPeriodsExtMRegion( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s )
{
    result = qp->ResultStorage( s );
    MRegion* mr = (MRegion*)args[0].addr;
    Periods* per = (Periods*)args[1].addr;
    MRegion* pResult = (MRegion*)result.addr;

    /*const URegionEmb* utemp;
    const Interval<Instant>* temp2;
    Interval<Instant> temp3;
    const MSegmentData* oldsmg;
    const MSegmentData* temp_smg;
    MSegmentData newsmg;
    vector<GroupOfIntervals> temp_intervals;
    GroupOfIntervals inter_temp;
    DBArray<MSegmentData>* tempsgms;
    URegion* out_ureg;

    for(int i=0;i<mr->GetNoComponents();i++)
    {
        mr->Get(i, utemp);
        for(int j=0;j<per->GetNoComponents();j++)
        {
            per->Get(j, temp2);
            if((utemp->timeInterval).Intersects(*temp2))
            {
                (utemp->timeInterval).Intersection(*temp2, temp3);
                inter_temp.unit_nr = i;
                (inter_temp.str_inst).CopyFrom(temp3);
                temp_intervals.push_back(inter_temp);
            }
        }
    }
    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<mr->GetNoComponents();i++)
    {
        mr->Get(i, utemp);
        tempsgms = new DBArray<MSegmentData>( utemp->GetSegmentsNum() );
        for(int k=0;k<utemp->GetSegmentsNum();k++)
        {
            utemp->GetSegment(k, oldsmg);
            oldsmg->restrictToInterval(utemp->timeInterval,
            temp3,
            newsmg);
            tempsgms->Append(newsmg);
        }
        out_ureg = new URegion(temp_intervals[i].str_inst);
        out_ureg->SetMSegmentData(tempsgms);
        for(int t=0;t<tempsgms->Size();t++)
        {
            tempsgms->Get(t, temp_smg);
            cout << endl << i << ": [";
            cout << temp_smg->GetInitialStartX() << " , ";
            cout << temp_smg->GetInitialStartY() << endl;
            cout << temp_smg->GetInitialEndX() << " , ";
            cout << temp_smg->GetInitialEndY() << endl;
            cout << temp_smg->GetFinalStartX() << " , ";
            cout << temp_smg->GetFinalStartY() << endl;
            cout << temp_smg->GetFinalEndX() << " , ";
            cout << temp_smg->GetFinalEndY() << endl;
            cout << "]" << endl;
        }
        pResult->Add(*out_ureg);
    }
    pResult->EndBulkLoad( false );*/


    return 0;
}

/*
4.3.3 Value mapping functions of operator ~initial~

*/
template <class Mapping, class Unit, class Alpha>
int MappingInitialExt( Word* args,
                       Word& result,
                       int message,
                       Word& local,
                       Supplier s )
{
    result = qp->ResultStorage( s );
    ((Mapping*)args[0].addr)->Initial( *((Intime<Alpha>*)result.addr) );
    return 0;
}

/*
4.3.4 Value mapping functions of operator ~final~

*/
template <class Mapping, class Unit, class Alpha>
int MappingFinalExt( Word* args,
                     Word& result,
                     int message,
                     Word& local,
                     Supplier s )
{
    result = qp->ResultStorage( s );
    ((Mapping*)args[0].addr)->Final( *((Intime<Alpha>*)result.addr) );
    return 0;
}

/*
4.3.5 Value mapping functions of operator ~present~

*/
template <class Mapping>
int MappingPresentExt_i( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping *m = ((Mapping*)args[0].addr);
    Instant* inst = ((Instant*)args[1].addr);

    if( !inst->IsDefined() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Present( *inst ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

template <class Mapping>
int MappingPresentExt_p( Word* args,
                         Word& result,
                         int message,
                         Word& local,
                         Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping *m = ((Mapping*)args[0].addr);
    Periods* periods = ((Periods*)args[1].addr);

    if( periods->IsEmpty() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Present( *periods ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

/*
4.3.6 Value mapping functions of operator ~at~

*/
template <class Mapping, class Unit, class Alpha>
int MappingAtExt( Word* args,
                  Word& result,
                  int message,
                  Word& local,
                  Supplier s )
{
  result = qp->ResultStorage( s );

  Mapping *m = ((Mapping*)args[0].addr);
  Alpha* val = ((Alpha*)args[1].addr);
  Mapping* pResult = ((Mapping*)result.addr);
  pResult->Clear();
  m->At( *val, *pResult );

  return 0;
}

/*
4.3.7 Value mapping functions of operator ~passes~

*/
template <class Mapping, class Alpha>
int MappingPassesExt( Word* args,
                      Word& result,
                      int message,
                      Word& local,
                      Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping *m = ((Mapping*)args[0].addr);
    Alpha* val = ((Alpha*)args[1].addr);

    if( !val->IsDefined() )
        ((CcBool *)result.addr)->Set( false, false );
    else if( m->Passes( *val ) )
        ((CcBool *)result.addr)->Set( true, true );
    else
        ((CcBool *)result.addr)->Set( true, false );

    return 0;
}

/*
4.3.8 Value mapping functions of operator ~deftime~

*/
template <class Mapping>
int MappingDefTimeExt( Word* args,
                       Word& result,
                       int message,
                       Word& local,
                       Supplier s )
{
    result = qp->ResultStorage( s );
    ((Mapping*)args[0].addr)->DefTime( *(Periods*)result.addr );
    return 0;
}

/*
4.3.9 Value mapping functions of operator ~inst~

*/
template <class Alpha>
int IntimeInstExt( Word* args,
                   Word& result,
                   int message,
                   Word& local,
                   Supplier s )
{
    result = qp->ResultStorage( s );
    Intime<Alpha>* i = (Intime<Alpha>*)args[0].addr;

    if( i->IsDefined())
        ((Instant*)result.addr)->CopyFrom(
        &((Intime<Alpha>*)args[0].addr)->instant);
    else
        ((Instant*)result.addr)->SetDefined( false );

    return 0;
}

/*
4.3.10 Value mapping functions of operator ~val~

*/
template <class Alpha>
int IntimeValExt( Word* args,
                  Word& result,
                  int message,
                  Word& local,
                  Supplier s )
{
    result = qp->ResultStorage( s );
    Intime<Alpha>* i = (Intime<Alpha>*)args[0].addr;

    if( i->IsDefined() )
        ((Alpha*)result.addr)->CopyFrom(
        &((Intime<Alpha>*)args[0].addr)->value );
    else
        ((Alpha*)result.addr)->SetDefined( false );

    return 0;
}

/*
4.3.11 Value mapping functions of operator ~derivativeext~

*/
template <class Mapping>
int MovingDerivativeExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping* m = ((Mapping*)args[0].addr);
    Mapping* pResult = ((Mapping*)result.addr);
    const UReal* unitin;
    UReal unitout;

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        if(!CheckURealDerivable(unitin))
        {
            unitout.a = 0.;
            unitout.b = 2*unitin->a;
            unitout.c = unitin->b;
            unitout.r = false;
            unitout.timeInterval = unitin->timeInterval;
            pResult->Add(unitout);
        }
    }
    pResult->EndBulkLoad( false );

    return 0;
}

/*
4.3.12 Value mapping functions of operator ~derivableext~

*/
template <class Mapping>
int MovingDerivableExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping* m = ((Mapping*)args[0].addr);
    MBool* pResult = ((MBool*)result.addr);
    const UReal* unitin;
    UBool unitout;
    CcBool myValue;

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        if(i==0)
        {
            /*
            Steps for the first Unit

            */
            myValue.Set(true, !unitin->r);
            unitout.constValue.CopyFrom(&myValue);

            if(m->GetNoComponents()==1)
            {
                /*
                Only one Unit available

                */
                unitout.timeInterval = unitin->timeInterval;
                pResult->Add(unitout);
            }
            else
            {
                /*
                Just the first Unit ...

                */
                unitout.timeInterval.start = unitin->timeInterval.start;
                unitout.timeInterval.end = unitin->timeInterval.end;
                unitout.timeInterval.lc = unitin->timeInterval.lc;
                unitout.timeInterval.rc = unitin->timeInterval.rc;
            }
        }
        else
        {
            if(!unitin->r == unitout.constValue.GetBoolval())
            {
                /*
                The Unit has the same Bool value as the
                previous. So, the time interval of the previous
                Unit will be extended.

                */
                unitout.timeInterval.end = unitin->timeInterval.end;
                unitout.timeInterval.rc = unitin->timeInterval.rc;
            }
            else
            {
                /*
                The current Unit has another Bool value as the
                previous. The previous Unit must be created. New
                values must be assumed from the current Unit.

                */
                pResult->Add(unitout);
                myValue.Set(true, !unitin->r);
                unitout.constValue.CopyFrom(&myValue);
                unitout.timeInterval.start = unitin->timeInterval.start;
                unitout.timeInterval.end = unitin->timeInterval.end;
                unitout.timeInterval.lc = unitin->timeInterval.lc;
                unitout.timeInterval.rc = unitin->timeInterval.rc;
            }
            if(i==m->GetNoComponents()-1)
            {
                /*
                Last Unit must be created.

                */
                pResult->Add(unitout);
            }
        }
    }
    pResult->EndBulkLoad( false );

    return 0;
}

/*
4.3.12 Value mapping functions of operator ~speedext~

*/
template <class Mapping>
int MovingSpeedExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    Mapping* m = ((Mapping*)args[0].addr);
    MReal* pResult = ((MReal*)result.addr);
    double speed, distance, t;
    const Point p0, p1;
    const UPoint* unitin;
    UReal unitout;

    pResult->Clear();
    pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        distance = unitin->p0.Distance(unitin->p1);
        t = unitin->timeInterval.end.ToDouble() -
            unitin->timeInterval.start.ToDouble();
        speed = distance / t;
        unitout.a = 0.;
        unitout.b = 0.;
        unitout.c = speed;
        unitout.r = false;
        unitout.timeInterval = unitin->timeInterval;
        pResult->Add(unitout);
    }
    pResult->EndBulkLoad( false );

    return 0;
}

/*
4.3.13 Value mapping functions of operator ~rangevalues~

*/
int RangeRangevaluesBoolExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MBool* m = ((MBool*)args[0].addr);
    RBool* pResult = ((RBool*)result.addr);

    const UBool* utemp;
    CcBool min, max;
    bool findmin=false, findmax=false, temp;

    m->Get(0, utemp);
    temp = utemp->constValue.GetBoolval();
    min.Set(true, temp);
    max.Set(true, temp);

    for(int i=1;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetBoolval();

        if(temp)
        {
          max.Set(true, temp);
          findmax = true;
        }
        else
        {
          min.Set(true, temp);
          findmin = true;
        }

        if(findmin && findmax) break;
    }

    Interval<CcBool> inter(min, max, true, true);
    pResult->Add(inter);

    return 0;
}

int RangeRangevaluesIntExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MInt* m = ((MInt*)args[0].addr);
    RInt* pResult = ((RInt*)result.addr);

    const UInt* utemp;
    int temp;
    set<int> BTree;
    clock_t clock1, clock2, clock3, clock4;
    float time1, time2;

    clock1 = clock();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetIntval();
        BTree.insert(temp);
    }
    clock2 = clock();
    time1 = ((clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << endl << "Time to insert values: "
          << time1 << " milliseconds" << endl;

    set<int>::iterator iter;
    CcInt mincc, maxcc;
    int min=0, max=0;
    bool start=true;
    Interval<CcInt> inter;

    clock3 = clock();
    pResult->Clear();
    pResult->StartBulkLoad();
    for(iter=BTree.begin(); iter!=BTree.end(); ++iter)
    {
        if(start)
        {
          min = *iter;
          max = min;
          start = false;
        }
        else
        {
          if(*iter-max != 1)
          {
            mincc.Set(true, min);
            maxcc.Set(true, max);
            inter.start = mincc;
            inter.end = maxcc;
            inter.lc = true;
            inter.rc = true;
            pResult->Add(inter);
            min = *iter;
            max = min;
          }
          else
          {
            max = *iter;
          }
        }
    }
    mincc.Set(true, min);
    maxcc.Set(true, max);
    inter.start = mincc;
    inter.end = maxcc;
    inter.lc = true;
    inter.rc = true;
    pResult->Add(inter);
    clock4 = clock();
    time2 = ((clock4-clock3)/CLOCKS_PER_SEC) * 1000.;
    cout << "Time to scan and build intervals: "
          << time2 << " milliseconds" << endl;
    pResult->EndBulkLoad( false );

    return 0;
}

int RangeRangevaluesStringExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MString* m = ((MString*)args[0].addr);
    RString* pResult = ((RString*)result.addr);

    const UString* utemp;
    set<string> BTree;
    string temp;
    clock_t clock1, clock2, clock3, clock4;
    float time1, time2;

    clock1 = clock();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetValue();
        BTree.insert(temp);
    }
    clock2 = clock();
    time1 = ((clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << endl << "Time to insert values: "
          << time1 << " milliseconds" << endl;

    set<string>::iterator iter;
    CcString minmaxcc;
    STRING minmax;
    Interval<CcString> inter;

    clock3 = clock();
    pResult->Clear();
    pResult->StartBulkLoad();
    for(iter=BTree.begin(); iter!=BTree.end(); ++iter)
    {
      temp = *iter;
      for(size_t i=0;i<temp.size();++i)
        minmax[i] = temp[i];
      minmaxcc.Set(true, &minmax);
      inter.start = minmaxcc;
      inter.end = minmaxcc;
      inter.lc = true;
      inter.rc = true;
      pResult->Add(inter);
    }
    clock4 = clock();
    time2 = ((clock4-clock3)/CLOCKS_PER_SEC) * 1000.;
    cout << "Time to scan and build intervals: "
          << time2 << " milliseconds" << endl;
    BTree.clear();
    pResult->EndBulkLoad( false );

    return 0;
}


int RangeRangevaluesRealExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MReal* m = ((MReal*)args[0].addr);
    RReal* pResult = ((RReal*)result.addr);

    clock_t clock1, clock2, clock3, clock4;
    float time1, time2;

    const UReal* utemp;
    double min=0.,max=0.;
    CcReal mincc, maxcc;
    Interval<CcReal> inter;
    multimap< double,const Interval<CcReal> > intermap;

    clock1 = clock();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        MinMaxValueFunction(utemp, min, max);
        mincc.Set(true, min);
        maxcc.Set(true, max);
        inter.start = mincc;
        inter.end = maxcc;
        inter.lc = true;
        inter.rc = true;
        intermap.insert(pair< double,const Interval<CcReal> >(max, inter));
    }
    clock2 = clock();
    time1 = ((clock2-clock1)/CLOCKS_PER_SEC) * 1000.;
    cout << endl << "Time to insert values: "
          << time1 << " milliseconds" << endl;

    multimap< double,const Interval<CcReal> >::iterator iter = intermap.end();
    pResult->Clear();
    pResult->StartBulkLoad();
    --iter;
    bool start=true;
    clock3 = clock();
    while(iter != intermap.begin())
    {
        //cout << "[" << (((*iter).second).start).GetValue();
        //cout << "," << (((*iter).second).end).GetValue() << "]" << endl;
        if(start)
        {
            inter = (*iter).second;
            start = false;
        }
        if(inter.Intersects((*iter).second))
        {
            if(inter.start.GetValue() > ((*iter).second).start.GetValue())
                inter.start = ((*iter).second).start;
        }
        else
        {
            pResult->Add(inter);
            inter = (*iter).second;
        }
        --iter;
    }
    if(inter.Intersects((*iter).second))
    {
        if(inter.start.GetValue() > ((*iter).second).start.GetValue())
            inter.start = ((*iter).second).start;
    }
    pResult->Add(inter);
    clock4 = clock();
    time2 = ((clock4-clock3)/CLOCKS_PER_SEC) * 1000.;
    cout << "Time to scan and build intervals: "
          << time2 << " milliseconds" << endl;

    pResult->EndBulkLoad( false );

    return 0;
}

/*
4.3.14 Value mapping functions of operator ~sometimes~

*/
int MovingSometimesExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MBool* m = ((MBool*)args[0].addr);

    const UBool* utemp;
    bool temp=false;

    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetBoolval();
        if(temp) break;
    }

    if(!m->IsDefined())
        ((CcBool *)result.addr)->Set( false, false );
    else
        ((CcBool *)result.addr)->Set( true, temp );

    return 0;
}

/*
4.3.15 Value mapping functions of operator ~always~

*/
int MovingAlwaysExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MBool* m = ((MBool*)args[0].addr);

    const UBool* utemp;
    bool temp=true;

    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetBoolval();
        if(!temp) break;
    }

    if(!m->IsDefined())
        ((CcBool *)result.addr)->Set( false, false );
    else
        ((CcBool *)result.addr)->Set( true, temp );

    return 0;
}

/*
4.3.16 Value mapping functions of operator ~never~

*/
int MovingNeverExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MBool* m = ((MBool*)args[0].addr);

    const UBool* utemp;
    bool temp=true;

    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, utemp);
        temp = utemp->constValue.GetBoolval();
        if(temp) break;
    }

    if(!m->IsDefined())
        ((CcBool *)result.addr)->Set( false, false );
    else
        ((CcBool *)result.addr)->Set( true, !temp );

    return 0;
}

/*
4.3.17 Value mapping functions of operator ~velocity~

*/
int MovingVelocityExt(
    Word* args,
    Word& result,
    int message,
    Word& local,
    Supplier s )
{
    result = qp->ResultStorage( s );

    MPoint* m = ((MPoint*)args[0].addr);
    MPoint* pResult = ((MPoint*)result.addr);
    //double speed, distance, t;
    //const Point p0, p1;
    const UPoint* unitin;

    //pResult->Clear();
    //pResult->StartBulkLoad();
    for(int i=0;i<m->GetNoComponents();i++)
    {
        m->Get(i, unitin);
        cout << endl;
        cout << "p0: " << unitin->p0.GetX() << ", ";
        cout << unitin->p0.GetY() << endl;
        cout << "p1: " << unitin->p1.GetX() << ", ";
        cout << unitin->p1.GetY() << endl;
        cout << "t0: " << unitin->timeInterval.start.ToDouble() << endl;
        cout << "t1: " << unitin->timeInterval.end.ToDouble() << endl;

        /*t = unitin->timeInterval.end.ToDouble() -
                unitin->timeInterval.start.ToDouble();
        speed = distance / t;*/
        //pResult->Add(unitout);
    }
    //pResult->EndBulkLoad( false );

    return 0;
}

/*
4.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of
value
mapping functions for each operator. For nonoverloaded operators there is also
such and array
defined, so it easier to make them overloaded.

*/

ValueMapping temporalatinstantextmap[] = {
    MappingAtInstantExt<MString, CcString> };

ValueMapping temporalatperiodsextmap[] = {
    MappingAtPeriodsExt<MString>,
    MappingAtPeriodsExtMRegion};

ValueMapping temporalinitialextmap[] = {
    MappingInitialExt<MString, UString, CcString> };

ValueMapping temporalfinalextmap[] = {
    MappingFinalExt<MString, UString, CcString> };

ValueMapping temporalpresentextmap[] = {
    MappingPresentExt_i<MString>,
    MappingPresentExt_p<MString>, };

ValueMapping temporalatextmap[] = {
    MappingAtExt<MBool, UBool, CcBool>,
    MappingAtExt<MInt, UInt, CcInt>,
    MappingAtExt<MReal, UReal, CcReal>,
    MappingAtExt<MString, UString, CcString>
    /*,MappingAt_r<MInt, UInt, RInt>*/ };

ValueMapping temporalpassesextmap[] = {
    MappingPassesExt<MBool, CcBool>,
    MappingPassesExt<MInt, CcInt>,
    MappingPassesExt<MReal, CcReal>,
    MappingPassesExt<MString, CcString>};

ValueMapping temporaldeftimeextmap[] = {
    MappingDefTimeExt<MString> };

ValueMapping temporalinstextmap[] = {
    IntimeInstExt<CcString>,
    IntimeInstExt<CRegion> };


ValueMapping temporalvalextmap[] = {
    IntimeValExt<CcString>,
    IntimeValExt<CRegion> };

ValueMapping temporalderivativeextmap[] = {
    MovingDerivativeExt<MReal> };

ValueMapping temporalderivableextmap[] = {
    MovingDerivableExt<MReal> };

ValueMapping temporalspeedextmap[] = {
    MovingSpeedExt<MPoint> };

ValueMapping rangerangevaluesextmap[] = {
    RangeRangevaluesBoolExt,
    RangeRangevaluesIntExt,
    RangeRangevaluesStringExt,
    RangeRangevaluesRealExt};

ValueMapping temporalsometimesextmap[] = {
    MovingSometimesExt };

ValueMapping temporalalwaysextmap[] = {
    MovingAlwaysExt };

ValueMapping temporalneverextmap[] = {
    MovingNeverExt };

ValueMapping temporalvelocityextmap[] = {
    MovingVelocityExt };

/*
4.5 Specification strings

*/
const string TemporalSpecAtInstantExt  =
    "( ( \"Signature\" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region},</text--->"
    "<text>mT x instant  -> intime(T)</text--->"
    "<text>_ atinstant _ </text--->"
    "<text>get the Intime value corresponding to the instant.</text--->"
    "<text>mpoint1 atinstant instant1</text--->"
    ") )";

const string TemporalSpecAtPeriodsExt  =
    "( ( \"Signature\" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>mT x periods -> moving(T)</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>_ atperiods _ </text--->"
    "<text>restrict the movement to the given periods.</text--->"
    "<text>mpoint1 atperiods periods1</text--->"
    ") )";

const string TemporalSpecInitialExt  =
    "( ( \"Signature\" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>moving(T) -> intime(T)</text--->"
    "<text> initial( _ )</text--->"
    "<text>get the intime value corresponding to the initial instant.</text--->"
    "<text>initial( mpoint1 )</text--->"
    ") )";

const string TemporalSpecFinalExt  =
    "( ( \"Signature\" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>moving(T) -> intime(T)</text--->"
    "<text> final( _ )</text--->"
    "<text>get the intime value corresponding to the final instant.</text--->"
    "<text>final( mpoint1 )</text--->"
    ") )";

const string TemporalSpecPresentExt  =
    "( ( \"Signature\" \" \" \" \" \" \" \"Syntax\" \"Meaning\" \" \""
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>moving(T) x instant -> bool,</text--->"
    "<text>moving(T) x periods -> bool</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>_ present _ </text--->"
    "<text>whether the object is present at the given instant</text--->"
    "<text>or period.</text--->"
    "<text>mpoint1 present instant1</text--->"
    ") )";

const string TemporalSpecAtExt =
    "( ( \"Signature\" \" \" \" \" \" \" \" \" \" \" \" \" \"Syntax\" "
    "\"Meaning\" \" \" \"Example\" ) "
    "( <text>T in {int, bool, real, string},</text--->"
    "<text>moving(T) x T -> moving(T);</text--->"
    "<text>T in {point, points*, line*, region*},</text--->"
    "<text>moving(T) x range(T) -> moving(T);</text--->"
    "<text>moving(region) x point -> mpoint**</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>(**) Operator combination is not implemented yet.</text--->"
    "<text> _ at _ </text--->"
    "<text>restrict the movement at the times where the equality </text--->"
    "<text>occurs.</text--->"
    "<text>mpoint1 at point1</text--->"
    ") )";

const string TemporalSpecPassesExt =
    "( ( \"Signature\" \" \" \" \" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string},</text--->"
    "<text>moving(T) x T -> bool;</text--->"
    "<text>T in {point, points*, line*, region*},</text--->"
    "<text>moving(point) x T -> bool</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text>_ passes _ </text--->"
    "<text>whether the object passes the given value.</text--->"
    "<text>mpoint1 passes point1</text--->"
    ") )";

const string TemporalSpecDefTimeExt  =
    "( ( \"Signature\" \" \" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region*},</text--->"
    "<text>moving(T) -> periods</text--->"
    "<text>(*) Not yet implemented for this type constructor.</text--->"
    "<text> deftime( _ )</text--->"
    "<text>get the defined time of the corresponding moving </text--->"
    "<text>data objects.</text--->"
    "<text>deftime( mp1 )</text--->"
    ") )";

const string TemporalSpecInstExt  =
    "( ( \"Signature\" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region},</text--->"
    "<text>intime(T) -> instant</text--->"
    "<text>inst ( _ )</text--->"
    "<text>Intime time instant.</text--->"
    "<text>inst ( i1 )</text--->"
    ") )";

const string TemporalSpecValExt  =
    "( ( \"Signature\" \" \" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string, point, region},</text--->"
    "<text>intime(T) -> T</text--->"
    "<text>val ( _ )</text--->"
    "<text>Intime value.</text--->"
    "<text>val ( i1 )</text--->"
    ") )";

const string TemporalSpecDerivativeExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>moving(real) -> moving(real)</text--->"
    "<text>derivative ( _ )</text--->"
    "<text>Derivative of a mreal.</text--->"
    "<text>derivative ( mr1 )</text--->"
    ") )";

const string TemporalSpecDerivableExt =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>moving(mreal) -> moving(bool)</text--->"
    "<text>derivable ( _ )</text--->"
    "<text>Checking if mreal is derivable.</text--->"
    "<text>derivable ( mr1 )</text--->"
    ") )";

const string TemporalSpecSpeedExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>moving(point) -> moving(real)</text--->"
    "<text>speed ( _ )</text--->"
    "<text>Velocity of a mpoint given as mreal.</text--->"
    "<text>speed ( mp1 )</text--->"
    ") )";

const string RangeSpecRangevaluesExt  =
    "( ( \"Signature\" \" \" \"Syntax\" \"Meaning\" \" \" "
    "\"Example\" ) "
    "( <text>T in {int, bool, real, string},</text--->"
    "<text>moving(T) -> range(T)</text--->"
    "<text>rangevalues ( _ )</text--->"
    "<text>Returns all the values assumed by the argument over time,</text--->"
    "<text>as a set of intervals.</text--->"
    "<text>rangevalues ( mb1 )</text--->"
    ") )";

const string BoolSpecSometimesExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "(<text>moving(bool) -> bool</text--->"
    "<text>sometimes ( _ )</text--->"
    "<text>Returns true if a unit at least is TRUE.</text--->"
    "<text>sometimes ( mb1 )</text--->"
    ") )";

const string BoolSpecAlwaysExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "(<text>moving(bool) -> bool</text--->"
    "<text>always ( _ )</text--->"
    "<text>Returns true if all units are TRUE.</text--->"
    "<text>always ( mb1 )</text--->"
    ") )";

const string BoolSpecNeverExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "(<text>moving(bool) -> bool</text--->"
    "<text>never ( _ )</text--->"
    "<text>Returns true if all units are FALSE.</text--->"
    "<text>never ( mb1 )</text--->"
    ") )";

const string TemporalSpecVelocityExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>moving(point) -> moving(point)</text--->"
    "<text>velocity ( _ )</text--->"
    "<text>Velocity of a mpoint given as mpoint(a vector function).</text--->"
    "<text>velocity ( mp1 )</text--->"
    ") )";

/*
4.6 Operators

*/
Operator temporalatinstantext(
    "atinstant",
    TemporalSpecAtInstantExt,
    5,
    temporalatinstantextmap,
    MovingExtSimpleSelect,
    MovingInstantExtTypeMapIntime );

Operator temporalatperiodsext(
    "atperiods",
    TemporalSpecAtPeriodsExt,
    2,
    temporalatperiodsextmap,
    MovingPeriodsSelect,
    MovingPeriodsExtTypeMapMoving );

Operator temporalinitialext(
    "initial",

    TemporalSpecInitialExt,
    5,
    temporalinitialextmap,
    MovingExtSimpleSelect,
    MovingExtTypeMapIntime );

Operator temporalfinalext(
    "final",
    TemporalSpecFinalExt,
    5,
    temporalfinalextmap,
    MovingExtSimpleSelect,
    MovingExtTypeMapIntime );

Operator temporalpresentext(
    "present",
    TemporalSpecPresentExt,
    10,
    temporalpresentextmap,
    MovingExtInstantPeriodsSelect,
    MovingInstantPeriodsExtTypeMapBool);

Operator temporalatext(
    "at",
    TemporalSpecAtExt,
    5,
    temporalatextmap,
    MovingExtBaseRangeSelect,
    MovingBaseExtTypeMapMoving );

Operator temporalpassesext(
    "passes",
    TemporalSpecPassesExt,
    5,
    temporalpassesextmap,
    MovingExtBaseSelect,
    MovingBaseExtTypeMapBool);

Operator temporaldeftimeext(
    "deftime",
    TemporalSpecDefTimeExt,
    5,
    temporaldeftimeextmap,
    MovingExtSimpleSelect,
    MovingExtTypeMapPeriods );

Operator temporalinstext(
    "inst",
    TemporalSpecInstExt,
    2,
    temporalinstextmap,
    IntimeExtSimpleSelect,
    IntimeExtTypeMapInstant );

Operator temporalvalext(
    "val",
    TemporalSpecValExt,
    2,
    temporalvalextmap,
    IntimeExtSimpleSelect,
    IntimeExtTypeMapBase );

Operator temporalderivativeext(
    "derivative",
    TemporalSpecDerivativeExt,
    1,
    temporalderivativeextmap,
    Operator::SimpleSelect,
    MovingRExtTypeMapMovingR);

Operator temporalderivableext(
    "derivable",
    TemporalSpecDerivableExt,
    1,
    temporalderivableextmap,
    Operator::SimpleSelect,
    MovingRExtTypeMapBool);

Operator temporalspeedext(
    "speed",
    TemporalSpecSpeedExt,
    1,
    temporalspeedextmap,
    Operator::SimpleSelect,
    MovingPointExtTypeMapMReal);

Operator rangerangevaluesext(
    "rangevalues",
    RangeSpecRangevaluesExt,
    4,
    rangerangevaluesextmap,
    RangeRangevaluesExtBaseSelect,
    RangeRangevaluesExtTypeMapRange );

Operator sometimesext(
    "sometimes",
    BoolSpecSometimesExt,
    1,
    temporalsometimesextmap,
    Operator::SimpleSelect,
    MovingSANExtTypeMap );

Operator alwaysext(
    "always",
    BoolSpecAlwaysExt,
    1,
    temporalalwaysextmap,
    Operator::SimpleSelect,
    MovingSANExtTypeMap );

Operator neverext(
    "never",
    BoolSpecNeverExt,
    1,
    temporalneverextmap,
    Operator::SimpleSelect,
    MovingSANExtTypeMap );

Operator temporalvelocityext(
    "velocity",
    TemporalSpecVelocityExt,
    1,
    temporalvelocityextmap,
    Operator::SimpleSelect,
    MPointExtTypeMapMPoint);

class TemporalExtAlgebra : public Algebra
{
  public:
    TemporalExtAlgebra() : Algebra()
    {
        AddTypeConstructor( &intimestring );

        AddTypeConstructor( &unitstring );

        AddTypeConstructor( &movingstring );

        AddTypeConstructor( &rangebool );
        AddTypeConstructor( &rangestring );

        intimestring.AssociateKind( "TEMPORAL" );
        intimestring.AssociateKind( "DATA" );

        unitstring.AssociateKind( "TEMPORAL" );
        unitstring.AssociateKind( "DATA" );

        movingstring.AssociateKind( "TEMPORAL" );
        movingstring.AssociateKind( "DATA" );

        rangebool.AssociateKind( "RANGE" );
        rangebool.AssociateKind( "DATA" );

        rangestring.AssociateKind( "RANGE" );
        rangestring.AssociateKind( "DATA" );

        AddOperator( &temporalatinstantext );
        AddOperator( &temporalatperiodsext );
        AddOperator( &temporalinitialext );
        AddOperator( &temporalfinalext );
        AddOperator( &temporalpresentext );
        AddOperator( &temporalatext );
        AddOperator( &temporalpassesext );
        AddOperator( &temporaldeftimeext );
        AddOperator( &temporalinstext );
        AddOperator( &temporalvalext );
        AddOperator( &temporalderivativeext );
        AddOperator( &temporalderivableext );
        AddOperator( &temporalspeedext );
        AddOperator( &temporalvelocityext );

        AddOperator( &rangerangevaluesext );

        AddOperator( &sometimesext );
        AddOperator( &alwaysext );
        AddOperator( &neverext );

    }
    ~TemporalExtAlgebra() {}
};

TemporalExtAlgebra tempExtAlgebra;

/*

5 Initialization

*/

extern "C"
Algebra*
InitializeTemporalExtAlgebra(NestedList *nlRef, QueryProcessor *qpRef)
{
  nl = nlRef;
  qp = qpRef;
  return (&tempExtAlgebra);
}




