/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Main Memory Relational Algebra

March 2003 Victor Almeida created the new Relational Algebra organization

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

#include "RelationAlgebra.h"
#include "CTable.h"
#include "SecondoSystem.h"
#include <vector>

extern NestedList *nl;

bool firsttime = true;
const int cachesize = 20;
int current = 0;
SmiRecordId key[cachesize];
Word cache[cachesize];
/*
These global variables are used for caching relations.

3 Type constructor ~tuple~

3.2 Struct ~PrivateTuple~

This struct contains the private attributes of the class ~Tuple~.

*/
struct PrivateTuple
{
  PrivateTuple( const TupleType& tupleType, const bool isFree ):
    tupleId( 0 ),
    tupleType( new TupleType( tupleType ) ),
    attrArray( new (Attribute*)[tupleType.GetNoAttributes()] ),
    isFree( isFree )
    {
      for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
        attrArray[i] = 0;
    }
/*
The constructor.

*/
  PrivateTuple( const ListExpr typeInfo, const bool isFree ):
    tupleId( 0 ),
    tupleType( new TupleType( typeInfo ) ),
    attrArray( new (Attribute*)[tupleType->GetNoAttributes()] ),
    isFree( isFree )
    {
      for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
        attrArray[i] = 0;
    }
/*
The constructor.

*/
  ~PrivateTuple()
  {
    for( int i = 0; i < tupleType->GetNoAttributes(); i++ )
      delete attrArray[i];
    delete []attrArray;
    delete tupleType;
  }
/*
The destructor.

*/
  TupleId tupleId;
/*
The unique identification of the tuple inside a relation.

*/
  TupleType *tupleType;
/*
Stores the tuple type.

*/
  Attribute** attrArray;
/*
The array of attribute pointers.

*/
  bool isFree;
/*
A flag that tells if a tuple is free for deletion. If a tuple is free, then a stream receiving
the tuple can delete or reuse it

*/
};

/*
3.3 Implementation of the class ~Tuple~

This class implements the memory representation of the type constructor ~tuple~. A tuple is simply
an array of attributes, as it can be seen in the definition of the ~PrivateTuple~ class.

*/
Tuple::Tuple( const TupleType& tupleType, const bool isFree ):
  privateTuple( new PrivateTuple( tupleType, isFree ) )
  {
    tuplesCreated++;
    tuplesInMemory++;
    if( tuplesInMemory > maximumTuples )
      maximumTuples = tuplesInMemory;
  }

Tuple::Tuple( const ListExpr typeInfo, const bool isFree ):
  privateTuple( new PrivateTuple( typeInfo, isFree ) )
  {
    tuplesCreated++;
    tuplesInMemory++;
    if( tuplesInMemory > maximumTuples )
      maximumTuples = tuplesInMemory;
  }

Tuple::~Tuple()
{
  tuplesDeleted++;
  tuplesInMemory--;
  delete privateTuple;
}

Tuple *Tuple::RestoreFromList( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct )
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

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
  while (!nl->IsEmpty(attrlist))
  {
    first = nl->First(attrlist);
    attrlist = nl->Rest(attrlist);
    attrno++;
    algebraId = nl->IntValue(nl->First(nl->Second(first)));
    typeId = nl->IntValue(nl->Second(nl->Second(first)));

    firstvalue = nl->First(valuelist);
    valuelist = nl->Rest(valuelist);

    attr = (algM->RestoreFromListObj(algebraId, typeId))(nl->Rest(first),
            firstvalue, attrno, errorInfo, valueCorrect);

    assert(valueCorrect);
    tupleaddr->PutAttribute(attrno - 1, (Attribute *)attr.addr);
    noOfAttrs++;
  }
  assert( tupleaddr->GetNoAttributes() == noOfAttrs );
  correct = true;
  return tupleaddr;
}

ListExpr Tuple::SaveToList( ListExpr typeInfo )
{
  int attrno, algebraId, typeId;
  ListExpr l, lastElem, attrlist, first, valuelist;

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();
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
    valuelist = (algM->SaveToListObj(algebraId, typeId))(nl->Rest(first), SetWord(attr));
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
  return (TupleId&)privateTuple->tupleId;
}

void Tuple::SetTupleId( const TupleId& tupleId )
{
  privateTuple->tupleId = tupleId;
}

Attribute* Tuple::GetAttribute( const int index ) const
{
  return privateTuple->attrArray[ index ];
}

void Tuple::PutAttribute( const int index, Attribute* attr )
{
  assert( index >= 0 && index < privateTuple->tupleType->GetNoAttributes() );
  privateTuple->attrArray[ index ] = attr;
}

const int Tuple::GetMemorySize() const
{
  int extensionSize = 0;

  for( int i = 0; i < privateTuple->tupleType->GetNoAttributes(); i++)
  {
    for( int j = 0; j < privateTuple->attrArray[i]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = privateTuple->attrArray[i]->GetFLOB(j);
      if( !tmpFLOB->IsLob() )
        extensionSize += tmpFLOB->Size();
    }
  }
  return privateTuple->tupleType->GetTotalSize() + extensionSize;
}

