#include "FLOB.h"
#include <stdlib.h>

const int 
FLOB::SWITCH_THRESHOLD = 1024;

FLOB::FLOB(SmiRecordFile* inlobFile) : lob() { 
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
  if (start)
    free(start);
}

void FLOB::Destroy() {
  SmiRecordId test = 0;	 
  if (start)
    {
      free(start);
      start = 0;
      size = 0;
    }
  else
    lobFile->DeleteRecord(test);
}

/*
char *FLOB::Pin(int offset, int length, int &actuallength)
{
  if (start)
    {
      actuallength = (size - offset < length ? size - offset : length);
      return start;
    }
  else
    return lob.Pin(offset, length, actuallength);
}

void FLOB::Unpin()
{
  if (!start) lob.Unpin();
}
*/

void FLOB::Get(int offset, int length, char *target) {
  if (start != 0)
    memcpy(target, start + offset, length);
  else
    lob.Read(target, offset, length);
}

void FLOB::Write(int offset, int length, char *source) {
  if (start != 0)  
    memcpy(start + offset, source,
	   (size - offset < length ? size - offset : length));
  else	  
    lobFile->AppendRecord(lobId, lob);
    lob.Write(source, length, offset);
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

bool FLOB::SaveToLob()
{
  if (IsLob())
  {
    size = lob.Size();
    start = (char*)malloc(size);
    lob.Read(start, size, 0); /* Now the actual FLOB is a memory flob. Another
				call of SaveToLob will create a new lob if the
				flob is large. This is exactly the behaviour
				we need: an underlying lob should be copied 
				when its comprising tuple is copied. */
    return SaveToLob();
  }

  if (size > SWITCH_THRESHOLD)
  {
    SmiRecordId rid = 0;
    SmiRecord newLob;

    lobFile->AppendRecord(rid, newLob);
    newLob.Write(start, size, 0);
    lob = newLob;
    //    free(start);
    start = 0;
    size = 0;
    return true;
  }
  
  else return false;

}
