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
//@ GETESTET
void dbg(char *message) {
	//cout << message << endl;
}

/* for debugging purposes. */
//@ GETESTET
void dbg2(int n) {
	cout << "\n\n\n\t\t\t(" << n << ")\n\n\n" << endl;
}

//@ GETETESTET
TupleAttributes::TupleAttributes(int noAttrs, AttributeType *attrTypes) {
	totalNumber = noAttrs;
	type = attrTypes;
	totalSize = 0;
	for (int i = 0; i < noAttrs; i++) {
    	totalSize = totalSize + attrTypes[i].size;
	}
}

/* initialisation of member variables */
//@ GETESTET
void Tuple::Init() {
	dbg("### Tuple::Init() called.");
	diskTupleId = 0;
	lobFile = 0;
	recFile = 0;
	attrNum = 0;
	attribInfo = 0;
	algM = 0;
	state = Fresh;
	coreTuple = 0;
	coreSize = 0;
	totalSize = 0;
	memoryTuple = 0;
	refCount = 0;
	dbg("### Tuple::Init() finished.");
}

/* Creates a fresh tuple. */
//@ GETESTET
Tuple::Tuple(const TupleAttributes *attributes) {
	dbg("### Tuple::Tuple(SmiRecordFile* recFile, const TupleAttributes *attributes) called.");
  	Init();
  	attrNum = attributes->totalNumber;
  	coreSize = attributes->totalSize;
  	totalSize = coreSize;
  
  	// get Reference of AlgebraManager  
  	algM = SecondoSystem::GetAlgebraManager();

  	// allocate memory for tuple representation
 	memoryTuple = (char *)malloc(attributes->totalSize);
	
	// copy attribute info
  	attribInfo = new AttributeInfo[attrNum];
  	char *valuePtr = memoryTuple;
  	for (int i = 0; i < attrNum; i++) {
		int actSize = attributes->type[i].size;
    	int algId = attributes->type[i].algId;
    	int typeId = attributes->type[i].typeId;
    	attribInfo[i].size = actSize;
    	// call cast function of tuple attribute
    	attribInfo[i].value = (TupleElement*) (*(algM->Cast(algId, typeId)))(valuePtr);
    	valuePtr = valuePtr + actSize;
  	} 
	state = Fresh;
  
	dbg("### Tuple::Tuple(SmiRecordFile* recFile, const TupleAttributes *attributes) finished."); 	
}

/* Creates a solid tuple. Reads its data into memory. */
//@ GETESTET
Tuple::Tuple(SmiRecordFile *recfile, const SmiRecordId rid, const TupleAttributes *attributes, SmiFile::AccessType mode) : diskTuple() {
	dbg("### Tuple::Tuple(SmiRecordFile *rfile, const SmiRecordId rid, const TupleAttributes *attributes, SmiFile::AccessType mode) called.");
	Init();
	attrNum = attributes->totalNumber;
  	coreSize = attributes->totalSize;
  	totalSize = coreSize;
  
  	// get Reference of AlgebraManager  
  	algM = SecondoSystem::GetAlgebraManager();

	// allocate memory for tuple representation
 	memoryTuple = (char *)malloc(attributes->totalSize);

	// copy attribute info
  	attribInfo = new AttributeInfo[attrNum];
  	char *valuePtr = memoryTuple;
  	for (int i = 0; i < attrNum; i++) {
		int actSize = attributes->type[i].size;
    	int algId = attributes->type[i].algId;
    	int typeId = attributes->type[i].typeId;
    	attribInfo[i].size = actSize;
    	// call cast function of tuple attribute
    	attribInfo[i].value = (TupleElement*) (*(algM->Cast(algId, typeId)))(valuePtr);
    	valuePtr = valuePtr + actSize;
	}
	
  	diskTupleId = rid;
  	recFile = recfile;
	
	// reading tuple header from disk
	recFile->SelectRecord(diskTupleId, diskTuple, SmiFile::ReadOnly);

	char *buf = (char *)malloc(sizeof(TupleHeader));
  	diskTuple.Read(buf, sizeof(TupleHeader), 0);
	diskTuple.Read(memoryTuple, totalSize, sizeof(TupleHeader));
	
	TupleHeader th = {0, 0};
  	BufferToTupleHeader(buf, &th);
  	free(buf);
	
	state = SolidRead;

	//lobFile = new SmiRecordFile(false);
	//lobFile->Open(th.lobFileId);
	
	// DER NACHFOLGENDE AUSKOMMENTIERTE TEIL IST FUER DIE BEHANDLUNG 
	// VON FLOBS VORGESHEN.	
	/*
	lobFile = th.rfile;
  
  	// reading main part of tuple
  	memoryTuple = (char*) malloc (totalSize);
  	diskTuple.Read((void *)memoryTuple, totalSize, sizeof(TupleHeader));
   
  	char *valuePtr = memoryTuple;
  	char *corePointer = memoryTuple;
  	char *extensionPointer = memoryTuple + coreSize;
  	attrNum = attributes->totalNumber;
  	attribInfo = new AttributeInfo[attrNum];
	
  	FLOB *tmpFLOB;
  
  	for (int i = 0; i < attrNum; i++) {
  		int actSize = attributes->type[i].size;
		int algId = attributes->type[i].algId;
		int typeId = attributes->type[i].typeId;
	
  		attribInfo[i].size = attributes->type[i].size;
	
		// call cast function of tuple attribute
		attribInfo[i].value = (TupleElement *)(*(algM->Cast(algId, typeId)))(valuePtr);
		valuePtr = valuePtr + actSize;
	
    	for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
      	tmpFLOB = attribInfo[i].value->GetFLOB(j);
      	if (!tmpFLOB->IsLob())
			extensionPointer = extensionPointer + tmpFLOB->Restore(extensionPointer);
    	}
    	corePointer = corePointer + attribInfo[i].size;
  	}
  	*/
	    
	dbg("### Tuple::Tuple(SmiRecordFile *rfile, const SmiRecordId rid, const TupleAttributes *attributes, SmiFile::AccessType mode) finished.");
}

