#include <iostream>
#include "Tuple.h"
#include "SecondoSystem.h"

/*
  //[ae] [\"a]
  //[ue] [\"u]
  //[oe] [\"o]

  1 Implementation of Module TupleManager

  Markus Spiekermann, 06/14/02 port to new secondo core System

  Stefan Dieker, 01/24/98

  \tableofcontents

*/

/* for debugging purposes. */
void dbg(char *message) {
	//cout << message << endl;
}

/* constructor. Sets all member variables, including size. */
TupleAttributes::TupleAttributes(int noAttrs, AttributeType *attrTypes) {
	dbg("### Tuple::TupleAttributes(int noAttrs, AttributeType *attrTypes) called.");
	totalNumber = noAttrs;
	type = attrTypes;
	totalSize = 0;
	for (int i = 0; i < noAttrs; i++) {
    	totalSize = totalSize + attrTypes[i].size;
	}
	dbg("### Tuple::TupleAttributes(int noAttrs, AttributeType *attrTypes) finished.");
}

/* initialisation of member variables */
void Tuple::Init(const TupleAttributes *attributes) {
	dbg("### Tuple::Init() called.");
	diskTupleId = 0;
	lobFile = 0;
	recFile = 0;
	lobFileOpened = false;
	attribInfo = 0;
	state = Fresh;
	attrNum = attributes->totalNumber;
	memorySize = attributes->totalSize;
	extensionSize = 0;
	
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
	
	dbg("### Tuple::Init() finished.");
}

/* creates a fresh tuple. */
Tuple::Tuple(const TupleAttributes *attributes) {
	dbg("### Tuple::Tuple(SmiRecordFile* recFile, const TupleAttributes *attributes) called.");
	
	// initialize member attributes.
  	Init(attributes);
	
	// this tuple is not persistent yet.	
	state = Fresh;
  
	dbg("### Tuple::Tuple(SmiRecordFile* recFile, const TupleAttributes *attributes) finished."); 	
}

