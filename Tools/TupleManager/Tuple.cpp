/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Implementation of Module TupleManager
  
Mirco G[ue]nster, 10/08/02 finish of porting.

Markus Spiekermann, 06/14/02 port to new secondo core System

Stefan Dieker, 01/24/98

1 Overview

This is the implementation of the tuple manager.
For an overview see also Tuple.h.

2 Includes

*/

#include <iostream>
#include "Tuple.h"
#include "SecondoSystem.h"

/*

3 AttributeInfo

  The following method implements the functionality of
  an AttributeInfo. Such AttributeInfo consists of several
  attributes:

  destruct, changed, prev, next, value, size, refCount
  

  destruct: indicates wheather this tuple attribute should 
  	be destroyed (deleted) if the owner tuple itself will 
	be destroyed and refCount holds the value 0. This flag 
	is false if this tuple component was put by the 
	Put-method of the Tuple. In this case the user of this 
	tuple has to delete the value himself. Otherwise if this tuple 
	component was put by the DelPut-method or AttrPut-method
	this flag is true and this tuple component will also be 
	destroyed if refCount holds the value 0.

  changed: indicates wheather this tuple component was changed.
  	If so, the memory representation of a tuple differs from 
	the persistent representation.

  prev/next: these pointers are important if tuple values are inserted
  	into a tuple by the AttrPut-method.
	All tuple elements which are members of more than one tuple are
	linked by prev/next.
	
  size:	the size of a tuple element.
  
  refCount: the number of tuples using this attribute.
  
*/

/*
 
3.1 constructor of Tuple::AttribInfo. 

*/
Tuple::AttributeInfo::AttributeInfo(){
  destruct = changed = false; 
  prev = 0; next = 0;
  value = 0; size = 0;
  refCount = 0;
}

/*
 
3.2 Tuple::AttributeInfo::incRefCount

increase RefCount in all AttributeInfos referring to the TupleElement value 

*/
void Tuple::AttributeInfo::incRefCount() {
  AttributeInfo *ai = this;
  
  if (ai->next != 0) {
    while (ai->next != 0) ai = ai->next;
    do {
      ai->refCount++;
      ai = ai->prev;
    } while (ai != 0);
  }
  else if (ai->prev != 0) {
    while (ai->prev != 0) ai = ai->prev;
    do {
      ai->refCount++;
      ai = ai->next;
    } while (ai != 0);
  }
  else refCount++;
}

/*

3.3 Tuple::AttributeInfo::decRefCount
 
decrease RefCount in all AttributeInfos
referring to the TupleElement value 

*/
void Tuple::AttributeInfo::decRefCount() {
  AttributeInfo *ai = this;
  
  if (ai->next != 0) {
    while (ai->next != 0) ai = ai->next;
    do {
      ai->refCount--;
      ai = ai->prev;
    } while (ai != 0);
  }
  else if (ai->prev != 0) {
    while (ai->prev != 0) ai = ai->prev;
    do {
      ai->refCount--;
      ai = ai->next;
    } while (ai != 0);
  }
  else refCount--;
}


/*
 
3.4 Tuple::AttributeInfo::deleteValue

delete value in all AttributeInfos referring 
to the TupleElement value. 

*/
void Tuple::AttributeInfo::deleteValue() {
  AttributeInfo *ai = this;
  // delete this value
  delete value;
  // In all other tuples which use also this value
  // their value pointer have to set to 0.
  if (ai->next != 0) {
    while (ai->next != 0) ai = ai->next;
    do {
      ai->value = 0;
      ai = ai->prev;
    } while (ai != 0);
  }
  else if (ai->prev != 0) {
    while (ai->prev != 0) ai = ai->prev;
    do {
      ai->value = 0;
      ai = ai->next;
    } while (ai != 0);
  }
  else value = 0;
}

