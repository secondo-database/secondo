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
an array of attributes and the ~rel~ is an array of tuples.

2 Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "CTable.h"
#include "SecondoSystem.h"

extern NestedList *nl;

/*
3 Type constructor ~tuple~

3.1 Class ~TupleId~

This class implements the unique identification for tuples inside a relation. Once a relation
is an array of tuples, the ~TupleId~ data type can be an integer indexing the position of the
tuple in the relation's array. 

*/
struct TupleId
{
  TupleId( const int id ):
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
  int value;
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
    attrArray( new Attribute*[tupleType.GetNoAttributes()] ),
    isFree( isFree )
    {
      
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
  return privateTuple->attrArray[ index ];
}

void Tuple::PutAttribute( const int index, Attribute* attr )
{
  assert( index >= 0 && index < privateTuple->tupleType->GetNoAttributes() );
  privateTuple->attrArray[ index ] = attr;
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

4.1 Struct ~PrivateRelation~

This struct contains the private attributes of the class ~Relation~.

*/
struct PrivateRelation
{
  PrivateRelation():
    noTuples( 0 ),
    tupleArray( new CTable<Tuple*>( 100 ) ),
    currentId( 1 )
    {}
/*
The constructor. 

*/
  ~PrivateRelation()
  {
    delete tupleArray;
  }

  int noTuples;
/*
Contains the number of tuples in the relation.

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
Relation::Relation():
  privateRelation( new PrivateRelation() )
  {}

Relation::~Relation()
{
  RelationIterator *iter = MakeScan();
  while( !iter->EndOfScan() )
  {
    Tuple *t = iter->GetNextTuple();
    delete t;
  }
  delete iter;    
  delete privateRelation;
}

void Relation::AppendTuple( Tuple *tuple )
{
  tuple->SetTupleId( privateRelation->currentId++ );
  privateRelation->tupleArray->Add( tuple );
  privateRelation->noTuples += 1;
}

const Tuple* Relation::GetTuple( const TupleId& tupleId ) const
{
  return (*privateRelation->tupleArray)[ tupleId.value ];
}

void Relation::Clear()
{
  CTable<Tuple*>::Iterator iter = privateRelation->tupleArray->Begin();
  
  while( iter != privateRelation->tupleArray->End() )
  {
    Tuple *t = *iter;
    delete t;
    ++iter;
  }

  delete privateRelation->tupleArray;
  privateRelation->currentId = 1;
  privateRelation->noTuples = 0;
  privateRelation->tupleArray = new CTable<Tuple*>( 100 );

}

const int Relation::GetNoTuples() const
{
  return privateRelation->noTuples;
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
    relation( rel )
    {}
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

Tuple* RelationIterator::GetNextTuple() const 
{
  if( EndOfScan() )
    return NULL;

  Tuple *result = *privateRelationIterator->iterator;
  privateRelationIterator->iterator++;
  return result; 
}

const bool RelationIterator::EndOfScan() const
{
  return privateRelationIterator->iterator.EndOfScan();
}