/* Creates a solid tuple. Reads its data into memory. */
Tuple::Tuple(SmiRecordFile *recfile, const SmiRecordId rid, const TupleAttributes *attributes, SmiFile::AccessType mode) : diskTuple() {
	dbg("### Tuple::Tuple(SmiRecordFile *rfile, const SmiRecordId rid, const TupleAttributes *attributes, SmiFile::AccessType mode) called.");
	
	// initialize member attributes.
	Init(attributes);
	
  	diskTupleId = rid;
  	recFile = recfile;
	
	// reading tuple header from disk
	bool ok = recFile->SelectRecord(diskTupleId, diskTuple, mode);
	
	TupleHeader th = {0, 0};

	if (ok == true) {
		char *buf = (char *)malloc(sizeof(TupleHeader));
  		ok = diskTuple.Read(buf, sizeof(TupleHeader), 0);
		ok = diskTuple.Read(memoryTuple, memorySize, sizeof(TupleHeader)) && ok;
  		BufferToTupleHeader(buf, &th);
  		free(buf);
		
		if (ok) {
		
			// HIER IST EIN BUG BEI HERRN TELLE.
			/*
			lobFile = new SmiRecordFile(false);
			lobFileOpened = lobFile->Open(th.lobFileId);
			ok = lobFileOpened;
			*/
			// HIER IST EIN BUG BEI HERRN TELLE.
			
			lobFile = new SmiRecordFile(false);
			lobFileOpened = lobFile->Open("LOBFILE");
			
		}

		state = SolidRead;

		char *valuePtr = memoryTuple;
		for (int i = 0; i < attrNum; i++) {
			memcpy(attribInfo[i].value, valuePtr, attribInfo[i].size);
			valuePtr = valuePtr + attributes->type[i].size;
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
	
	dbg("### Tuple::Tuple(SmiRecordFile *rfile, const SmiRecordId rid, const TupleAttributes *attributes, SmiFile::AccessType mode) finished.");
}

/* previously called Close. */
Tuple::~Tuple() {
	dbg("### Tuple::~Tuple() called.");
	
	if (lobFileOpened == true) {
		lobFile->Close();
		lobFileOpened = false;
	}
	
  	if (memoryTuple != 0) {
     	free(memoryTuple);
  	}
  
  	// delete attributes
  	for (int i = 0; i < attrNum; i++) {
		attribInfo[i].decRefCount();
		if ((attribInfo[i].refCount == 0) && (attribInfo[i].destruct == true)) {
			attribInfo[i].deleteValue();
		}
		attribInfo[i].destruct = false;
  	}
	  
  	if (attribInfo != 0) {
    	delete[] attribInfo;
  	}

  	dbg("### Tuple::~Tuple() finished.");
}

/* Copies the content of a TupleHeader into a buffer. */
void Tuple::TupleHeaderToBuffer(TupleHeader *th, char *str) {
	dbg("### Tuple::TupleHeaderToBuffer(TupleHeader *th, char *str) called.");
	memcpy(str, &(th->size), sizeof(int));
	memcpy(str + sizeof(int), &(th->lobFileId), sizeof(SmiFileId));
	dbg("### Tuple::TupleHeaderToBuffer(TupleHeader *th, char *str) finished.");
}

/* Recovers the data of a TupleHeader from a buffer. */
void Tuple::BufferToTupleHeader(char *str, TupleHeader *th) {
	dbg("### Tuple::BufferToTupleHeader(char *str, TupleHeader *th) called.");
	memcpy(&(th->size), str, sizeof(int));
	memcpy(&(th->lobFileId), str + sizeof(int), sizeof(SmiFileId));
	dbg("### Tuple::BufferToTupleHeader(char *str, TupleHeader *th) finished.");
}

/* for debug purposes. */
void Tuple::printBuffer(char *buf, int len) {
	cout << "{";
	for (int i = 0; i < len; i++) {
		cout << (int)buf[i];
		if (i < len - 1) cout  << ", ";
	}
	cout << "}" << endl;
}

/* for debug purposes. */
void Tuple::printMemoryTuple() {
	cout << "{";
	for (int i = 0; i < memorySize; i++) {
		cout << (int)memoryTuple[i];
		if (i < memorySize - 1) cout  << ", ";
	}
	cout << "}" << endl;
}

/* for debug purposes. */
void Tuple::printTupleHeader(TupleHeader th) {
	cout << "{" << th.size << ", " << th.lobFileId << "}" << endl;
}

/*
1.5.3 SaveTo

*/
bool Tuple::SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile) {
  	dbg("### bool Tuple::SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile) called.");
  
  	lobFile = lobfile;
  	recFile = tuplefile;
	
	/* Determine the size of FLOB data stored in lobFile */ 
	FLOB *tmpFLOB;
	extensionSize = 0;
	for (int i = 0; i < attrNum; i++) {
		for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
			tmpFLOB = attribInfo[i].value->GetFLOB(j);
			bool stl = tmpFLOB->SaveToLob();
			if (stl == false) {
				extensionSize = extensionSize + tmpFLOB->Size();
			}
		}
	}

	extensionTuple = 0;
	// move FLOB data to extension tuple if exists.
	if (extensionSize > 0) {
		extensionTuple = (char *)malloc(extensionSize);
		char *extensionPtr = extensionTuple;
		for (int i = 0; i < attrNum; i++) {
			for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
				tmpFLOB = attribInfo[i].value->GetFLOB(j);
				if (tmpFLOB->IsLob() == false) {
					tmpFLOB->Get(0, tmpFLOB->size, extensionPtr);
					extensionPtr = extensionPtr + tmpFLOB->size;
				}
			}
		}
	}
	
  	/* move external attribue values to memory tuple */
  	int offset = 0;
  	for (int i = 0; i < attrNum; i++) {
      	memcpy(&memoryTuple[offset], attribInfo[i].value, attribInfo[i].size);
      	attribInfo[i].changed = false;
    	offset = offset + attribInfo[i].size;
  	}
	

  	/* Store memory tuple to disk tuple */
  	TupleHeader th = {extensionSize, lobFile->GetFileId()};
  	char *buf = (char *)malloc(sizeof(TupleHeader));
  	TupleHeaderToBuffer(&th, buf);
  	SmiRecord newTuple;
  	bool rc = tuplefile->AppendRecord(diskTupleId, newTuple);
  	rc = newTuple.Write(buf, sizeof(TupleHeader), 0) && rc;
  	rc = newTuple.Write(memoryTuple, memorySize, sizeof(TupleHeader)) && rc;
	if (extensionTuple != 0) {
		rc = newTuple.Write(extensionTuple, extensionSize, sizeof(TupleHeader) + memorySize) && rc;
	}	
  	diskTuple = newTuple;
  	state = SolidWrite;
	free(buf);
  
  	dbg("### bool Tuple::SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile) finished.");
  	return rc;
}