/*

4 TupleAttributes

constructor. Sets all member variables, including size. 

*/
TupleAttributes::TupleAttributes(int noAttrs, AttributeType *attrTypes) {
  totalNumber = noAttrs;
  type = attrTypes;
  totalSize = 0;
  for (int i = 0; i < noAttrs; i++) {
    totalSize = totalSize + attrTypes[i].size;
  }
}

/*

5 Tuple

*/

/*

5.1 Init. 

This method is used by all constructors of the class tuple.
It initializes all members of a tuple.
	
*/
void Tuple::Init(const TupleAttributes *attributes) {
  diskTupleId = 0;
  lobFile = 0;
  recFile = 0;
  attribInfo = 0;
  state = Fresh;
  attrNum = attributes->totalNumber;
  memorySize = attributes->totalSize;
  extensionSize = 0;
  error = false;
  
  // get Reference of AlgebraManager
  algM = SecondoSystem::GetAlgebraManager();

  // allocate memory for tuple representation
  memoryTuple = (char *)malloc(attributes->totalSize);
  extensionTuple = 0;
  
  // copy attribute info
  attribInfo = new AttributeInfo[attrNum];
  for (int i = 0; i < attrNum; i++) {
    int actSize = attributes->type[i].size;
    int algId = attributes->type[i].algId;
    int typeId = attributes->type[i].typeId;
    attribInfo[i].size = actSize;
    char *attrMem = (char *)malloc(actSize);
    // call cast function of tuple attribute
    attribInfo[i].value = (TupleElement *) (*(algM->Cast(algId, typeId)))(attrMem);
    attribInfo[i].destruct = false;
    attribInfo[i].incRefCount();
  } 
}

/*

5.2 Constructor of the class Tuple. 

Creates a fresh tuple.

*/
Tuple::Tuple(const TupleAttributes *attributes) {
  // initialize member attributes.
  Init(attributes);
  
  // this tuple is not persistent yet.	
  state = Fresh;
}

/*

5.3 Constructor of the class Tuple. 

Creates a solid tuple and
reads its data into memory.

*/
Tuple::Tuple(SmiRecordFile* recfile, SmiRecordId rid, SmiRecordFile *lobfile,
	     const TupleAttributes *attributes, SmiFile::AccessType mode) {
  // initialize member attributes.
  Init(attributes);
  
  diskTupleId = rid;
  recFile = recfile;
  lobFile = lobfile;
  
  // read tuple header and memory tuple from disk
  bool ok = recFile->SelectRecord(diskTupleId, diskTuple, mode);
  if (ok == false) {
    error = true;
    return;
  }
  TupleHeader th = {0};
  char *buf = (char *)malloc(sizeof(TupleHeader));
  ok = diskTuple.Read(buf, sizeof(TupleHeader), 0);
  ok = diskTuple.Read(memoryTuple, memorySize, sizeof(TupleHeader)) && ok;
  memcpy(&(th.size), buf, sizeof(int));
  free(buf);
  
  if (ok == false) {
    error = true;
    return;
  }
  
  state = SolidRead;
  
  // read attribute values from memoryTuple.
  char *valuePtr = memoryTuple;
  for (int i = 0; i < attrNum; i++) {
    memcpy(attribInfo[i].value, valuePtr, attribInfo[i].size);
    valuePtr = valuePtr + attributes->type[i].size;
    FLOB *tmpFLOB;
    for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
      tmpFLOB = attribInfo[i].value->GetFLOB(j);
      tmpFLOB->lobFile = lobFile;
    }
    
  }
  
  if (th.size > 0) {
    // Determine the size of FLOB data stored in lobFile
    FLOB *tmpFLOB;
    extensionSize = 0;
    
    for (int i = 0; i < attrNum; i++) {
      for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
	tmpFLOB = attribInfo[i].value->GetFLOB(j);
	extensionSize = extensionSize + tmpFLOB->size;
      }
    }
    
    extensionTuple = 0;
    // move FLOB data to extension tuple if exists.
    if (extensionSize > 0) {
      extensionTuple = (char *)malloc(extensionSize);
      ok = diskTuple.Read(extensionTuple, extensionSize, sizeof(TupleHeader) + memorySize) && ok;
      
      char *extensionPtr = extensionTuple;
      for (int i = 0; i < attrNum; i++) {
	for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
	  tmpFLOB = attribInfo[i].value->GetFLOB(j);
	  if (!tmpFLOB->IsLob()) {
	    extensionPtr = extensionPtr + tmpFLOB->Restore(extensionPtr);
	  }	
	}
      }				
    }
  }
  
  if (ok == false) error = true;
}