/* previously called Close. */
//@ GETESTET
Tuple::~Tuple() {
	dbg("### Tuple::~Tuple() called.");
	
	// HIER MUESSEN GGF. SPAETER NOCH GEOEFFNETE LOBFILES GESCHLOSSEN WERDEN.

	/*
  	if (memoryTuple != 0) {
     	free(memoryTuple);
  	}
  
  	// delete attributes
  	for (int i = 0; i < attrNum; i++) {
	    Unput(i);
  	}
	  
  	if (attribInfo != 0) {
    	delete[] attribInfo;
  	}
	*/
  	dbg("### Tuple::~Tuple() finished.");
}

/* Copies the content of a TupleHeader into a buffer. */
//@ GETESTET
void Tuple::TupleHeaderToBuffer(TupleHeader *th, char *str) {
	dbg("### Tuple::TupleHeaderToBuffer(TupleHeader *th, char *str) called.");
	memcpy(str, &(th->size), sizeof(int));
	memcpy(str + sizeof(int), &(th->lobFileId), sizeof(SmiFileId));
	dbg("### Tuple::TupleHeaderToBuffer(TupleHeader *th, char *str) finished.");
}

/* Recovers the data of a TupleHeader from a buffer. */
//@ GETESTET
void Tuple::BufferToTupleHeader(char *str, TupleHeader *th) {
	dbg("### Tuple::BufferToTupleHeader(char *str, TupleHeader *th) called.");
	memcpy(&(th->size), str, sizeof(int));
	memcpy(&(th->lobFileId), str + sizeof(int), sizeof(SmiFileId));
	dbg("### Tuple::BufferToTupleHeader(char *str, TupleHeader *th) finished.");
}

/* for debug purposes. */
//@ GETESTET
void Tuple::printBuffer(char *buf, int len) {
	cout << "{";
	for (int i = 0; i < len; i++) {
		cout << (int)buf[i];
		if (i < len - 1) cout  << ", ";
	}
	cout << "}" << endl;
}

/* for debug purposes. */
//@ GETESTET
void Tuple::printMemoryTuple() {
	cout << "{";
	for (int i = 0; i < totalSize; i++) {
		cout << (int)memoryTuple[i];
		if (i < totalSize - 1) cout  << ", ";
	}
	cout << "}" << endl;
}

/* for debug purposes. */
//@ GETESTET
void Tuple::printTupleHeader(TupleHeader th) {
	cout << "{" << th.size << ", " << th.lobFileId << "}" << endl;
}

