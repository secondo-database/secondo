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



1 Aux Classes Header File

*/
#pragma once

#include "Operator.h"

#include <utility>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <memory>
#include <atomic>
#include "thread"
#include "condition_variable"
#include "Algebras/Relation-C++/RelationAlgebra.h"

namespace mthreaded {

class MThreadedSingleton {
   public:
   static void setCoresToUse(const size_t cores);

   static size_t getCoresToUse();

   private:
   static size_t cores_used;
};

class Buffer {
   public:
   virtual void appendTuple(Tuple* tuple) = 0;

   virtual Tuple* readTuple() = 0;

   virtual void closeWrite() = 0;

   virtual void openRead() = 0;

   Buffer() = default;

   ~Buffer() = default;
};

class FileBuffer : public Buffer {
   private:
   std::string fname;
   std::shared_ptr<std::ofstream> fileStreamOut;
   std::shared_ptr<std::ifstream> fileStreamIn;
   TupleType* tt;
   bool isEmpty;

   // fname from uuid
   std::string createFn();

   public:
   // append single tuple into a file
   void appendTuple(Tuple* tuple) override;

   // read single tuple
   Tuple* readTuple() override;

   // close writing connection
   void closeWrite() override;

   // open for read
   void openRead() override;

   bool empty();

   std::string getFname();

   // Constructor FileBuffer
   explicit FileBuffer(TupleType* _tt);

   // Destructor FileBuffer
   ~FileBuffer();


};

class MemoryBuffer : public Buffer {
   private:
   std::queue<Tuple*> tupleBuffer;
   size_t bufferSize;
   TupleType* tt;

   public:
   // append single tuple into a file
   void appendTuple(Tuple* tuple) override;

   // read single tuple
   Tuple* readTuple() override;

   void closeWrite() override {}

   void openRead() override {}

   explicit MemoryBuffer(TupleType* _tt);

   ~MemoryBuffer();

};


class SafeQueue {
   public:
   SafeQueue(size_t _n);

   ~SafeQueue() {}

   bool empty();

   // Add an element to the queue.
   void enqueue(Tuple* t);

   // Get the "front"-element.
   // If the queue is empty, wait till a element is avaiable.
   Tuple* dequeue();

   //bool getStreamOnMerge();
   //void setStreamOff();

   private:
   std::queue<Tuple*> q;
   bool stream;
   bool dataReadyQueue;
   mutable std::mutex m;
   std::condition_variable c;
   size_t n;
};

class HashTablePersist {
   public:
   HashTablePersist(size_t _bucketsNo, size_t _coreNoWorker,
                    size_t _maxMem, TupleType* _ttR, TupleType* _ttS,
                    std::pair<size_t, size_t> _joinAttr);

   ~HashTablePersist();

   void PushR(Tuple* tuple, size_t bucket);

   void PushS(Tuple* tuple, size_t bucket);

   Tuple* PullR(size_t bucket);

   Tuple* PullS(size_t bucket);

   void CloseWrite();

   size_t OpenRead(size_t bucket);

   void UseMemHashTable(size_t usedMem);

   void SetHashMod(size_t hashMod);

   void CalcS();

   std::shared_ptr<FileBuffer> GetOverflowS(size_t bucket);

   size_t GetOverflowBucketNo(size_t bucket);

   private:
   std::vector<std::shared_ptr<Buffer>> hashBucketsS;
   std::vector<std::shared_ptr<Buffer>> hashBucketsR;
   std::vector<std::shared_ptr<FileBuffer>> hashBucketsOverflowS;
   std::vector<size_t> sizeR;
   std::vector<size_t> sizeS;
   bool setSPersist;
   size_t lastMemBufferR;
   size_t lastMemBufferS;
   size_t hashMod;
   std::vector<size_t> overflowBucketNo;

   size_t bucketsNo;
   size_t coreNoWorker;
   const size_t maxMem;
   TupleType* ttR;
   TupleType* ttS;
   std::pair<size_t, size_t> joinAttr;
   size_t freeMem;
};


}