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

November 9, 2002. RHG Added operators ~randint~ and ~log~. Some other slight
revisions.

February 2004. F. Hoffmann added operators ~relcount~ and ~relcount2~.

April 28, 2004. M. Spiekermann added operators ~nextint~ and ~randmax~. The
calculation of random numbers in an specified range was revised according to the
recommendations documented in the rand() manpage.

July 08, 2004. M. Spiekermann changed the IN-function of data type ~real~.
Integer atoms are now also accepted.

November 2004. M. Spiekermann. Some small functions were moved to the header
file in order to implement them as inline functions.

November 2004. M. Spiekermann. Implementation of the operator ~substr~.
Moreover, the
add operator was overloaded once more in order to concatenate strings.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006, M. Spiekermann new operator ~elapsedtime~ implemented.

May 2006, M. Spiekermann new operator ~setoption~ implemented.

May 11, 2006, M. Spiekermann. Most of the value mappings are replaced by
template
functions using the generic ~Compare~ function. This reduces the code a lot
(about 400 lines of code) and there are still some functions left which may
be replaced by template implementations.

December 06, 2006 C. Duentgen added operators int2real, real2int, int2bool,
bool2int, ceil, floor.7

May 2007, M. Spiekermann. New operator abs and introduction of generic type
mappings.

\begin{center}
\footnotesize
\tableofcontents
\end{center}


1 Overview

In  this algebra the standard types and functions are defined. Standard types
are ~int~, ~real~, ~bool~ and ~string~. These types  are represented
In Classes of C++ whith a boolean flag which shows whether this value is defined
or not (e.g it could be undefined because it is the result of a divivion by
zero).


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
sequences of numbers starting by arg. The n-th call of seqnext will return
arg+n-1

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

#undef TRACE_ON
#include "Trace.h"

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SecondoSystem.h" //operator queries
#include "Counter.h"
#include "StopWatch.h"
#include "NList.h"
#include "RelationAlgebra.h"
#include "GenOps.h"
#include "ListUtils.h"
#include "AlmostEqual.h"
#include "Progress.h"
#include "Symbols.h"
#include "Stream.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include <math.h>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <cerrno>
#include <time.h>       //needed for random number generator
#include "LongInt.h"
#include "RationalAttr.h"


extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;


/*
4.1 Type investigation auxiliaries

For a standard type T, its type name string is available using static class
function const string T::BasicType().

For type mappings some auxiliary helper functions are defined in the
file "TypeMapUtils.h" which defines a namespace mappings.

*/

#include "TypeMapUtils.h"
#include "Symbols.h"

using namespace std;
using namespace mappings;


 void trimstring(string& str) {
     string ws = " \t";
     string::size_type pos = str.find_last_not_of(ws);
     if(pos != string::npos) {
       str.erase(pos + 1);
       pos = str.find_first_not_of(ws);
       if(pos != string::npos){
         str.erase(0, pos);
       }
     } else {
       str.erase(str.begin(), str.end());
     }
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
value of type ~INT~. It consists of a boolean flag, ~defined~,
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
                             nl->StringAtom(CcInt::BasicType()),
                             nl->StringAtom("(<intvalue>)"),
                             nl->StringAtom("12 or -32 or 0"))));
}

/*

Now we define a function, ~OutInt~, that takes as inputs a type description
and a pointer to a value of this type. The representation of this value in
nested list format is returned.

For the simple types int, real, string, bool we don't use the type description
at all. We will need it in the case of more complex type constructors,
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
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
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
    && listutils::isSymbolUndefined(value) )
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

ListExpr CcInt::Out(const ListExpr typeInfo, Word value)
{
  return OutCcInt(typeInfo, value);
}

Word CcInt::In(ListExpr typeInfo, ListExpr value,
           int errorPos, ListExpr& errorInfo, bool& correct)
{
  return InCcInt(typeInfo, value, errorPos, errorInfo, correct);
}

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
3.2.5 {\em Cast}-function of type constructor {\tt INT}

*/

void*
CastInt( void* addr )
{
  return (new (addr) CcInt);
}

/*
3.2.6 {\em Type check} function of type constructor {\tt INT}

*/

bool
CheckInt( ListExpr type, ListExpr& errorInfo )
{
  return (listutils::isSymbol( type, CcInt::BasicType() ));
}

/*
A type constructor is created by constructing an instance of
class ~TypeConstructor~. Constructor arguments are the type constructor's name
and the four functions previously defined.

*/
TypeConstructor ccInt( CcInt::BasicType(),            CcIntProperty,
                       OutCcInt,         InCcInt,
                       0,                0,
                       CreateCcInt,      DeleteCcInt,
                       OpenAttribute<CcInt>,
                       SaveAttribute<CcInt>,  // object open and save
                       CloseCcInt, CloneCcInt,
                       CastInt,          SizeOfCcInt, CheckInt );

/*
3.2 Type constructor *ccreal*

The following type constructor, ~REAL~, is defined in the same way as
~INT~.

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
                             nl->StringAtom(CcReal::BasicType()),
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
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  }
}

Word
InCcReal( ListExpr typeInfo, ListExpr value,
          int errorPos, ListExpr& errorInfo, bool& correct )
{
  if(listutils::isSymbolUndefined(value)){ // undefined
    correct=true;
    return SetWord(new CcReal(false,0.0));
  }
  if(listutils::isNumeric(value)){
    correct=true;
    return SetWord(new CcReal(true, listutils::getNumValue(value)));
  }
  correct = false;
  return (SetWord( Address( 0 ) ));
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
3.3.6 {\em Cast}-function of type constructor {\tt REAL}

*/

void*
CastReal( void* addr )
{
  return new (addr) CcReal;
}


/*
3.3.7 {\em Type check} function of type constructor {\tt REAL}

*/

bool
CheckReal( ListExpr type, ListExpr& errorInfo )
{
  return (listutils::isSymbol( type, CcReal::BasicType() ));
}


TypeConstructor ccReal( CcReal::BasicType(),       CcRealProperty,
                        OutCcReal,    InCcReal,
                        0,            0,
                        CreateCcReal, DeleteCcReal,
                        OpenAttribute<CcReal>,
                        SaveAttribute<CcReal>,  // object open and save
                        CloseCcReal, CloneCcReal,
                        CastReal,   SizeOfCcReal, CheckReal );

/*
3.3 Type constructor *ccbool*

Each instance of below defined class CcBool will be the main memory
representation of a
value of type ~BOOL~. It consists of a boolean flag, ~defined~,
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
                             nl->StringAtom(CcBool::BasicType()),
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
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
  }
}

/*
The function ~InBool~ provides a functionality complementary to ~OutBool~:
A pointer to a value's main memory representation is returned. It is calculated
by taking the value's nested list representation and its type description as
input parameters. Again, we don't need the type information in this example due
to simplicity of types used.

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
    && listutils::isSymbolUndefined(value) )
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
3.3.6 {\em Cast}-function of type constructor {\tt REAL}

*/

void*
CastBool( void* addr )
{
  return (new (addr) CcBool);
}

/*
3.2.5 {\em Type check} function of type constructor {\tt REAL}

*/

bool
CheckBool( ListExpr type, ListExpr& errorInfo )
{
  return (listutils::isSymbol( type, CcBool::BasicType() ));
}

TypeConstructor ccBool( CcBool::BasicType(),             CcBoolProperty,
                        OutCcBool,          InCcBool,
                        0,                  0,
                        CreateCcBool,       DeleteCcBool,
                        OpenAttribute<CcBool>,
                        SaveAttribute<CcBool>,  // object open and save
                        CloseCcBool,        CloneCcBool,
                        CastBool,           SizeOfCcBool,  CheckBool );

/*
3.5 Type constructor *CcString*

*/
long CcString::stringsCreated = 0;
long CcString::stringsDeleted = 0;

