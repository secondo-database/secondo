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


\title {Implementation of module DBArray}
\author{Miguel Rodr[i]guez Luaces}
\date{April 15th, 1998}

\maketitle

[12] October 30th, Mirco G[ue]nster. Port to new Secondo SMI.

[12] April 15th, 1999. MRL. First Version.

[12] May 28th, 1999. MRL. Comments.

Printed on: \today

1 Overview

This file implements the database array concept.
A database array offers an open array of elements of
equal size called ~slots~. One can read valid slots with "Get"[1] and
write in any slot with "Put"[1]. The size of the slot changes
automatically if a "Put"[1] operation refers to a non existent
element.

2 Implementation

******************************************************************************/
#include "DBArray.h"

/* 

2.1 Contructor which takes an SmiRecordFile only, to be used by the tuple
manager. It cannot used in a program.

*/
DBArray::DBArray(SmiRecordFile *inlobFile) : FLOB(inlobFile) {
	// there is no valid element yet.
	mHigh = -1;
	mSlotSize = 0;
	mSize = 0;
	mSizeOfMetaInfo  = 3 * sizeof(int);			
}

/* 

2.2 Creates a DBArray with slots of size SlotSize. The initial number of slots 
allocated is SizeClue. 

*/ 
DBArray::DBArray(SmiRecordFile *inlobFile, int SlotSize, int SizeClue) : FLOB(inlobFile, SlotSize*SizeClue + 3*sizeof(int)) {
	// there is no valid element yet.					
	mHigh = -1;
	// size of each slot.
	mSlotSize = SlotSize;
	// initial size (number of slots)
	mSize = SizeClue;
	
	mSizeOfMetaInfo = 3 * sizeof(int);
}

/*

2.3 Destructor.

*/

DBArray::~DBArray() {
	// empty. Call destructor of the super class.
}

/* 

2.4 Destroys the handle to the DBArray, but not the persistent array. 

*/
void DBArray::Destroy() {
  FLOB::Destroy();
}

/* 	

2.5 Changes the size of the DBArray to NewSize. 
NewSize must be smaller than the actual size
of the DBArray. 

*/
void DBArray::Shrink(int NewSize) {
	if (NewSize < mHigh) {
		// call Resize-method of super class.
		Resize(NewSize * mSlotSize);
	}
}

/*
 
2.6 Copies the slot with index Index to the memory
starting at Dest. 

*/
void DBArray::Get(int Index, char *Dest) {
  if (Index >= 0) {
    FLOB::Get(Index*mSlotSize, mSlotSize, Dest);
  }
}

/* 

2.7 Copies from memory starting at Source into the slot with
index Index. If necessary the DBArray will grow to make
this slot available.  

*/
void DBArray::Put(int Index, char *Source) {
	if (Index >= 0) {
		if ((Index + 1) > mSize) {
			Resize(Index + 1);
		}
		if (Index > mHigh) {
			mHigh = Index;
		}
		Write(Index * mSlotSize, mSlotSize, Source);
	}
}

/* 

2.8 Returns the current highest valid index. 

*/
int DBArray::High() {
  return mHigh;
}