/*

5.4 Destructor of class Tuple.

previously called Close. 

*/
Tuple::~Tuple() {
  if (memoryTuple != 0) free(memoryTuple);
  
  // delete attributes
  for (int i = 0; i < attrNum; i++) {
    attribInfo[i].decRefCount();
    if ((attribInfo[i].refCount == 0) && (attribInfo[i].destruct == true)) {
      attribInfo[i].deleteValue();
    }
    attribInfo[i].destruct = false;
    
    
    // attribute infos are members of a double connected 
    // list if tuple elements 
    // are put via AttrPut and DelPut.
    if (attribInfo[i].prev == 0) {
      if (attribInfo[i].next == 0) {
	// do nothing.
      }
      else {
	// attribInfo[i].next != 0
	attribInfo[i].next->prev = 0;
      }
    }
    else {
      // attribInfo[i].prev != 0
      if (attribInfo[i].next == 0) {
	attribInfo[i].prev->next = 0;
      }
      else {
	// prev and next != 0
	attribInfo[i].prev->next = attribInfo[i].next;
	attribInfo[i].next->prev = attribInfo[i].prev;
      }
    }
    // so the standard destructor cannot delete value.
    attribInfo[i].value = 0;
  }
  
  if (attribInfo != 0) delete[] attribInfo;	
}

/*

5.5 CalcSizeOfFLOBData

Determine the size of FLOB data stored in lobFile.

*/
int Tuple::CalcSizeOfFLOBData() {
  FLOB *tmpFLOB;
  int result = 0;
  for (int i = 0; i < attrNum; i++) {
    for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
      tmpFLOB = attribInfo[i].value->GetFLOB(j);
      bool stl = tmpFLOB->SaveToLob();
      if (stl == false) {
	result = result + tmpFLOB->Size();
      }
    }
  }
  return result;
}

/*

5.6 MoveFLOBDataToExtensionTuple

move FLOB data to extension tuple. 

*/
char *Tuple::MoveFLOBDataToExtensionTuple() {
  char *result = (char *)malloc(extensionSize);
  char *extensionPtr = result;
  FLOB *tmpFLOB;
  for (int i = 0; i < attrNum; i++) {
    for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
      tmpFLOB = attribInfo[i].value->GetFLOB(j);
      if (tmpFLOB->IsLob() == false) {
	tmpFLOB->Get(0, tmpFLOB->size, extensionPtr);
	extensionPtr = extensionPtr + tmpFLOB->size;
      }
    }
  }
  return result;
}

/*

5.7 MoveExternalAttributeToMemoryTuple

move external attribue values to memory tuple 

*/
void Tuple::MoveExternalAttributeToMemoryTuple() {
  int offset = 0;
  for (int i = 0; i < attrNum; i++) {
    memcpy(&memoryTuple[offset], attribInfo[i].value, attribInfo[i].size);
    attribInfo[i].changed = false;
    offset = offset + attribInfo[i].size;
  }
}


