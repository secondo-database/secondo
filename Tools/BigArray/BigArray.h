/*
----
This file is part of SECONDO.

Copyright (C) 2015,
Faculty of Mathematics and Computer Science,
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


//[$][\$]

*/

#include <iostream>
#include <fstream>
#include <exception>
#include <string>
#include <string.h> // memset
#include <assert.h>
#include <vector>
#include "WinUnix.h"

#ifdef THREAD_SAFE
#include <boost/thread.hpp>
#endif



#include "SecondoException.h"
#include "LRU.h"


#ifndef BIGARRAY_H
#define BIGARRAY_H

using std::cout;
using std::endl;




/*
1 Definition of ~CastFun~

This function is required to reconstruct virtual function pointers 
if classes a stored to a big array. 

*/

#ifndef CastFun
typedef void* (*CastFun)( void* );
#endif


inline static void* stdCast(void* arg){
  return arg;
}


/*
2 Auxiliary class ~SlotEntry~

This class is just a combination of value and 
a flag wether this thing was changed in the last time.

*/


template<class A>
class SlotEntry{
public:
   SlotEntry(): index(0), value(), changed(true){}

   SlotEntry(const size_t _index, const A& _value):index(_index), 
             value(_value), changed(true){}
   SlotEntry(const size_t _index,const A& _value, const bool _changed): 
             index(_index),value(_value), changed(_changed){}
   void set(const A& value){
      this->value = value;
      changed = true;
   }

   void set(const size_t index,const A& value){
      this->index = index;
      this->value = value;
      changed = true;
   }

   size_t index;
   A value;
   bool changed;
};


/*
3 Class ~BigArray~

This class provides functionality of an vector. Using ~append~
it may grow automatically. The class used for the template parameter
must provide a standard and a copy constructor. Furthermore the class
must be a compact class of fixed size, i.e., the class cannot have any
pointer structures. The cast function must be implemented if the used
template class used virtual functions.

*/

template<class T>
class BigArray{
  public:

/*
This function returns a new instance of an BigArray. The
result must be destroyed by the caller.

*/
    static BigArray* newInstance(const std::string& filename, 
                                 size_t slotCacheSize, 
                          bool overwrite, CastFun cast = stdCast){
      if(!overwrite){
           std::ifstream in(filename.c_str(), std::ios::in);
           if(in.good()){
             in.close();
             throw SecondoException("File already exists");
           }
      }
     std::fstream* out = new std::fstream(filename.c_str(), 
                               std::ios::in | std::ios::out | std::ios::trunc
                             | std::ios::binary);
      if(!out->good()){
          delete out;
          throw SecondoException("Could not open file " + filename);
      }
      return new BigArray<T>(out, filename,  slotCacheSize, cast);
   }


/*
~Destructor~


*/

   ~BigArray(){
      #ifdef THREAD_SAFE
         boost::lock_guard<boost::recursive_mutex> guard(mtx);
      #endif
      file->close(); 
      delete file;
      std::remove(fname.c_str());
      LRUEntry<size_t, char*>* victim;
      while( (victim = lru.deleteLast())){
           delete[] victim->value;
           delete victim;
      }


   }

/*
~NoEntries~

Returns the number of elements within the array.

*/
    size_t NoEntries() {
      #ifdef THREAD_SAFE
         boost::lock_guard<boost::recursive_mutex> guard(mtx);
      #endif
      return size;
    }

    bool IsValid(const size_t index) const{
      return (index > 0) && (index <= size); 
    }


/*
~get~

Retrieves an element from the array.

*/
    bool Get(const size_t index, T& result);


/*
~access~

Returns the element at a specified position.

*/
    T operator[](const size_t index) {
      #ifdef THREAD_SAFE
         boost::lock_guard<boost::recursive_mutex> guard(mtx);
      #endif
        T res;
        if(!Get(index,res)){
            throw SecondoException("Array index out of bounds");
        }
        return res;
    }


/*
~put~

Replaces an existing element.

*/
    void Put(const size_t index, const T& value);



/*
~EmptySlot~

*/

   size_t EmptySlot(){
      T t;
      return  append(t);
   }


/*
~append~

Appends a new element at the end of the array.

*/
    size_t append(const T& value);

    
  private:
     std::vector<SlotEntry<T> > cache; // cache
     size_t cacheSize;                 // maximum size of cache
     std::fstream* file;               // background storage
     std::string fname;                // file name of background storage
     size_t size;                      // current number of elements
     CastFun cast;                     // cast function
     const size_t pagesize;
     size_t entriesPerPage;
     LRU<size_t, char*> lru;
     size_t pagesInFile;


#ifdef THREAD_SAFE
   boost::recursive_mutex mtx;          // a mutex
#endif


/*
Constructor

*/
     BigArray(std::fstream* _out, const std::string& _fname, 
              size_t  _cacheSize, CastFun _cast): cache(),
         cacheSize(_cacheSize),file(_out), fname(_fname),
         size(0), cast(_cast), pagesize(WinUnix::getPageSize()*4), 
         entriesPerPage(0), lru(8), pagesInFile(0)
     {
        if(cacheSize<100){ // ensure a minimum cache size
           cacheSize = 100;
        }
        assert(sizeof(T) <= pagesize);
        entriesPerPage = pagesize / (sizeof(T));
        assert(entriesPerPage > 1);
        entriesPerPage--;
     }

