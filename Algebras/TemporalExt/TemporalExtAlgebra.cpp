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

/*
4.6 Operators

*/
Operator temporalatinstantext( "atinstantext",
                            TemporalSpecAtInstantExt,
                            5,
                            temporalatinstantextmap,
                            MovingExtSimpleSelect,
                            MovingInstantExtTypeMapIntime );

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


