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
#include <stdlib.h>

/*

2 Implementation of class FLOB.

2.1 The threshold size is set to 1K.

*/
const int FLOB::SWITCH_THRESHOLD = 1024;

/*

2.2 Constructor.

*/
FLOB::FLOB(SmiRecordFile* inlobFile) : lob() {
	start = 0;
	size = 0;
	
  	lobFile = inlobFile; 
  	lobId = 0;
}

/*

2.3 Constructor.

Create from scratch.

*/
FLOB::FLOB(SmiRecordFile* inlobFile, int sz) : lob() {
  start = (char *) malloc(sz);
  size = sz;
  
  lobFile = inlobFile; 
  lobId = 0;
}

/*
2.4 Destructor. 

Destroy LOB instance.

*/
FLOB::~FLOB() {
  if (start != 0) free(start);
  start = 0;
}

/*

2.5 Destroy.

Destroy persistent representation.

*/
void FLOB::Destroy() {
  SmiRecordId id = 0;	 
  if (start != 0) {
      free(start);
      start = 0;
      size = 0;
  }
  else lobFile->DeleteRecord(id);
}

/*
2.6 Get

Read by copying

*/
void FLOB::Get(int offset, int length, char *target) {
  if (start != 0) {
    memcpy(target, start + offset, length);
  }
  else {
	lobFile->SelectRecord(lobId, lob);
    lob.Read(target, length, offset);
  }
}

/* 
2.7	Write

Write Flob data into source. 

*/
void FLOB::Write(int offset, int length, char *source) {
	if (start != 0) {
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
int FLOB::Size() {
  if (start != 0)
    return size;
  else
    return lob.Size();
}

/*
2.9 Resize

Resizes the FLOB.

*/
void FLOB::Resize(int newSize) {
  	if (start != 0) {
		// the data is still in memory.
		size = newSize;
    	realloc((void *)start, size);
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
		}
	}
}

  
/*
2.10 IsLob

Returns treue, if value stored in underlying LOB, otherwise false.

*/ 
bool FLOB::IsLob() {
  if (start != 0) return false; else return true;
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
bool FLOB::SaveToLob() {
 	if (IsLob()){
    	size = lob.Size();
    	start = (char*)malloc(size);
	    lob.Read(start, size, 0); 
    	/* Now the actual FLOB is a memory flob. Another
			call of SaveToLob will create a new lob if the
			flob is large. This is exactly the behaviour
			we need: an underlying lob should be copied 
			when its comprising tuple is copied. */
		SaveToLob();
  	}

  	if (size > SWITCH_THRESHOLD) {
		cout << "(FLOB)";
    	SmiRecord newLob;

		lobFile->AppendRecord(lobId, newLob);
    	newLob.Write(start, size, 0);
		newLob.Read(start, size, 0);
    	lob = newLob;
    	free(start);
    	start = 0;
    	size = lob.Size();
    	return true;
  	}  
  	else {
		return false;
	}
}

