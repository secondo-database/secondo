/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Implementation File of Module FLOB

Victor Almeida, 24/04/03. Adapting the class to accept standalone objects
and objects inside tuples transparently.

Mirco G[ue]nster, 31/09/02 End of porting to the new SecondoSMI.

Markus Spiekermann, 14/05/02. Begin of porting to the new SecondoSMI.

Stefan Dieker, 03/05/98

1 Includes

*/
#include "FLOB.h"
#include <iostream>
#include <stdlib.h>

/*

2 Implementation of class FLOB.

2.1 The threshold size is set to 1K.

*/
const int FLOB::SWITCH_THRESHOLD = 1024;

/*

2.2 Constructor.

Create a new InMemory FLOB and initializes it with
size ~sz~.

*/
FLOB::FLOB( int sz ) :
type( InMemory )
{
  size = sz;
  if( sz > 0 )
  {
    fd.inMemory.buffer = (char*)malloc( sz );
    fd.inMemory.freeBuffer = true;
  }
  else
  {
    fd.inMemory.buffer = 0;
    fd.inMemory.freeBuffer = false;
  }
  assert( (size > 0  && fd.inMemory.buffer != 0 ) ||
          (size == 0 && fd.inMemory.buffer == 0 ) );
}

/*
2.5 Destructor.

Destroy LOB instance.

*/
FLOB::~FLOB()
{
  if( type == InMemory )
  {
    if( fd.inMemory.freeBuffer )
      free( fd.inMemory.buffer );
  }
#ifdef PERSISTENT_FLOB
  if( type == InDiskMemory )
  {
    if( fd.inDiskMemory.pageId != -1 )
      free( fd.inDiskMemory.pageBuffer );
  }
#endif
}

/*
2.4 BringToMemory

Brings a disk lob to memory, i.e., converts a flob in ~InDiskLarge~
state to a ~InMemory~ state.

*/
char *FLOB::BringToMemory()
{
#ifdef PERSISTENT_FLOB

    SmiRecordFile *lobFile = fd.inDiskLarge.lobFile;
    SmiRecordId lobId = fd.inDiskLarge.lobId;

    type = InDiskMemory;
    fd.inDiskMemory.lobFile = lobFile;
    fd.inDiskMemory.lobId = lobId;
    fd.inDiskMemory.pageBuffer = 0;
    fd.inDiskMemory.pageId = -1;

#else

    if( type != InMemory )
    {
      assert( type == InDiskLarge );
      char *buffer = (char*) malloc( size );
      SmiRecord lobRecord;
      assert( fd.inDiskLarge.lobFile->SelectRecord( fd.inDiskLarge.lobId, lobRecord ) );
      lobRecord.Read( buffer, size, 0 );

      type = InMemory;
      fd.inMemory.buffer = buffer;
      fd.inMemory.freeBuffer = true;
    }

#endif

    assert( type == InMemory );
    return fd.inMemory.buffer;
}

