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
#ifndef BIGARRAY_H
#define BIGARRAY_H

#include "SecondoConfig.h"   // decides SECONDO_WIN32

#include <cstdint>       // uint64_t (the Win32 branch)
#include <cstring>       // strerror
#include <fstream>       // the "already exists" check in newInstance
#include <string>
#include <type_traits>   // the static_asserts below

#include <iostream>
using std::cout;
using std::endl;

#ifndef SECONDO_WIN32
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#else
#include <windows.h>
#endif


#include "SecondoException.h"





/*
1 Class ~BigArray~

This class provides functionality of an vector. Using ~append~
it may grow automatically. The class used for the template parameter
must provide a standard and a copy constructor. Furthermore the class
must be a compact class of fixed size, i.e., the class cannot have any
pointer structures.

The array may be larger than main memory, hence the name. It is backed by a
temporary file that is *mapped into the address space*, so the operating system
does the paging: reading an element is one dereference, the kernel's page cache
decides what stays resident -- sized by how much memory the machine actually has
-- and dirty pages are written back to the file under memory pressure. The
mapping grows with the array, and the file is unlinked the moment it is opened,
so it cannot outlive the process even if that process is killed.

*This class is not thread safe, and no longer tries to be.* It used to take a
recursive mutex on every access, which was necessary when a read mutated shared
state -- reading an element could evict a slot, touch the LRU and write a page
back. Reading is now a dereference and mutates nothing; only ~append~ can move
the mapping, by growing it.

That leaves ~append~ against a concurrent ~Get~, which is serialized one level
up instead: ~NestedList~ is the only user of this class, and it takes its own
recursive mutex in every method before touching a table (~First~, ~Rest~,
~IsAtom~, ...), so the lock here could never be reached uncontended by a thread
that was not already serialized. It cost two atomic operations per element
access on the hottest path in the system for nothing.

A future caller that uses a ~BigArray~ directly from several threads has to
provide that serialization itself.

*/

template<class T>
class BigArray{

static_assert(!std::is_pointer<T>::value,
   "Template parameter cannot be a pointer type");

// Elements are assigned over raw mapped storage that no constructor ever ran
// on. That is what the "compact class of fixed size" requirement above means;
// this states it in a way the compiler checks.
static_assert(std::is_trivially_copyable<T>::value,
   "Elements live in a memory mapping and are assigned over raw storage");

  public:

/*
This function returns a new instance of an BigArray. The
result must be destroyed by the caller.

~slotCacheSize~ is a number of elements, and is now only the *initial* capacity:
the array grows past it on demand, so it decides how often the mapping has to be
grown rather than how much may stay in memory. Sizing it generously costs
nothing but address space -- the file is sparse, so pages nobody touches take
neither memory nor disk.

*/
    static BigArray* newInstance(const std::string& filename,
                                 size_t slotCacheSize,
                          bool overwrite){
      if(!overwrite){
           std::ifstream in(filename.c_str(), std::ios::in);
           if(in.good()){
             in.close();
             throw SecondoException("File already exists");
           }
      }
      return new BigArray<T>(filename, slotCacheSize);
   }


/*
~Destructor~


*/

