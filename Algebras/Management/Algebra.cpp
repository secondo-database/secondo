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

July 2004, M. Spiekermann. Counters for type constructors and operators were
introduced to assert correct vector indexes and avoid segmentation faults.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

*/

using namespace std;

#include "AlgebraManager.h"
#include "Algebra.h"
#include "NestedList.h"
#include "SecondoSystem.h"

NestedList *nl;
QueryProcessor *qp;

/* Member functions of class Operator: */
int
Operator::SimpleSelect( ListExpr )
{
  return (0);
}

Operator::Operator( const string& nm,
                    const string& spec,
                    const int noF,
                    ValueMapping vms[],
                    SelectFunction sf,
                    TypeMapping tm )
{
  name           = nm;
  specString     = spec;
  numOfFunctions = noF;
  selectFunc     = sf;
  valueMap       = new ValueMapping[numOfFunctions];
  typeMap        = tm;

  for ( int i = 0; i < numOfFunctions; i++ )
    AddValueMapping( i, vms[i] );
}

Operator::Operator( const string& nm,
                    const string& spec,
                    ValueMapping vm,
                    SelectFunction sf,
                    TypeMapping tm )
{
  name           = nm;
  specString     = spec;
  numOfFunctions = 1;
  selectFunc     = sf;
  valueMap       = new ValueMapping[1];
  typeMap        = tm;

  AddValueMapping( 0, vm );
}

Operator::~Operator()
{
  delete[] valueMap;
}

bool
Operator::AddValueMapping( const int index, ValueMapping f )
{
  if ( index < numOfFunctions && index >= 0 )
  {
    valueMap[index] = f;
    return (true);
  }
  else
  {
    return (false);
  }
}

string
Operator::Specification()
{
  return (specString);
}

ListExpr
Operator::CallTypeMapping( ListExpr argList )
{
  return ((*typeMap)( argList ));
}

/* Member functions of Class TypeConstructor */
bool
TypeConstructor::DefaultOpen( SmiRecord& valueRecord,
                              size_t& offset,
                              const ListExpr typeInfo,
                              Word& value )
{
  ListExpr valueList = 0;
  string valueString;
  int valueLength;

  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  bool correct;
  valueRecord.Read( &valueLength, sizeof( valueLength ), offset );
  offset += sizeof( valueLength );
  char* buffer = new char[valueLength];
  valueRecord.Read( buffer, valueLength, offset );
  offset += valueLength;
  valueString.assign( buffer, valueLength );
  delete []buffer;
  nl->ReadFromString( valueString, valueList );
  value = RestoreFromList( nl->First(typeInfo), nl->First( valueList ), 1, errorInfo, correct );
  if ( errorInfo != 0 )
  {
    nl->Destroy( errorInfo );
  }
  nl->Destroy( valueList );
  return (true);
}

bool
TypeConstructor::DefaultSave( SmiRecord& valueRecord,
                              size_t& offset,
                              const ListExpr typeInfo,
                              Word& value )
{
  ListExpr valueList;
  string valueString;
  int valueLength;

  valueList = SaveToList( nl->First(typeInfo), value );
  valueList = nl->OneElemList( valueList );
  nl->WriteToString( valueString, valueList );
  valueLength = valueString.length();
  valueRecord.Write( &valueLength, sizeof( valueLength ), offset );
  offset += sizeof( valueLength );
  valueRecord.Write( valueString.data(), valueString.length(), offset );
  offset += valueString.length();

  nl->Destroy( valueList );
  return (true);
}

Word
TypeConstructor::DummyCreate( const ListExpr typeInfo )
{
  return (SetWord( Address( 0 ) ));
}

void
TypeConstructor::DummyDelete( const ListExpr typeInfo, Word& w )
{
}

void
TypeConstructor::DummyClose( const ListExpr typeInfo, Word& w )
{
}

Word
TypeConstructor::DummyClone( const ListExpr typeInfo, const Word& w )
{
  return (SetWord( Address( 0 ) ));
}

int
TypeConstructor::DummySizeOf()
{
  return (0);
}