const int Tuple::GetTotalSize() const
{
  int totalSize = privateTuple->tupleType->GetTotalSize();

  for( int i = 0; i < privateTuple->tupleType->GetNoAttributes(); i++)
  {
    for( int j = 0; j < privateTuple->attrArray[i]->NumOfFLOBs(); j++)
    {
      totalSize += privateTuple->attrArray[i]->GetFLOB(j)->Size();
    }
  }
  return totalSize;
}

const int Tuple::GetNoAttributes() const
{
  return privateTuple->tupleType->GetNoAttributes();
}

const TupleType& Tuple::GetTupleType() const
{
  return *(privateTuple->tupleType);
}

const bool Tuple::IsFree() const
{
  return privateTuple->isFree;
}

void Tuple::SetFree( const bool onoff )
{
  privateTuple->isFree = onoff;
}

Tuple *Tuple::CloneIfNecessary()
{
  if( IsFree() )
    return this;
  else
    return this->Clone( false );
}

void Tuple::DeleteIfAllowed()
{
  if( IsFree() )
    delete this;
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
    totalSize( 0 )
    {}

  vector<Tuple*> buffer;
/*
The buffer which is a ~vector~ from STL.

*/
  double totalSize;
/*
The total size of the buffer in bytes.

*/

};

/*
3.9.2 Implementation of the class ~TupleBuffer~

*/
TupleBuffer::TupleBuffer():
privateTupleBuffer( new PrivateTupleBuffer() )
{
}

TupleBuffer::~TupleBuffer()
{
  delete privateTupleBuffer;
}

const int TupleBuffer::GetNoTuples() const
{
  return privateTupleBuffer->buffer.size();
}

const double TupleBuffer::GetTotalSize() const
{
  return privateTupleBuffer->totalSize;
}

const bool TupleBuffer::IsEmpty() const
{
  return privateTupleBuffer->buffer.empty();
}

void TupleBuffer::Clear()
{
  for( size_t i = 0; i < privateTupleBuffer->buffer.size(); i++ )
    delete privateTupleBuffer->buffer[i];
  privateTupleBuffer->buffer.clear();
  privateTupleBuffer->totalSize = 0;
}

void TupleBuffer::AppendTuple( Tuple *t )
{
  privateTupleBuffer->buffer.push_back( t );
  privateTupleBuffer->totalSize += t->GetTotalSize();
}

Tuple* TupleBuffer::GetTuple( const TupleId& tupleId ) const
{
  assert( tupleId >= 0 && tupleId < (TupleId)privateTupleBuffer->buffer.size() );
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
  if( privateTupleBufferIterator->currentTuple == privateTupleBufferIterator->tupleBuffer.privateTupleBuffer->buffer.size() )
    return 0;

  Tuple *result = privateTupleBufferIterator->tupleBuffer.privateTupleBuffer->buffer[privateTupleBufferIterator->currentTuple];
  privateTupleBufferIterator->currentTuple++;

  return result;
}

TupleId TupleBufferIterator::GetTupleId() const
{
  assert( privateTupleBufferIterator->currentTuple > 0 );
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
    totalSize( 0 ),
    tupleType( nl->Second( typeInfo ) ),
    tupleArray( new CTable<Tuple*>( 100 ) ),
    currentId( 1 )
    {
    }
/*
The first constructor. Creates an empty relation from a ~typeInfo~.

*/
  PrivateRelation( const TupleType& tupleType ):
    noTuples( 0 ),
    totalSize( 0 ),
    tupleType( tupleType ),
    tupleArray( new CTable<Tuple*>( 100 ) ),
    currentId( 1 )
    {
    }
/*
The second constructor. Creates an empty relation from a ~tupleType~.

*/
  ~PrivateRelation()
  {
    delete tupleArray;
  }

  int noTuples;
/*
Contains the number of tuples in the relation.

*/
  double totalSize;
/*
Contains the total size of the relation.

*/
  TupleType tupleType;
/*
Contains the tuple type.

*/
  CTable<Tuple*>* tupleArray;
/*
The array of tuples.

*/
  TupleId currentId;
/*
Keeps the ~id~ to be given to the next tuple appended in the relation. This
value is auto-incremented at each append of a new tuple.

*/
};

/*
4.2 Implementation of the class ~Relation~

This class implements the memory representation of the type constructor ~rel~.
It is simply an array of tuples.

*/
Relation::Relation( const ListExpr typeInfo, const bool isTemporary ):
  privateRelation( new PrivateRelation( typeInfo ) )
  {}

Relation::Relation( const TupleType& tupleType, const bool isTemporary ):
  privateRelation( new PrivateRelation( tupleType ) )
  {}

Relation::Relation( const ListExpr typeInfo, const RelationDescriptor& relDesc, const bool isTemporary ):
  privateRelation( new PrivateRelation( typeInfo ) )
  {
    // This main memory version of the relational algebra does not need to open
    // relations, they are always created from the scratch.
    assert( 0 );
  }

