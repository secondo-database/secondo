/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[1] Header File of Module FLOB

Stefan Dieker, 03/05/98

Markus Spiekermann, 14/05/02. Begin of porting to the new SecondoSMI.

Mirco G[ue]nster, 31/09/02 End of porting to the new SecondoSMI.

Victor Almeida, 24/04/03. Adapting the class to accept standalone 
objects and objects inside tuples transparently.

January 2006 Victor Almeida created the FLOB cache. Some assertions
were removed, since the code is stable.

1 Defines

*/

#ifndef FLOB_H
#define FLOB_H

/*

2 Includes

*/
#include "SecondoSMI.h"

enum FLOB_Type {Destroyed, InMemory, InMemoryCached, 
                InDiskSmall, InDiskLarge};

/*
3 class FLOB

This class implements a FLOB which define a new large object 
abstraction that offers most access methods of the original one.

It defines a threshold size namely SWITCH\_THRESHOLD as a static 
attribute. This value defines a limit between "small" and "large" 
values referring its size.

A small FLOB is stored in a main memory structure whereas a large 
FLOB is stored into a seperate SmiRecord in an SmiFile.

*/
class FLOB
{

  public:

    static const size_t SWITCH_THRESHOLD;
/*
This is the treshold size for a FLOB. Whenever the size of this 
FLOB exceeds the threshold size the data will stored in a separate 
file for LOBs.

*/

    FLOB() {}
/*
This constructor should not be used.

3.1 Constructor

Create a new FLOB from scratch.

*/
    FLOB( size_t sz );

/*
3.3 Destructor

Deletes the FLOB instance.

*/
    ~FLOB();


/*
3.4 BringToMemory

Brings a disk lob to memory, i.e., converts a flob in ~InDiskLarge~
state to a ~InMemory~ state.

*/
    char *BringToMemory();

/*
3.5 Get

Read by copying

*/
    void Get( size_t offset, size_t length, void *target );

/*
3.6 Put

Write Flob data into source.

*/
    void Put( size_t offset, size_t length, const void *source );

/*
3.7 Size

Returns the size of a FLOB.

*/
    size_t Size() const;


/*
3.8 Resize

Resizes the FLOB.

*/
    void Resize( size_t size );

/*
3.8 Clear

Clears the FLOB.

*/
    void Clear();

/*
3.8 Clean

Cleans the FLOB, removing it from memory. If it is cached, then a
reference in the cache is removed.

*/
    void Clean();

/*
3.8 Destroy

Destroys the physical representation of the FLOB.

*/
    void Destroy();

/*
3.10 SaveToLob

Saves the FLOB to the LOB file. The FLOB must be a LOB. The type is 
set to ~InDiskLarge~.

*/
    void SaveToLob( SmiRecordId& lobFileId, SmiRecordId lobId = 0 );

/*
3.10 SetLobFile

Sets the LOB file. The FLOB must be a LOB.

*/
    void SetLobFileId( SmiFileId lobFileId );

/*
3.11 SaveToExtensionTuple

Saves the FLOB to a buffer of an extension tuple and sets its type 
to ~InDiskSmall~.

*/
    void SaveToExtensionTuple( void *extensionTuple );

/*
3.12 ReadFromExtensionTuple

Reads the FLOB value from an extension tuple. There are two ways of 
reading, one uses a Prefetching Iterator and the other reads 
directly from the SMI Record.

The FLOB must be small.

*/
    size_t ReadFromExtensionTuple( PrefetchingIterator& iter, 
                                   size_t offset );
    size_t ReadFromExtensionTuple( SmiRecord& record, 
                                   size_t offset );

/*
3.10 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/
    bool IsLob() const;

/*
3.11 GetType

*/
    FLOB_Type GetType() const;


  protected:

/*
3.12 Attributes

*/
    FLOB_Type type;
    size_t size;

    union FLOB_Descriptor
    {
      struct InMemory
      {
        char *buffer;
      } inMemory;

      struct InMemoryCached
      {
        char *buffer;
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inMemoryCached;

      struct InDiskLarge
      {
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inDiskLarge;

    } fd;
};

#endif
