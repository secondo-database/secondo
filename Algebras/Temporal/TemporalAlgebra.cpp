/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Temporal Algebra

January 2004 Victor Almeida

[TOC]

1 Overview

This file contains the implementation of the type constructors ~instant~, 
~range~, ~intime~, ~const~, and ~mapping~. The memory data structures 
used for these type constructors are implemented in the TemporalAlgebra.h
file.

2 Defines, includes, and constants

*/
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebra.h"
#include "StandardTypes.h"
#include "TemporalAlgebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

/*
3 Type Constructor ~instant~

Type ~instant~ represents a point in time or is undefined. Time is considered to
be linear and continuous, i.e., isomorphic to the real numbers.

3.1 List Representation

The list representation of an ~instant~ is

----    ( i )
----

For example:

----    ( 1.0 )
----

3.2 function Describing the Signature of the Type Constructor

*/
ListExpr
InstantProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TIME"),
                             nl->StringAtom("instant"),
                             nl->StringAtom("(<instant_value>)"),
                             nl->StringAtom("12.0 or 14e-3 or .23"))));
}

/*
3.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckInstant( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "instant" ));
}

/*
3.4 ~Out~-function

*/
ListExpr
OutInstant( ListExpr typeinfo, Word value )
{
  if( ((Instant*)value.addr)->IsDefined() )
  {
    return (nl->RealAtom( ((Instant*)value.addr)->GetRealval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*
3.5 ~In~-function

*/
Word
InInstant( ListExpr typeInfo, ListExpr value,
           int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == RealType )
  {
    correct = true;
    return (SetWord( new Instant( true, nl->RealValue( value )) ));
  }
  else if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new Instant( false, 0.0) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

/*
3.6 ~Create~-function

*/
Word
CreateInstant( const ListExpr typeInfo )
{
  return (SetWord( new Instant( false, 0.0 ) ));
}

/*
3.7 ~Delete~-function

*/
void
DeleteInstant( Word& w )
{
  delete (Instant*)w.addr;
  w.addr = 0;
}

/*
3.8 ~Close~-function

*/
void
CloseInstant( Word& w )
{
  delete (Instant*)w.addr;
  w.addr = 0;
}

/*
3.9 ~Clone~-function

*/
Word
CloneInstant( const Word& w )
{
  return SetWord( ((Instant*)w.addr)->Clone() );
}

/*
3.10 ~Sizeof~-function

*/
int
SizeOfInstant()
{
  return sizeof(Instant);
}

/*
3.11 ~Cast~-function

*/
void*
CastInstant( void* addr )
{
  CcReal::realsCreated--;
  return new (addr) Instant;
}

/*
3.12 Creation of the type constructor ~instant~

*/
TypeConstructor instant( "instant",	InstantProperty,
                         OutInstant,    InInstant,
                         0,             0,
                         CreateInstant, DeleteInstant,
                         0,             0,
                         CloseInstant,  CloneInstant,
                         CastInstant,   SizeOfInstant, 
                         CheckInstant );

/*
4 Type Constructor ~rangeint~

This type constructor implements the carrier set for ~range(int)~.

4.1 List Representation

The list representation of a ~range(int)~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( (1 5 TRUE FALSE) (6 9 FALSE FALSE) (11 11 TRUE TRUE) )
----

4.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeIntProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"lci means left closed interval, rci respectively right closed interval,"
                             " e.g. (0 1 TRUE FALSE) means the range [0, 1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("int -> RANGE"),
                             nl->StringAtom("(rangeint) "),
                             nl->StringAtom("( (b1 e1 lci rci) ... "
                             "(bn en lci rci) )"),
                             nl->StringAtom("( (0 1 TRUE FALSE)"
                             "(2 5 TRUE TRUE) )"),
                             remarkslist)));
}

/*
4.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. It
checks if the argument $\alpha$ of the range belongs to the ~BASE~ kind.

*/
bool
CheckRangeInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "rangeint" ));
}

/*
4.4 Creation of the type constructor ~rangeint~

*/
TypeConstructor rangeint(
        "rangeint",                                     //name
        RangeIntProperty,                               //property function describing signature
        OutRange<CcInt, OutCcInt>,
        InRange<CcInt, InCcInt>,                        //Out and In functions
        0,                      0,                      //SaveToList and RestoreFromList functions
        CreateRange<CcInt>,     DeleteRange<CcInt>,     //object creation and deletion
        OpenRange<CcInt>,       SaveRange<CcInt>,       // object open and save
        CloseRange<CcInt>,      CloneRange<CcInt>,      //object close and clone
        CastRange<CcInt>,                               //cast function
        SizeOfRange<CcInt>,                             //sizeof function
        CheckRangeInt,                                  //kind checking function
        0,                                              //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );


/*
5 Type Constructor ~rangereal~

This type constructor implements the carrier set for ~range(real)~.

5.1 List Representation

The list representation of a ~rangereal~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( (1.01 5 TRUE FALSE) (6.37 9.9 FALSE FALSE) (11.93 11.99 TRUE TRUE) )
----

5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
RangeRealProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"lci means left closed interval, rci respectively right closed interval,"
                             " e.g. (0.5 1.1 TRUE FALSE) means the range [0.5, 1.1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("real -> RANGE"),
                             nl->StringAtom("(rangereal) "),
                             nl->StringAtom("( (b1 e1 lci rci) ... "
                             "(bn en lci rci) )"),
                             nl->StringAtom("( (0.5 1.1 TRUE FALSE)"
                             "(2 5.04 TRUE TRUE) )"),
                             remarkslist)));
}

/*
5.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. It
checks if the argument $\alpha$ of the range belongs to the ~BASE~ kind.

*/
bool
CheckRangeReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "rangereal" ));
}