Relation::Relation( const TupleType& tupleType, const RelationDescriptor& relDesc, const bool isTemporary ):
  privateRelation( new PrivateRelation( tupleType ) )
  {
    // This main memory version of the relational algebra does not need to open
    // relations, they are always created from the scratch.
    assert( 0 );
  }

Relation::~Relation()
{
  // First delete the relation from the cache ...
  if( !firsttime )
  {
    for( int i = 0; i < cachesize; i++ )
    {
      if( key[i] != 0 && cache[i].addr == this )
      {
        key[i] = 0;
        break;
      }
    }
  }

  // then delete the relation itself.
  RelationIterator *iter = MakeScan();
  Tuple *t;
  while( (t = iter->GetNextTuple()) != 0 )
    delete t;
  delete iter;
  delete privateRelation;
}

Relation *Relation::RestoreFromList( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct )
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
    tupleaddr = Tuple::RestoreFromList(TupleTypeInfo, first, tupleno, errorInfo, tupleCorrect);

    assert(tupleCorrect);
    rel->AppendTuple(tupleaddr);
    tupleaddr->DeleteIfAllowed();

    count++;
  }
  assert( rel->GetNoTuples() == count );
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

bool Relation::Open( SmiRecord& valueRecord, const ListExpr typeInfo, Relation*& value )
{
  ListExpr valueList;
  string valueString;
  int valueLength;

  SmiKey mykey;
  SmiRecordId recId;
  mykey = valueRecord.GetKey();
  if ( !mykey.GetKey(recId) )
  {
    cout << "\tRelOpen: Couldn't get the key!" << endl;
  }

  // initialize
  if ( firsttime ) {
    for ( int i = 0; i < cachesize; i++ ) { key[i] = 0; }
    firsttime = false;
  }

  // check whether value was cached
  for ( int j = 0; j < cachesize; j++ )
    if ( key[j]  == recId )
    {
      value = (Relation *)cache[j].addr;
      return true;
    }

  // prepare to cache the value constructed from the list
  if ( key[current] != 0 ) {
    delete (Relation *)cache[current].addr;
  }
  key[current] = recId;

  ListExpr errorInfo = nl->OneElemList( nl->SymbolAtom( "ERRORS" ) );
  bool correct;
  valueRecord.Read( &valueLength, sizeof( valueLength ), 0 );
  char* buffer = new char[valueLength];
  valueRecord.Read( buffer, valueLength, sizeof( valueLength ) );
  valueString.assign( buffer, valueLength );
  delete []buffer;
  nl->ReadFromString( valueString, valueList );
  value = Relation::In( typeInfo, nl->First(valueList), 1, errorInfo, correct);

  cache[current++] = SetWord(value);
  if ( current == cachesize ) current = 0;

  if ( errorInfo != 0 )     {
    nl->Destroy( errorInfo );
  }
  nl->Destroy( valueList );
  return (true);
}

bool Relation::Save( SmiRecord& valueRecord, const ListExpr typeInfo )
{
  ListExpr valueList;
  string valueString;
  int valueLength;

  valueList = Out( typeInfo );
  valueList = nl->OneElemList( valueList );
  nl->WriteToString( valueString, valueList );
  valueLength = valueString.length();
  valueRecord.Write( &valueLength, sizeof( valueLength ), 0 );
  valueRecord.Write( valueString.data(), valueString.length(), sizeof( valueLength ) );

  nl->Destroy( valueList );
  return (true);
}

void Relation::Close()
{
}

void Relation::Delete()
{
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
  tuple->SetFree( false );
  tuple->SetTupleId( privateRelation->currentId++ );
  privateRelation->tupleArray->Add( tuple );
  privateRelation->noTuples += 1;
  privateRelation->totalSize += tuple->GetTotalSize();
}

Tuple* Relation::GetTuple( const TupleId& tupleId ) const
{
  return (*privateRelation->tupleArray)[ tupleId ];
}

void Relation::Clear()
{
  CTable<Tuple*>::Iterator iter = privateRelation->tupleArray->Begin();

  while( iter != privateRelation->tupleArray->End() )
  {
    Tuple *t = *iter;
    t->DeleteIfAllowed();
    ++iter;
  }

  delete privateRelation->tupleArray;
  privateRelation->currentId = 1;
  privateRelation->noTuples = 0;
  privateRelation->totalSize = 0;
  privateRelation->tupleArray = new CTable<Tuple*>( 100 );

}

const int Relation::GetNoTuples() const
{
  return privateRelation->noTuples;
}

const double Relation::GetTotalSize() const
{
  return privateRelation->totalSize;
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
    iterator( rel.privateRelation->tupleArray->Begin() ),
    relation( rel ),
    currentTupleId( -1 )
    {
    }
/*
The constructor.

*/
  CTable<Tuple*>::Iterator iterator;
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
  assert( privateRelationIterator->currentTupleId != -1 );
  return privateRelationIterator->currentTupleId;
}

const bool RelationIterator::EndOfScan()
{
  return privateRelationIterator->iterator.EndOfScan();
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

  assert( t->GetNoAttributes() == tnoattrs );

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
