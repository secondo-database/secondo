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

#include <atomic>
#include <cstddef>
#include <cstring>     // memset, for Truncate's poison mode
#include <mutex>
#include <string>
#include <type_traits>   // the static_asserts below

#include "SecondoConfig.h"
#include "SecondoException.h"


/*
1 Class ~MappedChunkFile~

The storage under a ~BigArray~: a temporary file, and the mappings of the
slices of it that hold the elements. It is deliberately not a template -- none
of what it does depends on the element type -- so all of the platform code
lives in BigArray.cpp and is compiled once, instead of being pulled into every
translation unit that includes a nested list. That is most of the tree.

Nothing here is a handle type from a platform header either: ~fd~ is the POSIX
descriptor and ~handle~ the Win32 HANDLE, held as an int and a void pointer, so
that this header stays free of <sys/mman.h> and of <windows.h> in particular.
~handle~ exists on Windows only -- SecondoConfig.h settles that from the
compiler's own macros, so every translation unit sees the same layout.

The file is unlinked the moment it is created, so it cannot outlive the process
however that process ends -- a crash, a kill and a power cut included. Which
also means the name is never used to find the file again, so the one a caller
gives is only a prefix: what gets created carries a suffix of its own, because
callers do share names and two of them creating a file at once must not land on
one inode. BigArray.cpp holds the details.

*/

class MappedChunkFile {

  public:

/*
The Win32 allocation granularity, which is what ~MapViewOfFile~ requires of an
offset. 64 kB is also a multiple of every page size we build for -- 16 kB on
Apple Silicon included -- so satisfying it satisfies ~mmap~ as well, and neither
platform needs arithmetic of its own. ~BigArray~ sizes its chunks by it.

*/
    static constexpr size_t GRANULARITY = ((size_t) 1) << 16;

/*
Creates a file whose name begins with ~filename~ and unlinks it. Throws
~SecondoException~ if it cannot. ~name~ reports what was actually created,
which is the prefix plus a suffix that makes it this instance's own.

*/
    explicit MappedChunkFile(const std::string& filename);
    ~MappedChunkFile();

    MappedChunkFile(const MappedChunkFile&) = delete;
    MappedChunkFile& operator=(const MappedChunkFile&) = delete;

/*
~map~ extends the file to cover ~offset, offset+bytes~ and maps that range.
Both arguments have to be multiples of ~GRANULARITY~. It throws rather than
returning 0, and leaves every mapping already handed out untouched -- that is
what lets ~BigArray~ grow while another thread is reading.

*/
    void* map(const size_t offset, const size_t bytes);

/*
~unmap~ releases one range obtained from ~map~.

*/
    void unmap(void* base, const size_t bytes);

    const std::string& name() const { return fname; }

  private:
    std::string fname;
    int         fd;       // POSIX descriptor; -1 on Windows
#ifdef SECONDO_WIN32
    void*       handle;   // Win32 HANDLE
#endif
};


/*
2 Class ~BigArray~

This class provides functionality of an vector. Using ~append~
it may grow automatically. The class used for the template parameter
must provide a standard and a copy constructor. Furthermore the class
must be a compact class of fixed size, i.e., the class cannot have any
pointer structures.

The array may be larger than main memory, hence the name. The operating system
does the paging: reading an element is one dereference, the kernel's page cache
decides what stays resident -- sized by how much memory the machine actually has
-- and dirty pages are written back to the file under memory pressure.

The storage is *chunked*: a fixed table of pointers, each entry one mapping of
one slice of the file. Growing appends a chunk; it never moves or unmaps an
existing one, so **the address of an element never changes once it has been
mapped**. That is what lets a reader dereference without holding a lock while
another thread appends -- the two touch different chunks, or different slots of
the same chunk. A single mapping cannot offer that: growing it means ~mremap~
(or, on Windows, a second view), which may relocate the whole array out from
under a concurrent reader.

Chunking is also what keeps this one implementation on every platform. Nothing
is reserved up front, so there is no dependence on a 64-bit address space, and
no need for ~MAP\_FIXED~, ~PROT\_NONE~ or the Win32 placeholder APIs -- each
platform contributes one call that maps a slice and one that unmaps it.

*Threading.* Reads need no lock at all, and neither does ~append~: a slot is
claimed with one ~fetch\_add~ on ~size~, and the rare growth that follows it is
the only thing that takes a lock. Two threads may therefore read freely, and
either may append while the other reads.

What is *not* provided, and is the caller's to arrange:

  * Two threads writing the *same* slot, or one reading a slot the other is
    writing, order it themselves. Nothing here can help with that -- it is
    ~Put~ against ~Put~ on one index, which is a question about the caller's
    data, not about this container.
  * A slot may be read once the ~append~ that produced it has *returned*.
    ~size~ counts slots that are claimed, and the element is written just after
    the claim, so for a moment a slot can be counted but not yet filled in.
    This is not a restriction in practice: the index is the return value, so
    until the append returns no one else can name the slot. The slot is always
    *mapped*, though -- room is made before it is claimed, so ~size~ never
    exceeds ~capacity~ -- which is what lets ~at~ dereference without checking
    that the chunk is there.

~NestedList~, the only user, holds a recursive mutex across all of this today,
which subsumes both.

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
*A note on the cost of ~std::atomic~ here.* The shared fields below are
~std::atomic~, which is what they are, and the orderings are written out at
every use rather than left to the default -- the accessors are the hottest path
in the system and it should be visible that they carry no barriers.

*/

