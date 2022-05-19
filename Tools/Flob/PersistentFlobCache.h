/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Department of Computer Science,
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

[10] Header File of Module PersistentFlobCache

June 2010, C. D[ue]ntgen: Added comments

1  Overview

The ~PersistentFlobCache~ is responsible for accelerating the access to data
referenced by non-native Flobs. Non-native Flobs are Flobs, whose persistent
data content is resident within some file different from the native Flob file

To enhance read/ write access to the according data, it is cached.

The FlobCache's view of a Flob is a sequence of fixed size memory slices.
Each slice can be kept separately within the cache, which consists of a limited
number of fixed size memory buffers (slices), that are organized in an open
hashtable containing instances of class ~CacheEntry~.

As a secondary organisation, a LRU-priority list linking the cache entries is used,
since LRU-policy is used to replace data if necessary.
If cached Flob data is modified, it is marked as ~changed~. Whenever changed
Flob data is removed from the Cache, it is written to disk. Unchanged Flob data
does not need to be written back to disk.

The functionality is the same as for the slot-based ~NativeFlobCache~ with one
difference:

If data is changed, it is immediately written to disk. This is to prevent
inconsistent data in case of a system crash.


*/


#include "Flob.h"

class CacheEntry;  // entry for a single slot


class PersistentFlobCache{
  public:

/*
1.1 Constructor

Creates a new Cache with given maximum size and given slotsize.
The maximumSize must be greater than the slotsize.

*/
    PersistentFlobCache(size_t _maxSize, size_t _slotSize, size_t _avgSize);
/*
1.2 Destructor

Destroys the Cache.

*/
    ~PersistentFlobCache();


/*
1.3 clear

Removes all entries from the cache.

*/
    size_t clear();

/*
1.4 getData

Copies the content of the Flob __flob__ from offset __offset__
and size __size__ into a provided buffer __buffer__. If the
requested data are not cached, the FlobManager is used to
access the data.

*/
    bool getData(const Flob& flob,
                 char* buffer,
                 const SmiSize offset,
                 const SmiSize size) ;

/*
1.5 putData

Puts the data given in buffer into the cache. Afterwards the
data are also written to disk using the FlobManager.

*/
    bool putData(Flob& flob,
                 const char* buffer,
                 const SmiSize offset,
                 const SmiSize size);

/*
1.6 killLastSlot

Removes the last slot of a flob. required for resizing flobs.

*/
   void killLastSlot(const Flob& flob);


  private:
    size_t maxSize;         // maximum allocated memory
    size_t slotSize;        // size of a single slot
    size_t usedSize;        // currently used memory
    CacheEntry** hashtable; // open hash table
    size_t tableSize;       // size of the hashtable
    CacheEntry* first;      // pointer to the first entry for lru
    CacheEntry* last;       // pointer to the last entry for lru


/*
1.7 getDataFromSlot

Retrieves data from a slot. If the slot is not available in cache,
the slot is created possible replacing other slots. The data are
get from the cache.

flob         : the flob containing the data
slotno       : the slot number of the flob
slotoffset   : offset within the slot
bufferoffset : offset within the buffer, increased by the number of bytes readed
size         : reads maximum size - bufferoffset bytes from slot
buffer       : the target

*/

   bool getDataFromSlot(const Flob& flob,
                        const size_t slotNo,
                        const size_t slotOffset,
                        size_t& bufferOffset,
                        const size_t size,
                        char* buffer);
/*
1.8 putDataToFlobSlot

contrary to getDataFromFlobSlot

*/
   bool putDataToFlobSlot(const Flob& flob,
                        const size_t slotNo,
                        const size_t slotOffset,
                        size_t& bufferOffset,
                        const size_t size,
                        const char* buffer);

/*
1.9 getData

returns the requested data stored in a given flob

*/
   void getData(CacheEntry* entry,
                const size_t slotOffset,
                size_t& bufferOffset,
                const size_t size,
                char* buffer);


   void putData(CacheEntry* entry,
                const size_t slotOffset,
                size_t& bufferOffset,
                const size_t size,
                const char* buffer);

/*
1.10 createEntry

creates a new CacheEntry and fills the data part using the
FlobManager. The entry is not inserted in the cache structure.

*/
   CacheEntry* createEntry(const Flob& flob, const size_t slotNo,
                           const bool readData=true) const;



/*
1.11 reduce

removes Entry from the cache structure until the maximum size is not
longer overcomed.

*/
   void reduce();


/*
1.12 putAtFront

puts a new entry at the front of the lru list.

*/
  void putAtFront(CacheEntry* entry);


/*
1.13 bringToFront

changes the position of an existing element to the front of the list.

*/
  void bringToFront(CacheEntry* entry);



/*
1.14 check

checks the internal structure.


*/
  bool check();


};
