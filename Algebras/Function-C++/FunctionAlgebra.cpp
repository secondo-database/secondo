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

1 Implementation Module Function Algebra

January 26, 2001 RHG

April 2002 Ulrich Telle Port to C++

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

The sole purpose of this little algebra is to provide a type constructor ~map~ 
which can be used to store the list expressions defining functions 
(abstractions).

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"

extern NestedList* nl;
extern QueryProcessor *qp;

namespace FunctionAlgebra{

/*
2.1 Dummy Functions

The next function defines the type property of type constructor ~map~.

*/
ListExpr
FunctionProperty()
{
  ListExpr remarkslist = nl->TextAtom();
  nl->AppendText(remarkslist,"Stores list expressions defining functions "
  "(internal use).");

  return (nl->TwoElemList(
            nl->OneElemList(nl->StringAtom("Remarks")),
            nl->OneElemList(remarkslist)));
}

Word
NoSpace( const ListExpr typeInfo )
{
  return (SetWord( Address( 0 ) ));
}

void
DoNothing( Word& w )
{
  w.addr = 0;
}

Word
CloneNothing( const Word& w )
{
  return SetWord( Address(0) );
}

int
SizeOfNothing()
{
  return 0;
}

/*
2.2 Type Constructor ~map~

*/
Word
InMap( const ListExpr typeInfo, const ListExpr instance,
       const int errorPos, ListExpr& errorInfo, bool& correct )
{
/*
We don't do any checks here; any list expression will be accepted.
Errors will be found when the function is used, i.e., sent to the
query processor.

*/
  return (SetWord( instance ));
}

ListExpr
OutMap( ListExpr typeInfo, Word value )
{
  return (value.list);
}

void*
DummyCast( void* addr )
{
  return (0);
}

int
NullSize()
{
  return 0;
}

bool
CheckMap( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( nl->First( type ), "map" ));
}

TypeConstructor functionMap( "map",             FunctionProperty,
                             OutMap,            InMap,
                             0,                 0,
                             NoSpace,           DoNothing,
                             0,                 0,
                             DoNothing,         CloneNothing,
                             DummyCast,         NullSize, CheckMap );

/*
2.3 Type Operator ~ANY~

Type operators are used only for inferring argument types of parameter
functions. They have a type mapping but no evaluation function.

2.3.1 Type mapping function of operator ~ANY~

The type operator ~ANY~ corresponds to the type of the first argument.

----    x      -> x
----

*/
ListExpr ANYTypeMap( ListExpr args )
{
  return nl->First( args );
}

const string ANYSpec =
   "(( \"Signature\" \"Syntax\" \"Meaning\" \"Remarks\" )"
   "( <text>x -> x</text--->"
   "<text>type operator</text--->"
   "<text>Simply returns the type of the first argument.</text--->"
   "<text></text---> ))";

Operator ANY (
      "ANY",
      ANYSpec,
      0,
      Operator::SimpleSelect,
      ANYTypeMap );

/*
2.4 Operator ~within~

2.4.1 Type mapping function of operator ~within~

Result type of within operation.

----    ( a x (a -> b) ) -> b
----

*/
ListExpr WithinTypeMap(ListExpr args)
{
  ListExpr first, second;
  string argstr1, argstr2;

  CHECK_COND( nl->ListLength(args) == 2,
              "Operator within expects a list of length two.");

  first = nl->First(args);
  nl->WriteToString(argstr1, first);
  second  = nl->Second(args);
  nl->WriteToString(argstr2, first);

  CHECK_COND( nl->ListLength( second ) == 3 &&
              nl->IsEqual( nl->First( second ), "map" ),
              "Operator within expects a mapping function as the second argument, but gets\n" + 
              argstr2 + "." ); 

  CHECK_COND( nl->Equal( first, nl->Second(second) ),
              "Operator within expects that the first argument and the argument\n" 
              "of the mapping function are equal, but gets\n" 
              "First argument: " + argstr1 + "\n" +
              "Mapping argument: " + argstr2 + "." );

  return nl->Third( second );
}

/*
2.4.2 Selection function of operator ~within~

*/
int
WithinSelect( ListExpr args )
{
  if( nl->IsAtom( nl->Third( nl->Second( args ) ) ) )
    return 0;

  if( nl->IsEqual( nl->First( nl->Third( nl->Second( args ) ) ), "stream" ) )
    return 1;

  return 0;
}

/*
2.4.3 Value mapping function of operator ~within~

*/
int 
Within_s(Word* args, Word& result, int message, Word& local, Supplier s)
{
  ArgVectorPointer funArgs;
  Word w;

  switch ( message )
  {
    case OPEN:
      funArgs = qp->Argument( args[1].addr );
      qp->Request( args[0].addr, w );
      (*funArgs)[0] = w;
      qp->Open( args[1].addr );
      return 0;

    case REQUEST:
      qp->Request( args[1].addr, result );
      if( qp->Received( args[1].addr ) )
        return YIELD;
      return CANCEL;

    case CLOSE:
      qp->Close( args[1].addr );
      return 0;
  }

  return 0;
}

int
Within_o(Word* args, Word& result, int message, Word& local, Supplier s)
{
  ArgVectorPointer funArgs = qp->Argument( args[1].addr );
  (*funArgs)[0] = args[0];
  qp->Request( args[1].addr, result );

  return 0;
}

ValueMapping withinmap[] = { Within_o, Within_s };

/*

2.4.3 Specification of operator ~within~

*/
const string WithinSpec  = "( ( \"Signature\" \"Syntax\" \"Meaning\" "
                        "\"Example\" ) "
                        "( <text>a x (a -> b) -> b</text--->"
                        "<text>_ within [ fun ]</text--->"
                        "<text>Calls the function passing as argument "
                        "its own first argument.</text--->"
                        "<text>query plz createbtree[Ort] within[fun( index: ANY ) "
                        "Orte feed {o} loopjoin[index plz "
                        "exactmatch[.Ort_o]] consume]</text--->))";

/*
2.4.3 Definition of operator ~within~

*/
Operator within (
         "within",                  // name
         WithinSpec,                // specification
         2, 	                   // the number of overloaded functions
         withinmap,                 // value mapping function array
         WithinSelect,              // the selection function
         WithinTypeMap              // type mapping
);

/*
3 Creating the Algebra

*/
class FunctionAlgebra : public Algebra
{
 public:
  FunctionAlgebra() : Algebra()
  {
    AddTypeConstructor( &functionMap );
    AddOperator( &ANY );
    AddOperator( &within );
  }
  ~FunctionAlgebra() {};
};

FunctionAlgebra functionAlgebra;

/*
4 Initialization

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
InitializeFunctionAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&functionAlgebra);
}

} // end of namespace FunctionAlgebra