/*
Chunk sizes are counted in elements and are always a power of two, so that
indexing is a shift and a mask.

The smallest of them follows from ~MappedChunkFile::GRANULARITY~, which is a
constraint on *bytes*: the smallest chunk spanning a whole number of those is
$GRANULARITY / \gcd(sizeof(T), GRANULARITY)$ elements -- 8192 for a 24 byte
~NodeRecord~, 1024 for a 64 byte ~TextRecord~. Expressing it this way rather
than as a flat count of elements keeps the many small lists small: Distributed2
gives every connection one, and their tables never grow at all.

*/
    static constexpr size_t gcdOf(const size_t a, const size_t b){
       return b == 0 ? a : gcdOf(b, a % b);
    }

    static constexpr size_t MIN_CHUNK_ELEMENTS =
                MappedChunkFile::GRANULARITY
              / gcdOf(sizeof(T), MappedChunkFile::GRANULARITY);
    static constexpr size_t MAX_CHUNK_ELEMENTS = ((size_t) 1) << 24;
    static constexpr size_t DEFAULT_MAX_CHUNKS = 4096;

static_assert((MIN_CHUNK_ELEMENTS * sizeof(T))
                 % MappedChunkFile::GRANULARITY == 0,
   "A chunk must span a multiple of the Win32 allocation granularity");

/*
This function returns a new instance of an BigArray. The
result must be destroyed by the caller.

~initialEntries~ is the capacity to map up front, and also decides the chunk
size -- so it controls how often the array has to grow rather than how much may
stay in memory. Sizing it generously costs nothing but address space: the file
is sparse, so pages nobody touches take neither memory nor disk.

~maxChunks~ bounds the array at ~maxChunks~ chunks. It exists to be lowered by
the unit test, which would otherwise have to append hundreds of millions of
elements to reach the limit.

*/
    static BigArray* newInstance(const std::string& filename,
                                 size_t initialEntries,
                                 size_t maxChunks = DEFAULT_MAX_CHUNKS){
      return new BigArray<T>(filename, initialEntries, maxChunks);
   }


/*
~Destructor~


*/

   ~BigArray(){
      const size_t mapped = ChunkCount();
      for(size_t i = 0; i < mapped; i++){
        storage.unmap(chunks[i].load(std::memory_order_relaxed), chunkBytes);
      }
      delete[] chunks;
   }

/*
~NoEntries~

Returns the number of elements within the array.

*/
    size_t NoEntries() const {
      return size.load(std::memory_order_relaxed);
    }

    bool IsValid(const size_t index) const{
      return (index > 0) && (index <= size.load(std::memory_order_relaxed));
    }

