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

//paragraph [10] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[10] Header File of Module FLOB

Stefan Dieker, 03/05/98. FLOB implementation in Secondo with the Shore based SMI.

Markus Spiekermann, 14/05/02. Starting to port the ~TupleManager~ to the Berkeley-DB based SMI.

Mirco G[ue]nster, 31/09/02. First stable version.

Victor Almeida, 24/04/03. Adapting the class to accept standalone 
objects and objects inside tuples transparently.

January 2006, Victor Almeida created the FLOB cache. Some assertions
were removed, since the code is stable.

August 2006, M. Spiekermann. Adding more comprehensive documentation (state-transitions) 
and some auxiliary debugging functions.

April 2007, M. Spiekermann. Implementations of many functions moved to the cpp-file. The functions
~SaveTo-~ and ~ReadFromExtensionTuple~ were replaced by ~ReadFrom~ and ~writeTo~. From now on the
FLOB class will be the ~only~ instance which ~frees~ the buffer allocated for inline flobs.

1  Overview

FLOB is a shortcut for ~faked larged object~ which is a concept for implementing
datatypes such as regions which may vary strongly in size. The idea of FLOBs has
been studied in [1] (The current implementation differs in some aspects). The
basic idea is to store data of an attribute value depending on a threshold size
either inside the tuple representation or in a separate storage location.  

FLOBs can be used in implementations of secondo data types. Therefore typically the
subclass ~DBArray~ might be used. Since the persistent storage of tuples is organized by
class tuple this class will allocate memory for inline FLOBs. But the memory will be
released by the FLOBs destructor.

*/

#ifndef FLOB_H
#define FLOB_H

#include "string.h"
#include "SecondoSMI.h"
#include "QueryProcessor.h"

/*
A FLOB has one of the following states
   
*/

enum FLOB_Type { Destroyed, 
                 InMemory, 
		 InMemoryCached, 
                 InMemoryPagedCached, 
		 InDiskLarge          };

/*
 
The possible state transitions are presented below (Any denotes the set of all
possible states and S is a state variable):

-----
 GetPage: InDiskLarge -> InMemoryPagedCached 
          InMemoryPagedCached -> InMemoryPagedCached
 
 Get: InDiskLarge -> InMemory, InMemoryCached, InMemoryPagedCached
      InMemory -> InMemory
      InMemoryCached -> InMemoryCached 
      InMemoryPagedCached -> InMemoryPagedCached
 
 Put: InMemory -> InMemory
      
 BringToMemory: InDiskLarge -> InMemory, InMemoryCached
                InMemoryPagedCached -> InDiskLarge -> InMemory, InMemoryCached
                InMemory -> InMemory
                InMemoryCached -> InMemoryCached                


 SaveToLob: InMemory, InMemoryCached, InDiskLarge -> InDiskLarge
 
 Resize: InMemory -> InMemory 
 
 ReadFrom: InMemory -> InMemory 
 
 WriteTo: InMemory -> InMemory 
 
 Destroy: S in Any \ {Destroyed} -> Destroyed

----

   
3 Class ~FLOB~

This class implements a FLOB which defines a new large object 
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
FLOB exceeds the threshold size the data will be stored in a separate 
file for LOBs.

*/

    static const size_t PAGE_SIZE;
/*
The page size for paged access.

*/

    static bool debug; 
/*
A debug flag. If true some trace messages are printed

*/


    inline FLOB() {}
/*
This constructor should not be used.

3.1 Constructor

Create a new FLOB from scratch with the given size.

*/
    FLOB( size_t size );

/*
3.3 Destructor

Deletes the FLOB instance.

*/
    virtual ~FLOB();


/*
3.5 Get

Returns a pointer to a memory block of the FLOB data with
offset ~offset~.

*/
    void Get( size_t offset, const char **target, 
                     const bool paged = false ) const;

/*
3.6 Put

Write data from ~source~ into the FLOBs memory.

*/
    inline void Put( size_t offset, size_t length, const void *source )
    {
      assert( type == InMemory );
      memcpy( fd.inMemory.buffer + offset, source, length );
    }

/*
3.7 Size

Returns the size of a FLOB.

*/
    inline size_t Size() const
    {
      return size;
    }