/*
5.4 Creation of the type constructor ~rangereal~

*/
TypeConstructor rangereal(
        "rangereal",                        		//name
        RangeRealProperty,                  		//property function describing signature
        OutRange<CcReal, OutCcReal>,
	InRange<CcReal, InCcReal>, 		     	//Out and In functions
        0,              	0,              	//SaveToList and RestoreFromList functions
        CreateRange<CcReal>,	DeleteRange<CcReal>,   	//object creation and deletion
        OpenRange<CcReal>,	SaveRange<CcReal>,	// object open and save
        CloseRange<CcReal>,	CloneRange<CcReal>,	//object close and clone
        CastRange<CcReal>,				//cast function
        SizeOfRange<CcReal>,                   		//sizeof function
        CheckRangeReal,                     		//kind checking function
        0,                              		//predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
6 Type Constructor ~periods~

This type constructor implements the carrier set for ~range(instant)~, which is
called ~periods~.

5.1 List Representation

The list representation of a ~periods~ is

----    ( (i1b i1e lc1 rc1) (i2b i2e lc2 rc2) ... (inb ine lcn rcn) )
----

For example:

----    ( ( (instant 1.01)  (instant 5)     TRUE  FALSE) 
          ( (instant 6.37)  (instant 9.9)   FALSE FALSE) 
          ( (instant 11.93) (instant 11.99) TRUE  TRUE) )
----

5.2 function Describing the Signature of the Type Constructor

*/
ListExpr
PeriodsProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"lci means left closed interval, rci respectively right closed interval,"
                             " e.g. ((instant 0.5) (instant 1.1) TRUE FALSE) means the interval [0.5, 1.1[");

  return (nl->TwoElemList(
            nl->FiveElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List"),
                             nl->StringAtom("Remarks")),
            nl->FiveElemList(nl->StringAtom("instant -> RANGE"),
                             nl->StringAtom("(periods) "),
                             nl->StringAtom("( (b1 e1 lci rci) ... "
                             "(bn en lci rci) )"),
                             nl->StringAtom("( ((instant 0.5) (instant 1.1) TRUE FALSE)"
                             "((instant 2) (instant 5.04) TRUE TRUE) )"),
                             remarkslist)));
}

/*
5.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. It
checks if the argument $\alpha$ of the range belongs to the ~BASE~ kind.

*/
bool
CheckPeriods( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "periods" ));
}

/*
5.4 Creation of the type constructor ~periods~

*/
TypeConstructor periods(
        "periods",                                      //name
        PeriodsProperty,                                //property function describing signature
        OutRange<Instant, OutInstant>,
        InRange<Instant, InInstant>,                    //Out and In functions
        0,                      0,                      //SaveToList and RestoreFromList functions
        CreateRange<Instant>,   DeleteRange<Instant>,   //object creation and deletion
        OpenRange<Instant>,      SaveRange<Instant>,    // object open and save
        CloseRange<Instant>,     CloneRange<Instant>,   //object close and clone
        CastRange<Instant>,                             //cast function
        SizeOfRange<Instant>,                           //sizeof function
        CheckPeriods,                                   //kind checking function
        0,                                              //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );

/*
6 Type Constructor ~intimeint~

Type ~intimeint~ represents an (instant, value)-pair of integers.

6.1 List Representation

The list representation of an ~intimeint~ is

----    ( t int-value )
----

For example:

----    ( 1.0 5 )
----

6.2 function Describing the Signature of the Type Constructor

*/
ListExpr
IntimeIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("int -> TEMPORAL"),
                             nl->StringAtom("(intimeint) "),
                             nl->StringAtom("( (inst val) "),
                             nl->StringAtom("( ((instant 0.5) 1 )"))));
}

