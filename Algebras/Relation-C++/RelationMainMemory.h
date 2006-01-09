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

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006 Victor Almeida replaced the ~free~ tuples concept to
reference counters. There are reference counters on tuples and also
on attributes.

*/
#ifndef INC_RELATION_MAIN_MEMORY_H
#define INC_RELATION_MAIN_MEMORY_H

/*
3.2 Struct ~PrivateTuple~

This struct contains the private attributes of the class ~Tuple~.

*/
struct PrivateTuple
{
  PrivateTuple( const TupleType& tupleType ):
    tupleId( 0 ),
    tupleType( tupleType ),
    attributes( 0 )
    {
    }
/*
The first constructor.

*/
  PrivateTuple( const ListExpr typeInfo ):
    tupleId( 0 ),
    tupleType( typeInfo ),
    attributes( 0 )
    {
    }
/*
The second constructor.

*/
  ~PrivateTuple()
  {
    for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
      if( attributes[i] != 0 )
        attributes[i]->DeleteIfAllowed();
  }
/*
The destructor.

*/
  inline void CopyAttribute( const int sourceIndex, PrivateTuple *source, const int destIndex )
  {
    attributes[destIndex] = source->attributes[sourceIndex];
    attributes[destIndex]->IncReference();
  }
/*
This function is used to copy attributes from tuples to tuples without
cloning attributes.

*/
  
  TupleId tupleId;
/*
The unique identification of the tuple inside a relation.

*/
  TupleType tupleType;
/*
Stores the tuple type.

*/
  TupleElement** attributes;
/*
The array of attribute pointers.

*/
};

#endif