bool CcString::Adjacent( const Attribute* arg ) const
{
  const STRING_T* a = GetStringval();
  const STRING_T* b = ((CcString *)arg)->GetStringval();

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
                             nl->StringAtom(CcString::BasicType()),
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
    return (nl->SymbolAtom(Symbol::UNDEFINED()));
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
    CcString* cs = new CcString( true, (STRING_T*)s.c_str() );
    //cs->ShowMem();
    return SetWord( cs );
  }
  else if ( nl->IsAtom( value ) && nl->AtomType( value ) == SymbolType
        && listutils::isSymbolUndefined(value) )
  {
    correct = true;
    return (SetWord( new CcString( false, (STRING_T*)"" ) ));
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
  return (SetWord( new CcString( false, (STRING_T*)&p ) ));
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
3.3.6 {\em Cast}-function of type constructor {\tt REAL}

*/

void*
CastString( void* addr )
{
  return (new (addr) CcString);
}

/*
3.2.5 {\em Type check} function of type constructor {\tt REAL}

*/
bool
CheckString( ListExpr type, ListExpr& errorInfo )
{
  return (listutils::isSymbol( type, CcString::BasicType() ));
}

TypeConstructor ccString( CcString::BasicType(),       CcStringProperty,
                          OutCcString,    InCcString,
                          0,              0,
                          CreateCcString, DeleteCcString,
                          OpenAttribute<CcString>,
                          SaveAttribute<CcString>,  // object open and save
                          CloseCcString,  CloneCcString,
                          CastString,     SizeOfCcString, CheckString );



GenTC<LongInt> longint;
GenTC<Rational> rational;


/*
4 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

4.2 Type Mapping

A type mapping function takes a nested list as argument. Its contents are
type descriptions of an operator's input parameters. A nested list describing
the output type of the operator is returned.

4.2.1 Type mapping function CcMathTypeMap

The function below is used for the operators +, - and [*]. For ~simple~
type mappings - those which map a list of atomic types to an atomic result
type - a generic function called ~SimpleMap~ or ~SimpleMaps~ can be used.

*/

const string maps_arith[12][3] =
{
  {CcInt::BasicType(),    CcInt::BasicType(),    CcInt::BasicType()},
  {CcInt::BasicType(),    CcReal::BasicType(),   CcReal::BasicType()},
  {CcReal::BasicType(),   CcInt::BasicType(),    CcReal::BasicType()},
  {CcReal::BasicType(),   CcReal::BasicType(),   CcReal::BasicType()},

  {LongInt::BasicType(),  CcInt::BasicType(),      LongInt::BasicType()},
  {LongInt::BasicType(),  LongInt::BasicType(),  LongInt::BasicType()},
  {CcInt::BasicType(),    LongInt::BasicType(),  LongInt::BasicType()},
  {Rational::BasicType(), CcInt::BasicType(),    Rational::BasicType()},
  {Rational::BasicType(), LongInt::BasicType(),  Rational::BasicType()},
  {Rational::BasicType(), Rational::BasicType(), Rational::BasicType()},
  {CcInt::BasicType(),    Rational::BasicType(), Rational::BasicType()},
  {LongInt::BasicType(),  Rational::BasicType(), Rational::BasicType()},
};

ListExpr
CcMathTypeMap( ListExpr args )
{
  return SimpleMaps<12,3>(maps_arith, args);
}

const string maps_plus[13][3] =
{
  {CcInt::BasicType(),    CcInt::BasicType(),    CcInt::BasicType()},
  {CcInt::BasicType(),    CcReal::BasicType(),   CcReal::BasicType()},
  {CcReal::BasicType(),   CcInt::BasicType(),    CcReal::BasicType()},
  {CcReal::BasicType(),   CcReal::BasicType(),   CcReal::BasicType()},
  {CcString::BasicType(), CcString::BasicType(), CcString::BasicType()},
  
  {LongInt::BasicType(),   CcInt::BasicType(),    LongInt::BasicType()},
  {LongInt::BasicType(),  LongInt::BasicType(),  LongInt::BasicType()},
  {CcInt::BasicType(),    LongInt::BasicType(),  LongInt::BasicType()},

  {Rational::BasicType(), CcInt::BasicType(),    Rational::BasicType()},
  {Rational::BasicType(), LongInt::BasicType(),  Rational::BasicType()},
  {Rational::BasicType(), Rational::BasicType(), Rational::BasicType()},
  {CcInt::BasicType(),    Rational::BasicType(), Rational::BasicType()},
  {LongInt::BasicType(),  Rational::BasicType(), Rational::BasicType()},
};

ListExpr
CcPlusTypeMap( ListExpr args )
{
  return SimpleMaps<13,3>(maps_plus, args);
}

/*
4.2.2 Selection Functions

A selection function is quite similar to a type mapping function and is needed
for operators which accept different variants of input parameters. The only
difference is that it doesn't return a type but the index of a value mapping
function being able to deal with the respective combination of input parameter
types.

Note that a selection function does not need to check the correctness of
argument types; it has already been checked by the type mapping function that it
is applied to correct arguments.

4.3.1 Selection function  CcMathSelectCompute

*/

int
CcMathSelectCompute( ListExpr args )
{
  return SimpleSelect<12,3>(maps_arith, args);
}


int
CcPlusSelectCompute( ListExpr args )
{
  return SimpleSelect<13,3>(maps_plus, args);
}

int ifthenelseSelect(ListExpr args){
   if(listutils::isStream(nl->Second(args))){
       return 1;
   } else {
       return 0;
   }
}


/*
4.3.2 Selecttion Function for int | real

*/
int CcNumRealSelect( ListExpr args ){
  return listutils::isSymbol(nl->First(args),CcInt::BasicType()) ? 0 : 1;
}


/*
4.3.3 Selecttion Function for {int | real} x {int | real}

*/
int CcNumNumRealSelect( ListExpr args ){
  if(listutils::isSymbol(nl->First(args),CcInt::BasicType())){
    return listutils::isSymbol(nl->Second(args),CcInt::BasicType()) ? 0 : 1;
  } else {
    return listutils::isSymbol(nl->Second(args),CcInt::BasicType()) ? 2 : 3;
  }
}

/*
4.3.4 Selection Function for {int | real} x {int | real} [x {int | real}] [x T]

*/
int CcNumNumNumRealSelect( ListExpr args ){
  int res = 0;
  int noargs = nl->ListLength( args );
  if(listutils::isSymbol(nl->First(args),CcReal::BasicType())) { res+=4; }
  if(listutils::isSymbol(nl->Second(args),CcReal::BasicType())) { res+=2; }
  if(    (noargs > 2)
      && listutils::isSymbol(nl->Third(args),CcReal::BasicType())){ res +=1; }
  return res;
}


/*
4.2.2 Type mapping function CcMathTypeMapdiv

It is for the operators /. the only difference between CCMathTypeMap and
CcMathTypeMapdiv is that the latter give as resulttype REAL if the input type
is INT ("normal division of int with result real", the other division
of int is called div in this program).

*/


const string maps_div[12][3] =
{
  {CcInt::BasicType(),  CcInt::BasicType(),  CcReal::BasicType()},
  {CcInt::BasicType(),  CcReal::BasicType(), CcReal::BasicType()},
  {CcReal::BasicType(), CcInt::BasicType(),  CcReal::BasicType()},
  {CcReal::BasicType(), CcReal::BasicType(), CcReal::BasicType()},
  {LongInt::BasicType(),  CcInt::BasicType(),      LongInt::BasicType()},
  {LongInt::BasicType(),  LongInt::BasicType(),  LongInt::BasicType()},
  {CcInt::BasicType(),    LongInt::BasicType(),  LongInt::BasicType()},
  {Rational::BasicType(), CcInt::BasicType(),    Rational::BasicType()},
  {Rational::BasicType(), LongInt::BasicType(),  Rational::BasicType()},
  {Rational::BasicType(), Rational::BasicType(), Rational::BasicType()},
  {CcInt::BasicType(),    Rational::BasicType(), Rational::BasicType()},
  {LongInt::BasicType(),  Rational::BasicType(), Rational::BasicType()},
};

ListExpr
CcMathTypeMapdiv( ListExpr args )
{
  return SimpleMaps<12,3>(maps_div, args);
}

/*
4.2.3 Type mapping function CcMathTypeMap1

It is for the operators mod and div which have ~int~ as input and ~int~ as
result.

*/

ListExpr
CcMathTypeMap1( ListExpr args )
{
  const string mapping[] =
              {CcInt::BasicType(), CcInt::BasicType(), CcInt::BasicType()};
  return SimpleMap(mapping, 3, args);
}

/*
4.2.4 Type mapping function CcMathTypeMap2

It is for the operators ~intersection~ and ~minus~.

*/


const string maps_set[4][3] =
{
  {CcInt::BasicType(), CcInt::BasicType(), CcInt::BasicType()},
  {CcReal::BasicType(), CcReal::BasicType(), CcReal::BasicType()},
  {CcBool::BasicType(), CcBool::BasicType(), CcBool::BasicType()},
  {CcString::BasicType(), CcString::BasicType(), CcString::BasicType()}
};

ListExpr
CcMathTypeMap2( ListExpr args )
{
  return SimpleMaps<4,3>(maps_set, args);
}

int
CcMathSelectSet( ListExpr args )
{
  return SimpleSelect<4,3>(maps_set, args);
}

/*
4.2.4 Type mapping functions IntInt, IntBool, BoolInt, EmptyInt, IntString

Used for operators ~randint~, ~randmax~, ~initseq~, ~nextseq~ and ~log~.
And for ~int2bool~, ~bool2int~, ~char~.

*/

ListExpr
IntInt( ListExpr args )
{
  const string mapping[] = {CcInt::BasicType(), CcInt::BasicType()};
  return SimpleMap(mapping, 2, args);
}

ListExpr
RealReal( ListExpr args )
{
  const string mapping[] = {CcReal::BasicType(), CcReal::BasicType()};
  return SimpleMap(mapping, 2, args);
}

ListExpr
RealInt( ListExpr args )
{
  const string mapping[] = {CcReal::BasicType(), CcInt::BasicType()};
  return SimpleMap(mapping, 2, args);
}

ListExpr
IntReal( ListExpr args )
{
  const string mapping[] = {CcInt::BasicType(), CcReal::BasicType()};
  return SimpleMap(mapping, 2, args);
}

ListExpr
IntBool( ListExpr args )
{
  const string mapping[] = {CcInt::BasicType(), CcBool::BasicType()};
  return SimpleMap(mapping, 2, args);
}

/*
~EmptyInt~

Some operators create integer result without any kind of input, they map

----
  () -> (int)
----

*/

ListExpr
EmptyInt( ListExpr args )
{
  const string mapping[] = {CcInt::BasicType()};
  return SimpleMap(mapping, 1, args);
}

ListExpr
BoolInt( ListExpr args )
{
  const string mapping[] = {CcBool::BasicType(), CcInt::BasicType()};
  return SimpleMap(mapping, 2, args);
}

ListExpr
IntString( ListExpr args )
{
  const string mapping[] = {CcInt::BasicType(), CcString::BasicType()};
  return SimpleMap(mapping, 2, args);
}

/*
4.2.5 Type mapping function CompareTypeMap

It is for the Compare operators which have ~bool~ as resulttype.

*/

const string maps_comp[11][3] =
{
  {CcInt::BasicType(),    CcInt::BasicType(),    CcBool::BasicType()},
  {CcInt::BasicType(),    CcReal::BasicType(),   CcBool::BasicType()},
  {CcInt::BasicType(),    LongInt::BasicType(),   CcBool::BasicType()},

  {CcReal::BasicType(),   CcInt::BasicType(),    CcBool::BasicType()},
  {CcReal::BasicType(),   CcReal::BasicType(),   CcBool::BasicType()},
  {CcReal::BasicType(),   LongInt::BasicType(),   CcBool::BasicType()},

  {LongInt::BasicType(),   CcInt::BasicType(),    CcBool::BasicType()},
  {LongInt::BasicType(),   CcReal::BasicType(),   CcBool::BasicType()},
  {LongInt::BasicType(),   LongInt::BasicType(),   CcBool::BasicType()},

  {CcBool::BasicType(),   CcBool::BasicType(),   CcBool::BasicType()},
  {CcString::BasicType(), CcString::BasicType(), CcBool::BasicType()},
 
};


ListExpr
CompareTypeMap( ListExpr args )
{
  return SimpleMaps<11,3>(maps_comp, args);
}

/*
4.3.3 Selection function  CcMathSelectCompare

*/

int
CcMathSelectCompare( ListExpr args )
{
  return SimpleSelect<11,3>(maps_comp, args);
}


/*
4.2.6 Type mapping function CcMathTypeMapBool1

It is for the  operator ~not~ which have ~bool~ as input and resulttype.

*/

ListExpr
CcMathTypeMapBool1( ListExpr args )
{
  const string mapping[] = {CcBool::BasicType(), CcBool::BasicType()};
  return SimpleMap(mapping, 2, args);
}

/*
4.2.7 Type mapping function CcMathTypeMapBool2

It is for the  operators and and or  which have bool as input and resulttype.

*/

ListExpr
CcMathTypeMapBool2( ListExpr args )
{
  const string mapping[] =
            {CcBool::BasicType(), CcBool::BasicType(), CcBool::BasicType()};
  return SimpleMap(mapping, 3, args);
}

/*
4.2.8 Type mapping function CcMathTypeMapBool3

It is for the  operators ~starts~ and ~contains~  which have ~string~ as input
and ~bool~ resulttype.

*/

ListExpr
CcMathTypeMapBool3( ListExpr args )
{
  const string mapping[] =
          {CcString::BasicType(), CcString::BasicType(), CcBool::BasicType()};
  return SimpleMap(mapping, 3, args);
}

/*
4.2.9 Type mapping function CcMathTypeMapBool4

It is for the  operators ~isempty~ which have ~bool~, ~int~, ~real~, and
~string~ as input and ~bool~ resulttype.

*/


const string maps_isempty[4][2] =
{
  {CcBool::BasicType(),   CcBool::BasicType()},
  {CcInt::BasicType(),    CcBool::BasicType()},
  {CcReal::BasicType(),   CcBool::BasicType()},
  {CcString::BasicType(), CcBool::BasicType()}
};

ListExpr
CcMathTypeMapBool4( ListExpr args )
{
  return SimpleMaps<4,2>(maps_isempty, args);
}


/*
4.3.3 Selection function  CcMathSelectIsEmpty

It is used for the ~isempty~ operator.

*/
int
CcMathSelectIsEmpty( ListExpr args )
{
  return SimpleSelect<4,2>(maps_isempty, args);
}


/*
4.2.10 Type mapping function for the ~++~ operator:

string ---> string.

*/

ListExpr
CcStringMapCcString( ListExpr args )
{
  const string mapping[] = {CcString::BasicType(), CcString::BasicType()};
  return SimpleMap(mapping, 2, args);
}

/*
4.2.10 Type mapping function for the ~substr~ operator:

string x int x int -> string.

*/

ListExpr
SubStrTypeMap( ListExpr args )
{
  const string mapping[] =  { CcString::BasicType(), CcInt::BasicType(),
                              CcInt::BasicType(), CcString::BasicType() };
  return SimpleMap(mapping, 4, args);
}


/*
4.2.11 Type mapping function for the ~relcount~ operator:

string ---> int.

*/

static ListExpr
CcStringMapCcInt( ListExpr args )
{
  const string mapping[] = {CcString::BasicType(), CcInt::BasicType()};
  return SimpleMap(mapping, 2, args);
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
    if ( listutils::isSymbol(arg, CcString::BasicType()) )
      return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
        nl->SymbolAtom(CcString::BasicType()));
  }
  return nl->SymbolAtom(Symbol::TYPEERROR());
}

/*
4.2.13 Type mapping function for the ~ifthenelse~ operator:

Type mapping for ~ifthenelse~ is

----    (bool x T x T)) -> T , T in DATA or T = stream(...)
----

*/
ListExpr ifthenelseType(ListExpr args)
{
  ListExpr arg1, arg2, arg3;


  if(!nl->HasLength(args,3)){
    return listutils::typeError("Expected three arguments.");
  }

  arg1 = nl->First( args );
  arg2 = nl->Second( args );
  arg3 = nl->Third( args );

  if(!listutils::isSymbol(arg1, CcBool::BasicType())){
    return listutils::typeError("The 1st argument must be of type bool.");
  }

  if(!listutils::isDATA(arg2) && !listutils::isStream(arg2)){
    return listutils::typeError("The 2nd argument must be "
                                "in kind DATA or must be a stream.");
  }

  if(!nl->Equal(arg2,arg3)){
    return listutils::typeError("The 2nd and 3rd argument "
                                "must be of the same type.");
  }
  return arg2;
}

/*
4.2.14 Type mapping function for the ~ifthenelse2~ operator:

Type mapping for ~ifthenelse2~ is

----    (bool x T x T)) -> T
----

*/
ListExpr ifthenelse2Type(ListExpr args)
{
  ListExpr arg1, arg2, arg3;

  if ( nl->ListLength( args ) == 3 )
  {
    arg1 = nl->First( args );
    arg2 = nl->Second( args );
    arg3 = nl->Third( args );

    if (nl->Equal(arg2, arg3) && listutils::isSymbol(arg1,CcBool::BasicType()))
    {
      if ( IsRelDescription(arg2) || IsRelDescription(arg2, true) )
        return arg2;
    }
  }
  ErrorReporter::ReportError("Incorrect input for operator ifthenelse.");
  return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}




/*
4.2.14 Type mapping function for the ~between~ operator:

Type mapping for ~between~ is

----    (T x T x T) -> bool
----

*/

const string maps_between[4][4] =
{
  {CcInt::BasicType(),    CcInt::BasicType(),
                                    CcInt::BasicType(),    CcBool::BasicType()},
  {CcReal::BasicType(),   CcReal::BasicType(),
                                    CcReal::BasicType(),   CcBool::BasicType()},
  {CcString::BasicType(), CcString::BasicType(),
                                    CcString::BasicType(), CcBool::BasicType()},
  {CcBool::BasicType(),   CcBool::BasicType(),
                                    CcBool::BasicType(),   CcBool::BasicType()},
};

ListExpr
CcBetweenTypeMap( ListExpr args )
{
  return SimpleMaps<4,4>(maps_between, args);
}

/*
4.3.3 Selection function  CcBetweenSelect

*/
int
CcBetweenSelect( ListExpr args )
{
  return SimpleSelect<4,4>(maps_between, args);
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
  if(nl->ListLength(args)!=2){
    ErrorReporter::ReportError("DATA x int expected");
    return nl->TypeError();
  }
  ListExpr arg1 = nl->First(args);
  ListExpr arg2 = nl->Second(args);

  if(!listutils::isDATA(arg1) ||
     !listutils::isSymbol(arg2,CcInt::BasicType())){
    ErrorReporter::ReportError("DATA x int expected");
    return nl->TypeError();
  }

  return (nl->SymbolAtom( CcInt::BasicType() ));
}

/*
4.2.15 Type mapping function for the ~ldistance~ operator:

Type mapping for ~ldistance~ is string x string [->] int


*/

ListExpr CcLDistTypeMap(ListExpr args)
{
  const string mapping[] =
          {CcString::BasicType(), CcString::BasicType(), CcInt::BasicType()};
  return SimpleMap(mapping, 3, args);
}

/*
4.2.16 Type mapping function CcRoundTypeMap

It is for the  operator ~round~.

*/

ListExpr
CcRoundTypeMap( ListExpr args )
{
  const string map[] =
              { CcReal::BasicType(), CcInt::BasicType(), CcReal::BasicType() };
  return SimpleMap(map, 3, args);
}

/*
4.2.17 Type mappig function ~NumStringTypeMap~

For operator ~num2string~

*/

const string maps_num2str[3][2] =
{
  {CcReal::BasicType(), CcString::BasicType()},
  {CcInt::BasicType(),  CcString::BasicType()},
  {LongInt::BasicType(),  CcString::BasicType()}
};

