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


*/


#include "PersistentFlobCache.h"
#include "Flob.h"
#include "FlobManager.h"
#include <assert.h>
#include <iostream>
#include <cstdlib>


/*
1 Class CacheEntry

*/

class CacheEntry{
  public:

/*
1.1 Constructor

Creates a new entry for Flob __flob__ and slot number __slotNo__
with given slot size.

*/
    CacheEntry(const Flob& _flob, const size_t _slotNo, size_t _slotSize):
      flob(_flob), slotNo(_slotNo), tableNext(0), tablePrev(0),
      lruPrev(0), lruNext(0) {
       size_t offset = _slotNo * _slotSize;
       assert(offset < flob.getSize());
       size = std::min(_slotSize, (flob.getSize() - offset)); 
       mem = (char*) malloc(size);
    }


/*
1.2 Destructor

Detroys an entry of a Flob.

*/

    virtual ~CacheEntry(){
      free(mem);
      mem = 0;
    }

/*
1.3 hashValue

Returns a hash value for this entry.

*/
    size_t hashValue(size_t tableSize){
      return (flob.hashValue() + slotNo) % tableSize;
    }

/*
1.4 check for equality / inequality


The flob as well as the slot number unique identify 
a cache entry.

*/
    bool operator==(const CacheEntry& e){
      return flob == e.flob &&
             slotNo == e.slotNo;
    }

    bool operator!=(const CacheEntry& e){
      return !(*this == e);
    }

/*
1.5 Members

all members are private because only the PersistentFlobCache class
known the CacheEntry class.

*/

   Flob flob;     
   size_t slotNo;
   CacheEntry* tableNext;
   CacheEntry* tablePrev;
   CacheEntry* lruPrev;
   CacheEntry* lruNext;
   char* mem;
   size_t size;
};


/*
1.6 Output operator

for simple output an entry.

*/

std::ostream& operator<<(std::ostream& o, const CacheEntry& e){
   o << "[" << e.flob << ", " << e.slotNo << "]" ;
  return o;
}


/*
2 PersistentFlobCache

In the cache for persistent flobs, flobs are splitted to a maximum size,
called slotSize. So a flob may consist of several slots. Each of them holds
memory of size __slotSize__ except the last slot for the flob which may be
smaller (depending on the size of the flob). 

The works in lru manner. It's organized using open hashing. The entries
are additionally linked within a double linked list to hold the order for
lru replacement.

Requested data are taken directly from the cache. If the slot containing the
data is not cached, it is   put into the storage. Thus a flob is read from disk 
slotwise.

When the put operation is called (which should be occur rarely on
 persistent flobs),
The slot stored in the cache is updated and also the disk content is 
written instantly.


2.1 Constructor

Creates a new cache of maximum size and given slotsize.

*/

PersistentFlobCache::PersistentFlobCache(size_t _maxSize, size_t _slotSize,
                                         size_t _avgSize):
    maxSize(_maxSize), slotSize(_slotSize), usedSize(0), first(0), last(0) {


   // compute a good tablesize
   assert(maxSize > slotSize);
   assert(_avgSize <= slotSize);   
 
   tableSize = ((maxSize / _avgSize) * 2);
   if(tableSize < 1u){
      tableSize = 1u;
   }   
   hashtable = new CacheEntry*[tableSize]; 
   for(unsigned int i=0;i<tableSize; i++){
     hashtable[i] = 0;
   }
}

/*
2.2 Destructor

destroys the cache.

*/


PersistentFlobCache::~PersistentFlobCache(){
   clear();
   delete[] hashtable;
   hashtable = 0;
}


/*
2.3 clear

removes all entries from the cache.

*/
void PersistentFlobCache::clear(){
  // first, kill entries from hashtable without deleting them from lru

  for(unsigned int i=0;i<tableSize;i++){
    hashtable[i] =0;
  }
  CacheEntry* entry = first;
  size_t killed = 0;

  while(entry){
    CacheEntry* victim = entry;
    entry = entry->lruNext;
    killed += victim->size;
    delete victim;
  }


  first = 0;
  last = 0;
  usedSize = 0;
}


