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

/******************************************************************************
*Constructors:*

******************************************************************************/
DBArray::DBArray() : FLOB() {
}

DBArray::DBArray(int SlotSize, int SizeClue) : FLOB(SlotSize*SizeClue + 3*sizeof(int)) {
  int High;
  int Size;
  size_t SizeOfMetaInfo;

  Size = SizeClue;
  High = -1;
  SizeOfMetaInfo = 3 * sizeof(int);

  WriteMetaInfo(SlotSize, Size, High);
}

/******************************************************************************
 *Destructor:*

*******************************************************************************/
DBArray::~DBArray() {
}

/******************************************************************************
 *WriteMetaInfo:* Writes the information about the DBArray to the FLOB.

******************************************************************************/
void DBArray::WriteMetaInfo(int SlotSize, int Size, int High) {
  size_t sizeofint;

  sizeofint = sizeof(int);
  Write(0*sizeofint, sizeofint, (char *)&SlotSize);
  Write(1*sizeofint, sizeofint, (char *)&Size);
  Write(2*sizeofint, sizeofint, (char *)&High);  
}

/******************************************************************************
 *ReadMetaInfo:* Reads from the FLOB the information about the DBArray.

******************************************************************************/
void DBArray::ReadMetaInfo(int *SlotSize, int *Size, int *High) {
  size_t sizeofint;

  sizeofint = sizeof(int);
  FLOB::Get(0*sizeofint, sizeofint, (char *)SlotSize);
  FLOB::Get(1*sizeofint, sizeofint, (char *)Size);
  FLOB::Get(2*sizeofint, sizeofint, (char *)High);
}

/******************************************************************************
 *Destroy:*

******************************************************************************/
void DBArray::Destroy() {

  FLOB::Destroy();
}

/******************************************************************************
 *Shrink:*

******************************************************************************/
void DBArray::Shrink(int NewSize) {
  int High;
  int SlotSize;
  int Size;
  size_t SizeOfMetaInfo = 3 * sizeof(int);

  ReadMetaInfo(&SlotSize, &Size, &High);
  if ((NewSize < High) && (NewSize > 0)) {
    High = (NewSize-1);
    Size = NewSize;
    Resize(Size*SlotSize + SizeOfMetaInfo);
    WriteMetaInfo(SlotSize, Size, High);
  }
}

/******************************************************************************
 *Get:*

******************************************************************************/
void DBArray::Get(int Index, char *Dest) {
  int High;
  int SlotSize;
  int Size;
  size_t SizeOfMetaInfo = 3 * sizeof(int);

  ReadMetaInfo(&SlotSize, &Size, &High);
  if (Index >= 0) {
    FLOB::Get(SizeOfMetaInfo + Index*SlotSize, SlotSize, Dest);
  }
}

/******************************************************************************
 *Put:*

******************************************************************************/
void DBArray::Put(int Index, char *Source) {
  int High;
  int SlotSize;
  int Size;
  size_t SizeOfMetaInfo = 3 * sizeof(int);

  ReadMetaInfo(&SlotSize, &Size, &High);
  if (Index >= 0) {
    if ((Index+1) > Size) {
      Size = (Index+1);
      Resize(Size*SlotSize + SizeOfMetaInfo);
    }
    if (Index > High) {
      High = Index;
    }
    WriteMetaInfo(SlotSize, Size, High);
    Write(SizeOfMetaInfo + Index*SlotSize, SlotSize, Source);
  }
}

/******************************************************************************
 *High:*

******************************************************************************/
int DBArray::High() {
  int High;
  int SlotSize;
  int Size;
  size_t SizeOfMetaInfo = 3 * sizeof(int);

  ReadMetaInfo(&SlotSize, &Size, &High);
  return High;
}

/******************************************************************************
The following functions are not implemented.

Well, they are implemented but I do not like how. Do not use them yet.

*Select:*

******************************************************************************/
char *DBArray::Select(int Index) {
  int ActualLenght;
  int High;
  int SlotSize;
  int Size;
  size_t SizeOfMetaInfo = 3 * sizeof(int);

  ReadMetaInfo(&SlotSize, &Size, &High);
  if ((Index >= 0) && (Index <= High)) {
    /* TODO: Check this, how will I do with long elements */
    return Pin(Index*SlotSize, SlotSize, ActualLenght);
  }
}

/******************************************************************************
 *EndSelect:*

******************************************************************************/
void DBArray::EndSelect(int Index) {

  Unpin();
}

/******************************************************************************
 *EndSelectGet:*

******************************************************************************/
void DBArray::EndSelectGet(int Index, char *Dest) {

  EndSelect(Index);
  Get(Index, Dest);
}
