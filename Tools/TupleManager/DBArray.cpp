/******************************************************************************
//paragraph	[10]	title:		[\centerline{\Large \bf] [}]
//paragraph	[11]	TTp:		[\begin{quote} {TODO: ] [} \end{quote}]
//paragraph	[12]	Serif:		[{\sf ] [}]
//characters	[1]	tt:		[{\tt ] [}]
//characters	[2]	Type:		[\underline{\em ] [}]
//characters	[3]	ProperName:	[{\sc ]	[}]
//[i]	[\'{\i}]
//[n]	[\~{n}]

\title{DBArray.cc}
\author{Miguel Rodr[i]guez Luaces}
\date{April 15th, 1998}

\maketitle

[12] October 30th, Mirco G[ue]nster. Port to new Secondo SMI.

[12] April 15th, 1999. MRL. First Version.

[12] May 28th, 1999. MRL. Comments.

Printed on: \today

\tableofcontents

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


DBArray::DBArray(SmiRecordFile *inlobFile) : FLOB(inlobFile) {
	mHigh = -1;
	mSlotSize = 0;
	mSize = 0;
	mSizeOfMetaInfo  = 0;			
}

DBArray::DBArray(SmiRecordFile *inlobFile, int SlotSize, int SizeClue) : 
						FLOB(inlobFile, SlotSize*SizeClue + 3*sizeof(int)) {
						
	mHigh = -1; 			// there is no valid element yet.
	mSlotSize = SlotSize; 	// size of each slot.
	mSize = SizeClue;		// initial size (number of slots)
	mSizeOfMetaInfo = 3 * sizeof(int);
}

DBArray::~DBArray() {
}

void DBArray::Destroy() {
  FLOB::Destroy();
}


void DBArray::Shrink(int NewSize) {
	if (NewSize < mHigh) {
		Resize(NewSize);
	}
}


void DBArray::Get(int Index, char *Dest) {
  if (Index >= 0) {
    FLOB::Get(Index*mSlotSize, mSlotSize, Dest);
  }
}


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

int DBArray::High() {
  return mHigh;
}

