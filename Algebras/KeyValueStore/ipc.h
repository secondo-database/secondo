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


September 2015 - Nico Kaufmann: IPCLoopBuffer is inspired by an article
on codeproject.com by Richard Lin
www.codeproject.com/Articles/10618/Loop-buffer-an-efficient-way-of-using-shared-memor
(havn't looked at the source so i'm not sure how similar it actually is)

*/

#ifndef IPC_H_
#define IPC_H_

#include "boost/thread.hpp"
#include <set>

#include "IPCMessages.h"

using namespace std;

static const int IPC_QUEUE_BUFFER_SIZE = 1024 * 32;
static const int IPC_MAGIC_ID = 54923;
static const int IPC_MAX_STRING_SIZE = 1024 * 1024 * 10;
static const int IPC_MAX_ARRAY_SIZE = 1024 * 1024 * 10;

//
// Used to connect algebra and application
//

class IPCInit {
 public:
  IPCInit();

  int magicId;                   // magic number initialized last (sync)
  boost::mutex currentIdxMutex;  // to modify nextIdx
  int confirmIdx;                // modified by server
  int nextIdx;                   // modified by client
};

class IPCLoopBuffer {
 public:
  IPCLoopBuffer();

  int magicId;
  int bufferSize;
  int ownerId;
  int readIndex;
  int writeIndex;
  unsigned long long int totalWritten;
  unsigned long long int totalRead;
  char buffer[IPC_QUEUE_BUFFER_SIZE];

  bool avail();
  bool read(char* data, unsigned int n);
  bool write(const char* data, unsigned int n);
};

// to help transfer basic arrays
template <typename T>
struct IPCArray {
  IPCArray(T* array, unsigned int size) : array(array), size(size) {}

  T* array;
  unsigned int size;
};

class IPCConnection {
 public:
  IPCConnection(void* handle, bool server, int connectionId);
  ~IPCConnection();

  void close();
  bool health();

  static IPCConnection* connect(int id);

  bool avail();
  bool read(char* data, unsigned int n);
  bool write(const char* data, unsigned int n);

  bool write(IPCMessage msgType);
  bool write(IPCMessage* msgType);
  bool write(const int* data);
  bool write(const unsigned int* data);
  bool write(const string* data);
  bool write(const bool* data);
  bool write(const double* data);

  bool read(IPCMessage* msgType);
  bool read(int* data);
  bool read(unsigned int* data);
  bool read(string* data);
  bool read(bool* data);
  bool read(double* data);

  bool read(IPCArray<double>* data);
  bool write(const IPCArray<double>* data);
  bool read(set<int>* data);
  bool write(const set<int>* data);

  IPCLoopBuffer* writeBuffer;
  IPCLoopBuffer* readBuffer;

  int connectionId;

 private:
  void* handle;
  char* sharedBuffer;

  bool server;
  bool connected;

  int ownerId;
};

class IPCGate {
 public:
  IPCGate(int id);
  ~IPCGate();

  bool open();
  void close();
  IPCConnection* nextConnection();

 private:
  int id;
  IPCInit* initData;

  void* handle;
  char* sharedBuffer;
};

#endif /* IPC_H_ */
