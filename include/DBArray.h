/******************************************************************************
//paragraph	[10]	title:		[\centerline{\Large \bf] [}]
//paragraph	[11]	TTp:		[\begin{quote} {TODO: ] [} \end{quote}]
//paragraph	[12]	Serif:		[{\sf ] [}]
//characters	[1]	tt:		[{\tt ] [}]
//characters	[2]	Type:		[\underline{\em ] [}]
//characters	[3]	ProperName:	[{\sc ]	[}]
//[i]	[\'{\i}]
//[n]	[\~{n}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]


\title{DBArray.h}
\author{Miguel Rodr[i]guez Luaces}
\date{April 15th, 1998}

\maketitle

[12] October 30th, 2001 Mirco G[ue]nster. Port to the new Secondo SMI.

[12] April 15th, 1999. MRL. First Version.

[12] May 28th, 1999. MRL. Comments.

Printed on: \today

1 Overview

This file declares the database array interface
A database array offers an open array of elements of
equal size called ~slots~. One can read valid slots with "Get"[1] and
write in any slot with "Put"[1]. The size of the database array changes
automatically if a "Put"[1] operation refers to a non existent
element.

2 Implementation

******************************************************************************/
#ifndef DBARRAY_H
#define DBARRAY_H

#include "FLOB.h"


/* 

A database array is implemented in top of Secondo FLOBs, so a database 
array is a FLOB itself and it can be used as a FLOB. 

*/

class DBArray : public FLOB {
	private:
		// Slot number of highest valid element.
  		int mHigh;
		// Size of each slot.
  		int mSlotSize;
		// Size of the DBArray (number of slots allocated).
  		int mSize;
		// Size of the DB information (header of the DBArray).
  		int mSizeOfMetaInfo;
		
	public:

/* 

2.1 Contructor which takes an SmiRecordFile only, to be used by the tuple
manager. It cannot used in a program.

*/
  		DBArray(SmiRecordFile *inlobFile);
		
/* 

2.2 Creates a dbarray with slots of size SlotSize. The initial number of slots 
allocated is SizeClue. 

*/ 
  		DBArray(SmiRecordFile *inlobFile, int SlotSize, 
				int SizeClue = 20);
		
/* 

2.3 Destroys the handle to the DBArray, but not the persistent array. 

*/
  		~DBArray();
  
/* 

2.4 Destroys the persistent array. 

*/
  		void Destroy();
		
/* 	

2.5 Changes the size of the DBArray to NewSize. 
NewSize must be smaller than the actual size
of the DBArray. 

*/
  		void Shrink(int NewSize);
		
		
/*
 
2.6 Copies the slot with index Index to the memory
starting at Dest. 

*/
  		void Get(int Index, char *Dest);
		
/* 

2.7 Copies from memory starting at Source into the slot with
index Index. If necessary the DBArray will grow to make
this slot available.  

*/
  		void Put(int Index, char *Source);
	
/* 

2.8 Returns the current highest valid index. 

*/
		  		int High();
};

#endif
