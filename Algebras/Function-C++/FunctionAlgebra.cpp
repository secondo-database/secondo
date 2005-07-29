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

The sole purpose of this little algebra is to provide a type constructor ~map~ which can be used to store the list expressions defining functions (abstractions).

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
DummyInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

ListExpr
DummyOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

Word
DummyValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

Word
DummyValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
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
2.2 The Functions Needed

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
                             DummyCast,         NullSize, CheckMap,
                             0,
                             DummyInModel,      DummyOutModel,
                             DummyValueToModel, DummyValueListToModel );

class FunctionAlgebra : public Algebra
{
 public:
  FunctionAlgebra() : Algebra()
  {
    AddTypeConstructor( &functionMap );
  }
  ~FunctionAlgebra() {};
};

FunctionAlgebra functionAlgebra;

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
InitializeFunctionAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&functionAlgebra);
}

} // end of namespace FunctionAlgebra
