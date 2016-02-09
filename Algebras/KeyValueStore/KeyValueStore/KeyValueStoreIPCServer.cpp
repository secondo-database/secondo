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

#include "KeyValueStoreIPCServer.h"

#include "KeyValueStoreDispatch.h"
#include "KeyValueStoreDebug.h"

#include <sstream>

#ifdef SECONDO_WIN32
#include "windows.h"
#else
#include <sys/file.h>
#include <unistd.h>
#endif

namespace KVS {

KeyValueStoreIPCServer::KeyValueStoreIPCServer(std::string appPath, int appId,
                                               bool useConsole)
    : appId(appId),
      appPath(appPath),
      appMutexHandle(NULL),
      kvs(appPath),
      lastMessage(IPC_MSG_RESULT) {
  buildDispatchMap(dispatchMap);

  if (!useConsole) {
    cout << "Application initialized.\n";

    char hostname[255];
    hostname[0] = '\0';

#ifndef SECONDO_WIN32
    gethostname(hostname, 255);
#endif

    std::stringstream extStr;
    extStr << "_" << hostname << ".log";

    kvsLogfile = new std::ofstream(
        appPath.substr(0, appPath.find_last_of('.')) + extStr.str(), ios::app);
    KOUT.rdbuf(kvsLogfile->rdbuf());

    restructureLogfile = new std::ofstream(
        kvs.getRestructurePath() + PATH_SLASH + "kvsRestructure" + extStr.str(),
        ios::app);
    ROUT.rdbuf(restructureLogfile->rdbuf());
  } else {
    kvsLogfile = 0;
    restructureLogfile = 0;
  }
}
KeyValueStoreIPCServer::~KeyValueStoreIPCServer() {
#ifdef SECONDO_WIN32
  if (appMutexHandle != NULL) {
    CloseHandle(appMutexHandle);
  }
#endif

  if (kvsLogfile) {
    KOUT.flush();
    kvsLogfile->close();
    delete kvsLogfile;
  }

  if (restructureLogfile) {
    ROUT.flush();
    restructureLogfile->close();
    delete restructureLogfile;
  }
}

#ifdef SECONDO_WIN32
bool KeyValueStoreIPCServer::checkExclusive() {
  std::stringstream mutexName;
  mutexName << "Local\\SecondoApp" << appId;

  appMutexHandle = CreateMutexA(NULL,   // default security attributes
                                FALSE,  // initially not owned
                                mutexName.str().c_str());  // named mutex

  return (appMutexHandle != NULL && (GetLastError() != ERROR_ALREADY_EXISTS));
}
#else
bool KeyValueStoreIPCServer::checkExclusive() {
  char hostname[255];
  hostname[0] = '\0';
  gethostname(hostname, 255);

  stringstream lockfileName;
  lockfileName << appPath << "_lock_" << hostname << "_" << appId;

  int fd = open(lockfileName.str().c_str(), O_RDWR | O_CREAT,
                0666);  // open or create lockfile
  // probably should release file/lock at some point but i'm too lazy (doesn't
  // seem to cause problems yet)
  return !flock(
      fd,
      LOCK_EX | LOCK_NB);  // grab exclusive lock, fail if can't obtain.;
}
#endif

int KeyValueStoreIPCServer::run() {
  IPCGate gate(appId);
  std::vector<IPCConnection*> connections;

  unsigned int idleCounter = 0;

  if (gate.open()) {
    KOUT << "Gate opened. Waiting for new connections." << endl;
    while (true) {
      IPCConnection* newConn = gate.nextConnection();

      if (newConn) {
        KOUT << "Opened IPC-Connection:" << newConn->connectionId << endl;

        if (newConn->health()) {
          // KOUT<<"New Connection established.."<<endl;
          connections.push_back(newConn);
        } else {
          delete newConn;
          KOUT << "Connection failed.." << endl;
        }
      }

      for (unsigned int connIdx = 0; connIdx < connections.size(); ++connIdx) {
        if (connections[connIdx]->avail() && connections[connIdx]->health()) {
          // dispatch
          int result = dispatch(connections[connIdx]);
          if (result != NORESULT) {
            if (result == REMOVECONNECTION) {
              // KOUT<<"Removing Connection..."<<endl;
              connections.erase(connections.begin() + connIdx);
              connIdx--;
            }
          }

          idleCounter = 0;
        } else if (!connections[connIdx]->health()) {
          connections.erase(connections.begin() + connIdx);
          connIdx--;
          KOUT << "Connection removed..." << endl;
        }
      }

      idleCounter++;
      if (idleCounter > 50000) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(5));
      }
    }
  } else {
    KOUT << "Failed to open gate." << endl;
    return -1;
  }

  return 0;
}

int KeyValueStoreIPCServer::dispatch(IPCConnection* conn) {
  IPCMessage messageType;
  conn->read((char*)&messageType, sizeof(messageType));

  auto dispatchItr = dispatchMap.find(messageType);

  if (dispatchItr != dispatchMap.end()) {
    lastMessage = messageType;
    return dispatchItr->second(&kvs, conn);
  } else {
    KOUT << "Error: Unknown message type (" << messageType << ")."
         << " Last Message:" << lastMessage << endl;
  }

  return NORESULT;
}
}