/*
1.5.3 SaveTo

*/
//@ GETESTET
bool Tuple::SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile) {
  	dbg("### bool Tuple::SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile) called.");
  
  	lobFile = lobfile;
  	recFile = tuplefile;
 	
	free(memoryTuple);
	memoryTuple = (char *)malloc(totalSize);
	
  	/* move external attribue values to memory tuple */
  	int offset = 0;
  	for (int i = 0; i < attrNum; i++) {
      	memcpy(&memoryTuple[offset], attribInfo[i].value, attribInfo[i].size);
      	attribInfo[i].changed = false;
    	offset = offset + attribInfo[i].size;
  	}
	
  
  	// DER NACHFOLGENDE AUSKOMMENTIERTE TEIL IST FUER DIE BEHANDLUNG 
	// VON FLOBS VORGESHEN.
	/*
  	// Determine size of extension tuple 
  	int extensionSize = 0;
  	FLOB *tmpFLOB;
  	for (int i = 0; i < attrNum; i++) {
     	for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
     		cout << "(6)" << endl;
        	tmpFLOB = attribInfo[i].value->GetFLOB(j);
      		if (!(tmpFLOB->SaveToLob())) {
      			cout << "(7)" << endl;
				extensionSize += tmpFLOB->size;
      		}
    	}
  	}
	*/
	
  	// DER NACHFOLGENDE AUSKOMMENTIERTE TEIL IST FUER DIE BEHANDLUNG 
	// VON FLOBS VORGESHEN.
	/*
  	// Realloc memory tuple 
  	if (totalSize < coreSize + extensionSize) {
  		cout << "(8)" << endl;
    	char *oldTuple = memoryTuple;
    	memoryTuple = (char*)realloc(memoryTuple, coreSize + extensionSize);
    	int diff = (memoryTuple - oldTuple) / sizeof(TupleElement *);
    	if (diff != 0) {
    		cout << "(9)" << endl;
      		for (int i = 0; i < attrNum; i++) {
      			cout << "(10)" << endl;
				attribInfo[i].value += diff;
      		}
    	}
    	totalSize = coreSize + extensionSize;
  	}
	*/
	
	// DER NACHFOLGENDE AUSKOMMENTIERTE TEIL IST FUER DIE BEHANDLUNG 
	// VON FLOBS VORGESHEN.
	/*
  	// Assemble extension tuple 
  	char *extensionPointer = memoryTuple + coreSize;
  	for (int i = 0; i < attrNum; i++) {
    	for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
    		cout << "(13)" << endl;
      		tmpFLOB = attribInfo[i].value->GetFLOB(j);
      		if (!(tmpFLOB->IsLob())) {
      			cout << "(14)" << endl;
				tmpFLOB->Get(0, tmpFLOB->size, extensionPointer);
				extensionPointer += tmpFLOB->size;
      		}
    	}
  	}
	*/
  
  	/* Store memory tuple to disk tuple */
  	TupleHeader th = {totalSize, lobFile->GetFileId()};
  	char *buf = (char *)malloc(sizeof(TupleHeader));
  	TupleHeaderToBuffer(&th, buf);
  	SmiRecord newTuple;
  	bool rc = tuplefile->AppendRecord(diskTupleId, newTuple);
  	int rc1 = newTuple.Write(buf, sizeof(TupleHeader), 0);
  	int rc2 = newTuple.Write(memoryTuple, totalSize, sizeof(TupleHeader));
 	if (rc1 != sizeof(TupleHeader)) rc = false;
	if (rc2 != totalSize) rc = false;
 
  	diskTuple = newTuple;
  	state = SolidWrite;
	free(buf);
  
  	dbg("### bool Tuple::SaveTo(SmiRecordFile *tuplefile, SmiRecordFile *lobfile) finished.");
  	return rc;
}