/*
6.3 Kind Checking Function

This function checks whether the type constructor is applied correctly. 

*/
bool
CheckIntimeInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "intimeint" ));
}

/*
5.4 Creation of the type constructor ~intimeint~

*/
TypeConstructor intimeint(
        "intimeint",                                  //name
        IntimeIntProperty,                            //property function describing signature
        OutIntime<CcInt, OutCcInt>,
        InIntime<CcInt, InCcInt>,                     //Out and In functions
        0,                      0,                    //SaveToList and RestoreFromList functions
        CreateIntime<CcInt>,    DeleteIntime<CcInt>,  //object creation and deletion
        0,                      0,                    // object open and save
        CloseIntime<CcInt>,     CloneIntime<CcInt>,   //object close and clone
        CastIntime<CcInt>,                            //cast function
        SizeOfIntime<CcInt>,                          //sizeof function
        CheckIntimeInt,                               //kind checking function
        0,                                            //predef. pers. function for model
        TypeConstructor::DummyInModel,
        TypeConstructor::DummyOutModel,
        TypeConstructor::DummyValueToModel,
        TypeConstructor::DummyValueListToModel );


/*
6 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

6.1 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

6.1.1 Typa mapping function InstantTypeMapBool

*/
ListExpr
InstantTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "instant" )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.1 Typa mapping function InstantInstantTypeMapBool

*/
ListExpr
InstantInstantTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "instant" &&
        nl->IsAtom( arg2 ) &&
        nl->AtomType( arg2 ) == SymbolType &&
        nl->SymbolValue( arg2 ) == "instant" )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.1 Type mapping function RangeTypeMapBool1

It is for the operator ~isempty~ which have a ~range~ as input and ~bool~ result type.

*/
bool IsRangeAtom( const ListExpr atom )
{
  if( nl->IsAtom( atom ) &&
      nl->AtomType( atom ) == SymbolType &&
      ( nl->SymbolValue( atom ) == "rangeint" ||
        nl->SymbolValue( atom ) == "rangereal" ||
        nl->SymbolValue( atom ) == "periods" ) )
    return true;
  return false;
}

bool IsOfRangeType( const ListExpr type, const ListExpr range )
{
  assert( IsRangeAtom( range ) );
  if( nl->IsAtom( type ) &&
      nl->AtomType( type ) == SymbolType &&
      ( nl->SymbolValue( range ) == string("range") + nl->SymbolValue( type ) ||
        nl->SymbolValue( range ) == string("periods") && nl->SymbolValue( type ) == string("instant") ) )
    return true;
  return false;
}

