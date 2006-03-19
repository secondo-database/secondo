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

1 Includes and Initialization

Place for initialization of pointer variables, constants and namespaces and
inclusion of header files concerning Secondo.

*/

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include "TemporalAlgebra.h"

extern NestedList* nl;
extern QueryProcessor *qp;

#include "DateTime.h"
using namespace datetime;

/*

2 Type definitions

*/
typedef ConstTemporalUnit<CcString> UString;
typedef Mapping< UString, CcString > MString;

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
            if( nl->IsEqual( arg1, "mbool" ) )
                return nl->SymbolAtom( "ibool" );

            if( nl->IsEqual( arg1, "mint" ) )
                return nl->SymbolAtom( "iint" );

            if( nl->IsEqual( arg1, "mreal" ) )
                return nl->SymbolAtom( "ireal" );

            if( nl->IsEqual( arg1, "mpoint" ) )
                return nl->SymbolAtom( "ipoint" );

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
            if( nl->IsEqual( arg1, "mbool" ) )
                return nl->SymbolAtom( "mbool" );

            if( nl->IsEqual( arg1, "mint" ) )
                return nl->SymbolAtom( "mint" );

            if( nl->IsEqual( arg1, "mreal" ) )
                return nl->SymbolAtom( "mreal" );

            if( nl->IsEqual( arg1, "mpoint" ) )
                return nl->SymbolAtom( "mpoint" );

            if( nl->IsEqual( arg1, "mstring" ) )
                return nl->SymbolAtom( "mstring" );
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

        if( nl->IsEqual( arg1, "mbool" ) )
            return nl->SymbolAtom( "ibool" );

        if( nl->IsEqual( arg1, "mint" ) )
            return nl->SymbolAtom( "iint" );

        if( nl->IsEqual( arg1, "mreal" ) )
            return nl->SymbolAtom( "ireal" );

        if( nl->IsEqual( arg1, "mpoint" ) )
            return nl->SymbolAtom( "ipoint" );

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
            if( nl->IsEqual( arg1, "mbool" ) ||
                 nl->IsEqual( arg1, "mint" ) ||
                 nl->IsEqual( arg1, "mreal" ) ||
                 nl->IsEqual( arg1, "mpoint" ) ||
                 nl->IsEqual( arg1, "mstring" ))
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

        if( nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "point" ) )
            return nl->SymbolAtom( "mpoint" );

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
            (nl->IsEqual( arg1, "mpoint" ) && nl->IsEqual( arg2, "point" )) )
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

        if( nl->IsEqual( arg1, "mbool" ) ||
                  nl->IsEqual( arg1, "mint" ) ||
                  nl->IsEqual( arg1, "mreal" ) ||
                  nl->IsEqual( arg1, "mpoint" ) ||
                  nl->IsEqual( arg1, "mstring" ))

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

        if( nl->IsEqual( arg1, "ibool" ) ||
                  nl->IsEqual( arg1, "iint" ) ||
                  nl->IsEqual( arg1, "ireal" ) ||
                  nl->IsEqual( arg1, "ipoint" ) ||
                  nl->IsEqual( arg1, "istring" ))

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

        if( nl->IsEqual( arg1, "ibool" ) )
            return nl->SymbolAtom( "bool" );

        if( nl->IsEqual( arg1, "iint" ) )
            return nl->SymbolAtom( "int" );

        if( nl->IsEqual( arg1, "ireal" ) )
            return nl->SymbolAtom( "real" );

        if( nl->IsEqual( arg1, "ipoint" ) )
            return nl->SymbolAtom( "point" );

        if( nl->IsEqual( arg1, "istring" ) )
            return nl->SymbolAtom( "string" );

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

Is used for the ~deftimeext~, ~initialext~, ~finalext~, ~instext~, ~valext~, ~atinstantext~,
~atperiods~  operations.

*/
int
MovingExtSimpleSelect( ListExpr args )
{
    ListExpr arg1 = nl->First( args );

    if( nl->SymbolValue( arg1 ) == "mbool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mreal" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mpoint" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "mstring" )
        return 4;

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

    if( nl->SymbolValue( arg1 ) == "mbool" &&
        nl->SymbolValue( arg2 ) == "instant" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "instant" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "instant" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "instant" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "instant" )
        return 4;

    if( nl->SymbolValue( arg1 ) == "mbool" &&
        nl->SymbolValue( arg2 ) == "periods" )
        return 5;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "periods" )
        return 6;

    if( nl->SymbolValue( arg1 ) == "mreal" &&
        nl->SymbolValue( arg2 ) == "periods" )
        return 7;

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "periods" )
        return 8;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "periods" )
        return 9;

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

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "point" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "string" )
        return 4;

    if( nl->SymbolValue( arg1 ) == "mint" &&
        nl->SymbolValue( arg2 ) == "rint" )
        return 5;

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

    if( nl->SymbolValue( arg1 ) == "mpoint" &&
        nl->SymbolValue( arg2 ) == "point" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "mstring" &&
        nl->SymbolValue( arg2 ) == "string" )
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

    if( nl->SymbolValue( arg1 ) == "ibool" )
        return 0;

    if( nl->SymbolValue( arg1 ) == "iint" )
        return 1;

    if( nl->SymbolValue( arg1 ) == "ireal" )
        return 2;

    if( nl->SymbolValue( arg1 ) == "ipoint" )
        return 3;

    if( nl->SymbolValue( arg1 ) == "istring" )
        return 4;

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
4.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