/*
1.5.4 Save

*/
bool Tuple::Save() {
	dbg("### bool Tuple::Save() called.");
	
	// to make fresh tuple persistent use SaveTo instead.
	if (state == Fresh) {
		return false;
	}
	
	/* Determine the size of FLOB data stored in lobFile */ 
	FLOB *tmpFLOB;
	extensionSize = 0;
	for (int i = 0; i < attrNum; i++) {
		for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
			tmpFLOB = attribInfo[i].value->GetFLOB(j);
			bool stl = tmpFLOB->SaveToLob();
			if (stl == true) {
				extensionSize = extensionSize + tmpFLOB->Size();
			}
		}
	}

	extensionTuple = 0;
	// move FLOB data to extension tuple if exists.
	if (extensionSize > 0) {
		extensionTuple = (char *)malloc(extensionSize);
		char *extensionPtr = extensionTuple;
		for (int i = 0; i < attrNum; i++) {
			for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
				tmpFLOB = attribInfo[i].value->GetFLOB(j);
				if (tmpFLOB->IsLob() == false) {
					tmpFLOB->Get(0, tmpFLOB->size, extensionPtr);
					extensionPtr = extensionPtr + tmpFLOB->size;
				}
			}
		}
	}

	
	/* check wheather attributes has changed. */
	bool hasChanged = false;
	for (int i = 0; i < attrNum; i++) {
		if (attribInfo[i].changed) {
			hasChanged = true;
		}
	}
	
  	if (hasChanged == true) {
		/* move external attribute values to memory tuple. */
		int offset = 0;
  		for (int i = 0; i < attrNum; i++) {
			memcpy(&(memoryTuple[offset]), attribInfo[i].value, attribInfo[i].size);
      		attribInfo[i].changed = false;
			offset = offset + attribInfo[i].size;
  		}
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

	dbg("### bool Tuple::Save() finished.");
	
	return res;											
}

/*
1.5.5 Destroy

*/
bool Tuple::Destroy() {
	dbg("### bool Tuple::Destroy() called.");
	bool rc = recFile->DeleteRecord(diskTupleId);
	diskTupleId = 0;
  	state = Fresh;
	dbg("### bool Tuple::Destroy() finished.");
	return rc;
}


/*
The tuple record ~diskTuple~ is destroyed. 
This includes destroying all external database arrays 
referenced by that tuple record.

1.5.6 Put

*/
bool Tuple::Put(int attrno, TupleElement *value) {
	dbg("### bool Tuple::Put(int attrno, TupleElement *value) called.");
	bool succ = false;
  	if ((attrno < attrNum) && (attrno >= 0)) {
		// attrno is correct.
		attribInfo[attrno].decRefCount();
		if ((attribInfo[attrno].refCount == 0) && 
			(attribInfo[attrno].destruct == true)) {
				// the old value was put by DelPut before,
				// therefore delete this value.
				attribInfo[attrno].deleteValue();
		}
    	attribInfo[attrno].value = value;
		attribInfo[attrno].incRefCount();
    	attribInfo[attrno].destruct = false;
    	attribInfo[attrno].parent = 0;
    	attribInfo[attrno].changed = true;
    	succ = true;
  	}
	
	dbg("### bool Tuple::Put(int attrno, TupleElement *value) finished.");

 	return succ; 
}

