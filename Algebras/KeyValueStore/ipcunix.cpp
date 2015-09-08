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

*/

#include "ipc.h"

#include "StandardTypes.h"  //for SECONDO_WIN32 to be defined

#ifndef SECONDO_WIN32

#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

static const key_t IPC_INIT_KEY = 487348423;
static const key_t IPC_CONNECTION_KEY = 23423522;

IPCConnection::IPCConnection(void* handle, bool server, double timeout)
    : handle(handle), server(server), timeout(timeout), timedout(false) {
  int* pId = (int*)handle;
  sharedBuffer = (char*)shmat(*pId, (void*)0, 0);
  connected = (sharedBuffer != (char*)(-1));

  if (server) {
    writeBuffer = new (sharedBuffer) IPCLoopBuffer();
    writeBuffer->magicId = IPC_MAGIC_ID;
    readBuffer = new (sharedBuffer + sizeof(IPCLoopBuffer)) IPCLoopBuffer();
    readBuffer->magicId = IPC_MAGIC_ID;
  } else {
    writeBuffer = (IPCLoopBuffer*)(sharedBuffer + sizeof(IPCLoopBuffer));
    while (writeBuffer->magicId != IPC_MAGIC_ID) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }

    readBuffer = (IPCLoopBuffer*)sharedBuffer;
    while (readBuffer->magicId != IPC_MAGIC_ID) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(20));
    }
  }
}

void IPCConnection::close() {
  if (sharedBuffer != 0 && sharedBuffer != (char*)(-1)) {
    shmdt(sharedBuffer);
    sharedBuffer = 0;
  }

  int* pId = (int*)handle;
  if (server) {
    if (*pId != -1) {
      shmctl(*pId, IPC_RMID, NULL);
    }
  }
  delete pId;
  handle = 0;
}

IPCConnection* IPCConnection::connect(int id) {
  // magic number - better to use ftok to avoid conflicts ftok("somepath", 'R')
  key_t initKey = IPC_INIT_KEY + id;

  // 0644 | IPC_CREAT
  int initId = shmget(initKey, sizeof(IPCInit), 0644);

  if (initId != -1) {
    char* initBuffer = (char*)shmat(initId, (void*)0, 0);

    if (initBuffer != (char*)(-1)) {
      IPCInit* init = (IPCInit*)initBuffer;

      time_t timeout = time(NULL);
      while (init->magicId != IPC_MAGIC_ID) {
        if (difftime(time(NULL), timeout) > 10.0) {
          cout << "Could not connect to IPC-Server (timeout)" << endl;
          shmdt(initBuffer);
          return 0;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
      }

      timeout = time(NULL);
      while (!init->currentIdxMutex.try_lock()) {
        if (difftime(time(NULL), timeout) > 10.0) {
          cout << "Could not connect to IPC-Server (timeout)" << endl;
          shmdt(initBuffer);
          return 0;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
      }

      int connectionId = init->nextIdx;
      init->nextIdx++;
      init->currentIdxMutex.unlock();

      // wait for memory to be initialized
      timeout = time(NULL);
      while (connectionId >= init->confirmIdx) {
        if (difftime(time(NULL), timeout) > 10.0) {
          cout << "Could not connect to IPC-Server (timeout)" << endl;
          shmdt(initBuffer);
          return 0;
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
      }

      shmdt(initBuffer);

      key_t connectionKey = IPC_CONNECTION_KEY + connectionId;
      int* connectionHandle = new int;
      *connectionHandle =
          shmget(connectionKey, sizeof(IPCLoopBuffer) * 2, 0644);

      if (*connectionHandle != -1) {
        return new IPCConnection((void*)connectionHandle, false);
      } else {
        cout << "Could not create shared memory object (Connection) (shmget)"
             << endl;
        delete connectionHandle;
      }
    } else {
      cout << "Could not map shared memory object (Init) (shmat)" << endl;
    }
  } else {
    cout << "Could not create shared memory object (Init) (shmget)" << endl;
  }

  return 0;
}

IPCGate::IPCGate(int id) : id(id), initData(0), handle(0), sharedBuffer(0) {}

IPCGate::~IPCGate() { close(); }

bool IPCGate::open() {
  // magic number - better to use ftok to avoid conflicts ftok("somepath", 'R')
  key_t initKey = IPC_INIT_KEY + id;

  int* pId = new int;
  *pId = shmget(initKey, sizeof(IPCInit), 0644 | IPC_CREAT);

  if (*pId != -1) {
    handle = (void*)pId;

    sharedBuffer = (char*)shmat(*pId, (void*)0, 0);

    if (sharedBuffer != (char*)(-1)) {
      initData = new (sharedBuffer) IPCInit();
      initData->magicId = IPC_MAGIC_ID;
      return true;
    } else {
      cout << "Could not map shared memory object (Init) (shmat)" << endl;
      shmctl(*pId, IPC_RMID, NULL);
      handle = 0;
    }
  } else {
    cout << "Could not create shared memory object (Init) (shmget)" << endl;
  }

  delete pId;
  return false;
}

IPCConnection* IPCGate::nextConnection() {
  if (initData->nextIdx > initData->confirmIdx) {
    key_t connectionKey = IPC_CONNECTION_KEY + initData->confirmIdx;
    int* connectionHandle = new int;

    *connectionHandle =
        shmget(connectionKey, sizeof(IPCLoopBuffer) * 2, 0644 | IPC_CREAT);

    if (*connectionHandle != -1) {
      initData->confirmIdx++;
      return new IPCConnection((void*)connectionHandle, true);
    } else {
      cout << "Could not create shared memory object (Init) (shmget)" << endl;
      delete connectionHandle;
      return 0;
    }
  } else {
    return 0;
  }
}

void IPCGate::close() {
  if (sharedBuffer != 0 && sharedBuffer != (char*)(-1)) {
    initData->magicId = 0;
    shmdt(sharedBuffer);
    sharedBuffer = 0;
  }

  int* pId = (int*)handle;
  if (*pId != -1) {
    shmctl(*pId, IPC_RMID, NULL);
  }
  delete pId;
  handle = 0;

  initData = 0;
}

#endif  // SECONDO_WIN32
