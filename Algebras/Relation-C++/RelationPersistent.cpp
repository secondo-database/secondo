/*
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Relation Algebra

[1] Separate part of persistent data representation

June 1996 Claudia Freundorfer

May 2002 Frank Hoffmann port to C++

November 7, 2002 RHG Corrected the type mapping of ~tcount~.

November 30, 2002 RHG Introduced a function ~RelPersistValue~ instead of
~DefaultPersistValue~ which keeps relations that have been built in memory in a
small cache, so that they need not be rebuilt from then on.

March 2003 Victor Almeida created the new Relational Algebra organization

[TOC]

1 Overview

The Relational Algebra basically implements two type constructors, namely ~tuple~ and ~rel~.
More information about the Relational Algebra can be found in the RelationAlgebra.h header
file.

This file contains the implementation of the Persistent Relational Algebra, where the
type constructors ~tuple~ and ~rel~ are kept in secondary memory. This implementation uses
the Tuple Manager.

2 Includes, Constants, Globals, Enumerations

*/
#ifdef RELALG_PERSISTENT
/*
This ~RELALG_PERSISTENT~ defines which kind of relational algebra is to be compiled.
If it is set, the persistent version of the relational algebra will be compiled, and
otherwise, the main memory version will be compiled.

*/

using namespace std;

#include "RelationAlgebra.h"
#include "TMTuple.h"

extern NestedList *nl;

/*
3 Type constructor ~tuple~

3.1 Class ~TupleId~

This class implements the unique identification for tuples inside a relation. Once a relation
is persistent in an ~SmiFile~ and each tuple is stored in a different ~SmiRecord~ of this file, 
the ~SmiRecordId~ will be this identification.

*/
struct TupleId
{
  TupleId( const SmiRecordId id ):
    value( id )
    {}
  TupleId( const TupleId& id ):
    value( id.value )
    {}
/*
The constructors.

*/
  const TupleId& operator= ( const TupleId& id )
    { value = id.value; return *this; }
  const TupleId& operator+= ( const TupleId& id )
    { value += id.value; return *this; }
  const TupleId& operator-= ( const TupleId& id )
    { value -= id.value; return *this; }
  const TupleId& operator++ ()
    { value++; return *this; }
  TupleId operator++ (int)
    { TupleId result = *this; result += 1; return result; }
  const TupleId& operator-- ()
    { value--; return *this; }
  TupleId operator-- (int)
    { TupleId result = *this; result -= 1; return result; }
  int operator==( const TupleId& id ) const
  { return value == id.value; }
  int operator!=( const TupleId& id ) const
  { return value != id.value; }
  int operator<=( const TupleId& id ) const
  { return value <= id.value; }
  int operator>=( const TupleId& id ) const
  { return value >= id.value; }
  int operator<( const TupleId& id ) const
  { return value < id.value; }
  int operator>( const TupleId& id ) const
  { return value > id.value; }
/*
Operator redefinitions.

*/
  SmiRecordId value;
/*
The ~id~ value.

*/
};

