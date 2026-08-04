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

//paragraph [1] Title: [{\Large \bf ] [}]

[1] Module TestBigArray: the chunked storage behind the nested lists

~BigArray~ is the storage layer of every ~NestedList~, and it had no test of its
own -- the suite reaches it only through ~tnestedlist~, which never grows an
array far enough to leave its first chunk.

The property under test is the one the class is built around: *a chunk, once
mapped, is never moved or unmapped while the array lives*. Everything else in
the system depends on it. A reader dereferences an element while another thread
appends, so an implementation that grew by relocating -- as this one did until
the storage was chunked -- can hand a reader an address that no longer exists.
Values reading back correctly is not evidence either way, because a relocating
implementation copies them; the addresses have to be checked directly, which is
what ~ChunkCount~ and ~chunkBase~ are for.

The Windows half of ~MappedChunkFile~ cannot be run from here -- nothing in
this tree builds for Windows. It is type-checked instead, by compiling
Tools/BigArray/BigArray.cpp with SECONDO\_WIN32 defined against a stub
windows.h declaring the exact Win32 surface it uses. That check is manual and
belongs in the commit message of whatever changes that branch.

*/

#include <atomic>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "SecondoConfig.h"   // decides SECONDO_WIN32

#ifndef SECONDO_WIN32
#include <sys/stat.h>
#endif

#include "../Tools/BigArray/BigArray.h"
#include "NestedList.h"
#include "CTestFrame.h"

using namespace std;

/*
A record shaped like the ones the nested lists store: trivially copyable, no
pointers, and big enough that a few million of them span many chunks.

*/
struct TestRecord {
  uint64_t id;
  uint64_t check;
  char     pad[8];
};

static TestRecord makeRecord(const uint64_t i) {
  TestRecord r;
  r.id = i;
  r.check = ~i;
  memset(r.pad, (int) (i & 0xFF), sizeof(r.pad));
  return r;
}

static bool recordIsValid(const TestRecord& r, const uint64_t i) {
  if(r.id != i || r.check != ~i){
    return false;
  }
  for(size_t k = 0; k < sizeof(r.pad); k++){
    if((unsigned char) r.pad[k] != (unsigned char) (i & 0xFF)){
      return false;
    }
  }
  return true;
}


class TestBigArray : public CTestFrame {

  public:

  TestBigArray(char x = '*') : CTestFrame(x) {}

/*
1 The invariant: chunks never move

Three million entries out of an array asked for a thousand, so the array grows
many times over. After every growth, every chunk address recorded so far must
still be the same address -- and the entries in it must still read back.

*/
  void TestChunksNeverMove() {

    const uint64_t entries = 3000000;
    BigArray<TestRecord>* a =
       BigArray<TestRecord>::newInstance("tbigarray_move", 1000, true);

    vector<const void*> seen;
    bool moved = false;
    size_t growths = 0;

    for(uint64_t i = 0; i < entries; i++){
      a->append(makeRecord(i));
      if(a->ChunkCount() > seen.size()){
        growths++;
        for(size_t c = 0; c < seen.size(); c++){
          if(seen[c] != a->chunkBase(c)){
            moved = true;
          }
        }
        while(seen.size() < a->ChunkCount()){
          seen.push_back(a->chunkBase(seen.size()));
        }
      }
    }

    cout << "*** " << entries << " entries in " << a->ChunkCount()
         << " chunks of " << a->ChunkElements() << " elements, "
         << growths << " growths" << endl;

    CHECK(a->ChunkCount() > 1, true);      // it really did leave chunk 0
    CHECK(moved, false);

    // Every entry still readable, in list order and then scattered.
    bool ok = true;
    for(uint64_t i = 0; i < entries && ok; i++){
      ok = recordIsValid((*a)[i+1], i);
    }
    CHECK(ok, true);

    ok = true;
    for(uint64_t i = 0; i < entries && ok; i += 7919){
      TestRecord r;
      a->Get(i+1, r);
      ok = recordIsValid(r, i);
    }
    CHECK(ok, true);

    // Overwrites land where they are read from, across chunk boundaries.
    ok = true;
    for(uint64_t i = 0; i < entries; i += 65535){
      a->Put(i+1, makeRecord(i + 1000000000ull));
    }
    for(uint64_t i = 0; i < entries && ok; i += 65535){
      ok = recordIsValid((*a)[i+1], i + 1000000000ull);
    }
    CHECK(ok, true);

    delete a;
  }

/*
2 Reaching the end of the chunk table

The limit is normally hundreds of millions of entries, so the test lowers it
rather than appending its way there. The message has to name the limit: it is
what tells an operator which of NodeMem, StringMem and TextMem to raise.

*/
  void TestChunkTableLimit() {

    BigArray<TestRecord>* a =
       BigArray<TestRecord>::newInstance("tbigarray_limit", 1, true, 2);

    const size_t limit = 2 * a->ChunkElements();
    bool threw = false;
    bool named = false;

    try {
      for(size_t i = 0; i <= limit; i++){
        a->append(makeRecord(i));
      }
    } catch(SecondoException& e){
      threw = true;
      named = e.msg().find(to_string(limit)) != string::npos;
      cout << "*** " << e.msg() << endl;
    }

    CHECK(threw, true);
    CHECK(named, true);
    CHECK(a->NoEntries() == limit, true);   // the array is intact and full

    // ... and still readable after the failed append.
    bool ok = true;
    for(size_t i = 0; i < limit && ok; i += 1021){
      ok = recordIsValid((*a)[i+1], i);
    }
    CHECK(ok, true);

    delete a;
  }

/*
3 Chunk offsets are legal on every platform

A chunk of $2^{16}$ elements spans a multiple of 64 kB whatever the element
size, which is what makes every chunk's file offset acceptable both to
~MapViewOfFile~ (64 kB allocation granularity) and to ~mmap~ under any page size
we build for, 16 kB on Apple Silicon included. It holds for the three record
types the nested lists actually instantiate, and the check is here so that a
later change to one of them, or to the minimum chunk size, cannot quietly break
the Windows and Apple Silicon builds that nothing else covers.

*/
  template<class T>
  bool chunkOffsetsAligned(const string& what) {
    BigArray<T>* a = BigArray<T>::newInstance("tbigarray_align", 1, true);
    const size_t bytes = a->ChunkElements() * sizeof(T);
    const bool ok = (bytes % 65536) == 0;
    cout << "*** " << what << ": sizeof = " << sizeof(T)
         << ", chunk = " << bytes << " bytes" << (ok ? "" : "  MISALIGNED")
         << endl;
    delete a;
    return ok;
  }

