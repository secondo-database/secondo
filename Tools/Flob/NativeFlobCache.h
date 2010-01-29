
/*
----
This file is part of SECONDO.

Copyright (C) 2010, University in Hagen, Department of Computer Science,
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


*/

#include "AvlTree.h"
#include "FlobId.h"
#include "Flob.h"
#include "FlobManager.h"
#include <set>



#undef __TRACE_ENTER__
#undef __TRACE_LEAVE__

#define __TRACE_ENTER__
#define __TRACE_LEAVE__

 /*#define __TRACE_ENTER__ std::cerr << "Enter : " << \
        __PRETTY_FUNCTION__ << std::endl;
        
#define __TRACE_LEAVE__ std::cerr << "Leave : " << \
        __PRETTY_FUNCTION__ << "@" << __LINE__ << std::endl;
  */


class CacheEntry{

 public:
    FlobId  flobId;
    SmiSize size;
    char* mem;
    CacheEntry* prev;
    CacheEntry* next;
    bool changed;


    CacheEntry(): flobId(0,0,0), size(0),mem(0),prev(0),next(0),changed(true){}


    static void putAtFront(CacheEntry* e, 
                           CacheEntry*& first, 
                           CacheEntry*& last){
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

    static void connect(CacheEntry* e1, CacheEntry* e2, 
                        CacheEntry*& first, CacheEntry*& last){
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

    CacheEntry(const Flob& flob): flobId(flob.id), 
                                  size(flob.size),
                                  mem(0), 
                                  prev(0), 
                                  next(0), 
                                  changed(false) {}

  

    bool operator==(const CacheEntry& e) const{
      __TRACE_ENTER__;
     return flobId == e.flobId;
    }

    bool operator<(const CacheEntry& e) const {
      __TRACE_ENTER__;
      return flobId < e.flobId;
    }
    
    bool operator>(const CacheEntry& e) const{
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

ostream& operator<<(ostream& o, const CacheEntry entry);


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
    //  assert(check());

       CacheEntry* entry =  useFlob(flob); 
    //  assert(check());
       if(entry==0){
         return  false;
       }
       assert(offset + size <= entry->size);
       memcpy(dest, entry->mem + offset, size);
    //  assert(check());
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
    void clear(){
      __TRACE_ENTER__;
   //   assert(check());

      if(size > maxSize){
         cerr << "NativeFlobCache::clear(), size > maxSize detected" << endl;
         cerr << "size = " << size << "maxSize = " << maxSize << endl;
      }
      CacheEntry* e = first;
      while(e){
        free(e->mem);
        size -= e->size;
        e = e->next;
      }
      cache.Empty();
      first = 0;
      last = 0; 
      if(size!=0){
         cout << "Error in size computation, size =" << size << endl;
      }
      if(size!=0){
        cerr << "FlobCache::clear() : size computation failed, remaining size:"
             << size;
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
 //   assert(check());
    CacheEntry finder(victim);
    CacheEntry* entry = (CacheEntry*)cache.getMember(finder);
    if(entry==0){
   //   assert(check());
      return false;
    }
    // delete from double linked list
    CacheEntry::connect(entry->prev, entry->next, first, last);
    // give up memory
    free(entry->mem);
    entry->mem = 0;
    size -= entry->size;
    // delete from tree
    cache.remove(finder);
  //  assert(check());
    return true; 
 }

/*
~create~

Allocates memory for the (uncached) flob.

*/
  bool create(const Flob& flob){
     __TRACE_ENTER__;
 //   assert(check());
    return useFlob(flob)!=0;
  }


/*
~eraseFromCache~

Stores chached made only in cache to disk and erases the flob from cache.

*/
  bool eraseFromCache(const Flob& victim){
      __TRACE_ENTER__;
  //  assert(check());
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
  //  assert(check());
    assert(targetoffset + length <= dest.size);
    CacheEntry* entry = useFlob(dest);
    if(entry==0){
    //  assert(check());
      return false;
    }
    memcpy(entry->mem+targetoffset, buffer, length);
    entry->changed = true;
  //  assert(check());
    return true;
  }

/*
~resize~

resizes the given flob using the size stored in the argument.

*/
 bool resize(const Flob& flob){
    __TRACE_ENTER__;
   // assert(check());
    if(flob.size >= maxSize ){
      eraseFromCache(flob);
   //   assert(check());
      return false;
    }
    if(flob.size==0){
      erase(flob);
   //   assert(check());
      return false;
    }
    CacheEntry finder(flob);
    CacheEntry* entry = (CacheEntry*) cache.getMember(finder);
    if(entry==0){
    //  assert(check());
      return useFlob(flob);
    }
    CacheEntry::connect(entry->prev, entry->next, first, last);
    CacheEntry::putAtFront(entry, first, last);
    if(entry->size == flob.size){ // no change
   //   assert(check());
      return true;
    }
    entry->mem = (char*)realloc(entry->mem, flob.size);
    size = size + flob.size - entry->size; 
    entry->size = flob.size; // store size
   // assert(check());
    return true;
 } 

 




  private:
    avltree::AVLTree<CacheEntry> cache;
    size_t maxSize;
    size_t size;
    CacheEntry* first;
    CacheEntry* last;


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
       CacheEntry* e = first;
       int length = 0;
       int csize = 0;
       set<CacheEntry> testset;
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
       
       set<CacheEntry>::iterator it = testset.begin();
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



    CacheEntry* useFlob(const Flob& flob){
      __TRACE_ENTER__;
      CacheEntry finder(flob);
      CacheEntry* entry1 = (CacheEntry*) cache.getMember(finder);
      if(entry1!=0){
         assert(entry1->size = flob.size);
         CacheEntry::connect(entry1->prev, entry1->next, first, last);
         CacheEntry::putAtFront(entry1, first, last);
      //   assert(check());
         return entry1;
      }
      // flob not already stored
      if(flob.size>maxSize || flob.size==0){ // noncachable sizes
        return false;
      }
      CacheEntry entry(flob);
      entry.changed = false;
      entry.mem = (char*) malloc(flob.size);
      FlobManager::getInstance().getData(flob, entry.mem,0, flob.size, true);

      CacheEntry* stored = (CacheEntry*) cache.insert2(entry);
      CacheEntry::putAtFront(stored, first, last);
 
      size += flob.size;
      
    //  assert(check());
      if(size > maxSize){
         reduce();
      }
    //  assert(check());
      return stored;
       
    }

    bool saveChanges(const Flob& flob){
      __TRACE_ENTER__;
      CacheEntry finder(flob);
      CacheEntry* entry = (CacheEntry*) cache.getMember(finder);
      if(entry==0){
         return false;
      }
      assert(entry->size == flob.size);
      return saveChanges(entry);
    }

    bool saveChanges(CacheEntry* entry){
      if(!entry->changed){
        return true;
      }
      Flob flob(entry->flobId, entry->size);
      bool res = FlobManager::getInstance().putData(flob,
                                                entry->mem,
                                                0,
                                                entry->size,
                                                true);
      if(res){
         entry->changed=false;
      }
      return res;
    }


    bool reduce(){
       __TRACE_ENTER__;
   //    assert(check());
       while(!cache.IsEmpty() && size > maxSize){
          CacheEntry* e = last;
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
   //    assert(check());
       return true;
       
    }



};