/*
2.7 Get

Read by copying

*/
void FLOB::Get( int offset, int length, void *target )
{
  assert( type != Destroyed );
  assert( size > 0 );

  if( type == InMemory )
  {
    assert( fd.inMemory.buffer != 0 && offset + length <= size );
    memcpy( target, fd.inMemory.buffer + offset, length );

    assert( (size > 0  && fd.inMemory.buffer != 0 ) ||
            (size == 0 && fd.inMemory.buffer == 0 ) );

  }
  else if( type == InDiskLarge )
  {
    assert( fd.inDiskLarge.lobFile != 0 &&
            fd.inDiskLarge.lobId != 0 );

    BringToMemory();
    Get( offset, length, target );
  }

#ifdef PERSISTENT_FLOB
  else if( type == InDiskMemory )
  {
    assert( length < SWITCH_THRESHOLD );
    assert( fd.inDiskMemory.lobFile != 0 && fd.inDiskMemory.lobId != 0 );

    if( fd.inDiskMemory.pageId == -1 )
    {
      SmiRecord lobRecord;
      assert( fd.inDiskMemory.lobFile->SelectRecord( fd.inDiskMemory.lobId, lobRecord ) );

      assert( fd.inDiskMemory.pageBuffer == 0 );
      fd.inDiskMemory.pageId = offset / SWITCH_THRESHOLD;
      if( offset + SWITCH_THRESHOLD > (int)lobRecord.Size() )
        // This is the last page 
      {
        fd.inDiskMemory.pageBuffer = (char*) malloc( lobRecord.Size() - (fd.inDiskMemory.pageId * SWITCH_THRESHOLD) );
        lobRecord.Read( fd.inDiskMemory.pageBuffer, 
                         lobRecord.Size() - (fd.inDiskMemory.pageId * SWITCH_THRESHOLD), 
                         fd.inDiskMemory.pageId * SWITCH_THRESHOLD );
      }
      else
      {
        fd.inDiskMemory.pageBuffer = (char*) malloc( SWITCH_THRESHOLD );
        lobRecord.Read( fd.inDiskMemory.pageBuffer, SWITCH_THRESHOLD, fd.inDiskMemory.pageId * SWITCH_THRESHOLD );
      }

      Get( offset, length, target ); 
    }
    else
      // There is one page allocated in the structure
    {
      if( offset >= fd.inDiskMemory.pageId * SWITCH_THRESHOLD &&
          offset < fd.inDiskMemory.pageId * SWITCH_THRESHOLD + SWITCH_THRESHOLD )
        // The offset is between the page that is in memory
      {
        int insidePageOffset = offset % SWITCH_THRESHOLD;
        if( insidePageOffset + length <= SWITCH_THRESHOLD )
          // The element is completely covered by the page
        {
          memcpy( target, fd.inDiskMemory.pageBuffer + insidePageOffset, length );
        }
        else
          // We need to read the first part of the element and then the next in the next page
        {
          memcpy( target, fd.inDiskMemory.pageBuffer + insidePageOffset, (SWITCH_THRESHOLD - insidePageOffset) );

          assert( fd.inDiskMemory.pageBuffer != 0 );
          free( fd.inDiskMemory.pageBuffer );
          fd.inDiskMemory.pageBuffer = 0;
          fd.inDiskMemory.pageId = -1;

          Get( offset + (SWITCH_THRESHOLD - insidePageOffset), 
               length - (SWITCH_THRESHOLD - insidePageOffset), 
               target + (SWITCH_THRESHOLD - insidePageOffset) );
        }
      }
      else
        // It is needed to allocate another page
      { 
        assert( fd.inDiskMemory.pageBuffer != 0 );
        free( fd.inDiskMemory.pageBuffer ); 
        fd.inDiskMemory.pageBuffer = 0;
        fd.inDiskMemory.pageId = -1;

        Get( offset, length, target );
      }
    }
  }
#endif
  else
    assert( false );
}

/*
2.8	Put

Write Flob data into source.

*/
void FLOB::Put( int offset, int length, const void *source)
{
  assert( type != Destroyed );
  assert( size > 0 );

  if( type == InMemory )
  {
    assert( fd.inMemory.buffer != 0 && offset + length <= size );
    memcpy( fd.inMemory.buffer + offset, source, length ); 

    assert( (size > 0  && fd.inMemory.buffer != 0 ) ||
            (size == 0 && fd.inMemory.buffer == 0 ) );
  }
  else if( type == InDiskLarge )
  {
    assert( fd.inDiskLarge.lobFile != 0 &&
            fd.inDiskLarge.lobId != 0 );

    BringToMemory();
    Put( offset, length, source );
  }
#ifdef PERSISTENT_FLOB
  else if( type == InDiskMemory )
  {
    assert( length < SWITCH_THRESHOLD );
    assert( fd.inDiskMemory.lobFile != 0 && fd.inDiskMemory.lobId != 0 );

    SmiRecord lobRecord;
    assert( fd.inDiskMemory.lobFile->SelectRecord( fd.inDiskMemory.lobId, lobRecord ) );

    if( fd.inDiskMemory.pageId == -1 )
    {
      assert( fd.inDiskMemory.pageBuffer == 0 );
      fd.inDiskMemory.pageId = offset / SWITCH_THRESHOLD;

      if( offset + SWITCH_THRESHOLD > (int)lobRecord.Size() )
        // This is the last page
      {
        fd.inDiskMemory.pageBuffer = (char*) malloc( lobRecord.Size() - (fd.inDiskMemory.pageId * SWITCH_THRESHOLD) );
        lobRecord.Read( fd.inDiskMemory.pageBuffer,
                         lobRecord.Size() - (fd.inDiskMemory.pageId * SWITCH_THRESHOLD),
                         fd.inDiskMemory.pageId * SWITCH_THRESHOLD );
      }
      else
      {
        fd.inDiskMemory.pageBuffer = (char*) malloc( SWITCH_THRESHOLD );
        lobRecord.Read( fd.inDiskMemory.pageBuffer, 
                         SWITCH_THRESHOLD, 
                         fd.inDiskMemory.pageId * SWITCH_THRESHOLD );
      }
      Put( offset, length, source );
    }
    else
      // There is one page allocated in the structure
    {
      if( offset >= fd.inDiskMemory.pageId * SWITCH_THRESHOLD &&
          offset < fd.inDiskMemory.pageId * SWITCH_THRESHOLD + SWITCH_THRESHOLD )
        // The offset is between the page that is in memory
      {
        int insidePageOffset = offset % SWITCH_THRESHOLD;
        if( insidePageOffset + length <= SWITCH_THRESHOLD )
          // The element is completely covered by the page
        {
          memcpy( fd.inDiskMemory.pageBuffer + insidePageOffset, source, length );
        }
        else
          // We need to read the first part of the element and then the next in the next page
        {
          memcpy( fd.inDiskMemory.pageBuffer + insidePageOffset, source, (SWITCH_THRESHOLD - insidePageOffset) );

          assert( fd.inDiskMemory.pageBuffer != 0 );
          free( fd.inDiskMemory.pageBuffer ); 
          fd.inDiskMemory.pageBuffer = 0;
          fd.inDiskMemory.pageId = -1;

          Put( offset + (SWITCH_THRESHOLD - insidePageOffset),
               length - (SWITCH_THRESHOLD - insidePageOffset),
               source + (SWITCH_THRESHOLD - insidePageOffset) );
        }
      }
      else
        // It is needed to allocate another page
      {
        assert( fd.inDiskMemory.pageBuffer != 0 );
        free( fd.inDiskMemory.pageBuffer );
        fd.inDiskMemory.pageBuffer = 0;
        fd.inDiskMemory.pageId = -1;

        Put( offset, length, source );
      }
    }
  }
#endif
  else
    assert( false );
}