  void TestChunkAlignment() {
    CHECK(chunkOffsetsAligned<NodeRecord>("NodeRecord"), true);
    CHECK(chunkOffsetsAligned<StringRecord>("StringRecord"), true);
    CHECK(chunkOffsetsAligned<TextRecord>("TextRecord"), true);
    CHECK(chunkOffsetsAligned<TestRecord>("TestRecord"), true);
  }

/*
4 Bounds

Out of range throws rather than returning, and says so in the exception rather
than on stdout -- in a "-srv" process stdout is the monitor's console.

*/
  void TestBounds() {

    BigArray<TestRecord>* a =
       BigArray<TestRecord>::newInstance("tbigarray_bounds", 1, true);
    a->append(makeRecord(42));

    TestRecord r;
    CHECK(threwOnGet(a, 0), true);            // indices are 1 based
    CHECK(threwOnGet(a, 2), true);            // one past the end
    CHECK(threwOnPut(a, 0), true);
    CHECK(threwOnPut(a, 2), true);
    CHECK(a->Get(1, r) && recordIsValid(r, 42), true);
    CHECK(a->IsValid(1) && !a->IsValid(0) && !a->IsValid(2), true);

    delete a;
  }

  bool threwOnGet(BigArray<TestRecord>* a, const size_t index) {
    try { TestRecord r; a->Get(index, r); } catch(SecondoException&){
      return true;
    }
    return false;
  }

