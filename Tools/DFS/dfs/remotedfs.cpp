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
#include "../shared/io.h"
#include "../shared/remotemetadata.h"
#include "../shared/numberUtils.h"
#include "../shared/debug.h"
#include <iostream>

using namespace dfs::remote;
using namespace dfs::comm;
using namespace dfs;
using namespace std;

RemoteFilesystem::RemoteFilesystem(URI m, log::Logger *l) {
  fileCopyBuffer = 1024 * 1024;
  indexURI = m;
  this->logger = l;
  this->canDebug = this->logger->canDebug;
}

RemoteFilesystem::~RemoteFilesystem() {

}

void RemoteFilesystem::debug(const Str &s) {
  this->logger->debug(Str("RemoteFilesystem - ").append(s));
}

void RemoteFilesystem::debugMsg(const Str &s) {
  Str x = "message to server: <";
  x = x.append(s).append("> ").append(s.len());
  this->debug(x);
}

void RemoteFilesystem::deleteFile(FILEID fileId) {
  EndpointClient c;

  ToStrSerializer ser;
  ser.appendRaw("dele");
  ser.append(Str(fileId));
  Str r = syncm(ser.output);
  assertResponse(r);
  StrReader reader(&r);
  if (r.len() < 5) return;

  //we get the indexentry with all chunkinfo and chunk locations
  reader.setPos(4);

  IndexEntry e = IndexEntry::deserialize(reader.copyOfReminder());
  for (int i = 0; i < e.chunkInfoListLength; i++) {
    ChunkInfo *pChunk =
      e.chunkInfoList + i; //adding index is easier than [] and then pointer
    for (int j = 0; j < pChunk->chunkLocationListLength; j++) {
      ChunkLocation *pLoc = pChunk->chunkLocationList + j;
      Str chunkId = pLoc->chunkId;
      ToStrSerializer serDataNode;
      serDataNode.appendRaw("dele");
      serDataNode.append(chunkId);
      sendRequestToDataNodeDontCareKilling(pLoc->dataNodeUri,
                                           serDataNode.output);
      //syncmg(pLoc->dataNodeUri,serDataNode.output);
    }
  }

}

void RemoteFilesystem::loadChunkContentFromDataNode(
  const ChunkInfo *chunkInfo,
  io::file::Writer &fileWriter) {

  debug("load chunk");

  NUMBER blockSizeForFetchingData = this->fileCopyBuffer;

  DataNodeCommandBuilder builder;

  int *locIndices = numberUtils::findPermutationOfListIndices(
    chunkInfo->chunkLocationListLength);

  NUMBER offsetInChunk = 0;
  NUMBER bytesFetched = 0;
  NUMBER bytesLeft = chunkInfo->length;

  Str content;
  bool locationIsAlive = true;

  for (int i = 0; i < chunkInfo->chunkLocationListLength; i++) {

    //current location
    int locIndex = locIndices[i];
    ChunkLocation *pLoc = &chunkInfo->chunkLocationList[locIndex];

    while (bytesFetched < chunkInfo->length) {

      //ask for bytes
      NUMBER lengthForGetBytes = bytesLeft >= blockSizeForFetchingData ?
                                 blockSizeForFetchingData : bytesLeft;
      Str cmd = builder.getBytes(pLoc->chunkId, offsetInChunk,
                                 lengthForGetBytes);

      //cout << cmd << endl;

      //good case - we got the data from the current location
      if (sendRequestToDataNodeKilling(pLoc->dataNodeUri, cmd, &content)) {

        debug(Str("got content - Length").append(content.len()));

        StrReader reader(&content);
        reader.setPos(4);
        UI64 contentLength = reader.readUInt64();
        char *pointerToContent = reader.pointerToCurrentRawBuf();

        //cout << offsetInChunk << " " << chunkInfo->offsetInFile << endl;

        fileWriter.writeBufferAt(chunkInfo->offsetInFile+offsetInChunk,
                                 contentLength,
                                 pointerToContent);

        offsetInChunk += contentLength;
        bytesFetched += contentLength;
        bytesLeft -= contentLength;


      } else {
        locationIsAlive = false;
      }

      if (!locationIsAlive) continue;

    }


  }

}


bool RemoteFilesystem::hasFile(FILEID fileId) {
  ToStrSerializer ser;
  ser.appendRaw("hasf");
  ser.append(Str(fileId));
  Str r = syncm(ser.output);
  assertResponse(r);
  return r[4] == '1';
}

void RemoteFilesystem::renameFile(const char *currentFileId,
                                  const char *newFileId) {

  ToStrSerializer ser;
  ser.appendRaw("rena");
  ser.append(Str(currentFileId));
  ser.append(Str(newFileId));
  Str r = syncm(ser.output);
  assertResponse(r);

}

bool RemoteFilesystem::hasFeature(FEATURE name) {
  Str msg = Str("?   ");
  //result ist CCCC1|0
  Str r = syncm(msg);
  assertResponse(r);
  return r.substr(4) == Str("1");
}

int RemoteFilesystem::countFiles() {
  Str msg = Str("cofi");
  Str r = syncm(msg);
  assertResponse(r);
  //result ist CCCCX where X is count as int
  return r.substr(4).toInt();
}

UI64 RemoteFilesystem::totalSize() {
  Str msg = Str("sizt");
  Str r = syncm(msg);
  assertResponse(r);
  return r.substr(4).toUInt64();
}