/*
2.9 Size

Returns the size of a FLOB.

*/
int FLOB::Size() const
{
  return size;
}

/*
2.10 Destroy 

Destroys the physical representation of the FLOB.

*/
void FLOB::Destroy()
{
  assert( type != Destroyed );

  if( type == InMemory )
  {
    if( size > 0 )
    {
      assert( fd.inMemory.buffer != 0 );
      if( fd.inMemory.freeBuffer )
      {
        free( fd.inMemory.buffer );
        fd.inMemory.buffer = 0;
      }
    }
  }
  else if( type == InDiskLarge )
  {
    assert( size > SWITCH_THRESHOLD );
    assert( fd.inDiskLarge.lobFile != 0 && fd.inDiskLarge.lobId != 0 );

    assert( fd.inDiskLarge.lobFile->DeleteRecord( fd.inDiskLarge.lobId ) );
  }
#ifdef PERSISTENT_FLOB
  else if( type == InDiskMemory )
  {
    assert( size > SWITCH_THRESHOLD );
    assert( fd.inDiskMemory.lobFile != 0 && fd.inDiskMemory.lobId != 0 );

    if( fd.inDiskMemory.pageId >= 0 )
    {
      assert( fd.inDiskMemory.pageBuffer != 0 );
      free( fd.inDiskMemory.pageBuffer );
      fd.inDiskMemory.pageId = 0;
    }
     
    assert( fd.inDiskMemory.lobFile->DeleteRecord( fd.inDiskMemory.lobId ) );
  }
#endif
  size = 0;
  type = Destroyed;
}

/*
2.10 Clear 

Clears the FLOB.

*/
void FLOB::Clear()
{
  assert( type != Destroyed );

  if( type == InMemory )
  {
    if( size > 0 )
    {
      if( fd.inMemory.freeBuffer == true )
      {
        free( fd.inMemory.buffer ); 
        fd.inMemory.buffer = 0;
      }
    }
  }
#ifdef PERSISTENT_FLOB
  else if( type == InDiskMemory )
  {
    if( fd.inDiskMemory.pageId >= 0 )
    {
      assert( fd.inDiskMemory.pageBuffer != 0 );
      free( fd.inDiskMemory.pageBuffer ); 
      fd.inDiskMemory.pageBuffer = 0;
      fd.inDiskMemory.pageId = -1;
    }
    assert( fd.inDiskMemory.lobFile != 0 && fd.inDiskMemory.lobId != 0 );

    SmiRecord lobRecord;
    assert( fd.inDiskMemory.lobFile->SelectRecord( fd.inDiskMemory.lobId, lobRecord ) );

    lobRecord.Truncate( 0 );
  }
#endif
  else
    // This code cannot be reached because the other
    // cases are not implemented yet.
    assert( false );

  size = 0;
  assert( ( type == InMemory && ((size > 0  && fd.inMemory.buffer != 0 ) ||
                                 (size == 0 && fd.inMemory.buffer == 0 )) ) ||
          ( type != InMemory ) );
}

