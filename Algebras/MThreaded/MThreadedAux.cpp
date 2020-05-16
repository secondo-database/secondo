/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Basic Operators Header File

*/
#include "Operator.h"
#include "MThreadedAux.h"
#include "MThreadedAlgebra.h"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace mthreaded;
using namespace std;

namespace mthreadedGlobal {
mutex mutexRead_;
mutex mutexWrite_;
}


void MThreadedSingleton::setCoresToUse(const size_t cores) {
   cores_used = cores;
}

size_t MThreadedSingleton::getCoresToUse() {
   return cores_used;
}

size_t MThreadedSingleton::cores_used = 1;


// FileBuffer

FileBuffer::FileBuffer(TupleType* _tt) : tt(_tt) {
   fname = createFn() + ".tmp";
   cout << fname << endl;
   fileStreamOut = make_shared<std::ofstream>(fname.c_str(), std::ios::binary |
                                                             std::ios::trunc);
   isEmpty = true;
}

FileBuffer::~FileBuffer() {
   std::remove(fname.c_str());
}

std::string FileBuffer::createFn() {
   boost::uuids::random_generator generator;
   const boost::uuids::uuid uuid = generator();
   return boost::uuids::to_string(uuid);
}

void FileBuffer::appendTuple(Tuple* tuple) {
   size_t coreSize;
   size_t extensionSize;
   size_t flobSize;
   size_t blocksize = tuple->GetBlockSize(coreSize, extensionSize,
                                          flobSize);
   // allocate buffer and write flob into it
   char* buffer = new char[blocksize];;
   {
      std::lock_guard<mutex> lock(mthreadedGlobal::mutexWrite_);
      tuple->WriteToBin(buffer, coreSize, extensionSize, flobSize);
   }
   uint32_t tsize = blocksize;
   fileStreamOut->write((char*) &tsize, sizeof(uint32_t));
   fileStreamOut->write(buffer, tsize);
   delete[] buffer;
   tuple->DeleteIfAllowed();
   isEmpty = false;
}

Tuple* FileBuffer::readTuple() {
   if (!fileStreamIn) {
      cout << "no filestream" << endl;
      return nullptr;
   }
   if (fileStreamIn->eof()) {
      cout << "EOF" << endl;
      return nullptr;
   }
   // size of the next tuple
   uint32_t size;
   fileStreamIn->read((char*) &size, sizeof(uint32_t));
   if (size == 0) {
      return nullptr;
   }
   char* buffer = new char[size];
   fileStreamIn->read(buffer, size);
   if (!fileStreamIn->good()) {
      delete[] buffer;
      cout << "error during reading file " << endl;
      cout << "position " << fileStreamIn->tellg() << endl;
      return nullptr; // error
   }
   Tuple* res = nullptr;
   {
      std::lock_guard<mutex> lock(mthreadedGlobal::mutexRead_);
      res = new Tuple(tt);;
      res->ReadFromBin(0, buffer);
      delete[] buffer;
   }
   return res;
}

void FileBuffer::closeWrite() {
   fileStreamOut->close();
}

void FileBuffer::openRead() {
   fileStreamIn = make_shared<std::ifstream>(fname.c_str(), ios::binary);
}

bool FileBuffer::empty() {
   return isEmpty;
}

string FileBuffer::getFname() {
   return fname;
}

// append single tuple into a file
void MemoryBuffer::appendTuple(Tuple* tuple) {
   tupleBuffer.push(tuple);
}

// read single tuple
Tuple* MemoryBuffer::readTuple() {
   if (tupleBuffer.empty()) {
      return nullptr;
   } else {
      Tuple* val = tupleBuffer.front();
      tupleBuffer.pop();
      return val;
   }
}

MemoryBuffer::MemoryBuffer(TupleType* _tt) : tt(_tt) {}

MemoryBuffer::~MemoryBuffer() {}

//template<typename T>  SafeQueue<T>::SafeQueue(std::size_t _n)
//        : q(), m(), c(), n(_n) {
//   stream = true;
//   dataReadyQueue = false;
//}
//
//template<typename T>  bool SafeQueue<T>::empty() {
//   return q.empty();
//}
//
//template<typename T>  void SafeQueue<T>::enqueue(T t) {
//   std::lock_guard<std::mutex> lock(m);
//   q.push(t);
//   dataReadyQueue = true;
//   c.notify_one();
//}
//
//
//// Get the "front"-element.
//// If the queue is empty, wait till a element is available.
//template<typename T> T SafeQueue<T>::dequeue() {
//   std::unique_lock<std::mutex> lock(m);
//   while (q.empty()) {
//      c.wait(lock, [&] { return dataReadyQueue; });
//      dataReadyQueue = false;
//   }
////   if (stream) {
//   T val = q.front();
//   q.pop();
//   return val;
//}

//template class SafeQueue<Tuple>::SafeQueue<Tuple>;

//void SafeQueue::setStreamOff() {
//   std::unique_lock<std::mutex> lock(m);
//   cout << "off" << n << "--" << q.empty() << endl;
//   while (!q.empty()) {
//      c.wait(lock, [&] { return !dataReadyQueue; });
//   }
//   stream = false;
//   c.notify_one();
//   cout << "notify" << n << endl;
//}

