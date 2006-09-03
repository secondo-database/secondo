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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Main Memory Relational Algebra

March 2003 Victor Almeida created the new Relational Algebra organization

November 2004 M. Spiekermann. The declarations of the PrivateRelation have been
moved to the files RelationPersistent.h and RelationMainMemory.h. This was
necessary to implement some little functions as inline functions. Moreover, the
Constructor of the TupleBuffer has been modified since it was changed in
RelationAlgebra.h to set the maximum amount of memory for the buffer.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes. Some assertions were removed, since the code is
stable.

1 Overview

The Relational Algebra basically implements two type constructors, namely ~tuple~ and ~rel~.
More information about the Relational Algebra can be found in the RelationAlgebra.h header
file.

This file contains the implementation of the Main Memory Relational Algebra, where the
type constructors ~tuple~ and ~rel~ are kept in main memory. The ~tuple~ data type is
an array of attributes and the ~rel~ is an array of tuples. FLOBs are represented in
memory only. Relations are stored using In and Out function, i.e., they are stored
using list representation. A schema of a relation can be viewed in the figure below:

                Figure 1: Example schema of a main memory relation. [MainMemoryRelation.eps]

2 Defines, includes, and constants

*/
#ifndef RELALG_PERSISTENT
/*
This ~RELALG\_PERSISTENT~ defines which kind of relational algebra is to be compiled.
If it is set, the persistent version of the relational algebra will be compiled, and
otherwise, the main memory version will be compiled.

*/

using namespace std;

#include <vector>

#include "RelationAlgebra.h"
#include "SecondoSystem.h"
#include "RelationMainMemory.h"


extern NestedList *nl;
extern AlgebraManager* am;

RelationCache cache( 20 );

/*
These global variables are used for caching relations.

3 Type constructor ~tuple~

3.3 Implementation of the class ~Tuple~

This class implements the memory representation of the type constructor ~tuple~. A tuple is simply
an array of attributes, as it can be seen in the definition of the ~PrivateTuple~ class.

*/
Tuple *Tuple::RestoreFromList( ListExpr typeInfo, ListExpr value, int errorPos, 
                               ListExpr& errorInfo, bool& correct )
{
  int  attrno, algebraId, typeId, noOfAttrs;
  Word attr;
  Tuple* tupleaddr;
  bool valueCorrect;
  ListExpr first, firstvalue, valuelist, attrlist;

  attrno = 0;
  noOfAttrs = 0;
  tupleaddr = new Tuple( nl->First( typeInfo ) );

  attrlist =  nl->Second(nl->First(typeInfo));
  valuelist = value;
  correct = true;

  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    attrno++;
    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));

    firstvalue = nl->First(valuelist);
    valuelist = nl->Rest(valuelist);

    attr = (am->RestoreFromListObj(algebraId, typeId))(nl->Rest(first),
            firstvalue, attrno, errorInfo, valueCorrect);

    tupleaddr->PutAttribute(attrno - 1, (Attribute *)attr.addr);
    noOfAttrs++;
  }
  correct = true;
  return tupleaddr;
}

ListExpr Tuple::SaveToList( ListExpr typeInfo )
{
  int attrno, algebraId, typeId;
  ListExpr l, lastElem, attrlist, first, valuelist;

  attrlist = nl->Second(nl->First(typeInfo));
  attrno = 0;
  l = nl->TheEmptyList();
  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));

    Attribute *attr = GetAttribute( attrno );
    valuelist = 
      (am->SaveToListObj(algebraId, typeId))(nl->Rest(first), SetWord(attr));
    attrno++;
    if (l == nl->TheEmptyList())
    {
      l = nl->Cons(valuelist, nl->TheEmptyList());
      lastElem = l;
    }
    else
      lastElem = nl->Append(lastElem, valuelist);
  }
  return l;
}

const TupleId& Tuple::GetTupleId() const
{
  return privateTuple->tupleId;
}

