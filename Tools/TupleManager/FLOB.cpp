#include "FLOB.h"
#include <stdlib.h>

const int FLOB::SWITCH_THRESHOLD = 102400;

FLOB::FLOB(SmiRecordFile* inlobFile) : lob() {
	start = 0;
	size = 0;
	
  	lobFile = inlobFile; 
  	lobId = 0;
}


FLOB::FLOB(SmiRecordFile* inlobFile, int sz) : lob() {
  start = (char *) malloc(sz);
  size = sz;
  
  lobFile = inlobFile; 
  lobId = 0;
}


FLOB::~FLOB() {
  if (start != 0) free(start);
  start = 0;
}

void FLOB::Destroy() {
  SmiRecordId id = 0;	 
  if (start != 0) {
      free(start);
      start = 0;
      size = 0;
  }
  else lobFile->DeleteRecord(id);
}

void FLOB::Get(int offset, int length, char *target) {
  if (start != 0) {
    memcpy(target, start + offset, length);
  }
  else {
	lobFile->SelectRecord(lobId, lob);
    lob.Read(target, length, offset);
  }
}

void FLOB::Write(int offset, int length, char *source) {
	if (start != 0) {
    	memcpy(start + offset, source, (size - offset < length ? size - offset : length));
	}    
  	else {	
    	lobFile->AppendRecord(lobId, lob);
    	lob.Write(source, length, offset);
	}
}

int FLOB::Size() {
  if (start != 0)
    return size;
  else
    return lob.Size();
}

/* 
 * to be worked out later
 * 
void FLOB::Resize(int size)
{
  if (start)
    start = (char*)realloc(start, size);
  else
    lob.Resize(size);
  this->size = size;
}
*/

bool FLOB::IsLob() {
  if (start != 0) return false; else return true;
}


int FLOB::Restore(char *address) {
  start = address;
  return size;
}

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