/*
~Truncate~

Gives back every slot above ~n~, so the next ~append~ reuses them. Chunks stay
mapped -- the invariant this class is built around is that a chunk never moves
once published, and reuse rather than unmapping is the point.

This is the *only* way anything is ever reclaimed here: ~append~ is the sole
allocator and it only grows. Truncation is therefore sound exactly when the
caller knows nothing outside the surviving region refers to the slots above
~n~, which no general caller can know -- see ~NestedList::release~, which is
the one intended user and carries that contract.

~poison~ overwrites the released slots first, so a stale reference to one is a
recognisable value rather than the data that happened to be there. Off by
default: it costs a full pass over the released region.

*/
    void Truncate(const size_t n, const bool poison = false){
      const size_t old = size.load(std::memory_order_relaxed);
      if(n >= old){
        return;                       // nothing to give back
      }
      if(poison){
        T dead;
        std::memset((void*) &dead, 0xFF, sizeof(T));
        for(size_t i = n; i < old; i++){
          at(i) = dead;
        }
      }
      size.store(n, std::memory_order_relaxed);
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


/*
The three below report on the storage layout. They exist so that the unit test
can check the invariant this class is built around -- that a chunk, once
mapped, is never moved -- rather than infer it from values reading back
correctly.

*/
    size_t ChunkCount() const {
       return nChunks.load(std::memory_order_relaxed);
    }
    size_t ChunkElements() const { return chunkElements; }
    const void* chunkBase(const size_t n) const {
       return n < ChunkCount()
                ? (const void*) chunks[n].load(std::memory_order_relaxed) : 0;
    }


  private:
     MappedChunkFile storage;      // the file and its mappings

     std::atomic<size_t> size;     // slots claimed -- see the note on threading
     std::atomic<size_t> capacity; // elements the mapped chunks hold
     std::atomic<T*>* chunks;      // the chunk table; allocated once, never
                                   // moved, and each entry written once
     std::atomic<size_t> nChunks;  // entries of it in use

     size_t maxChunks;             // the table's length; fixed at construction
     std::mutex growth;            // taken only to add a chunk

     // Fixed by the constructor before anything can reach the object, so these
     // are read without synchronization.
     size_t chunkElements;         // elements per chunk, a power of two
     size_t chunkBytes;            // its size in bytes
     unsigned shift;               // log2(chunkElements)
     size_t mask;                  // chunkElements - 1


/*
Constructor

*/
     BigArray(const std::string& _fname, size_t _initialEntries,
              size_t _maxChunks)
       : storage(_fname), size(0), capacity(0), chunks(0), nChunks(0),
         maxChunks(_maxChunks > 0 ? _maxChunks : 1),
         chunkElements(0), chunkBytes(0), shift(0), mask(0)
     {
        chooseChunkSize(_initialEntries);
        chunks = new std::atomic<T*>[maxChunks]();
        try {
          // Mapped up front so the common case never has to grow.
          ensureCapacity(_initialEntries > 0 ? _initialEntries : 1);
        } catch(...) {
          const size_t mapped = ChunkCount();
          for(size_t i = 0; i < mapped; i++){
            storage.unmap(chunks[i].load(std::memory_order_relaxed),
                          chunkBytes);
          }
          delete[] chunks;
          throw;
        }
     }

/*
~chooseChunkSize~

A chunk holds an eighth of what the caller asked for, so an array that stays
within its initial size is eight mappings rather than one, and one that grows
does so in steps proportional to its own scale. Both bounds matter: the lower
one is the granularity rule described above, the upper one keeps a single
mapping from becoming unreasonably large for an array that only just crossed a
threshold.

*/
     void chooseChunkSize(const size_t initialEntries){
        size_t want = initialEntries / 8;
        if(want < MIN_CHUNK_ELEMENTS){
          want = MIN_CHUNK_ELEMENTS;
        }
        chunkElements = MIN_CHUNK_ELEMENTS;
        shift = 0;
        while((((size_t) 1) << shift) < chunkElements){
          shift++;
        }
        while(chunkElements < want && chunkElements < MAX_CHUNK_ELEMENTS){
          chunkElements <<= 1;
          shift++;
        }
        mask = chunkElements - 1;
        chunkBytes = chunkElements * sizeof(T);
     }

/*
~ensureCapacity~

Maps chunks until at least ~entries~ elements fit. This is the one path that
takes a lock, and it runs once per chunk -- once per 48 MiB of nodes at the
kernel's configured size -- rather than once per element. The check is repeated
under the lock, so several appenders that find the array full at the same moment
add one chunk between them and not one each.

*/
     void ensureCapacity(const size_t entries){
        std::lock_guard<std::mutex> guard(growth);
        // Relaxed: under the lock there is no other writer to race with.
        while(capacity.load(std::memory_order_relaxed) < entries){
          addChunk();
        }
     }

/*
~addChunk~

Maps one more slice of the file. Called with ~growth~ held.

If this throws, every chunk mapped so far is still mapped and still readable --
nothing is taken down in order to put something else up, which is the property
the whole design rests on.

The store of the chunk pointer is a *release*, and ~at~ reads it with an
*acquire*: that pair, and not the value of ~size~, is what makes a chunk another
thread has just mapped safe to dereference. Keeping the edge local to the
pointer means the orderings on ~size~ can stay relaxed, where they are on the
hottest path in the system.

*/
     void addChunk(){
        const size_t n = nChunks.load(std::memory_order_relaxed);
        if(n == maxChunks){
          throw SecondoException("Array " + storage.name()
                                 + " reached its maximum of "
                                 + std::to_string(maxChunks * chunkElements)
                                 + " entries");
        }
        T* const chunk = (T*) storage.map(n * chunkBytes, chunkBytes);
        chunks[n].store(chunk, std::memory_order_release);
        nChunks.store(n + 1, std::memory_order_relaxed);
        capacity.store((n + 1) * chunkElements, std::memory_order_release);
     }

/*
~at~

Locates an element. ~index~ counts from 0 here, unlike the public interface.

There is deliberately no check that the chunk is there. ~append~ makes room
before it claims a slot, so ~size~ never exceeds ~capacity~, and every caller
reaches this through a bounds check against ~size~ -- an index that passes it
has a mapped chunk by construction. The *acquire* on the load is what makes
that chunk visible to a thread that did not map it.

This is the hottest line in the system: it runs once per node visit, and
~query roads~ visits tens of millions of nodes several times over. A check
here that can only fire for an index the caller was never given cost about a
second of the query, measured, and bought nothing that the invariant does not
already give.

*/
     T& at(const size_t index){
        return chunks[index >> shift].load(std::memory_order_acquire)
                                          [index & mask];
     }

};