/*

5.8 SaveTo
  
saves a *fresh* tuple to file tuplefile. FLOBs which are larger than 
the thresholdsize are stored separately to file lobfile. 

*/
bool Tuple::SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile) {
  if (error == true) return false;
  
  lobFile = lobfile;
  recFile = tuplefile;
  extensionSize = CalcSizeOfFLOBData();
  
  
  extensionTuple = 0;
  // move FLOB data to extension tuple if exists.
  if (extensionSize > 0) {
    extensionTuple = MoveFLOBDataToExtensionTuple();
  }
  
  MoveExternalAttributeToMemoryTuple();
  
  /* Store memory tuple to disk tuple */
  TupleHeader th = {extensionSize};
  char *buf = (char *)malloc(sizeof(TupleHeader));
  
  memcpy(buf, &(th.size), sizeof(int));
  
  diskTuple.Truncate(0);
  bool rc = tuplefile->AppendRecord(diskTupleId, diskTuple);
  rc = diskTuple.Write(buf, sizeof(TupleHeader), 0) && rc;
  rc = diskTuple.Write(memoryTuple, memorySize, sizeof(TupleHeader)) && rc;

  if (extensionTuple != 0) 
  {
    rc = diskTuple.Write(extensionTuple, extensionSize,
      sizeof(TupleHeader) + memorySize) && rc;
  }

  state = SolidWrite;
  free(buf);
  
  return rc;
}

/*

5.9 Save

saves a *solid* tuple to its files.

*/
bool Tuple::Save() {
  if (error == true) return false;
	
  // to make fresh tuple persistent use SaveTo instead.
  if (state == Fresh) return false;
	
  extensionSize = CalcSizeOfFLOBData();

  extensionTuple = 0;
  // move FLOB data to extension tuple if exists.
  if (extensionSize > 0) {
    extensionTuple = MoveFLOBDataToExtensionTuple();
  }
  
  /* check wheather attributes has changed. */
  bool hasChanged = false;
  for (int i = 0; i < attrNum; i++) {
    if (attribInfo[i].changed) {
      hasChanged = true;
    }
  }
  
  if (hasChanged == true) {
    MoveExternalAttributeToMemoryTuple();
  }
  
  bool res = true;
  if (hasChanged == true) {
    /* Store memory tuple to disk tuple. */
    res = recFile->SelectRecord(diskTupleId, diskTuple, SmiFile::Update);
    
    /* Write the tuple header and proper data. */
    res = diskTuple.Write(memoryTuple, memorySize, sizeof(TupleHeader)) && res;
    if (extensionTuple != 0) {
      res = diskTuple.Write(extensionTuple, extensionSize, sizeof(TupleHeader) + memorySize) && res;
    }
  }
  return res;
}

/*

5.10 Destroy

destroys the persistent representation of a tuple.
This includes destroying all external database arrays referenced by 
that tuple record.


*/
bool Tuple::Destroy() {
  if (error == true) return false;
  bool rc = recFile->DeleteRecord(diskTupleId);
  diskTupleId = 0;
  state = Fresh;
  return rc;
}


/*

5.11 Put

The attribute ~attrno~ within the ~tuple~ is assigned
the value. The attribute number must be between 0 and attrNum-1.
This is a pointer assignment.
The ~value~ must be the address of a data type representation 
which agrees with the corresponding type field in the tuple type 
description (supplied at creation or opening of the tuple).
If this tuple will be destroyed the ~value~ must be destroyed
separately by the user. For an automatic deletion of ~value~ during
deletion of tuple see DelPut.
  
*/
bool Tuple::Put(int attrno, TupleElement *value) {
  if (error == true) return false;
  bool succ = false;
  if ((attrno < attrNum) && (attrno >= 0)) {
    // attrno is correct.
    if ((attribInfo[attrno].refCount == 0) && 
	(attribInfo[attrno].destruct == true)) {
      // the old value was put by DelPut before,
      // therefore delete this value.
      attribInfo[attrno].deleteValue();
    }
    attribInfo[attrno].value = value;
    attribInfo[attrno].incRefCount();
    attribInfo[attrno].destruct = false;
    attribInfo[attrno].prev = 0;
    attribInfo[attrno].next = 0;
    attribInfo[attrno].changed = true;
    succ = true;
  }  
  return succ; 
}