ListExpr
RangeTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( IsRangeAtom( arg1 ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.2 Type mapping function RangeRangeTypeMapBool

It is for the operators $=$, $\neq$, and ~intersects~ which have two
~ranges~ as input and ~bool~ result type.

*/
ListExpr
RangeRangeTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if( IsRangeAtom( arg1 ) &&
        IsRangeAtom( arg2 ) &&
        nl->IsEqual( arg1, nl->SymbolValue( arg2 ) ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.3 Type mapping function RangeBaseTypeMapBool1

It is for the operator ~inside~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in this order as arguments and ~bool~ as the result type.

*/
ListExpr
RangeBaseTypeMapBool1( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );
    if( IsRangeAtom( arg1 ) &&
        IsRangeAtom( arg2 ) &&
        nl->IsEqual( arg1, nl->SymbolValue( arg2 ) ) )
      return (nl->SymbolAtom( "bool" ));
    else if( IsRangeAtom( arg2 ) &&
             IsOfRangeType( arg1, arg2 ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.4 Type mapping function RangeBaseTypeMapBool2

It is for the operator ~before~ which have two ~ranges~ as input or a
~BASE~ and a ~range~ in any order as arguments and ~bool~ as the result type.

*/
ListExpr
RangeBaseTypeMapBool2( ListExpr args )
{
  if ( nl->ListLength( args ) == 2 )
  {
    ListExpr arg1 = nl->First( args ),
             arg2 = nl->Second( args );
    if( IsRangeAtom( arg1 ) &&
        IsRangeAtom( arg2 ) &&
        nl->IsEqual( arg1, nl->SymbolValue( arg2 ) ) )
      return (nl->SymbolAtom( "bool" ));
    else if( IsRangeAtom( arg2 ) &&
             IsOfRangeType( arg1, arg2 ) )
      return (nl->SymbolAtom( "bool" ));
    else if( IsRangeAtom( arg1 ) &&
             IsOfRangeType( arg2, arg1 ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.5 Type mapping function RangeRangeTypeMapRange

It is for the operators ~intersection~, ~union~, and ~minus~ which have two
~ranges~ as input and a ~range~ as result type.

*/
ListExpr
RangeRangeTypeMapRange( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if( IsRangeAtom( arg1 ) &&
        IsRangeAtom( arg2 ) &&
        nl->SymbolValue( arg1 ) == nl->SymbolValue( arg2 ) )
      return (nl->SymbolAtom( nl->SymbolValue( arg1 ) ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.6 Type mapping function RangeTypeMapBase

It is for the aggregate operators ~min~, ~max~, and ~avg~ which have one
~range~ as input and a ~BASE~ as result type.

*/
ListExpr
RangeTypeMapBase( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( IsRangeAtom( arg1 ) )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.1.7 Type mapping function RangeTypeMapInt

It is for the ~no\_components~ operator which have one
~range~ as input and a ~int~ as result type.

*/
ListExpr
RangeTypeMapInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( IsRangeAtom( arg1 ) )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
6.2 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

6.2.1 Selection function ~RangeSelectPredicates~

Is used for the ~inside~ and ~before~ operations.

*/
int
RangeSelectPredicates( ListExpr args )
{
  ListExpr arg1 = nl->First( args ),
           arg2 = nl->Second( args );

  if( IsRangeAtom( arg1 ) &&
      IsRangeAtom( arg2 ) &&
      nl->SymbolValue( arg1 ) == nl->SymbolValue( arg2 ) )
    return (0);

  if( IsRangeAtom( arg2 ) &&
      IsOfRangeType( arg1, arg2 ) )
    return (1);

  if( IsRangeAtom( arg1 ) &&
      IsOfRangeType( arg2, arg1 ) )
    return (2);

  assert( false );
  return (-1); // This point should never be reached
}

/*
6.3 Value mapping functions

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators there are
several value mapping functions, one for each possible combination of input
parameter types.

6.3.1 Value mapping functions of operator ~isempty~

*/
int InstantIsEmpty( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Instant*)args[0].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

template <class Alpha>
int RangeIsEmpty_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->IsEmpty() )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
6.3.2 Value mapping functions of operator $=$ (~equal~)

*/
int
InstantEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->GetRealval() ==
                 ((Instant*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

template <class Alpha>
int RangeEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range<Alpha>*)args[0].addr) == *((Range<Alpha>*)args[1].addr) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }

  return (0);
}

/*
6.3.3 Value mapping functions of operator $\#$ (~not equal~)

*/
int
InstantNotEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->GetRealval() !=
                 ((Instant*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

template <class Alpha>
int RangeNotEqual_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( *((Range<Alpha>*)args[0].addr) != *((Range<Alpha>*)args[1].addr) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
6.3.3 Value mapping functions of operator $<$

*/
int
InstantLess( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->GetRealval() <
                 ((Instant*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
6.3.3 Value mapping functions of operator $<=$

*/
int
InstantLessEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->GetRealval() <=
                 ((Instant*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
6.3.3 Value mapping functions of operator $>$

*/
int
InstantGreater( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->GetRealval() >
                 ((Instant*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
6.3.3 Value mapping functions of operator $>=$

*/
int
InstantGreaterEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( ((Instant*)args[0].addr)->IsDefined() &&
       ((Instant*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((Instant*)args[0].addr)->GetRealval() >=
                 ((Instant*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
6.3.4 Value mapping functions of operator ~intersects~

*/
template <class Alpha>
int RangeIntersects_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->Intersects( *((Range<Alpha>*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
6.3.5 Value mapping functions of operator ~inside~

*/
template <class Alpha>
int RangeInside_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->Inside( *((Range<Alpha>*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

template <class Alpha>
int RangeInside_ar( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[1].addr)->Contains( *((Alpha*)args[0].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
6.3.6 Value mapping functions of operator ~before~

*/
template <class Alpha>
int RangeBefore_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->Before( *((Range<Alpha>*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

template <class Alpha>
int RangeBefore_ar( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[1].addr)->After( *((Alpha*)args[0].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

template <class Alpha>
int RangeBefore_ra( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((Range<Alpha>*)args[0].addr)->Before( *((Alpha*)args[1].addr) ) )
  {
    ((CcBool*)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, false );
  }
  return (0);
}

/*
6.3.7 Value mapping functions of operator ~intersection~

*/
template <class Alpha>
int RangeIntersection_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range<Alpha>*)args[0].addr)->Intersection( *((Range<Alpha>*)args[1].addr), (*(Range<Alpha>*)result.addr) );
  return (0);
}

/*
6.3.8 Value mapping functions of operator ~union~

*/
template <class Alpha>
int RangeUnion_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range<Alpha>*)args[0].addr)->Union( *((Range<Alpha>*)args[1].addr), (*(Range<Alpha>*)result.addr) );
  return (0);
}

/*
6.3.9 Value mapping functions of operator ~minus~

*/
template <class Alpha>
int RangeMinus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((Range<Alpha>*)args[0].addr)->Minus( *((Range<Alpha>*)args[1].addr), (*(Range<Alpha>*)result.addr) );
  return (0);
}

/*
6.3.10 Value mapping functions of operator ~min~

*/
template <class Alpha>
int RangeMinimum_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range<Alpha>*)args[0].addr)->IsEmpty() )
  {
    ((Alpha *)result.addr)->SetDefined( false );
  }
  else
  {
    ((Range<Alpha>*)args[0].addr)->Minimum( *(Alpha *)result.addr);
  }
  return (0);
}

/*
6.3.11 Value mapping functions of operator ~max~

*/
template <class Alpha>
int RangeMaximum_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if( ((Range<Alpha>*)args[0].addr)->IsEmpty() )
  {
    ((Alpha*)result.addr)->SetDefined( false );
  }
  else
  {
    ((Range<Alpha>*)args[0].addr)->Maximum( *(Alpha*)result.addr);
  }
  return (0);
}

/*
6.3.12 Value mapping functions of operator ~no\_components~

*/
template <class Alpha>
int RangeNoComponents_r( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)->Set( true, ((Range<Alpha>*)args[0].addr)->GetNoComponents() );
  return (0);
}

/*
6.4 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also such and array
defined, so it easier to make them overloaded.

6.4.1 ValueMapping arrays

*/
ValueMapping instantisemptymap[] = { InstantIsEmpty };
ValueMapping instantequalmap[] = { InstantEqual };
ValueMapping instantnotequalmap[] = { InstantNotEqual };
ValueMapping instantlessmap[] = { InstantLess };
ValueMapping instantlessequalmap[] = { InstantLessEqual };
ValueMapping instantgreatermap[] = { InstantGreater };
ValueMapping instantgreaterequalmap[] = { InstantGreaterEqual };

ValueMapping rangeintisemptymap[] = { RangeIsEmpty_r<CcInt> };
ValueMapping rangeintequalmap[] = { RangeEqual_rr<CcInt> };
ValueMapping rangeintnotequalmap[] = { RangeNotEqual_rr<CcInt> };
ValueMapping rangeintintersectsmap[] = { RangeIntersects_rr<CcInt> };
ValueMapping rangeintinsidemap[] = { RangeInside_rr<CcInt>, RangeInside_ar<CcInt> };
ValueMapping rangeintbeforemap[] = { RangeBefore_rr<CcInt>, RangeBefore_ar<CcInt>, RangeBefore_ra<CcInt> };
ValueMapping rangeintintersectionmap[] = { RangeIntersection_rr<CcInt> };
ValueMapping rangeintunionmap[] = { RangeUnion_rr<CcInt> };
ValueMapping rangeintminusmap[] = { RangeMinus_rr<CcInt> };
ValueMapping rangeintminmap[] = { RangeMinimum_r<CcInt> };
ValueMapping rangeintmaxmap[] = { RangeMaximum_r<CcInt> };
ValueMapping rangeintnocomponentsmap[] = { RangeNoComponents_r<CcInt> };

ValueMapping rangerealisemptymap[] = { RangeIsEmpty_r<CcReal> };
ValueMapping rangerealequalmap[] = { RangeEqual_rr<CcReal> };
ValueMapping rangerealnotequalmap[] = { RangeNotEqual_rr<CcReal> };
ValueMapping rangerealintersectsmap[] = { RangeIntersects_rr<CcReal> };
ValueMapping rangerealinsidemap[] = { RangeInside_rr<CcReal>, RangeInside_ar<CcReal> };
ValueMapping rangerealbeforemap[] = { RangeBefore_rr<CcReal>, RangeBefore_ar<CcReal>, RangeBefore_ra<CcReal> };
ValueMapping rangerealintersectionmap[] = { RangeIntersection_rr<CcReal> };
ValueMapping rangerealunionmap[] = { RangeUnion_rr<CcReal> };
ValueMapping rangerealminusmap[] = { RangeMinus_rr<CcReal> };
ValueMapping rangerealminmap[] = { RangeMinimum_r<CcReal> };
ValueMapping rangerealmaxmap[] = { RangeMaximum_r<CcReal> };
ValueMapping rangerealnocomponentsmap[] = { RangeNoComponents_r<CcReal> };

Word TemporalNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

ModelMapping instantnomodelmap[] = { TemporalNoModelMapping };
ModelMapping rangenomodelmap[] = { TemporalNoModelMapping, TemporalNoModelMapping,
                                   TemporalNoModelMapping, TemporalNoModelMapping,
                                   TemporalNoModelMapping, TemporalNoModelMapping };

/*
6.4.2 Specification strings

*/
const string TemporalSpecIsEmpty  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                    "\"Example\" ) "
                                    "( <text>instant -> bool, range -> bool</text--->"
                                    "<text>isempty ( _ )</text--->"
                                    "<text>Returns whether the instant is empty or "
                                    "not.</text--->"
                                    "<text>query isempty ( instant )</text--->"
                                    ") )";

const string TemporalSpecEQ  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool, (range range) -> bool</text--->"
                               "<text>_ = _</text--->"
                               "<text>Equal.</text--->"
                               "<text>query i1 = i2</text--->"
                               ") )";

const string TemporalSpecNE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool, (range range) -> bool</text--->"
                               "<text>_ # _</text--->"
                               "<text>Not equal.</text--->"
                               "<text>query i1 # i2</text--->"
                               ") )";

const string TemporalSpecLT  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool</text--->"
                               "<text>_ < _</text--->"
                               "<text>Less than.</text--->"
                               "<text>query i1 < i2</text--->"
                               ") )";

const string TemporalSpecLE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool</text--->"
                               "<text>_ <= _</text--->"
                               "<text>Less or equal than.</text--->"
                               "<text>query i1 <= i2</text--->"
                               ") )";

const string TemporalSpecGT  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool</text--->"
                               "<text>_ > _</text--->"
                               "<text>Greater than.</text--->"
                               "<text>query i1 > i2</text--->"
                               ") )";

const string TemporalSpecGE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                               "\"Example\" )"
                               "( <text>(instant instant) -> bool</text--->"
                               "<text>_ >= _</text--->"
                               "<text>Greater or equal than.</text--->"
                               "<text>query i1 >= i2</text--->"
                               ") )";

const string TemporalSpecIntersects  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                       "\"Example\" ) "
                                       "( <text>( (range x) (range x) ) -> bool</text--->"
                                       "<text>_ intersects _</text--->"
                                       "<text>Intersects.</text--->"
                                       "<text>query range1 intersects range2</text--->"
                                       ") )";

const string TemporalSpecInside  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                   "\"Example\" ) "
                                   "( <text>( (range x) (range x) ) -> bool,"
                                   "( x (range x) ) -> bool</text--->"
                                   "<text>_ inside _</text--->"
                                   "<text>Inside.</text--->"
                                   "<text>query 5 inside rangeint1</text--->"
                                   ") )";

const string TemporalSpecBefore  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                   "\"Example\" ) "
                                   "( <text>( (range x) (range x) ) -> bool, "
                                   "( x (range x) ) -> bool, ( (range x) x ) -> "
                                   "bool</text--->"
                                   "<text>_ before _</text--->"
                                   "<text>Before.</text--->"
                                   "<text>query 5 before rangeint1</text--->"
                                   ") )";

const string TemporalSpecIntersection  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                         "\"Example\" ) "
                                         "( <text>( (range x) (range x) ) -> (range x)</text--->"
                                         "<text>_ intersection _</text--->"
                                         "<text>Intersection.</text--->"
                                         "<text>query range1 intersection range2</text--->"
                                         ") )";

const string TemporalSpecUnion  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                  "\"Example\" ) "
                                  "( <text>( (range x) (range x) ) -> (range x)</text--->"
                                  "<text>_ union _</text--->"
                                  "<text>Union.</text--->"
                                  "<text>query range1 union range2</text--->"
                                  ") )";

const string TemporalSpecMinus  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                  "\"Example\" ) "
                                  "( <text>( (range x) (range x) ) -> (range x)</text--->"
                                  "<text>_ minus _</text--->"
                                  "<text>Minus.</text--->"
                                  "<text>query range1 minus range2</text--->"
                                  ") )";

const string TemporalSpecMinimum  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                    "\"Example\" ) "
                                    "( <text>(range x) -> x</text--->"
                                    "<text>minimum ( _ )</text--->"
                                    "<text>Minimum.</text--->"
                                    "<text>minimum ( range1 )</text--->"
                                    ") )";

const string TemporalSpecMaximum  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                    "\"Example\" ) "
                                    "( <text>(range x) -> x</text--->"
                                    "<text>maximum ( _ )</text--->"
                                    "<text>Maximum.</text--->"
                                    "<text>maximum ( range1 )</text--->"
                                    ") )";

const string TemporalSpecNoComponents  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                                         "\"Example\" ) "
                                         "( <text>(range x) -> int</text--->"
                                         "<text>no_components ( _ )</text--->"
                                         "<text>Number of components.</text--->"
                                         "<text>no_components ( range1 )</text--->"
                                         ") )";

/*
6.4.3 Operators

*/
Operator instantisempty( "isempty",
                          TemporalSpecIsEmpty,
                          1,
                          instantisemptymap,
                          instantnomodelmap,
                          Operator::SimpleSelect,
                          InstantTypeMapBool );

Operator instantequal( "=",
                       TemporalSpecEQ,
                       1,
                       instantequalmap,
                       instantnomodelmap,
                       Operator::SimpleSelect,
                       InstantInstantTypeMapBool );

Operator instantnotequal( "#",
                          TemporalSpecNE,
                          1,
                          instantnotequalmap,
                          instantnomodelmap,
                          Operator::SimpleSelect,
                          InstantInstantTypeMapBool );

Operator instantless( "<",
                      TemporalSpecLT,
                      1,
                      instantlessmap,
                      instantnomodelmap,
                      Operator::SimpleSelect,
                      InstantInstantTypeMapBool );

Operator instantlessequal( "<=",
                           TemporalSpecLE,
                           1,
                           instantlessequalmap,
                           instantnomodelmap,
                           Operator::SimpleSelect,
                           InstantInstantTypeMapBool );

Operator instantgreater( ">",
                         TemporalSpecLT,
                         1,
                         instantgreatermap,
                         instantnomodelmap,
                         Operator::SimpleSelect,
                         InstantInstantTypeMapBool );

Operator instantgreaterqual( ">=",
                             TemporalSpecLE,
                             1,
                             instantgreaterequalmap,
                             instantnomodelmap,
                             Operator::SimpleSelect,
                             InstantInstantTypeMapBool );

Operator rangeintisempty( "isempty",
                          TemporalSpecIsEmpty,
                          1,
                          rangeintisemptymap,
                          rangenomodelmap,
                          Operator::SimpleSelect,
                          RangeTypeMapBool1 );

Operator rangeintequal( "=",
                        TemporalSpecEQ,
                        1,
                        rangeintequalmap,
                        rangenomodelmap,
                        Operator::SimpleSelect,
                        RangeRangeTypeMapBool );

Operator rangeintnotequal( "#",
                           TemporalSpecNE,
                           1,
                           rangeintnotequalmap,
                           rangenomodelmap,
                           Operator::SimpleSelect,
                           RangeRangeTypeMapBool );

Operator rangeintintersects( "intersects",
                             TemporalSpecIntersects,
                             1,
                             rangeintintersectsmap,
                             rangenomodelmap,
                             Operator::SimpleSelect,
                             RangeRangeTypeMapBool );

Operator rangeintinside( "inside",
                         TemporalSpecInside,
                         2,
                         rangeintinsidemap,
                         rangenomodelmap,
                         RangeSelectPredicates,
                         RangeBaseTypeMapBool1 );

Operator rangeintbefore( "before",
                         TemporalSpecBefore,
                         3,
                         rangeintbeforemap,
                         rangenomodelmap,
                         RangeSelectPredicates,
                         RangeBaseTypeMapBool2 );

Operator rangeintintersection( "intersection",
                               TemporalSpecIntersection,
                               1,
                               rangeintintersectionmap,
                               rangenomodelmap,
                               Operator::SimpleSelect,
                               RangeRangeTypeMapRange );

Operator rangeintunion( "union",
                        TemporalSpecUnion,
                        1,
                        rangeintunionmap,
                        rangenomodelmap,
                        Operator::SimpleSelect,
                        RangeRangeTypeMapRange );

Operator rangeintminus( "minus",
                        TemporalSpecMinus,
                        1,
                        rangeintminusmap,
                        rangenomodelmap,
                        Operator::SimpleSelect,
                        RangeRangeTypeMapRange );

Operator rangeintmin( "minimum",
                      TemporalSpecMinimum,
                      1,
                      rangeintminmap,
                      rangenomodelmap,
                      Operator::SimpleSelect,
                      RangeTypeMapBase );

Operator rangeintmax( "maximum",
                      TemporalSpecMaximum,
                      1,
                      rangeintmaxmap,
                      rangenomodelmap,
                      Operator::SimpleSelect,
                      RangeTypeMapBase );
 
Operator rangeintnocomponents( "no_components",
                               TemporalSpecNoComponents,
                               1,
                               rangeintnocomponentsmap,
                               rangenomodelmap,
                               Operator::SimpleSelect,
                               RangeTypeMapInt );

Operator rangerealisempty( "isempty",
                           TemporalSpecIsEmpty,
                           1,
                           rangerealisemptymap,
                           rangenomodelmap,
                           Operator::SimpleSelect,
                           RangeTypeMapBool1 );

Operator rangerealequal( "=",
                         TemporalSpecEQ,
                         1,
                         rangerealequalmap,
                         rangenomodelmap,
                         Operator::SimpleSelect,
                         RangeRangeTypeMapBool );

Operator rangerealnotequal( "#",
                            TemporalSpecNE,
                            1,
                            rangerealnotequalmap,
                            rangenomodelmap,
                            Operator::SimpleSelect,
                            RangeRangeTypeMapBool );

Operator rangerealintersects( "intersects",
                              TemporalSpecIntersects,
                              1,
                              rangerealintersectsmap,
                              rangenomodelmap,
                              Operator::SimpleSelect,
                              RangeRangeTypeMapBool );

Operator rangerealinside( "inside",
                          TemporalSpecInside,
                          2,
                          rangerealinsidemap,
                          rangenomodelmap,
                          RangeSelectPredicates,
                          RangeBaseTypeMapBool1 );

Operator rangerealbefore( "before",
                          TemporalSpecBefore,
                          3,
                          rangerealbeforemap,
                          rangenomodelmap,
                          RangeSelectPredicates,
                          RangeBaseTypeMapBool2 );
 
Operator rangerealintersection( "intersection",
                                TemporalSpecIntersection,
                                1,
                                rangerealintersectionmap,
                                rangenomodelmap,
                                Operator::SimpleSelect,
                                RangeRangeTypeMapRange );

Operator rangerealunion( "union",
                         TemporalSpecUnion,
                         1,
                         rangerealunionmap,
                         rangenomodelmap,
                         Operator::SimpleSelect,
                         RangeRangeTypeMapRange );

Operator rangerealminus( "minus",
                         TemporalSpecMinus,
                         1,
                         rangerealminusmap,
                         rangenomodelmap,
                         Operator::SimpleSelect,
                         RangeRangeTypeMapRange );

Operator rangerealmin( "minimum",
                       TemporalSpecMinimum,
                       1,
                       rangerealminmap,
                       rangenomodelmap,
                       Operator::SimpleSelect,
                       RangeTypeMapBase );

Operator rangerealmax( "maximum",
                       TemporalSpecMaximum,
                       1,
                       rangerealmaxmap,
                       rangenomodelmap,
                       Operator::SimpleSelect,
                       RangeTypeMapBase );
  
Operator rangerealnocomponents( "no_components",
                                TemporalSpecNoComponents,
                                1,
                                rangerealnocomponentsmap,
                                rangenomodelmap,
                                Operator::SimpleSelect,
                                RangeTypeMapInt );

/*
6 Creating the Algebra

*/

class TemporalAlgebra : public Algebra
{
 public:
  TemporalAlgebra() : Algebra()
  {
    AddTypeConstructor( &instant );
    AddTypeConstructor( &rangeint );
    AddTypeConstructor( &rangereal );
    AddTypeConstructor( &intimeint );

    instant.AssociateKind( "TIME" );
    rangeint.AssociateKind( "RANGE" );
    rangereal.AssociateKind( "RANGE" );
    intimeint.AssociateKind( "TEMPORAL" );

    instant.AssociateKind( "DATA" );
    rangeint.AssociateKind( "DATA" );
    rangereal.AssociateKind( "DATA" );

    AddOperator( &instantisempty );
    AddOperator( &instantequal );
    AddOperator( &instantnotequal );
    AddOperator( &instantless );
    AddOperator( &instantlessequal );
    AddOperator( &instantgreater );
    AddOperator( &instantgreaterqual );

    AddOperator( &rangeintisempty );
    AddOperator( &rangeintequal );
    AddOperator( &rangeintnotequal );
    AddOperator( &rangeintintersects );
    AddOperator( &rangeintinside );
    AddOperator( &rangeintbefore );
    AddOperator( &rangeintintersection );
    AddOperator( &rangeintunion );
    AddOperator( &rangeintminus );
    AddOperator( &rangeintmin );
    AddOperator( &rangeintmax );
    AddOperator( &rangeintnocomponents );

    AddOperator( &rangerealisempty );
    AddOperator( &rangerealequal );
    AddOperator( &rangerealnotequal );
    AddOperator( &rangerealintersects );
    AddOperator( &rangerealinside );
    AddOperator( &rangerealbefore );
    AddOperator( &rangerealintersection );
    AddOperator( &rangerealunion );
    AddOperator( &rangerealminus );
    AddOperator( &rangerealmin );
    AddOperator( &rangerealmax );
    AddOperator( &rangerealnocomponents );

  }
  ~TemporalAlgebra() {};
};

TemporalAlgebra temporalAlgebra;

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializeTemporalAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&temporalAlgebra);
}