     void retrieve(const size_t arrayIndex);

     void save(const size_t cacheIndex); 

     void bringToPageCache(const size_t pageNo, const size_t offset, 
                           const char* content, const size_t size);

     char* ensurePageInCache(const size_t pageNo);

     void use(size_t pageNo, char* page);


     size_t filesize(){
         file->seekg(0,std::ios_base::end);
         return file->tellg();
     }

};


template<class T>
size_t BigArray<T>::append(const T& value){
    #ifdef THREAD_SAFE
       boost::lock_guard<boost::recursive_mutex> guard(mtx);
    #endif
    // no problem, use cache
    if(cache.size() < cacheSize){
       cache.push_back(SlotEntry<T>(size,value));
       size++;
       return size;
    }
    size++;
    Put(size, value);
    return size;
}

template<class T>
bool BigArray<T>::Get(size_t index, T& result){
    #ifdef THREAD_SAFE
       boost::lock_guard<boost::recursive_mutex> guard(mtx);
    #endif
   if((index > size) || (index < 1)){
     cout << "index = " << index << ", size = " << size << endl;
     throw SecondoException("get: array index out of bounds");
   }
   index--;
   retrieve(index);
   size_t vindex = index % cacheSize;
   result =  cache[vindex].value;
   return true;
}



template<class T>
void BigArray<T>::Put(size_t index, const T& value){
  #ifdef THREAD_SAFE
     boost::lock_guard<boost::recursive_mutex> guard(mtx);
  #endif
   if((index < 1) || (index > size)){
     cout << "index = " << index << ", size = " << size << endl;
     throw SecondoException("put: array index out of bounds");
   }
   index--;
   size_t vindex = index % cacheSize;

   if(cache[vindex].index == index){
     // overwrite the value in cache
      cache[vindex].value = value;
      cache[vindex].changed = true;
      return;
   }
   save(vindex);
   cache[vindex] = SlotEntry<T>(index,value);
}


template<class T>
void BigArray<T>::save(size_t cacheIndex){
   if(!cache[cacheIndex].changed){
     // already in the same form on disk
     return;
   }

   size_t arrayIndex = cache[cacheIndex].index;
   T arrayEntry = cache[cacheIndex].value;
   size_t pageNo = arrayIndex / entriesPerPage;
   size_t posOnPage = arrayIndex % entriesPerPage;
   size_t offset  = posOnPage * sizeof(T);
   bringToPageCache(pageNo, offset, (char*) &arrayEntry, sizeof(T));
}


template<class T>
void BigArray<T>::retrieve(size_t arrayIndex){
  size_t vindex = arrayIndex%cacheSize;
  if(cache[vindex].index==arrayIndex){
    // in main cache
    return;
  }
  // store the old value to disk
  save(vindex);

  size_t pageNo = arrayIndex / entriesPerPage;

  char* page = ensurePageInCache(pageNo);

  size_t posOnPage = arrayIndex % entriesPerPage;
  size_t offset  = posOnPage * sizeof(T);

  cache[vindex].changed = false;
  cache[vindex].index = arrayIndex;

  T* entry =(T*) (page + offset);
  cache[vindex] = SlotEntry<T>(arrayIndex,*entry);

}


template<class T>
void BigArray<T>::bringToPageCache(const size_t pageNo, const size_t offset, 
                                   const char* content, const size_t size){
    char** pagePtr = lru.get(pageNo);
    if(pagePtr != 0){
       memcpy( (*pagePtr) + offset, content, size);
       return;
    }

    // create new page having given pageno
    char* page;

    if(pagesInFile > pageNo){ // data already on disk, get from thereA
        page = new char[pagesize];
        file->seekg(pageNo*pagesize);
        file->read(page, pagesize);
    } else {
        page = new char[pagesize]; 
        // memset(page,0,pagesize); not necessary
    }
    memcpy(page+offset, content, size);

    // bring new page to cache
    use(pageNo, page);
}

template<class T>
char* BigArray<T>::ensurePageInCache(size_t pageNo){

   char** pagePtr = lru.get(pageNo);
   if(pagePtr){
      return *pagePtr;
   }

   char* page = new char[pagesize];
   if(pagesInFile > pageNo){ // data already on disk, get from there
      file->seekg(pageNo*pagesize);
      file->read(page, pagesize);
      assert(file->tellg() == (pageNo+1) * pagesize);
   } else {
      //memset(page,0,pagesize);
   }
   use(pageNo, page);
   return page;
}

template<class T> 
void BigArray<T>::use(size_t pageNo, char* page){

    LRUEntry<size_t, char*>* victim = lru.use(pageNo, page);


    if(victim){
       if(victim->key <=  pagesInFile){
          file->seekp(victim->key*pagesize);
       } else { // fill file with dummy pages

          file->seekp(pagesInFile*pagesize);
          char* emptypage = new char[pagesize];
          memset(emptypage,0,pagesize);
          while(pagesInFile < victim->key){
             file->write(emptypage, pagesize);
             pagesInFile++;
          }
          delete[] emptypage;
       }
       // write content of victim to file
       file->write(victim->value,pagesize);

       pagesInFile = std::max(victim->key + 1, pagesInFile);


       delete[] victim->value;
       delete victim;
    }




}




#endif


