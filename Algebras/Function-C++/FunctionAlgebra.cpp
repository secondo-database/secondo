/*
1 Implementation Module Function Algebra

January 26, 2001 RHG

April 2002 Ulrich Telle Port to C++

The sole purpose of this little algebra is to provide a type constructor ~map~ which can be used to store the list expressions defining functions (abstractions).

*/

using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"

static NestedList* nl;
static QueryProcessor* qp;

/*
2.1 Dummy Functions

*/

static ListExpr
FunctionProperty()
{
  return (nl->TheEmptyList());
}

static Word
DummyInModel( ListExpr typeExpr, ListExpr list, int objNo )
{
  return (SetWord( Address( 0 ) ));
}

static ListExpr
DummyOutModel( ListExpr typeExpr, Word model )
{
  return (0);
}

static Word
DummyValueToModel( ListExpr typeExpr, Word value )
{
  return (SetWord( Address( 0 ) ));
}

static Word
DummyValueListToModel( const ListExpr typeExpr, const ListExpr valueList,
                       const int errorPos, ListExpr& errorInfo, bool& correct )
{
  correct = true;
  return (SetWord( Address( 0 ) ));
}

static Word
NoSpace( const ListExpr typeInfo )
{
  return (SetWord( Address( 0 ) ));
}

static void
DoNothing( Word& w )
{
  w.addr = 0;
}

static Word
CloneNothing( const Word& w )
{
  return SetWord( Address(0) );
}

static int
SizeOfNothing()
{
  return 0;
}

/*
2.2 The Functions Needed

*/
static Word
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

static ListExpr
OutMap( ListExpr typeInfo, Word value )
{
  return (value.list);
}

static void*
DummyCast( void* addr, SmiRecordFile* )
{
  return (0);
}

static bool
CheckMap( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( nl->First( type ), "map" ));
}

TypeConstructor functionMap( "map",             FunctionProperty,
                             OutMap,            InMap,         NoSpace,
                             DoNothing,         0, 0,          DoNothing, CloneNothing,
                             DummyCast,         SizeOfNothing, CheckMap,
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

