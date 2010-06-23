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

[10] Header File of Module NativeFlobCache

June 2010, C. D[ue]ntgen: Added comments

1  Overview

The ~NativeFlobCache~ is responsible for accelerating the access to data
referenced by native Flobs. Native Flobs are temporary Flobs, i.e. Flobs that
are created e.g. within local variables of a Flob-containing data type. The
data referenced by these native Flobs is maintained within a dedicated file, the
native Flob file, which is managed by the FlobManager.

Since temporary Flobs are often accessed repeatedly by many algorithms, e.g. to
append data, not every single access should rely on disk access. Thus, a main
memory buffer is used for cached read/ write access to the according data.

If a native Flob is destroyed or deleted by the FlobManager, its content is never
modified on disk, regardless whether is is marked modified or not. The according
cache entries are just removed from the cache. This is due to the temporary
nature of native Flobs.

1.1 Slot Based Approach

The FlobCache's view of a Flob is a sequence of fixed size memory slices.
Each slice can be kept separately within the cache, which consists of a limited
number of fixed size memory buffers (slices), that are organized in an open
hashtable containing instances of class ~NativeCacheEntry~.

As a secondary organisation, a LRU-priority list linking the cache entries is used,
since LRU-policy is used to replace data if necessary.
If cached Flob data is modified, it is marked as ~changed~. Whenever changed
Flob data is removed from the Cache, it is written to disk. Unchanged Flob data
does not need to be written back to disk.

1.2 Whole-Flob-Based Approach

In this variant, the Flob data is organized using an AVLTree. A Flob is
copied to the cache either whole or not at all (e.g. if it does not fit into the
cache).

*/

#include "Flob.h"
#include "FlobManager.h"
#include <assert.h>
#include <iostream>
#include <cstdlib>

#undef __TRACE_ENTER__
#undef __TRACE_LEAVE__

#define __TRACE_ENTER__
#define __TRACE_LEAVE__

 /*#define __TRACE_ENTER__ std::cerr << "Enter : " << \
        __PRETTY_FUNCTION__ << std::endl;

#define __TRACE_LEAVE__ std::cerr << "Leave : " << \
        __PRETTY_FUNCTION__ << "@" << __LINE__ << std::endl;
  */


 //#undef SLOT_BASED_APPROACH

#define SLOT_BASED_APPROACH


#ifdef SLOT_BASED_APPROACH

/*
1 Class NativeCacheEntry

*/

class NativeCacheEntry{
  public:

/*
1.1 Constructor

Creates a new entry for Flob __flob__ and slot number __slotNo__
with given slot size.

*/
  NativeCacheEntry(const Flob& _flob, const size_t _slotNo, size_t _slotSize):
    flobId(_flob.id), slotNo(_slotNo), tableNext(0), tablePrev(0),
    lruPrev(0), lruNext(0), changed(false) {

    assert(_flob.size>0);
    size_t offset = _slotNo * _slotSize;

 //    if(offset > _flob.getSize()){
 //      cout << "Inavlid offset" << endl;
 //      cout << "offset = " << offset << endl;
 //      cout << " flob.size = " << _flob.getSize() << endl;
 //     cout << "slotno = " << _slotNo << endl;
 //   }

    assert(offset <=  _flob.getSize());

    size = min(_slotSize, (_flob.getSize() - offset));
    mem = (char*) malloc(size);
  }

  void resize(const size_t _newFlobSize, const size_t _slotSize){
    size_t offset = slotNo * _slotSize;
    assert(offset <= _newFlobSize);
    size_t mySize = min(_slotSize, (_newFlobSize  - offset));
    if(mySize==size){
      return;
    }
    if(mySize>0){
       mem = (char*) realloc(mem, mySize);
    } else {
       free(mem);
       mem = 0;
    }
    size = mySize;
  }


/*
1.2 Destructor

Detroys an entry of a Flob.

*/
    virtual ~NativeCacheEntry(){
      if(mem){
         free(mem);
      }
      mem = 0;
    }

/*
1.3 hashValue

Returns a hash value for this entry. The hash value is used to quickly test
whether a slice of Flob data is within the cache.

*/
    size_t hashValue(size_t tableSize){
      return (flobId.hashValue() + slotNo) % tableSize;
    }

/*
1.4 check for equality/ inequality

A pair (Flob-id, slot number) uniquely identifies a cache entry.

*/
    bool operator==(const NativeCacheEntry& e){
      return flobId == e.flobId &&
             slotNo == e.slotNo;
    }

