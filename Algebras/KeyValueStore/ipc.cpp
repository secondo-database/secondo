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

#include "ipc.h"

#include "boost/date_time.hpp"

using namespace std;

IPCInit::IPCInit() : magicId(0), confirmIdx(0), nextIdx(0) {}

IPCLoopBuffer::IPCLoopBuffer()
    : magicId(0),
      bufferSize(IPC_QUEUE_BUFFER_SIZE),
      ownerId(0),
      readIndex(0),
      writeIndex(0),
      totalWritten(0),
      totalRead(0) {}

bool IPCLoopBuffer::avail() { return totalWritten != totalRead; }

bool IPCLoopBuffer::read(char* data, unsigned int n) {
  int offset = 0;
  while (true) {
    int available = totalWritten - totalRead;

    if (available == 0) {
      continue;
    }

    if (available > static_cast<int>(n)) {
      available = n;
    }

    if (readIndex + available >= bufferSize) {
      available = bufferSize - readIndex;
      assert(available > 0);
    }

    memcpy(data + offset, buffer + readIndex, available);

    totalRead += available;
    readIndex += available;
    if (readIndex == bufferSize) {
      readIndex = 0;
    }

    n -= available;
    if (n == 0) {
      return true;
    }
    offset += available;
  }
}

bool IPCLoopBuffer::write(const char* data, unsigned int n) {
  int offset = 0;

  while (true) {
    int space = bufferSize - (totalWritten - totalRead);
    assert(space >= 0);

    if (space == 0) {
      continue;
    }

    if (space > static_cast<int>(n)) {
      space = n;
    }

    if (writeIndex + space >= bufferSize) {
      space = bufferSize - writeIndex;
      assert(space > 0);
    }

    memcpy(buffer + writeIndex, data + offset, space);

    totalWritten += space;
    writeIndex += space;

    if (writeIndex == bufferSize) {
      writeIndex = 0;
    }

    n -= space;
    if (n == 0) {
      return true;
    }
    offset += space;
  }
}

bool IPCConnection::avail() {
  return readBuffer->totalWritten != readBuffer->totalRead;
}

bool IPCConnection::read(char* data, unsigned int n) {
  if (writeBuffer->ownerId == ownerId) {
    if (n > 0) {
      return readBuffer->read(data, n);
    }
    return true;
  } else {
    cout << "IPC-Error: Ownership corrupted..." << endl;
    return false;
  }
}

bool IPCConnection::write(const char* data, unsigned int n) {
  if (writeBuffer->ownerId == ownerId) {
    if (n > 0) {
      return writeBuffer->write(data, n);
    }
    return true;
  } else {
    cout << "IPC-Error: Ownership corrupted..." << endl;
    return false;
  }
}

bool IPCConnection::read(IPCMessage* msgType) {
  return read((char*)msgType, sizeof(IPCMessage));
}

bool IPCConnection::write(IPCMessage msgType) {
  return write((char*)&msgType, sizeof(IPCMessage));
}

bool IPCConnection::write(IPCMessage* msgType) {
  return write((char*)msgType, sizeof(IPCMessage));
}

bool IPCConnection::read(int* data) { return read((char*)data, sizeof(int)); }

bool IPCConnection::write(const int* data) {
  return write((char*)data, sizeof(int));
}

bool IPCConnection::read(unsigned int* data) {
  return read((char*)data, sizeof(unsigned int));
}

bool IPCConnection::write(const unsigned int* data) {
  return write((char*)data, sizeof(unsigned int));
}

bool IPCConnection::read(double* data) {
  return read((char*)data, sizeof(double));
}

bool IPCConnection::write(const double* data) {
  return write((char*)data, sizeof(double));
}

bool IPCConnection::read(string* data) {
  int len = 0;
  if (read(&len)) {
    if (len > 0 && len < IPC_MAX_STRING_SIZE) {
      char* temp = new char[len];
      if (read(temp, len)) {
        data->assign(temp, len);
        delete[] temp;
        return true;
      }
      delete[] temp;
    } else {
      data->clear();
      return (len == 0);
    }
  }
  return false;
}

bool IPCConnection::write(const string* data) {
  int len = data->length();
  if (write(&len)) {
    return write(data->c_str(), len);
  } else {
    return false;
  }
}

bool IPCConnection::read(IPCArray<double>* data) {
  if (read(&data->size)) {
    if (data->size < IPC_MAX_ARRAY_SIZE) {
      delete[] data->array;

      bool result = true;
      data->array = new double[data->size];
      for (unsigned int i = 0; i < data->size; ++i) {
        result &= read((char*)&data->array[i], sizeof(double));
      }

      return result;
    }
  }

  return false;
}

bool IPCConnection::write(const IPCArray<double>* data) {
  if (write(&data->size)) {
    bool result = true;
    for (unsigned int i = 0; i < data->size; ++i) {
      result &= write((char*)&data->array[i], sizeof(double));
    }
    return result;
  }
  return false;
}

bool IPCConnection::read(set<int>* data) {
  unsigned int nrResults;
  if (read(&nrResults)) {
    bool result = true;
    int tempElem;
    for (unsigned int i = 0; i < nrResults; ++i) {
      result &= read(&tempElem);
      if (result) {
        data->insert(tempElem);
      }
    }
    return result;
  }
  return false;
}

bool IPCConnection::write(const set<int>* data) {
  unsigned int nrResults = data->size();
  if (write(&nrResults)) {
    bool result = true;
    if (nrResults > 0) {
      for (auto it = data->begin(); it != data->end(); ++it) {
        auto tempResult = *it;
        result &= write(&tempResult);
      }
    }
    return result;
  }
  return false;
}

bool IPCConnection::read(bool* data) { return read((char*)data, sizeof(bool)); }

bool IPCConnection::write(const bool* data) {
  return write((char*)data, sizeof(bool));
}

IPCConnection::~IPCConnection() { close(); }

bool IPCConnection::health() { return connected; }