void Tuple::SetTupleId( const TupleId& tupleId )
{
  privateTuple->tupleId = tupleId;
}

void Tuple::PutAttribute( const int index, Attribute* attr )
{
  if( privateTuple->attributes[index] != 0 )
    privateTuple->attributes[index]->DeleteIfAllowed();
  privateTuple->attributes[index] = attr;

  recomputeExtSize = true;
  recomputeSize = true;
}

/*
3.9 Class ~TupleBuffer~

This class is used to collect tuples for sorting, for example, or
to do a cartesian product. In this main memory version the buffer
will be in memory, i.e., the buffer will be an array of tuples.

3.9.1 Struct ~PrivateTupleBuffer~

*/
struct PrivateTupleBuffer
{
  PrivateTupleBuffer() :
    totalExtSize( 0 ),
    totalSize( 0 )
    {
    }
/*
The constructor.

*/
  ~PrivateTupleBuffer()
  {
    Tuple *t;
    for( vector<Tuple*>::iterator i = buffer.begin();
         i != buffer.end();
         i++ )
    {
      t = *i;
      t->DecReference();
      t->DeleteIfAllowed();
    }
    buffer.clear();
  }
/*
The destructor.

*/
  vector<Tuple*> buffer;
/*
The buffer which is a ~vector~ from STL.

*/
  double totalExtSize;
/*
The total size occupied by the tuples in the buffer,
taking into account the small FLOBs.

*/
  double totalSize;
/*
The total size occupied by the tuples in the buffer,
taking into account the FLOBs.

*/
};

/*
3.9.2 Implementation of the class ~TupleBuffer~

*/
TupleBuffer::TupleBuffer( const size_t ):
privateTupleBuffer( new PrivateTupleBuffer() )
{
}

TupleBuffer::~TupleBuffer()
{
  delete privateTupleBuffer;
}

int TupleBuffer::GetNoTuples() const
{
  return privateTupleBuffer->buffer.size();
}

double TupleBuffer::GetTotalRootSize() const
{
  if( IsEmpty() )
    return 0;

  return GetNoTuples() *
         privateTupleBuffer->buffer[0]->GetRootSize();
}

double TupleBuffer::GetTotalExtSize() const
{
  return privateTupleBuffer->totalExtSize;
}

double TupleBuffer::GetTotalSize() const
{
  return privateTupleBuffer->totalSize;
}

bool TupleBuffer::IsEmpty() const
{
  return privateTupleBuffer->buffer.empty();
}

void TupleBuffer::Clear()
{
  for( size_t i = 0; i < privateTupleBuffer->buffer.size(); i++ )
  {
    privateTupleBuffer->buffer[i]->DecReference();
    privateTupleBuffer->buffer[i]->DeleteIfAllowed();
  }
  privateTupleBuffer->buffer.clear();
  privateTupleBuffer->totalSize = 0;
}

void TupleBuffer::AppendTuple( Tuple *t )
{
  t->IncReference();
  privateTupleBuffer->buffer.push_back( t );
  t->SetTupleId( privateTupleBuffer->buffer.size()-1 );
  privateTupleBuffer->totalExtSize += t->GetExtSize();
  privateTupleBuffer->totalSize += t->GetSize();
}

Tuple* TupleBuffer::GetTuple( const TupleId& tupleId ) const
{
  return privateTupleBuffer->buffer[tupleId];
}

TupleBufferIterator *TupleBuffer::MakeScan() const
{
  return new TupleBufferIterator( *this );
}

/*
3.9.3 Struct ~PrivateTupleBufferIterator~

*/
struct PrivateTupleBufferIterator
{
  PrivateTupleBufferIterator( const TupleBuffer& tupleBuffer ):
    tupleBuffer( tupleBuffer ),
    currentTuple( 0 )
    {
    }
/*
The constructor.

*/
  const TupleBuffer& tupleBuffer;
/*
A pointer to the tuple buffer.

*/
  size_t currentTuple;
/*
The iterator from STL.

*/
};

