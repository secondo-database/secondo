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

#ifndef TRANSFERMETHOD_H_
#define TRANSFERMETHOD_H_

#include "Connection.h"

#include <fstream>

namespace KVS {

//
// Helps implement different ways to transfer data.
// Like TCP vs SCP (not supported anymore)
//
class TransferMethod {
 public:
  TransferMethod(Connection* connection);
  virtual ~TransferMethod();

  virtual bool init(unsigned int transferId) = 0;
  virtual bool sendTuple(char* data, unsigned int dataLen) = 0;
  virtual bool endStream() = 0;
  virtual bool cleanUp() = 0;
  virtual bool import(string targetRelation, string clientCommand) = 0;
  virtual bool resendUnconfirmed(unsigned int tId) = 0;
  virtual bool retry() = 0;
  virtual void changeConnection(Connection* connection) = 0;

  void clearUnconfirmed();

  vector<pair<char*, unsigned int> > unconfirmed;
  unsigned int transferId;
  int tupleCounter;

  Connection* connection;

 protected:
};

//
// To transfer data via TCP (data = tuple streams)
//
class TransferMethodTCP : public TransferMethod {
 public:
  TransferMethodTCP(Connection* connection, string streamType,
                    string baseAttributeList, bool creationCheck = false);
  ~TransferMethodTCP();

  bool init(unsigned int tId);
  bool sendTuple(char* data, unsigned int dataLen);
  bool endStream();
  bool cleanUp();
  bool import(string targetRelation, string clientCommand);
  bool resendUnconfirmed(unsigned int tId);
  bool retry();
  void changeConnection(Connection* connection);

 private:
  static const int MAX_BUFFER_SIZE = 32 * 1024;  // 1440;

  char buffer[MAX_BUFFER_SIZE];
  unsigned int bufferPos;

  string streamType;
  string baseAttributeList;

  bool creationCheck;
};

//
// Was originally supported but isnt anymore
//
class TransferMethodSCP : public TransferMethod {
 public:
  TransferMethodSCP(Connection* connection, string streamType);
  ~TransferMethodSCP();

  bool init(unsigned int tId);
  bool sendTuple(char* data, unsigned int dataLen);
  bool endStream();
  bool cleanUp();
  bool import(string targetRelation, string clientCommand);
  bool resendUnconfirmed(unsigned int tId);
  bool retry();
  void changeConnection(Connection* connection);

 private:
  string streamType;
  string sshAddress;
  string destinationBasePath;

  string localDataPath;
  ofstream* dataFile;

  char* tupleBuffer;
  size_t tupleBufferSize;

  unsigned int targetId;
};
}

#endif /* TRANSFERMETHOD_H_ */