/*
3.8 Resize and Clean

Resizes the FLOB. A resize of 0 is not allowed. In order
to remove the memory representation the function ~Clean~
must be used.

*/
    void Resize( size_t size );

    void Clean();


/*
3.8 Destroy

Destroys the physical representation of the FLOB.

*/
    void Destroy();

/*
3.9 Restrict

Restricts the FLOB to the interval set passed as argument.

*/
    virtual void Restrict( const vector< pair<int, int> >& intervals )
    {}

/*
3.10 SaveToLob

Saves the FLOB to the LOB file. The FLOB must be a LOB. The type is 
set to ~InDiskLarge~.

*/
    virtual void SaveToLob( SmiRecordId& lobFileId, 
                            SmiRecordId lobId = 0 ) const;

/*
3.10 SetLobFile

Sets the LOB file. The FLOB must be a LOB.

*/
    inline void SetLobFileId( SmiFileId lobFileId )
    {
      assert( type == InDiskLarge );
      fd.inDiskLarge.lobFileId = lobFileId;
    }


/*
3.10 IsLob

Returns true, if value stored in underlying LOB, otherwise false.

*/
    bool IsLob() const;

/*
3.11 GetType

return the FLOB's internal state.

*/
    inline FLOB_Type GetType() const
    {
      return type;
    }


/*
3.12 Input output for ~inline~ FLOBs
  
Write or read the internal buffer of an ~InMemory~ FLOB to or from
the given ~dest~ or ~src~ pointer. 

Function ~ReadFrom~ allocates memory of the sufficient size and stores this
pointer in ~fd.inMemory.buffer~.  The current value of ~fd.inMemory.buffer~
will be ignored since it might be an address which was stored when the
attribute using this FLOB was written to disk. Afterwards it copies the buffer
pointed to by ~src~ into this new internal buffer. 

The Function ~WriteTo~ simply copies the internal buffer to a buffer pointed
to by ~dest~.

*/

    
    unsigned int ReadFrom(void* src);
    unsigned int WriteTo(void* dest) const;

/*
3.13 Allocation of the FLOB's internal buffer

The function below will allocate memory of sufficent size for the internal
buffer and stores a pointer to it in member ~fd.inMemory.buffer~

*/    
    void* Malloc(size_t newSize = 0);


  protected:

/*
4.1 BringToMemory

Reads a disk LOB to memory, i.e., converts a FLOB in ~InDiskLarge~
state to a ~InMemory~ state.

*/
    virtual const char *BringToMemory() const;

    
/*
4.2 GetPage

*/
    void GetPage( size_t page ) const;

/*
3.12 Attributes

*/

    mutable FLOB_Type type;
    size_t size;

    mutable 
    union FLOB_Descriptor
    {
      struct InMemory
      {
        char *buffer; 
	// a pointer to the memory representation of a flob
      } inMemory;

      struct InMemoryCached
      {
        const char *buffer;
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inMemoryCached;

      struct InMemoryPagedCached
      {
        char *buffer;
        bool cached;
        size_t pageno;
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inMemoryPagedCached;

      struct InDiskLarge
      {
        SmiFileId lobFileId;
        SmiRecordId lobId;
      } inDiskLarge;

    } fd;

    private:

/*

4 Internal Functions   


4.3 Handling States
   
*/    
    bool checkState(const FLOB_Type f) const;
	    
    bool checkState(const FLOB_Type f[], const int n) const;

    const string stateStr(const FLOB_Type& f) const;
   
    inline void setState(const FLOB_Type state) const
    {
      if (debug)
      { 
	cerr << "Flob " << (const void*)this << ": " 
	     << stateStr(type) << " -> " << stateStr(state) << endl;
      }  
      type = state;
      memset(&fd, sizeof(FLOB_Descriptor), 0);
    } 

};

#endif

/*
7 References

 [1] S. Dieker, R.H. G[ue]ting, and M. Rodriguez-Luaces, A Tool for Nesting and Clustering Large Objects. Proc. of the 12th Int. Conf. on Scientific and Statistical Database Management (SSDBM 2000), 169-181, July 2000.

*/