/*

5.12 DelPut

The attribute ~attrno~ within the ~tuple~ is assigned the value. 
The attribute must be between 0 and attrNum-1. 
This a pointer assignment. 
The ~value~ must be the address of a data type representation 
which agrees with the corresponding type field in the tuple type 
description (supplied at creation or opening of the tuple). 
The tuple is now assumed to own this data type value which means 
it will destroy that value 
calling a destroy operation of the data type -- 
when executing the ~Close~ operation. 
to be changed as the close operation is now the destructor.

*/
bool Tuple::DelPut(int attrno, TupleElement *value) {
  if (error == true) return false;
  bool succ = false;
  if ( (attrno < attrNum) && (attrno >= 0) ) {
    // attrno is correct.
    if ((attribInfo[attrno].refCount == 0) && 
	(attribInfo[attrno].destruct == true)) {
      // the old value was put by DelPut before,
      // therefore delete this value.
      attribInfo[attrno].deleteValue();
    }
    attribInfo[attrno].value = value;
    attribInfo[attrno].destruct = true;
    attribInfo[attrno].prev = 0;
    attribInfo[attrno].next = 0;
    attribInfo[attrno].changed = true;
    attribInfo[attrno].incRefCount();
    succ = true;
  } 
  return succ; 
}

/*

5.13 AttrPut

This is like ~Put~ with the only difference that the tuple does 
not own the datatype value, hence will not destroy it on closing. 
This can be used to assign attribute values belonging to other 
tuples or data structures.
  
*/
bool Tuple::AttrPut(int attrno_to, Tuple *tup, int attrno_from) {
  if (error == true) return false;
  bool succ = false;
  	if (
	    (attrno_to < attrNum) && 
	    (attrno_to >= 0) && 
	    (attrno_from < tup->attrNum) && 
	    (attrno_from >= 0)) {
	  // attribute number of this tuple and 
	  // the attribute number of Tuple *tup are correct.
	  if ((attribInfo[attrno_to].destruct == true) && 
	      (attribInfo[attrno_to].refCount == 0)) {
	    // the old value of this tuple was put by DelPut before,
	    // therefore delete this value
	    attribInfo[attrno_to].deleteValue();
	  }
	  attribInfo[attrno_to].value = tup->attribInfo[attrno_from].value;
	  attribInfo[attrno_to].destruct = true;
	  
	  AttributeInfo *ai = &(tup->attribInfo[attrno_from]);
	  
	  // insert this tuple attribute to the end of the 
	  // doubly connect list.
	  while (ai->next != 0) ai = ai->next;
	  ai->next = &attribInfo[attrno_to];
	  attribInfo[attrno_to].prev = ai;
	  attribInfo[attrno_to].next = 0;
	  
	  
	  attribInfo[attrno_to].changed = true;
	  attribInfo[attrno_to].incRefCount();
	  succ = true;
  	}
	
	return succ; 
}


/*
5.14 Get

returns the ith element of a tuple.

*/
TupleElement* Tuple::Get(int attrno) {
  return ((attrno >= 0 && attrno < attrNum && error == false) ? attribInfo[attrno].value : 0);
}

/*
5.15 destruct

destructs the ith element of a tuple.

*/

bool Tuple::destruct(int attrno) {
  return ((attrno >= 0 && attrno < attrNum) ? attribInfo[attrno].destruct : false);
}


/*
5.16 GetAttrNum

*/
int Tuple::GetAttrNum(){ 
  return attrNum;
}


/*
5.17 GetSize

*/
int Tuple::GetSize() {
  return memorySize; 
};

/*
5.18 Print

*/
ostream &Tuple::Print(ostream &os) {
  os << "(";
  
  if (error == true) {
    os << "ERROR";
  }
  else {
    for (int i = 0; i < attrNum; i++) {
      os << *attribInfo[i].value;
      if (i < attrNum - 1) os << ", ";
    }
  }
  return os << ")";
}

/*
5.19 operator<<

*/
ostream& operator<< (ostream &os, Tuple &tup) {
  return tup.Print(os);
}

/*
5.20 operator<<

*/
ostream &operator<< (ostream &os, TupleElement &attrib) {
  return attrib.Print(os);
}

