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
#include "SecondoSystem.h"
#include "FLOB.h"

extern NestedList *nl;

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
    if( memoryTuple == 0 )
    {
      assert( extensionTuple == 0 );
      // This was a fresh tuple saved. In this way, the attributes were
      // created outside the tuple and inserted in the tuple using the
      // ~PutAttribute~ method. In this way, they must be deleted.
      for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
        delete attributes[i];
    }
    else
    {
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
  const bool Save( SmiRecordFile *tuplefile, SmiRecordFile *lobfile );
/*
Saves a fresh tuple into ~tuplefile~ and ~lobfile~.

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

const bool PrivateTuple::Save( SmiRecordFile *tuplefile, SmiRecordFile *lobfile )
{
  assert( state == Fresh &&
          lobFile == 0 && tupleFile == 0 && tupleRecord == 0 &&
          memoryTuple == 0 && extensionTuple == 0 );

  lobFile = lobfile;
  tupleFile = tuplefile;
  tupleRecord = new SmiRecord();

  // Calculate the size of the small FLOB data which will be saved together
  // with the tuple attributes and save the LOBs in the lobFile.
  int extensionSize = 0;
  for( int i = 0; i < tupleType.GetNoAttributes(); i++)
  {
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
      if( !tmpFLOB->IsLob() )
        extensionSize += tmpFLOB->Size();
      else
        tmpFLOB->SaveToLob( *lobFile );
    }
  }

  // Move FLOB data to extension tuple.
  if( extensionSize > 0 )
  {
    extensionTuple = (char *)malloc(extensionSize);
    char *extensionPtr = extensionTuple;
    for( int i = 0; i < tupleType.GetNoAttributes(); i++)
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++)
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB(j);
        if( !tmpFLOB->IsLob() )
        {
          tmpFLOB->SaveToExtensionTuple( extensionPtr );
          extensionPtr += tmpFLOB->Size();
        }
      }
    }
  }

  // Move external attributes to memory tuple
  memoryTuple = (char *)malloc( tupleType.GetTotalSize() );
  int offset = 0;
  for( int i = 0; i < tupleType.GetNoAttributes(); i++)
  {
    memcpy( &memoryTuple[offset], attributes[i], tupleType.GetAttributeType(i).size );
    offset += tupleType.GetAttributeType(i).size;
  }

  bool rc = tupleFile->AppendRecord( tupleId, *tupleRecord );
  rc = tupleRecord->Write( &extensionSize, sizeof(int), 0) && rc;
  rc = tupleRecord->Write( memoryTuple, tupleType.GetTotalSize(), sizeof(int)) && rc;
  if( extensionSize > 0 )
    rc = tupleRecord->Write( extensionTuple, extensionSize, sizeof(int) + tupleType.GetTotalSize() ) && rc;

  state = Solid;

  if( extensionSize > 0 )
  {
    free( extensionTuple ); extensionTuple = 0;
  }
  free( memoryTuple ); memoryTuple = 0;

  return rc;
}

const bool PrivateTuple::Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile, SmiRecordId rid )
{
  assert( state == Fresh &&
          lobFile == 0 && tupleFile == 0 && tupleRecord == 0 &&
          memoryTuple == 0 && extensionTuple == 0 );

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  tupleId = rid;
  tupleRecord = new SmiRecord();
  this->tupleFile = tuplefile;
  this->lobFile = lobfile;

  // read tuple header and memory tuple from disk
  bool ok = tupleFile->SelectRecord( tupleId, *tupleRecord );
  if( !ok )
  {
    tupleId = 0;
    delete tupleRecord;
    tupleFile = 0;
    lobFile = 0;
    return false;
  }

  int extensionSize = 0;
  ok = tupleRecord->Read( &extensionSize, sizeof(int), 0 );

  memoryTuple = (char *)malloc( tupleType.GetTotalSize() );
  ok = tupleRecord->Read( memoryTuple, tupleType.GetTotalSize(), sizeof(int) ) && ok;

  if( !ok )
  {
    tupleId = 0;
    delete tupleRecord;
    tupleFile = 0;
    lobFile = 0;
    free( memoryTuple ); memoryTuple = 0;
    return false;
  }


  // Read attribute values from memoryTuple.
  char *valuePtr = memoryTuple;
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    int algId = tupleType.GetAttributeType(i).algId;
    int typeId = tupleType.GetAttributeType(i).typeId;
    attributes[i] = (TupleElement*)(*(algM->Cast(algId, typeId)))(valuePtr);
    valuePtr += tupleType.GetAttributeType(i).size;
  }

  // move FLOB data to extension tuple if exists.
  if( extensionSize > 0 )
  {
    extensionTuple = (char *)malloc( extensionSize );
    ok = tupleRecord->Read( extensionTuple, extensionSize, sizeof(int) + tupleType.GetTotalSize() ) && ok;
    if( !ok )
    {
      tupleId = 0;
      delete tupleRecord;
      tupleFile = 0;
      lobFile = 0;
      free( memoryTuple ); memoryTuple = 0;
      free( extensionTuple ); extensionTuple = 0;
      return false;
    }

    char *extensionPtr = extensionTuple;
    for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
        if( !tmpFLOB->IsLob() )
        {
          tmpFLOB->Restore( extensionPtr );
          extensionPtr = extensionPtr + tmpFLOB->Size();
        }
      }
    }
  }

  // Sets the lobFile for all lobs
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( tmpFLOB->IsLob() )
      {
        tmpFLOB->SetLobFile( lobFile );
      }
    }
  }

  state = Solid;
  return true;
}

const bool PrivateTuple::Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile, PrefetchingIterator *iter )
{
  assert( state == Fresh && lobFile == 0 && tupleFile == 0 && tupleRecord == 0 );
  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  iter->ReadCurrentRecordNumber( tupleId );
  this->tupleFile = tuplefile;
  this->lobFile = lobfile;


  int extensionSize = 0;
  bool ok = iter->ReadCurrentData( &extensionSize, sizeof(int), 0 );

  memoryTuple = (char *)malloc( tupleType.GetTotalSize() );
  ok = iter->ReadCurrentData( memoryTuple, tupleType.GetTotalSize(), sizeof(int) ) && ok;

  if( !ok )
  {
    tupleId = 0;
    tupleFile = 0;
    lobFile = 0;
    free( memoryTuple ); memoryTuple = 0;
    return false;
  }

  // Read attribute values from memoryTuple.
  char *valuePtr = memoryTuple;
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    int algId = tupleType.GetAttributeType(i).algId;
    int typeId = tupleType.GetAttributeType(i).typeId;
    attributes[i] = (TupleElement*)(*(algM->Cast(algId, typeId)))(valuePtr);
    valuePtr += tupleType.GetAttributeType(i).size;
  }

  // move FLOB data to extension tuple if exists.
  if( extensionSize > 0 )
  {
    extensionTuple = (char *)malloc( extensionSize );
    ok = iter->ReadCurrentData( extensionTuple, extensionSize, sizeof(int) + tupleType.GetTotalSize() ) && ok;
    if( !ok )
    {
      tupleId = 0;
      tupleFile = 0;
      lobFile = 0;
      free( memoryTuple ); memoryTuple = 0;
      free( extensionTuple ); extensionTuple = 0;
      return false;
    }

    char *extensionPtr = extensionTuple;
    for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
        if( !tmpFLOB->IsLob() )
        {
          tmpFLOB->Restore( extensionPtr );
          extensionPtr = extensionPtr + tmpFLOB->Size();
        }
      }
    }
  }

  // Sets the lobFile for all lobs
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( tmpFLOB->IsLob() )
      {
        tmpFLOB->SetLobFile( lobFile );
      }
    }
  }

  state = Solid;
  return true;
}

const bool PrivateTuple::Open( SmiRecordFile *tuplefile, SmiRecordFile *lobfile, SmiRecord *record )
{
  assert( state == Fresh &&
          lobFile == 0 && tupleFile == 0 && tupleRecord == 0 &&
          memoryTuple == 0 && extensionTuple == 0 );

  AlgebraManager* algM = SecondoSystem::GetAlgebraManager();

  SmiKey key;
  key = record->GetKey();
  key.GetKey( tupleId );
  this->tupleRecord = record;
  this->tupleFile = tuplefile;
  this->lobFile = lobfile;

  // read tuple header and memory tuple from disk
  bool ok = tupleFile->SelectRecord( tupleId, *tupleRecord );
  if( !ok )
  {
    tupleId = 0;
    delete tupleRecord;
    tupleFile = 0;
    lobFile = 0;
    return false;
  }

  int extensionSize = 0;
  ok = tupleRecord->Read( &extensionSize, sizeof(int), 0 );

  memoryTuple = (char *)malloc( tupleType.GetTotalSize() );
  ok = tupleRecord->Read( memoryTuple, tupleType.GetTotalSize(), sizeof(int) ) && ok;

  if( !ok )
  {
    tupleId = 0;
    delete tupleRecord;
    tupleFile = 0;
    lobFile = 0;
    free( memoryTuple ); memoryTuple = 0;
    return false;
  }

  // Read attribute values from memoryTuple.
  char *valuePtr = memoryTuple;
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    int algId = tupleType.GetAttributeType(i).algId;
    int typeId = tupleType.GetAttributeType(i).typeId;
    attributes[i] = (TupleElement*)(*(algM->Cast(algId, typeId)))(valuePtr);
    valuePtr += tupleType.GetAttributeType(i).size;
  }

  // move FLOB data to extension tuple if exists.
  if( extensionSize > 0 )
  {
    extensionTuple = (char *)malloc( extensionSize );
    ok = tupleRecord->Read( extensionTuple, extensionSize, sizeof(int) + tupleType.GetTotalSize() ) && ok;
    if( !ok )
    {
      tupleId = 0;
      delete tupleRecord;
      tupleFile = 0;
      lobFile = 0;
      free( memoryTuple ); memoryTuple = 0;
      free( extensionTuple ); extensionTuple = 0;
      return false;
    }

    char *extensionPtr = extensionTuple;
    for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
    {
      for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
      {
        FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
        if( !tmpFLOB->IsLob() )
        {
          tmpFLOB->Restore( extensionPtr );
          extensionPtr = extensionPtr + tmpFLOB->Size();
        }
      }
    }
  }

  // Sets the lobFile for all lobs
  for( int i = 0; i < tupleType.GetNoAttributes(); i++ )
  {
    for( int j = 0; j < attributes[i]->NumOfFLOBs(); j++ )
    {
      FLOB *tmpFLOB = attributes[i]->GetFLOB( j );
      if( tmpFLOB->IsLob() )
      {
        tmpFLOB->SetLobFile( lobFile );
      }
    }
  }

  state = Solid;
  return true;
}

/*
3.3 Implementation of the class ~Tuple~

This class implements the persistent representation of the type constructor ~tuple~.
A tuple contains a pointer to a ~TMTuple~ from the Tuple Manager. For more information
about tuples in the TupleManager see the file TMTuple.h.

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
  // This function should never be called.
  assert( false );
}

ListExpr Tuple::SaveToList( ListExpr typeInfo )
{
  // This function should never be called.
  assert( false );
}

const TupleId& Tuple::GetTupleId() const
{
  return (TupleId)privateTuple->tupleId;
}

void Tuple::SetTupleId( const TupleId& tupleId )
{
  privateTuple->tupleId = (SmiRecordId)tupleId;
}

Attribute* Tuple::GetAttribute( const int index ) const
{
  assert( index >= 0 && index < GetNoAttributes() );
  assert( privateTuple->attributes[index] != 0 );

  return (Attribute *)privateTuple->attributes[index];
}

void Tuple::PutAttribute( const int index, Attribute* attr )
{
  assert( index >= 0 && index < GetNoAttributes() );

  if( privateTuple->attributes[index] != 0 )
    delete privateTuple->attributes[index];
  privateTuple->attributes[index] = attr;
}

const long Tuple::GetMemorySize() const
{
  long extensionSize = 0;
  for( int i = 0; i < privateTuple->tupleType.GetNoAttributes(); i++)
  {
    for( int j = 0; j < privateTuple->attributes[i]->NumOfFLOBs(); j++)
    {
      FLOB *tmpFLOB = privateTuple->attributes[i]->GetFLOB(j);
      if( !tmpFLOB->IsLob() )
        extensionSize += tmpFLOB->Size();
    }
  }
  return privateTuple->tupleType.GetTotalSize() + extensionSize;
}

const int Tuple::GetNoAttributes() const
{
  return privateTuple->tupleType.GetNoAttributes();
}

const TupleType& Tuple::GetTupleType() const
{
  return privateTuple->tupleType;
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
  if( privateTuple->state == Fresh && privateTuple->isFree )
    return this;
  else
    return this->Clone( false );
}

void Tuple::DeleteIfAllowed()
{
  if( privateTuple->isFree )
    delete this;
}

void Tuple::Delete()
{
  delete this;
}

/*
3.9 Class ~TupleBuffer~

This class is used to collect tuples for sorting, for example, or
to do a cartesian product. In this persistent version, if the buffer
is small it will be stored in memory and if it is large, it will be
stored into a disk file.

3.9.1 Struct ~PrivateTupleBuffer~

*/
struct PrivateTupleBuffer
{
  PrivateTupleBuffer():
    diskBuffer( 0 ),
    MAX_TUPLES_IN_MEMORY( 0 ),
    inMemory( true )
    {}
/*
The constructor.

*/
  ~PrivateTupleBuffer()
  {
    if( !inMemory )
      diskBuffer->Delete();
    else
    {
      for( size_t i = 0; i < memoryBuffer.size(); i++ )
        memoryBuffer[i]->Delete();
    }
  }
/*
The destructor.

*/
  static const size_t MAX_MEMORY_SIZE = 2097152;
/*
The maximum size of the memory in bytes. 2 MBytes being used.

*/
  vector<Tuple*> memoryBuffer;
/*
The memory buffer which is a ~vector~ from STL.

*/
  Relation* diskBuffer;
/*
The buffer stored on disk.

*/
  size_t MAX_TUPLES_IN_MEMORY;
/*
The number of tuples that should fit in memory.

*/
  bool inMemory;
/*
A flag that tells if the buffer fit in memory or not.

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
  if( privateTupleBuffer->inMemory )
    return privateTupleBuffer->memoryBuffer.size();
  else
    return privateTupleBuffer->diskBuffer->GetNoTuples();
}

const bool TupleBuffer::IsEmpty() const
{
  if( privateTupleBuffer->inMemory )
    return privateTupleBuffer->memoryBuffer.empty();
  else
    return false;
}

void TupleBuffer::Clear()
{
  if( privateTupleBuffer->inMemory )
  {
    for( size_t i = 0; i < privateTupleBuffer->memoryBuffer.size(); i++ )
      privateTupleBuffer->memoryBuffer[i]->Delete();
    privateTupleBuffer->memoryBuffer.clear();
  }
  else
  {
    privateTupleBuffer->diskBuffer->Clear();
  }
}

void TupleBuffer::AppendTuple( Tuple *t )
{
  if( privateTupleBuffer->MAX_TUPLES_IN_MEMORY == 0 )
  // first tuple being inserted in the buffer.
  {
    privateTupleBuffer->MAX_TUPLES_IN_MEMORY = PrivateTupleBuffer::MAX_MEMORY_SIZE / t->GetMemorySize();
  }

  if( privateTupleBuffer->inMemory )
  {
    if( privateTupleBuffer->memoryBuffer.size() < privateTupleBuffer->MAX_TUPLES_IN_MEMORY )
    {
      t->SetFree( false );
      privateTupleBuffer->memoryBuffer.push_back( t );
    }
    else
    {
      privateTupleBuffer->diskBuffer = new Relation( t->GetTupleType() );

      vector<Tuple*>::iterator iter = privateTupleBuffer->memoryBuffer.begin();
      while( iter != privateTupleBuffer->memoryBuffer.end() )
      {
        Tuple *t1 =  (*iter)->CloneIfNecessary();
        privateTupleBuffer->diskBuffer->AppendTuple( t1 );
        t1->Delete();
        iter++;
      }

      privateTupleBuffer->memoryBuffer.clear();
      privateTupleBuffer->diskBuffer->AppendTuple( t );
      privateTupleBuffer->inMemory = false;
    }
  }
  else
  {
    privateTupleBuffer->diskBuffer->AppendTuple( t );
  }
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
    currentTuple( 0 ),
    diskIterator( tupleBuffer.privateTupleBuffer->inMemory ? 0 : tupleBuffer.privateTupleBuffer->diskBuffer->MakeScan() )
    {
    }
/*
The constructor.

*/
  ~PrivateTupleBufferIterator()
  {
    delete diskIterator;
  }
/*
The destructor.

*/
  const TupleBuffer& tupleBuffer;
/*
A pointer to the tuple buffer.

*/
  size_t currentTuple;
/*
The current tuple if it is in memory.

*/
  RelationIterator *diskIterator;
/*
The iterator if it is not in memory.

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
  if( privateTupleBufferIterator->diskIterator )
  {
    return privateTupleBufferIterator->diskIterator->GetNextTuple();
  }
  else
  {
    if( privateTupleBufferIterator->currentTuple == privateTupleBufferIterator->tupleBuffer.privateTupleBuffer->memoryBuffer.size() )
      return 0;

    Tuple *result = privateTupleBufferIterator->tupleBuffer.privateTupleBuffer->memoryBuffer[privateTupleBufferIterator->currentTuple];
    privateTupleBufferIterator->currentTuple++;

    return result;
  }
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
The first constructor.

*/
  RelationDescriptor( const RelationDescriptor& desc ):
    noTuples( desc.noTuples ),
    tupleFileId( desc.tupleFileId ),
    lobFileId( desc.lobFileId )
    {}
/*
The copy constructor.

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
  PrivateRelation( const ListExpr typeInfo, const bool isTemporary ):
    noTuples( 0 ),
    tupleType( nl->Second( typeInfo ) ),
    tupleFile( false, 0, isTemporary ),
    lobFile( false, 0, isTemporary )
    {
      assert( tupleFile.Create() );
      assert( lobFile.Create() );
    }
/*
The first constructor. Creates an empty relation from a ~typeInfo~.

*/
  PrivateRelation( const TupleType& tupleType, const bool isTemporary ):
    noTuples( 0 ),
    tupleType( tupleType ),
    tupleFile( false, 0, isTemporary ),
    lobFile( false, 0, isTemporary )
    {
      assert( tupleFile.Create() );
      assert( lobFile.Create() );
    }
/*
The second constructor. Creates an empty relation from a ~tupleType~.

*/
  PrivateRelation( const TupleType& tupleType, const RelationDescriptor& relDesc, const bool isTemporary ):
    noTuples( relDesc.noTuples ),
    tupleType( tupleType ),
    tupleFile( false, 0, isTemporary ),
    lobFile( false, 0, isTemporary )
    {
      assert( tupleFile.Open( relDesc.tupleFileId ) );
      assert( lobFile.Open( relDesc.lobFileId ) );
    }
/*
The third constructor. Opens a previously created relation.

*/
  PrivateRelation( const ListExpr typeInfo, const RelationDescriptor& relDesc, const bool isTemporary ):
    noTuples( relDesc.noTuples ),
    tupleType( nl->Second( nl->First( typeInfo ) ) ),
    tupleFile( false, 0 ),
    lobFile( false, 0 )
    {
      assert( tupleFile.Open( relDesc.tupleFileId ) );
      assert( lobFile.Open( relDesc.lobFileId ) );
    }
/*
The fourth constructor. It opens a previously created relation using the ~typeInfo~ instead of
the ~tupleType~.

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
Relation::Relation( const ListExpr typeInfo, const bool isTemporary ):
  privateRelation( new PrivateRelation( typeInfo, isTemporary ) )
  {}

Relation::Relation( const TupleType& tupleType, const bool isTemporary ):
  privateRelation( new PrivateRelation( tupleType, isTemporary ) )
  {}

Relation::Relation( const TupleType& tupleType, const RelationDescriptor& relDesc, const bool isTemporary ):
  privateRelation( new PrivateRelation( tupleType, relDesc, isTemporary ) )
  {}

Relation::Relation( const ListExpr typeInfo, const RelationDescriptor& relDesc, const bool isTemporary ):
  privateRelation( new PrivateRelation( typeInfo, relDesc, isTemporary ) )
  {}

Relation::~Relation()
{
  delete privateRelation;
}

Relation *Relation::RestoreFromList( ListExpr typeInfo, ListExpr value, int errorPos, ListExpr& errorInfo, bool& correct )
{
  // This function should never be called.
  assert( false );
}

ListExpr Relation::SaveToList( ListExpr typeInfo )
{
  // This function should never be called.
  assert( false );
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
  privateRelation->tupleFile.Close();
  privateRelation->tupleFile.Drop();
  privateRelation->lobFile.Close();
  privateRelation->lobFile.Drop();
  delete this;
}

void Relation::AppendTuple( Tuple *tuple )
{
  tuple->GetPrivateTuple()->Save( &privateRelation->tupleFile, &privateRelation->lobFile );
  privateRelation->noTuples += 1;
}

void Relation::Clear()
{
  privateRelation->noTuples = 0;
  privateRelation->tupleFile.Close();
  assert( privateRelation->tupleFile.Drop() );
  assert( privateRelation->tupleFile.Create() );
  privateRelation->lobFile.Close();
  assert( privateRelation->lobFile.Drop() );
  assert( privateRelation->lobFile.Create() );
}

Tuple *Relation::GetTuple( const TupleId& id ) const
{
  Tuple *result = new Tuple( privateRelation->tupleType );
  result->GetPrivateTuple()->Open( &privateRelation->tupleFile,
                                   &privateRelation->lobFile,
                                   id );
  return result;
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
4.3 Struct ~PrivateRelationIterator~ (using ~PrefetchingIterator~)

This struct contains the private attributes of the class ~RelationIterator~.

*/
struct PrivateRelationIterator
{
  PrivateRelationIterator( const Relation& rel ):
    iterator( rel.privateRelation->tupleFile.SelectAllPrefetched() ),
    relation( rel ),
    endOfScan( false )
    {
    }
/*
The constructor.

*/
  ~PrivateRelationIterator()
  {
    delete iterator;
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
};

/*
4.4 Implementation of the class ~RelationIterator~ (using ~PrefetchingIterator~)

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
  if( !privateRelationIterator->iterator->Next() )
  {
    privateRelationIterator->endOfScan = true;
    return 0;
  }

  Tuple *result = new Tuple( privateRelationIterator->relation.privateRelation->tupleType );
  result->GetPrivateTuple()->Open( &privateRelationIterator->relation.privateRelation->tupleFile,
                                   &privateRelationIterator->relation.privateRelation->lobFile,
                                   privateRelationIterator->iterator );
  return result;
}

const bool RelationIterator::EndOfScan()
{
  return privateRelationIterator->endOfScan;
}
#else
/*
4.5 Struct ~PrivateRelationIterator~ (using ~SmiRecordFileIterator~)

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
4.6 Implementation of the class ~RelationIterator~ (using ~SmiRecordFileIterator~)

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
  SmiRecord *record = new SmiRecord();
  privateRelationIterator->iterator.Next( *record );

  if( EndOfScan() )
    return 0;

  Tuple *result = new Tuple( privateRelationIterator->relation.privateRelation->tupleType );
  result->GetPrivateTuple()->Open( &privateRelationIterator->relation.privateRelation->tupleFile,
                                   &privateRelationIterator->relation.privateRelation->lobFile,
                                   record );
  return result;
}

const bool RelationIterator::EndOfScan()
{
  return privateRelationIterator->iterator.EndOfScan();
}

#endif // _PREFETCHING_

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
    attr = r->GetAttribute( i )->Clone();
    t->PutAttribute( i, attr );
  }
  for (int j = 0; j < snoattrs; j++)
  {
    attr = s->GetAttribute( j )->Clone();
    t->PutAttribute( rnoattrs + j, attr );
  }
}

#endif // RELALG_PERSISTENT