ListExpr NumStringTypeMap(ListExpr args) {
  if (nl->ListLength(args) == 1) {
    if (CcInt::checkType(nl->First(args)) || CcReal::checkType(nl->First(args))
     || LongInt::checkType(nl->First(args))) {
      return nl->SymbolAtom(CcString::BasicType());
    }
  }
  if (nl->ListLength(args) == 2) {
    if ((CcInt::checkType(nl->First(args)) || CcReal::checkType(nl->First(args))
         || LongInt::checkType(nl->First(args))) 
         && CcInt::checkType(nl->Second(args))) {
      return nl->SymbolAtom(CcString::BasicType());
    }
  }
  return NList::typeError("Expecting either int|real|longint or "
                          "int|real|longint x int .");
}

int ccnum2stringSelect(ListExpr args) {
  if (nl->ListLength(args) == 1) {
    if (CcReal::checkType(nl->First(args)))  return 0;
    if (CcInt::checkType(nl->First(args)))   return 1;
    if (LongInt::checkType(nl->First(args))) return 2;
  }
  if (CcReal::checkType(nl->First(args)))  return 3;
  if (CcInt::checkType(nl->First(args)))   return 4;
  if (LongInt::checkType(nl->First(args))) return 5;
  return -1;
}

/*
4.2.16 Type mapping function ~DATAbool~

Maps DATA [->] bool
It is for the  operator ~isdefined~.

*/

ListExpr DATAbool( ListExpr args )
{
  NList mArgs(args);
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  if(    !mArgs.hasLength(1)
      || !SecondoSystem::GetAlgebraManager()->
          CheckKind(Kind::DATA(), mArgs.first().listExpr(), errorInfo)
    )
  {
    return NList::typeError("Expected single argument of kind DATA.");
  }
  return NList(CcBool::BasicType()).listExpr();
}






/*
4.2.16 Type mapping function ~CcTypeMapTinDATA\_TinDATAint~

Maps

----  T x T --> bool, T in kind DATA
----

It is, e.g. for the  operator ~compare~.

*/

ListExpr CcTypeMapTinDATA_TinDATAint( ListExpr args )
{
  NList mArgs(args);
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  if(   !mArgs.hasLength(2)
     || ( mArgs.first() != mArgs.second() )
     || !SecondoSystem::GetAlgebraManager()->
          CheckKind(Kind::DATA(), mArgs.first().listExpr(), errorInfo)
    )
  {
    return NList::typeError("Expected T x T for T in kind DATA.");
  }
  return NList(CcInt::BasicType()).listExpr();
}

/*
4.2.17 Type mapping function ~CcTypeMapTinDATAexpplus2TinDATA~

For operators ~getMinVal~, ~getMaxVal~:

---- T [x T x T x ...] -> T, T in DATA
----

*/

ListExpr CcTypeMapTinDATAexpplus2TinDATA( ListExpr args )
{
  NList mArgs(args);
  ListExpr errorInfo = nl->OneElemList(nl->SymbolAtom("ERROR"));
  int noArgs = mArgs.length();
  if(noArgs < 1){
    return NList::typeError("Expected T^n, T in kind DATA, n >= 1.");
  }
  NList type = mArgs.first();
  if (!SecondoSystem::GetAlgebraManager()->
         CheckKind(Kind::DATA(), type.listExpr(), errorInfo)){
      return NList::typeError("Expected T^n, T in kind DATA, n >= 1.");
  }

  ListExpr first = nl->First(args);
  ListExpr rest = nl->Rest(args);

  while(!nl->IsEmpty(rest)){
    if(!nl->Equal(first, nl->First(rest))){
      return NList::typeError("Expected T^n, T in kind DATA, n >= 1.");
    }
    rest = nl->Rest(rest);
  }

  return type.listExpr();
}


/*
4.2.18 Type mapping function for: int | real -> real

*/
ListExpr CcTypeMapNumReal( ListExpr args ){
  if(!nl->HasLength(args, 1)){
    return listutils::typeError("Expected real or int (only 1 argument!).");
  }
  ListExpr arg = nl->First(args);
  if(listutils::isNumericType(arg)){
    return nl->SymbolAtom(CcReal::BasicType());
  }
  return listutils::typeError("Expected real or int.");
}

/*
4.2.18 Type mapping function for: int | real -> real

*/
ListExpr CcTypeMapEmptyReal( ListExpr args ){
  if(!nl->HasLength(args,0)){
    return listutils::typeError("Expected no argument.");
  }
  return nl->SymbolAtom(CcReal::BasicType());
}

/*
4.2.19 Type mapping function for: {int | real} x {int | real} -> real

*/
ListExpr CcTypeMapNumNumReal( ListExpr args ){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("Expected {int | real} x {int | real}");
  }
  if(    listutils::isNumericType(nl->First(args))
      && listutils::isNumericType(nl->Second(args))){
    return nl->SymbolAtom(CcReal::BasicType());
  }
  return listutils::typeError("Expected {int | real} x {int | real}");
}

/*
4.2.19 Type mapping function for: 
{int|real} x {int|real} [ x {int|real} ] [ x bool ]->real

Appends a bool == FALSE, if the optional bool parameter is not present.

*/
ListExpr CcTypeMapNumNumOptNumOptBoolReal( ListExpr args ){
  string errmsg = "Expected {int | real} x {int | real} [ x {int | real} ] "
                  "[ x bool ]";
  int noargs = nl->ListLength(args);
  if( (noargs<2) || (noargs>4)){
    return listutils::typeError(errmsg);
  }
  //check first and second
  if(!listutils::isNumericType(nl->First(args)) ||
     !listutils::isNumericType(nl->Second(args)) ){
    return listutils::typeError(errmsg);
  }
  if( noargs== 2 ) { // numeric x numeric: OK, but APPEND bool parameter
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList(nl->BoolAtom(false)),
                             nl->SymbolAtom(CcReal::BasicType()));
  }
  // check third and fourth
  if( noargs==3 ){
    if( listutils::isSymbol(nl->Third(args),CcBool::BasicType()) ) {
      // mumeric x numeric x bool: OK.
      return nl->SymbolAtom(CcReal::BasicType());
    }
    if( !listutils::isNumericType(nl->Third(args)) ) {
      return listutils::typeError(errmsg);
    } // mumeric x numeric x numeric: OK, but APPEND bool parameter
    return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()),
                             nl->OneElemList(nl->BoolAtom(false)),
                             nl->SymbolAtom(CcReal::BasicType()));
  }
  if( noargs==4 ){
    if( !listutils::isNumericType(nl->Third(args)) ) {
      return listutils::typeError(errmsg);
    }
    if( !listutils::isSymbol(nl->Fourth(args),CcBool::BasicType()) ) {
      return listutils::typeError(errmsg);
    }
    return nl->SymbolAtom(CcReal::BasicType()); // mun x num x num x bool: OK.
  }
  // should not be reached:
  return listutils::typeError(errmsg);
}

/*
4.2.20 Type mapping function for: real x  real -> real

*/
ListExpr CcTypeMapRealRealReal( ListExpr args ){
  if(!nl->HasLength(args,2)){
    return listutils::typeError("Expected real x real");
  }
  if(    listutils::isSymbol(nl->First(args), CcReal::BasicType())
      && listutils::isSymbol(nl->Second(args), CcReal::BasicType())){
    return nl->SymbolAtom(CcReal::BasicType());
  }
  return listutils::typeError("Expected real x real");
}
/*
4.4 Value mapping functions of operator ~+~

A value mapping function implements an operator's main functionality: it takes
input arguments and computes the result. Each operator consists of at least
one value mapping function. In the case of overloaded operators --- like in
this example --- there are several value mapping functions, one for each
possible combination of input parameter types. We have to provide
four functions for each of the operators ~+~, ~-~,  ~[*]~ ....., since
each of them accepts four input parameter combinations: ~INT~ $\times$
~INT~, ~INT~ $\times$ ~REAL~,  ~REAL~ $\times$ ~INT~, and
~REAL~ $\times$ ~REAL~.

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
    wres->Set( true,
         (STRING_T*)(str1 + str2).substr(0,MAX_STRINGSIZE).c_str() );
  }
  else
  {
    STRING_T str = "";
    wres->Set( false, &str );
  }
  return (0);
}

template<class T>
int CcPlus_long( Word* args, Word& result, 
                 int message, Word& local, Supplier s )
{
   LongInt* arg1 = (LongInt*) args[0].addr;
   T* arg2 = (T*) args[1].addr;
   result = qp->ResultStorage(s);
   LongInt* res = (LongInt*) result.addr;
   (*res) = (*arg1) + (*arg2);
   return 0;
}

template<class T>
int CcPlus_long2( Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
   T* arg1 = (T*) args[0].addr;
   LongInt* arg2 = (LongInt*) args[1].addr;
   result = qp->ResultStorage(s);
   LongInt* res = (LongInt*) result.addr;
   (*res) = (*arg2) + (*arg1);
   return 0;
}

template<class T>
int CcPlus_rat( Word* args, Word& result, int message, Word& local, Supplier s )
{
   Rational* arg1 = (Rational*) args[0].addr;
   T* arg2 = (T*) args[1].addr;
   Rational r2 (*arg2);
   result = qp->ResultStorage(s);
   Rational* res = (Rational*) result.addr;
   (*res) = (*arg1) + r2;
   return 0;
}

template<class T>
int CcPlus_rat2( Word* args, Word& result, 
                 int message, Word& local, Supplier s )
{
   T* arg1 = (T*) args[0].addr;
   Rational* arg2 = (Rational*) args[1].addr;
   Rational r1 (*arg1);
   result = qp->ResultStorage(s);
   Rational* res = (Rational*) result.addr;
   (*res) = (*arg2) + r1;
   return 0;
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

template<class T>
int CcMinus_long( Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
   LongInt* arg1 = (LongInt*) args[0].addr;
   T* arg2 = (T*) args[1].addr;
   result = qp->ResultStorage(s);
   LongInt* res = (LongInt*) result.addr;
   LongInt t(*arg2);
   (*res) = (*arg1) - t;
   return 0;
}

template<class T>
int CcMinus_long2(Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
  T* arg1 = (T*) args[0].addr;
  LongInt* arg2 = (LongInt*) args[1].addr;
  LongInt t(*arg1);
  result = qp->ResultStorage(s);
  LongInt* res = (LongInt*) result.addr;
  (*res) = t - (*arg2);
  return 0;
}

template<class T>
int CcMinus_rat( Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
   Rational* arg1 = (Rational*) args[0].addr;
   T* arg2 = (T*) args[1].addr;
   result = qp->ResultStorage(s);
   Rational* res = (Rational*) result.addr;
   Rational t(*arg2);
   (*res) = (*arg1) - t;
   return 0;
}

template<class T>
int CcMinus_rat2(Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
  T* arg1 = (T*) args[0].addr;
  Rational* arg2 = (Rational*) args[1].addr;
  Rational t(*arg1);
  result = qp->ResultStorage(s);
  Rational* res = (Rational*) result.addr;
  (*res) = t - (*arg2);
  return 0;
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

template<class T>
int CcProduct_long( Word* args, Word& result, int message, 
                    Word& local, Supplier s ){
  LongInt* arg1 = (LongInt*) args[0].addr;
  T* arg2 = (T*) args[1].addr;
  result = qp->ResultStorage(s);
  LongInt* res = (LongInt*) result.addr;
  LongInt t(*arg2);
  (*res) = (*arg1) * t;
  return 0;
}

template<class T>
int CcProduct_long2( Word* args, Word& result, int message, 
                    Word& local, Supplier s ){
  T* arg1 = (T*) args[0].addr;
  LongInt* arg2 = (LongInt*) args[1].addr;
  result = qp->ResultStorage(s);
  LongInt* res = (LongInt*) result.addr;
  LongInt t(*arg1);
  (*res) = t * (*arg2);
  return 0;
}

template<class T>
int CcProduct_rat( Word* args, Word& result, int message, 
                    Word& local, Supplier s ){
  Rational* arg1 = (Rational*) args[0].addr;
  T* arg2 = (T*) args[1].addr;
  result = qp->ResultStorage(s);
  Rational* res = (Rational*) result.addr;
  Rational f2(*arg2);
  (*res) = (*arg1)*f2; 
  return 0;
}

template<class T>
int CcProduct_rat2( Word* args, Word& result, int message, 
                    Word& local, Supplier s ){
  T* arg1 = (T*) args[0].addr;
  Rational* arg2 = (Rational*) args[1].addr;
  result = qp->ResultStorage(s);
  Rational* res = (Rational*) result.addr;
  Rational t(*arg1);
  (*res) = t * (*arg2);
  return 0;
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
      Set( true, ((SEC_STD_REAL )((CcInt*)args[0].addr)->GetIntval()) /
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

template<class T>
int CcDivision_long( Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
  LongInt* arg1 = (LongInt*) args[0].addr;
  T* arg2 = (T*) args[1].addr;
  result = qp->ResultStorage(s);
  LongInt* res = (LongInt*) result.addr;
  LongInt t(*arg2);
  (*res) = (*arg1) / t;
  return 0; 
}


template<class T>
int CcDivision_long2( Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
  T* arg1 = (T*) args[0].addr;
  LongInt* arg2 = (LongInt*) args[1].addr;
  result = qp->ResultStorage(s);
  LongInt* res = (LongInt*) result.addr;
  LongInt t(*arg1);
  (*res) = t / (*arg2);
  return 0; 
}


template<class T>
int CcDivision_rat( Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
  Rational* arg1 = (Rational*) args[0].addr;
  T* arg2 = (T*) args[1].addr;
  result = qp->ResultStorage(s);
  Rational* res = (Rational*) result.addr;
  Rational t(*arg2);
  (*res) = (*arg1) / t;
  return 0; 
}


template<class T>
int CcDivision_rat2( Word* args, Word& result, 
                  int message, Word& local, Supplier s )
{
  T* arg1 = (T*) args[0].addr;
  Rational* arg2 = (Rational*) args[1].addr;
  result = qp->ResultStorage(s);
  Rational* res = (Rational*) result.addr;
  Rational t(*arg1);
  (*res) = t / (*arg2);
  return 0; 
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
4.8 Value mapping function of operators ~randint~, ~maxrand~, ~initseq~ and
~nextseq~

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
  int strlen = str1.length();

  // compute result value
  if (    wstr->IsDefined()
       && wpos1->IsDefined()
       && wpos2->IsDefined()
       && (p2 >= p1)
       && (p1 >= 1)
       && (p1 <= strlen) )
  {
    int n = min( static_cast<long unsigned int>(p2-p1),
                 static_cast<long unsigned int>(strlen-p1) );
    wres->Set( true, (STRING_T*)(str1.substr(p1-1, n+1).c_str()) );
  }
  else
  {
    STRING_T str = "";
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
4.16 Value mapping functions of operators ~and~ and ~andS~

For ~and~ the arguments are evaluated lazy. The result is TRUE, iff both 
arguments are DEFINED and TRUE, otherwise the result is FALSE. The result 
is always DEFINED.

~andS~ has strict evaluation. If both arguments are DEFINED and TRUE, 
it returns TRUE.  Otherwise if both arguments are DEFINED and at least 
one is FALSE, it returns FALSE.
In all other cases, it returns UNDEFINED.

*/

