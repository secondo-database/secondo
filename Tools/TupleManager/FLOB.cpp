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

Create a new FLOB from scratch.

*/
FLOB::FLOB() : 
lobFile(0),
lob(),
lobId(0),
size(0),
start(0),
isLob(false),
insideTuple(false),
freeStart(false)
{
}

/*
2.3 Constructor.

Create a new FLOB from scratch and initialize it
with size ~sz~ if ~alloc~ is true. This ~alloc~
flag is important because sometimes the tuple
manager allocates the memory for the FLOB, specially
when the FLOB stays on the extension of the tuple.
In these situations, the FLOB does not allocate any
memory and leaves it to be done by the tuple.

*/

FLOB::FLOB(SmiRecordFile *inlobFile, const SmiRecordId id, const bool update) : 
lobFile(inlobFile),
lob(),
lobId(id),
size(0),
start(0),
isLob(false),
insideTuple(false),
freeStart(true)
{
  SmiFile::AccessType at = update ? SmiFile::Update : SmiFile::ReadOnly;
  assert( lobFile->SelectRecord( lobId, lob, at ) );
  size = lob.Size(); 
  if( size <= SWITCH_THRESHOLD )
  {
    isLob = false;
    start = (char *)malloc( size );
    lob.Read(start, size, 0);
  }
  else
  {
    isLob = true;
  }
}

/*
2.4 Constructor.

Opens a FLOB from a file ~inlobFile~ and record identification ~id~.
The flag ~update~ tells if the FLOB is being opened for update or
read only.

*/
FLOB::FLOB(const int sz, const bool alloc, const bool update) :
lobFile(0),
lob(),
lobId(0),
size(sz),
start(alloc ? (char *) malloc(sz) : NULL),
isLob(false),
insideTuple(false),
freeStart(alloc)
{
}

/*
2.5 Destructor. 

Destroy LOB instance.

*/
FLOB::~FLOB() {
  if (start != 0)
  {
    assert( !IsLob() );
    if( freeStart ) free(start);
    start = 0;
  }
}

/*

2.6 Destroy.

Destroy persistent representation.

*/
void FLOB::Destroy() {
  if (start != 0) {
      assert( !IsLob() );
      if( freeStart ) free(start);
      start = 0;
      size = 0;
  }
  else 
  {
    assert( lobFile != 0 && lobId != 0 );
    lobFile->DeleteRecord(lobId);
  }
}

/*
2.7 Get

Read by copying

*/
void FLOB::Get(const int offset, const int length, char *target) 
{
  if (!IsLob()) 
  {
    assert( start != 0 && offset + length <= size );
    memcpy(target, start + offset, length);
  }
  else 
  {
    assert( lobFile != 0 );
	  lobFile->SelectRecord(lobId, lob);
    assert( offset + length <= (int)lob.Size() );
    lob.Read(target, length, offset);
  }
}

/* 
2.8	Write

Write Flob data into source. 

*/
void FLOB::Write(const int offset, const int length, char *source) 
{
	if (!IsLob()) 
  {
    assert( start != 0 );
    memcpy(start + offset, source, (size - offset < length ? size - offset : length));
	}    
  else 
  {	
    assert( lobFile != 0 );
    lobFile->SelectRecord(lobId, lob);
    lob.Write(source, length, offset);
	}
}

/* 
2.9 Size

Returns the size of a FLOB.

*/
const int FLOB::GetSize() const  
{
  return size;
}

/*
2.10 Resize

Resizes the FLOB.

*/
void FLOB::Resize(const int newSize) 
{
  if (!IsLob()) 
  // the data is still in memory.
  {
    size = newSize;
    if( start == 0 )
    {
      start = (char *)malloc(size);
      freeStart = true;
    }
    else
    {
      assert( freeStart );
      start = (char *)realloc((void *)start, size);
    }
  }
  else 
  // the data is saved in a lob.
  {
    assert( lobFile != 0 && lobId != 0 );
    if (newSize <= size) 
    {
      // the data become smaller
      size = newSize;
      lob.Truncate(newSize);
    }
    else 
    {
      // the data become larger.
      char *data = (char *)malloc(size);
      lob.Read(data, size, 0);
      realloc((void *)data, newSize);
      lob.Write(data, newSize, 0);
      size = newSize;
      free(data);
    }
  }
}

  
/*
2.11 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/ 
const bool FLOB::IsLob() const 
{
  return isLob;
}
  
/*
2.12 Restore

Restore from byte string.

*/
const int FLOB::Restore(char *address) 
{
  start = address;
  return size;
}

/* 
2.13 SaveToLob

Switch from Main Memory to LOB Representation 
if the size of this FLOB exceeds thresholdsize.

*/
const bool FLOB::SaveToLob( SmiRecordFile *lobfile ) 
{
  lobFile = lobfile;
  if (IsLob())
  {
    size = lob.Size();
    start = (char*)malloc(size);
    freeStart = true;
    lob.Read(start, size, 0); 
    isLob = false;
      /* Now the actual FLOB is a memory flob. Another
         call of SaveToLob will create a new lob if the
	 flob is large. This is exactly the behaviour
	 we need: an underlying lob should be copied 
	 when its comprising tuple is copied. */
    SaveToLob( lobFile );
  }

  if (size > SWITCH_THRESHOLD && insideTuple ) 
  {
    isLob = true;
    lobFile->AppendRecord(lobId, lob);
    lob.Write(start, size, 0);
    if( freeStart ) free(start);
    start = 0;
    size = lob.Size();
    return true;
  }  
  else if( !insideTuple ) 
  {
    if( lobId == 0 )
    {
      isLob = true;
      lobFile->AppendRecord(lobId, lob);
    }
    lob.Write(start, size, 0);
    size = lob.Size();
    assert( freeStart );
    free(start);
    start = 0;
    return true;
  }  
  else 
  {
    assert( !IsLob() );
    return false;
  }
}

/*
2.14 GetLobId

Returns the lob record id.

*/
const SmiRecordId FLOB::GetLobId() const
{
  return lobId;
}

/*
2.15 SetInsideTuple

Sets this flob to be inside a tuple.

*/
void FLOB::SetInsideTuple()
{
  insideTuple = true;
}

