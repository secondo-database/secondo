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

