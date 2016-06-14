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

#include "AlgebraOperators.h"

#include <fstream>
#include <string>

using namespace std;

namespace KVS {

extern KeyValueStoreIPC* kvsIPC;

// **********************
// Operator: kvsDataSourceSCP()
//
//

ListExpr kvsDataSourceSCPTM(ListExpr args) {
  // get type

  // busy wait for type file creation
  string typeFilePath = kvsIPC->getSCPSourcePath() + PATH_SLASH + "source_type";
  while (!FileSystem::FileOrFolderExists(typeFilePath)) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(250));
  }

  // read file
  ifstream typeFile(typeFilePath, ios::binary);

  size_t typeLen;
  typeFile.read((char*)&typeLen, sizeof(typeLen));

  char* typeBuffer = new char[typeLen];
  typeFile.read(typeBuffer, typeLen);

  typeFile.close();

  ListExpr streamType;
  nl->ReadFromString(typeBuffer, streamType);

  delete[] typeBuffer;

  return streamType;
}

int kvsDataSourceSCPVM(Word* args, Word& result, int message, Word& local,
                       Supplier s) {
  class SCPDataSource {
   public:
    SCPDataSource() : currentFile(0), currentId(1){};
    ~SCPDataSource() { delete currentFile; }

    ifstream* currentFile;
    int currentId;
  };

  SCPDataSource* dataSource = static_cast<SCPDataSource*>(local.addr);

  switch (message) {
    case OPEN: {
      delete dataSource;
      dataSource = new SCPDataSource;

      local.addr = dataSource;
      return 0;
    }
    case REQUEST: {
      while (true) {
        if (dataSource->currentFile == 0) {
          // busy wait for type file creation
          string dataFilePath = kvsIPC->getSCPSourcePath() + PATH_SLASH +
                                stringutils::int2str(dataSource->currentId);
          string endPath = kvsIPC->getSCPSourcePath() + PATH_SLASH + "end";

          while (!FileSystem::FileOrFolderExists(dataFilePath) ||
                 !FileSystem::FileOrFolderExists(endPath)) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(250));
          }

          if (FileSystem::FileOrFolderExists(dataFilePath)) {
            dataSource->currentFile = new ifstream(dataFilePath, ios::binary);
          } else {
            result.setAddr(0);
            return CANCEL;
          }
        }

        size_t tupleBlockSize;
        dataSource->currentFile->read((char*)&tupleBlockSize,
                                      sizeof(tupleBlockSize));

        if (tupleBlockSize != 0) {
          TupleType* tupleType;
          ListExpr resultType = GetTupleResultType(s);
          tupleType = new TupleType(nl->Second(resultType));

          Tuple* tempTuple = new Tuple(tupleType);

          char* tupleBuffer = new char[tupleBlockSize];
          dataSource->currentFile->read(tupleBuffer, tupleBlockSize);

          tempTuple->ReadFromBin(0,tupleBuffer);

          delete[] tupleBuffer;

          result = SetWord(tempTuple);
          return YIELD;
        } else {
          dataSource->currentFile->close();
          delete dataSource->currentFile;
          dataSource->currentFile = 0;
          dataSource->currentId++;
        }
      }
      result.setAddr(0);
      return CANCEL;
    }
    case CLOSE: {
      if (dataSource != 0) {
        delete dataSource;
        result.setAddr(0);
      }
      return 0;
    }
  }
  /* should not happen */
  return -1;
}
}
