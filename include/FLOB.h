/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Header File of Module FLOB

Mirco G[ue]nster, 31/09/02 End of porting to the new SecondoSMI.

Markus Spiekermann, 14/05/02. Begin of porting to the new SecondoSMI.

Stefan Dieker, 03/05/98

1 Defines

*/

#ifndef FLOB_H
#define FLOB_H

/*

2 Includes

*/
#include "SecondoSMI.h"

/*
3 class FLOB

This class implements a FLOB which define a new 
large object abstraction that offers most access
methods of the original one.
It defines a threshold size namely SWITCH\_THRESHOLD 
as a static attribute. This value defines a limit
between "small" and "large" values referring its
size.
A small FLOB is stored in a main memory structure
whereas a large FLOB is stored into a seperate 
SmiRecord in an SmiFile. 

*/
class FLOB {
protected:
	/* This is the tresholdsize for a FLOB. 
		Whenever the size of this FLOB exceeds
		the thresholdsize the data will stored
		in a separate file for lobs.
	*/
  	static const int SWITCH_THRESHOLD;
  
  	/* In this file the FLOB object will store the data
		if the FLOB is large. */
  	SmiRecordFile* lobFile;
	/* In this record the FLOB object will store the data
		if the FLOB is large. */
  	SmiRecord lob;
	/* This is the number of the record in which the FLOB data
		is stored if the FLOB is large. */
  	SmiRecordId lobId;
  
  	/* This is the size of memory allocated by start. */
  	int size;
	
	/* This is a pointer to the memory where the data
		of the FLOB is stored if the FLOB is small. */
	char *start;

        /* This is a flag that tells if the FLOB is a LOB or  
                is stored in memory */
        bool isLob;

        /* This is a flag that tells if the FLOB is used inside
                a tuple of separetely in a Secondo object */
        bool insideTuple;

public:
/*

3.1 Constructor.

*/
  	FLOB(SmiRecordFile* inlobFile);
	
/*

3.2 Constructor.

Create from scratch.

*/
  	FLOB(SmiRecordFile* inlobFile, int sz, const bool alloc, const bool update);
/*

3.3 Constructor.

Opens a FLOB.

*/
  	FLOB(SmiRecordFile* inlobFile, const SmiRecordId id, const bool update);

/*
3.3 Destructor. 

Destroy LOB instance.

*/
  	~FLOB();
	
/*

3.4 Destroy.

Destroy persistent representation

*/
  	void Destroy();
	
/*
3.5 Get

Read by copying

*/
  	void Get(int offset, int length, char *target);
	
/* 
3.6	Write

Write Flob data into source. 

*/
  	void Write(int offset, int length, char *source);
	
/* 
3.7 Size

Returns the size of a FLOB.

*/
   	int GetSize();
  
  
/*
3.8 Resize

Resizes the FLOB.

*/
  	void Resize(int size);
  
  
/*
3.9 Restore

Restore from byte string.

*/
   	int Restore(char *address);
	
/*
3.10 IsLob

Returns treue, if value stored in underlying LOB, otherwise false.

*/
  	bool IsLob() const;
	
/* 
3.11 SaveToLob

Switch from Main Memory to LOB Representation 
if the size of this FLOB exceeds threshold size.

*/
  	bool SaveToLob();
/* 
3.12 GetLobId 

Returns the lob record id.

*/
  	const SmiRecordId GetLobId();

/*
3.13 SetInsideTuple

Sets this flob to be inside a tuple or not depending on the value of ~it~.

*/
        void SetInsideTuple( const bool it = true );
};
#endif
