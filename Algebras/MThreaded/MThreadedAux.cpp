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

size_t MThreadedSingleton::cores_used = 3;

// FileBufferRW

FileBufferRW::FileBufferRW(TupleType* _tt) : tt(_tt) {
   fname = createFn() + ".tmp";
   isEmpty = true;
}

FileBufferRW::~FileBufferRW() {
   //fileStreamInOut->close();
   std::remove(fname.c_str());
}

std::string FileBufferRW::createFn() {
   boost::uuids::random_generator generator;
   const boost::uuids::uuid uuid = generator();
   return boost::uuids::to_string(uuid);
}

void FileBufferRW::appendTuple(Tuple* tuple) {
   if (isEmpty) {
      fileStreamInOut = make_shared<std::fstream>(fname.c_str(),
                                                  std::ios::binary |
                                                  std::ios::trunc);
      //cout << "make file" << endl;

   }
   size_t coreSize;
   size_t extensionSize;
   size_t flobSize;
   size_t blocksize = tuple->GetBlockSize(coreSize, extensionSize,
                                          flobSize);
   // allocate buffer and write flob into it
   char* buffer = new char[blocksize];;
   {
      //std::lock_guard<mutex> lock(mthreadedGlobal::mutexWrite_);
      tuple->WriteToBin(buffer, coreSize, extensionSize, flobSize);
   }
   uint32_t tsize = blocksize;
   fileStreamInOut->write((char*) &tsize, sizeof(uint32_t));
   fileStreamInOut->write(buffer, tsize);
   delete[] buffer;
   tuple->DeleteIfAllowed();
   isEmpty = false;
}

Tuple* FileBufferRW::readTuple() {
   if (!fileStreamInOut) {
      cout << "no filestream" << endl;
      return nullptr;
   }
   if (fileStreamInOut->eof()) {
      cout << "EOF" << endl;
      return nullptr;
   }
   // size of the next tuple
   uint32_t size;
   fileStreamInOut->read((char*) &size, sizeof(uint32_t));
   if (size == 0) {
      return nullptr;
   }
   char* buffer = new char[size];
   fileStreamInOut->read(buffer, size);
   if (!fileStreamInOut->good()) {
      delete[] buffer;
      cout << "error during reading file " << endl;
      cout << "position " << fileStreamInOut->tellg() << endl;
      return nullptr; // error
   }
   //Tuple* res = nullptr;
   Tuple* res = new Tuple(tt);;
   res->ReadFromBin(0, buffer);
   delete[] buffer;

   return res;
}

void FileBufferRW::closeWrite() {

}

void FileBufferRW::openRead() {
}

bool FileBufferRW::empty() const {
   return isEmpty;
}

string FileBufferRW::getFname() const {
   return fname;
}


// FileBuffer

FileBuffer::FileBuffer(TupleType* _tt) : tt(_tt) {
   isEmpty = true;
   it = nullptr;
}

FileBuffer::~FileBuffer() {
   //fileStreamIn->close();
   //cout << "destroy F-Buffer" << endl;
   delete it;
}

void FileBuffer::appendTuple(Tuple* tuple) {
   if (isEmpty) {
      tupleBuffer = std::make_shared<TupleFile>(tt, PAGESIZE);
   }
   tupleBuffer->Append(tuple);
   tuple->DeleteIfAllowed();
   isEmpty = false;
}

Tuple* FileBuffer::readTuple() {
   Tuple* val = it->GetNextTuple();
   if (val == 0) {
      val = nullptr;
      //cout << "endReadBuff" << endl;
   }
   return val;
}

void FileBuffer::closeWrite() {
   //cout << "tbCWrite" << endl;
   //tupleBuffer->Close();
}

void FileBuffer::openRead() {
   //cout << "tbOpenRead" << endl;
   delete it;
   it = tupleBuffer->MakeScan();
}

bool FileBuffer::empty() const {
   return isEmpty;
}

// append single tuple into a file
void MemoryBuffer::appendTuple(Tuple* tuple) {
   tupleBuffer.push(tuple);
   isEmpty = false;
}