/*
3.2 Struct ~PrivateTuple~

This struct contains the private attributes of the class ~Tuple~.

*/
struct PrivateTuple
{
  PrivateTuple( const TupleType& tupleType, const bool isFree ):
    tupleId( 0 ),
    tupleType( new TupleType( tupleType ) ),
    tmTuple( new TMTuple( tupleType ) ),
    isFree( isFree )
    {

    }
/*
The constructor.

*/
  PrivateTuple( const ListExpr typeInfo, const bool isFree ):
    tupleId( 0 ),
    tupleType( new TupleType( typeInfo ) ),
    tmTuple( new TMTuple( *tupleType ) ),
    isFree( isFree )
    {
    }
/*
The constructor.

*/
  ~PrivateTuple()
  {
    delete tmTuple;
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
  TMTuple *tmTuple;
/*
The tuple from the Tuple Manager.

*/
  bool isFree;
/*
A flag that tells if a tuple is free for deletion. If a tuple is free, then a stream receiving
the tuple can delete or reuse it

*/
};

/*
3.3 Implementation of the class ~Tuple~

This class implements the persistent representation of the type constructor ~tuple~. 
A tuple contains a pointer to a ~TMTuple~ from the Tuple Manager. For more information
about tuples in the TupleManager see the file TMTuple.h.

*/
Tuple::Tuple( const TupleType& tupleType, const bool isFree ):
  privateTuple( new PrivateTuple( tupleType, isFree ) )
  {}

Tuple::Tuple( const ListExpr typeInfo, const bool isFree ):
  privateTuple( new PrivateTuple( typeInfo, isFree ) )
  {}

Tuple::~Tuple()
{
  delete privateTuple;
}

const TupleId& Tuple::GetTupleId() const
{
  return privateTuple->tupleId;
}

void Tuple::SetTupleId( const TupleId& tupleId )
{
  privateTuple->tupleId = tupleId;
}

Attribute* Tuple::GetAttribute( const int index ) const
{
  return (Attribute *)privateTuple->tmTuple->Get( index );
}

void Tuple::PutAttribute( const int index, Attribute* attr )
{
  assert( index >= 0 && index < privateTuple->tupleType->GetNoAttributes() );
  privateTuple->tmTuple->Put( index, attr );
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

Tuple *Tuple::Clone( const bool isFree ) const
{
  Tuple *result = new Tuple( this->GetTupleType(), isFree );
  for( int i = 0; i < this->GetNoAttributes(); i++ )
  {
    Attribute *attr = GetAttribute( i )->Clone();
    result->PutAttribute( i, attr );
  }
  return result;
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
4 Type constructor ~rel~

4.2 Struct ~RelationDescriptor~

This struct contains necessary information for opening a relation.

*/
struct RelationDescriptor
{
  RelationDescriptor( const int noTuples, const SmiFileId tId, const SmiFileId lId ):
    noTuples( noTuples ),
    tupleFileId( tId ),
    lobFileId( lId )
    {}
/*
The constructor.

*/
  int noTuples;
/*
The quantity of tuples inside the relation.

*/
  SmiFileId tupleFileId;
/*
The tuple's file identification.

*/
  SmiFileId lobFileId;
/*
The LOB's file identification.

*/
};

/*
4.1 Struct ~PrivateRelation~

This struct contains the private attributes of the class ~Relation~.

*/
struct PrivateRelation
{
  PrivateRelation( const ListExpr typeInfo ):
    noTuples( 0 ),
    tupleType( nl->Second( typeInfo ) ),
    tupleFile( false, 0 ),
    lobFile( false, 0 ) 
    {
      assert( tupleFile.Create() );
      assert( lobFile.Create() );
    }
/*
The first constructor. Creates an empty relation.

*/
  PrivateRelation( const ListExpr typeInfo, const RelationDescriptor& relDesc ):
    noTuples( relDesc.noTuples ),
    tupleType( nl->Second( nl->First( typeInfo ) ) ),
    tupleFile( false, 0 ),
    lobFile( false, 0 ) 
    {
      assert( tupleFile.Open( relDesc.tupleFileId ) );
      assert( lobFile.Open( relDesc.lobFileId ) );
    }
/*
The second constructor. Opens a previously created relation.

*/
  ~PrivateRelation()
  {
    assert( tupleFile.Close() );
    assert( lobFile.Close() );
  }
/*
The destuctor.

*/
  int noTuples;
/*
Contains the number of tuples in the relation.

*/
  TupleType tupleType;
/*
Stores the tuple type for every tuple of this relation.

*/
  SmiRecordFile tupleFile;
/*
The file to store tuples.

*/
  SmiRecordFile lobFile;
/*
The file to store FLOBs

*/
};

/*
4.2 Implementation of the class ~Relation~

This class implements the persistent representation of the type constructor ~rel~.
A relation is stored into two files: one for the tuples and another for the large
objects (FLOBs) of the tuples.

*/
Relation::Relation( const ListExpr typeInfo ):
  privateRelation( new PrivateRelation( typeInfo ) )
  {}

Relation::Relation( const ListExpr typeInfo, const RelationDescriptor& relDesc ):
  privateRelation( new PrivateRelation( typeInfo, relDesc ) )
  {}

Relation::~Relation()
{
  delete privateRelation;
}

bool Relation::Open( SmiRecord& valueRecord, const ListExpr typeInfo, Relation*& value )
{
  SmiFileId tupleId, lobId;
  int noTuples;
  valueRecord.Read( &tupleId, sizeof( SmiFileId ), 0 );
  valueRecord.Read( &lobId, sizeof( SmiFileId ), sizeof( SmiFileId ) );
  valueRecord.Read( &noTuples, sizeof( int ), 2 * sizeof( SmiFileId ) );

  RelationDescriptor relDesc( noTuples, tupleId, lobId );
  value = new Relation( typeInfo, relDesc );

  return true;
}

bool Relation::Save( SmiRecord& valueRecord, const ListExpr typeInfo )
{
  SmiFileId tupleId = privateRelation->tupleFile.GetFileId(), 
            lobId = privateRelation->lobFile.GetFileId();
  valueRecord.Write( &tupleId, sizeof( SmiFileId ), 0 );
  valueRecord.Write( &lobId, sizeof( SmiFileId ), sizeof( SmiFileId ) );
  valueRecord.Write( &(privateRelation->noTuples), sizeof( int ), 2 * sizeof( SmiFileId ) );

  return true;
}

void Relation::Close()
{
  delete this;
}

void Relation::Delete()
{
  privateRelation->tupleFile.Drop();
  privateRelation->lobFile.Drop();
  delete this;
}

void Relation::AppendTuple( Tuple *tuple )
{
  tuple->GetPrivateTuple()->tmTuple->SaveTo( &privateRelation->tupleFile, &privateRelation->lobFile );
  privateRelation->noTuples += 1;
}

void Relation::Clear()
{
  privateRelation->noTuples = 0;
  assert( privateRelation->tupleFile.Drop() );
  assert( privateRelation->tupleFile.Create() );
  assert( privateRelation->lobFile.Drop() );
  assert( privateRelation->lobFile.Create() );
}

const int Relation::GetNoTuples() const
{
  return privateRelation->noTuples;
}

RelationIterator *Relation::MakeScan() const
{
  return new RelationIterator( *this );
}

#ifdef _PREFETCHING_
/*
4.3 Struct ~PrivateRelationIterator~

This struct contains the private attributes of the class ~RelationIterator~.

*/
struct PrivateRelationIterator
{
  PrivateRelationIterator( const Relation& rel ):
    iterator( rel.privateRelation->tupleFile.SelectAllPrefetched() ),
    relation( rel ),
    endOfScan( false ),
    lastTuple( 0 )
    {
    }
/*
The constructor.

*/
  ~PrivateRelationIterator()
  {
    delete iterator;
    delete lastTuple;
  }
/*
The destructor.

*/
  PrefetchingIterator *iterator;
/*
The iterator.

*/
  const Relation& relation;
/*
A reference to the relation.

*/
  bool endOfScan;
/*
Stores the state of the iterator.

*/
  Tuple *lastTuple;
/*
Stores the last tuple of the iteration for deletion purposes.

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
  if( privateRelationIterator->lastTuple != 0 )
  {
    delete privateRelationIterator->lastTuple;
    privateRelationIterator->lastTuple = 0;
  }

  if( !privateRelationIterator->iterator->Next() )
  {
    privateRelationIterator->endOfScan = true;
    return 0; 
  }

  Tuple *result = new Tuple( privateRelationIterator->relation.privateRelation->tupleType );
  delete result->GetPrivateTuple()->tmTuple;
  result->GetPrivateTuple()->tmTuple = 
    new TMTuple( &privateRelationIterator->relation.privateRelation->tupleFile,
                 privateRelationIterator->iterator,
                 &privateRelationIterator->relation.privateRelation->lobFile,
                 privateRelationIterator->relation.privateRelation->tupleType );
  privateRelationIterator->lastTuple = result;
  return result;
}

const bool RelationIterator::EndOfScan() 
{
  return privateRelationIterator->endOfScan;
}
#else
/*
4.3 Struct ~PrivateRelationIterator~

This struct contains the private attributes of the class ~RelationIterator~.

*/
struct PrivateRelationIterator
{
  PrivateRelationIterator( const Relation& rel ):
    iterator(),
    relation( rel )
    {
      rel.privateRelation->tupleFile.SelectAll( iterator );
    }
/*
The constructor.

*/
  ~PrivateRelationIterator()
  {}
/*
The destructor.

*/
  SmiRecordFileIterator iterator;
/*
The iterator.

*/
  const Relation& relation;
/*
A reference to the relation.

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
  SmiRecord record;
  privateRelationIterator->iterator.Next( record );

  if( EndOfScan() )
    return 0;

  Tuple *result = new Tuple( privateRelationIterator->relation.privateRelation->tupleType );
  delete result->GetPrivateTuple()->tmTuple;
  result->GetPrivateTuple()->tmTuple =
    new TMTuple( &privateRelationIterator->relation.privateRelation->tupleFile,
                 record,
                 &privateRelationIterator->relation.privateRelation->lobFile,
                 privateRelationIterator->relation.privateRelation->tupleType );
  return result;
}

const bool RelationIterator::EndOfScan()
{
  return privateRelationIterator->iterator.EndOfScan();
}

#endif // _PREFETCHING_

#endif // RELALG_PERSISTENT