HashTablePersist::HashTablePersist(size_t _bucketsNo, size_t _coreNoWorker,
                                   size_t _maxMem,
                                   TupleType* _ttR, TupleType* _ttS,
                                   pair<size_t, size_t> _joinAttr) :
        bucketsNo(_bucketsNo), coreNoWorker(_coreNoWorker), maxMem(_maxMem),
        ttR(_ttR), ttS(_ttS), joinAttr(_joinAttr) {
   hashBucketsR.reserve(bucketsNo - 1);
   hashBucketsS.reserve(bucketsNo - 1);
   for (size_t i = 0; i < bucketsNo - 1; ++i) {
      hashBucketsR.push_back(make_shared<MemoryBuffer>(ttR));
      hashBucketsS.push_back(make_shared<MemoryBuffer>(ttS));
      sizeR.push_back(0);
      sizeS.push_back(0);
      hashBucketsOverflowS.push_back(make_shared<FileBuffer>(ttS));
   }
   freeMem = maxMem;
   setSPersist = false;
   lastMemBufferR = bucketsNo - 2;
   lastMemBufferS = bucketsNo - 2;
}

HashTablePersist::~HashTablePersist() {
   hashBucketsR.clear();
   hashBucketsS.clear();
   hashBucketsOverflowS.clear();
}

void HashTablePersist::PushR(Tuple* tuple, size_t bucket) {
   size_t size = tuple->GetMemSize() + sizeof(void*);
   if (bucket <= lastMemBufferR) {
      freeMem -= size;
   }
   sizeR[bucket] += size;
   hashBucketsR[bucket]->appendTuple(tuple);
   // no memory
   if (freeMem > maxMem && lastMemBufferR <= bucket &&
       lastMemBufferR < bucketsNo) {
      cout << "write to disk R: " << lastMemBufferR << endl;
      if (!setSPersist) {
         for (size_t i = 0; i < bucketsNo - 1; ++i) {
            hashBucketsS[i] = make_shared<FileBuffer>(ttS);
         }
         setSPersist = true;
      }
      shared_ptr<FileBuffer> tempFileBuffer = make_shared<FileBuffer>(ttR);
      Tuple* tupleNext = hashBucketsR[lastMemBufferR]->readTuple();
      while (tupleNext != nullptr) {
         tempFileBuffer->appendTuple(tupleNext);
         tupleNext = hashBucketsR[lastMemBufferR]->readTuple();
      }
      hashBucketsR[lastMemBufferR] = move(tempFileBuffer);
      freeMem += sizeR[lastMemBufferR];
      --lastMemBufferR;
      if (lastMemBufferR > bucketsNo) {
         cout << "Overflow!!!!!!!!!!!!!!!!";
      }
   }
}

void HashTablePersist::PushS(Tuple* tuple, size_t bucket) {
   size_t size = tuple->GetMemSize() + sizeof(void*);
   if (!setSPersist && bucket <= lastMemBufferS) {
      freeMem -= size;
   }
   size_t partNo = tuple->HashValue(joinAttr.second) / coreNoWorker /
                   bucketsNo % hashMod;
   if (partNo >= overflowBucketNo[bucket]) {
      // save in overflow
      hashBucketsOverflowS[bucket]->appendTuple(tuple);
   } else {
      sizeS[bucket] += size;
      hashBucketsS[bucket]->appendTuple(tuple);
      if (!setSPersist && freeMem > maxMem && lastMemBufferS < bucketsNo) {
         //cout << "write to disk S: " << lastMemBufferS << endl;
         shared_ptr<FileBuffer> tempFileBuffer = make_shared<FileBuffer>(ttS);
         Tuple* tupleNext = hashBucketsS[lastMemBufferS]->readTuple();
         while (tupleNext != nullptr) {
            tempFileBuffer->appendTuple(tupleNext);
            tupleNext = hashBucketsS[lastMemBufferS]->readTuple();
         }
         hashBucketsS[lastMemBufferS] = move(tempFileBuffer);
         freeMem += sizeS[lastMemBufferS];
         --lastMemBufferS;
      }
   }
}

Tuple* HashTablePersist::PullR(size_t bucket) {
   Tuple* tuple = hashBucketsR[bucket]->readTuple();
   return tuple;
}

Tuple* HashTablePersist::PullS(size_t bucket) {
   Tuple* tuple = hashBucketsS[bucket]->readTuple();
   return tuple;
}

void HashTablePersist::UseMemHashTable(size_t usedMem) {
   freeMem -= usedMem;
}

void HashTablePersist::SetHashMod(size_t hashMod) {
   this->hashMod = hashMod;
}

void HashTablePersist::CalcS() {
   for (size_t i = 0; i < bucketsNo - 1; ++i) {
      if (sizeR[i] != 0) {
         overflowBucketNo.emplace_back(
                 (size_t) (maxMem * 0.8) / (sizeR[i] / hashMod));
      } else {
         overflowBucketNo.emplace_back();
      }
   }
}

shared_ptr<FileBuffer> HashTablePersist::GetOverflowS(size_t bucket) {
   return hashBucketsOverflowS[bucket];
}

size_t HashTablePersist::GetOverflowBucketNo(size_t bucket) {
   return overflowBucketNo[bucket];
}

void HashTablePersist::CloseWrite() {
   for (size_t i = 0; i < bucketsNo - 1; ++i) {
      hashBucketsR[i]->closeWrite();
      hashBucketsS[i]->closeWrite();
      hashBucketsOverflowS[i]->closeWrite();
   }
}

size_t HashTablePersist::OpenRead(size_t bucket) {
   hashBucketsR[bucket]->openRead();
   hashBucketsS[bucket]->openRead();
   return overflowBucketNo[bucket];
}