int
AndFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  Word res;
  result = qp->ResultStorage( s );

  qp->Request(args[0].addr, res);
  if ( !((CcBool*)res.addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  if (! ((CcBool*)res.addr)->GetBoolval() )
  {
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }

  qp->Request(args[1].addr, res);
  if ( !((CcBool*)res.addr)->IsDefined() )
  {
    ((CcBool*)result.addr)->Set(true,false);
    return 0;
  }
  if ( ((CcBool*)res.addr)->GetBoolval() )
    ((CcBool*)result.addr)->Set(true,true);
  else
    ((CcBool*)result.addr)->Set(true,false);
  return 0;
}

int
    AndSFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( !((CcBool*)args[0].addr)->IsDefined() ||
       !((CcBool*)args[1].addr)->IsDefined()
     )
    ((CcBool*)result.addr)->Set(false,false);
  else if( ((CcBool*)args[0].addr)->GetBoolval() &&
           ((CcBool*)args[1].addr)->GetBoolval()
         )
    ((CcBool*)result.addr)->Set(true,true);
  else
    ((CcBool*)result.addr)->Set(true,false);
  return 0;
}


/*
4.17 Value mapping functions of operators ~or~ and ~orS~

For ~or~ the arguments are evaluated lazy. The result is TRUE, if at least one
arguments is DEFINED and TRUE.
Otherwise, it returns FALSE.

~orS~ has strict evaluation. If both arguments are DEFINED and at least one is
TRUE, it returns TRUE.
Otherwise if both arguments are DEFINED and both are FALSE, it returns FALSE.
In all other cases, it returns UNDEFINED.

*/

int
OrFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  Word res;
  result = qp->ResultStorage( s );

  qp->Request(args[0].addr, res);
  if ( ((CcBool*)res.addr)->IsDefined() && ((CcBool*)res.addr)->GetBoolval() )
  {
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }

  qp->Request(args[1].addr, res);
  if ( ((CcBool*)res.addr)->IsDefined() && ((CcBool*)res.addr)->GetBoolval() )
  {
    ((CcBool*)result.addr)->Set(true,true);
    return 0;
  }
  ((CcBool*)result.addr)->Set(true,false);
  return 0;
}

int
OrSFun( Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );

  if ( !((CcBool*)args[0].addr)->IsDefined() ||
       !((CcBool*)args[1].addr)->IsDefined()
     )
    ((CcBool*)result.addr)->Set(false,false);
  else if( ((CcBool*)args[0].addr)->GetBoolval() ||
             ((CcBool*)args[1].addr)->GetBoolval()
         )
    ((CcBool*)result.addr)->Set(true,true);
  else
    ((CcBool*)result.addr)->Set(true,false);
  return 0;
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
4.18 Value mapping functions of operator ~++~

*/

int
PlusPlusFun( Word* args, Word& result, int message, Word& local, Supplier s )
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
      ((CcString *)result.addr)->Set( true, (STRING_T *)&newStr );
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
  STRING_T nullStr = "";
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
      STRING_T nullStr = "";
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
        Word tmpresult;
        qpp->EvalS( tree, tmpresult, 1 );

        // create the result list ( type, value )
        valueList = SecondoSystem::GetCatalog()->
          OutObject( resultType, tmpresult );
        resultList = nl->TwoElemList( resultType, valueList );

        // set the result value and destroy the operator tree
        ((CcInt *)result.addr)->Set ( true,
          nl->IntValue(nl->Second(resultList)) );
        qpp->Destroy( tree, false );
        if(tmpresult.addr){
            ((CcInt*)tmpresult.addr)->DeleteIfAllowed();
        }
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
  string errorMessage = "";

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
      ((CcInt*)resultword.addr)->DeleteIfAllowed();
    }
    else cout << "Error in executing operator query: "<< errorMessage << endl;
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
  struct Subword {int start, nochr, strlength; STRING_T* subw;}* subword;

  CcString* elem, *str;
  //CcString* str;
  int i;
  string teststr, tmpstr;
  STRING_T outstr;
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

      subword->subw = (STRING_T*)malloc(strlen(*str->GetStringval()) + 1);
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
      if(local.addr)
      {
        subword = ((Subword*) local.addr);
        free(subword->subw);
        delete subword;
        local.setAddr(0);
      }
      return 0;
  }
  /* should not happen */
  return -1;
}

int
ifthenelseDataFun(Word* args, Word& result,
                 int message, Word& local, Supplier s)
{
    if(message==OPEN || message==REQUEST){
      result = qp->ResultStorage( s );

      Word res;

      qp->Request(args[0].addr, res);
      CcBool* arg1 = (CcBool*) res.addr;

      if ( !arg1->IsDefined() )
      {
        ((Attribute*)result.addr)->SetDefined( false );
        return 0;
      }

      int index = ( arg1->GetBoolval() ? 1 : 2 );
      qp->Request(args[index].addr, res);

      ((Attribute*)result.addr)->CopyFrom(
                  (Attribute*) res.addr );
      return 0;
  } else {
      return CANCEL;
  }
}



int
ifthenelseStreamFun(Word* args, Word& result,
                    int message, Word& local, Supplier s)
{

  switch(message){

    case OPEN:{
          if(local.addr){
              delete  (int*) local.addr;
              local.addr = 0;
          }
          qp->Request(args[0].addr, result);
          CcBool* arg1 = (CcBool*) result.addr;
          if ( !arg1->IsDefined() ) {
             return 0; // create an empty stream
          } else {
            int index = ( arg1->GetBoolval() ? 1 : 2 );
            local.addr = new int(index);
            qp->Open(args[index].addr);
          }
          return 0;
    }
    case REQUEST: {
          if(!local.addr){
             return CANCEL;
          }
          int index = *((int *) local.addr);
          qp->Request(args[index].addr, result);
          return qp->Received(args[index].addr)?YIELD: CANCEL;
    }

    case CLOSE: {
          if(!local.addr){
             return 0;
          }
          int index = *((int *) local.addr);
          qp->Close(args[index].addr);
          delete (int*) local.addr;
          local.addr=0;
          return 0;
    }

    case CLOSEPROGRESS:{
      return 0;
    }
    case REQUESTPROGRESS:{

          if(!local.addr){
             return CANCEL;
          }
          int index = *((int *) local.addr);
          ProgressInfo p1;
          ProgressInfo *pRes;
          pRes = (ProgressInfo*) result.addr;
          if ( qp->RequestProgress(args[index].addr, &p1) ) {
              pRes->Copy(p1);
              return YIELD;
          } else {
             return CANCEL;
          }
    }
    default: abort();
   }
   return 0;
}


ValueMapping ifthenelseVM[] = { ifthenelseDataFun, ifthenelseStreamFun, 0};


