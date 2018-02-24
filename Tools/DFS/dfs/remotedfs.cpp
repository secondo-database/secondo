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
#include <iostream>

using namespace dfs::remote;
using namespace dfs::comm;
using namespace dfs;

RemoteFilesystem::RemoteFilesystem(URI m, log::Logger *l) {
  fileCopyBuffer = 1024 * 1024 * 2; //FIXME ueber Werkzeug
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

  debug("Lade Chunk");

  NUMBER blockSizeForFetchingData = 4096; //FIXME
  DataNodeCommandBuilder builder;

  int *locIndices = numberUtils::findPermutationOfListIndices(
    chunkInfo->chunkLocationListLength);

  for (int i = 0; i < chunkInfo->chunkLocationListLength; i++) {
    int locIndex = locIndices[i];

    ChunkLocation *pLoc = &chunkInfo->chunkLocationList[locIndex];
    NUMBER offsetInChunk = 0;
    Str cmd = builder.getBytes(pLoc->chunkId, offsetInChunk,
                               blockSizeForFetchingData);
    debug(cmd);

    Str content;
    if (sendRequestToDataNodeKilling(pLoc->dataNodeUri, cmd, &content)) {
      debug(Str("Inhalt erhalten - Length").append(content.len()));
      debug(content);

      StrReader reader(&content);
      reader.setPos(4);
      Str byteContent = reader.readStrSer();

      fileWriter.writeBufferAt(chunkInfo->offsetInFile, byteContent.len(),
                               byteContent.buf());
      break;
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

  vector<std::string> result; //FIXME optimize init capacity

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
  debug(Str("send message to uri ").append(uri.toString()));
  debug(Str("message content - ").append(msg));
  EndpointClient c;
  c.setLogger(logger);
  Str r = c.sendSyncMessage(uri, msg);
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