TypeConstructor::TypeConstructor( const string& nm,
                                  TypeProperty prop,
                                  OutObject out,
                                  InObject in,
                                  OutObject saveToList,
                                  InObject restoreFromList,
                                  ObjectCreation create,
                                  ObjectDeletion del,
                                  ObjectOpen open,
                                  ObjectSave save,
                                  ObjectClose close,
                                  ObjectClone clone,
                                  ObjectCast ca,
                                  ObjectSizeof sizeOf,
                                  TypeCheckFunction tcf )
{
  name                 = nm;
  propFunc             = prop;
  outFunc              = out;
  inFunc               = in;
  saveToListFunc       = saveToList;
  restoreFromListFunc  = restoreFromList;
  createFunc           = create;
  deleteFunc           = del;
  openFunc             = open;
  saveFunc             = save;
  closeFunc            = close;
  cloneFunc            = clone;
  castFunc             = ca;
  sizeofFunc           = sizeOf;
  typeCheckFunc        = tcf;
}

TypeConstructor::~TypeConstructor()
{
}

void
TypeConstructor::AssociateKind( const string& kindName )
{
  if ( kindName.length() > 0 )
  {
    kinds.push_back( kindName );
  }
}

ListExpr
TypeConstructor::Property()
{
  ListExpr property = (*propFunc)();
  return (property);
}

ListExpr
TypeConstructor::Out( ListExpr type, Word value )
{
  return ((*outFunc)( type, value ));
}

Word
TypeConstructor::In( const ListExpr typeInfo, const ListExpr value,
                     const int errorPos, ListExpr& errorInfo, bool& correct )
{
  return ((*inFunc)( typeInfo, value, errorPos, errorInfo, correct ));
}

ListExpr
TypeConstructor::SaveToList( ListExpr type, Word value )
{
  if( saveToListFunc != 0 )
    return ((*saveToListFunc)( type, value ));
  else
    return ((*outFunc)( type, value ));
}

Word
TypeConstructor::RestoreFromList( const ListExpr typeInfo, const ListExpr value,
                                  const int errorPos, ListExpr& errorInfo, bool& correct )
{
  if( restoreFromListFunc != 0 )
    return ((*restoreFromListFunc)( typeInfo, value, errorPos, errorInfo, correct ));
  else
    return ((*inFunc)( typeInfo, value, errorPos, errorInfo, correct ));
}

Word
TypeConstructor::Create( const ListExpr typeInfo )
{
  return ((*createFunc)( typeInfo ));
}

void
TypeConstructor::Delete( const ListExpr typeInfo, Word& w )
{
  (*deleteFunc)( typeInfo, w );
}

bool
TypeConstructor::Open( SmiRecord& valueRecord,
                       size_t& offset,
                       const ListExpr typeInfo,
                       Word& value )
{
  if ( openFunc != 0 )
  {
    return ((*openFunc)( valueRecord, offset, typeInfo, value ));
  }
  else
  {
    return (DefaultOpen( valueRecord, offset, typeInfo, value ));
  }
}

bool
TypeConstructor::Save( SmiRecord& valueRecord,
                       size_t& offset,
                       const ListExpr typeInfo,
                       Word& value )
{
  if ( saveFunc != 0 )
  {
    return ((*saveFunc)( valueRecord, offset, typeInfo, value ));
  }
  else
  {
    return (DefaultSave( valueRecord, offset, typeInfo, value ));
  }
}

void
TypeConstructor::Close( const ListExpr typeInfo, Word& w )
{
  (*closeFunc)( typeInfo, w );
}

Word
TypeConstructor::Clone( const ListExpr typeInfo, const Word& w )
{
  return (*cloneFunc)( typeInfo, w );
}

int
TypeConstructor::SizeOf()
{
  return (*sizeofFunc)();
}

/* Member functions of Class Algebra: */
Algebra::Algebra() : tcsNum(0), opsNum(0)
{
}

Algebra::~Algebra()
{
}

void
Algebra::AddTypeConstructor( TypeConstructor* tc )
{
  tcs.push_back( tc );
  tcsNum++;
}

void
Algebra::AddOperator( Operator* op )
{
  ops.push_back( op );
  opsNum++;
}

