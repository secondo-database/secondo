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

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

static const key_t IPC_INIT_KEY = 487348423;
static const key_t IPC_CONNECTION_KEY = 23423522;

IPCConnection::IPCConnection(void* handle, bool server, int connectionId)
    : connectionId(connectionId),
      handle(handle),
      server(server),
      ownerId(rand()) {
  sharedBuffer = (char*)handle;
  connected = (handle != MAP_FAILED);

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

  // debug
  writeBuffer->ownerId = ownerId;
}

void IPCConnection::close() {
  if (sharedBuffer != 0 && handle != MAP_FAILED) {
    if (writeBuffer) {
      writeBuffer->magicId = 0;
    }

    if (readBuffer) {
      readBuffer->magicId = 0;
    }

    munmap(handle, sizeof(IPCLoopBuffer) * 2);

    if (server) {
      // shm_unlink(name.str().c_str());
    }

    sharedBuffer = 0;
  }

  handle = 0;
}

IPCConnection* IPCConnection::connect(int id) {
  // magic number - better to use ftok to avoid conflicts ftok("somepath", 'R')
  // key_t initKey = IPC_INIT_KEY+id;

  stringstream name;
  name << "/SecondoInit" << id;

  // 0644 | IPC_CREAT
  int initId =
      shm_open(name.str().c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

  if (initId != -1) {
    if (ftruncate(initId, sizeof(IPCInit)) != -1) {
      void* initBuffer = (char*)mmap(
          NULL, sizeof(IPCInit), PROT_READ | PROT_WRITE, MAP_SHARED, initId, 0);

      if (initBuffer != MAP_FAILED) {
        IPCInit* init = (IPCInit*)initBuffer;

        time_t timeout = time(NULL);
        while (init->magicId != IPC_MAGIC_ID) {
          if (difftime(time(NULL), timeout) > 3.0) {
            cout << "Could not connect to IPC-Server (timeout)" << endl;
            munmap(initBuffer, sizeof(IPCInit));
            return 0;
          }
          boost::this_thread::sleep(boost::posix_time::milliseconds(20));
        }

        timeout = time(NULL);
        while (!init->currentIdxMutex.try_lock()) {
          if (difftime(time(NULL), timeout) > 3.0) {
            cout << "Could not connect to IPC-Server (timeout)" << endl;
            munmap(initBuffer, sizeof(IPCInit));
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
          if (difftime(time(NULL), timeout) > 5.0) {
            cout << "Could not connect to IPC-Server (timeout)" << endl;
            munmap(initBuffer, sizeof(IPCInit));
            return 0;
          }
          boost::this_thread::sleep(boost::posix_time::milliseconds(20));
        }

        munmap(initBuffer, sizeof(IPCInit));

        stringstream connectionName;
        connectionName << "/SecondoConnection" << connectionId;

        void* connectionHandle;

        int fd = shm_open(connectionName.str().c_str(), O_CREAT | O_RDWR,
                          S_IRUSR | S_IWUSR);
        if (fd != -1) {
          if (ftruncate(fd, sizeof(IPCLoopBuffer) * 2) != -1) {
            connectionHandle = mmap(NULL, sizeof(IPCLoopBuffer) * 2,
                                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            if (connectionHandle != MAP_FAILED) {
              return new IPCConnection(connectionHandle, false, connectionId);
            } else {
              cout
                  << "Could not create shared memory object (Connection) (mmap)"
                  << endl;
            }
          } else {
            cout << "Could not create shared memory object (Connection) "
                    "(ftruncate)"
                 << endl;
          }
        } else {
          cout
              << "Could not create shared memory object (Connection) (shm_open)"
              << endl;
        }
      } else {
        cout << "Could not map shared memory object (Init) (mmap)" << endl;
      }
    } else {
      cout << "Could not set size of shared memory object (Init)" << endl;
    }
  } else {
    cout << "Could not create shared memory object (Init) (shm_open)" << endl;
  }

  return 0;
}

IPCGate::IPCGate(int id) : id(id), initData(0), handle(0), sharedBuffer(0) {}

IPCGate::~IPCGate() { close(); }

bool IPCGate::open() {
  // magic number - better to use ftok to avoid conflicts ftok("somepath", 'R')
  /*key_t initKey = IPC_INIT_KEY+id;

  int* pId = new int;
  *pId = shmget(initKey, sizeof(IPCInit), 0644 | IPC_CREAT);

  if(*pId != -1) {
    handle = (void*)pId;

    sharedBuffer = (char*)shmat(*pId, (void *)0, 0);

    if (sharedBuffer != (char *)(-1)) {
      initData = new(sharedBuffer) IPCInit();
      initData->magicId = IPC_MAGIC_ID;
      return true;
    } else {
      cout<<"Could not map shared memory object (Init) (shmat)"<<endl;
      shmctl(*pId, IPC_RMID, NULL);
      handle = 0;
    }
  } else {
    cout<<"Could not create shared memory object (Init) (shmget)"<<endl;
  }

  delete pId;
  return false;*/
  stringstream name;
  name << "/SecondoInit" << id;

  int fd;

  fd = shm_open(name.str().c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd != -1) {
    if (ftruncate(fd, sizeof(IPCInit)) != -1) {
      sharedBuffer = (char*)mmap(NULL, sizeof(IPCInit), PROT_READ | PROT_WRITE,
                                 MAP_SHARED, fd, 0);

      // cout<<"Server Inithandle:"<<(void*)sharedBuffer<<endl;

      if (sharedBuffer != (char*)MAP_FAILED) {
        initData = new (sharedBuffer) IPCInit();
        initData->magicId = IPC_MAGIC_ID;
        return true;
      } else {
        cout << "Could not map shared memory object (Init)" << endl;
      }
    } else {
      cout << "Could not set size of shared memory object (Init)" << endl;
    }
  } else {
    cout << "Could not create shared memory object (Init) (shm_open)" << endl;
  }

  sharedBuffer = 0;
  handle = 0;

  return false;
}

IPCConnection* IPCGate::nextConnection() {
  if (initData->nextIdx > initData->confirmIdx) {
    stringstream name;
    name << "/SecondoConnection" << initData->confirmIdx;
    // key_t connectionKey = IPC_CONNECTION_KEY+initData->confirmIdx;

    void* sharedBuffer = 0;

    int fd = shm_open(name.str().c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd != -1) {
      if (ftruncate(fd, sizeof(IPCLoopBuffer) * 2) != -1) {
        sharedBuffer = mmap(NULL, sizeof(IPCLoopBuffer) * 2,
                            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        // cout<<"Server Handle:"<<sharedBuffer<<endl;
        // cout<<"Server Connname:"<<name.str()<<endl;
        if (sharedBuffer != MAP_FAILED) {
          int connectionId = initData->confirmIdx;
          initData->confirmIdx++;
          return new IPCConnection(sharedBuffer, true, connectionId);
        } else {
          cout << "Could not map shared memory object (nextConnection)" << endl;
        }
      } else {
        cout << "Could not set size of shared memory object (nextConnection)"
             << endl;
      }
    } else {
      cout
          << "Could not create shared memory object (nextConnection) (shm_open)"
          << endl;
    }
  }

  return 0;
}

void IPCGate::close() {
  if (sharedBuffer != 0) {
    initData->magicId = 0;
    initData->nextIdx = 0;
    initData->confirmIdx = 0;

    stringstream name;
    name << "/SecondoInit" << id;

    munmap(sharedBuffer, sizeof(IPCInit));
    shm_unlink(name.str().c_str());
    sharedBuffer = 0;
  }

  handle = 0;
  initData = 0;
}

#endif  // SECONDO_WIN32
