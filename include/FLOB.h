/*
//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Header File of Module FLOB

Victor Almeida, 24/04/03. Adapting the class to accept standalone objects 
and objects inside tuples transparently.

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
class FLOB 
{

  public:
/*
3.1 Constructor.

Create a new FLOB from scratch.

*/
  	FLOB();
	
/*
3.2 Constructor.

Create a new FLOB from scratch and initialize it
with size ~sz~ if ~alloc~ is true. This ~alloc~
flag is important because sometimes the tuple
manager allocates the memory for the FLOB, specially
when the FLOB stays on the extension of the tuple.
In these situations, the FLOB does not allocate any
memory and leaves it to be done by the tuple.

*/
  	FLOB( const int sz, const bool alloc, const bool update);
/*

3.3 Constructor.

Opens a FLOB from a file ~inlobFile~ and record identification ~id~.
The flag ~update~ tells if the FLOB is being opened for update or 
read only.

*/
  	FLOB( SmiRecordFile* inlobFile, const SmiRecordId id, const bool update);

/*
3.3 Destructor. 

Deletes the FLOB instance.

*/
  	~FLOB();
	
/*

3.4 Destroy.

Destroy the persistent representation of the FLOB.

*/
  	void Destroy();
	
/*
3.5 Get

Read by copying

*/
  	void Get(const int offset, const int length, char *target);
	
/* 
3.6	Write

Write Flob data into source. 

*/
  	void Write(const int offset, const int length, char *source);
	
/* 
3.7 Size

Returns the size of a FLOB.

*/
   	const int GetSize() const;
  
  
/*
3.8 Resize

Resizes the FLOB.

*/
  	void Resize(const int size);
  
  
/*
3.9 Restore

Restore from byte string.

*/
   	const int Restore(char *address);
	
/*
3.10 IsLob

Returns treue, if value stored in underlying LOB, otherwise false.

*/
  	const bool IsLob() const;
	
/* 
3.11 SaveToLob

Switch from Main Memory to LOB Representation 
if the size of this FLOB exceeds threshold size.

*/
  	const bool SaveToLob( SmiRecordFile *lobFile );
/* 
3.12 GetLobId 

Returns the lob record id.

*/
  	const SmiRecordId GetLobId() const;

/*
3.13 SetInsideTuple

Sets this flob to be inside a tuple.

*/
        void SetInsideTuple();

  protected:

    static const int SWITCH_THRESHOLD;
/* 
This is the tresholdsize for a FLOB.  Whenever the size of this FLOB exceeds 
the thresholdsize the data will stored in a separate file for lobs.

*/
  
  	SmiRecordFile* lobFile;
/* 
In this file the FLOB object will store the data if the FLOB is large. 

*/
  	SmiRecord lob;
/*
In this record the FLOB object will store the data if the FLOB is large. 

*/
  	SmiRecordId lobId;
/* 
This is the number of the record in which the FLOB data is stored if the 
FLOB is large. 

*/
  
  	int size;
/* 
This is the size of memory allocated by start. 

*/
	
	  char *start;
/* 
This is a pointer to the memory where the data of the FLOB is stored 
if the FLOB is small. 

*/

    bool isLob;
/* 
This is a flag that tells if the FLOB is a LOB or is stored in memory 

*/

    bool insideTuple;
/* 
This is a flag that tells if the FLOB is used inside a tuple of 
separetely in a Secondo object 

*/
    bool freeStart;
/*
The ~start~ pointer is allocated sometimes by the FLOB class and sometimes
by the Tuple class in the tuple manager. If it is allocated by the FLOB
class this flag is set and the ~start~ pointer is freed in the destructor,
otherwise, nothing is done and the deallocation is done by the tuple
manager itself.

*/
};

#endif