    bool operator!=(const NativeCacheEntry& e){
      return !(*this == e);
    }

    bool matches(const Flob& f, const size_t slotNo) const{
      return (f.id == flobId) && (this->slotNo == slotNo);
    }

    bool matches(const FlobId& fid, const size_t slotNo) const{
      return (fid == flobId) && (this->slotNo == slotNo);
    }

   ostream& print(ostream& o) const{
     o << "FlobId = " << flobId << ", slot = " << slotNo << ", size = " << size;
     return o;
   }

/*
1.5 Members

All members are private because only the ~FlobCache~ class knows about and uses
the ~NativeCacheEntry~ class.

*/

   FlobId flobId;
   size_t slotNo;
   NativeCacheEntry* tableNext;
   NativeCacheEntry* tablePrev;
   NativeCacheEntry* lruPrev;
   NativeCacheEntry* lruNext;
   bool changed;
   char* mem;
   size_t size;
};


/*
1.6 Output operator

For simple output of an entry to an ostream.

*/

ostream& operator<<(ostream& o, const NativeCacheEntry& e){
   o << "[" << e.flobId << ", " << e.slotNo << "]" ;
  return o;
}



class NativeFlobCache{

 public:

/*

2.1 Constructor

Creates a new cache with a given maximum size and a given slotsize.

*/


NativeFlobCache(size_t _maxSize, size_t _slotSize, size_t _avgSize):
    maxSize(_maxSize), slotSize(_slotSize), usedSize(0), first(0), last(0) {

  assert(maxSize > slotSize);
  assert(_avgSize <= slotSize);

   assert(slotSize);

   tableSize = ((maxSize / _avgSize) * 2);
   if(tableSize < 1u){
      tableSize = 1u;
   }
   // initialize hashTable
   hashtable = new NativeCacheEntry*[tableSize];
   for(unsigned int i=0;i<tableSize; i++){
     hashtable[i] = 0;
   }
   //assert(check());
}

/*
2.2 Destructor

destroys the cache.

*/


~NativeFlobCache(){
   //assert(check());
   clear();
   delete[] hashtable;
   hashtable = 0;
}


/*
2.3 clear

removes all entries from the cache.

*/
void clear(){
  // first, kill entries from hashtable without deleting them from lru
  //assert(check());
  for(unsigned int i=0;i<tableSize;i++){
    hashtable[i] =0;
  }
  NativeCacheEntry* entry = first;
  size_t killed = 0;

  while(entry){
    NativeCacheEntry* victim = entry;
    entry = entry->lruNext;
    killed += victim->size;
    delete victim;
  }


  first = 0;
  last = 0;
  usedSize = 0;
  //assert(check());

}


/*
2.4 getData

Retrieves Flob data. First, it tries to get it from the cache.
If the data are not cached, the FlobManager is used to access the data and bring
it into the cache. Empty Flobs are not cached.

*/
bool getData(
             const Flob& flob,
             char* buffer,
             const SmiSize offset,
             const SmiSize size ){

    if(size==0){  // nothing to do
      return true;
    }

    //assert(check());

    size_t slotNo = offset / slotSize;
    size_t slotOffset = offset % slotSize;
    size_t bufferOffset(0);

    while(bufferOffset < size){
      if(!getDataFromSlot(flob, slotNo, slotOffset,
                          bufferOffset, size, buffer)){
        cerr << "Warning getData failed" << endl;
        //assert(check());
        return false;
      }
      slotNo++;
      slotOffset = 0;  // slotoffset only neeeded for the first slot
    }
    //assert(check());
    return true;
}

/*
2.5 putData

Updates the data of a Flob. If the data is not resident within the FlobCache,
it is brought to memory. All touched slices are marked ~modified~.

*/
bool putData(
      const Flob& flob,
      const char* buffer,
      const SmiSize offset,
      const SmiSize size) {

   //assert(check());
   size_t slotNo = offset / slotSize;
   size_t slotOffset = offset % slotSize;
   size_t bufferOffset(0);

   while(bufferOffset < size){
     putDataToFlobSlot(flob, slotNo, slotOffset, bufferOffset, size, buffer);
     slotNo++;
     slotOffset = 0;
   }
   //assert(check());
   return true;
}


bool saveToDisk(Flob& flob, NativeCacheEntry* e){
    if(e==0 || e->size==0){ // nothing to do
      return true;
    }
    return FlobManager::getInstance().putData(flob, e->mem,
                                              e->slotNo * slotSize,
                                              e->size, true);
}


bool saveToDisk(const FlobId& flobId, NativeCacheEntry* e){
    if(e==0 || e->size==0){ // nothing to do
      return true;
    }
    return FlobManager::getInstance().putData(flobId, e->mem,
                                e->slotNo * slotSize, e->size);
}



/*
2.7 eraseSlot

Removes a slot from the cache. If saveChanges is set to true,
the slot is stored to disk if the ~modified~ flag is set.

*/
bool eraseSlot(Flob& flob, const size_t slotNo, const bool saveChanges){
  //assert(check());

  size_t index = (flob.id.hashValue() + slotNo) % tableSize;
  NativeCacheEntry* e = hashtable[index];
  while(e && !e->matches(flob, slotNo)){
    e = e->tableNext;
  }
  if(e==0){ // slot not chached
    return true;
  }
  if(saveChanges && e->changed){
    if(!this->saveToDisk(flob,e)){
      return false;
    }
  }


  // remove slot from hashtable
  if(hashtable[index] == e){ // e is the first entry in the table
     hashtable[index] = e->tableNext;
     if(hashtable[index]){
        hashtable[index]->tablePrev = 0;
     }
  } else { // not the top entry
    e->tablePrev->tableNext = e->tableNext;
    if(e->tableNext){
       e->tableNext->tablePrev = e->tablePrev;
    }
  }
  e->tablePrev = 0;
  e->tableNext = 0;
  // remove from lru
  if(e==first){
    if(e==last){
       first = 0;
       last = 0;
    } else {
       first = e->lruNext;
       first->lruPrev = 0;
       e->lruNext = 0;
    }
  } else { // e is not the top lru element
    if(e==last){
      last = e->lruPrev;
      last->lruNext = 0;
      e->lruPrev = 0;
    } else {
      e->lruPrev->lruNext = e->lruNext;
      e->lruNext->lruPrev = e->lruPrev;
      e->lruPrev = 0;
      e->lruNext = 0;
    }
  }
  assert(usedSize >= e->size);
  usedSize -= e->size;
  delete e;
  //assert(check());
  return true;
}


/*
removes a slot from cache

*/
bool erase(Flob& flob, const bool saveChanges=false){

  if(flob.size > 536870912){ // 512 MB
    cerr << "Warning try to erase very big flob , size = " << flob.size <<endl;
  }
  //assert(check());
  if(flob.size/slotSize < tableSize*4 ){
     size_t numFlobs = flob.getSize() / slotSize;
     for(unsigned int i = 0 ; i<= numFlobs; i++){
       if(!eraseSlot(flob, i, saveChanges)){
         //assert(check());
         return false;
       }
     }
  } else {
   // scan the whole cache, because its faster in most cases
   for(unsigned int i=0;i<tableSize;i++){
     NativeCacheEntry* e = hashtable[i];
     while(e){
        if(e->flobId==flob.id){
           if(!eraseSlot(flob,e->slotNo, saveChanges)){
             return false;
           }  else {
             e = hashtable[i];
           }
        } else {
           e = e->tableNext;
        }
     }
   }
  }
  return true;
}


/*


*/
bool create(Flob& flob){
  //assert(check());
  if(flob.size >0){
    size_t num = flob.size/slotSize;
    for(size_t i =0; i<num; i++){
      NativeCacheEntry* e = new NativeCacheEntry(flob,i, slotSize );
      if(e->size>0){
        usedSize += e->size;
        putAtFront(e);
        size_t index = e->hashValue(tableSize);
        if(hashtable[index]){
           e->tableNext = hashtable[index];
           hashtable[index]->tablePrev = e;
        }
        hashtable[index] = e;
        reduce();
      } else {
        delete e;
      }
    }
  }
  //assert(check());
  return true;
}



/*
2.7 getDataFromSlot

retrieves flob data for a specified flob from the cache and copies it to the
indicated position of a provided buffer.

*/

bool getDataFromSlot(const Flob& flob,
                     const size_t slotNo,
                     const size_t slotOffset,
                     size_t& bufferOffset,
                     const size_t size,
                     char* buffer) {

   unsigned int index = (flob.hashValue() + slotNo) % tableSize;


   if(hashtable[index]==0){
     NativeCacheEntry* newEntry = createEntry(flob, slotNo,true);
     if(newEntry){
       hashtable[index] = newEntry;
       usedSize += newEntry->size;
       putAtFront(newEntry);
       reduce(); // remove entries if too much memory is used
       getData(newEntry, slotOffset, bufferOffset, size, buffer);
     }
     return true;
   }

   // hashtable[index] is already used
   NativeCacheEntry* entry = hashtable[index];
   while(entry->tableNext && !(entry->matches(flob.id, slotNo))){
     entry = entry->tableNext;
   }

   if(!entry->matches(flob.id, slotNo)){ // no hit
     NativeCacheEntry* newEntry = createEntry(flob, slotNo, true);
     if(newEntry){
       newEntry->tablePrev = entry;
       entry->tableNext = newEntry;
       assert(first);
       putAtFront(newEntry);
       usedSize += newEntry->size;
       reduce();
       getData(newEntry, slotOffset, bufferOffset, size, buffer);
     }
     return true;
   } else { // hit, does not access disk data
     getData(entry, slotOffset, bufferOffset, size, buffer);
     bringToFront(entry);
     return true;
   }
}

/*
2.8 getData

Copies data from a given NativeCacheEntry to a buffer.

*/
void getData(NativeCacheEntry* entry,
                              size_t slotOffset,
                              size_t& bufferOffset,
                              const size_t size,
                              char* buffer){
  size_t mb = size-bufferOffset; // missing bytes
  size_t ab = entry->size - slotOffset; // bytes available in slot
  size_t pb = min(mb,ab); // provided bytes
  memcpy(buffer+bufferOffset, entry->mem + slotOffset, pb);
  bufferOffset += pb;
}

/*
2.9 putData

puts data from a buffer to a given NativeCacheEntry and marks it modified.

*/

void putData(NativeCacheEntry* entry,
                              const size_t slotOffset,
                              size_t& bufferOffset,
                              const size_t size,
                              const char* buffer){
  size_t mb = size-bufferOffset; // missing bytes
  size_t ab = entry->size - slotOffset; // bytes available in slot
  size_t pb = min(mb,ab); // provided bytes
  if(pb>0){
    memcpy(entry->mem + slotOffset, buffer+bufferOffset, pb);
    bufferOffset += pb;
    entry->changed=true;
  }
}

/*
2.10 createEntry

produces a new NativeCacheEntry. If readData are set to be __true__ (default),
the slot data are loaded from disk using the ~FlobManager~.

*/
NativeCacheEntry* createEntry(const Flob& flob,
                                const size_t slotNo,
                                const bool readData=true) const{
   NativeCacheEntry* res = new NativeCacheEntry(flob, slotNo, slotSize);
   if(res->size==0){
      delete res;
      return 0;
   }
   if(readData){
      FlobManager::getInstance().getData(flob,res->mem,
                                         slotNo*slotSize, res->size, true);
      res->changed = false;
   }
   return res;
}

/*
2.11 reduce

removes slots from the cache until the size is smaller than the cache's maximum
size.

*/

void reduce(){
   while(last && usedSize > maxSize){
      NativeCacheEntry* victim = last;

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
      assert(usedSize >= victim->size);
      usedSize -= victim->size;
      if(victim->changed){
        saveToDisk(victim->flobId, victim);
      }
      delete victim;
   }
   if(!last){
     first = 0;
   }
}

/*
2.12 putDataToFlobSlot

puts data to a given slot. If the slot is not present in the
cache, it is created.


*/
bool putDataToFlobSlot(
        const Flob& flob,
        const size_t slotNo,
        const size_t slotOffset,
        size_t& bufferOffset,
        const size_t size,
        const char* buffer){

  //assert(check());
  size_t index = (flob.hashValue() + slotNo) % tableSize;

  NativeCacheEntry* entry = hashtable[index];
  while(entry && !entry->matches(flob.id, slotNo)){
    entry = entry->tableNext;
  }

  if(!entry){ // slot not cached
    entry = createEntry(flob, slotNo);
    if(entry){
      putAtFront(entry);
      entry->tableNext = hashtable[index];
      if(hashtable[index]){
         hashtable[index]->tablePrev = entry;
      }
      hashtable[index] = entry;
      usedSize += entry->size;
      reduce();
    } else {
      return true;
    }
  }
  putData(entry, slotOffset, bufferOffset, size,buffer);
  return true;

}


bool resize(Flob& flob, const size_t newSize){

   if(flob.size==0){
      flob.size= newSize;
      create(flob);
      return true;
   }
   if(newSize==0){
      erase(flob,false);
      flob.size = 0;
      return true;
   }
   if(newSize < flob.size){
      size_t start = newSize/slotSize + 1;
      size_t end = flob.size/slotSize + 1;
      for(size_t i = start; i< end; i++){
        eraseSlot(flob,i,false);
      }
      resizeSlot(flob,start-1,newSize);
      flob.size=newSize;
      return true;
   }
   // flob is enlarged
   size_t start = flob.size/slotSize + 1;
   resizeSlot(flob, start-1, newSize);
   size_t end = newSize/slotSize +1;
   flob.size = newSize;
   for(size_t i=start;i<end;i++){
      NativeCacheEntry* e = new NativeCacheEntry(flob,i,slotSize);
      if(e->size>0){
        usedSize += e->size;
        putAtFront(e);
        size_t index = e->hashValue(tableSize);
        if(hashtable[index]){
           e->tableNext = hashtable[index];
           hashtable[index]->tablePrev = e;
        }
        hashtable[index] = e;
      } else {
        delete e;
      }
   }
   reduce();
   return true;
}

bool resizeSlot(Flob& flob, size_t slot, size_t newFlobSize){
   size_t index = (flob.id.hashValue()+slot)%tableSize;
   NativeCacheEntry* e = hashtable[index];
   while( e && !e->matches(flob.id, slot)){
     e = e->tableNext;
   }
   if(e){
     size_t oldSize = e->size;
     e->resize(newFlobSize, slotSize);
     if(e->size==0){ // delete entry because size is 0
       usedSize -= oldSize;
       // delete from lru
       if(first==e){
         first = first->lruNext;
         if(first){
            first->lruPrev = 0;
         } else {
           last = 0;
         }
       } else if(e==last){
          last = last->lruPrev;
          last->lruNext = 0;
       } else {
           e->lruPrev->lruNext = e->lruNext;
           e->lruNext->lruPrev = e->lruPrev;
       }
       e->lruNext=0;
       e->lruPrev=0;
       // delete from hashtbale
       if(e==hashtable[index]){
          hashtable[index] = e->tableNext;
          if(hashtable[index]){
            hashtable[index]->tablePrev = 0;
          }
       } else {
          e->tablePrev->tableNext = e->tableNext;
          if(e->tableNext){
             e->tableNext->tablePrev = e->tablePrev;
          }
       }
       e->tableNext = 0;
       e->tablePrev = 0;
       delete e;
     } else {
       usedSize = (usedSize + e->size) - oldSize;
     }
   }
   return true;
}


/*
2.13 putAtFront

puts an unused NativeCacheEntry to the front of the LRU list.

*/
void putAtFront(NativeCacheEntry* newEntry){
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

moves an entry already present in the lru-list to the top of that list.

*/
void bringToFront(NativeCacheEntry* entry){
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
bool check(){
  if(first==0 || last==0){
    if(first!=last){
      cerr << "inconsistence in lru list, first = " << (void*) first
           << " , last = " << (void*) last << endl;
      return false;
    }
    for(unsigned int i=0;i< tableSize;i++){
      if(hashtable[i]){
         cerr << "lru is empty, but hashtable[" << i
              << "] contains an element" << endl;
         return false;
      }
    }
    // empty cache
    return true;
  }
  // lru is not empty, check first and last element
  if(first->lruPrev){
    cerr << "lru: first has a predecessor" << endl;
    return false;
  }
  if(last->lruNext){
    cerr << "lru: last has a successor" << endl;
    return false;
  }
  // check whether ech element in lru is also an element in hashtable
  NativeCacheEntry* e = first;
  int lrucount = 0;
  size_t compSize(0);
  while(e){
    if(e->size > slotSize){
      cerr << "entry found having a size > slotSize" << endl;
      cerr << " slotSize = " << slotSize << " entry = " << (*e) << endl;
      return false;
    }
    compSize += e->size ;
    lrucount++;
    size_t index = e->hashValue(tableSize);
    if(!hashtable[index]){
      cerr << "element " << (*e) << " stored in lru but hashtable["
           << index << " is null" << endl;
      return  false;
    }
    NativeCacheEntry* e2 = hashtable[index];
    while(e2 && (*e)!=(*e2)){
      e2 = e2->tableNext;
    }
    if(!e2){
      cerr << "element " << (*e) << " stored in lru but not in hashtable["
           << index << "]" << endl;
      return false;
    }
    e = e->lruNext;
  }
  if(compSize!=usedSize){
    cerr << "difference in usedSize and stored size " << endl;
    cerr << "usedSiez = " << usedSize << ", stored Size = " << compSize << endl;
    return false;
  }

  // check hashtable
  int tablecount = 0;
  for(unsigned int i=0; i<tableSize;i++){
    if(hashtable[i]){
       if(hashtable[i]->tablePrev){
           cerr << " hashtable[" << i << " has a predecessor" << endl;
       }
       e = hashtable[i];
       while(e){
         tablecount++;
         if(e->hashValue(tableSize)!=i){
           cerr << "element << " << (*e) << " has hashvalue "
                << e->hashValue(tableSize) << " but is stored at position "
                << i << endl;
           return false;
         }

         if(e->tableNext){
             if(e->tableNext->tablePrev!=e){
                cerr << "error in tablelist found" << endl;
                return false;
             }
         }
         e = e->tableNext;
       }
    }
  }

  if(lrucount!=tablecount){
    cerr << "lrucount = " << lrucount << " #  tablecount = "
         << tablecount << endl;
    return false;
  }

  return true;
}

  private:
    size_t maxSize;         // maximum allocated memory
    size_t slotSize;        // size of a single slot
    size_t usedSize;        // currently used memory
    NativeCacheEntry** hashtable; // open hash table
    size_t tableSize;       // size of the hashtable
    NativeCacheEntry* first;      // pointer to the first entry for lru
    NativeCacheEntry* last;       // pointer to the last entry for lru

};


#else   // whole flob based approach


#include "AvlTree.h"
#include <set>

class NativeCacheEntry{

 public:
    FlobId  flobId;
    SmiSize size;
    char* mem;
    NativeCacheEntry* prev;
    NativeCacheEntry* next;
    bool changed;


    NativeCacheEntry(): flobId(0,0,0,true), size(0),mem(0),prev(0),
                        next(0),changed(true){}


    static void putAtFront(NativeCacheEntry* e,
                           NativeCacheEntry*& first,
                           NativeCacheEntry*& last){
       if(first==0){  // first element in the list
          assert(last==0);
          first = e;
          last = e;
          e->next = 0;
          e->prev = 0;
          return;
       }
       assert(last!=0);
       e->prev = 0;
       e->next = first;
       first->prev = e;
       first = e;
    }

    static void connect(NativeCacheEntry* e1, NativeCacheEntry* e2,
                        NativeCacheEntry*& first, NativeCacheEntry*& last){
      __TRACE_ENTER__;
       if(e1==0 && e2==0){ // connect nothing
          first = 0;
          last = 0;
          return;
       }

       if(e1==0){
         e2->prev = 0;
         first = e2;
         return;
       }
       if(e2==0){
         e1->next=0;
         last = e1;
         return;
       }
       e1->next = e2;
       e2->prev = e1;
    }

    NativeCacheEntry(const Flob& flob, const bool alloc = false):
       flobId(flob.id), size(flob.size),
       mem(0), prev(0), next(0), changed(false) {
       if(alloc){
          mem = (char*) malloc(size);
       }
    }



    bool operator==(const NativeCacheEntry& e) const{
      __TRACE_ENTER__;
     return flobId == e.flobId;
    }

    bool operator<(const NativeCacheEntry& e) const {
      __TRACE_ENTER__;
      return flobId < e.flobId;
    }

    bool operator>(const NativeCacheEntry& e) const{
      __TRACE_ENTER__;
      return flobId > e.flobId;
    }

    ostream& print(ostream& o) const{
       o <<  " [ FlobId = " << flobId << ", "
         <<  " size = " << size << ", "  ;
       if(mem){
         o << " mem : allocated ,";
       } else {
         o << " mem : NULL ,";
       }
       if(prev){
         o << "prev = " << prev->flobId <<", ";
       } else {
         o << "prev = NULL, ";
       }
       if(next){
         o << "next = " << next->flobId <<", ";
       } else {
         o << "next = NULL, ";
       }
       o << "changed = " << changed << "]";
       return o;
    }

};

ostream& operator<<(ostream& o, const NativeCacheEntry entry);


class NativeFlobCache{

  public:

/*
~Constructor~

Creates an empy cache with a given capacity.


*/

    NativeFlobCache(const size_t _maxSize): cache(),
                                            maxSize(_maxSize),
                                            size(0),
                                            first(0),
                                            last(0){
      __TRACE_ENTER__;
//      assert(check());
    }

/*
~getData~

Returns data stored in the cahce. If the flob in not in cache and
even the flob cannot be put into the cache, the result is zero.

*/

    bool getData(const Flob& flob,
                 char* dest,
                 const SmiSize offset,
                 const SmiSize size) {
      __TRACE_ENTER__;

      NativeCacheEntry* entry =  useFlob(flob);
      if(entry==0){ // entry not in cache and not cachable
         return  FlobManager::getInstance().getData(flob, dest,
                                            offset, size, true);
      }
      assert(offset + size <= entry->size);
      memcpy(dest, entry->mem + offset, size);
      return true;
    }

    ~NativeFlobCache(){
      __TRACE_ENTER__;
     // assert(check());
      clear();
   //   assert(check());
    }


/*
~clear~

Removes all entries from the cache.

*/
    void clear(const bool saveChanges = false ){
      __TRACE_ENTER__;
   //   assert(check());

      if(size > maxSize){
         cerr << "NativeFlobCache::clear(), size > maxSize detected" << endl;
         cerr << "size = " << size << "maxSize = " << maxSize << endl;
      }
      NativeCacheEntry* e = first;
      while(e){
        if(saveChanges){
           this->saveChanges(e);
        }
        free(e->mem);
        size -= e->size;
        e = e->next;
      }
      cache.Empty();
      first = 0;
      last = 0;
      if(size!=0){
        cerr << "FlobCache::clear() : size computation failed, remaining size:"
             << size << endl;
        size = 0;
      }
    }


/*
~erase~

Removes a given flob from the cache without
flushing changes to disk. If the flob was not cached,
the result will be false.

*/
 bool erase(const Flob& victim){
      __TRACE_ENTER__;
    NativeCacheEntry finder(victim);
    NativeCacheEntry* entry = (NativeCacheEntry*)cache.getMember(finder);
    if(entry==0){
      return false;
    }
    // delete from double linked list
    NativeCacheEntry::connect(entry->prev, entry->next, first, last);
    // give up memory
    free(entry->mem);
    entry->mem = 0;
    size -= entry->size;
    // delete from tree
    cache.remove(finder);
    return true;
 }

/*
~create~

Allocates memory for the (uncached) flob.

*/
  bool create(const Flob& flob){
     __TRACE_ENTER__;
     if((flob.size==0 ) || (flob.size>maxSize)){ // non-cachable sizes
        return false;
     }
     NativeCacheEntry* entry = (NativeCacheEntry*)
                                   cache.insert2(NativeCacheEntry(flob,true));
     NativeCacheEntry::putAtFront(entry,first,last);
     size += entry->size;
     if(size>maxSize){
       reduce();
     }
     return true;
  }


/*
~eraseFromCache~

Stores chached made only in cache to disk and erases the flob from cache.

*/
  bool eraseFromCache(const Flob& victim){
      __TRACE_ENTER__;
    return saveChanges(victim) && erase(victim);
  }


/*
~putData~

Stores new data of a flob.

*/
  bool putData(const Flob& dest,
               const char* buffer,
               const SmiSize& targetoffset,
               const SmiSize& length){
    __TRACE_ENTER__;
    assert(targetoffset + length <= dest.size);
    NativeCacheEntry* entry = useFlob(dest);
    if(entry==0){
       return FlobManager::getInstance().putData(dest,buffer,
                                   targetoffset, length, true);
    }
    // put data into memory representation
    memcpy(entry->mem+targetoffset, buffer, length);
    entry->changed = true;
    return true;
  }

/*
~resize~

resizes the given flob using the size stored in the argument.

*/
 bool resize(Flob& flob, SmiSize newSize){
    __TRACE_ENTER__;

    if(newSize > maxSize ){
      eraseFromCache(flob); // saveChanges and kill cache
      return FlobManager::getInstance().resize(flob, newSize,true);
    }
    if(newSize==0){
      erase(flob); // kill cache without saving changes
      flob.size = 0;
      return true;
    }

    NativeCacheEntry finder(flob);
    NativeCacheEntry* entry = (NativeCacheEntry*) cache.getMember(finder);
    if(entry==0){
       if(flob.size==0){
         flob.size = newSize;
         return create(flob);
       }
       flob.size = newSize;
       bool ok = useFlob(flob)!=0; // chache flob
       assert(ok);
       return true;
    }

    NativeCacheEntry::connect(entry->prev, entry->next, first, last);
    NativeCacheEntry::putAtFront(entry, first, last);
    if(entry->size == newSize){ // no change
      return true;
    }
    entry->mem = (char*)realloc(entry->mem, newSize);
    size = size +  - entry->size + newSize;
    entry->size = newSize; // store size
    flob.size = newSize;
    return true;
 }


  private:
    avltree::AVLTree<NativeCacheEntry> cache;
    size_t maxSize;
    size_t size;
    NativeCacheEntry* first;
    NativeCacheEntry* last;


/*
~check~

This is debugging function checking the cache for invalid state.

In the double linked list, each flobid must be unique.
The content of the avl tree and the list muts be the same.
If the chache is not empty, last and first cannot be null.
The size is the sum of all stored sizes.


*/
    bool check(){
       if(cache.IsEmpty()){
          if(first!=0){
             cout << "cache is empty, but first is not null !" << endl;
             return false;
          }
          if(last!=0){
             cout << "cache is empty, but last is not null !" << endl;
             return false;
          }
          return true;
       }
       if(first==0){
           cout << "cache is not empty, but first is null" << endl;
           return false;
       }
       if(last==0){
           cout << "cache is not empty, but last is null" << endl;
           return false;
       }
       NativeCacheEntry* e = first;
       int length = 0;
       int csize = 0;
       set<NativeCacheEntry> testset;
       while(e){
         length++;
         csize += e->size;
         if(testset.find(*e) != testset.end()){
            cout << "Element found twice in the list " << (*e) << endl;
            return false;
         }
         testset.insert(*e);
         if(e->next==0){
            if(last!=e){
              cout << "last does not point to the last list element" << endl;
              return false;
            }
         }
         e = e->next;
       }


       // list structure ok, check content with the avltree
       if(length!=cache.Size()){
         cout << "different number of elements in cache and list " << endl;
         cout << "listLength = " << length << endl;
         cout << "treeSize = " << cache.Size() << endl;
         return false;
       }

       if(csize!=(int)size){
          cout << "different sizes), computed : " << csize
               << " , stored : " << size << endl;
          return false;
       }

       set<NativeCacheEntry>::iterator it = testset.begin();
       while(it!=testset.end()){
         if(cache.getMember(*it) == 0){
            cout << "Element " << (*it)
                 << "stored in list but not in tree " << endl;
            return false;
         }
         it++;
       }
       return true;
    }



    NativeCacheEntry* useFlob(const Flob& flob){
      __TRACE_ENTER__;
      NativeCacheEntry finder(flob);
      NativeCacheEntry* entry1 = (NativeCacheEntry*) cache.getMember(finder);

      if(entry1!=0){
         // entry cached
         NativeCacheEntry::connect(entry1->prev, entry1->next, first, last);
         NativeCacheEntry::putAtFront(entry1, first, last);
         return entry1;
      }

      // flob not already stored
      if(flob.size>maxSize || flob.size==0){ // non-cachable sizes
        return 0;
      }

      // bring entry to chache
      NativeCacheEntry entry(flob);
      entry.changed = false;
      entry.mem = (char*) malloc(flob.size);
      FlobManager::getInstance().getData(flob, entry.mem,0, flob.size, true);
      NativeCacheEntry* stored = (NativeCacheEntry*) cache.insert2(entry);
      NativeCacheEntry::putAtFront(stored, first, last);
      size += flob.size;
      if(size > maxSize){
         reduce();
      }
      return stored;
    }

    bool saveChanges(const Flob& flob){
      __TRACE_ENTER__;
      NativeCacheEntry finder(flob);
      NativeCacheEntry* entry = (NativeCacheEntry*) cache.getMember(finder);
      if(entry==0){
         return true;
      }
      return saveChanges(entry);
    }

    bool saveChanges(NativeCacheEntry* entry){
      if(!entry->changed){
        return true;
      }
      Flob flob(entry->flobId, entry->size);
      bool res1 = FlobManager::getInstance().resize(flob, entry->size, true);
      bool res = FlobManager::getInstance().putData(flob,
                                                entry->mem,
                                                0,
                                                entry->size,
                                                true);
      if(res){
         entry->changed=false;
      }
      return res&&res1;
    }


    bool reduce(){
       __TRACE_ENTER__;
       while(!cache.IsEmpty() && size > maxSize){
          NativeCacheEntry* e = last;
          saveChanges(last);
          assert(e!=0);
          // remove from list
          last = e->prev;
          if(last==0){
             first = 0;
          }else {
             last->next = 0;
          }
          e->prev = 0;
          e->next = 0;
          free(e->mem);
          e->mem=0;
          size -= e->size;
          bool rm = cache.remove(*e);
          assert(rm);
       }
       return true;
    }



};



#endif