// read single tuple
Tuple* MemoryBuffer::readTuple() {
   if (tupleBuffer.empty()) {
      return nullptr;
   } else {
      Tuple* val = tupleBuffer.front();
      tupleBuffer.pop();
      val->IncReference();
      return val;
   }
}

bool MemoryBuffer::empty() const {
   return isEmpty;
}

MemoryBuffer::MemoryBuffer(TupleType* _tt) : tt(_tt) {
   isEmpty = true;
}

MemoryBuffer::~MemoryBuffer() {}

MultiBuffer::MultiBuffer(TupleType* _tt, const size_t _bufferSize) :
        tt(_tt), bufferSize(_bufferSize) {
   memoryBuffer = make_shared<MemoryBuffer>(tt);
   overflowBuffer = nullptr;
   bufferCounter = bufferSize;
   isEmpty = true;
   overflow = false;
}

MultiBuffer::~MultiBuffer() {
}

void MultiBuffer::appendTuple(Tuple* tuple) {
   assert(tuple != nullptr);
   if (bufferCounter > bufferSize) {
      if (!overflow) {
         overflowBuffer = make_shared<FileBuffer>(tt);
      }
      overflowBuffer->appendTuple(tuple);
      overflow = true;
   } else {
      bufferCounter -= tuple->GetSize() + sizeof(void*);
      memoryBuffer->appendTuple(tuple);
      isEmpty = false;
   }
}

Tuple* MultiBuffer::readTuple() {
   Tuple* tuple = memoryBuffer->readTuple();
   if (tuple != nullptr) {
      tuple->DeleteIfAllowed();
      return tuple;
   }
   if (overflow) {
      tuple = overflowBuffer->readTuple();
   }
   return tuple;
}

void MultiBuffer::closeWrite() {
   if (overflow) {
      overflowBuffer->closeWrite();
   }
}

void MultiBuffer::openRead() {
   if (overflow) {
      overflowBuffer->openRead();
   }
}

bool MultiBuffer::empty() const {
   return isEmpty;
};


SafeQueuePersistent::SafeQueuePersistent(const size_t _bufferSize,
                                         TupleType* _tt)
        : bufferSize(_bufferSize), tt(_tt) {
   bufferCounter = bufferSize;
   dataReadyQueue = false;
   overflow = false;
   it = nullptr;
   tupleBuffer = nullptr;
}

SafeQueuePersistent::~SafeQueuePersistent() {
   delete it;
};

bool SafeQueuePersistent::empty() const {
   return q.empty();
}

// Add an element to the queue.
void SafeQueuePersistent::enqueue(Tuple* t) {
   std::lock_guard<std::mutex> lock(m);
   if (bufferCounter <= bufferSize) {
      if (t != nullptr) {
         bufferCounter -= t->GetMemSize() + sizeof(void*);
      }
      if (bufferCounter > bufferSize) {
         tupleBuffer = std::make_shared<TupleFile>(tt, PAGESIZE);
         tupleBuffer->Open();
      }
      q.push(t);
      dataReadyQueue = true;
      c.notify_one();
   } else {
      if (t != nullptr) {
         tupleBuffer->Append(t);
         t->DeleteIfAllowed();
      } else {
         tupleBuffer->Close();
         //delete it;
         it = tupleBuffer->MakeScan();
         dataReadyQueue = true;
         overflow = true;
         c.notify_one();
      }
   }
}

// Get the "front"-element.
// If the queue is empty, wait till a element is avaiable.
Tuple* SafeQueuePersistent::dequeue() {
   std::unique_lock<std::mutex> lock(m);
   while (q.empty() && !overflow) {
      c.wait(lock, [&] { return dataReadyQueue; });
      dataReadyQueue = false;
   }
   Tuple* val = nullptr;
   if (!q.empty()) {
      val = q.front();
      q.pop();
   } else {
      val = it->GetNextTuple();
      if (val == 0) {
         val = nullptr;
      } else {
      }
   }
   return val;
}



