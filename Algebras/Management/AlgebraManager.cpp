#include <string>
#include <algorithm>
using namespace std;

#include "AlgebraManager.h"
#include "Algebra.h"
#include "SecondoSystem.h"
#include "DynamicLibrary.h"

/*
1.2 List of available Algebras

*/

/*
Creation of the prototypes of the initilization functions of all requested
algebra modules.

*/
#define ALGEBRA_INCLUDE ALGEBRA_PROTO_INCLUDE
#define ALGEBRA_EXCLUDE ALGEBRA_PROTO_EXCLUDE
#define ALGEBRA_DYNAMIC ALGEBRA_PROTO_DYNAMIC
#include "AlgebraList.i"

/*
Creation of the list of all requested algebra modules.
The algebra manager uses this list to initialize the algebras and
to access the type constructor and operator functions provided by
the algebra modules.

*/

#undef ALGEBRA_INCLUDE
#undef ALGEBRA_EXCLUDE
#undef ALGEBRA_DYNAMIC
#define ALGEBRA_INCLUDE ALGEBRA_LIST_INCLUDE
#define ALGEBRA_EXCLUDE ALGEBRA_LIST_EXCLUDE
#define ALGEBRA_DYNAMIC ALGEBRA_LIST_DYNAMIC

static
AlgebraListEntry& GetAlgebraEntry( const int j )
{
ALGEBRA_LIST_START
#include "AlgebraList.i"
ALGEBRA_LIST_END
/*
is the static list of all available algebra modules.

*/
  return (algebraList[j]);
}

AlgebraManager::AlgebraManager( NestedList& nlRef )
{
  int j;
  nl = &nlRef;
  maxAlgebraId = 0;
  for ( j = 0; GetAlgebraEntry( j ).algebraId > 0; j++ )
  {
    if ( GetAlgebraEntry( j ).useAlgebra )
    {
      if ( GetAlgebraEntry( j ).algebraId > maxAlgebraId )
      {
        maxAlgebraId = GetAlgebraEntry( j ).algebraId;
      }
    }
  }
  algebra.resize( maxAlgebraId+1 );
  algType.resize( maxAlgebraId+1 );
  for ( j = 0; j <= maxAlgebraId; j++ )
  {
    algebra[j] = 0;
    algType[j] = UndefinedLevel;
  }
}

AlgebraManager::~AlgebraManager()
{
}

void
AlgebraManager::LoadAlgebras()
{
  QueryProcessor* qp = SecondoSystem::GetQueryProcessor();
  TypeConstructor* tc;
  int j, k;

  for ( j = 0; GetAlgebraEntry( j ).algebraId > 0; j++ )
  {
    if ( GetAlgebraEntry( j ).useAlgebra )
    {
      if ( GetAlgebraEntry( j ).algebraInit != 0 )
      {
        algebra[GetAlgebraEntry( j ).algebraId] =
          (GetAlgebraEntry( j ).algebraInit)( nl, qp );
      }
      else
      {
        bool loaded = false;
        string libraryName  = string( "lib" ) + GetAlgebraEntry( j ).algebraName;
        string initFuncName = string( "Initialize" ) + GetAlgebraEntry( j ).algebraName;
        GetAlgebraEntry( j ).dynlib = new DynamicLibrary();
        transform( libraryName.begin(), libraryName.end(), libraryName.begin(), tolower );
        if ( GetAlgebraEntry( j ).dynlib->Load( libraryName ) )
        {
          AlgebraInitFunction initFunc =
            (AlgebraInitFunction) GetAlgebraEntry( j ).dynlib->GetFunctionAddress( initFuncName );
          if ( initFunc != 0 )
          {
            algebra[GetAlgebraEntry( j ).algebraId] = (initFunc)( nl, qp );
            loaded = true;
          }
          else
          {
            GetAlgebraEntry( j ).dynlib->Unload();
          }
        }
        if ( !loaded )
        {
          delete GetAlgebraEntry( j ).dynlib;
          GetAlgebraEntry( j ).dynlib = 0;
          continue;
        }
      }
      algType[GetAlgebraEntry( j ).algebraId] = GetAlgebraEntry( j ).level;
      for ( k = 0; k < algebra[GetAlgebraEntry( j ).algebraId]->GetNumTCs(); k++ )
      {
        tc = algebra[GetAlgebraEntry( j ).algebraId]->GetTypeConstructor( k );
        for ( vector<string>::size_type idx = 0; idx < tc->kinds.size(); idx++ )
        {
          if ( tc->kinds[idx] != "" )
          {
            kindTable.insert( make_pair( tc->kinds[idx], tc->typeCheckFunc ) );
          }
        }
      }
    }
  }
}

void
AlgebraManager::UnloadAlgebras()
{
  for ( int j = 0; GetAlgebraEntry( j ).algebraId > 0; j++ )
  {
    if ( GetAlgebraEntry( j ).useAlgebra && GetAlgebraEntry( j ).dynlib != 0 )
    {
      GetAlgebraEntry( j ).dynlib->Unload();
      delete GetAlgebraEntry( j ).dynlib;
      GetAlgebraEntry( j ).dynlib = 0;
    }
  }
}

bool
AlgebraManager::IsAlgebraLoaded( const int algebraId )
{
  bool loaded = false;
  if ( algebraId >= 1 && algebraId <= maxAlgebraId )
  {
    loaded = (algType[algebraId] != UndefinedLevel);
  }
  return (loaded);
}

bool
AlgebraManager::IsAlgebraLoaded( const int algebraId,
                                 const AlgebraLevel level )
{
  bool loaded = false;
  if ( algebraId >= 1 && algebraId <= maxAlgebraId )
  {
    loaded = (algType[algebraId] == level);
  }
  return (loaded);
}

