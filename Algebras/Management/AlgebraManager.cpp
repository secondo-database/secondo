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

August 2004, M. Spiekermann. This comment was inserted to make it a PD-File. Moreover, 
implementation of ~GetAlgebraName~ was done.

*/

#include <string>
#include <algorithm>
using namespace std;

#include "AlgebraManager.h"
#include "Algebra.h"
#include "CharTransform.h"
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
  InitOpPtrField();
}

AlgebraManager::~AlgebraManager()
{
}

ListExpr
AlgebraManager::ListAlgebras()
{
  int j;
  ListExpr list, lastElem = 0;
    
  list = nl->TheEmptyList();
 
  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraInit != 0 )
      {
        if ( list == nl->TheEmptyList() )
        {
          list = nl->OneElemList( nl->SymbolAtom( (*getAlgebraEntry)( j ).algebraName ) ); 
          lastElem = list;
        }
        else
        {
          lastElem = nl->Append( lastElem, nl->SymbolAtom( (*getAlgebraEntry)( j ).algebraName ) );
        }
      }
    }
  } 
  return (list);
}

int
AlgebraManager::GetAlgebraId(const string& algName)
{
  int j;
  
  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraInit != 0 )
      {
        if ( !(algName.compare( (*getAlgebraEntry)( j ).algebraName )) )
	  return (*getAlgebraEntry)( j ).algebraId;
      }
    }
  }
  return 0;
}

const string& 
AlgebraManager::GetAlgebraName( const int algId ) {
  
	static const string unknown("UnknownAlgebra");
	assert( algId >= 0 && algId <= maxAlgebraId );
	map<int, string>::const_iterator it = algebraNames.find(algId);
	
	if ( it != algebraNames.end() ) {
	  return it->second;
	} else {
		return unknown;
	}    
}

void
AlgebraManager::LoadAlgebras()
{
  QueryProcessor* qp = SecondoSystem::GetQueryProcessor();
  TypeConstructor* tc;
  int j, k;
  
  for ( j = 0; (*getAlgebraEntry)( j ).algebraId > 0; j++ )
  {
    string algNameStr = (*getAlgebraEntry)( j ).algebraName;
    int algId = (*getAlgebraEntry)( j ).algebraId;
    
    if ( (*getAlgebraEntry)( j ).useAlgebra )
    {
      if ( (*getAlgebraEntry)( j ).algebraInit != 0 )
      {
        algebraNames[algId] = algNameStr;
        algebra[algId] = ((*getAlgebraEntry)( j ).algebraInit)( nl, qp );
      }
      else
      {
        bool loaded = false;
        string libraryName  = string( "lib" ) + algNameStr;
        string initFuncName = string( "Initialize" ) + algNameStr;
        (*getAlgebraEntry)( j ).dynlib = new DynamicLibrary();
        transform( libraryName.begin(), libraryName.end(), libraryName.begin(), ToLowerProperFunction );
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
  return (algebra[algebraId]->GetOperator( operatorId )->GetName());
}

ListExpr
AlgebraManager::Specs( int algebraId, int operatorId )
{
  ListExpr spec = nl->TheEmptyList();
  nl->ReadFromString(
        algebra[algebraId]->GetOperator( operatorId )->GetSpecString(),
        spec );
  return (spec);
}



void
AlgebraManager::InitOpPtrField() {

  for ( int alg = 0; alg < MAX_ALG; alg++ ) {
    for ( int op = 0; op < MAX_OP; op++ ) {      
        opPtrField[alg][op] = 0;
      }}
}


// Currently not used!
//
//TypeMapping
//AlgebraManager::ExecuteCost( int algebraId, int operatorId )
//{
//  return (algebra[algebraId]->GetOperator( operatorId )->costMap);
//}

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

OutObject
AlgebraManager::SaveToListObj( int algebraId, int typeId )
{
  if( algebra[algebraId]->GetTypeConstructor( typeId )->saveToListFunc != 0 )
    return (algebra[algebraId]->GetTypeConstructor( typeId )->saveToListFunc);
  else
    return (algebra[algebraId]->GetTypeConstructor( typeId )->outFunc);
}

InObject
AlgebraManager::RestoreFromListObj( int algebraId, int typeId )
{
  if( algebra[algebraId]->GetTypeConstructor( typeId )->restoreFromListFunc != 0 )
    return (algebra[algebraId]->GetTypeConstructor( typeId )->restoreFromListFunc);
  else 
    return (algebra[algebraId]->GetTypeConstructor( typeId )->inFunc);
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

bool
AlgebraManager::OpenObj( const int algebraId, const int typeId,
                         SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->
    Open( valueRecord, offset, typeInfo, value ));
}

bool
AlgebraManager::SaveObj( const int algebraId, const int typeId,
                         SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->
    Save( valueRecord, offset, typeInfo, value ));
}

ObjectClose
AlgebraManager::CloseObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->closeFunc);
}

ObjectClone
AlgebraManager::CloneObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->cloneFunc);
}

ObjectCast
AlgebraManager::Cast( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->castFunc);
}

ObjectSizeof
AlgebraManager::SizeOfObj( int algebraId, int typeId )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->sizeofFunc);
}

bool
AlgebraManager::PersistModel( const int algebraId, const int typeId,
                              const PersistDirection dir,
                              SmiRecord& modelRecord,
                              const ListExpr typeExpr, Word& model )
{
  return (algebra[algebraId]->GetTypeConstructor( typeId )->
    PersistModel( dir, modelRecord, typeExpr, model ));
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
  ListExpr tempErrorInfo = errorInfo;
  multimap<string,TypeCheckFunction>::iterator pos;
  for ( pos  = kindTable.lower_bound( kindName );
        pos != kindTable.upper_bound( kindName );
        pos++ )
  {
    if ( (pos->second)( type, errorInfo ) )
    {
      errorInfo = tempErrorInfo;
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