  bool threwOnPut(BigArray<TestRecord>* a, const size_t index) {
    try { a->Put(index, makeRecord(0)); } catch(SecondoException&){
      return true;
    }
    return false;
  }

/*
5 The file is gone the moment it is opened

An array that outlives its process leaves its backing file behind. Unlinking at
creation is what stops that; before it, dead processes left over a gigabyte of
TMP\_NESTED\_LIST\_ behind.

*/
  void TestFileIsUnlinked() {
#ifndef SECONDO_WIN32
    const string name = "tbigarray_unlinked";
    BigArray<TestRecord>* a =
       BigArray<TestRecord>::newInstance(name, 1000, true);
    a->append(makeRecord(1));

    struct stat st;
    CHECK(::stat(name.c_str(), &st) != 0, true);
    CHECK(recordIsValid((*a)[1], 1), true);   // still usable without a name

    delete a;
    CHECK(::stat(name.c_str(), &st) != 0, true);
#else
    cout << "*** FILE_FLAG_DELETE_ON_CLOSE, not checked here" << endl;
#endif
  }

/*
6 EmptySlot hands back initialized storage

The elements live in a mapping that no constructor ever ran on, and the records
the nested lists store have only an implicitly defined default constructor --
which initializes nothing. ~EmptySlot~ therefore has to value-initialize, or
every byte of a fresh slot is whatever was in the page.

*/
  void TestEmptySlotIsZeroed() {

    BigArray<StringRecord>* a =
       BigArray<StringRecord>::newInstance("tbigarray_empty", 1000, true);

    // Dirty the storage first, so that zeros cannot come from a fresh page.
    StringRecord dirty;
    memset(&dirty, 0xAB, sizeof(dirty));
    for(size_t i = 0; i < 1000; i++){
      a->append(dirty);
    }

    const size_t slot = a->EmptySlot();
    const StringRecord fresh = (*a)[slot];

    bool zeroed = true;
    const unsigned char* p = (const unsigned char*) &fresh;
    for(size_t i = 0; i < sizeof(StringRecord); i++){
      if(p[i] != 0){
        zeroed = false;
      }
    }
    CHECK(zeroed, true);

    delete a;
  }


/*
7 Many threads appending at once

The reason ~size~ is an atomic and a slot is claimed with ~fetch\_add~. Nothing
else in the suite reaches this: NestedList holds a recursive mutex across every
append, so its own concurrency tests serialize exactly the code under test here.

Each thread writes records carrying its own id, so a slot handed to two threads,
a lost increment, or an element written into the wrong chunk all show up as a
record that does not match its own id. The growth path is deliberately hit hard
-- the array starts at one chunk and ends at many, with every thread appending
across the boundaries.

*/
  void TestConcurrentAppend() {

    const unsigned threads = 8;
    const uint64_t perThread = 200000;

    BigArray<TestRecord>* a =
       BigArray<TestRecord>::newInstance("tbigarray_conc", 1, true);

    std::atomic<size_t> mismatches(0);
    std::vector<std::thread> pool;

    for(unsigned t = 0; t < threads; t++){
      pool.push_back(std::thread([&, t](){
        // The index comes back from append, so each thread can check its own
        // slots -- and only its own -- while the others are still writing.
        for(uint64_t i = 0; i < perThread; i++){
          const uint64_t v = ((uint64_t) t << 40) | i;
          const size_t slot = a->append(makeRecord(v));
          TestRecord r;
          a->Get(slot, r);
          if(!recordIsValid(r, v)){
            mismatches++;
          }
        }
      }));
    }
    for(auto& th : pool){
      th.join();
    }

    const size_t total = threads * perThread;
    cout << "*** " << threads << " threads x " << perThread << " appends -> "
         << a->NoEntries() << " entries in " << a->ChunkCount() << " chunks"
         << endl;

    CHECK(a->NoEntries() == total, true);   // no lost or double claim
    CHECK(mismatches.load() == 0, true);
    CHECK(a->ChunkCount() > 1, true);       // growth really was concurrent

    // Every slot holds exactly one well formed record, and every (thread, i)
    // pair appears exactly once -- which is what a doubly handed out index
    // would break.
    std::vector<uint64_t> seen(threads, 0);
    bool wellFormed = true;
    for(size_t k = 1; k <= total; k++){
      const TestRecord r = (*a)[k];
      const uint64_t t = r.id >> 40;
      if(t >= threads || !recordIsValid(r, r.id)){
        wellFormed = false;
        break;
      }
      seen[t]++;
    }
    CHECK(wellFormed, true);

    bool counts = true;
    for(unsigned t = 0; t < threads; t++){
      if(seen[t] != perThread){
        counts = false;
      }
    }
    CHECK(counts, true);

    delete a;
  }


  bool TestRun() {
    TestChunksNeverMove();
    TestChunkTableLimit();
    TestChunkAlignment();
    TestBounds();
    TestFileIsUnlinked();
    TestEmptySlotIsZeroed();
    TestConcurrentAppend();
    return true;
  }

}; // end of class TestBigArray


int
main() {

  TestBigArray test('*');

  test.TestCase("Chunked storage of the class BigArray");

  cout << endl;
  cout << "Internal used Sizes (bytes): " << endl;
  cout << NestedList::SizeOfStructs() << endl;

  test.TestRun();

  test.ShowErrors();

  exit( test.GetNumOfErrors() );
}
