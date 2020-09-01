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

const size_t PAGESIZE = WinUnix::getPageSize();

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

   virtual bool empty() const = 0;

   Buffer() = default;

   ~Buffer() = default;
};

class FileBufferRW : public Buffer {
   private:
   std::string fname;
   std::shared_ptr<std::fstream> fileStreamInOut;
   TupleType* tt;
   bool isEmpty;

   // fname from uuid
   static std::string createFn();

   public:
   // append single tuple into a file
   void appendTuple(Tuple* tuple) override;

   // read single tuple
   Tuple* readTuple() override;

   // close writing connection
   void closeWrite() override;

   // open for read
   void openRead() override;

   bool empty() const override;

   std::string getFname() const;

   // Constructor FileBuffer
   explicit FileBufferRW(TupleType* _tt);

   // Destructor FileBuffer
   ~FileBufferRW();


};

class FileBuffer : public Buffer {
   private:
   std::string fname;
   //std::shared_ptr<std::ofstream> fileStreamOut;
   //std::shared_ptr<std::ifstream> fileStreamIn;
   std::shared_ptr<TupleFile> tupleBuffer;
   TupleFileIterator* it;
   TupleType* tt;
   bool isEmpty;

   // fname from uuid
   //static std::string createFn();

   public:
   // Constructor FileBuffer
   explicit FileBuffer(TupleType* _tt);

   // Destructor FileBuffer
   ~FileBuffer();

   // append single tuple into a file
   void appendTuple(Tuple* tuple) override;

   // read single tuple
   Tuple* readTuple() override;

   // close writing connection
   void closeWrite() override;

   // open for read
   void openRead() override;

   bool empty() const override;
};

class MemoryBuffer : public Buffer {
   private:
   std::queue<Tuple*> tupleBuffer;
   size_t bufferSize;
   TupleType* tt;
   bool isEmpty;

   public:
   // append single tuple into a file
   void appendTuple(Tuple* tuple) override;

   // read single tuple
   Tuple* readTuple() override;

   void closeWrite() override {}

   void openRead() override {}

   bool empty() const override;

   explicit MemoryBuffer(TupleType* _tt);

   ~MemoryBuffer();

};

class MultiBuffer : public Buffer {
   private:
   std::shared_ptr<MemoryBuffer> memoryBuffer;
   std::shared_ptr<FileBuffer> overflowBuffer;
   TupleType* tt;
   const size_t bufferSize;
   size_t bufferCounter;
   bool overflow;
   bool isEmpty;

   public:
   // append single tuple into a file
   void appendTuple(Tuple* tuple) override;

   // read single tuple
   Tuple* readTuple() override;

   void closeWrite() override;

   void openRead() override;

   bool empty() const override;

   MultiBuffer(TupleType* _tt, const size_t _bufferSize);

   ~MultiBuffer();

};

template <typename T>
class SafeQueue {
   public:
   SafeQueue(size_t _n)
           : q(), m(), c(), n(_n) {
      dataReadyQueue = false;
   }

   ~SafeQueue() {
   };

   bool empty() const {
      return q.empty();
   }

   size_t size() const {
      return q.size();
   }

   // Add an element to the queue.
   void enqueue(T t) {
      std::lock_guard<std::mutex> lock(m);
      q.push(t);
      dataReadyQueue = true;
      c.notify_one();
   }

   // Get the "front"-element.
   // If the queue is empty, wait till a element is avaiable.
   T dequeue() {
      std::unique_lock<std::mutex> lock(m);
      while (q.empty()) {
         c.wait(lock, [&] { return dataReadyQueue; });
         dataReadyQueue = false;
      }
      T val = q.front();
      q.pop();
      return val;
   }


   //bool getStreamOnMerge();
   //void setStreamOff();

   private:
   std::queue<T> q;
   bool dataReadyQueue;
   mutable std::mutex m;
   std::condition_variable c;
   size_t n;
};


class SafeQueuePersistent {
   public:
   SafeQueuePersistent (const size_t _bufferSize, TupleType* _tt);

   ~SafeQueuePersistent ();

   bool empty() const;

   // Add an element to the queue.
   void enqueue(Tuple* t);

   // Get the "front"-element.
   // If the queue is empty, wait till a element is avaiable.
   Tuple* dequeue();


   //bool getStreamOnMerge();
   //void setStreamOff();

   private:
   const size_t bufferSize;
   TupleType* tt;
   std::shared_ptr<TupleFile> tupleBuffer;
   TupleFileIterator* it;
   //std::shared_ptr<FileBuffer> bufferPersist;
   std::queue<Tuple*> q;
   bool dataReadyQueue;
   mutable std::mutex m;
   std::condition_variable c;
   size_t bufferCounter;
   bool overflow;
};






}