*/

ValueMapping temporalatinstantextmap[] = {
    MappingAtInstantExt<MBool, CcBool>,
    MappingAtInstantExt<MInt, CcInt>,
    MappingAtInstantExt<MReal, CcReal>,
    MappingAtInstantExt<MPoint, Point>,
    MappingAtInstantExt<MString, CcString> };

ValueMapping temporalatperiodsextmap[] = {
    MappingAtPeriodsExt<MBool>,
    MappingAtPeriodsExt<MInt>,
    MappingAtPeriodsExt<MReal>,
    MappingAtPeriodsExt<MPoint>,
    MappingAtPeriodsExt<MString>};

ValueMapping temporalinitialextmap[] = {
    MappingInitialExt<MBool, UBool, CcBool>,
    MappingInitialExt<MInt, UInt, CcInt>,
    MappingInitialExt<MReal, UReal, CcReal>,
    MappingInitialExt<MPoint, UPoint, Point>,
    MappingInitialExt<MString, UString, CcString> };

ValueMapping temporalfinalextmap[] = {
    MappingFinalExt<MBool, UBool, CcBool>,
    MappingFinalExt<MInt, UInt, CcInt>,
    MappingFinalExt<MReal, UReal, CcReal>,
    MappingFinalExt<MPoint, UPoint, Point>,
    MappingFinalExt<MString, UString, CcString> };

ValueMapping temporalpresentextmap[] = {
    MappingPresentExt_i<MBool>,
    MappingPresentExt_i<MInt>,
    MappingPresentExt_i<MReal>,
    MappingPresentExt_i<MPoint>,
    MappingPresentExt_i<MString>,
    MappingPresentExt_p<MBool>,
    MappingPresentExt_p<MInt>,
    MappingPresentExt_p<MReal>,
    MappingPresentExt_p<MPoint>,
    MappingPresentExt_p<MString>, };

ValueMapping temporalatextmap[] = {
    MappingAtExt<MBool, UBool, CcBool>,
    MappingAtExt<MInt, UInt, CcInt>,
    MappingAtExt<MReal, UReal, CcReal>,
    MappingAtExt<MPoint, UPoint, Point>,
    MappingAtExt<MString, UString, CcString>/*,
    MappingAt_r<MInt, UInt, RInt>*/ };

ValueMapping temporalpassesextmap[] = {
    MappingPassesExt<MBool, CcBool>,
    MappingPassesExt<MInt, CcInt>,
    MappingPassesExt<MReal, CcReal>,
    MappingPassesExt<MPoint, Point>,
    MappingPassesExt<MString, CcString> };

ValueMapping temporaldeftimeextmap[] = {
    MappingDefTimeExt<MBool>,
    MappingDefTimeExt<MInt>,
    MappingDefTimeExt<MReal>,
    MappingDefTimeExt<MPoint>,
    MappingDefTimeExt<MString> };

ValueMapping temporalinstextmap[] = {
    IntimeInstExt<CcBool>,
    IntimeInstExt<CcInt>,
    IntimeInstExt<CcReal>,
    IntimeInstExt<Point>,
    IntimeInstExt<CcString> };


ValueMapping temporalvalextmap[] = {
    IntimeValExt<CcBool>,
    IntimeValExt<CcInt>,
    IntimeValExt<CcReal>,
    IntimeValExt<Point>,
    IntimeValExt<CcString> };

/*
4.5 Specification strings

*/
const string TemporalSpecAtInstantExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(moving(x) instant) -> intime(x)</text--->"
    "<text>_ atinstant _ </text--->"
    "<text>get the Intime value corresponding to the instant.</text--->"
    "<text>mpoint1 atinstant instant1</text--->"
    ") )";

const string TemporalSpecAtPeriodsExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(moving(x) instant) -> intime(x)</text--->"
    "<text>_ atperiods _ </text--->"
    "<text>restrict the movement to the given periods.</text--->"
    "<text>mpoint1 atperiods periods1</text--->"
    ") )";

const string TemporalSpecInitialExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>moving(x) -> intime(x)</text--->"
    "<text> initial( _ )</text--->"
    "<text>get the intime value corresponding to the initial instant.</text--->"
    "<text>initial( mpoint1 )</text--->"
    ") )";