/*
1.5.4 Save

*/
//@ GETESTET
bool Tuple::Save() {
	dbg("### bool Tuple::Save() called.");
	
	if (state == Fresh) {
		return false;
	}
	
	/* check wheather attributes has changed. */
	bool hasChanged = false;
	for (int i = 0; i < attrNum; i++) {
		if (attribInfo[i].changed) {
			hasChanged = true;
		}
	}
	
	/* move external attribue values to memory tuple */
  	if (hasChanged) {
		free(memoryTuple);
		memoryTuple = (char *)malloc(totalSize);
		
		int offset = 0;
  		for (int i = 0; i < attrNum; i++) {
			memcpy(&(memoryTuple[offset]), attribInfo[i].value, attribInfo[i].size);
      		attribInfo[i].changed = false;
			offset = offset + attribInfo[i].size;
  		}
	}
  
  	// DER NACHFOLGENDE AUSKOMMENTIERTE TEIL IST FUER DIE BEHANDLUNG 
	// VON FLOBS VORGESHEN.
	/*
  	// Determine size of extension tuple 
  	int extensionSize = 0;
  	FLOB *tmpFLOB;
  	for (int i = 0; i < attrNum; i++) {
     	for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
     		cout << "(6)" << endl;
        	tmpFLOB = attribInfo[i].value->GetFLOB(j);
      		if (!(tmpFLOB->SaveToLob())) {
      			cout << "(7)" << endl;
				extensionSize += tmpFLOB->size;
      		}
    	}
  	}
	*/
  	// DER NACHFOLGENDE AUSKOMMENTIERTE TEIL IST FUER DIE BEHANDLUNG 
	// VON FLOBS VORGESHEN.
	/*
  	// Realloc memory tuple 
  	if (totalSize < coreSize + extensionSize) {
  		cout << "(8)" << endl;
    	char *oldTuple = memoryTuple;
    	memoryTuple = (char*)realloc(memoryTuple, coreSize + extensionSize);
    	int diff = (memoryTuple - oldTuple) / sizeof(TupleElement *);
    	if (diff != 0) {
    		cout << "(9)" << endl;
      		for (int i = 0; i < attrNum; i++) {
      			cout << "(10)" << endl;
				attribInfo[i].value += diff;
      		}
    	}
    	totalSize = coreSize + extensionSize;
  	}
	*/
	
	// DER NACHFOLGENDE AUSKOMMENTIERTE TEIL IST FUER DIE BEHANDLUNG 
	// VON FLOBS VORGESHEN.
	/*
  	// Assemble extension tuple 
  	char *extensionPointer = memoryTuple + coreSize;
  	for (int i = 0; i < attrNum; i++) {
    	for (int j = 0; j < attribInfo[i].value->NumOfFLOBs(); j++) {
    		cout << "(13)" << endl;
      		tmpFLOB = attribInfo[i].value->GetFLOB(j);
      		if (!(tmpFLOB->IsLob())) {
      			cout << "(14)" << endl;
				tmpFLOB->Get(0, tmpFLOB->size, extensionPointer);
				extensionPointer += tmpFLOB->size;
      		}
    	}
  	}
	*/			
		
	// HIER LIEGT DER HUND BEGRABEN!!!!!!!!!!!!!!!!!!!!!!!!!!
	// wahrscheinlich kann auf lobFile->GetFileId nicht zugegriffen werden!!!
	
	bool res = true;
	if (hasChanged) {
		/* Store memory tuple to disk tuple. */
		//TupleHeader th = {totalSize, lobFile->GetFileId()};	
		TupleHeader th = {totalSize, 0};

		char *buf = (char *)malloc(sizeof(TupleHeader));

		TupleHeaderToBuffer(&th, buf);
	
		res = recFile->SelectRecord(diskTupleId, diskTuple, SmiFile::Update);

		/* Write the tuple header and proper data. */
		int rc1 = diskTuple.Write(buf, sizeof(TupleHeader), 0);
		int rc2 = diskTuple.Write(memoryTuple, totalSize, sizeof(TupleHeader));

		if (rc1 != sizeof(TupleHeader)) res = false;
		if (rc2 != totalSize) res = false;
		free(buf);
	}

	dbg("### bool Tuple::Save() finished.");
	
	return res;											
}

/*
1.5.5 Destroy

*/
//@ GESTESTET 
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
//@ GETESTET
bool Tuple::Put(int attrno, TupleElement *value) {
	dbg("### bool Tuple::Put(int attrno, TupleElement *value) called.");
	bool succ = false;
  	if ( (attrno < attrNum) && (attrno >= 0) ) {
    	attribInfo[attrno].value = value;
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
	return false;
}

/*
1.5.8 AttrPut

*/

bool Tuple::AttrPut(int attrno_to, Tuple *tup, int attrno_from) {
  	return false;
}


/*
1.5.8 Get

*/
//@ GETESTET
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
//@ GETESTET
int Tuple::GetAttrNum(){ 
	return attrNum;
}


/*
1.5.9 GetSize

*/
//@ GETESTET
int Tuple::GetSize() {
	return totalSize; 
};

//@ GETESTET
ostream &Tuple::Print(ostream &os) {
  	os << "(";
	
	for (int i = 0; i < attrNum; i++) {
		os << (*attribInfo[i].value);
		if (i < attrNum - 1) os << ", ";
	}
	
  	return os << ")";
}

/*
Get a pointer to the attribute value with number ~attrno~. The ~attrno~ must be between 1 and k and the attribute must have assigned a value earlier.

*/
//@ GETESTET
ostream& operator<< (ostream &os, Tuple &tup) {
  	return tup.Print(os);
}

//@ GETESTET
ostream &operator<< (ostream &os, TupleElement &attrib) {
  	return attrib.Print(os);
}