/*
3.9.3 Implementation of the class ~TupleBufferIterator~

*/
TupleBufferIterator::TupleBufferIterator( const TupleBuffer& tupleBuffer ):
  privateTupleBufferIterator( new PrivateTupleBufferIterator( tupleBuffer ) )
  {}

TupleBufferIterator::~TupleBufferIterator()
{
  delete privateTupleBufferIterator;
}

Tuple *TupleBufferIterator::GetNextTuple()
{
  if( privateTupleBufferIterator->currentTuple == 
      privateTupleBufferIterator->
        tupleBuffer.privateTupleBuffer->buffer.size() )
    return 0;

  Tuple *result = 
    privateTupleBufferIterator->tupleBuffer.privateTupleBuffer->
      buffer[privateTupleBufferIterator->currentTuple];
  privateTupleBufferIterator->currentTuple++;

  return result;
}

TupleId TupleBufferIterator::GetTupleId() const
{
  return privateTupleBufferIterator->currentTuple - 1;
}

/*
4 Type constructor ~rel~

4.1 Struct ~PrivateRelation~

This struct contains the private attributes of the class ~Relation~.

*/
struct PrivateRelation
{
  PrivateRelation( const ListExpr typeInfo ):
    noTuples( 0 ),
    totalExtSize( 0 ),
    totalSize( 0 ),
    attrExtSize( 0 ),
    attrSize( 0 ),
    tupleType( new TupleType( nl->Second( typeInfo ) ) )
    {
      attrExtSize.resize( tupleType->GetNoAttributes() );
      attrSize.resize( tupleType->GetNoAttributes() );

      for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
      {
        attrExtSize[i] = 0;
        attrSize[i] = 0;
      }
    }
/*
The first constructor. Creates an empty relation from a ~typeInfo~.

*/
  PrivateRelation( TupleType *tupleType ):
    noTuples( 0 ),
    totalExtSize( 0 ),
    totalSize( 0 ),
    attrExtSize( tupleType->GetNoAttributes() ),
    attrSize( tupleType->GetNoAttributes() ),
    tupleType( tupleType )
    {
      for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
      {
        attrExtSize[i] = 0;
        attrSize[i] = 0;
      }
    }
/*
The second constructor. Creates an empty relation from a ~tupleType~.

*/
  ~PrivateRelation()
  {
    Tuple *t;
    for( int i = 0; i < noTuples; i++ )
    { 
      t = tuples[i];
      t->DecReference();
      t->DeleteIfAllowed();
    }
    tuples.clear();
    tupleType->DeleteIfAllowed();
  }

/*
Attributes

*/
  int noTuples;
/*
The quantity of tuples inside the relation.

*/
  double totalExtSize;
/*
The total size occupied by the tuples in the relation taking
into account the small FLOBs, i.e. the extension part of
the tuples.

*/
  double totalSize;
/*
The total size occupied by the tuples in the relation taking
into account all parts of the tuples, including the FLOBs.

*/
  vector<double> attrExtSize;
/*
The total size occupied by the attributes in the relation
taking into account the small FLOBs, i.e. the extension part
of the tuples.

*/
  vector<double> attrSize;
/*
The total size occupied by the attributes in the relation
taking into account all parts of the tuples, including the
FLOBs.

*/
  TupleType *tupleType;
/*
Contains the tuple type.

*/
  vector<Tuple*> tuples;
/*
The array of tuples.

*/
};

/*
4.1 Implementation of the classes ~RelationDescriptor~ and ~RelationDescriptorCompare~

These classes are only used for the Persistend Relational Algebra.

*/
struct RelationDescriptor
{};

class RelationDescriptorCompare
{
  public:
    bool operator()( const RelationDescriptor&, 
                     const RelationDescriptor& ) const
    { return false; }
};