const string TemporalSpecFinalExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>moving(x) -> intime(x)</text--->"
    "<text> final( _ )</text--->"
    "<text>get the intime value corresponding to the final instant.</text--->"
    "<text>final( mpoint1 )</text--->"
    ") )";

const string TemporalSpecPresentExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(moving(x) instant) -> bool, (moving(x) periods) -> bool</text--->"
    "<text>_ present _ </text--->"
    "<text>whether the object is present at the given instant</text--->"
    "<text>or period.</text--->"
    "<text>mpoint1 present instant1</text--->"
    ") )";

const string TemporalSpecAtExt =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(moving(x) x) -> moving(x)</text--->"
    "<text> _ at _ </text--->"
    "<text>restrict the movement at the times where the equality </text--->"
    "<text>occurs.</text--->"
    "<text>mpoint1 at point1</text--->"
    ") )";

const string TemporalSpecPassesExt =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>(moving(x) x) -> bool</text--->"
    "<text>_ passes _ </text--->"
    "<text>whether the object passes the given value.</text--->"
    "<text>mpoint1 passes point1</text--->"
    ") )";

const string TemporalSpecDefTimeExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>moving(x) -> periods</text--->"
    "<text> deftime( _ )</text--->"
    "<text>get the defined time of the corresponding moving </text--->"
    "<text>data objects.</text--->"
    "<text>deftime( mp1 )</text--->"
    ") )";

const string TemporalSpecInstExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>intime(x) -> instant</text--->"
    "<text>inst ( _ )</text--->"
    "<text>Intime time instant.</text--->"
    "<text>inst ( i1 )</text--->"
    ") )";

const string TemporalSpecValExt  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" ) "
    "( <text>intime(x) -> x</text--->"
    "<text>val ( _ )</text--->"
    "<text>Intime value.</text--->"
    "<text>val ( i1 )</text--->"
    ") )";

/*
4.6 Operators

*/
Operator temporalatinstantext( "atinstantext",
                            TemporalSpecAtInstantExt,
                            5,
                            temporalatinstantextmap,
                            MovingExtSimpleSelect,
                            MovingInstantExtTypeMapIntime );

Operator temporalatperiodsext( "atperiodsext",
                            TemporalSpecAtPeriodsExt,
                            5,
                            temporalatperiodsextmap,
                            MovingExtSimpleSelect,
                            MovingPeriodsExtTypeMapMoving );

Operator temporalinitialext( "initialext",
                          TemporalSpecInitialExt,
                          5,
                          temporalinitialextmap,
                          MovingExtSimpleSelect,
                          MovingExtTypeMapIntime );

Operator temporalfinalext( "finalext",
                        TemporalSpecFinalExt,
                        5,
                        temporalfinalextmap,
                        MovingExtSimpleSelect,
                        MovingExtTypeMapIntime );

Operator temporalpresentext( "presentext",
                          TemporalSpecPresentExt,
                          10,
                          temporalpresentextmap,
                          MovingExtInstantPeriodsSelect,
                          MovingInstantPeriodsExtTypeMapBool);

Operator temporalatext( "atext",
                     TemporalSpecAtExt,
                     5,
                     temporalatextmap,
                     MovingExtBaseRangeSelect,
                     MovingBaseExtTypeMapMoving );

Operator temporalpassesext( "passesext",
                         TemporalSpecPassesExt,
                         5,
                         temporalpassesextmap,
                         MovingExtBaseSelect,
                         MovingBaseExtTypeMapBool);

Operator temporaldeftimeext( "deftimeext",
                        TemporalSpecDefTimeExt,
                          5,
                          temporaldeftimeextmap,
                          MovingExtSimpleSelect,
                          MovingExtTypeMapPeriods );

Operator temporalinstext( "instext",
                       TemporalSpecInstExt,
                       5,
                       temporalinstextmap,
                       IntimeExtSimpleSelect,
                       IntimeExtTypeMapInstant );

Operator temporalvalext( "valext",
                      TemporalSpecValExt,
                      5,
                      temporalvalextmap,
                      IntimeExtSimpleSelect,
                      IntimeExtTypeMapBase );

class TemporalExtAlgebra : public Algebra
{
  public:
    TemporalExtAlgebra() : Algebra()
    {
        AddTypeConstructor( &intimestring );

        AddTypeConstructor( &unitstring );

        AddTypeConstructor( &movingstring );

        intimestring.AssociateKind( "TEMPORAL" );
        intimestring.AssociateKind( "DATA" );

        unitstring.AssociateKind( "TEMPORAL" );
        unitstring.AssociateKind( "DATA" );

        movingstring.AssociateKind( "TEMPORAL" );
        movingstring.AssociateKind( "DATA" );

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