int
AlgebraManager::CountAlgebra()
{
  int count = 0;
  for ( int j = 1; j <= maxAlgebraId; j++ )
  {
    if ( algType[j] != UndefinedLevel )
    {
      count++;
    }
  }
  return (count);
}

int
AlgebraManager::CountAlgebra( const AlgebraLevel level )
{
  int count = 0;
  if ( level != UndefinedLevel )
  {
    for ( int j = 1; j <= maxAlgebraId; j++ )
    {
      if ( algType[j] == level || algType[j] == HybridLevel )
      {
        count++;
      }
    }
  }
  return (count);
}

bool
AlgebraManager::NextAlgebraId( int& algebraId, AlgebraLevel& level )
{
  bool found = false;
  algebraId++;
  if ( algebraId > 0 && algebraId <= maxAlgebraId )
  {
    while ( !found && algebraId <= maxAlgebraId )
    {
      if ( algType[algebraId] != UndefinedLevel )
      {
        found = true;
        level = algType[algebraId];
      }
      else
      {
        algebraId++;
      }
    }
  }
  else
  {
    algebraId = 0;
    level = UndefinedLevel;
  }
  return (found);
}

bool
AlgebraManager::NextAlgebraId( const AlgebraLevel level, int& algebraId )
{
  bool found = false;
  algebraId++;
  if ( algebraId > 0 && algebraId <= maxAlgebraId )
  {
    while ( !found && algebraId <= maxAlgebraId )
    {
      if ( algType[algebraId] == level ||
           algType[algebraId] == HybridLevel )
      {
        found = true;
      }
      else
      {
        algebraId++;
      }
    }
  }
  else
  {
    algebraId = 0;
  }
  return (found);
}

int
AlgebraManager::OperatorNumber( int algebraId )
{
  return (algebra[algebraId]->GetNumOps());
}

string
AlgebraManager::Ops( int algebraId, int operatorId )
{
  return (algebra[algebraId]->GetOperator( operatorId )->name);
}

ListExpr
AlgebraManager::Specs( int algebraId, int operatorId )
{
  ListExpr spec = nl->TheEmptyList();
  nl->ReadFromString(
        algebra[algebraId]->GetOperator( operatorId )->specString,
        spec );
  return (spec);
}

SelectFunction
AlgebraManager::Select( int algebraId, int operatorId )
{
  return (algebra[algebraId]->GetOperator( operatorId )->selectFunc);
}

ValueMapping
AlgebraManager::Execute( int algebraId, int opFunId )
{
/*
Parameter ~opFunId~ holds the operator id as well as the function id.
The operator id is stored in the lower 2 bytes, the function id is stored
in the upper 2 bytes.

*/
  int opId  = opFunId % 65536;
  int funId = opFunId / 65536;
  return (algebra[algebraId]->GetOperator( opId )->valueMap[funId]);
}

ModelMapping
AlgebraManager::TransformModel( int algebraId, int opFunId )
{
  int opId  = opFunId % 65536;
  int funId = opFunId / 65536;
  return (algebra[algebraId]->GetOperator( opId )->modelMap[funId]);
}

TypeMapping
AlgebraManager::TransformType( int algebraId, int operatorId )
{
  return (algebra[algebraId]->GetOperator( operatorId )->typeMap);
}

TypeMapping
AlgebraManager::ExecuteCost( int algebraId, int operatorId )
{
  return (algebra[algebraId]->GetOperator( operatorId )->costMap);
}

/*

*/

int
AlgebraManager::ConstrNumber( int algebraId )
{
  return (algebra[algebraId]->GetNumTCs());
}

string
AlgebraManager::Constrs( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->name);
}

ListExpr
AlgebraManager::Props( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->Property());
}

InObject
AlgebraManager::InObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->inFunc);
}

OutObject
AlgebraManager::OutObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->outFunc);
}

ObjectCreation
AlgebraManager::CreateObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->createFunc);
}

ObjectDeletion
AlgebraManager::DeleteObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->deleteFunc);
}

ObjectCast
AlgebraManager::Cast( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->castFunc);
}

bool
AlgebraManager::PersistValue( const int algebraId, const int typeId,
                              const PersistDirection dir,
                              SmiRecord& valueRecord,
                              const string& type, Word& value )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->
    PersistValue( dir, valueRecord, type, value ));
}

bool
AlgebraManager::PersistModel( const int algebraId, const int typeId,
                              const PersistDirection dir,
                              SmiRecord& modelRecord,
                              const string& type, Word& model )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->
    PersistModel( dir, modelRecord, type, model ));
}

InModelFunction
AlgebraManager::InModel( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->inModelFunc);
}

OutModelFunction
AlgebraManager::OutModel( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->outModelFunc);
}

ValueToModelFunction
AlgebraManager::ValueToModel( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->valueToModelFunc);
}

ValueListToModelFunction
AlgebraManager::ValueListToModel( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->valueListToModelFunc);
}

TypeCheckFunction
AlgebraManager::TypeCheck( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->typeCheckFunc);
}

bool
AlgebraManager::CheckKind( const string& kindName,
                           const ListExpr type,
                           ListExpr& errorInfo )
{
  ListExpr typeErrors = nl->OneElemList( nl->TheEmptyList() );
  multimap<string,TypeCheckFunction>::iterator pos;
  for ( pos  = kindTable.lower_bound( kindName );
        pos != kindTable.upper_bound( kindName );
        pos++ )
  {
    if ( (pos->second)( type, typeErrors ) )
    {
      errorInfo = nl->TheEmptyList();
      return (true);
    }
  }
  errorInfo = nl->Append(
                errorInfo,
                nl->ThreeElemList(
                  nl->IntAtom( 60 ),
                  nl->SymbolAtom( kindName ),
                  type ) );
  return (false);
}