/*
4.2 Implementation of the class ~Relation~

This class implements the memory representation of the type constructor ~rel~.
It is simply an array of tuples.

*/
map<RelationDescriptor, Relation*, RelationDescriptorCompare> 
Relation::pointerTable;

Relation::Relation( const ListExpr typeInfo, const bool isTemporary ):
privateRelation( new PrivateRelation( typeInfo ) )
{}

Relation::Relation( TupleType *tupleType, const bool isTemporary ):
privateRelation( new PrivateRelation( tupleType ) )
{
}

Relation::Relation( const RelationDescriptor& relDesc, const bool isTemporary )
{
  // This main memory version of the relational algebra does not need to open
  // relations, they are always created from the scratch.
  assert( 0 );
}

Relation::~Relation()
{
  delete privateRelation;
}

Relation*
Relation::GetRelation( const RelationDescriptor& d )
{
  // This main memory version of the relational algebra does not open relations.
  // Thus, this function always return a null pointer.
  return 0;
}

Relation*
Relation::RestoreFromList( ListExpr typeInfo, ListExpr value, int errorPos, 
                                     ListExpr& errorInfo, bool& correct )
{
  ListExpr tuplelist, TupleTypeInfo, first;
  Relation* rel;
  Tuple* tupleaddr;
  int tupleno, count;
  bool tupleCorrect;

  correct = true;
  count = 0;
  rel = new Relation( typeInfo );

  tuplelist = value;
  TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
    nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
  tupleno = 0;

  while (!nl->IsEmpty(tuplelist))
  {
    first = nl->First(tuplelist);
    tuplelist = nl->Rest(tuplelist);
    tupleno++;
    tupleaddr = Tuple::RestoreFromList(TupleTypeInfo, first, tupleno, 
                                       errorInfo, tupleCorrect);

    rel->AppendTuple(tupleaddr);
    tupleaddr->DeleteIfAllowed();

    count++;
  }
  correct = true;
  return rel;
}

ListExpr Relation::SaveToList( ListExpr typeInfo )
{
  Tuple* t;
  ListExpr l, lastElem, tlist, TupleTypeInfo;

  RelationIterator* rit = MakeScan();
  l = nl->TheEmptyList();

  //cerr << "OutRel " << endl;
  while ( (t = rit->GetNextTuple()) != 0 )
  {
    TupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
          nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));
    tlist = t->SaveToList(TupleTypeInfo);
    if (l == nl->TheEmptyList())
    {
      l = nl->Cons(tlist, nl->TheEmptyList());
      lastElem = l;
    }
    else
      lastElem = nl->Append(lastElem, tlist);
  }
  delete rit;
  return l;
}

Relation *Relation::Open( SmiRecord& valueRecord, 
                          size_t& offset, 
                          const ListExpr typeInfo )
{
  ListExpr valueList;
  string valueString;
  int valueLength;

  SmiKey mykey;
  mykey = valueRecord.GetKey();

  Relation *result = cache.GetRelation( mykey );
  if( result == 0 )
  {
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
    result = Relation::In( typeInfo, nl->First(valueList), 1, 
                           errorInfo, correct );
    if ( errorInfo != 0 )     
      nl->Destroy( errorInfo );
    nl->Destroy( valueList );
    cache.Insert( mykey, result );
  }
  return result;
}