/*
2.4 getData

retrieves data from the cahce. If the data are not cached, the
FlobManager is used to access the data.

*/
bool PersistentFlobCache::getData(
             const Flob& flob, 
             char* buffer,
             const SmiSize offset, 
             const SmiSize size ){

    if(size==0){  // nothing to do
      return true;
    }
    size_t slotNo = offset / slotSize;
    size_t slotOffset = offset % slotSize; 
    size_t bufferOffset(0);

    while(bufferOffset < size){
      if(!getDataFromSlot(flob, slotNo, slotOffset, 
                          bufferOffset, size, buffer)){
        std::cerr << "Warning getData failed" << std::endl;
        return false;
      }
       
      slotNo++;
      slotOffset = 0; 
    }
    return true;
}

/*
2.5 putData

updates the data in cache and on disk

*/
bool PersistentFlobCache::putData(
      Flob& flob,
      const char* buffer,
      const SmiSize offset,
      const SmiSize size) {

   size_t slotNo = offset / slotSize; 
   size_t slotOffset = offset % slotSize;
   size_t bufferOffset(0);
   while(bufferOffset < size){
     putDataToFlobSlot(flob, slotNo, slotOffset, bufferOffset, size, buffer);
     slotNo++;
     slotOffset = 0;
   }
   return FlobManager::getInstance().putData(flob, buffer, offset, size, true);
}


/*
2.6 killLastSlot

removes the last slot from the cache if exist. Its required if a resize
function is called.


*/

void PersistentFlobCache::killLastSlot(const Flob& flob){

  size_t slotNo = flob.getSize() / slotSize;
  size_t index = (flob.hashValue() + slotNo) % tableSize;

  if(hashtable[index]==0){
    return;
  } 
  CacheEntry* e = hashtable[index];
  while(e){
    if(e->flob!=flob || e->slotNo!=slotNo){
      e = e->tableNext;
    } else { // victim found
      // remove from table


      if(hashtable[index]==e){
        hashtable[index] = e->tableNext;
        if(e->tableNext){
          e->tableNext->tablePrev = 0; 
        }
      } else {
        e->tablePrev->tableNext = e->tableNext;
        if(e->tableNext){
          e->tableNext->tablePrev = e->tablePrev;
        }
      }
      e->tablePrev=0;
      e->tableNext=0;
      // delete from lru
      if(e==first){
         if(e==last){ // last element 
            first = 0;
            last = 0;
         } else {
            first = first->lruNext;
            first->lruPrev = 0;
            e->lruNext = 0;
         }
      } else{ // e!=first
         if(e==last){
           last = last->lruPrev;
           last->lruNext = 0;
           e->lruPrev=0;
         } else { // remove from the middle of lrulist
           e->lruPrev->lruNext = e->lruNext;
           e->lruNext->lruPrev = e->lruPrev;
           e->lruPrev = 0;
           e->lruNext = 0;
         }
      }
      usedSize -= e->size;
      delete e; 
      e = 0; // stop loop
    }
  } 
}

/*
2.7 getDataFromSlot

retrieves the flob data for a specidied flob.

*/

bool PersistentFlobCache::getDataFromSlot(const Flob& flob,
                     const size_t slotNo,
                     const size_t slotOffset,
                     size_t& bufferOffset,
                     const size_t size,
                     char* buffer) {

   unsigned int index = (flob.hashValue() + slotNo) % tableSize;
   if(hashtable[index]==0){
     CacheEntry* newEntry = createEntry(flob, slotNo); 
     hashtable[index] = newEntry;
     usedSize += newEntry->size; 
     putAtFront(newEntry);
     reduce(); // remove entries if too much memory is used
     getData(newEntry, slotOffset, bufferOffset, size, buffer);
     return true;
   }
 
   // hashtable[index] is already used
   CacheEntry* entry = hashtable[index];
   while(entry->tableNext && (entry->flob != flob || entry->slotNo!=slotNo)){
     entry = entry->tableNext;
   }

   if(entry->flob != flob || entry->slotNo!=slotNo){ // no hita
     CacheEntry* newEntry = createEntry(flob, slotNo); 
     newEntry->tablePrev = entry;
     entry->tableNext = newEntry;
     assert(first);
     putAtFront(newEntry);
     usedSize += newEntry->size; 
     reduce(); 
     getData(newEntry, slotOffset, bufferOffset, size, buffer);
     return true;
   } else { // hit, does not access disk data
     getData(entry, slotOffset, bufferOffset, size, buffer);
     bringToFront(entry);
     return true;
   }
}

