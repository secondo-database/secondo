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

IPCInit::IPCInit() : magicId(0), confirmIdx(0), nextIdx(0) {}

IPCLoopBuffer::IPCLoopBuffer()
    : magicId(0),
      bufferSize(IPC_QUEUE_BUFFER_SIZE),
      readIndex(0),
      writeIndex(0),
      totalWritten(0),
      totalRead(0) {}

bool IPCLoopBuffer::avail() { return totalWritten != totalRead; }

bool IPCLoopBuffer::read(char* data, unsigned int n, double timeout) {
  int readed = 0;  // ;)
  time_t lastWrite = time(NULL);
  while (true) {
    int available;

    int tempWriteIndex = writeIndex;
    if (readIndex < tempWriteIndex) {
      available = tempWriteIndex - readIndex;
    } else {
      if (totalWritten == totalRead) {
        available = 0;
      } else {
        available = bufferSize - readIndex;
      }
    }

    if (available == 0) {
      //      if(difftime(time(NULL), lastWrite) > timeout) {
      //        return false;
      //      } else {
      //        //boost::this_thread::sleep( boost::posix_time::milliseconds(10)
      //        );
      //        continue;
      //      }
      continue;
    }

    if (static_cast<int>(n) <= available) {
      memcpy(data + readed, buffer + readIndex, n);

      totalRead += n;
      readIndex += n;
      if (readIndex == bufferSize) {
        readIndex = 0;
      }

      // cout<<"Read: readIndex:"<<readIndex<<" writeIndex:"<<writeIndex<<"
      // totalRead:"<<totalRead<<" totalWritten:"<<totalWritten<<endl;
      return true;
    } else {
      memcpy(data + readed, buffer + readIndex, available);

      totalRead += available;
      readIndex += available;
      if (readIndex == bufferSize) {
        readIndex = 0;
      }

      n -= available;
      readed += available;
    }
    lastWrite = time(NULL);
  }
}

bool IPCLoopBuffer::write(const char* data, unsigned int n, double timeout) {
  int written = 0;
  time_t lastRead = time(NULL);
  while (true) {
    int space;

    int tempReadIndex = readIndex;
    if (writeIndex > tempReadIndex ||
        (writeIndex == tempReadIndex && totalWritten == totalRead)) {
      space = bufferSize - writeIndex;
    } else {
      space = tempReadIndex - writeIndex;
    }

    if (space == 0) {
      //      if(difftime(time(NULL),lastRead) > timeout) {
      //        return false;
      //      } else {
      //        //boost::this_thread::sleep( boost::posix_time::milliseconds(10)
      //        );
      //        continue;
      //      }
      continue;
    }

    if (static_cast<int>(n) <= space) {
      memcpy(buffer + writeIndex, data + written, n);

      totalWritten += n;
      writeIndex += n;
      if (writeIndex == bufferSize) {
        writeIndex = 0;
      }

      // cout<<"Write: readIndex:"<<readIndex<<" writeIndex:"<<writeIndex<<"
      // totalRead:"<<totalRead<<" totalWritten:"<<totalWritten<<endl;
      return true;
    } else {
      memcpy(buffer + writeIndex, data + written, space);

      totalWritten += space;
      writeIndex += space;
      if (writeIndex == bufferSize) {
        writeIndex = 0;
      }

      n -= space;
      written += space;
    }
    lastRead = time(NULL);
  }
}

bool IPCConnection::avail() {
  return readBuffer->totalWritten != readBuffer->totalRead;
}

bool IPCConnection::read(char* data, unsigned int n) {
  if (!timedout) {
    if (n > 0) {
      if (readBuffer->read(data, n, timeout)) {
        return true;
      } else {
        timedout = true;
        return false;
      }
    } else {
      return true;
    }
  } else {
    return false;
  }
}

bool IPCConnection::write(const char* data, unsigned int n) {
  if (!timedout) {
    if (n > 0) {
      if (writeBuffer->write(data, n, timeout)) {
        return true;
      } else {
        timedout = true;
        return false;
      }
    } else {
      return true;
    }
  } else {
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
    char* temp = new char[len];
    if (read(temp, len)) {
      data->assign(temp, len);
      delete temp;
      return true;
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

bool IPCConnection::read(bool* data) { return read((char*)data, sizeof(bool)); }

bool IPCConnection::write(const bool* data) {
  return write((char*)data, sizeof(bool));
}

IPCConnection::~IPCConnection() { close(); }

bool IPCConnection::health() { return connected && !timedout; }