   ~BigArray(){
      closeStorage();   // which is also what deletes the file; see openStorage
   }

/*
~NoEntries~

Returns the number of elements within the array.

*/
    size_t NoEntries() {
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

Appends a slot for the caller to fill in, and returns its index.

*/
   size_t EmptySlot(){
      T t{};
      return  append(t);
   }

/*
~append~

Appends a new element at the end of the array.

*/
    size_t append(const T& value);


  private:
     std::string fname;      // file name of background storage
     size_t size;            // current number of elements
     char* base;             // the mapping, or 0 while there is none
     size_t mapped;          // its length in bytes
     size_t capacity;        // elements that fit without regrowing
     size_t pagesize;        // the *operating system's* page size

#ifndef SECONDO_WIN32
     int fd;                 // background storage
#else
     HANDLE hFile;           // background storage
     HANDLE hMap;            // its current file mapping object
#endif


/*
Constructor

~WinUnix::getPageSize~ is deliberately not used here: it answers with a fixed
4096 so that database files stay portable across machines, while a mapping has
to be told about the page size the machine really has (16384 on Apple Silicon).

*/
     BigArray(const std::string& _fname, size_t _initialEntries)
       : fname(_fname), size(0), base(0), mapped(0), capacity(0),
         pagesize(0)
#ifndef SECONDO_WIN32
       , fd(-1)
#else
       , hFile(INVALID_HANDLE_VALUE), hMap(0)
#endif
     {
        pagesize = systemPageSize();
        openStorage();
        try {
          // Sized up front so the common case never has to regrow.
          grow(_initialEntries > 0 ? _initialEntries : 1);
        } catch(...) {
          closeStorage();
          throw;
        }
     }

/*
~grow~

Makes room for at least ~entries~ elements, rounded up to whole pages.

*/
     void grow(const size_t entries){
        if(entries <= capacity){
          return;
        }
        size_t bytes = entries * sizeof(T);
        bytes = ((bytes + pagesize - 1) / pagesize) * pagesize;
        mapAtLeast(bytes);
        capacity = mapped / sizeof(T);
     }

     T* elements(){ return (T*) base; }


/*
The three operations that differ between platforms. Everything above is written
in terms of them, so both platforms run the same logic.

Each of them leaves the array usable if it fails: ~mapAtLeast~ in particular
puts the new mapping in place before taking the old one down, so a failed
growth throws with the data still mapped and reachable.

*/

#ifndef SECONDO_WIN32

     static size_t systemPageSize(){
        const long ps = ::sysconf(_SC_PAGESIZE);
        return ps > 0 ? (size_t) ps : 4096;
     }

     void openStorage(){
        fd = ::open(fname.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
        if(fd < 0){
          throw SecondoException("Could not open file " + fname + ": "
                                 + strerror(errno));
        }
        // Deleted straight away: the descriptor and the mapping keep the inode
        // alive for exactly as long as this array needs it, and the kernel
        // reclaims it however this process ends. Deleting in the destructor
        // instead meant that a crash, a kill or a power cut left the file
        // behind.
        ::unlink(fname.c_str());
     }

     void mapAtLeast(const size_t bytes){
        if(::ftruncate(fd, (off_t) bytes) != 0){
          throw SecondoException("Could not extend file " + fname + ": "
                                 + strerror(errno));
        }
        void* p;
#ifdef __linux__
        // Preferred where it exists: it leaves the old mapping in place if it
        // fails, where unmapping first would lose the array.
        p = base ? ::mremap(base, mapped, bytes, MREMAP_MAYMOVE)
                 : ::mmap(0, bytes, PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_NORESERVE, fd, 0);
#else
        p = ::mmap(0, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(p != MAP_FAILED && base){
          ::munmap(base, mapped);
        }
#endif
        if(p == MAP_FAILED){
          throw SecondoException("Could not map file " + fname + ": "
                                 + strerror(errno));
        }
        base = (char*) p;
        mapped = bytes;
     }

     void closeStorage(){
        if(base){
          ::munmap(base, mapped);
          base = 0;
          mapped = 0;
        }
        if(fd >= 0){
          ::close(fd);
          fd = -1;
        }
     }

#else

/*
The Windows equivalents. ~CreateFileMapping~ extends the file to the requested
size by itself, so there is no separate step for that, and there is no
~mremap~: growing means a second mapping object over the same file, which is
created and mapped before the old view is taken down.

FILE\_FLAG\_DELETE\_ON\_CLOSE is how the POSIX branch's ~unlink~ is spelled here:
the file goes away once the last handle closes, however the process ends. It
needs FILE\_SHARE\_DELETE to be allowed at all. FILE\_ATTRIBUTE\_TEMPORARY asks
Windows to avoid writing pages back while it has the memory to hold them, which
is what this file is for.

*/

     static size_t systemPageSize(){
        SYSTEM_INFO si;
        ::GetSystemInfo(&si);
        return si.dwPageSize > 0 ? (size_t) si.dwPageSize : 4096;
     }

     static std::string lastError(){
        return "error " + std::to_string((unsigned long) ::GetLastError());
     }

     void openStorage(){
        hFile = ::CreateFileA(fname.c_str(),
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE
                                | FILE_SHARE_DELETE,
                              0, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_TEMPORARY
                                | FILE_FLAG_DELETE_ON_CLOSE, 0);
        if(hFile == INVALID_HANDLE_VALUE){
          throw SecondoException("Could not open file " + fname + ": "
                                 + lastError());
        }
     }

     void mapAtLeast(const size_t bytes){
        const uint64_t want = (uint64_t) bytes;
        HANDLE newMap = ::CreateFileMappingA(hFile, 0, PAGE_READWRITE,
                                             (DWORD) (want >> 32),
                                             (DWORD) (want & 0xFFFFFFFFull),
                                             0);
        if(newMap == 0){
          throw SecondoException("Could not extend file " + fname + ": "
                                 + lastError());
        }
        void* p = ::MapViewOfFile(newMap, FILE_MAP_ALL_ACCESS, 0, 0,
                                  (SIZE_T) bytes);
        if(p == 0){
          const std::string why = lastError();
          ::CloseHandle(newMap);
          throw SecondoException("Could not map file " + fname + ": " + why);
        }
        // Only now that the new view exists is the old one given up.
        if(base){
          ::UnmapViewOfFile(base);
        }
        if(hMap){
          ::CloseHandle(hMap);
        }
        hMap = newMap;
        base = (char*) p;
        mapped = bytes;
     }

     void closeStorage(){
        if(base){
          ::UnmapViewOfFile(base);
          base = 0;
          mapped = 0;
        }
        if(hMap){
          ::CloseHandle(hMap);
          hMap = 0;
        }
        if(hFile != INVALID_HANDLE_VALUE){
          ::CloseHandle(hFile);
          hFile = INVALID_HANDLE_VALUE;
        }
     }

#endif

};


template<class T>
size_t BigArray<T>::append(const T& value){
    if(size == capacity){
       grow(capacity * 2);
    }
    elements()[size] = value;
    size++;
    return size;
}

template<class T>
bool BigArray<T>::Get(size_t index1, T& result){
    size_t index = index1;
   if((index > size) || (index < 1)){
     throw SecondoException("get: array index out of bounds: index = "
                            + std::to_string(index) + ", size = "
                            + std::to_string(size));
   }
   result = elements()[index-1];
   return true;
}


template<class T>
void BigArray<T>::Put(size_t index1, const T& value){
  size_t index = index1;
   if((index < 1) || (index > size)){
     throw SecondoException("put: array index out of bounds: index = "
                            + std::to_string(index) + ", size = "
                            + std::to_string(size));
   }
   elements()[index-1] = value;
}


#endif