/*
The attribute ~attrno~ within the ~tuple~ is assigned the value. 
The attribute must be between 0 and attrNum-1. 
This a pointer assignment. 
The ~value~ must be the address of a data type representation 
which agrees with the corresponding type field in the tuple type 
description (supplied at creation or opening of the tuple). 
The tuple is now assumed to own this data type value which means 
it will destroy that value 
-- calling a destroy operation of the data type -- 
when executing the ~Close~ operation.  !!!! 
to be changed as the close operation is now the destructor !!!!

1.5.7 DelPut

*/

bool Tuple::DelPut(int attrno, TupleElement *value) {
	dbg("### bool Tuple::DelPut(int attrno, TupleElement *value) called.");
	bool succ = false;
  	if ( (attrno < attrNum) && (attrno >= 0) ) {
		// attrno is correct.
		attribInfo[attrno].decRefCount();
		if ((attribInfo[attrno].refCount == 0) && 
			(attribInfo[attrno].destruct == true)) {
				// the old value was put by DelPut before,
				// therefore delete this value.
				attribInfo[attrno].deleteValue();
		}
    	attribInfo[attrno].value = value;
    	attribInfo[attrno].destruct = true;
    	attribInfo[attrno].parent = 0;
    	attribInfo[attrno].changed = true;
		attribInfo[attrno].incRefCount();
    	succ = true;
  	}
	
	dbg("### bool Tuple::DelPut(int attrno, TupleElement *value) finished.");

 	return succ; 
}

/*
1.5.8 AttrPut

*/

bool Tuple::AttrPut(int attrno_to, Tuple *tup, int attrno_from) {
	dbg("### bool Tuple::AttrPut(int attrno_to, Tuple *tup, int attrno_from) called.");
  	bool succ = false;
  	if (
			(attrno_to < attrNum) && 
			(attrno_to >= 0) && 
			(attrno_from < tup->attrNum) && 
			(attrno_from >= 0)) {
		// attribute number of this tuple and the attribute number of Tuple *tup are correct.
		attribInfo[attrno_to].decRefCount();
		if ((attribInfo[attrno_to].destruct == true) && 
			(attribInfo[attrno_to].refCount == 0)) {
				// the old value of this tuple was put by DelPut before,
				// therefore delete this value
				attribInfo[attrno_to].deleteValue();
		
		}
    	attribInfo[attrno_to].value = tup->attribInfo[attrno_from].value;
    	attribInfo[attrno_to].destruct = true;
    	attribInfo[attrno_to].parent = &(tup->attribInfo[attrno_from]);
    	attribInfo[attrno_to].changed = true;
		attribInfo[attrno_to].incRefCount();
    	succ = true;
  	}
	
	dbg("### bool Tuple::AttrPut(int attrno_to, Tuple *tup, int attrno_from) finished.");

 	return succ; 
}


/*
1.5.8 Get

*/

TupleElement* Tuple::Get(int attrno) {
	return ((attrno >= 0 && attrno < attrNum) ? attribInfo[attrno].value : 0);
}

/*
1.5.9 destruct

*/

bool Tuple::destruct(int attrno) {
	return ((attrno >= 0 && attrno < attrNum) ? attribInfo[attrno].destruct : false);
}


/*
1.5.9 GetAttrNum

*/

int Tuple::GetAttrNum(){ 
	return attrNum;
}


/*
1.5.9 GetSize

*/

int Tuple::GetSize() {
	return memorySize; 
};


ostream &Tuple::Print(ostream &os) {
  	os << "(";
	
	for (int i = 0; i < attrNum; i++) {
		os << *attribInfo[i].value;
		if (i < attrNum - 1) os << ", ";
	}
	
  	return os << ")";
}

ostream& operator<< (ostream &os, Tuple &tup) {
  	return tup.Print(os);
}


ostream &operator<< (ostream &os, TupleElement &attrib) {
  	return attrib.Print(os);
}

