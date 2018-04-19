/*
----
This file is part of SECONDO.
Realizing a simple distributed filesystem for master thesis of stephan scheide

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


//[$][\$]

*/

#include "remotedfs.h"
#include "../commlayer/EndpointClient.h"

using namespace dfs::remote;
using namespace dfs::comm;
using namespace dfs;
using namespace std;

void RemoteFilesystem::receiveFileToLocal(FILEID fileId, FILEPATH localPath) {
  ToStrSerializer ser;
  ser.appendRaw("read");
  ser.append(fileId);
  Str r = syncm(ser.output);
  assertResponse(r);

  IndexEntry entry = IndexEntry::deserialize(r.substr(4));
  unsigned long long fileSize = entry.calculateFileSize();


  dfs::io::file::allocate(localPath, fileSize);

  io::file::Writer fileWriter(localPath, false);

  //Teile der Datei nun anfordern
  for (int i = 0; i < entry.chunkInfoListLength; i++) {
    ChunkInfo *pChunkInfo = &entry.chunkInfoList[i];

    loadChunkContentFromDataNode(pChunkInfo, fileWriter);
  }

  fileWriter.close();
}

bool containsChunkFilePart(ChunkInfo *chunk, UI64 index, UI64 length,
                           UI64 *outOffsetInChunk, UI64 *outBytesPresentInChunk,
                           UI64 bytesConsumed) {

  //byteindices of file mentioned in the chunk
  UI64 cs = chunk->offsetInFile;
  UI64 ce = cs + chunk->length - 1;

  UI64 end = index + length - 1;

  //are there bytes which are needed
  if (numberUtils::hasIntersect(cs, ce, index, end)) {

    //cout << "index " << index << " cs " << cs << endl;

    if (index < cs) {
      *outOffsetInChunk = 0;
      UI64 l = length - bytesConsumed;
      *outBytesPresentInChunk = l > chunk->length ? chunk->length : l;
    }

    if (index == cs) {
      *outOffsetInChunk = 0;
      *outBytesPresentInChunk = length > chunk->length ? chunk->length : length;
    }

    if (index > cs) {
      UI64 x = index - cs;
      *outOffsetInChunk = x;
      UI64 l = length - bytesConsumed;
      //cout << "length " << length << endl;
      //cout << "bytesConsumed " << bytesConsumed << endl;
      if (l > chunk->length) l = chunk->length;
      //cout << "l " << l << endl;
      //l -= cs;
      //cout << "l " << l << endl;
      *outBytesPresentInChunk = l;
    }

    return true;

  }

  return false;
}

bool RemoteFilesystem::receiveBytesFromDataNodeDirectly(const URI &dataNodeUri,
                                                        const Str &localChunkId,
                                                        UI64 offsetInChunk,
                                                        UI64 length,
                                                        char *buffer) {

  ToStrSerializer ser;
  ser.appendRaw("byte");
  ser.append(localChunkId);
  ser.appendUInt64(offsetInChunk);
  ser.appendUInt64(length);

  Str response;
  if (sendRequestToDataNodeKilling(dataNodeUri, ser.output, &response)) {
    StrReader reader(&response);
    reader.setPos(4);
    UI64 realContentLength = reader.readUInt64();
    char* realContent = reader.pointerToCurrentRawBuf();

    memcpy(buffer, realContent, realContentLength);
    return true;
  }

  return false;
}

void RemoteFilesystem::receiveFilePartially(const char *fileId,
                                            unsigned long startIndex,
                                            unsigned long length,
                                            char *targetBuffer,
                                            unsigned long
                                            targetBufferStartIndex) {

  //ask index which chunk and locations would fit
  ToStrSerializer ser;
  ser.appendRaw("read");
  ser.append(fileId);
  Str r = syncm(ser.output);
  assertResponse(r);

  IndexEntry entry = IndexEntry::deserialize(r.substr(4));

  int cl = entry.chunkInfoListLength;

  UI64 offsetInChunk = 0;
  UI64 bytesPresentInChunk = 0;
  UI64 bytesConsumed = 0;

  UI64 runningOutputOffset = 0;

  for (int i = 0; i < cl; i++) {
    ChunkInfo *chunkInfo = entry.chunkInfoList + i;

    if (containsChunkFilePart(chunkInfo, startIndex, length, &offsetInChunk,
                              &bytesPresentInChunk, bytesConsumed)) {
      bytesConsumed += bytesPresentInChunk;
      cout << "from chunk with order " << chunkInfo->order << " we need ("
           << offsetInChunk << "," << bytesPresentInChunk << ")" << endl;

      char *bytes = new char[bytesPresentInChunk];
      bzero(bytes, bytesPresentInChunk);
      this->receiveBytesFromDataNodeFailSafe(chunkInfo, offsetInChunk,
                                             bytesPresentInChunk, bytes);

      memcpy(targetBuffer + runningOutputOffset, bytes, bytesPresentInChunk);
      runningOutputOffset += bytesPresentInChunk;
      delete[] bytes;
    }
  }

}

void RemoteFilesystem::receiveBytesFromDataNodeFailSafe(ChunkInfo *info,
                                                        UI64 offsetInChunk,
                                                        UI64 length,
                                                        char *buffer) {

  int locationLength = info->chunkLocationListLength;
  int *indices = numberUtils::findPermutationOfListIndices(locationLength);

  for (int i = 0; i < locationLength; i++) {
    int locationIndex = indices[i];
    ChunkLocation *pLoc = (info->chunkLocationList + locationIndex);
    if (this->receiveBytesFromDataNodeDirectly(pLoc->dataNodeUri, pLoc->chunkId,
                                               offsetInChunk, length, buffer)) {
      break;
    }
  }

}
