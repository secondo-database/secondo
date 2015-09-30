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

#include "DistributeIPC.h"
#include "IPCMessages.h"

namespace KVS {

DistributeIPC::DistributeIPC(IPCConnection* conn)
    : resultStreamSuccess(false),
      resultAvailable(false),
      resultCode(0),
      resultMessage(""),
      conn(conn) {}
DistributeIPC::~DistributeIPC() {
  if (conn) {
    delete conn;
  }
}

bool DistributeIPC::init(int distributionId, string streamType,
                         string baseAttributeList, string targetRelation,
                         string insertCommand, string deleteCommand,
                         bool restructure) {
  resultStreamSuccess = false;

  if (conn) {
    conn->write(IPC_MSG_INITDISTRIBUTE);
    conn->write(&distributionId);
    conn->write(&streamType);
    conn->write(&baseAttributeList);
    conn->write(&targetRelation);
    conn->write(&insertCommand);
    conn->write(&deleteCommand);
    conn->write(&restructure);

    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      return conn->read(&result);
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }

  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

bool DistributeIPC::sendTuple(int serverId, const char* data, unsigned int n) {
  if (conn) {
    conn->write(IPC_MSG_SENDTUPLE);
    conn->write(&serverId);
    conn->write(&n);
    conn->write(data, n);
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

bool DistributeIPC::requestResultStream() {
  if (conn) {
    conn->write(IPC_MSG_REQUESTRESULT);

    cout << "Waiting for response...\n";
    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      cout << "Got response... waiting for result\n";
      if (conn->read(&result)) {
        cout << "Got result...\n";
        resultStreamSuccess = result;
        return result;
      } else {
        return false;
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

char* DistributeIPC::nextTuple(int* n) {
  if (conn) {
    conn->read(n);

    if (*n > 0) {
      char* data = new char[*n];
      conn->read(data, *n);
      return data;
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }

  return 0;
}

void DistributeIPC::end() {
  if (conn) {
    conn->write(IPC_MSG_ENDDISTRIBUTE);
  } else {
    cout << "Error: No IPC Connection.\n";
  }
}

void DistributeIPC::close() {
  if (conn) {
    conn->write(IPC_MSG_CLOSEDISTRIBUTE);
  } else {
    cout << "Error: No IPC Connection.\n";
  }
}

bool DistributeIPC::health() {
  if (conn->avail()) {
    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_ERROR) {
      string errMsg("");
      conn->read(&errMsg);
      cout << "Distribute ended: " << errMsg << endl;
      return false;
    } else {
      return true;
    }
  } else {
    return true;
  }
}

bool DistributeIPC::checkResult() {
  if (conn->avail()) {
    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      conn->read(&resultCode);
      conn->read(&resultMessage);

      cout << "Distribute ended: Code(" << resultCode << "), Message("
           << resultMessage << ")" << endl;

      resultAvailable = true;
    }
    return true;
  } else {
    return false;
  }
}

bool DistributeIPC::getResult() {
  if (!resultAvailable) {
    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      conn->read(&resultCode);
      conn->read(&resultMessage);

      resultAvailable = true;
    }
  }
  return resultAvailable;
}
}
