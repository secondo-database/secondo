/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Implementation File of Module FLOB

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

*/
FLOB::FLOB(SmiRecordFile *inlobFile) : 
lobFile(inlobFile),
lob(),
lobId(0),
size(0),
start(0),
isLob(false),
insideTuple(false)
{
}

FLOB::FLOB(SmiRecordFile *inlobFile, const SmiRecordId id, const bool update) : 
lobFile(inlobFile),
lob(),
lobId(id),
size(0),
start(0),
isLob(false),
insideTuple(false)
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

2.3 Constructor.

Create from scratch.

*/
FLOB::FLOB(SmiRecordFile *inlobFile, int sz, const bool alloc, const bool update) :
lobFile(inlobFile),
lob(),
lobId(0),
size(sz),
start(alloc ? (char *) malloc(sz) : NULL),
isLob(false),
insideTuple(false)
{
}

/*
2.4 Destructor. 

Destroy LOB instance.

*/
FLOB::~FLOB() {
  if (start != 0)
  {
    assert( !IsLob() );
//    free(start);
    start = 0;
  }
}

/*

2.5 Destroy.

Destroy persistent representation.

*/
void FLOB::Destroy() {
  if (start != 0) {
      assert( !IsLob() );
//      free(start);
      start = 0;
      size = 0;
  }
  else 
  {
    assert( lobId != 0 );
    lobFile->DeleteRecord(lobId);
  }
}

/*
2.6 Get

Read by copying

*/
void FLOB::Get(int offset, int length, char *target) {
  if (!IsLob()) {
    assert( start != 0 && offset + length <= size );
    memcpy(target, start + offset, length);
  }
  else {
	lobFile->SelectRecord(lobId, lob);
    assert( offset + length <= (int)lob.Size() );
    lob.Read(target, length, offset);
  }
}

/* 
2.7	Write

Write Flob data into source. 

*/
void FLOB::Write(int offset, int length, char *source) {
	if (!IsLob()) {
          assert( start != 0 );
      	  memcpy(start + offset, source, (size - offset < length ? size - offset : length));
	}    
  	else {	
    	  lobFile->SelectRecord(lobId, lob);
    	  lob.Write(source, length, offset);
	}
}

/* 
2.8 Size

Returns the size of a FLOB.

*/
int FLOB::GetSize() {
  if (!IsLob())
    return size;
  else
    return lob.Size();
}

/*
2.9 Resize

Resizes the FLOB.

*/
void FLOB::Resize(int newSize) {
  if (!IsLob()) {
    // the data is still in memory.
    size = newSize;
    if( start == 0 )
    {
      start = (char *)malloc(size);
    }
    else
    {
      start = (char *)realloc((void *)start, size);
    }
  }
  else {
    // the data is saved in a lob.
    if (newSize <= size) {
      // the data become smaller
      size = newSize;
      lob.Truncate(newSize);
    }
    else {
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
2.10 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/ 
bool FLOB::IsLob() const {
  return isLob;
}
  
/*
2.11 Restore

Restore from byte string.

*/
int FLOB::Restore(char *address) {
  start = address;
  return size;
}

/* 
2.12 SaveToLob

Switch from Main Memory to LOB Representation 
if the size of this FLOB exceeds thresholdsize.

*/
bool FLOB::SaveToLob() 
{
  if (IsLob())
  {
    size = lob.Size();
    start = (char*)malloc(size);
    lob.Read(start, size, 0); 
    isLob = false;
      /* Now the actual FLOB is a memory flob. Another
         call of SaveToLob will create a new lob if the
	 flob is large. This is exactly the behaviour
	 we need: an underlying lob should be copied 
	 when its comprising tuple is copied. */
    SaveToLob();
  }

  if (size > SWITCH_THRESHOLD && insideTuple ) 
  {
    cout << "(FLOB inside tuple)";
    SmiRecord newLob;
    isLob = true;
    lobFile->AppendRecord(lobId, newLob);
    newLob.Write(start, size, 0);
    lob = newLob;
    free(start);
    start = 0;
    size = lob.Size();
    return true;
  }  
  else if( !insideTuple ) 
  {
    cout << "(FLOB stantalone)";
    if( lobId == 0 )
    {
      isLob = true;
      lobFile->AppendRecord(lobId, lob);
    }
    lob.Write(start, size, 0);
    size = lob.Size();
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
2.13 GetLobId

Returns the lob record id.

*/
const SmiRecordId FLOB::GetLobId()
{
  return lobId;
}

/*
2.14 SetInsideTuple

Sets this flob to be inside a tuple or not depending on the value of ~it~.

*/
void FLOB::SetInsideTuple( const bool it )
{
  insideTuple = it;
}