void RemoteFilesystem::deleteAllFiles() {
  Str msg = Str("dela");
  Str r = syncm(msg);
  assertResponse(r);
  if (r.len() < 5) return;

  StrReader reader(&r);
  reader.setPos(4);
  UI64 countUris = reader.readUInt64();

  for (int i = 0; i < countUris; i++) {
    Str uriStr = reader.readStrSer();
    URI uri = URI::fromString(uriStr);
    sendRequestToDataNodeDontCareKilling(uri, "dela");
  }

}

UI64 RemoteFilesystem::deleteAllFilesOfCategory(CATEGORY c) {
  ToStrSerializer ser;
  ser.appendRaw("dfac");
  ser.append(c);
  Str r = syncm(ser.output);
  assertResponse(r);
  StrReader reader(&r);
  reader.setPos(4);
  return reader.readUInt64();
}


Str RemoteFilesystem::echo(const Str &s) {
  Str msg = Str("echo").append(s);
  Str r = syncm(msg);
  assertResponse(r);
  return r.substr(4);
}

std::vector<std::string> RemoteFilesystem::listFileNames() {

  Str msg = Str("list");
  Str r = syncm(msg);
  assertResponse(r);

  StrReader reader(&r);
  reader.setPos(4);

  vector<std::string> result; //IMPROVE optimize init capacity

  long amount = reader.readLong(14);
  for (long i = 0; i < amount; i++) {
    Str name = reader.readStrSer();
    CStr cs(name);
    result.push_back(cs.cstr());
  }

  return result;
}


Str RemoteFilesystem::syncm(const Str &msg) {
  debugMsg(msg);
  return syncMessageToURI(indexURI, msg);
}

bool
RemoteFilesystem::sendRequestToDataNodeDontCareKilling(const URI &dataNodeUri,
                                                       const Str &request) {
  bool success = false;
  try {
    this->syncMessageToURI(dataNodeUri, request);
    success = true;
  }
  catch (dfs::BaseException &be) {
    cerr << "baseException " << be.what() << endl;
  }
  catch (...) {
    cerr << "unknown error";
  }
  if (!success) {
    this->markDataNodeAsErrornous(dataNodeUri);
  }
  return success;
}

bool RemoteFilesystem::sendRequestToDataNodeKilling(const URI &dataNodeUri,
                                                    const Str &request,
                                                    Str *response) {
  bool success = false;
  try {
    *response = this->syncMessageToURI(dataNodeUri, request);
    success = true;
  }
  catch (dfs::BaseException &be) {
    cerr << "baseException " << be.what() << endl;
  }
  catch (...) {
    cerr << "unknown error";
  }
  if (!success) {
    this->markDataNodeAsErrornous(dataNodeUri);
  }
  return success;
}

void RemoteFilesystem::markDataNodeAsErrornous(const URI &dataNodeUri) {
  cerr << "markDataNodeAsErrornous" << endl;
  ToStrSerializer ser;
  ser.appendRaw("kill");
  ser.append(dataNodeUri.toString());
  syncm(ser.output);
}

Str RemoteFilesystem::syncMessageToURI(const URI &uri, const Str &msg) {
  //debug(Str("send message to uri ").append(uri.toString()));
  //debug(Str("message content - ").append(msg));
  EndpointClient c;

  c.setLogger(logger);
  bool doEnvelope = msg.len() > 0 && msg[0] != '@';
  Str r = c.sendSyncMessage(uri, msg, doEnvelope);
  return r;
}

void RemoteFilesystem::assertResponse(const Str &s) {
  if (s.len() < 4) {
    throw ResultException("response from remote too short");
  }
  Str c = s.substr(0, 4);
  if (c != Str("0000") && c != Str("0004")) {
    throw ResultException("response from remote contains errors");
  }
}

void RemoteFilesystem::registerDataNode(const URI &uri) {
  ToStrSerializer ser;
  ser.appendRaw("dnrg");
  ser.append(uri.toString());
  Str r = syncm(ser.output);
  assertResponse(r);
}

void RemoteFilesystem::changeSetting(const char *key, const char *value) {
  ToStrSerializer ser;
  ser.appendRaw("sett");
  ser.append(Str(key));
  ser.append(Str(value));
  Str r = syncm(ser.output);
  assertResponse(r);
}

void RemoteFilesystem::changeChunkSize(int newSize) {
  char *value = Str(newSize).cstr();
  changeSetting("chunksize", value);
  delete[] value;
}

UI64 RemoteFilesystem::nextWritePosition(const char *fileId) {
  return this->fileSize(fileId);
}

UI64 RemoteFilesystem::fileSize(const char *fileId) {
  ToStrSerializer ser;
  ser.appendRaw("fsiz");
  ser.append(Str(fileId));
  Str r = syncm(ser.output);
  assertResponse(r);
  StrReader reader(&r);
  reader.setPos(4);
  return reader.readUInt64();
}

Str DataNodeCommandBuilder::getBytes(const Str &chunkId, NUMBER offsetInChunk,
                                     NUMBER length) {
  ToStrSerializer ser;
  ser.appendRaw("byte");
  ser.append(chunkId);
  ser.appendUInt64(offsetInChunk);
  ser.appendUInt64(length);
  return ser.output;
}

void RemoteFilesystem::triggerIndexBackup() {
  ToStrSerializer ser;
  ser.appendRaw("back");
  syncm(ser.output);
}

void RemoteFilesystem::quitWholeCluster() {
  ToStrSerializer ser;
  ser.appendRaw("quia");
  syncm(ser.output);
}