/*
2.8 getData

Copies data from a given CacheEntry to a buffer.

*/
void PersistentFlobCache::getData(CacheEntry* entry,  
                              size_t slotOffset, 
                              size_t& bufferOffset, 
                              const size_t size,
                              char* buffer){
  size_t mb = size-bufferOffset; // missing bytes
  size_t ab = entry->size - slotOffset; // bytes available in slot
  size_t pb = std::min(mb,ab); // provided bytes
  memcpy(buffer+bufferOffset, entry->mem + slotOffset, pb);
  bufferOffset += pb;
}

/*
2.9 putData

puts data to a given CacheEntry

*/

void PersistentFlobCache::putData(CacheEntry* entry,  
                              const size_t slotOffset, 
                              size_t& bufferOffset, 
                              const size_t size,
                              const char* buffer){
  size_t mb = size-bufferOffset; // missing bytes
  size_t ab = entry->size - slotOffset; // bytes available in slot
  size_t pb = std::min(mb,ab); // provided bytes
  memcpy(entry->mem + slotOffset, buffer+bufferOffset, pb);
  bufferOffset += pb;
}

/*
2.10 createEntry

produces a new CacheEntry. If readData are set to be __true__ (default), 
the slot data are load from disk using the FlobManager.

*/
CacheEntry* PersistentFlobCache::createEntry(const Flob& flob, 
                                const size_t slotNo,
                                const bool readData/*=true*/) const{
   CacheEntry* res = new CacheEntry(flob, slotNo, slotSize);
   if(readData){
      FlobManager::getInstance().getData(flob,res->mem, 
                                         slotNo*slotSize, res->size, true);
   }
   return res;
}

/*
2.11 reduce

removes slot from the cache until the size is smaller than the maximum one.

*/

void PersistentFlobCache::reduce(){
   while(usedSize > maxSize){
      CacheEntry* victim = last;
      // remove from lru 
      last = victim->lruPrev;
      last->lruNext = 0;
      victim->lruPrev = 0;
      assert(victim->lruNext == 0);

      // remove from hashtable

      if(victim->tablePrev){ // not the first entry in table
        if(victim->tableNext){
          victim->tablePrev->tableNext = victim->tableNext;
          victim->tableNext->tablePrev = victim->tablePrev;
          victim->tablePrev = 0;
          victim->tableNext = 0;
        } else { // last entry in table
          victim->tablePrev->tableNext = 0;
          victim->tablePrev = 0;
        }
      } else {  // first entry in table
        size_t index = victim->hashValue(tableSize);
        assert(hashtable[index] == victim);
        if(victim->tableNext==0){ // the only entry in hashtable
           hashtable[index] = 0;
        } else {
           hashtable[index] = victim->tableNext;
           victim->tableNext->tablePrev = 0;
           victim->tableNext = 0;
        }
      }
      usedSize -= victim->size;
      delete victim;
   }
}

/*
2.12 putDataToFlobSlot

puts data to a given slot. If the slot is not present in the
cache, it is created.


*/
bool PersistentFlobCache::putDataToFlobSlot(
        const Flob& flob,
        const size_t slotNo,
        const size_t slotOffset,
        size_t& bufferOffset,
        const size_t size,
        const char* buffer){

  size_t index = (flob.hashValue() + slotNo) % tableSize;
  if(hashtable[index] == 0){ // first entry in hashtable
     bool rd = slotOffset==0 && size >=slotSize;
     CacheEntry* newEntry = createEntry(flob, slotNo, rd);
     putData(newEntry, slotOffset, bufferOffset, size, buffer);
     hashtable[index] = newEntry;
     usedSize += newEntry->size;
     putAtFront(newEntry);
     reduce();
     return true;
  }

  CacheEntry* entry = hashtable[index];
  while(entry->tableNext && (entry->flob!=flob || entry->slotNo!=slotNo)){
     entry = entry->tableNext;
  }

  if(entry->flob==flob && entry->slotNo==slotNo){ // hit in cache
     putData(entry, slotOffset, bufferOffset, size,buffer);
     bringToFront(entry);
     return true;
  }

  // append a new entry at the end of the tablelist
  bool rd = slotOffset==0 && size >=slotSize;
  CacheEntry* newEntry = createEntry(flob, slotNo, rd);
  putData(newEntry, slotOffset, bufferOffset, size, buffer);
  entry->tableNext = newEntry;
  entry->tablePrev = entry;
  usedSize += newEntry->size;
  putAtFront(newEntry);
  reduce();
  return true;
}

