/*
November 2004. M. Spiekermann

*/
#ifndef INC_RELATION_MAIN_MEMORY_H
#define INC_RELATION_MAIN_MEMORY_H

//#include "RelationAlgebra.h"

/*
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

#endif

