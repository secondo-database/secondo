#include <string>
#include <algorithm>
using namespace std;

#include "AlgebraManager.h"
#include "Algebra.h"
#include "SecondoSystem.h"
#include "DynamicLibrary.h"

AlgebraManager::AlgebraManager( NestedList& nlRef, GetAlgebraEntryFunction getAlgebraEntryFunc )
{
  int j;
  nl = &nlRef;
  getAlgebraEntry = getAlgebraEntryFunc;
  maxAlgebraId = 0;
  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraId > maxAlgebraId )
      {
        maxAlgebraId = (*getAlgebraEntry)( j ).algebraId;
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

  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraInit != 0 )
      {
        algebra[(*getAlgebraEntry)( j ).algebraId] =
          ((*getAlgebraEntry)( j ).algebraInit)( nl, qp );
      }
      else
      {
        bool loaded = false;
        string libraryName  = string( "lib" ) + (*getAlgebraEntry)( j ).algebraName;
        string initFuncName = string( "Initialize" ) + (*getAlgebraEntry)( j ).algebraName;
        (*getAlgebraEntry)( j ).dynlib = new DynamicLibrary();
        transform( libraryName.begin(), libraryName.end(), libraryName.begin(), tolower );
        if ( (*getAlgebraEntry)( j ).dynlib->Load( libraryName ) )
        {
          AlgebraInitFunction initFunc =
            (AlgebraInitFunction) (*getAlgebraEntry)( j ).dynlib->GetFunctionAddress( initFuncName );
          if ( initFunc != 0 )
          {
            algebra[(*getAlgebraEntry)( j ).algebraId] = (initFunc)( nl, qp );
            loaded = true;
          }
          else
          {
            (*getAlgebraEntry)( j ).dynlib->Unload();
          }
        }
        if ( !loaded )
        {
          delete (*getAlgebraEntry)( j ).dynlib;
          (*getAlgebraEntry)( j ).dynlib = 0;
          continue;
        }
      }
      algType[(*getAlgebraEntry)( j ).algebraId] = (*getAlgebraEntry)( j ).level;
      for ( k = 0; k < algebra[(*getAlgebraEntry)( j ).algebraId]->GetNumTCs(); k++ )
      {
        tc = algebra[(*getAlgebraEntry)( j ).algebraId]->GetTypeConstructor( k );
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
  for ( int j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra && (*getAlgebraEntry)( j ).dynlib != 0 )
    {
      (*getAlgebraEntry)( j ).dynlib->Unload();
      delete (*getAlgebraEntry)( j ).dynlib;
      (*getAlgebraEntry)( j ).dynlib = 0;
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

