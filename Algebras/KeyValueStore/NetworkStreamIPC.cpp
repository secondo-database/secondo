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

#include "NetworkStreamIPC.h"
#include "IPCMessages.h"

#include <iostream>

namespace KVS {

NetworkStreamIPC::NetworkStreamIPC(IPCConnection* conn, int streamid)
    : streamid(streamid), conn(conn) {}
NetworkStreamIPC::~NetworkStreamIPC() {
  if (conn) {
    //    if(conn->health()) {
    //      conn->write(IPC_MSG_CLOSECONNECTION);
    //    }
    delete conn;
  }
}

bool NetworkStreamIPC::init() {
  if (conn) {
    conn->write(IPC_MSG_INITNETWORKSTREAM);
    conn->write(&streamid);

    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (conn->read(&result)) {
        return result;
      } else {
        cout << "Error: Couldn't read result response.\n";
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

string NetworkStreamIPC::getStreamType() {
  if (conn) {
    conn->write(IPC_MSG_REQUESTSTREAMTYPE);
    conn->write(&streamid);

    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      string result("");
      if (conn->read(&result)) {
        return result;
      } else {
        cout << "Error: (IPC) Couldn't read result string.\n";
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return string("");
}

bool NetworkStreamIPC::requestStream() {
  if (conn) {
    conn->write(IPC_MSG_REQUESTSTREAM);
    conn->write(&streamid);

    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (conn->read(&result)) {
        return result;
      } else {
        cout << "Error: (IPC) Couldn't read result.\n";
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

char* NetworkStreamIPC::nextTuple(unsigned int* n) {
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

bool NetworkStreamIPC::remove() {
  if (conn) {
    conn->write(IPC_MSG_REMOVESTREAM);
    conn->write(&streamid);

    IPCMessage responseType;
    conn->read(&responseType);
    if (responseType == IPC_MSG_RESULT) {
      bool result = false;
      if (conn->read(&result)) {
        return result;
      } else {
        cout << "Error: (IPC) Couldn't read result.\n";
      }
    } else {
      cout << "Error: Wrong IPC response type.\n";
    }
  } else {
    cout << "Error: No IPC Connection.\n";
  }
  return false;
}

bool NetworkStreamIPC::health() {
  if (conn->avail()) {
    return true;
  } else {
    return true;
  }
}
}