/*
2.10 Resize

Resizes the FLOB.

*/
void FLOB::Resize( int newSize )
{
  assert( type != Destroyed );
  assert( newSize > 0 );

  if( type == InMemory )
  {
    if( size == 0 )
    {
      fd.inMemory.freeBuffer = true;
      fd.inMemory.buffer = (char *) malloc( newSize );
    }
    else if( fd.inMemory.freeBuffer == true )
    {
      fd.inMemory.buffer = (char *)realloc( fd.inMemory.buffer, newSize ); 
    }
    else
      // This code cannot be reached because the other
      // cases are not implemented yet.
      assert( false );
    
    assert( (newSize > 0  && fd.inMemory.buffer != 0 ) ||
            (newSize == 0 && fd.inMemory.buffer == 0 ) );
  }
#ifdef PERSISTENT_FLOB
  else if( type == InDiskMemory )
  {
    if( size != newSize )
    {
      if( fd.inDiskMemory.pageId >= 0 ) 
      {
        assert( fd.inDiskMemory.pageBuffer != 0 );
        free( fd.inDiskMemory.pageBuffer );
        fd.inDiskMemory.pageBuffer = 0;
        fd.inDiskMemory.pageId = -1;
      }
      char zero = 0;
       
      assert( fd.inDiskMemory.lobFile != 0 && fd.inDiskMemory.lobId != 0 );

      SmiRecord lobRecord;
      assert( fd.inDiskMemory.lobFile->SelectRecord( fd.inDiskMemory.lobId, lobRecord ) );

      if( size > newSize ) 
        lobRecord.Write( &zero, 1, newSize - 1 );
      else if( size < newSize )
        lobRecord.Truncate( newSize );
    }
  }
#endif
  else
    // This code cannot be reached because the other
    // cases are not implemented yet.
    assert( false );

  size = newSize;

  assert( ( type == InMemory && ((size > 0  && fd.inMemory.buffer != 0 ) ||
                                 (size == 0 && fd.inMemory.buffer == 0 )) ) ||
          ( type != InMemory ) );
}

/*
2.10 SetLobFile

*/
void FLOB::SetLobFile( SmiRecordFile* lobFile )
{
  assert( type == InDiskLarge && fd.inDiskLarge.lobFile == 0 && fd.inDiskLarge.lobId != 0 );
  fd.inDiskLarge.lobFile = lobFile;
}

/*
2.10 SaveToLob

*/
void FLOB::SaveToLob( SmiRecordFile& lobFile, SmiRecordId lobId )
{
#ifdef PERSISTENT_FLOB
  if( type == InDiskMemory )
  {
    if( fd.inDiskMemory.pageId >= 0 )
      free( fd.inDiskMemory.pageBuffer );

    SmiRecordId lobId = fd.inDiskMemory.lobId;

    type = InDiskLarge;
    fd.inDiskLarge.lobFile = 0;
    fd.inDiskLarge.lobId = lobId;
  }
  else
  {
#endif
  assert( type == InMemory && size > SWITCH_THRESHOLD );

  SmiRecord lob;
  if( lobId == 0 )
    assert( lobFile.AppendRecord( lobId, lob ) );
  else
    assert( lobFile.SelectRecord( lobId, lob ) );

  lob.Write( fd.inMemory.buffer, size, 0 );

  if( fd.inMemory.freeBuffer )
  {
    free( fd.inMemory.buffer );
    fd.inMemory.buffer = 0;
    fd.inMemory.freeBuffer = false;
  }

  type = InDiskLarge;
  fd.inDiskLarge.lobFile = 0;
  fd.inDiskLarge.lobId = lobId;
#ifdef PERSISTENT_FLOB
  }
#endif
}

/*
2.10 SaveToExtensionTuple

*/
void FLOB::SaveToExtensionTuple( void *extensionTuple )
{
  assert( type == InMemory && size <= SWITCH_THRESHOLD );

  Get( 0, size, extensionTuple );

  if( fd.inMemory.freeBuffer )
  {
    free( fd.inMemory.buffer );
    fd.inMemory.buffer = 0;
    fd.inMemory.freeBuffer = false;
  }

  type = InDiskSmall;
}

/*
2.11 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/
bool FLOB::IsLob() const
{
  assert( type != Destroyed );

  if( type == InDiskLarge )
  {
    assert( size > SWITCH_THRESHOLD );
    return true;
  }
  else if( type == InMemory && size > SWITCH_THRESHOLD )
    return true;
#ifdef PERSISTENT_FLOB
  else if( type == InDiskMemory )
  {
    assert( size > SWITCH_THRESHOLD );
    return true;
  }
#endif
  return false;
}

/*
2.12 GetType

*/
FLOB_Type FLOB::GetType() const
{
  return type;
}

/*
2.12 Restore

Restore from byte string.

*/
void FLOB::Restore( void *newBuffer )
{
  assert( type == InDiskSmall );

  type = InMemory;
  fd.inMemory.buffer = (char*)newBuffer;
  fd.inMemory.freeBuffer = false;
}