/*
2.13 putAtFront

puts an unused CacheEntry to the front of the LRU list.

*/
void PersistentFlobCache::putAtFront(CacheEntry* newEntry){
   assert(newEntry->lruPrev==0);
   assert(newEntry->lruNext==0);
   if(!first){
     assert(!last);
     first = newEntry;
     last = newEntry;
   } else {
     newEntry->lruNext = first;
     first->lruPrev = newEntry;
     first = newEntry;
   }
}

/*
2.14 bringToFront

moves an entry already present in the lrulist to the top of that list.

*/
void PersistentFlobCache::bringToFront(CacheEntry* entry){
  if(first==entry){ // already on front
     return;
  }
  if(last==entry){ // the last element
    assert(last->lruNext==0);
    last= last->lruPrev;
    last->lruNext=0;
    entry->lruPrev=0;
    entry->lruNext = first;
    first->lruPrev = entry;
    first = entry;
    return;
  }
  // entry in in the middle of the list
  assert(entry->lruPrev);
  assert(entry->lruNext);
  entry->lruPrev->lruNext = entry->lruNext;
  entry->lruNext->lruPrev = entry->lruPrev;
  entry->lruPrev = 0;
  entry->lruNext = first;
  first->lruPrev = entry;
  first = entry;
}


/*
2.14 check

Debugging function.

*/
bool PersistentFlobCache::check(){
  if(first==0 || last==0){
    if(first!=last){
      std::cerr << "inconsistence in lru list, first = " << (void*) first 
           << " , last = " << (void*) last << std::endl;
      return false;
    }
    for(unsigned int i=0;i< tableSize;i++){
      if(hashtable[i]){
         std::cerr << "lru is empty, but hashtable[" << i 
              << "] contains an element" << std::endl;
         return false;
      }
    }
    // empty cache
    return true;
  }
  // lru is not empty, check first and last element
  if(first->lruPrev){
    std::cerr << "lru: first has a predecessor" << std::endl;
    return false;
  }
  if(last->lruNext){
    std::cerr << "lru: last has a successor" << std::endl;
    return false;
  }
  // check whether ech element in lru is also an element in hashtable
  CacheEntry* e = first;
  int lrucount = 0;
  while(e){
    lrucount++;
    size_t index = e->hashValue(tableSize); 
    if(!hashtable[index]){
      std::cerr << "element " << (*e) << " stored in lru but hashtable[" 
           << index << " is null" << std::endl;
      return  false;
    }
    CacheEntry* e2 = hashtable[index];
    while(e2 && (*e)!=(*e2)){
      e2 = e2->tableNext;
    }  
    if(!e2){
      std::cerr << "element " << (*e) << " stored in lru but not in hashtable[" 
           << index << "]" << std::endl;
      return false;
    }
    e = e->lruNext;
  }
  // check hashtable
  int tablecount = 0;
  for(unsigned int i=0; i<tableSize;i++){
    if(hashtable[i]){
       if(hashtable[i]->tablePrev){
           std::cerr << " hashtable[" << i << " has a predecessor" << std::endl;
       }
       e = hashtable[i];
       while(e){
         tablecount++;
         if(e->hashValue(tableSize)!=i){
           std::cerr << "element << " << (*e) << " has hashvalue " 
                << e->hashValue(tableSize) << " but is stored at position " 
                << i << std::endl;
           return false;
         }

         if(e->tableNext){
             if(e->tableNext->tablePrev!=e){
                std::cerr << "error in tablelist found" << std::endl;
                return false;
             }
         }
         e = e->tableNext;
       }
    }
  }

  if(lrucount!=tablecount){
    std::cerr << "lrucount = " << lrucount << " #  tablecount = " 
         << tablecount << std::endl;
    return false;
  }

  return true;
}



