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

//paragraph [1] Title:  [{\Large \bf ]  [}]
//[->] [\ensuremath{\rightarrow}]

[1] Secondo Standardalgebra

Nov 1998. Friedhelm Becker.

August 16, 2000. RHG Changed includes to show dependencies more clearly.

March 2002. Ulrich Telle Port to C++

November 9, 2002. RHG Added operators ~randint~ and ~log~. Some other slight revisions.

February 2004. F. Hoffmann added operators ~relcount~ and ~relcount2~.

April 28, 2004. M. Spiekermann added operators ~nextint~ and ~randmax~. The
calculation of random numbers in an specified range was revised according to the
recommendations documented in the rand() manpage.

July 08, 2004. M. Spiekermann changed the IN-function of data type ~real~.
Integer atoms are now also accepted.

November 2004. M. Spiekermann. Some small functions were moved to the header
file in order to implement them as inline functions.

November 2004. M. Spiekermann. Implementation of the operator ~substr~. Moreover, the
add operator was overloaded once more in order to concatenate strings.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006, M. Spiekermann new operator ~elapsedtime~ implemented.

May 2006, M. Spiekermann new operator ~setoption~ implemented.

May 11, 2006, M. Spiekermann. Most of the value mappings are replaced by template
functions using the generic ~Compare~ function. This reduces the code a lot
(about 400 lines of code) and there are still some functions left which may
be replaced by template implementations.

December 06, 2006 C. Duentgen added operators int2real, real2int, int2bool, bool2int,
ceil, floor.7

\begin{center}
\footnotesize
\tableofcontents
\end{center}


1 Overview

In  this algebra the standard types and functions are defined. Standard types are
~int~, ~real~, ~bool~ and ~string~. These types  are represented
In Classes of C++ whith a boolean flag which shows whether this value is defined
or not (e.g it could be undefined because it is the result of a divivion by zero).


Following operators are defined:

  * + (add)

----    int x int --> int
        int x real --> real
        real x int --> real
        real x real --> real
        string x string --> string
----

  * - (subtract)

----    int x int --> int
        int x real --> real
        real x int --> real
        real x real --> real
----

  * [*] (multiply)

----    int x int --> int
        int x real --> real
        real x int --> real
        real x real --> real
----

  * / (divide)

----    int x int --> real
        int x real --> real
        real x int --> real
        real x real --> real
----

  * mod (remainder)

----    int x int --> int
----

  * div (integer division)

----    int x int --> int
----

  * randint
  * randmax

----    int -> int
            -> int
----

Computes a random integer within the range [0, arg-1]. The argument must be
greater than 0. Otherwise it is set to 2.

  * seqinit
  * seqnext

----    int -> bool
            -> int
----
The seqinit operator can be used to create
sequences of numbers starting by arg. The n-th call of seqnext will return arg+n-1

  * log

----    int -> int
----

Computes the base 2 logarithm of the first argument (i.e., the integer part of
it).

  * \verb+<+ , \verb+>+ , =, \verb+<=+ , \verb+>=+ , \#

----    int  x int  --> bool
        int  x real --> bool
        real x int  --> bool
        real x real --> bool
        bool x bool --> bool
        string x string --> bool
----


  * starts, contains

----    string x string --> bool
----

  * not

----    bool  --> bool
----

  * or, and

----    bool x bool --> bool
----

  * int2real

----    int --> real
----

  * real2int

----   real --> int
----

  * int2bool

----   int --> bool
----

  * bool2int

----   boot --> int
----

  * ceil

----  real --> real
----

  * floor

----  real --> real
----

2 Includes

In addition to the normal CC includes we have to include header files
for QueryProcessor and Tuplemanager and the file which contains the
definitions of our four classes: ~CcInt~, ~CcReal~, ~CcBool~, ~CcString~.

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoSystem.h" //operator queries
#include "Counter.h"
#include "StopWatch.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include <math.h>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <time.h>       //needed for random number generator

#include <NList.h>

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

/*
4.1 Type investigation auxiliaries

Within this algebra module, we have to handle with values of four different
types: ~ccint~ and ~ccreal~, ~ccbool~ and ~CcString~.

Later on we will
examine nested list type descriptions. In particular, we
are going to check whether they describe one of the four types just introduced.
In order to simplify dealing with list expressions describing these types, we
declare an enumeration, ~CcType~, containing the four types, and a function,
~TypeOfSymbol~, taking a nested list as  argument and returning the
corresponding ~CcType~ type name.

*/

enum CcType { ccint, ccreal, ccerror, ccbool, ccstring, ccconst, ccset };


CcType
TypeOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == "int"    ) return (ccint);
    if ( s == "real"   ) return (ccreal);
    if ( s == "bool"   ) return (ccbool);
    if ( s == "string" ) return (ccstring);
    if ( s == "const"  ) return (ccconst);
    if ( s == "set"    ) return (ccset);
  }
  return (ccerror);
}

/*
3 Type constructors

A type constructor is created by defining an instance of class
~TypeConstructor~. Before this instance definition may take place we have
to define some functions which are passed as constructor arguments
during ~TypeConstructor~ instantiation.

3.1 Type constructor *CcInt*

Each instance of below defined class CcInt will be the main memory
representation of a
value of type ~ccint~. It consists of a boolean flag, ~defined~,
and an integer value, ~intval~. ~defined~ may be used to indicate whether
an instance of class CcInt represents a valid value or not. E.g., a
division by zero might result in a CcInt object with ~defined~ set to false.
Within this implementation, we don't use the flag but maintain it in order
to demonstrate how to handle complex  objects.

*/
long CcInt::intsCreated = 0;
long CcInt::intsDeleted = 0;

/*

The next function defines the type property of type constructor ~CcInt~.

*/
ListExpr
CcIntProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("int"),
                             nl->StringAtom("(<intvalue>)"),
                             nl->StringAtom("12 or -32 or 0"))));
}

/*

Now we define a function, ~OutInt~, that takes as inputs a type description
and a pointer to a value of this type. The representation of this value in
nested list format is returned.

For the simple types int, real, string, bool we don't use the type description at all. We will need it in the case of more complex type constructors,
e.g. to be able to compute the nested list representation of a tuple value
we must know the types of the respective attribute values.

*/