int
ifthenelse2Fun(Word* args, Word& result, int message, Word& local, Supplier s)
{
    Word res(Address(0));

    qp->Request(args[0].addr, res);
    CcBool* arg1 = static_cast<CcBool*>( res.addr );

    int index = ((arg1->IsDefined() &&  arg1->GetBoolval()) ? 1 : 2 );
    qp->Request(args[index].addr, res);

    qp->DeleteResultStorage(s);

    if ( qp->IsOperatorNode(qp->GetSon(s, index))
     && !qp->IsFunctionNode(qp->GetSon(s, index)) )
    {
      qp->ChangeResultStorage(s, res);
      qp->ReInitResultStorage(qp->GetSon(s,index));
    }
    else
    {
      Relation* r = static_cast<Relation*>( res.addr );
      qp->ChangeResultStorage(s, SetWord(r->Clone()) );
    }
    result = qp->ResultStorage(s);
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
  if ( ((Attribute*)args[0].addr)->IsDefined() &&
       ((CcInt*)args[1].addr)->IsDefined() &&
       (((CcInt*)args[1].addr)->GetValue() > -1) )
  {
    ((CcInt *)result.addr)->Set( true,
                                ((Attribute*)args[0].addr)->HashValue()
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
4.22 Value mapping function of operator ~length~

*/

int
CcLengthvaluemap( Word* args, Word& result, int message,
                  Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcInt* res = static_cast<CcInt*>(result.addr);
  CcString* a = static_cast<CcString*>(args[0].addr);
  if(!a->IsDefined()){
    res->SetDefined(false);
  } else {
    res->Set(true, a->GetValue().length());
  }
  return (0);
}


int
DistanceStrStrFun( Word* args, Word& result, int message, Word& local,
                   Supplier s ){
   result = qp->ResultStorage(s);
   CcString* source = (CcString*) args[0].addr;
   CcString* target = (CcString*) args[1].addr;
   string str1 = (const char*)(source->GetStringval());
   string str2 = (const char*)(target->GetStringval());
   int dist = stringutils::ld(str1,str2);
   ((CcInt*)result.addr)->Set(true,dist);
   return 0;
}


/*
Map any type to a string

*/

ListExpr
CcElapsedTypeMap( ListExpr args )
{
  const string mapping[] = {CcString::BasicType()};
  return SimpleMap(mapping, 1, args);
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

  resStr->Set(true, (STRING_T*) estr.str().c_str());

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

struct setoptionInfo : OperatorInfo {

  setoptionInfo() : OperatorInfo()
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
  const string mapping[] = {CcString::BasicType(), CcInt::BasicType(),
                                                          CcBool::BasicType()};
  return SimpleMap(mapping, 3, args);
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
4.18 Operator ~abs~

*/

struct absInfo : OperatorInfo {

  absInfo() : OperatorInfo()
  {
    name =      "abs";
    signature = "real -> real, int -> int";
    syntax =    "abs(_)";
    meaning =   "Returns the absolute value of its argument";
  }

};

const string maps_abs[2][2] = { {CcReal::BasicType(), CcReal::BasicType()},
                                {CcInt::BasicType(),  CcInt::BasicType() } };

static ListExpr
abs_tm(ListExpr args)
{
  return SimpleMaps<2,2>(maps_abs, args);
}

int
abs_sf( ListExpr args )
{
  return SimpleSelect<2,2>(maps_abs, args);
}

template<class S, class T>
int
abs_vm( Word* args, Word& result, int message, Word& local, Supplier s )
{
  // args[0] : real
  S* arg0 = static_cast<S*>(args[0].addr);
  result = qp->ResultStorage(s);
  S* res = static_cast<S*>(result.addr);
  if(!arg0->IsDefined()){
     res->Set(false,0);
  } else {
     res->Set(true,abs(arg0->GetValue()));
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
template<class T, bool lengthGiven>
int CcNum2String(Word* args, Word& result, int message,
                 Word& local, Supplier s) {
  T* arg = (T*)args[0].addr;
  int length = 47;
  result = qp->ResultStorage(s);
  CcString* res = (CcString*)result.addr;
  if (lengthGiven) {
    CcInt* len = (CcInt*)args[1].addr;
    if (len->IsDefined()) {
      length = len->GetValue();
      if (length < 1) {
        length = 1;
      }
      else if (length > 47) {
        length = 47;
      }
    }
    else {
      res->SetDefined(false);
    }
  }
  if (!arg->IsDefined()) {
    res->SetDefined(false);
  }
  else{
    ostringstream os;
    if (!lengthGiven) {
      length = arg->toText().length();
    }
    os.precision(length);
    os << arg->GetValue();
    string s = os.str().substr(0, MAX_STRINGSIZE);
    STRING_T S;
    strcpy(S,s.c_str());
    res->Set(true, &S);
  }
  return 0;
}

/*
4.25 Operator ~char~

*/
int CcCharFun( Word* args, Word& result, int message,
                      Word& local, Supplier s )
{
  CcInt* Cccode = (CcInt*) args[0].addr;
  result = qp->ResultStorage( s );
  CcString* res = (CcString*) result.addr;

  if ( !Cccode->IsDefined() )
    res->SetDefined( false );
  else{
     STRING_T v;
     CcInt::inttype value = Cccode->GetValue();
     if(value == 0){
       v[0] = 0;
       v[1] = 0;
     } else {
        int written = 0;
        uint32_t s = sizeof(CcInt::inttype);
        if(WinUnix::isLittleEndian()){
           for(uint32_t i=0;i<s;i++){
              unsigned char c = ((char*)&value)[(s-i)-1];
              if(c>0 || written>0){
                 v[written] =  c;
                 written++;
              } 
           }
        } else {
           for(uint32_t i=0;i<s;i++){
              unsigned char c = value &0xFF;
              value = value >> 8; 
              if(c>0 || written>0){
                 v[written] = c;
                 written++;
              } 
           }
           
        }
        v[written] = 0;
        cout << "v[" << written <<"] =0" << endl;

     }
     res->Set(true,v); 
  }
  return 0;
}

/*
4.25 Operator ~isdefined~

*/
int CCisdefinedValueMap( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Attribute* Obj = (Attribute*) args[0].addr;
  ((CcBool *)result.addr)->Set( true, Obj->IsDefined() );
  return 0;
}

/*
4.25 Operator ~assert~

*/
int CCassertValueMap( Word* args, Word& result, int message,
                         Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcBool* arg = (CcBool*)(args[0].addr);
  assert(arg->IsDefined());
  assert(arg->GetValue());
  CcBool* res = (CcBool*)(result.addr);
  res->CopyFrom(arg);
  return 0;
}


/*
4.25 Operator ~cccomparevaluemap~

*/
int CCcomparevaluemap( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  Attribute* Obj1 = (Attribute*) args[0].addr;
  Attribute* Obj2 = (Attribute*) args[1].addr;
  ((CcInt *)result.addr)->Set( true, Obj1->Compare(Obj2) );
  return 0;
}

/*
4.26 Operators ~getMinVal~ and ~getMaxVal~

A single template function is used to implement both operators.

*/

template<bool ismin>
int CCgetminmaxvaluemap( Word* args, Word& result, int message,
                           Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  int noArgs = qp->GetNoSons(s); // get number of parameters
  int resultIndex = 0; // index of the parameter holding the result
  // compare parameter with current result and update currentResult
  for(int i = 1;i<noArgs; i++){
    Attribute* Obj1 = (Attribute*) args[i].addr;
    Attribute* Obj2 = (Attribute*) args[resultIndex].addr;
    int cmp = Obj1->Compare(Obj2);
    if((ismin && cmp<0) || (!ismin && cmp>0)){
      resultIndex = i;
    }
  }
  (static_cast<Attribute*>(result.addr))->
      CopyFrom(static_cast<Attribute*>(args[resultIndex].addr));
  return 0;
}

/*
4.27 Trigonometric functions: sin, cos, tan, arcsin, arccos, arctan, pi

*/
enum TrigonOps {sin_op, cos_op, tan_op, arcsin_op, arccos_op, arctan_op,
                deg2rad_op, rad2deg_op};

template<class T, int OP>
int CCtrigonVM (Word* args, Word& result, int message, Word& local, Supplier s )
{
  T* arg = static_cast<T*>(args[0].addr);
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);

  if(!arg->IsDefined()){
    res->SetDefined(false);
    return 0;
  }

  double a = arg->GetValue();
  double r = 0.0;
  errno=0;

  switch(OP){
    case sin_op : {
        r = sin(a);
        break;
      }
    case cos_op: {
      r = cos(a);
      break;
    }
    case tan_op: {
      r = tan(a);
      break;
    }
    case arcsin_op : {
      r = asin(a);
      break;
    }
    case arccos_op: {
      r = acos(a);
      break;
    }
    case arctan_op: {
      r = atan(a);
      break;
    }
    case deg2rad_op: {
      r = (a * M_PI)/180.0;
      break;
    }
    case rad2deg_op: {
      r = (180 * a)/M_PI;
      break;
    }
    default: assert( false );
  }
  res->Set(errno==0,r);
  errno=0;
  return 0;
}

int CCArctan2VM (
    Word* args, Word& result, int message, Word& local, Supplier s )
{
  CcReal* yarg = static_cast<CcReal*>(args[0].addr);
  CcReal* xarg = static_cast<CcReal*>(args[1].addr);
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);

  if(!yarg->IsDefined() || !xarg->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  double r= atan2(yarg->GetRealval(), xarg->GetRealval());
  res->Set(true, r);
  return 0;
}

int CcPi (Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);
  res->Set(true,M_PI);
  return 0;
}

int CcE (Word* args, Word& result, int message, Word& local, Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);
  res->Set(true,M_E);
  return 0;
}

template<class T1, class T2>
int CClogBVM (Word* args, Word& result, int message, Word& local, Supplier s )
{
  T1* v = static_cast<T1*>(args[0].addr);
  T2* b = static_cast<T2*>(args[1].addr);
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);
  if(!v->IsDefined() || !b->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  double vd = v->GetValue();
  double bd = b->GetValue();
  if(vd<=0 || bd <=0){
    res->SetDefined(false);
    return 0;
  }
  res->Set(true,log(vd)/log(bd));
  return 0;
}

/*
Value Mapping for the power operator ~pow~

*/
template<class T1, class T2>
int CCpowVM (Word* args, Word& result, int message, Word& local, Supplier s )
{
  T1* b = static_cast<T1*>(args[0].addr);
  T2* e = static_cast<T2*>(args[1].addr);
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);
  if(!b->IsDefined() || !e->IsDefined()){
    res->SetDefined(false);
    return 0;
  }
  double base = b->GetValue();
  double exponent = e->GetValue();
  errno = 0;
  res->Set(true,pow(base,exponent));
  res->SetDefined(errno == 0);
  return 0;
}

/*
Value Mapping for the power operator ~dms2deg~

*/
template<class T1, class T2, class T3>
int CCdms2degVM (Word* args, Word& result, int message, Word& local,
                 Supplier s )
{
  result = qp->ResultStorage( s );
  CcReal* res = static_cast<CcReal*>(result.addr);

  int noargs = qp->GetNoSons(s);
  assert( (noargs==3) || (noargs==4) );
  T1* ccDeg = static_cast<T1*>(args[0].addr);
  T2* ccMin = static_cast<T2*>(args[1].addr);
  T3* ccSec = (noargs==4)?(static_cast<T3*>(args[2].addr)):0;
  CcBool* ccCheckrange = static_cast<CcBool*>(args[noargs-1].addr); // last
  if(!ccDeg->IsDefined() || !ccMin->IsDefined() ||
     (ccSec && !ccSec->IsDefined()) || !ccCheckrange->IsDefined() ){
    res->Set(false,0.0);
    return 0;
  }
  double deg = ccDeg->GetValue();
  double min = ccMin->GetValue();
  double sec = (ccSec)?ccSec->GetValue():0.0;
  bool checkrange = ccCheckrange->GetBoolval();
  bool ok = true;
  double degres = 0.0;
  if(checkrange){
    if( !((-60.0<min) && (min<60.0)) || !((-60.0<sec) && (sec<60.0)) ){
      ok = false;
    }
    ok = ok && AlmostEqual(modf(deg,&deg),0.0); // deg must be integral
    if(noargs==4){
      ok = ok && AlmostEqual(modf(min,&min),0.0); // min must be integral
    }
    degres = fabs(deg) + fabs(min/60.0) + fabs(sec/3600.0);
    if( (deg<0.0) || (min<0.0) || (sec<0.0) ){ // negative result!
      degres *= -1.0;
    }
  } else { // sum up all components
    degres = deg + (min/60.0) + (sec/3600.0);
  }
  res->Set(ok, degres);
  return 0;
}

/*
TypeMapping for binand and binands operator

*/

ListExpr CCBinAndTM (ListExpr args)
{
  if(!nl->HasLength(args,2)){
    return listutils::typeError("Expected int x int ");
  }
  if(!CcInt::checkType(nl->First(args)) ||
     !CcInt::checkType(nl->Second(args))){
    return listutils::typeError("Expected int x int ");
  }
  return listutils::basicSymbol<CcInt>();
}

ListExpr CCBinAndSTM  (ListExpr args)
{
  string err = "expected stream(int) ";
  if(!nl->HasLength(args,1)){
    return listutils::typeError(err);
  }
  if(!Stream<CcInt>::checkType(nl->First(args))){
    return listutils::typeError(err);
  }
  return listutils::basicSymbol<CcInt>();;
}

/*
ValueMapping for binand and binands operators

*/


int CCBinAndVM (Word* args, Word& result, int message, Word& local,
                Supplier s )
{
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*> (result.addr);

  CcInt* int1 = static_cast<CcInt*>(args[0].addr);
  CcInt* int2 = static_cast<CcInt*>(args[1].addr);

  if (!int1->IsDefined() || !int2->IsDefined()){
    res->SetDefined(false);
    return 0;
  }

  res->Set(true, int1->GetIntval() & int2->GetIntval());
  return 0;
}

int CCBinAndSVM (Word* args, Word& result, int message, Word& local,
                 Supplier s)
{
  result = qp->ResultStorage(s);
  CcInt* res = static_cast<CcInt*> (result.addr);

  Word wint;
  qp->Open(args[0].addr);
  qp->Request(args[0].addr, wint);
  if (!qp->Received(args[0].addr)){
    res->SetDefined(false);
    qp->Close(args[0].addr);
    return 0;
  }
  CcInt* curInt = static_cast<CcInt*> (wint.addr);
  int inter = curInt->GetIntval();
  curInt->DeleteIfAllowed();
  qp->Request(args[0].addr, wint);
  while (qp->Received(args[0].addr)){
    curInt = static_cast<CcInt*> (wint.addr);
    if (curInt->IsDefined()){
      inter = inter & curInt->GetIntval();
    }
    curInt->DeleteIfAllowed();
    qp->Request(args[0].addr, wint);
  }
  res->Set(true, inter);
  qp->Close(args[0].addr);
  return 0;
}

/*
5 Definition of operators

Definition of operators is done in a way similar to definition of
type constructors: an instance of class ~Operator~ is defined.

Because almost all operators are overloaded, we have first do 
define an array of value mapping functions for each operator. For 
nonoverloaded operators there is also
such and array defined, so it easier to make them overloaded.

*/

ValueMapping ccplusmap[] =
        { CcPlus_ii, CcPlus_ir, CcPlus_ri, CcPlus_rr, CcPlus_ss,
          CcPlus_long<CcInt>, CcPlus_long<LongInt>,
          CcPlus_long2<CcInt>,
          CcPlus_rat<CcInt>, CcPlus_rat<LongInt>,
          CcPlus_rat<Rational>, CcPlus_rat2<CcInt>,
          CcPlus_rat2<LongInt> };


ValueMapping ccminusmap[] =
        { CcMinus_ii, CcMinus_ir, CcMinus_ri, CcMinus_rr,
          CcMinus_long<CcInt>, CcMinus_long<LongInt>, CcMinus_long2<CcInt>,
          CcMinus_rat<CcInt>, CcMinus_rat<LongInt>, CcMinus_rat<Rational>,
          CcMinus_rat2<CcInt>, CcMinus_rat2<LongInt> };
ValueMapping ccproductmap[] =
        { CcProduct_ii, CcProduct_ir, CcProduct_ri, CcProduct_rr,
          CcProduct_long<CcInt>, CcProduct_long<LongInt>, 
          CcProduct_long2<CcInt>,
          CcProduct_rat<CcInt>, CcProduct_rat<LongInt>, 
          CcProduct_rat<Rational>,
          CcProduct_rat2<CcInt>, CcProduct_rat2<LongInt>
         };
ValueMapping ccdivisionmap[] =
        { CcDivision_ii, CcDivision_ir, CcDivision_ri, CcDivision_rr,
          CcDivision_long<CcInt>, CcDivision_long<LongInt>, 
          CcDivision_long2<CcInt>,
          CcDivision_rat<CcInt>, CcDivision_rat<LongInt>, 
          CcDivision_rat<Rational>,
          CcDivision_rat2<CcInt>, CcDivision_rat2<LongInt>
         };

ValueMapping ccmodmap[] = { CcMod };
ValueMapping ccdivmap[] = { CcDiv };
ValueMapping ccsqrtmap[] = { CcSqrt };

ValueMapping cclessmap[] = { CcLess<CcInt>,
                             CcLess2<CcInt, CcReal>,
                             CcLess2<CcInt, LongInt>,
                             CcLess2<CcReal, CcInt>,
                             CcLess<CcReal>,
                             CcLess2<CcReal, LongInt>,
                             CcLess2<LongInt, CcInt>,
                             CcLess2<LongInt, CcReal>,
                             CcLess<LongInt>,
                             CcLess<CcBool>,
                             CcLess<CcString> };

ValueMapping cclessequalmap[] = { CcLessEqual<CcInt>,
                                  CcLessEqual2<CcInt, CcReal>,
                                  CcLessEqual2<CcInt, LongInt>,
                                  CcLessEqual2<CcReal, CcInt>,
                                  CcLessEqual<CcReal>,
                                  CcLessEqual2<CcReal,LongInt>,
                                  CcLessEqual2<LongInt, CcInt>,
                                  CcLessEqual2<LongInt, CcReal>,
                                  CcLessEqual<LongInt>,
                                  CcLessEqual<CcBool>,
                                  CcLessEqual<CcString> };

ValueMapping ccgreatermap[] = { CcGreater<CcInt>,
                                CcGreater2<CcInt, CcReal>,
                                CcGreater2<CcInt, LongInt>,
                                CcGreater2<CcReal, CcInt>,
                                CcGreater<CcReal>,
                                CcGreater2<CcReal, LongInt>,
                                CcGreater2<LongInt, CcInt>,
                                CcGreater2<LongInt, CcReal>,
                                CcGreater<LongInt>,
                                CcGreater<CcBool>,
                                CcGreater<CcString> };

ValueMapping ccgreaterequalmap[] = { CcGreaterEqual<CcInt>,
                                     CcGreaterEqual2<CcInt, CcReal>,
                                     CcGreaterEqual2<CcInt, LongInt>,
                                     CcGreaterEqual2<CcReal, CcInt>,
                                     CcGreaterEqual<CcReal>,
                                     CcGreaterEqual2<CcReal, LongInt>,
                                     CcGreaterEqual2<LongInt, CcInt>,
                                     CcGreaterEqual2<LongInt, CcReal>,
                                     CcGreaterEqual<LongInt>,
                                     CcGreaterEqual<CcBool>,
                                     CcGreaterEqual<CcString> };

ValueMapping ccequalmap[] = { CcEqual<CcInt>,
                              CcEqual2<CcInt, CcReal>,
                              CcEqual2<CcInt, LongInt>,
                              CcEqual2<CcReal, CcInt>,
                              CcEqual<CcReal>,
                              CcEqual2<CcReal, LongInt>,
                              CcEqual2<LongInt, CcInt>,
                              CcEqual2<LongInt, CcReal>,
                              CcEqual<LongInt>,
                              CcEqual<CcBool>,
                              CcEqual<CcString> };

ValueMapping ccdiffmap[] = { CcDiff<CcInt>,
                             CcDiff2<CcInt, CcReal>,
                             CcDiff2<CcInt, LongInt>,
                             CcDiff2<CcReal, CcInt>,
                             CcDiff<CcReal>,
                             CcDiff2<CcReal, LongInt>,
                             CcDiff2<LongInt, CcInt>,
                             CcDiff2<LongInt,CcReal>,
                             CcDiff<LongInt>,
                             CcDiff<CcBool>,
                             CcDiff<CcString> };

ValueMapping ccstartsmap[] = { StartsFun };
ValueMapping cccontainsmap[] = { ContainsFun };
ValueMapping ccandmap[] = { AndFun };
ValueMapping ccandSmap[] = { AndSFun };
ValueMapping ccormap[] = { OrFun };
ValueMapping ccorSmap[] = { OrSFun };
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
                          { CcNum2String<CcReal, false>, 
                            CcNum2String<CcInt, false>,
                            CcNum2String<LongInt, false>,
                            CcNum2String<CcReal, true>, 
                            CcNum2String<CcInt, true>,
                            CcNum2String<LongInt, true>};
ValueMapping abs_vms[] = { abs_vm<CcReal, double>, abs_vm<CcInt, int>, 0 };
ValueMapping cccharvaluemap[] = { CcCharFun };

ValueMapping CCsinVM[] = {
  CCtrigonVM<CcInt, sin_op>,
  CCtrigonVM<CcReal, sin_op>, 0};
ValueMapping CCcosVM[] = {
  CCtrigonVM<CcInt, cos_op>,
  CCtrigonVM<CcReal, cos_op>, 0};
ValueMapping CCtanVM[] = {
  CCtrigonVM<CcInt, tan_op>,
  CCtrigonVM<CcReal, tan_op>, 0};
ValueMapping CCarcsinVM[] = {
  CCtrigonVM<CcInt, arcsin_op>,
  CCtrigonVM<CcReal, arcsin_op>, 0};
ValueMapping CCarccosVM[] = {
  CCtrigonVM<CcInt, arccos_op>,
  CCtrigonVM<CcReal, arccos_op>, 0};
ValueMapping CCarctanVM[] = {
  CCtrigonVM<CcInt, arctan_op>,
  CCtrigonVM<CcReal, arctan_op>, 0};
ValueMapping CCdeg2radVM[] = {
    CCtrigonVM<CcInt, deg2rad_op>,
    CCtrigonVM<CcReal, deg2rad_op>, 0};
ValueMapping CCrad2degVM[] = {
      CCtrigonVM<CcInt, rad2deg_op>,
      CCtrigonVM<CcReal, rad2deg_op>, 0};
ValueMapping CClogBmap[] = {
        CClogBVM<CcInt, CcInt>,
        CClogBVM<CcInt, CcReal>,
        CClogBVM<CcReal, CcInt>,
        CClogBVM<CcReal, CcReal>, 0};
ValueMapping CCpowmap[] = {
        CCpowVM<CcInt, CcInt>,
        CCpowVM<CcInt, CcReal>,
        CCpowVM<CcReal, CcInt>,
        CCpowVM<CcReal, CcReal>, 0};

ValueMapping CCdms2degmap[] = {
  CCdms2degVM<CcInt, CcInt, CcInt>,
  CCdms2degVM<CcInt, CcInt, CcReal>,
  CCdms2degVM<CcInt, CcReal, CcInt>,
  CCdms2degVM<CcInt, CcReal, CcReal>,
  CCdms2degVM<CcReal, CcInt, CcInt>,
  CCdms2degVM<CcReal, CcInt, CcReal>,
  CCdms2degVM<CcReal, CcReal, CcInt>,
  CCdms2degVM<CcReal, CcReal, CcReal>, 0};

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

const string CCSpecAnd  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" )"
    "( <text>(bool bool) -> bool</text--->"
    "<text>_ and _</text--->"
    "<text>Lazy logical conjunction (AND). Returns TRUE iff both arguments are "
    "defined and TRUE. The result is always defined. Arguments are evaluated "
    "in a lazy fashion, first the left argument, then the right one. "
    "The second argument is only evaluated if the first one is defined and "
    "TRUE. </text--->"
    "<text>query (8 = 8) and (3 < 4)</text--->"
    ") )";

const string CCSpecAndS  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" )"
    "( <text>(bool bool) -> bool</text--->"
    "<text>_ sand _</text--->"
    "<text>Strict logical conjunction (AND). Returns TRUE iff both arguments "
    "are defined and TRUE. If one of the arguments is UNDEFINED, so is the "
    "result. All arguments are always evaluated.</text--->"
    "<text>query (8 = 8) sand (3 < 4)</text--->"
    ") )";

const string CCSpecOr  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" )"
    "( <text>(bool bool) -> bool</text--->"
    "<text>_ or _</text--->"
    "<text>Lazy logical disjunction (OR). Returns TRUE iff at least one "
    "argument is defined and TRUE. The result is always defined. Arguments are "
    "evaluated in a lazy fashion, first the left argument, then the right one. "
    "The second argument is only evaluated if the first one is undefined or "
    "FALSE. </text--->"
    "<text>query (3 <= 4) or (\"hotel\" > "
    "\"house\")"
    "</text--->"
    ") )";

const string CCSpecOrS  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" "
    "\"Example\" )"
    "( <text>(bool bool) -> bool</text--->"
    "<text>_ sor _</text--->"
    "<text>Strict logical disjunction (OR). Returns TRUE iff both arguments "
    "are defined and at least one is TRUE. If one of the arguments is "
    "UNDEFINED, so is the result. All arguments are always evaluated.</text--->"
    "<text>query (8 = 8) sor (3 < 4)</text--->"
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

const string CCSpecPlusPlus  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>string -> string</text--->"
                               "<text> _ ++</text--->"
                               "<text>Returns successor for the passed string "
                               "</text--->"
                               "<text>query \"hello\"++</text--->"
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
             "( <text>(bool x T x T) ->  T, T in DATA, "
             "or T = stream(...) </text--->"
             "<text>ifthenelse(P, R1, R2)</text--->"
             "<text> Evalutes and returns the second argument R1, if the "
             "boolean value expression, given as a first argument P, can be "
             "evaluated to TRUE. If P evaluates to FALSE, the third argument "
             "R2 is evaluated and returned. If P is undefined, so is the result"
             " undefined resp. an empty stream."
             "</text--->"
             "<text>query ifthenelse(3 < 5,[const string value \"less\"],"
                "[const string value \"greater\"])</text--->"
             ") )";

const string CCSpecIfthenelse2  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                            "\"Example\" )"
                             "( <text>(bool x T x T) ->  T </text--->"
             "<text>ifthenelse2(P, R1, R2)</text--->"
             "<text>Evalutes and returns the second argument R1, if the "
             "boolean value expression, given as a first argument P, can be "
             "evaluated to TRUE. If P evaluates to FALSE or it is undefined, "
             "the third argument "
             "R2 is evaluated and returned.  "
             "NOTE: The second and the third argument must be of the "
             "same type T.</text--->"
             "<text>query ifthenelse2(3 < 5, ten,"
                "thousand feed consume) count</text--->"
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
                   + "'Returns the part of a string s starting at "
                   + "position p and ending at position q. Positions at 1.'"
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
             "( <text>{real, int, longint} -> string</text--->"
             "<text>num2string( num )</text--->"
             "<text>Returns the string representation of numeric argument "
             "'num'.</text--->"
             "<text>query num2string(12.345)</text--->"
             ") )";

const string CCcharSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>int -> string</text--->"
    "<text>char( Code )</text--->"
    "<text>Returns a string corresponding with a single charater "
    "using ANSI-Coding. 'Code'. Code must be in {0, 32 - 126, 128 - 255}, "
    "otherwise an undefined string is returned. For '0', the string will be "
    "empty, and '34' ('\"') will be mapped to \"''\".</text--->"
    "<text>query char(25)</text--->"
    ") )";

const string CCisdefinedSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>DATA -> bool</text--->"
    "<text>isdefined( v )</text--->"
    "<text>Tests, whether a value 'v' of a type in kind DATA is defined. "
    "Actually, the operator depends on the implementation of 'IsDefined()'. "
    "For several types with set-sementics, this is not implemented "
    "consequently.</text--->"
    "<text>query isdefined(987)</text--->"
    ") )";


const string CCassertSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>bool -> bool</text--->"
    "<text>assert( v )</text--->"
    "<text>Invokes a assertion if the argument is not defined or false "
    "This operator is for debugging only !!! "
    "</text--->"
    "<text>query assert(987 = 987)</text--->"
    ") )";


const string CCcompareSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>T x T -> int, T in DATA</text--->"
    "<text>compare( v1, v2 )</text--->"
    "<text>Applies the internal 'Compare' function to both arguments. The "
    "result is always a defined integer. If both arguments are 'equal', "
    "the result is 0; if 'v1 < v2', the result is a negative integer, if "
    "'v1 > v2', the result is a positive integer. Undefined values are "
    "treated as 'infinitely small', two undefined values are 'equal'. Since "
    "this operator relies on the type's implementation of 'Compare', the "
    "behaviour may differ, if the implementation is non-canonical.</text--->"
    "<text>query compare(-987, [const int value undef])</text--->"
    ") )";

const string CCgetminvalSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>T^n -> T, T in DATA, n >= 1</text--->"
    "<text>getMinVal( v1, v2, ... )</text--->"
    "<text>Extracts the first (leftmost) minimum value from the parameter "
    "list, using the generic compare function. Since this operator relies on "
    "the type's implementation of 'Compare', the behaviour may differ, if the "
    "implementation is non-canonical.</text--->"
    "<text>query getMinVal(45, -45, 12, 0, -75, 5)</text--->"
    ") )";

const string CCgetmaxvalSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text>T^n -> T, T in DATA, n >= 1</text--->"
    "<text>getMaxVal( v1, v2, ... )</text--->"
    "<text>Extracts the first (leftmost) maximum value from the parameter "
    "list, using the generic compare function. Since this operator relies on "
    "the type's implementation of 'Compare', the behaviour may differ, if the "
    "implementation is non-canonical.</text--->"
    "<text>query getMaxVal(45, -45, 12, 0, -75, 5)</text--->"
    ") )";

const string CClengthSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> string -> int </text--->"
    "<text>length(_)</text--->"
    "<text> Returns the length of the argument</text--->"
    "<text>query length(\"Hello World\")</text--->"
    ") )";

/*
Trigonometric functions

*/
const string CCsinSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int | real -> real </text--->"
    "<text>sin( v )</text--->"
    "<text>Returns the sinus of v (v is given in rad).</text--->"
    "<text>query sin(deg2rad(90))</text--->"
    ") )";

const string CCcosSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int | real -> real </text--->"
    "<text>cos( v )</text--->"
    "<text>Returns the cosinus of v (v is given in rad).</text--->"
    "<text>query cos(deg2rad(90))</text--->"
    ") )";

const string CCtanSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int | real -> real </text--->"
    "<text>tan( v )</text--->"
    "<text>Returns the tangens of v (v is given in rad).</text--->"
    "<text>query tan(deg2rad(90))</text--->"
    ") )";

const string CCarcsinSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int | real -> real </text--->"
    "<text>arcsin( v )</text--->"
    "<text>Returns the principal value of the arc sine of x, expressed "
    "in radians, in the interval [-pi/2,+pi/2] radians.</text--->"
    "<text>query arcsin(sin(deg2rad(90)))</text--->"
    ") )";

const string CCarccosSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int | real -> real </text--->"
    "<text>arccos( v )</text--->"
    "<text>Returns the principal value of the arc cosine of x, "
    "expressed in radians, in the interval [0,pi] radians.</text--->"
    "<text>query arccos(sin(deg2rad(90)))</text--->"
    ") )";

const string CCarctanSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int | real -> real </text--->"
    "<text>arctan( v )</text--->"
    "<text>Returns the principal value of the arc tangent of x, expressed in "
    "radians, in the interval [-pi/2,+pi/2] radians"
    " of v (v is given in rad).</text--->"
    "<text>query arctan(tan(deg2rad(90)))</text--->"
    ") )";

const string CCarctan2Spec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> real x real -> real </text--->"
    "<text>arctan2( dy, dx )</text--->"
    "<text>Returns the principal value of the arc tangent of y/x, "
    "expressed in radians, in the interval [-pi,+pi] radians.</text--->"
    "<text>query arctan2(tan(deg2rad(90)))</text--->"
    ") )";

const string CCpiSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> -> real </text--->"
    "<text>const_pi()</text--->"
    "<text>Returns pi (= 3.14159...)</text--->"
    "<text>query sin(const_pi())</text--->"
    ") )";

const string CCdeg2radSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int | real -> real </text--->"
    "<text>deg2rad( v )</text--->"
    "<text>Convert angle from degrees to radians.</text--->"
    "<text>query arctan(tan(deg2rad(90)))</text--->"
    ") )";

const string CCrad2degSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> int | real -> real </text--->"
    "<text>rad2deg( v )</text--->"
    "<text>Convert angle from radians to degrees.</text--->"
    "<text>query rad2deg(const_pi())</text--->"
    ") )";

const string CCeSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> -> real </text--->"
    "<text>const_e()</text--->"
    "<text>Returns E (= 2.718281828459...)</text--->"
    "<text>query const_e()</text--->"
    ") )";

const string CClogBSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> {int | real} x {int | real} -> real </text--->"
    "<text>logB( val , base)</text--->"
    "<text>Returns the logarithm of 'val' to the given 'base'.</text--->"
    "<text>query logB(const_e()*const_e(),const_e())</text--->"
    ") )";

const string CCpowSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> {int | real} x {int | real} -> real </text--->"
    "<text>pow( Base , Exponent)</text--->"
    "<text>Returns Base to the power of Exponent. Returns UNDEF if any "
    "argument is UNDEF, type real cannot represent the result, or Base is "
    "negative and Exponent is not an integral value.</text--->"
    "<text>query pow(2.0, 10)</text--->"
    ") )";

const string CCdms2degSpec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
    "( <text> {int|real} x {int|real} [x {int|real}] [x bool] -> real</text--->"
    "<text>dms2deg( Deg , Min [, Sec] [, CheckRange] )</text--->"
    "<text>Converts an Angle with Deg degrees, Min minutes and Sec seconds into"
    " a single real value with fractional degrees. If CheckRange is FALSE "
    " (default), all arguments are converted to degrees and summed up. "
    "Otherwise only the last numeric argument may be fractional and -60<Min<60,"
    " and -60<Sec<60 must hold to avoid an UNDEF result. If CheckRange=TRUE, "
    "the result is calculated using the absolute values of the numeric "
    "parameters. Then, iff any numeric parameter is negative, the result is "
    "negative.</text--->"
    "<text>query dms2deg(7, -17.1, 64, FALSE)</text--->"
    ") )";

/*
binary and of integer values and streams of integer values

*/

const string CCBinAndSpec =
  "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text> int X int -> int </text--->"
  "<text> _ binand _ </text--->"
  "<text>Computes the binary and conjunction of the two given int as int value."
  "</text--->"
  "<text>query 15 binand 4 </text--->"
  ") )";

const string CCBinAndSSpec =
  "(( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text> stream(int) -> int </text--->"
  "<text> _ binands </text--->"
  "<text>Computes the binary conjunction of the stream of int values."
  "</text--->"
  "<text>query intstream(1,5) binands </text--->"
  ") )";

/*
Operator instance definitions

*/

Operator ccplus( "+", CCSpecAdd, 13, ccplusmap,
                 CcPlusSelectCompute, CcPlusTypeMap );

Operator ccminus( "-", CCSpecSub, 12, ccminusmap,
                  CcMathSelectCompute, CcMathTypeMap );

Operator ccproduct( "*", CCSpecMul, 12, ccproductmap,
                    CcMathSelectCompute, CcMathTypeMap );

Operator ccdivision( "/", CCSpecDiv, 12, ccdivisionmap,
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

Operator ccless( "<", CCSpecLT, 11, cclessmap,
                 CcMathSelectCompare, CompareTypeMap);

Operator cclessequal( "<=", CCSpecLE, 11, cclessequalmap,
                      CcMathSelectCompare, CompareTypeMap );

Operator ccgreater( ">", CCSpecGT, 11, ccgreatermap,
                    CcMathSelectCompare, CompareTypeMap );

Operator ccgreaterequal( ">=", CCSpecGE, 11, ccgreaterequalmap,
                         CcMathSelectCompare, CompareTypeMap );

Operator ccequal( "=", CCSpecEQ, 11, ccequalmap,
                  CcMathSelectCompare, CompareTypeMap );

Operator ccdiff( "#", CCSpecNE, 11, ccdiffmap,
                 CcMathSelectCompare, CompareTypeMap );

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

Operator ccandS( "sand", CCSpecAndS, 1, ccandSmap,
                Operator::SimpleSelect, CcMathTypeMapBool2 );

Operator ccor( "or", CCSpecOr, 1, ccormap,
               Operator::SimpleSelect, CcMathTypeMapBool2 );

Operator ccorS( "sor", CCSpecOrS, 1, ccorSmap,
               Operator::SimpleSelect, CcMathTypeMapBool2 );

Operator ccisempty( "isempty", CCSpecIsEmpty, 4, ccisemptymap,
                    CcMathSelectIsEmpty, CcMathTypeMapBool4 );

Operator ccplusplus( "++", CCSpecPlusPlus, PlusPlusFun,
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

Operator ccopifthenelse( "ifthenelse", CCSpecIfthenelse, 2, ifthenelseVM,
                         ifthenelseSelect, ifthenelseType );

Operator ccopifthenelse2( "ifthenelse2", CCSpecIfthenelse2, ifthenelse2Fun,
                         Operator::SimpleSelect, ifthenelse2Type );

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

Operator ccnum2string( "num2string", CCnum2stringSpec, 6, ccnum2stringvaluemap,
                 ccnum2stringSelect, NumStringTypeMap);

Operator ccchar( "char", CCcharSpec, 1, cccharvaluemap,
                 Operator::SimpleSelect, IntString);

Operator ccisdefined( "isdefined", CCisdefinedSpec, CCisdefinedValueMap,
                      Operator::SimpleSelect, DATAbool);

Operator ccassert( "assert", CCassertSpec, CCassertValueMap,
                    Operator::SimpleSelect, CcMathTypeMapBool1);

Operator cccompare( "compare", CCcompareSpec, CCcomparevaluemap,
                      Operator::SimpleSelect, CcTypeMapTinDATA_TinDATAint);

Operator ccgetminval( "getMinVal", CCgetminvalSpec, CCgetminmaxvaluemap<true>,
                      Operator::SimpleSelect, CcTypeMapTinDATAexpplus2TinDATA);

Operator ccgetmaxval( "getMaxVal", CCgetmaxvalSpec, CCgetminmaxvaluemap<false>,
                      Operator::SimpleSelect, CcTypeMapTinDATAexpplus2TinDATA);

Operator cclength( "length", CClengthSpec, CcLengthvaluemap,
                   Operator::SimpleSelect, TypeMap1<CcString, CcInt>);

/*
Trigonometric functions

*/

Operator ccsin( "sin", CCsinSpec, 2, CCsinVM, CcNumRealSelect,
                CcTypeMapNumReal);
Operator cccos( "cos", CCcosSpec, 2, CCcosVM, CcNumRealSelect,
                CcTypeMapNumReal);
Operator cctan( "tan", CCtanSpec, 2, CCtanVM, CcNumRealSelect,
                CcTypeMapNumReal);
Operator ccarcsin( "arcsin", CCarcsinSpec, 2, CCarcsinVM, CcNumRealSelect,
                   CcTypeMapNumReal);
Operator ccarccos( "arccos", CCarccosSpec, 2, CCarccosVM, CcNumRealSelect,
                   CcTypeMapNumReal);
Operator ccarctan( "arctan", CCarctanSpec, 2, CCarctanVM, CcNumRealSelect,
                   CcTypeMapNumReal);
Operator ccarctan2( "arctan2", CCarctan2Spec, CCArctan2VM,
                    Operator::SimpleSelect, CcTypeMapRealRealReal);
Operator ccpi( "const_pi", CCpiSpec, CcPi, Operator::SimpleSelect,
               CcTypeMapEmptyReal);
Operator ccdeg2rad( "deg2rad", CCdeg2radSpec, 2, CCdeg2radVM, CcNumRealSelect,
                              CcTypeMapNumReal);
Operator ccrad2deg( "rad2deg", CCrad2degSpec, 2, CCrad2degVM, CcNumRealSelect,
                               CcTypeMapNumReal);

Operator cce( "const_e", CCeSpec, CcE, Operator::SimpleSelect,
                               CcTypeMapEmptyReal);
Operator cclogb("logB", CClogBSpec, 4, CClogBmap, CcNumNumRealSelect,
                                               CcTypeMapNumNumReal);

Operator ccpow("pow", CCpowSpec, 4, CCpowmap, CcNumNumRealSelect,
                        CcTypeMapNumNumReal);

Operator ccdms2deg("dms2deg", CCdms2degSpec, 8, CCdms2degmap,
                   CcNumNumNumRealSelect, CcTypeMapNumNumOptNumOptBoolReal);

/*
1.1 binary and of two integers and binary and for a stream of integers

*/

Operator ccbinand("binand", CCBinAndSpec, CCBinAndVM, Operator::SimpleSelect,
                  CCBinAndTM);

Operator ccbinands("binands", CCBinAndSSpec, CCBinAndSVM,
                    Operator::SimpleSelect, CCBinAndSTM);



/*
5.98 Operator switch

THis operator simulates a switch case statement.

5.98.1 TypeMapping

*/
ListExpr switchTM(ListExpr args){

  int len = nl->ListLength(args);
  if(len < 4){
     return listutils::typeError("at least four arguments required");
  }

  bool isEven = (len & 1)==0;

  if(!isEven){
    return listutils::typeError("even number of arguments required");
  }

  ListExpr firstArg = nl->First(args);

  if(!listutils::isDATA(firstArg)){
     return listutils::typeError("first argument must be in kind data");
  }

  ListExpr rest = nl->Rest(args);
  bool isFirst = true;
  ListExpr resType = nl->TheEmptyList();
  bool odd = true;
  int no = 1;
  while(!nl->IsEmpty(rest)){
     ListExpr first = nl->First(rest);
     rest = nl->Rest(rest);    
     no++;
     if(odd && (no!=len)){ // switch type
       if(!nl->Equal(firstArg,first)){
          return listutils::typeError("One of the Cases has different "
                                      "type to first argument");
       }
     } else { // result type
        if(isFirst){
           resType = first;
           if(!listutils::isDATA(resType) && !listutils::isStream(resType)){
              return listutils::typeError(nl->ToString(resType) + 
                                       " is not in Kind Data");
           }
        } else {
           if(!nl->Equal(resType,first)){
             return listutils::typeError("Different result types detected");
           }
        }
     }
     odd = !odd;
 }
 return resType;
}


/*
5.98.2 Value Mapping

*/

int switchVM0 (Word* args, Word& result, int message, 
              Word& local, Supplier s ) {

   Attribute* sw = (Attribute*) args[0].addr;
   int noArgs = qp->GetNoSons(s);
   int index = noArgs - 1; // the default value
   // try to find a better index
   bool found = false;
   for(int i=1;i<noArgs-1 && ! found; i += 2){
      Attribute* v = (Attribute*) args[i].addr;
      int cmp = sw->Compare(v);
      if(cmp==0){
        found = true;
        index = i+1;
      }
   }
   result = qp->ResultStorage(s);
   Attribute* res = (Attribute*) result.addr;
   res->CopyFrom((Attribute*) args[index].addr);
   return 0;
}


int switchVM1 (Word* args, Word& result, int message, 
              Word& local, Supplier s ) {

   int* li = (int*) local.addr;
   switch(message){
       case OPEN: {
           if(li){
              delete li;
           }
           Attribute* sw = (Attribute*) args[0].addr;
           int noArgs = qp->GetNoSons(s);
           int index = noArgs - 1; // the default value
           // try to find a better index
           bool found = false;
           for(int i=1;i<noArgs-1 && ! found; i += 2){
               Attribute* v = (Attribute*) args[i].addr;
               int cmp = sw->Compare(v);
               if(cmp==0){
                  found = true;
                  index = i+1;
               }
           }
           qp->Open(args[index].addr);
           local.addr = new int(index);
           return 0;
       }
       case REQUEST: {
         if(!li){
            result.addr = 0;
            return CANCEL;
         }
         Word w;
         qp->Request(args[*li].addr,w);
         if(qp->Received(args[*li].addr)){
            result = w;
            return YIELD;
         } else {
            result.addr = 0;
            return CANCEL;
         }
       }
       case CLOSE: {
         if(li){
           qp->Close(args[*li].addr);
           delete li;
           local.addr = 0;
         }
         return 0;
       }
   }
   return -1;
}

/*
1.98.3 Selection function

*/
int switchSelect(ListExpr args){
  int len = nl->ListLength(args);
  ListExpr resType;
  if(len==2){
    resType = nl->Second(args);
  } else {
    resType = nl->Third(args);
  }
  if(listutils::isDATA(resType)){
     return 0;
  } else {
     return 1;
  }
}

/*
1.98.4 Value Mapping array

*/

ValueMapping switchVM[]={
   switchVM0,
   switchVM1
};

/*
1.98.5 Specification

*/

OperatorSpec switchSpec(
  "t x (t x r)* x r -> r. with t in DATA, r in DATA U STREAM",
  " switchvalue switch[ case1 , res1; case2 , res2 ; ... ; default] ",
  " Returns the first res_i for what holds case_i = switchvalue. If no such"
  " case exists, default is returned",
  " query randint(2) switch[ 0 , 'Null'; 1, 'One'; 2 , 'Two'; 'Unknown']"
);

/*
1.98.6 Operator instance

*/
Operator switchOp(
   "switch",
   switchSpec.getStr(),
   2,
   switchVM,
   switchSelect,
   switchTM);



/*
1.99 Operator int2longint

*/
ListExpr int2longintTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("int expected");
  }
  if(CcInt::checkType(nl->First(args))){
    return listutils::basicSymbol<LongInt>();
  }
  return listutils::typeError("int expected");
}

int int2longintVM (Word* args, Word& result, int message, 
              Word& local, Supplier s ) {

  CcInt* arg = (CcInt*) args[0].addr;
  result=qp->ResultStorage(s);
  LongInt* res = (LongInt*) result.addr;
  res->SetValue(*arg);
  return 0;
}


OperatorSpec int2longintSpec(
  "int -> longint",
  "int2longint(_) ",
  "Converts an int to a longint. ",
  " query int2longint(2)"
);


Operator int2longint(
   "int2longint",
   int2longintSpec.getStr(),
   int2longintVM,
   Operator::SimpleSelect,
   int2longintTM);


/*
1.99 Operator longint2int

*/
ListExpr longint2intTM(ListExpr args){
  if(!nl->HasLength(args,1)){
    return listutils::typeError("longint expected");
  }
  if(LongInt::checkType(nl->First(args))){
    return listutils::basicSymbol<CcInt>();
  }
  return listutils::typeError("longint expected");
}

int longint2intVM (Word* args, Word& result, int message, 
              Word& local, Supplier s ) {

  LongInt* arg = (LongInt*) args[0].addr;
  result=qp->ResultStorage(s);
  CcInt* res = (CcInt*) result.addr;
  if(!arg->IsDefined()){
    res->SetDefined(false);
  } else {
    int64_t v = arg->GetValue();
    int32_t vi = (int32_t) v;
    if(v!=vi){
       res->SetDefined(false);
    } else {
       res->Set(true,vi);
    }
  }
  return 0;
}


OperatorSpec longint2intSpec(
  "longint -> int",
  "longint2int(_) ",
  "Converts a longint to an int. ",
  " query longint2int(int2longint(2))"
);


Operator longint2int(
   "longint2int",
   longint2intSpec.getStr(),
   longint2intVM,
   Operator::SimpleSelect,
   longint2intTM);

/*
Operator ~rat~

This operator converts several numeric types into a rational

*/
ListExpr ratTM(ListExpr args){
   string err = "{int,long,string} or (i1 x i2) with ii int type expected";
   if(!nl->HasLength(args,1) && !nl->HasLength(args,2)){
      return listutils::typeError(err);
   } 
   if(nl->HasLength(args,1)){
     ListExpr arg1 = nl->First(args);
     if(CcInt::checkType(arg1) || LongInt::checkType(arg1) ||
        CcString::checkType(arg1)){
         return listutils::basicSymbol<Rational>();
     }
     return listutils::typeError(err);
   } 
   // two arguments
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if(!CcInt::checkType(arg1) && !LongInt::checkType(arg1)){
      return listutils::typeError(err);
   } 
   if(!CcInt::checkType(arg2) && !LongInt::checkType(arg2)){
      return listutils::typeError(err);
   } 
   return listutils::basicSymbol<Rational>();
}

template<class S>
int ratVM1 (Word* args, Word& result, int message, 
              Word& local, Supplier s ) {

  S* arg = (S*) args[0].addr;
  result = qp->ResultStorage(s);
  Rational* res = (Rational*) result.addr;
  Rational r(*arg);
  (*res) = r;
  return 0;
}

template<class S1, class S2>
int ratVM2 (Word* args, Word& result, int message, 
              Word& local, Supplier s ) {

  S1* arg1 = (S1*) args[0].addr;
  S2* arg2 = (S2*) args[1].addr;
  result = qp->ResultStorage(s);
  Rational* res = (Rational*) result.addr;
  Rational r(*arg1, *arg2);
  (*res) = r;
  return 0;
}

ValueMapping ratVM[] ={
     ratVM1<CcInt>, ratVM1<LongInt>, ratVM1<CcString>,
     ratVM2<CcInt,CcInt>, ratVM2<CcInt,LongInt>,
     ratVM2<LongInt,CcInt>, ratVM2<LongInt,LongInt>
};

int ratSelect(ListExpr args){
  if(nl->HasLength(args,1)){
     ListExpr arg = nl->First(args);
     if(CcInt::checkType(arg)){
        return 0;
     }
     if(LongInt::checkType(arg)){
        return 1;
     }
     if(CcString::checkType(arg)){
        return 2;
     }
     return -1;
   }
   // 2 arguments
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   int s1 = CcInt::checkType(arg1)?3:5;
   int s2 = CcInt::checkType(arg2)?0:1;
   return s1+s2;
}


OperatorSpec ratSpec(
  "{int, longint, string}  -> rational "
  "or i1 x i2 -> rational , i1,i2 in {int,longint}",
  "rat(_) ",
  "Converts the argument(s) to a rational ",
  " query rat(-32,8)"
);


Operator rat(
   "rat",
   ratSpec.getStr(),
   7, 
   ratVM,
   ratSelect,
   ratTM);



/*
Operator chars

*/
ListExpr charsTM(ListExpr args){
   string err = "string expected";
   if(!nl->HasLength(args,1)){
     return listutils::typeError(err);
   }
   if(!CcString::checkType(nl->First(args))){
     return listutils::typeError(err);
   }
   return nl->TwoElemList(
              listutils::basicSymbol<Stream<CcInt> >(),
              listutils::basicSymbol<CcInt>());
}

class charsInfo{
  public:
  charsInfo(CcString& s): str(""), pos(0), finished(false){
     if(!s.IsDefined()){
        finished=true;  
     }
     str = s.GetValue();
     finished = str.length()==pos;
  }
  CcInt* getNext(){
     if(finished ){
       return 0;
     } 
     unsigned char s = str[pos];
     pos++;
     finished = pos == str.length();
     return new CcInt(true,s);
  }
 private:
    string str;
    size_t pos;
    bool finished;
};



int charsVM (Word* args, Word& result, int message, 
              Word& local, Supplier s ) {

  charsInfo* li = (charsInfo*) local.addr;
  switch(message){
     case OPEN : if(li){
                    delete li;
                 }
                 local.addr = new charsInfo(*((CcString*)args[0].addr));
                 return 0;
     case REQUEST:
                  result.addr = li?li->getNext():0;
                  return result.addr?YIELD:CANCEL;
     case CLOSE:
               if(li){
                  delete li;
                  local.addr = 0;
               }
               return 0;

  }
  return -1;

}

OperatorSpec charsSpec(
  "string -> stream(int)",
  "chars(_) ",
  "Returns the characters from the inputs codes as integers ",
  " query chars(\"test\") count"
);


Operator chars(
   "chars",
   charsSpec.getStr(),
   charsVM,
   Operator::SimpleSelect,
   charsTM);



/*
6 Class ~CcAlgebra~

The last steps in adding an algebra to the Secondo system are

  * Associating value mapping functions with their operators

  * ``Bunching'' all
type constructors and operators in one instance of class ~Algebra~.

Therefore, a new subclass ~CcAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual
 algebra.

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
    AddTypeConstructor( &longint);
    AddTypeConstructor( &rational);

    ccInt.AssociateKind( Kind::DATA() );
    ccReal.AssociateKind( Kind::DATA() );
    ccBool.AssociateKind( Kind::DATA() );
    ccString.AssociateKind( Kind::DATA() );
    longint.AssociateKind(Kind::DATA());
    rational.AssociateKind(Kind::DATA());

    ccInt.AssociateKind( Kind::BASE() );
    ccReal.AssociateKind( Kind::BASE() );
    ccBool.AssociateKind( Kind::BASE() );
    ccString.AssociateKind( Kind::BASE() );
    longint.AssociateKind( Kind::BASE() );
    rational.AssociateKind( Kind::BASE() );

    ccInt.AssociateKind( Kind::CSVEXPORTABLE() );
    ccReal.AssociateKind( Kind::CSVEXPORTABLE() );
    ccBool.AssociateKind( Kind::CSVEXPORTABLE() );
    ccString.AssociateKind( Kind::CSVEXPORTABLE() );
    longint.AssociateKind( Kind::CSVEXPORTABLE() );
    rational.AssociateKind( Kind::CSVEXPORTABLE() );

    ccInt.AssociateKind( Kind::CSVIMPORTABLE() );
    ccReal.AssociateKind( Kind::CSVIMPORTABLE() );
    ccBool.AssociateKind( Kind::CSVIMPORTABLE() );
    ccString.AssociateKind( Kind::CSVIMPORTABLE() );
    longint.AssociateKind( Kind::CSVIMPORTABLE() );
    rational.AssociateKind( Kind::CSVIMPORTABLE() );
    
    ccInt.AssociateKind( Kind::SQLEXPORTABLE() );
    ccReal.AssociateKind( Kind::SQLEXPORTABLE() );
    ccBool.AssociateKind( Kind::SQLEXPORTABLE() );
    ccString.AssociateKind( Kind::SQLEXPORTABLE() );

    ccInt.AssociateKind( Kind::DELIVERABLE() );
    ccReal.AssociateKind( Kind::DELIVERABLE() );
    ccBool.AssociateKind( Kind::DELIVERABLE() );
    ccString.AssociateKind( Kind::DELIVERABLE() );

    
    longint.AssociateKind(Kind::INDEXABLE());

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
    ccand.SetRequestsArguments();
    AddOperator( &ccandS );
    AddOperator( &ccor );
    ccor.SetRequestsArguments();
    AddOperator( &ccorS );
    AddOperator( &ccisempty );
    AddOperator( &ccplusplus );
    AddOperator( &ccsetintersection );
    AddOperator( &ccsetminus );
    AddOperator( &ccoprelcount );
    AddOperator( &ccoprelcount2 );
    AddOperator( &ccopkeywords );
    AddOperator( &ccopifthenelse );
    ccopifthenelse.SetRequestsArguments();
    AddOperator( &ccopifthenelse2 );
    ccopifthenelse2.SetRequestsArguments();
    AddOperator( &ccbetween );
    //AddOperator( &ccelapsedtime );
    AddOperator( &ccldistance );
    AddOperator( &cchashvalue ) ;
    AddOperator( &ccround );
    AddOperator( &ccint2real );
    AddOperator( &ccreal2int );
    AddOperator( &ccint2bool );
    AddOperator( &ccbool2int );
    AddOperator( &ccceil );
    AddOperator( &ccfloor );
    AddOperator( &ccchar );
    AddOperator( &ccnum2string );
    AddOperator( &ccisdefined );
    AddOperator( &ccassert );
    AddOperator( &cccompare );
    AddOperator( &ccgetminval );
    AddOperator( &ccgetmaxval );
    AddOperator( &cclength );

    AddOperator( &ccsin );
    AddOperator( &cccos );
    AddOperator( &cctan );
    AddOperator( &ccarcsin );
    AddOperator( &ccarccos );
    AddOperator( &ccarctan );
    AddOperator( &ccarctan2);
    AddOperator( &ccpi );
    AddOperator( &ccdeg2rad );
    AddOperator( &ccrad2deg );
    AddOperator( &cce );
    AddOperator( &cclogb );
    AddOperator( &ccpow );
    AddOperator( &ccdms2deg );

    AddOperator( setoptionInfo(), setoption_vm, setoption_tm );
    AddOperator( absInfo(), abs_vms, abs_sf, abs_tm );

    AddOperator (&ccbinand);
    AddOperator( &ccbinands);

    AddOperator (&switchOp);
    AddOperator (&int2longint);
    AddOperator (&longint2int);
    AddOperator (&rat);
    AddOperator (&chars);

#ifdef USE_PROGRESS
    ccopifthenelse.EnableProgress();
#endif



  }
  ~CcAlgebra1() {};

};


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
  return (new CcAlgebra1);
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
Statistics2Counters(bool reset, bool show)
{
  if( reset )
  {
    CcInt::intsCreated = 0; CcInt::intsDeleted = 0;
    CcReal::realsCreated = 0; CcReal::realsDeleted = 0;
    CcBool::boolsCreated = 0; CcBool::boolsDeleted = 0;
    CcString::stringsCreated = 0; CcString::stringsDeleted = 0;
  }
  Counter::getRef(Symbol::CTR_INT_Created()) = CcInt::intsCreated;
  Counter::getRef(Symbol::CTR_INT_Deleted()) = CcInt::intsDeleted;
  Counter::getRef(Symbol::CTR_REAL_Created()) = CcReal::realsCreated;
  Counter::getRef(Symbol::CTR_REAL_Deleted()) = CcReal::realsDeleted;
  Counter::getRef(Symbol::CTR_BOOL_Created()) = CcBool::boolsCreated;
  Counter::getRef(Symbol::CTR_BOOL_Deleted()) = CcBool::boolsDeleted;
  Counter::getRef(Symbol::CTR_STR_Created()) = CcString::stringsCreated;
  Counter::getRef(Symbol::CTR_STR_Deleted()) = CcString::stringsDeleted;

  Counter::reportValue(Symbol::CTR_INT_Created(), show);
  Counter::reportValue(Symbol::CTR_INT_Deleted(), show);
  Counter::reportValue(Symbol::CTR_REAL_Created(), show);
  Counter::reportValue(Symbol::CTR_REAL_Deleted(), show);
  Counter::reportValue(Symbol::CTR_BOOL_Created(), show);
  Counter::reportValue(Symbol::CTR_BOOL_Deleted(), show);
  Counter::reportValue(Symbol::CTR_STR_Created(), show);
  Counter::reportValue(Symbol::CTR_STR_Deleted(), show);
}


long
StdTypes::UpdateBasicOps(bool reset/*=false*/) {

  long& basicOps = Counter::getRef(Symbol::CTR_ATTR_BASIC_OPS());
  long& hashOps = Counter::getRef(Symbol::CTR_ATTR_HASH_OPS());
  long& compareOps = Counter::getRef(Symbol::CTR_ATTR_COMPARE_OPS());

  long& intHash = Counter::getRef(Symbol::CTR_INT_HASH());
  long& intLess  = Counter::getRef(Symbol::CTR_INT_LESS());
  long& intEqual = Counter::getRef(Symbol::CTR_INT_EQUAL());
  long& intCompare = Counter::getRef(Symbol::CTR_INT_COMPARE());

  if (reset)
  {
    intHash = 0;
    intLess = 0;
    intEqual = 0;
    intCompare = 0;
  }

  hashOps += intHash;

  compareOps += (intLess + intEqual + intCompare);

  basicOps += hashOps + compareOps;

  return basicOps;
}

void
StdTypes::ResetBasicOps() {
  UpdateBasicOps(true);
}

void
StdTypes::InitCounters(bool show) {
  Statistics2Counters(true, show);
}

void
StdTypes::SetCounterValues(bool show) {
  Statistics2Counters(false, show);
}


int
StdTypes::GetInt(const Word& w) {
   return Attribute::GetValue<CcInt, int>(w);
}


SEC_STD_REAL
StdTypes::GetReal(const Word& w) {
   return Attribute::GetValue<CcReal, SEC_STD_REAL>(w);
}


bool
StdTypes::GetBool(const Word& w) {
   return Attribute::GetValue<CcBool, bool>(w);
}

string
StdTypes::GetString(const Word& w) {
   return Attribute::GetValue<CcString, string>(w);
}
