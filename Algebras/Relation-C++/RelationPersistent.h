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

#ifndef INC_RELALG_PERSISTENT_H
#define INC_RELALG_PERSISTENT_H

#ifdef RELALG_PERSISTENT
/*
This ~RELALG\_PERSISTENT~ defines which kind of relational algebra is to be compiled.
If it is set, the persistent version of the relational algebra will be compiled, and
otherwise, the main memory version will be compiled.

*/

using namespace std;

#include "SecondoSMI.h"

/*
3 Type constructor ~tuple~

3.2 Struct ~PrivateTuple~

This struct contains the private attributes of the class ~Tuple~.

*/
enum TupleState {Fresh, Solid};

struct PrivateTuple
{
  PrivateTuple( const TupleType& tupleType, const bool isFree ):
    tupleId( 0 ),
    tupleType( tupleType ),
    attributes( new (TupleElement*)[ tupleType.GetNoAttributes() ] ),
    tupleRecord( 0 ),
    lobFile( 0 ),
    tupleFile( 0 ),
    state( Fresh ),
    isFree( true ),
    memoryTuple( 0 ),
    extensionTuple( 0 )
    {
      for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
        attributes[i] = 0;
    }
/*
The first constructor. It creates a fresh tuple from a ~tupleType~.

*/
  PrivateTuple( const ListExpr typeInfo, const bool isFree ):
    tupleId( 0 ),
    tupleType( typeInfo ),
    attributes( new (TupleElement*)[ tupleType.GetNoAttributes() ] ),
    tupleRecord( 0 ),
    lobFile( 0 ),
    tupleFile( 0 ),
    state( Fresh ),
    isFree( true ),
    memoryTuple( 0 ),
    extensionTuple( 0 )
    {
      for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
        attributes[i] = 0;
    }
/*
The second constructor. It creates a fresh tuple from a ~typeInfo~.

*/
  ~PrivateTuple()
  {
    if( state == Fresh || 
        state == Solid && memoryTuple == 0 )
      // This was a fresh tuple saved. In this way, the attributes were
      // created outside the tuple and inserted in the tuple using the
      // ~PutAttribute~ method. In this way, they must be deleted.
    {
      assert( extensionTuple == 0 );
      for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
        delete attributes[i];
    }
    else // state == Solid && memoryTuple != 0
    {
      for( int i = 0; i < tupleType.GetNoAttributes(); i++)
      {
        for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
          attributes[i]->GetFLOB(j)->Clear();
      }

      free( memoryTuple );
      if( extensionTuple != 0 )
        free( extensionTuple );
    }
    delete []attributes;
    delete tupleRecord;
  }
/*
The destructor.

*/
  const int Save( SmiRecordFile *tuplefile, SmiRecordFile *lobfile );
/*
Saves a fresh tuple into ~tuplefile~ and ~lobfile~. Returns the total size of
the tuple saved.

*/
  const bool Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile,
                   SmiRecordId rid );
/*
Opens a solid tuple from ~tuplefile~(~rid~) and ~lobfile~.

*/
  const bool Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile,
                          PrefetchingIterator *iter );
/*
Opens a solid tuple from ~tuplefile~ and ~lobfile~ reading the current record of ~iter~.

*/
  const bool Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile,
                   SmiRecord *record );                
                   
/*
Opens a solid tuple from ~tuplefile~ and ~lobfile~ reading from ~record~.

*/

  SmiRecordId tupleId;
/*
The unique identification of the tuple inside a relation.

*/
  TupleType tupleType;
/*
Stores the tuple type.

*/
  TupleElement **attributes;
/*
The attributes pointer array. The tuple information is kept in memory.

*/
  SmiRecord *tupleRecord;
/*
The record that persistently holds the tuple value.

*/
  SmiRecordFile* lobFile;
/*
Reference to an ~SmiRecordFile~ which contains LOBs.

*/
  SmiRecordFile* tupleFile;
/*
Reference to an ~SmiRecordFile~ which contains the tuple.

*/
  TupleState state;
/*
State of the tuple (Fresh, Solid).

*/
  bool isFree;
/*
A flag that tells if a tuple is free for deletion. If a tuple is free, then a stream receiving
the tuple can delete or reuse it

*/
  char *memoryTuple;
/*
Stores the attributes array in memory.

*/
  char *extensionTuple;
/*
Stores the extension (small FLOBs) in memory.

*/
};

#endif // RELALG_PERSISTENT
#endif // INC_RELALGPERSISTENT_H