/*
~append~

The slot is claimed with one compare-and-exchange, which is what lets two
threads append at once without either of them holding anything: the winner gets
the index, the loser is handed the value it lost to and tries the next one.

*Room is made before the slot is claimed*, not after, and that ordering is
load-bearing in two ways. It keeps ~size~ from ever exceeding ~capacity~, so a
bounds check that passes means the slot is mapped; and it means an append that
cannot grow -- the chunk table is full -- throws without having claimed
anything, leaving the array exactly as it was. Claiming first with a plain
~fetch\_add~ is cheaper by a hair and gets both of those wrong: the failed
append would leave ~size~ counting an entry that does not exist.

*/
template<class T>
size_t BigArray<T>::append(const T& value){
    size_t index = size.load(std::memory_order_relaxed);
    for(;;){
      if(index >= capacity.load(std::memory_order_acquire)){
        ensureCapacity(index + 1);            // throws if there is no room
      }
      if(size.compare_exchange_weak(index, index + 1,
                                    std::memory_order_relaxed,
                                    std::memory_order_relaxed)){
        break;
      }
      // Lost the race; `index` now holds what the winner left behind.
    }
    at(index) = value;
    return index + 1;
}

template<class T>
bool BigArray<T>::Get(size_t index1, T& result){
    size_t index = index1;
   const size_t entries = size.load(std::memory_order_relaxed);
   if((index > entries) || (index < 1)){
     throw SecondoException("get: array index out of bounds: index = "
                            + std::to_string(index) + ", size = "
                            + std::to_string(entries));
   }
   result = at(index-1);
   return true;
}


template<class T>
void BigArray<T>::Put(size_t index1, const T& value){
  size_t index = index1;
   const size_t entries = size.load(std::memory_order_relaxed);
   if((index < 1) || (index > entries)){
     throw SecondoException("put: array index out of bounds: index = "
                            + std::to_string(index) + ", size = "
                            + std::to_string(entries));
   }
   at(index-1) = value;
}


#endif
