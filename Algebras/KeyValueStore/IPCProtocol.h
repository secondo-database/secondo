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

#ifndef IPCPROTOCOL_H_
#define IPCPROTOCOL_H_

#include "ipc.h"

//
// This is supposed to define an order in which data is read and written
// from and in the IPCLoopBuffer. To make sure there are no mismatches on client
// and server
// and to make communication easier.
//
// Some messages are defined below, didn't have the time to implement all
// messages this way.
//

template <typename... MessageData>
class IPCMessageBase {
 public:
  static bool read(IPCConnection* conn, MessageData... vals) {
    if (conn) {
      return readUnpack(conn, vals...);
    } else {
      return false;
    }
  }

  static bool write(IPCConnection* conn, MessageData... vals) {
    if (conn) {
      return writeUnpack(conn, vals...);
    } else {
      return false;
    }
  }

 private:
  template <typename First, typename... UnpackArgs>
  static bool readUnpack(IPCConnection* conn, First* first,
                         UnpackArgs... vals) {
    if (conn->read(first)) {
      return readUnpack(conn, vals...);
    }
    return false;
  }

  static bool readUnpack(IPCConnection* conn) { return true; }

  template <typename First, typename... UnpackArgs>
  static bool writeUnpack(IPCConnection* conn, First* first,
                          UnpackArgs... vals) {
    if (conn->write(first)) {
      return writeUnpack(conn, vals...);
    }
    return false;
  }

  static bool writeUnpack(IPCConnection* conn) { return true; }
};

typedef IPCMessageBase<IPCMessage*, bool*> IPCMessageBoolResult;

typedef IPCMessageBase<IPCMessage*, set<int>*> IPCMessageIntSetResult;

// refid, update, globalid, coords
typedef IPCMessageBase<int*, bool*, unsigned int*, IPCArray<double>*>
    IPCMessageDistFilter;

// refid, requestOnly, coords
typedef IPCMessageBase<int*, bool*, IPCArray<double>*> IPCMessageDistAddRect;

#endif /* IPCPROTOCOL_H_ */
