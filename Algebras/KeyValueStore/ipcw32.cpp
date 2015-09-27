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

#ifdef SECONDO_WIN32

#include "windows.h"

IPCConnection::IPCConnection(void* handle, bool server, double timeout)
    : handle(handle), server(server), timeout(timeout), timedout(false) {
  sharedBuffer =
      (char*)MapViewOfFile(handle,               // handle to map object
                           FILE_MAP_ALL_ACCESS,  // read/write permission
                           0, 0, sizeof(IPCLoopBuffer) * 2);

  connected = (sharedBuffer != NULL);

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
  if (sharedBuffer) {
    UnmapViewOfFile(sharedBuffer);
    sharedBuffer = 0;
  }

  if (handle) {
    CloseHandle(handle);
    handle = 0;
  }
}

IPCConnection* IPCConnection::connect(int id) {
  stringstream name;
  name << "Local\\SecondoInit" << id;

  void* w32InitHandle =
      OpenFileMappingA(FILE_MAP_ALL_ACCESS,  // read/write access
                       FALSE,                // do not inherit the name
                       name.str().c_str());  // name of mapping object

  if (w32InitHandle != NULL) {
    char* initBuffer =
        (char*)MapViewOfFile(w32InitHandle,        // handle to map object
                             FILE_MAP_ALL_ACCESS,  // read/write permission
                             0, 0, sizeof(IPCInit));

    if (initBuffer != NULL) {
      IPCInit* init = (IPCInit*)initBuffer;

      while (init->magicId != IPC_MAGIC_ID) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
      }

      while (!init->currentIdxMutex.try_lock()) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
      }

      int connectionId = init->nextIdx;
      init->nextIdx++;
      init->currentIdxMutex.unlock();

      // wait for memory to be initialized
      while (connectionId >= init->confirmIdx) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
      }

      UnmapViewOfFile(initBuffer);
      CloseHandle(w32InitHandle);

      stringstream connectionName;
      connectionName << "Local\\SecondoConnection" << connectionId;

      void* w32ConnectionHandle = OpenFileMappingA(
          FILE_MAP_ALL_ACCESS,            // read/write access
          FALSE,                          // do not inherit the name
          connectionName.str().c_str());  // name of mapping object

      if (w32ConnectionHandle != NULL) {
        return new IPCConnection(w32ConnectionHandle, false);
      } else {
        cout << "Could not create file mapping object (Connection) ("
             << GetLastError() << ")" << endl;
      }
    } else {
      cout << "Could not map view of file (Init) (" << GetLastError() << ")"
           << endl;
      CloseHandle(w32InitHandle);
    }
  } else {
    cout << "Could not create file mapping object (Init) (" << GetLastError()
         << ")" << endl;
  }

  return 0;
}

IPCGate::IPCGate(int id) : id(id), initData(0), handle(0), sharedBuffer(0) {}

IPCGate::~IPCGate() { close(); }

bool IPCGate::open() {
  stringstream name;
  name << "Local\\SecondoInit" << id;

  handle = CreateFileMappingA(
      INVALID_HANDLE_VALUE,  // use paging file
      NULL,                  // default security
      PAGE_READWRITE,        // read/write access
      0,                     // maximum object size (high-order DWORD)
      sizeof(IPCInit),       // maximum object size (low-order DWORD)
      name.str().c_str());   // name of mapping object

  if (handle != NULL) {
    sharedBuffer =
        (char*)MapViewOfFile(handle,               // handle to map object
                             FILE_MAP_ALL_ACCESS,  // read/write permission
                             0, 0, sizeof(IPCInit));

    if (sharedBuffer != NULL) {
      initData = new (sharedBuffer) IPCInit();
      initData->magicId = IPC_MAGIC_ID;
      return true;
    } else {
      cout << "Could not map view of file (" << GetLastError() << ")" << endl;
      CloseHandle(handle);
      handle = NULL;
    }
  } else {
    cout << "Could not create file mapping object (" << GetLastError() << ")"
         << endl;
  }

  return false;
}

IPCConnection* IPCGate::nextConnection() {
  if (initData->nextIdx > initData->confirmIdx) {
    stringstream name;
    name << "Local\\SecondoConnection" << initData->confirmIdx;

    void* tempHandle = CreateFileMappingA(
        INVALID_HANDLE_VALUE,       // use paging file
        NULL,                       // default security
        PAGE_READWRITE,             // read/write access
        0,                          // maximum object size (high-order DWORD)
        sizeof(IPCLoopBuffer) * 2,  // maximum object size (low-order DWORD)
        name.str().c_str());        // name of mapping object

    if (tempHandle != NULL) {
      initData->confirmIdx++;
      return new IPCConnection(tempHandle, true);
    } else {
      cout << "Could not create file mapping object (" << GetLastError() << ")"
           << endl;
      return 0;
    }
  } else {
    return 0;
  }
}

void IPCGate::close() {
  if (sharedBuffer) {
    UnmapViewOfFile(sharedBuffer);
    sharedBuffer = 0;
  }

  if (handle) {
    CloseHandle(handle);
    handle = 0;
  }

  initData = 0;
}

#endif  // SECONDO_WIN32