ListExpr
OutCcInt( ListExpr typeinfo, Word value )
{
  if( ((CcInt*)value.addr)->IsDefined() )
  {
    return (nl->IntAtom( ((CcInt*)value.addr)->GetIntval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*

The function ~InInt~ provides a functionality complementary to ~OutInt~:
A pointer to a value's main memory representation is returned. It is calculated
by taking the value's nested list representation and its type description as
input parameters.

*/

Word
InCcInt( ListExpr typeInfo, ListExpr value,
         int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == IntType )
  {
    correct = true;
    return (SetWord( new CcInt( true, nl->IntValue( value ) ) ));
  }
  else if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType
        && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new CcInt( false, 0) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

/*

*/

Word
CreateCcInt( const ListExpr typeInfo )
{
  return (SetWord( new CcInt( false, 0 ) ));
}

void
DeleteCcInt( const ListExpr typeInfo, Word& w )
{
  ((CcInt*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseCcInt( const ListExpr typeInfo, Word& w )
{
  ((CcInt*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word
CloneCcInt( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((CcInt*)w.addr)->Clone() );
}

int
SizeOfCcInt()
{
  return sizeof(CcInt);
}

/*
3.2.5 {\em Cast}-function of type constructor {\tt ccint}

*/

void*
CastInt( void* addr )
{
  return (new (addr) CcInt);
}

/*
3.2.6 {\em Type check} function of type constructor {\tt ccint}

*/

bool
CheckInt( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "int" ));
}

/*
A type constructor is created by constructing an instance of
class ~TypeConstructor~. Constructor arguments are the type constructor's name
and the four functions previously defined.

*/
TypeConstructor ccInt( "int",            CcIntProperty,
                       OutCcInt,         InCcInt,
                       0,                0,
                       CreateCcInt,      DeleteCcInt,
                       0,        0,      CloseCcInt, CloneCcInt,
                       CastInt,          SizeOfCcInt, CheckInt );

/*
3.2 Type constructor *ccreal*

The following type constructor, ~ccreal~, is defined in the same way as
~ccint~.

*/
long CcReal::realsCreated = 0;
long CcReal::realsDeleted = 0;

/*

The next function defines the type property of type constructor ~CcReal~.

*/
ListExpr
CcRealProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("real"),
                             nl->StringAtom("(<realvalue>)"),
                             nl->StringAtom("12.0 or -1.342 or 14e-3 "
                             "or .23"))));
}

ListExpr
OutCcReal( ListExpr typeinfo, Word value )
{
  if( ((CcReal*)value.addr)->IsDefined() )
  {
    return (nl->RealAtom( ((CcReal*)value.addr)->GetRealval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

Word
InCcReal( ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct )
{
  bool isAtom = nl->IsAtom( value );
  NodeType nodeType = nl->AtomType( value );

  if ( isAtom && nodeType == RealType )
  {
    correct = true;
    return (SetWord( new CcReal( true, nl->RealValue( value ) )));
  }
  else if ( isAtom && nodeType == SymbolType
        && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new CcReal( false, 0.0) ));
  }
  else if ( isAtom && nodeType == IntType  )
  {
    return ( SetWord( new CcReal( true, 1.0 * nl->IntValue(value) )));
    correct = true;
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

Word
CreateCcReal( const ListExpr typeInfo )
{
  return (SetWord( new CcReal( false, 0 ) ));
}

void
DeleteCcReal( const ListExpr typeInfo, Word& w )
{
  ((CcReal*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseCcReal( const ListExpr typeInfo, Word& w )
{
  ((CcReal*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word
CloneCcReal( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((CcReal*)w.addr)->Clone() );
}

int
SizeOfCcReal()
{
  return sizeof(CcReal);
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}

*/

void*
CastReal( void* addr )
{
  return new (addr) CcReal;
}


/*
3.3.7 {\em Type check} function of type constructor {\tt ccreal}

*/

bool
CheckReal( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "real" ));
}


TypeConstructor ccReal( "real",       CcRealProperty,
                        OutCcReal,    InCcReal,
                        0,            0,
                        CreateCcReal, DeleteCcReal,
                        0,            0,
                        CloseCcReal, CloneCcReal,
                        CastReal,   SizeOfCcReal, CheckReal );

/*
3.3 Type constructor *ccbool*

Each instance of below defined class CcBool will be the main memory
representation of a
value of type ~ccbool~. It consists of a boolean flag, ~defined~,
and an boolean value, ~boolval~. ~defined~ may be used to indicate whether
an instance of class CcBool represents a valid value or not. E.g., a
division by zero might result in a CcBool object with ~defined~ set to false.
Within this implementation, we don't use the flag but maintain it in order
to demonstrate how to handle complex  objects.

*/
long CcBool::boolsCreated = 0;
long CcBool::boolsDeleted = 0;

/*

The next function defines the type property of type constructor ~CcBool~.

*/
ListExpr
CcBoolProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("bool"),
                             nl->StringAtom("(<boolvalue>)"),
                             nl->StringAtom("TRUE or FALSE"))));
}

/*
Now we define a function, ~OutBool~, that takes as inputs a type description
and a pointer to a value of this type. The representation of this value in
nested list format is returned.

*/

ListExpr
OutCcBool( ListExpr typeinfo, Word value )
{
  if( ((CcBool*)value.addr)->IsDefined() )
  {
    return (nl->BoolAtom( ((CcBool*)value.addr)->GetBoolval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*
The function ~InBool~ provides a functionality complementary to ~OutBool~:
A pointer to a value's main memory representation is returned. It is calculated
by taking the value's nested list representation and its type description as
input parameters. Again, we don't need the type information in this example due to
simplicity of types used.

The next three parameters are used to return error information.

*/

Word
InCcBool( ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == BoolType )
  {
    correct = true;
    return (SetWord( new CcBool( true, nl->BoolValue( value ) ) ));
  }
  else if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType
        && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new CcBool( false, false) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

Word
CreateCcBool( const ListExpr typeInfo )
{
  return (SetWord( new CcBool( false, 0 ) ));
}

void
DeleteCcBool( const ListExpr typeInfo, Word& w )
{
  ((CcBool*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseCcBool( const ListExpr typeInfo, Word& w )
{
  ((CcBool*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word
CloneCcBool( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((CcBool*)w.addr)->Clone() );
}

int
SizeOfCcBool()
{
  return sizeof(CcBool);
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}

*/

void*
CastBool( void* addr )
{
  return (new (addr) CcBool);
}

/*
3.2.5 {\em Type check} function of type constructor {\tt ccreal}

*/

bool
CheckBool( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "bool" ));
}

TypeConstructor ccBool( "bool",             CcBoolProperty,
                        OutCcBool,          InCcBool,
                        0,                  0,
                        CreateCcBool,       DeleteCcBool,
                        0,                  0,
                        CloseCcBool,        CloneCcBool,
                        CastBool,           SizeOfCcBool,  CheckBool );

/*
3.5 Type constructor *CcString*

*/
long CcString::stringsCreated = 0;
long CcString::stringsDeleted = 0;

bool CcString::Adjacent( const Attribute* arg ) const
{
  const STRING *a = GetStringval(),
               *b = ((CcString *)arg)->GetStringval();

  if( strcmp( *a, *b ) == 0 )
    return 1;

  if( strlen( *a ) == strlen( *b ) )
  {
    if( strncmp( *a, *b, strlen( *a ) - 1 ) == 0 )
    {
      char cha = (*a)[strlen(*a)-1],
           chb = (*b)[strlen(*b)-1];
      return( cha == chb + 1 || chb == cha + 1 );
    }
  }
  else if( strlen( *a ) == strlen( *b ) + 1 )
  {
    return( strncmp( *a, *b, strlen( *b ) ) == 0 &&
            ( (*a)[strlen(*a)-1] == 'a' || (*a)[strlen(*a)-1] == 'A' ) );
  }
  else if( strlen( *a ) + 1 == strlen( *b ) )
  {
    return( strncmp( *a, *b, strlen( *a ) ) == 0 &&
            ( (*b)[strlen(*b)-1] == 'a' || (*b)[strlen(*b)-1] == 'A' ) );
  }

  return 0;
}

/*

The next function defines the type property of type constructor ~CcString~.

*/
ListExpr
CcStringProperty()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist, "\"A piece of text up to 48 "
                 "characters\"");
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("string"),
                             nl->StringAtom("(<stringvalue>)"),
                             examplelist)));
}

/*

*/

ListExpr
OutCcString( ListExpr typeinfo, Word value )
{
  if( ((CcString*)value.addr)->IsDefined() )
  {
    return (nl->StringAtom( *((CcString*)value.addr)->GetStringval() ));
  }
  else
  {
    return (nl->SymbolAtom("undef"));
  }
}

/*

*/

Word
InCcString( ListExpr typeInfo, ListExpr value,
            int errorPos, ListExpr& errorInfo, bool& correct )
{
  if ( nl->IsAtom( value ) && nl->AtomType( value ) == StringType )
  {
    correct = true;
    string s = nl->StringValue( value );
    return (SetWord( new CcString( true, (STRING*)s.c_str() ) ));
  }
  else if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType
        && nl->SymbolValue( value ) == "undef" )
  {
    correct = true;
    return (SetWord( new CcString( false, (STRING*)"" ) ));
  }
  else
  {
    correct = false;
    return (SetWord( Address( 0 ) ));
  }
}

/*

*/

Word
CreateCcString( const ListExpr typeInfo )
{
  char p[MAX_STRINGSIZE+1] = "";
  return (SetWord( new CcString( false, (STRING*)&p ) ));
}

void
DeleteCcString( const ListExpr typeInfo, Word& w )
{
  ((CcString*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void
CloseCcString( const ListExpr typeInfo, Word& w )
{
  ((CcString*)w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word
CloneCcString( const ListExpr typeInfo, const Word& w )
{
  return SetWord( ((CcString*)w.addr)->Clone() );
}

int
SizeOfCcString()
{
  return sizeof(CcString);
}

/*
3.3.6 {\em Cast}-function of type constructor {\tt ccreal}

*/

void*
CastString( void* addr )
{
  return (new (addr) CcString);
}

/*
3.2.5 {\em Type check} function of type constructor {\tt ccreal}

*/
bool
CheckString( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "string" ));
}

TypeConstructor ccString( "string",       CcStringProperty,
                          OutCcString,    InCcString,
                          0,              0,
                          CreateCcString, DeleteCcString,
                          0,              0,
                          CloseCcString,  CloneCcString,
                          CastString,     SizeOfCcString, CheckString );

/*
4 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

4.2 Type mapping function

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

4.2.1 Type mapping function CcMathTypeMap

It is for the operators +, - and [*].

*/

ListExpr
CcMathTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
    {
      return (nl->SymbolAtom( "int" ));
    }
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal )
    {
      return (nl->SymbolAtom( "real" ));
    }
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
    {
      return (nl->SymbolAtom( "real" ));
    }
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
    {
      return (nl->SymbolAtom( "real" ));
    }
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
    {
      return (nl->SymbolAtom( "string" ));
    }

  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.2 Type mapping function CcMathTypeMapdiv

It is for the operators /. the only difference between CCMathTypeMap and
CcMathTypeMapdiv is that the latter give as resulttype ccreal if the input type
is ccint ("normal division of int with result real", the other division
of int is called div in this program).

*/

ListExpr
CcMathTypeMapdiv( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "real" ));
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal )
      return (nl->SymbolAtom( "real" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "real" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
      return (nl->SymbolAtom( "real" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.3 Type mapping function CcMathTypeMap1

It is for the operators mod and div which have ~int~ as input and ~int~ as result.

*/

ListExpr
CcMathTypeMap1( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.4 Type mapping function CcMathTypeMap2

It is for the operators ~intersection~ and ~minus~.

*/
ListExpr
CcMathTypeMap2( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "int" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
      return (nl->SymbolAtom( "real" ));
    if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
      return (nl->SymbolAtom( "string" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.4 Type mapping functions IntInt, IntBool, BoolInt and EmptyInt

Used for operators ~randint~, ~randmax~, ~initseq~, ~nextseq~ and ~log~.
And for ~int2bool~, ~bool2int~.

*/

ListExpr
IntInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( nl->IsEqual( arg1, "int" ) )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

ListExpr
RealReal( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( nl->IsEqual( arg1, "real" ) )
      return (nl->SymbolAtom( "real" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

ListExpr
RealInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( nl->IsEqual( arg1, "real" ) )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

ListExpr
IntReal( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( nl->IsEqual( arg1, "int" ) )
      return (nl->SymbolAtom( "real" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

ListExpr
IntBool( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( nl->IsEqual( arg1, "int" ) )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

ListExpr
EmptyInt( ListExpr args )
{
  if ( !nl->IsEmpty( args ) ) {
    return (nl->SymbolAtom( "typeerror" ));
  } else {
    return (nl->SymbolAtom( "int" ));
  }
}

ListExpr
BoolInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( nl->IsEqual( arg1, "bool" ) )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.5 Type mapping function CcMathTypeMapBool

It is for the Compare operators which have ~bool~ as resulttype.

*/

ListExpr
CcMathTypeMapBool( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring)
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.6 Type mapping function CcMathTypeMapBool1

It is for the  operator ~not~ which have ~bool~ as input and resulttype.

*/

ListExpr
CcMathTypeMapBool1( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccbool )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.7 Type mapping function CcMathTypeMapBool2

It is for the  operators and and or  which have bool as input and resulttype.

*/

ListExpr
CcMathTypeMapBool2( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.8 Type mapping function CcMathTypeMapBool3

It is for the  operators ~starts~ and ~contains~  which have ~string~ as input and ~bool~ resulttype.

*/

ListExpr
CcMathTypeMapBool3( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.9 Type mapping function CcMathTypeMapBool4

It is for the  operators ~isempty~ which have ~bool~, ~int~, ~real~, and ~string~ as input and ~bool~ resulttype.

*/

ListExpr
CcMathTypeMapBool4( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccbool )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccint )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccreal )
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccstring )
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.10 Type mapping function for the ~upper~ operator:

string ---> string.

*/

ListExpr
CcStringMapCcString( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccstring )
      return (nl->SymbolAtom( "string" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.10 Type mapping function for the ~substr~ operator:

string x int x int -> string.

*/

ListExpr
SubStrTypeMap( ListExpr args )
{
  if ( nl->ListLength( args ) == 3 )
  {
    ListExpr arg1 = nl->First( args );
    ListExpr arg2 = nl->Second( args );
    ListExpr arg3 = nl->Third( args );

    if (    (TypeOfSymbol( arg1 ) == ccstring)
         && (TypeOfSymbol( arg2 ) == ccint)
         && (TypeOfSymbol( arg3 ) == ccint)
        )
    {
      return (nl->SymbolAtom( "string" ));
    }
  }
  ErrorReporter::ReportError(
        "Expecting an argument list of type (string int int).");
  return (nl->SymbolAtom( "typeerror" ));

}


/*
4.2.11 Type mapping function for the ~relcount~ operator:

string ---> int.

*/

static ListExpr
CcStringMapCcInt( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccstring )
      return (nl->SymbolAtom( "int" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.12 Type mapping function for the ~keywords~ operator:

Type mapping for ~keywords~ is

----    (string) -> (stream string)
----

*/
ListExpr
keywordsType( ListExpr args ){
  ListExpr arg;

  if ( nl->ListLength(args) == 1 )
  {
    arg = nl->First(args);
    if ( nl->IsEqual(arg, "string") )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
        nl->SymbolAtom("string"));
  }
  return nl->SymbolAtom("typeerror");
}

/*
4.2.13 Type mapping function for the ~ifthenelse~ operator:

Type mapping for ~ifthenelse~ is

----    (bool x T x T)) -> T
----

*/
ListExpr ifthenelseType(ListExpr args)
{
  ListExpr arg1, arg2, arg3,
           errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  if ( nl->ListLength( args ) == 3 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Third( args );
    errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

    if (nl->Equal(arg2, arg3) && nl->SymbolValue(arg1) == "bool" &&
        SecondoSystem::GetAlgebraManager()
                ->CheckKind("DATA", arg2, errorInfo) &&
        SecondoSystem::GetAlgebraManager()
                ->CheckKind("DATA", arg3, errorInfo) )
    {
      return arg2;
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator ifthenelse.");
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.14 Type mapping function for the ~between~ operator:

Type mapping for ~between~ is

----    (T x T x T) -> bool
----

*/
ListExpr
CcBetweenTypeMap( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if ( nl->ListLength( args ) == 3 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint
                                       && TypeOfSymbol( arg3 ) == ccint)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal
                                        && TypeOfSymbol( arg3 ) == ccreal)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring
                                          && TypeOfSymbol( arg3 ) == ccstring)
      return (nl->SymbolAtom( "bool" ));
    if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool
                                        && TypeOfSymbol( arg3 ) == ccbool)
      return (nl->SymbolAtom( "bool" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.14 Type mapping function for the ~hashvalue~ operator:

Type mapping for ~hashvalue~ is

----    T in kind DATA, T x int -> int
----

*/
ListExpr
CcHashValueTypeMap( ListExpr args )
{
  ListExpr arg1, arg2,
           errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));

  CHECK_COND(nl->ListLength(args) == 2,
  "Operator hasvalue expects a list of length two.");

  arg1 = nl->First( args );
  arg2 = nl->Second( args );

  CHECK_COND(am->CheckKind("DATA", arg1, errorInfo),
  "Object type of first argument does not belong to kind DATA!");

  CHECK_COND(TypeOfSymbol( arg2 ) == ccint,
  "Object type of second argument must be int!");

  return (nl->SymbolAtom( "int" ));
}

/*
4.2.15 Type mapping function for the ~ldistance~ operator:

Type mapping for ~ldistance~ is string x string [->] int


*/

ListExpr CcLDistTypeMap(ListExpr args){
   if(nl->ListLength(args)!=2){
       ErrorReporter::ReportError("two arguments expected\n");
       return nl->SymbolAtom("typeerror");
   }
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(!nl->IsEqual(arg1,"string") ||
      !nl->IsEqual(arg2,"string")){
       ErrorReporter::ReportError("string x string required");
       return nl->SymbolAtom("typeerror");
   }
   return nl->SymbolAtom("int");
}

/*
4.2.16 Type mapping function CcRoundTypeMap

It is for the  operators ~round~, which has ~real~ and ~int~ as input and ~real~ resulttype.

*/

ListExpr
CcRoundTypeMap( ListExpr args )
{
  ListExpr arg1, arg2;
  if ( nl->ListLength( args ) == 2 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
      return (nl->SymbolAtom( "real" ));
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.2.17 Type mappig function ~NumStringTypeMap~

For operator ~num2string~

*/
ListExpr
NumStringTypeMap( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccreal || TypeOfSymbol( arg1 ) == ccint )
      return (nl->SymbolAtom( "string" ));
    else
    {
       ErrorReporter::ReportError("Operator num2string expects an argument "
                                  "of type 'int' or 'real'");
    }
  }
  else
    ErrorReporter::ReportError("Operator num2string expects an argument "
                               "list of length 1.");
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.3 Selection function

A selection function is quite similar to a type mapping function. The only
difference is that it doesn't return a type but the index of a value
mapping function being able to deal with the respective combination of
input parameter types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

*/

/*
4.3.1 Selection function  CcMathSelectCompute

It is used for the  operators +, - [*] and / .

*/

int
CcMathSelectCompute( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
    return (0);
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal )
    return (1);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
    return (2);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
    return (3);
  if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
    return (4);
  return (-1); // This point should never be reached
}

/*
4.3.3 Selection function  CcMathSelectCompare

It is used for the  all compare operators .

*/

int
CcMathSelectCompare( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
    return (0);
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccreal )
    return (1);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccint )
    return (2);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
    return (3);
  if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool )
    return (4);
  if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
    return (5);
  return (-1); // This point should never be reached
}

/*
4.3.4 Selection function  CcMathSelectSet

It is used for the set operators ~intersection~ and ~minus~.

*/

int
CcMathSelectSet( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );
  if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint )
    return (0);
  if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal )
    return (1);
  if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool )
    return (2);
  if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring )
    return (3);
  return (-1); // This point should never be reached
}


/*
4.3.3 Selection function  CcMathSelectIsEmpty

It is used for the ~isempty~ operator.

*/
int
CcMathSelectIsEmpty( ListExpr args )
{
  ListExpr arg1 = nl->First( args );
  if ( TypeOfSymbol( arg1 ) == ccbool )
    return (0);
  if ( TypeOfSymbol( arg1 ) == ccint )
    return (1);
  if ( TypeOfSymbol( arg1 ) == ccreal )
    return (2);
  if ( TypeOfSymbol( arg1 ) == ccstring )
    return (3);
  return (-1); // This point should never be reached
}

/*
4.3.3 Selection function  CcBetweenSelect

It is used for the ~between~ operator.

*/
int
CcBetweenSelect( ListExpr args )
{
  ListExpr arg1, arg2, arg3;
  if ( nl->ListLength( args ) == 3 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Second( args );
    if ( TypeOfSymbol( arg1 ) == ccint && TypeOfSymbol( arg2 ) == ccint
                                       && TypeOfSymbol( arg3 ) == ccint)
      return ( 0 );
    if ( TypeOfSymbol( arg1 ) == ccreal && TypeOfSymbol( arg2 ) == ccreal
                                        && TypeOfSymbol( arg3 ) == ccreal)
      return ( 1 );
    if ( TypeOfSymbol( arg1 ) == ccstring && TypeOfSymbol( arg2 ) == ccstring
                                          && TypeOfSymbol( arg3 ) == ccstring)
      return ( 2 );
    if ( TypeOfSymbol( arg1 ) == ccbool && TypeOfSymbol( arg2 ) == ccbool
                                        && TypeOfSymbol( arg3 ) == ccbool)
      return ( 3 );
  }
  return ( -1 );
}

int
ccnum2stringSelect( ListExpr args )
{
  ListExpr arg1;
  if ( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if ( TypeOfSymbol( arg1 ) == ccreal )
      return ( 0 );
    if ( TypeOfSymbol( arg1 ) == ccint )
      return ( 1 );
  }
  return ( -1 );
}


/*
4.4 Value mapping functions of operator ~+~

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators --- like in
this example --- there are several value mapping functions, one for each
possible combination of input parameter types. We have to provide
four functions for each of the operators ~+~, ~-~,  ~[*]~ ....., since
each of them accepts four input parameter combinations: ~ccint~ $\times$
~ccint~, ~ccint~ $\times$ ~ccreal~,  ~ccreal~ $\times$ ~ccint~, and
~ccreal~ $\times$ ~ccreal~.

*/

int
CcPlus_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    // overflow save implementation
    int a = ((CcInt*)args[0].addr)->GetIntval();
    int b = ((CcInt*)args[1].addr)->GetIntval();
    int sum = a+b;
    if( ((b>0 ) && (sum<a)) || ((b<0) && (sum)>a)){
           ((CcInt*)result.addr)->Set(false,0);
    } else {
        ((CcInt *)result.addr)->Set( true, sum);
    }
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcPlus_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() +
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcPlus_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcReal*)args[0].addr)->IsDefined() &&
      ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() +
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcPlus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() +
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcPlus_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  // extract arguments
  CcString* wstr1 = reinterpret_cast<CcString*>( args[0].addr );
  CcString* wstr2 = reinterpret_cast<CcString*>( args[1].addr );

  string str1 = reinterpret_cast<const char*>( wstr1->GetStringval() );
  string str2 = reinterpret_cast<const char*>( wstr2->GetStringval() );

  CcString* wres = reinterpret_cast<CcString*>( result.addr );

  // compute result value
  if ( wstr1->IsDefined() && wstr2->IsDefined() )
  {
    wres->Set( true, (STRING*)(str1 + str2).substr(0,MAX_STRINGSIZE).c_str() );
  }
  else
  {
    STRING str = "";
    wres->Set( false, &str );
  }
  return (0);
}


/*
4.5 Value mapping functions of operator ~-~

*/

int
CcMinus_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    int a = ((CcInt*)args[0].addr)->GetIntval();
    int b = ((CcInt*)args[1].addr)->GetIntval();
    int diff = a-b;
    if( ((b>0) && (diff>a) ) || ((b<0) && (diff<a)))
    {
       ((CcInt *)result.addr)->Set(false, 0);
    } 
    else
    {
       ((CcInt *)result.addr)->Set(true, diff);
    }
     
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcMinus_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() -
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcMinus_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() -
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcMinus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() -
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.6 Value mapping functions of operator ~[*]~

Value mapping functions of operator ~[*]~ are defined in the same way as
value mapping functions of operator ~+~.

*/

int
CcProduct_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    int a = ((CcInt*)args[0].addr)->GetIntval();
    int b = ((CcInt*)args[1].addr)->GetIntval();
    int prod = a*b;
    if( (b!=0) && ((prod/b)!=a)) 
    {
        ((CcInt *)result.addr)->Set( false, 0 );
    }
    else
    {
        ((CcInt *)result.addr)->Set( true, prod );
    }
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcProduct_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() *
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcProduct_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() *
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcProduct_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() *
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.7 Value mapping functions of operator ~/~

*/

int
CcDivision_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->GetIntval() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((REAL )((CcInt*)args[0].addr)->GetIntval()) /
                          ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcDivision_ir( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->GetRealval() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() /
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcDivision_ri( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->GetIntval() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() /
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

int
CcDivision_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->GetRealval() )
  {
    ((CcReal *)result.addr)->
      Set( true, ((CcReal*)args[0].addr)->GetRealval() /
                 ((CcReal*)args[1].addr)->GetRealval() );
  }
  else
  {
    ((CcReal *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.8 Value mapping functions of operator ~mod~

*/

int
CcMod( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    ((CcInt *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() %
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.8 Value mapping functions of operator ~div~

*/

int
CcDiv( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() &&
      ((CcInt*)args[1].addr)->IsDefined() &&
      ((CcInt*)args[1].addr)->GetIntval() )
  {
    ((CcInt *)result.addr)->
      Set( true, ((CcInt*)args[0].addr)->GetIntval() /
                 ((CcInt*)args[1].addr)->GetIntval() );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}




/*
4.8 Value mapping function of operators ~randint~, ~maxrand~, ~initseq~ and ~nextseq~

*/

int randint(int u)      //Computes a random integer in the range 0..u-1,
                        //for u >= 2
{
  if ( u < 2 ) {u=2; srand ( time(NULL) );}
        // For u < 2 also initialize the random number generator
  // rand creates a value between [0,RAND_MAX]. The calculation procedure
  // below is recommended in the manpage of the rand() function.
  // Using rand() % u will yield poor results.
  return (int) ( (float)u * rand()/(RAND_MAX+1.0) );
}


int
RandInt( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() )
  {
    ((CcInt *)result.addr)->
      Set( true, randint(((CcInt*)args[0].addr)->GetIntval()) );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}


int
RandSeed( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() )
  {
    srand(((CcInt*)args[0].addr)->GetIntval());
    ((CcBool *)result.addr)->
      Set( true, true);
  }
  else
  {
    ((CcBool*)result.addr)->Set( false, false );
  }
  return (0);
}

int
MaxRand( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)-> Set( true, RAND_MAX );
  return (0);
}


static int SequenceCounter = 0;


int
InitSeq( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->Set( true, true );
      SequenceCounter = ((CcInt*)args[0].addr)->GetIntval();
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

int
NextSeq( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  ((CcInt *)result.addr)-> Set( true, SequenceCounter );
  SequenceCounter++;
  return (0);
}


/*
4.8 Value mapping function of operator ~log~

*/

int intlog(int n)
{
  int i = 0;
  while (n > 1) {n >>= 1; i++;}
  return i;
}


int
LogFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcInt*)args[0].addr)->IsDefined() &&
      ((CcInt*)args[0].addr)->GetIntval() )
  {
    int n = intlog(((CcInt*)args[0].addr)->GetIntval());
    ((CcInt *)result.addr)-> Set( true, n );
  }
  else
  {
    ((CcInt *)result.addr)-> Set( false, 0 );
  }
  return (0);
}


/*
4.9 Value mappings for operator <


*/

template<class T>
int
CcLess( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const T* a = static_cast<const T*>( args[0].addr );
  const T* b = static_cast<const T*>( args[1].addr );

  ((CcBool *)result.addr)->Set( true, a->Compare(b) == -1 );
  return (0);
}

template<class T>
int
CcGreater( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const T* a = static_cast<const T*>( args[0].addr );
  const T* b = static_cast<const T*>( args[1].addr );

  ((CcBool *)result.addr)->Set( true, a->Compare(b) == 1 );
  return (0);
}

template<class T>
int
CcEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const T* a = static_cast<const T*>( args[0].addr );
  const T* b = static_cast<const T*>( args[1].addr );

  ((CcBool *)result.addr)->Set( true, a->Compare(b) == 0 );
  return (0);
}


template<class S, class T>
int
CcLess2( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((S*)args[0].addr)->IsDefined() &&
       ((T*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((S*)args[0].addr)->GetValue() <
                 ((T*)args[1].addr)->GetValue() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}


/*
4.10 Value mapping functions of operator $ <= $

*/

template<class T>
int
CcLessEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const T* a = static_cast<const T*>( args[0].addr );
  const T* b = static_cast<const T*>( args[1].addr );

  ((CcBool *)result.addr)->Set( true, a->Compare(b) <= 0 );
  return (0);
}


template<class S, class T>
int
CcLessEqual2( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((S*)args[0].addr)->IsDefined() &&
       ((T*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((S*)args[0].addr)->GetValue() <=
                 ((T*)args[1].addr)->GetValue() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}


/*
4.11 Value mapping functions of operator $ > $

*/


template<class S, class T>
int
CcGreater2( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((S*)args[0].addr)->IsDefined() &&
       ((T*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((S*)args[0].addr)->GetValue() >
                 ((T*)args[1].addr)->GetValue() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}



/*
4.12 Value mapping functions of operator $ >= $

*/

template<class T>
int
CcGreaterEqual( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const T* a = static_cast<const T*>( args[0].addr );
  const T* b = static_cast<const T*>( args[1].addr );

  ((CcBool *)result.addr)->Set( true, a->Compare(b) >= 0 );
  return (0);
}



template<class S, class T>
int
CcGreaterEqual2( Word* args, Word& result, int message, Word& local,
                 Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((S*)args[0].addr)->IsDefined() &&
       ((T*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((S*)args[0].addr)->GetValue() >=
                 ((T*)args[1].addr)->GetValue() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}



/*
4.13 Value mapping functions of operator ~=~

*/


template<class S, class T>
int
CcEqual2( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((S*)args[0].addr)->IsDefined() &&
       ((T*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((S*)args[0].addr)->GetValue() ==
                 ((T*)args[1].addr)->GetValue() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}


/*
4.14 Value mapping functions of operator ~\#~

*/

template<class T>
int
CcDiff( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const T* a = static_cast<const T*>( args[0].addr );
  const T* b = static_cast<const T*>( args[1].addr );

  ((CcBool *)result.addr)->Set( true, a->Compare(b) != 0 );
  return (0);
}


template<class S, class T>
int
CcDiff2( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((S*)args[0].addr)->IsDefined() &&
       ((T*)args[1].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)->
      Set( true, ((S*)args[0].addr)->GetValue() !=
                   ((T*)args[1].addr)->GetValue() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.14 Value mapping functions of operator ~starts~

*/

int
StartsFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    string str1 = (string)(char*)((CcString*)args[0].addr)->GetStringval();
    string str2 = (string)(char*)((CcString*)args[1].addr)->GetStringval();
    char* cstr1 = (char*)((CcString*)args[0].addr)->GetStringval();
    char* cstr2 = (char*)((CcString*)args[1].addr)->GetStringval();
    ((CcBool *)result.addr)->
    Set (true, strncmp(cstr1, cstr2, str2.length()) == 0 );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.14 Value mapping functions of operator ~contains~

*/

int
ContainsFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    string str1 = (string)(char*)((CcString*)args[0].addr)->GetStringval();
    string str2 = (string)(char*)((CcString*)args[1].addr)->GetStringval();
    ((CcBool *)result.addr)->
    Set (true, str1.find(str2) != string::npos);

    //((CcBool *)result.addr)->
      //Set( true, ((CcString*)args[0].addr)->GetStringval()->find(
     //*((CcString*)args[1].addr)->GetStringval() ) != string::npos );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.15 type and value mapping functions of operator ~substr~

*/



int
SubStrFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  // extract arguments
  CcString* wstr = reinterpret_cast<CcString*>( args[0].addr );
  CcInt* wpos1 = reinterpret_cast<CcInt*>( args[1].addr );
  CcInt* wpos2 = reinterpret_cast<CcInt*>( args[2].addr );

  string str1 = reinterpret_cast<const char*>( wstr->GetStringval() );
  int p1 = wpos1->GetIntval();
  int p2 = wpos2->GetIntval();

  CcString* wres = reinterpret_cast<CcString*>( result.addr );

  // compute result value
  if (    wstr->IsDefined()
       && wpos1->IsDefined()
       && wpos2->IsDefined()
       && (p2 >= p1) && (p1 >= 1) )
  {
    int n = min( static_cast<long unsigned int>(p2-p1),
        static_cast<long unsigned int>(str1.length()-p1) );
    wres->Set( true, (STRING*)(str1.substr(p1-1, n+1).c_str()) );
  }
  else
  {
    STRING str = "";
    wres->Set( false, &str );
  }
  return (0);
}



/*
4.15 Value mapping functions of operator ~not~

*/

int
NotFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() )
  {
    ((CcBool *)result.addr)
        ->Set( true, !((CcBool*)args[0].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.16 Value mapping functions of operator ~and~

*/

int
AndFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( (((CcBool*)args[0].addr)->IsDefined() &&
        ((CcBool*)args[1].addr)->IsDefined()) )
  {
    ((CcBool*)result.addr)->Set( true,
        ((CcBool*)args[0].addr)->GetBoolval() &&
                       ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.17 Value mapping functions of operator ~or~

*/

int
OrFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcBool*)args[0].addr)->IsDefined() &&
      ((CcBool*)args[1].addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true,
        ((CcBool*)args[0].addr)->GetBoolval() ||
                  ((CcBool*)args[1].addr)->GetBoolval() );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.17 Value mapping functions of operator ~isempty~

*/

template<class T>
int
IsEmpty( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  const T* arg = static_cast<const T*>( args[0].addr );
  if( arg->IsDefined() )
  {
    ((CcBool*)result.addr)->Set( true, false );
  }
  else
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  return (0);
}


/*
4.18 Value mapping functions of operator ~upper~

*/

int
UpperFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  char * orgStr = (char*)((CcString*)args[0].addr)->GetStringval();
  char newStr[256];
  char *lastChar;

  if( ((CcString*)args[0].addr)->IsDefined() )
  {
      strcpy(newStr, orgStr);
      if (strlen(newStr)>0)
      {
           lastChar= newStr + strlen(newStr) -1;
           *lastChar=*lastChar+1;
       }
      ((CcString *)result.addr)->Set( true, (STRING *)&newStr );
  }
  else
  {
      ((CcString *)result.addr)->Set( false,
        ((CcString*)args[0].addr)->GetStringval());
  }
  return (0);
}

/*
4.19 Value mapping functions of operator ~intersection~

*/

int
CcSetIntersection_ii( Word* args, Word& result, int message, Word& local,
        Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    if( ((CcInt*)args[0].addr)->GetIntval()
        == ((CcInt*)args[1].addr)->GetIntval() )
    {
      ((CcInt *)result.addr)->Set( true,
        ((CcInt*)args[0].addr)->GetIntval() );
      return (0);
    }
  }
  ((CcInt *)result.addr)->Set( false, 0 );
  return (0);
}

int
CcSetIntersection_rr( Word* args, Word& result, int message, Word& local,
        Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    if( ((CcReal*)args[0].addr)->GetRealval()
        == ((CcReal*)args[1].addr)->GetRealval() )
    {
      ((CcReal *)result.addr)->Set( true,
        ((CcReal*)args[0].addr)->GetRealval() );
      return (0);
    }
  }
  ((CcReal *)result.addr)->Set( false, 0.0 );
  return (0);
}

int
CcSetIntersection_bb( Word* args, Word& result, int message, Word& local,
        Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    if( ((CcBool*)args[0].addr)->GetBoolval()
        == ((CcBool*)args[1].addr)->GetBoolval() )
    {
      ((CcBool*)result.addr)->Set( true,
        ((CcBool*)args[0].addr)->GetBoolval() );
      return (0);
    }
  }
  ((CcBool *)result.addr)->Set( false, false );
  return (0);
}

int
CcSetIntersection_ss( Word* args, Word& result, int message, Word& local,
        Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    if( strcmp( *((CcString*)args[0].addr)->GetStringval(),
        *((CcString*)args[1].addr)->GetStringval() ) == 0 )
    {
      ((CcString*)result.addr)->Set( true,
        ((CcString*)args[0].addr)->GetStringval() );
      return (0);
    }
  }
  STRING nullStr = "";
  ((CcString*)result.addr)->Set( false, &nullStr );
  return (0);
}

/*
4.19 Value mapping functions of operator ~minus~

*/

int
CcSetMinus_ii( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcInt*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() )
  {
    if( ((CcInt*)args[0].addr)->GetIntval()
        == ((CcInt*)args[1].addr)->GetIntval() )
    {
      ((CcInt *)result.addr)->Set( false, 0 );
      return (0);
    }
  }
  ((CcInt *)result.addr)->Set( true, ((CcInt*)args[0].addr)->GetIntval() );
  return (0);
}

int
CcSetMinus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcReal*)args[0].addr)->IsDefined() &&
       ((CcReal*)args[1].addr)->IsDefined() )
  {
    if( ((CcReal*)args[0].addr)->GetRealval()
        == ((CcReal*)args[1].addr)->GetRealval() )
    {
      ((CcReal *)result.addr)->Set( false, 0.0 );
      return (0);
    }
  }
  ((CcReal *)result.addr)->Set( true, ((CcReal*)args[0].addr)->GetRealval() );
  return (0);
}

int
CcSetMinus_bb( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcBool*)args[0].addr)->IsDefined() &&
       ((CcBool*)args[1].addr)->IsDefined() )
  {
    if( ((CcBool*)args[0].addr)->GetBoolval()
        == ((CcBool*)args[1].addr)->GetBoolval() )
    {
      ((CcBool *)result.addr)->Set( false, false );
      return (0);
    }
  }
  ((CcBool*)result.addr)->Set( true, ((CcBool*)args[0].addr)->GetBoolval() );
  return (0);
}

int
CcSetMinus_ss( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if ( ((CcString*)args[0].addr)->IsDefined() &&
       ((CcString*)args[1].addr)->IsDefined() )
  {
    if( strcmp( *((CcString*)args[0].addr)->GetStringval(),
        *((CcString*)args[1].addr)->GetStringval() ) == 0 )
    {
      STRING nullStr = "";
      ((CcString*)result.addr)->Set( false, &nullStr );
      return (0);
    }
  }
  ((CcString*)result.addr)->Set( true,
        ((CcString*)args[0].addr)->GetStringval() );
  return (0);
}

/*
4.20 Value mapping function of operator ~relcount~

*/

static int
RelcountFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  ListExpr resultType, queryList, resultList, valueList;
  QueryProcessor* qpp = 0;
  OpTree tree;
  char* relname;
  bool correct      = false;
  bool evaluable    = false;
  bool defined      = false;
  bool isFunction   = false;

  result = qp->ResultStorage( s );

  if ( ((CcString*)args[0].addr)->IsDefined() )
  {
    // create the query list (count (feed <relname>))
    relname = (char*)(((CcString*)args[0].addr)->GetStringval());
    string querystring = "(count(feed " + (string)relname + "))";
    nl->ReadFromString(querystring, queryList);

    // construct the operator tree within a new query processor instance
    // NOTE: variable name for this instance must differ from qp
    qpp = new QueryProcessor( nl, SecondoSystem::GetAlgebraManager() );
    qpp->Construct( queryList, correct,
                    evaluable, defined, isFunction, tree, resultType );
    if ( !defined )
    {
      cout << "object value is undefined" << endl;
    }
    else if ( correct )
    {
      if ( evaluable )
      {
        // evaluate the operator tree
        qpp->Eval( tree, result, 1 );

        // create the result list ( type, value )
        valueList = SecondoSystem::GetCatalog()->
          OutObject( resultType, result );
        resultList = nl->TwoElemList( resultType, valueList );

        // set the result value and destroy the operator tree
        ((CcInt *)result.addr)->Set ( true,
          nl->IntValue(nl->Second(resultList)) );
        qpp->Destroy( tree, false );
      }
      else cout << "Operator query not evaluable" << endl;
    }
    else cout << "Error in operator query" << endl;
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  delete qpp;
  return (0);
}

/*
4.21 Value mapping function of operator ~relcount2~

*/

static int
RelcountFun2( Word* args, Word& result, int message, Word& local, Supplier s )
{
  char* relname;
  string querystring;
  Word resultword;

  result = qp->ResultStorage( s );

  if ( ((CcString*)args[0].addr)->IsDefined() )
  {
    // create the query list (count (feed <relname>))
    relname = (char*)(((CcString*)args[0].addr)->GetStringval());
    querystring = "(count(feed " + (string)relname + "))";

    // the if statement can be simply reduced to:
    // if ( QueryProcessor::ExecuteQuery(querystring, result) )
    if ( QueryProcessor::ExecuteQuery(querystring, resultword) )
    {
      ((CcInt *)result.addr)->Set( true,
      ((CcInt*)resultword.addr)->GetIntval() );
    }
    else cout << "Error in executing operator query" << endl;
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.21 Value mapping function of operator ~keywords~

The following auxiliary function ~trim~ removes any kind of space
characters from the end of a string.

*/

int trim (char s[])
{
  int n;

  for(n = strlen(s) - 1; n >= 0; n--)
   if ( !isspace(s[n]) ) break;
  s[n+1] = '\0';
  return n;
}

int
keywordsFun (Word* args, Word& result, int message, Word& local, Supplier s)
/*
Creates a stream of strings containing the single words
of the origin string, on the assumption, that words in a string
are separated by a space character.

*/
{
  struct Subword {int start, nochr, strlength; STRING* subw;}* subword;

  CcString* elem, *str;
  //CcString* str;
  int i;
  string teststr, tmpstr;
  STRING outstr;
  Word arg0;

  switch( message )
  {
    case OPEN:
      // cout << "open" << endl;
      arg0 = args[0];

      str = ((CcString*)arg0.addr);

      subword = new Subword;
      subword->start = 0;
      subword->nochr = 0;

      subword->subw = (STRING*)malloc(strlen(*str->GetStringval()) + 1);
      // copy input string to allocated memory
      strcpy(*subword->subw, *str->GetStringval());
      trim( *subword->subw ); //remove spaces from the end of the string

      subword->strlength = strlen(*subword->subw);
      (*subword->subw)[subword->strlength] = '\0';
      // get the necessary values to determine the first single word in the
      // string, if it is not empty or contains only space characters.
      if ( subword->strlength > 0) {
        i=0;
        while ( isspace(*subword->subw[i]) ) i++;
        subword->start = i;

        i=subword->start;
        while ( (!(isspace(((*subword->subw)[i]))) &&
                ((*subword->subw)[i]) != '\0') ) i++;
        subword->nochr = i - subword->start;
      }
      local.addr = subword;
      return 0;

    case REQUEST:
      //  cout << "request" << endl;
      subword = ((Subword*) local.addr);
      // another single word in the string still exists
      if ( (subword->strlength > 0) && (subword->start < subword->strlength) )
      {
        tmpstr = (((string)(*subword->subw)).substr(subword->start,
                subword->nochr));
        strcpy(outstr, (char*)tmpstr.c_str());
        trim(outstr);
        elem = new CcString(true, &outstr);
        result.addr = elem;
        // determine the necessary values to get the next word in the string,
        // if there is any.
        subword->start += subword->nochr;
        i = subword->start;
        if (i < subword->strlength ) {
          while ( isspace((*subword->subw)[i]) ) i++;

          subword->start = i;
          while ( (!isspace((*subword->subw)[i])) &&
                  (((*subword->subw)[i]) != '\0') )  i++;
          subword->nochr = i - subword->start + 1;
        }
        local.addr = subword;

        return YIELD;
      }
      // no more single words in the string
      else
      {
        // string is empty or contains only space characters
        if ( subword->strlength == 0 ) {
          outstr[0] = '\0';
          elem = new CcString(true, &outstr);
          result.addr = elem;
          subword->start = subword->strlength = 1;
          local.addr = subword;
          return YIELD;
        }
      return CANCEL;
      }

    case CLOSE:
      // cout << "close" << endl;
      subword = ((Subword*) local.addr);
      free(subword->subw);
      delete subword;
      return 0;
  }
  /* should not happen */
  return -1;
}

int
ifthenelseFun(Word* args, Word& result, int message, Word& local, Supplier s)
{
    result = qp->ResultStorage( s );

    if (!((CcBool*)args[0].addr)->IsDefined() )
    {
        ((StandardAttribute*)result.addr)->SetDefined( false );
    }
    else if(((CcBool*)args[0].addr)->GetBoolval())
    {
        ((StandardAttribute*)result.addr)->CopyFrom(
                (StandardAttribute*)args[1].addr );
    }
    else
    {
        ((StandardAttribute*)result.addr)->CopyFrom(
                (StandardAttribute*)args[2].addr );
    }

    return 0;
}

/*
4.21 Value mapping functions of operator ~between~

*/

template<class T>
int
CcBetween( Word* args, Word& result, int message, Word& local,
           Supplier s)
{
  result = qp->ResultStorage( s );
  if ( ((T*)args[0].addr)->IsDefined() &&
       ((T*)args[1].addr)->IsDefined() &&
        ((T*)args[2].addr)->IsDefined() )
  {
    if ( ((T*)args[1].addr)->GetValue()
        <= ((T*)args[2].addr)->GetValue() )
    {
      ((CcBool *)result.addr)->Set( true, (
        ((T*)args[0].addr)->GetValue() >=
        ((T*)args[1].addr)->GetValue()) &&
       (((T*)args[0].addr)->GetValue() <=
        ((T*)args[2].addr)->GetValue()));
    }
    else cerr << "ERROR in operator between: second argument must be less or"
                 " equal third argument!" << endl;
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }

  return (0);
}

/*
4.21 Value mapping function of operator ~hashvalue~

*/

int
CcHashValue( Word* args, Word& result, int message, Word& local,
           Supplier s)
{
  result = qp->ResultStorage( s );
  if ( ((StandardAttribute*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() &&
       (((CcInt*)args[1].addr)->GetValue() > -1) )
  {
    ((CcInt *)result.addr)->Set( true,
                                ((StandardAttribute*)args[0].addr)->HashValue()
                                 % ((CcInt*)args[1].addr)->GetValue() );
  }
  else
  {
    ((CcInt *)result.addr)->Set( false, 0 );
  }
  return (0);
}

/*
4.22 Value mapping function of operator ~sqrt~

*/

int
CcSqrt( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  if( ((CcReal*)args[0].addr)->IsDefined() )
  {
    const double tmp = ((CcReal*)args[0].addr)->GetRealval();
    if ( tmp >= 0 )
      ((CcReal *)result.addr)-> Set( true, sqrt( tmp ) );
    else
    {
      ((CcReal *)result.addr)-> Set( false, 0 );
    }
  }
  else
  {
    ((CcReal *)result.addr)-> Set( false, 0 );
  }
  return (0);
}


/*
4.15 Computes the Levenshtein distance between two strings.

This distance is defined by the minimum count of operators in

  * add character

  * remove character

  * replace character

to get the __target__ from the __source__.

The complexity is source.length [*] target.length.

*/
static  int ld(const string source, const string target){
  int n = source.length();
  int m = target.length();
  if(n==0){
     return m;
  }
  if(m==0){
     return n;
  }
  n++;
  m++;
  int matrix[n][m];
  // initialize
  for(int i=0;i<n;i++){
    matrix[i][0] = i;
  }
  for(int i=0;i<m;i++){
    matrix[0][i] = i;
  }
  int cost;
  for(int i=1;i<n;i++){
     for(int j=1;j<m;j++){
        cost = source[i-1]==target[j-1]?0:1;
        matrix[i][j] = min(matrix[i-1][j]+1,
                           min(matrix[i][j-1]+1,
                           matrix[i-1][j-1]+cost));
     }
  }
  int res = matrix[n-1][m-1];
  return res;
}


int
DistanceStrStrFun( Word* args, Word& result, int message, Word& local,
                   Supplier s ){
   result = qp->ResultStorage(s);
   CcString* source = (CcString*) args[0].addr;
   CcString* target = (CcString*) args[1].addr;
   string str1 = (const char*)(source->GetStringval());
   string str2 = (const char*)(target->GetStringval());
   int dist = ld(str1,str2);
   ((CcInt*)result.addr)->Set(true,dist);
   return 0;
}


/*
Map any type to a string

*/

ListExpr
CcElapsedTypeMap( ListExpr args )
{
  NList list(args);
  if ( list.hasLength(1) )
   if ( list.first().str() == "typeerror" )
     return list.typeError("elapsedtime: input has a typeerror");

  return (nl->SymbolAtom( "string" ));
}



int
ccelapsedfun(Word* args, Word& result, int message, Word& local, Supplier s)
{
  result = qp->ResultStorage( s );
  CcString* resStr = reinterpret_cast<CcString*>( result.addr );

  StopWatch& elapsedTime = qp->GetTimer();
  elapsedTime.diffTimes();

  stringstream estr;
  estr << elapsedTime.diffSecondsReal()
       << "/" << elapsedTime.diffSecondsCPU();

  // reset timer
  elapsedTime.start();

  resStr->Set(true, (STRING*) estr.str().c_str());

  return 0;
}

/*
4.16 Operator ~setoption~

This operator maps

----   (string x int -> bool)
----

As a side effect internal configuration parameter of SECONDO are changed.

5.12.0 Specification

*/

struct SetOptionInfo : OperatorInfo {

  SetOptionInfo() : OperatorInfo()
  {
    name =      "setoption";
    signature = "string x int -> bool";
    syntax =    "setoption(key, n)";
    meaning =   "Changes the value for key to n. Currently, only "
                "the option \"MaxMemPerOperator\" is avalaible.";
    example =   "setoption(\"MaxMemPerOperator\", 512*1024);";
  }

};


/*
5.12.1 Type mapping

The type mapping uses the wrapper class ~NList~ which hides calls
to class NestedList. Moreover, there are some useful functions for
handling streams of tuples.

*/

static ListExpr setoption_tm(ListExpr args)
{
  NList l(args);

  const string opName = "setoption";
  string err1 = opName + "expects (string int)!";

  if ( !l.checkLength(2, err1) )
    return l.typeError( err1 );

  if ( !l.first().isSymbol(Symbols::STRING()) )
    return l.typeError(err1);

  if ( !l.second().isSymbol(Symbols::INT()) )
    return l.typeError(err1);

  return NList(Symbols::BOOL()).listExpr();
}

int
setoption_vm( Word* args, Word& result, int message, Word& local, Supplier s )
{
  // args[0] : string
  // args[1] : int

  result = qp->ResultStorage( s );
  string key = StdTypes::GetString(args[0]);
  int value = StdTypes::GetInt(args[1]);

  bool found=false;
  if ( key == "MaxMemPerOperator" ) {

    found=true;
    qp->SetMaxMemPerOperator(value);
  }

  if( found )
  {
    ((CcBool *)result.addr)->Set( true, true );
  }
  else
  {
    ((CcBool *)result.addr)->Set( false, false );
  }
  return (0);
}

/*
4.17 Operator ~round~ rounds a real with a given precision

*/
int
CcRoundValueMap( Word* args, Word& result, int message,
                 Word& local, Supplier s )
{
    CcReal* Svalue = (CcReal*) args[0].addr;
    CcInt*  Sprecision = (CcInt*) args[1].addr;
    result = qp->ResultStorage( s );
    CcReal* res = (CcReal*) result.addr;

    if ( !Svalue->IsDefined() || !Sprecision->IsDefined() )
    {
      res->SetDefined( false );
    }
    else
    {
      double value = Svalue->GetRealval();
      int precision = Sprecision->GetIntval();
      static const double base = 10.0;
      double complete5, complete5i;

      complete5 = value * pow(base, (double) (precision + 1));
      if(value < 0.0)
          complete5 -= 5.0;
      else
          complete5 += 5.0;
      complete5 /= base;
      modf(complete5, &complete5i);
      res->Set( true, complete5i / pow(base, (double) precision) );
    }
    return 0;
}

/*
4.18 Operator ~int2real~

*/
int
CcInt2realValueMap( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
    CcInt* arg = (CcInt*) args[0].addr;
    result = qp->ResultStorage( s );
    CcReal* res = (CcReal*) result.addr;

    if ( !arg->IsDefined() )
      res->SetDefined( false );
    else
      res->Set( true, (double) arg->GetIntval() );
    return 0;
}

/*
4.19 Operator ~real2int~

*/
int
CcReal2intValueMap( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
    CcReal* arg = (CcReal*) args[0].addr;
    result = qp->ResultStorage( s );
    CcInt* res = (CcInt*) result.addr;

    if ( !arg->IsDefined() )
      res->SetDefined( false );
    else
    {
      double val = arg->GetRealval();
      int ival = (int) val;
      if( abs((double)ival - val) >2)
      {
          res->Set(false,0);
      } 
      else
      {
          res->Set( true, ival );
      }
    }
    return 0;
}

/*
4.20 Operator ~int2bool~

*/
int
CcInt2boolValueMap( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
    CcInt* arg = (CcInt*) args[0].addr;
    result = qp->ResultStorage( s );
    CcBool* res = (CcBool*) result.addr;

    if ( !arg->IsDefined() )
      res->SetDefined( false );
    else
      res->Set( true, arg->GetIntval() != 0);
    return 0;
}

/*
4.21 Operator ~bool2int~

*/
int
CcBool2intValueMap( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
    CcBool* arg = (CcBool*) args[0].addr;
    result = qp->ResultStorage( s );
    CcInt* res = (CcInt*) result.addr;

    if ( !arg->IsDefined() )
      res->SetDefined( false );
    else
      res->Set( true, arg->GetBoolval() ? 1 : 0);
    return 0;
}

/*
4.22 Operator ~floor~

*/
int
CcFloorValueMap( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
    CcReal* arg = (CcReal*) args[0].addr;
    result = qp->ResultStorage( s );
    CcReal* res = (CcReal*) result.addr;

    if ( !arg->IsDefined() )
      res->SetDefined( false );
    else
      res->Set( true, floor(arg->GetRealval()));
    return 0;
}

/*
4.23 Operator ~ceil~

*/
int
CcCeilValueMap( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
    CcReal* arg = (CcReal*) args[0].addr;
    result = qp->ResultStorage( s );
    CcReal* res = (CcReal*) result.addr;

    if ( !arg->IsDefined() )
      res->SetDefined( false );
    else
      res->Set( true, ceil(arg->GetRealval()));
    return 0;
}

/*
4.24 Operator ~num2string~

*/
template<class T>
int CcNum2String( Word* args, Word& result, int message,
                    Word& local, Supplier s )
{
    T* arg = (T*) args[0].addr;
    result = qp->ResultStorage( s );
    CcString* res = (CcString*) result.addr;

    if ( !arg->IsDefined() )
      res->SetDefined( false );
    else{
      ostringstream os;
      os.precision(47);
      os << arg->GetValue();
      string s = os.str().substr(0,48);
      STRING S;
      strcpy(S,s.c_str());
      res->Set( true, &S);
    }
    return 0;
}

/*
5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do define an array of value
mapping functions for each operator. For nonoverloaded operators there is also
such and array defined, so it easier to make them overloaded.

*/

ValueMapping ccplusmap[] =
        { CcPlus_ii, CcPlus_ir, CcPlus_ri, CcPlus_rr, CcPlus_ss };
ValueMapping ccminusmap[] =
        { CcMinus_ii, CcMinus_ir, CcMinus_ri, CcMinus_rr };
ValueMapping ccproductmap[] =
        { CcProduct_ii, CcProduct_ir, CcProduct_ri, CcProduct_rr };
ValueMapping ccdivisionmap[] =
        { CcDivision_ii, CcDivision_ir, CcDivision_ri, CcDivision_rr };

ValueMapping ccmodmap[] = { CcMod };
ValueMapping ccdivmap[] = { CcDiv };
ValueMapping ccsqrtmap[] = { CcSqrt };

ValueMapping cclessmap[] = { CcLess<CcInt>,
                             CcLess2<CcInt, CcReal>,
                             CcLess2<CcReal, CcInt>,
                             CcLess<CcReal>,
                             CcLess<CcBool>,
                             CcLess<CcString> };

ValueMapping cclessequalmap[] = { CcLessEqual<CcInt>,
                                  CcLessEqual2<CcInt, CcReal>,
                                  CcLessEqual2<CcReal, CcInt>,
                                  CcLessEqual<CcReal>,
                                  CcLessEqual<CcBool>,
                                  CcLessEqual<CcString> };

ValueMapping ccgreatermap[] = { CcGreater<CcInt>,
                                CcGreater2<CcInt, CcReal>,
                                CcGreater2<CcReal, CcInt>,
                                CcGreater<CcReal>,
                                CcGreater<CcBool>,
                                CcGreater<CcString> };

ValueMapping ccgreaterequalmap[] = { CcGreaterEqual<CcInt>,
                                     CcGreaterEqual2<CcInt, CcReal>,
                                     CcGreaterEqual2<CcReal, CcInt>,
                                     CcGreaterEqual<CcReal>,
                                     CcGreaterEqual<CcBool>,
                                     CcGreaterEqual<CcString> };

ValueMapping ccequalmap[] = { CcEqual<CcInt>,
                              CcEqual2<CcInt, CcReal>,
                              CcEqual2<CcReal, CcInt>,
                              CcEqual<CcReal>,
                              CcEqual<CcBool>,
                              CcEqual<CcString> };

ValueMapping ccdiffmap[] = { CcDiff<CcInt>,
                             CcDiff2<CcInt, CcReal>,
                             CcDiff2<CcReal, CcInt>,
                             CcDiff<CcReal>,
                             CcDiff<CcBool>,
                             CcDiff<CcString> };

ValueMapping ccstartsmap[] = { StartsFun };

ValueMapping cccontainsmap[] = { ContainsFun };

ValueMapping ccandmap[] = { AndFun };
ValueMapping ccormap[] = { OrFun };
ValueMapping ccnotmap[] = { NotFun };

ValueMapping ccisemptymap[] = { IsEmpty<CcBool>,
                                IsEmpty<CcInt>,
                                IsEmpty<CcReal>,
                                IsEmpty<CcString> };

ValueMapping ccsetintersectionmap[] =
        { CcSetIntersection_ii, CcSetIntersection_rr, CcSetIntersection_bb,
        CcSetIntersection_ss };

ValueMapping ccsetminusmap[] =
        { CcSetMinus_ii, CcSetMinus_rr, CcSetMinus_bb, CcSetMinus_ss };

ValueMapping ccoprelcountmap[] = { RelcountFun };
ValueMapping ccoprelcountmap2[] = { RelcountFun2 };
ValueMapping cckeywordsmap[] = { keywordsFun };
ValueMapping ccifthenelsemap[] = { ifthenelseFun };

ValueMapping ccbetweenmap[] = { CcBetween<CcInt>, CcBetween<CcReal>,
                                CcBetween<CcString>, CcBetween<CcBool> };

//ValueMapping cchashvaluemap[] = { CcHashValue<CcInt>, CcHashValue<CcReal>,
                                //CcHashValue<CcString>, CcHashValue<CcBool> };

ValueMapping cchashvaluemap[] = { CcHashValue };

ValueMapping ccroundvaluemap[] = { CcRoundValueMap };

ValueMapping ccint2realvaluemap[] = { CcInt2realValueMap };
ValueMapping ccreal2intvaluemap[] = { CcReal2intValueMap };
ValueMapping ccint2boolvaluemap[] = { CcInt2boolValueMap };
ValueMapping ccbool2intvaluemap[] = { CcBool2intValueMap };
ValueMapping ccfloorvaluemap[] = { CcFloorValueMap };
ValueMapping ccceilvaluemap[] = { CcCeilValueMap };

ValueMapping ccnum2stringvaluemap[] = 
{ CcNum2String<CcReal>, 
  CcNum2String<CcInt> 
};

const string CCSpecAdd  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(int int) -> int, (int real) -> real, "
                          "(real int)"
                          " -> real, (real real) -> real "
                                "(string string) -> string</text--->"
                           "<text>_ + _</text--->"
                           "<text>Addition. Strings are concatenated.</text--->"
                           "<text>query -1.2 + 7</text--->"
                              ") )";

const string CCSpecSub  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(int int) -> int, (int real) -> real, "
                          "(real int) -> real, (real real) -> real</text--->"
                               "<text>_ - _</text--->"
                               "<text>Subtraction</text--->"
                               "<text>query -.2 - 4</text--->"
                              ") )";

const string CCSpecMul  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(int int) -> int, (int real) -> real, "
                          "(real int) -> real, (real real) -> real</text--->"
                               "<text>_ * _</text--->"
                               "<text>Multiplication.</text--->"
                               "<text>query 5 * 1.4 </text--->"
                              ") )";

const string CCSpecDiv  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(int int) -> real, (int real) -> real, "
                          "(real int) -> real, (real real) -> real</text--->"
                               "<text>_ / _</text--->"
                               "<text>Division.</text--->"
                               "<text>query 5 / 2 </text--->"
                              ") )";

const string CCSpecMod  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(int int) -> int</text--->"
                               "<text>_ mod _</text--->"
                               "<text>Modulo.</text--->"
                               "<text>query 8 mod 3 </text--->"
                              ") )";

const string CCSpecDiv2  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" )"
                             "( <text>(int int) -> int</text--->"
                               "<text>_ div _</text--->"
                               "<text>Integer Division.</text--->"
                               "<text>query 5 div 2 </text--->"
                              ") )";

const string CCSpecSqrt  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                           "\"Example\" )"
                             "( <text>real -> real</text--->"
                               "<text>sqrt( _ )</text--->"
                               "<text>Extract a root.</text--->"
                               "<text>query sqrt(2.1)</text--->"
                              ") )";

const string CCSpecRandInt  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text>int -> int </text--->"
                           "<text>randint ( _ )</text--->"
                           "<text>Returns a random integer between 0 and "
                        "arg - 1, the argument must be at least 2 otherwise "
                        "it is set to 2. Calling randint(n) for "
                        "n < 2 initializes the "
                        "random number generator with a seed value depending "
                        "on the current time. </text--->"
                               "<text>query randint (9)</text--->"
                              ") )";


const string CCSpecRandSeed = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text>int -> bool </text--->"
                           "<text>randseed ( _ )</text--->"
                           "<text>initializes the randon number generator with "
                           "a defined value. </text--->"
                               "<text>query randseed(8000)</text--->"
                              ") )";

const string CCSpecMaxRand  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text> -> int </text--->"
                               "<text>randmax()</text--->"
                               "<text>Returns the value of MAX_RAND </text--->"
                               "<text>query randmax()</text--->"
                              ") )";


const string CCSpecInitSeq  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text>int -> bool </text--->"
                        "<text>seqinit ( _ ) </text--->"
                        "<text>Returns true and sets the start value "
                        " of the sequence to the argument value</text--->"
                        "<text>query seqinit(100)</text--->"
                              ") )";

const string CCSpecNextSeq  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text> -> int </text--->"
                "<text>seqnext ()</text--->"
                "<text>Returns s+n-1 at the n-th call when the sequence"
                " was initialized with initseq (s) "
                "otherwise s defaults to 0.</text--->"
                         "<text>query seqnext ()</text--->"
                              ") )";

const string CCSpecLog  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>int -> int </text--->"
                               "<text>log ( _ )</text--->"
                               "<text>Computes the logarithmus of base 2."
                               " The argument must be greater"
                               "than 0.</text--->"
                               "<text>query log (256)</text--->"
                              ") )";

const string CCSpecLT  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
                         "(real int) -> bool, (real real) -> bool, (bool bool)"
                         " -> bool, (string string) -> bool</text--->"
                               "<text>_ < _</text--->"
                               "<text>Less.</text--->"
                               "<text>query \"house\" < \"hotel\"</text--->"
                              ") )";

const string CCSpecLE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
                         "(real int) -> bool, (real real) -> bool, (bool bool)"
                         " -> bool, (string string) -> bool</text--->"
                               "<text>_ <= _</text--->"
                               "<text>Less or equal.</text--->"
                               "<text>query 8.2 <= 8.2</text--->"
                              ") )";

const string CCSpecGT  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
                         "(real int) -> bool, (real real) -> bool, (bool bool)"
                         " -> bool, (string string) -> bool</text--->"
                               "<text>_ > _</text--->"
                               "<text>Greater.</text--->"
                               "<text>query 3 > 4.1</text--->"
                              ") )";

const string CCSpecGE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
                         "(real int) -> bool, (real real) -> bool, (bool bool)"
                         " -> bool, (string string) -> bool</text--->"
                               "<text>_ >= _</text--->"
                               "<text>Greater or equal.</text--->"
                               "<text>query 3 >= 5</text--->"
                              ") )";

const string CCSpecEQ  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
                         "(real int) -> bool, (real real) -> bool, (bool bool)"
                         " -> bool, (string string) -> bool</text--->"
                               "<text>_ = _</text--->"
                               "<text>Equal.</text--->"
                               "<text>query 2.1 = 2.01</text--->"
                              ") )";

const string CCSpecNE  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                         "( <text>(int int) -> bool, (int real) -> bool, "
                         "(real int) -> bool, (real real) -> bool, (bool bool)"
                         " -> bool, (string string) -> bool</text--->"
                               "<text>_ # _</text--->"
                               "<text>Not equal.</text--->"
                               "<text>query 2.1 # 2.01</text--->"
                              ") )";

const string CCSpecBeg  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(string string) -> bool</text--->"
                               "<text>_ starts _</text--->"
                               "<text>Starts.</text--->"
                               "<text>query \"starts\" starts \"st\"</text--->"
                              ") )";

const string CCSpecCon  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>(string string) -> bool</text--->"
                               "<text>_ contains _</text--->"
                               "<text>Contains.</text--->"
                            "<text>query \"contains\" contains \"tai\""
                            "</text--->"
                              ") )";

const string CCSpecNot  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                             "( <text>bool -> bool</text--->"
                               "<text>not ( _ )</text--->"
                               "<text>Logical Not.</text--->"
                               "<text>query not ( 4=4 )</text--->"
                             " ) )";

const string CCSpecAnd  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                          "\"Example\" )"
                          "( <text>(bool bool) -> bool</text--->"
                               "<text>_ and _</text--->"
                               "<text>Logical And.</text--->"
                               "<text>query (8 = 8) and (3 < 4)</text--->"
                              ") )";

const string CCSpecOr  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                         "\"Example\" )"
                             "( <text>(bool bool) -> bool</text--->"
                               "<text>_ or _</text--->"
                               "<text>Logical Or.</text--->"
                               "<text>query (3 <= 4) or (\"hotel\" > "
                               "\"house\")"
                               "</text--->"
                               ") )";

const string CCSpecIsEmpty  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                              "\"Example\" )"
                             "( <text>bool -> bool, int -> bool, real -> bool,"
                             " string -> bool</text--->"
                             "<text>isempty ( _ )</text--->"
                             "<text>Returns whether the value is defined or "
                             "not.</text--->"
                               "<text>query isempty ( 8 )</text--->"
                              ") )";

const string CCSpecUpper  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>string -> string</text--->"
                               "<text>upper ( _ )</text--->"
                               "<text>Returns a string immediately upper to"
                               " the original one.</text--->"
                               "<text>query upper ( \"hello\" )</text--->"
                               ") )";

const string CCSpecSetIntersection  = "( ( \"Signature\" \"Meaning\" )"
                "( <text> (int int) -> int, (real real) -> real,"
                "(bool bool) -> bool, (string string) -> string</text--->"
                "<text> Set intersection. </text--->"
                        ") )";

const string CCSpecSetMinus  = "( ( \"Signature\" \"Meaning\" )"
                "( <text> (int int) -> int, (real real) -> real, "
                "(bool bool) -> bool, (string string) -> string</text--->"
                "<text> Set minus. </text--->"
                ") )";

const string CCSpecRelcount  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                        "\"Example\" )"
                        "( <text>string -> int</text--->"
                        "<text>_ relcount</text--->"
                        "<text>Counts the number of tuples of a relation, "
                        "which is specified by its objectname"
                        " of type string.</text--->"
                        "<text>query \"Staedte\" relcount</text--->"
                        ") )";

const string CCSpecRelcount2  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>string -> int</text--->"
             "<text>_ relcount2</text--->"
             "<text>Counts the number of tuples of a relation, "
             "which is specified by its objectname"
             " of type string. Uses static method ExecuteQuery"
             " from class QueryProcessor.</text--->"
             "<text>query \"Orte\" relcount2</text--->"
             ") )";

const string CCSpecKeywords  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>(string) -> (stream string)</text--->"
             "<text>_ keywords</text--->"
             "<text>Creates a stream of strings containing the single words"
             " of the origin string, on the assumption, that words in a string"
             " are separated by a space character.</text--->"
             "<text>query ten feed extendstream(name: mystring keywords) "
                "consume</text--->"
             ") )";

const string CCSpecIfthenelse  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>(bool x T x T) ->  T</text--->"
             "<text>ifthenelse(_, _, _)</text--->"
             "<text>Returns the second argument, if the boolean value "
             "expression, given"
             " as a first argument, can be evaluated to true."
             " If not, the operator returns the third argument."
             " NOTE: The second and the third argument must be of the "
                "same type T"
             " of kind DATA.</text--->"
             "<text>query ifthenelse(3 < 5,[const string value \"less\"],"
                "[const string value \"greater\"])</text--->"
             ") )";

const string CCSpecBetween  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>(T x T x T) ->  bool</text--->"
             "<text>_ between[_, _]</text--->"
             "<text>Returns true, if the first argument is in the range "
                "of the second"
             " and third argument, otherwise false. "
                "T can be of type int, real, "
                "string or bool."
             " NOTE: The second argument must be less or equal than the "
                "third argument.</text--->"
             "<text>query 5 between [3, 8], query \"house\" "
                "between [\"ha\", \"hu\"]</text--->"
             ") )";

const string specListHeader =
        "( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )";
const string ST = "<text>";
const string ET = "</text--->";

const string
CCSpecSubStr = "(" + specListHeader + "("
                   + "'(string x int x int) ->  string.'"
                   + "'substr(s, p, q)'"
                   + "'Returns the part of a string s between the"
                     + "position parameters p and q. Positions start at 0.'"
                   + "'query substr(\"test\",2,3)'))";

const string
CCSpecElapsed = "(" + specListHeader + "("
                    + "'(any type) ->  string.'"
                    + "'_ elapsedtime'"
                    + "'Returns the elpased time and the "
                      + "cpu time in seconds encoded in a string'"
                    + "'query plz feed elapsedtime'))";

const string CCLDistSpec  =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" " "\"Example\" )"
            "( <text>string x string  -> int</text--->"
               "<text>ldistance ( _ _ )</text--->"
               "<text>compute the distance between two strings </text--->"
               "<text>query ldistance( \"hello\" \"world\" )</text--->"
                               ") )";

const string CCHashValueSpec  =
            "( ( \"Signature\" \"Syntax\" \"Meaning\" " "\"Example\" )"
            "( <text>T in DATA, y in int, T x y -> int</text--->"
               "<text>hashvalue ( _, _ )</text--->"
               "<text>computes the hashvalue from object of type T, "
               "assuming that hashtable has size y.</text--->"
               "<text>query hashvalue( \"Test\", 9997 )</text--->"
                               ") )";

const string CCRoundSpec =
             "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
             "( <text>(real x int) -> real</text--->"
             "<text>round(_, _)</text--->"
             "<text>Rounds a real value with a precision of n decimals."
             "</text--->"
             "<text>query round(10.7367, 3)</text--->"
             ") )";

const string CCint2realSpec =
             "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
             "( <text>int -> real</text--->"
             "<text>int2real( _ )</text--->"
             "<text>Creates a real value from an integer value using "
             "C++ casting logic.</text--->"
             "<text>query int2real(12)</text--->"
             ") )";

const string CCreal2intSpec =
             "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
             "( <text>real -> int</text--->"
             "<text>real2int( _ )</text--->"
             "<text>Creates an int value from a real value</text--->"
             "<text>query real2int(12.345)</text--->"
             ") )";

const string CCint2boolSpec =
             "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
             "( <text>int -> bool</text--->"
             "<text>int2bool( _ )</text--->"
             "<text>Creates a bool value from an integer value. '0' will be "
             "translated to 'FALSE', all other values to 'TRUE'</text--->"
             "<text>query int2bool(0)</text--->"
             ") )";

const string CCbool2intSpec =
             "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
             "( <text>bool -> int</text--->"
             "<text>bool2int( _ )</text--->"
             "<text>Creates an inteer value from a bool value. 'TRUE' will be "
             "translated to '1', 'FALSE' to '0'</text--->"
             "<text>query bool2int(TRUE)</text--->"
             ") )";

const string CCfloorSpec =
             "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
             "( <text>real -> real</text--->"
             "<text>floor( _ )</text--->"
             "<text>Returns the largest integer not greater than the "
             "argument.</text--->"
             "<text>query floor(12.345)</text--->"
             ") )";

const string CCceilSpec =
             "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
             "( <text>real -> real</text--->"
             "<text>ceil( _ )</text--->"
             "<text>Returns the smallest integer not less than the "
             "argument.</text--->"
             "<text>query ceil(12.345)</text--->"
             ") )";

const string CCnum2stringSpec =
             "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
             "( <text>real -> string\n int -> string</text--->"
             "<text>num2string( num )</text--->"
             "<text>Returns the string representation of numeric argument "
             "'num'.</text--->"
             "<text>query num2string(12.345)</text--->"
             ") )";


Operator ccplus( "+", CCSpecAdd, 5, ccplusmap,
                 CcMathSelectCompute, CcMathTypeMap );

Operator ccminus( "-", CCSpecSub, 4, ccminusmap,
                  CcMathSelectCompute, CcMathTypeMap );

Operator ccproduct( "*", CCSpecMul, 4, ccproductmap,
                    CcMathSelectCompute, CcMathTypeMap );

Operator ccdivision( "/", CCSpecDiv, 4, ccdivisionmap,
                     CcMathSelectCompute, CcMathTypeMapdiv );

Operator ccmod( "mod", CCSpecMod, 1, ccmodmap,
                Operator::SimpleSelect, CcMathTypeMap1 );

Operator ccdiv( "div", CCSpecDiv2, 1, ccdivmap,
                Operator::SimpleSelect, CcMathTypeMap1 );

Operator ccsqrt( "sqrt", CCSpecSqrt, 1, ccsqrtmap,
                Operator::SimpleSelect, RealReal );

Operator ccrandint( "randint", CCSpecRandInt, RandInt,
                    Operator::SimpleSelect, IntInt );

Operator ccrandseed( "randseed", CCSpecRandSeed, RandSeed,
                    Operator::SimpleSelect, IntBool );

Operator ccrandmax( "randmax", CCSpecMaxRand, MaxRand,
                    Operator::SimpleSelect, EmptyInt );

Operator ccseqinit( "seqinit", CCSpecInitSeq, InitSeq,
                    Operator::SimpleSelect, IntBool );

Operator ccseqnext( "seqnext", CCSpecNextSeq, NextSeq,
                    Operator::SimpleSelect, EmptyInt );

Operator cclog( "log", CCSpecLog, LogFun,
                Operator::SimpleSelect, IntInt );

Operator ccless( "<", CCSpecLT, 6, cclessmap,
                 CcMathSelectCompare, CcMathTypeMapBool );

Operator cclessequal( "<=", CCSpecLE, 6, cclessequalmap,
                      CcMathSelectCompare, CcMathTypeMapBool );

Operator ccgreater( ">", CCSpecGT, 6, ccgreatermap,
                    CcMathSelectCompare, CcMathTypeMapBool );

Operator ccgreaterequal( ">=", CCSpecGE, 6, ccgreaterequalmap,
                         CcMathSelectCompare, CcMathTypeMapBool );

Operator ccequal( "=", CCSpecEQ, 6, ccequalmap,
                  CcMathSelectCompare, CcMathTypeMapBool );

Operator ccdiff( "#", CCSpecNE, 6, ccdiffmap,
                 CcMathSelectCompare, CcMathTypeMapBool );

Operator ccstarts( "starts", CCSpecBeg, 1, ccstartsmap,
                   Operator::SimpleSelect, CcMathTypeMapBool3 );

Operator cccontains( "contains", CCSpecCon, 1, cccontainsmap,
                     Operator::SimpleSelect, CcMathTypeMapBool3 );

Operator ccsubstr( "substr", CCSpecSubStr, SubStrFun,
                   Operator::SimpleSelect, SubStrTypeMap );

Operator ccnot( "not", CCSpecNot, 1, ccnotmap,
                Operator::SimpleSelect, CcMathTypeMapBool1 );

Operator ccand( "and", CCSpecAnd, 1, ccandmap,
                Operator::SimpleSelect, CcMathTypeMapBool2 );

Operator ccor( "or", CCSpecOr, 1, ccormap,
               Operator::SimpleSelect, CcMathTypeMapBool2 );

Operator ccisempty( "isempty", CCSpecIsEmpty, 4, ccisemptymap,
                    CcMathSelectIsEmpty, CcMathTypeMapBool4 );

Operator ccuper( "upper", CCSpecUpper, UpperFun,
                 Operator::SimpleSelect, CcStringMapCcString );

Operator ccsetintersection( "intersection", CCSpecSetIntersection, 4,
                ccsetintersectionmap, CcMathSelectSet, CcMathTypeMap2 );

Operator ccsetminus( "minus", CCSpecSetMinus, 4, ccsetminusmap,
                     CcMathSelectSet, CcMathTypeMap2 );

Operator ccoprelcount( "relcount", CCSpecRelcount, 1, ccoprelcountmap,
                       Operator::SimpleSelect, CcStringMapCcInt );

Operator ccoprelcount2( "relcount2", CCSpecRelcount2, 1, ccoprelcountmap2,
                        Operator::SimpleSelect, CcStringMapCcInt );

Operator ccopkeywords( "keywords", CCSpecKeywords, 1, cckeywordsmap,
                       Operator::SimpleSelect, keywordsType );

Operator ccopifthenelse( "ifthenelse", CCSpecIfthenelse, 1, ccifthenelsemap,
                         Operator::SimpleSelect, ifthenelseType );

Operator ccbetween( "between", CCSpecBetween, 4, ccbetweenmap,
                    CcBetweenSelect, CcBetweenTypeMap );

Operator ccelapsedtime( "elapsedtime", CCSpecElapsed, ccelapsedfun,
                        Operator::SimpleSelect, CcElapsedTypeMap );

Operator ccldistance( "ldistance", CCLDistSpec, DistanceStrStrFun,
                 Operator::SimpleSelect, CcLDistTypeMap);

//Operator cchashvalue( "hashvalue", CCHashValueSpec, 4, cchashvaluemap,
                 //CcHashValueSelect, CcHashValueTypeMap);

Operator cchashvalue( "hashvalue", CCHashValueSpec, 1, cchashvaluemap,
                 Operator::SimpleSelect, CcHashValueTypeMap);

Operator ccround( "round", CCRoundSpec, 1, ccroundvaluemap,
                 Operator::SimpleSelect, CcRoundTypeMap);

Operator ccint2real( "int2real", CCint2realSpec, 1, ccint2realvaluemap,
                 Operator::SimpleSelect, IntReal);

Operator ccreal2int( "real2int", CCreal2intSpec, 1, ccreal2intvaluemap,
                 Operator::SimpleSelect, RealInt);

Operator ccint2bool( "int2bool", CCint2boolSpec, 1, ccint2boolvaluemap,
                 Operator::SimpleSelect, IntBool);

Operator ccbool2int( "bool2int", CCbool2intSpec, 1, ccbool2intvaluemap,
                 Operator::SimpleSelect, BoolInt);

Operator ccceil( "ceil", CCceilSpec, 1, ccceilvaluemap,
                 Operator::SimpleSelect, RealReal);

Operator ccfloor( "floor", CCfloorSpec, 1, ccfloorvaluemap,
                 Operator::SimpleSelect, RealReal);

Operator ccnum2string( "num2string", CCnum2stringSpec, 2, ccnum2stringvaluemap,
                 ccnum2stringSelect, NumStringTypeMap);

/*
6 Class ~CcAlgebra~

The last steps in adding an algebra to the Secondo system are

  * Associating value mapping functions with their operators

  * ``Bunching'' all
type constructors and operators in one instance of class ~Algebra~.

Therefore, a new subclass ~CcAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~ccalgebra1~ is defined.

*/

class CcAlgebra1 : public Algebra
{
 public:
  CcAlgebra1() : Algebra()
  {
    AddTypeConstructor( &ccInt );
    AddTypeConstructor( &ccReal );
    AddTypeConstructor( &ccBool );
    AddTypeConstructor( &ccString );

    ccInt.AssociateKind( "DATA" );
    ccReal.AssociateKind( "DATA" );
    ccBool.AssociateKind( "DATA" );
    ccString.AssociateKind( "DATA" );

    ccInt.AssociateKind( "BASE" );
    ccReal.AssociateKind( "BASE" );
    ccBool.AssociateKind( "BASE" );
    ccString.AssociateKind( "BASE" );

    AddOperator( &ccplus );
    AddOperator( &ccminus );
    AddOperator( &ccproduct );
    AddOperator( &ccdivision );
    AddOperator( &ccmod );
    AddOperator( &ccdiv );
    AddOperator( &ccsqrt );
    AddOperator( &ccrandint );
    AddOperator( &ccrandseed );
    AddOperator( &ccrandmax );
    AddOperator( &ccseqinit );
    AddOperator( &ccseqnext );
    AddOperator( &cclog );
    AddOperator( &ccless );
    AddOperator( &cclessequal );
    AddOperator( &ccgreater );
    AddOperator( &ccgreaterequal );
    AddOperator( &ccequal );
    AddOperator( &ccdiff );
    AddOperator( &ccstarts );
    AddOperator( &cccontains );
    AddOperator( &ccsubstr );
    AddOperator( &ccnot );
    AddOperator( &ccand );
    AddOperator( &ccor );
    AddOperator( &ccisempty );
    AddOperator( &ccuper );
    AddOperator( &ccsetintersection );
    AddOperator( &ccsetminus );
    AddOperator( &ccoprelcount );
    AddOperator( &ccoprelcount2 );
    AddOperator( &ccopkeywords );
    AddOperator( &ccopifthenelse );
    AddOperator( &ccbetween );
    //AddOperator( &ccelapsedtime );
    AddOperator( &ccldistance );
    AddOperator( &cchashvalue );
    AddOperator( &ccround );
    AddOperator( &ccint2real );
    AddOperator( &ccreal2int );
    AddOperator( &ccint2bool );
    AddOperator( &ccbool2int );
    AddOperator( &ccceil );
    AddOperator( &ccfloor );
    static SetOptionInfo setoption_oi;
    static Operator setoption_op( setoption_oi,
                                  setoption_vm,
                                  setoption_tm );
    AddOperator(&setoption_op);
    AddOperator( &ccnum2string );



  }
  ~CcAlgebra1() {};

};

CcAlgebra1 ccalgebra1;

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
InitializeStandardAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&ccalgebra1);
}

/*
8 Query examples

Now we are ready to
compile this algebra implementation and link the resulting object
file with the Secondo library to a Secondo executable containing our
example algebra.

Here are some example queries:

  * "(list operators)"[1]

    Among others, the operators ~+~, ~-~, ~[*]~, and ~/~ are listed.

  * "(list type constructors)"[1]

    ~int~ , ~real~, ~bool~and ~string~ are part of the output.


  * "(query (+ 4 5))"[1]

    Result value 9 is returned.

  * "(query ([*] (+ 4 5.7)(- 2.5 4)))"[1]

    Result value -14.55 is returned.

  * "(query (or ( $ < $  4 5.7)( $ > $  2.5 4)))"[1]

    Result value TRUE is returned.

*/

void
ShowStandardTypesStatistics( const bool reset )
{
  Counter::getRef("STD:INT_created") = CcInt::intsCreated;
  Counter::getRef("STD:INT_deleted") = CcInt::intsDeleted;
  Counter::getRef("STD:REAL_created") = CcReal::realsCreated;
  Counter::getRef("STD:REAL_deleted") = CcReal::realsDeleted;
  Counter::getRef("STD:BOOL_created") = CcBool::boolsCreated;
  Counter::getRef("STD:BOOL_deleted") = CcBool::boolsDeleted;
  Counter::getRef("STD:STRING_created") = CcString::stringsCreated;
  Counter::getRef("STD:STRING_deleted") = CcString::stringsDeleted;

  if( reset )
  {
    CcInt::intsCreated = 0; CcInt::intsDeleted = 0;
    CcReal::realsCreated = 0; CcReal::realsDeleted = 0;
    CcBool::boolsCreated = 0; CcBool::boolsDeleted = 0;
    CcString::stringsCreated = 0; CcString::stringsDeleted = 0;
  }
}


int
StdTypes::GetInt(const Word& w) {
   return Attribute::GetValue<CcInt, int>(w);
}


REAL
StdTypes::GetReal(const Word& w) {
   return Attribute::GetValue<CcReal, REAL>(w);
}


bool
StdTypes::GetBool(const Word& w) {
   return Attribute::GetValue<CcBool, bool>(w);
}

string
StdTypes::GetString(const Word& w) {
   return Attribute::GetValue<CcString, string>(w);
}