bool Relation::Save( SmiRecord& valueRecord, 
                     size_t& offset, 
                     const ListExpr typeInfo )
{
  ListExpr valueList;
  string valueString;
  int valueLength;

  valueList = Out( typeInfo );
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

void Relation::Close()
{
}

void Relation::Delete()
{
  cache.Remove( this );
  delete this;
}

Relation *Relation::Clone()
{
  Relation *r = new Relation( privateRelation->tupleType );

  Tuple *t;
  RelationIterator *iter = MakeScan();
  while( (t = iter->GetNextTuple()) != 0 )
  {
    r->AppendTuple( t->Clone() );
  }
  delete iter;

  return r;
}

void Relation::AppendTuple( Tuple *tuple )
{
  tuple->IncReference();
  privateRelation->tuples.push_back( tuple );
  tuple->SetTupleId( privateRelation->tuples.size() );
  privateRelation->noTuples += 1;
  privateRelation->totalExtSize += tuple->GetExtSize();
  privateRelation->totalSize += tuple->GetSize();

  for( int i = 0; i < tuple->GetNoAttributes(); i++ )
  {
    privateRelation->attrExtSize[i] += 
      privateRelation->tupleType->GetAttributeType(i).size;
    privateRelation->attrSize[i] += 
      privateRelation->tupleType->GetAttributeType(i).size;
    for( int j = 0; j < tuple->GetAttribute(i)->NumOfFLOBs(); j++ )
    {
      FLOB *flob = tuple->GetAttribute(i)->GetFLOB(j);
      privateRelation->attrExtSize[i] += flob->IsLob() ? 0 : flob->Size();
      privateRelation->attrSize[i] += flob->Size();
    }
  }
}

bool Relation::DeleteTuple( Tuple *tuple )
{
  cout << "This functionality is not yet implemented for the "
          "RelationMainMemoryAlgebra" << endl;
  assert(false);
}

void Relation::UpdateTuple( Tuple *tuple, 
                            const vector<int>& changedIndices,
                            const vector<Attribute *>& newAttrs )
{
  cout << "This functionality is not yet implemented for the "
          "RelationMainMemoryAlgebra" << endl;
  assert(false);
}

Tuple* Relation::GetTuple( const TupleId& tupleId ) const
{
  return privateRelation->tuples[tupleId-1];
}

void Relation::Clear()
{
  vector<Tuple*>::iterator iter = privateRelation->tuples.begin();

  while( iter != privateRelation->tuples.end() )
  {
    Tuple *t = *iter;
    t->DecReference();
    t->DeleteIfAllowed();
    iter++;
  }
  privateRelation->tuples.clear();
  privateRelation->noTuples = 0;
  privateRelation->totalSize = 0;
}

int Relation::GetNoTuples() const
{
  return privateRelation->noTuples;
}

TupleType *Relation::GetTupleType() const
{
  return privateRelation->tupleType;
}

double Relation::GetTotalRootSize() const
{
  return privateRelation->noTuples *
         privateRelation->tupleType->GetTotalSize();
}

double Relation::GetTotalRootSize( int i ) const
{
  return privateRelation->noTuples *
         privateRelation->tupleType->GetAttributeType(i).size;
}

double Relation::GetTotalExtSize() const
{
  return privateRelation->totalExtSize;
}

double Relation::GetTotalExtSize( int i ) const
{
  assert( i >= 0 &&
          (size_t)i < privateRelation->attrExtSize.size() );
  return privateRelation->attrExtSize[i];
}

double Relation::GetTotalSize() const
{
  return privateRelation->totalSize;
}

double Relation::GetTotalSize( int i ) const
{
  assert( i >= 0 &&
          (size_t)i < privateRelation->attrSize.size() );
  return privateRelation->attrSize[i];
}

RelationIterator *Relation::MakeScan() const
{
  return new RelationIterator( *this );
}

/*
4.3 Struct ~PrivateRelationIterator~

This struct contains the private attributes of the class ~RelationIterator~.

*/
struct PrivateRelationIterator
{
  PrivateRelationIterator( const Relation& rel ):
    relation( rel ),
    currentTupleId( -1 )
    {
      iterator = rel.privateRelation->tuples.begin();
    }
/*
The constructor.

*/
  vector<Tuple*>::iterator iterator;
/*
The iterator.

*/
  const Relation& relation;
/*
A reference to the relation.

*/
  TupleId currentTupleId;
/*
The identification of the current tuple.

*/
};

/*
4.4 Implementation of the class ~RelationIterator~

This class is used for scanning (iterating through) relations.

*/
RelationIterator::RelationIterator( const Relation& relation ):
  privateRelationIterator( new PrivateRelationIterator( relation ) )
  {}

RelationIterator::~RelationIterator()
{
  delete privateRelationIterator;
}

Tuple* RelationIterator::GetNextTuple()
{
  if( EndOfScan() )
    return NULL;

  Tuple *result = *privateRelationIterator->iterator;
  privateRelationIterator->currentTupleId = result->GetTupleId();
  privateRelationIterator->iterator++;
  return result;
}

TupleId RelationIterator::GetTupleId() const
{
  return privateRelationIterator->currentTupleId;
}

bool RelationIterator::EndOfScan()
{
  return privateRelationIterator->iterator == 
         privateRelationIterator->relation.privateRelation->tuples.end();
}

/*
5 Implementation of the class ~RelationCache~

*/
RelationCache::RelationCache( const size_t size ):
size( size ),
current( 0 ),
key( new SmiKey[size] ),
cache( new Relation*[size] )
{
  for( size_t i = 0; i < size; i++ )
  {
    key[i] = SmiRecordId(0);
    cache[i] = 0;
  }
}

RelationCache::~RelationCache()
{
  delete []key;
  delete []cache;
}

void RelationCache::Insert( const SmiKey& ikey, Relation *rel )
{
// VTA - I commented out this piece of code so that the
// cache is not used. There is a bug with the Spatial
// Algebra if the cache of relation is used.
//  if( current == size )
//    delete cache[current-1];
//  if( current > 0 )
//    for( size_t i = current - 1; i > 0; i-- )
//    {
//      key[i] = key[i-1];
//      cache[i] = cache[i-1];
//    }
//  key[0] = ikey;
//  cache[0] = rel;
//
//  if( current < size )
//    current++;
}

void RelationCache::Remove( Relation *rel )
{
  for( size_t i = 0; i < current; i++ )
  {
    if ( cache[i] == rel )
    {
      for( size_t j = i; j < current - 1; j++ )
      {
        key[j] = key[j+1];
        cache[j] = cache[j+1];
      }
      current--;
    }
  }
}

Relation *RelationCache::GetRelation( const SmiKey& skey )
{
  Relation *result = 0;
  for( size_t i = 0; i < current; i++ )
  {
    if ( key[i] == skey )
    {
      result = cache[i];
      for( int j = i-1; j >= 0; j-- )
      {
        key[j+1] = key[j];
        cache[j+1] = cache[j];
      }
      key[0] = skey;
      cache[0] = result;
    }
  }
  return result;
}

void RelationCache::Clear()
{
  for( size_t i = 0; i < current; i++ )
  {
    key[i] = SmiRecordId(0);
    cache[i] = 0;
  }
  current = 0;
}

/*
5 Auxiliary functions

5.1 Function ~Concat~

Copies the attribute values of two tuples
(words) ~r~ and ~s~ into tuple (word) ~t~.

*/
void Concat( Tuple *r, Tuple *s, Tuple *t )
{
  int rnoattrs, snoattrs, tnoattrs;
  Attribute* attr;

  rnoattrs = r->GetNoAttributes();
  snoattrs = s->GetNoAttributes();
  tnoattrs = rnoattrs + snoattrs;

  for( int i = 0; i < rnoattrs; i++)
  {
    attr = r->GetAttribute( i );
    t->PutAttribute( i, ((StandardAttribute*)attr)->Clone() );
  }
  for (int j = rnoattrs; j < tnoattrs; j++)
  {
    attr = s->GetAttribute( j - rnoattrs );
    t->PutAttribute( j, ((StandardAttribute*)attr)->Clone() );
  }
}

#endif
