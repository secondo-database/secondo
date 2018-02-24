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
using namespace std;
void RemoteFilesystem::appendToFile(FILEID fileId, FILEBUFFER appendix,
                                    long length) {
  if (!this->hasFile(fileId)) {
    this->storeFullBufferFromMemory(fileId, appendix, length);
    return;
  }

  this->appendToExistingFile(fileId, appendix, length);
}

void RemoteFilesystem::appendToExistingFile(FILEID fileId, FILEBUFFER appendix,
                                            long length) {

  bool canDebug = this->logger->canDebug;

  //we send appd<file><length> to index
  //we will get the info where to send content
  ToStrSerializer ser;
  ser.appendRaw("appd");
  ser.append(fileId);
  ser.appendUInt64(length);

  Str r = syncm(ser.output);
  assertResponse(r);

  StrReader reader(&r);
  reader.setPos(4);
  UI64 count = reader.readUInt64();

  for (UI64 i = 0; i < count; i++) {
    Str verb = reader.readStrSer();

    if (canDebug) debug(Str("got verb ").append(verb));

    if (verb == "donewchunk") {

      Str uri = reader.readStrSer();
      UI64 bufferOffset = reader.readUInt64();
      UI64 bufferLengthToSend = reader.readUInt64();
      UI64 order = reader.readUInt64();
      Str category = reader.readStrSer();

      //build command for datanode
      ToStrSerializer cmd;
      cmd.appendRaw("part");
      if (category.len() > 0) {
        cmd.append(category.len(), 3);
        cmd.appendRaw(category);
      } else {
        cmd.appendRaw("000");
      }
      cmd.appendUInt64(bufferLengthToSend);
      cmd.appendRaw(Str(appendix + bufferOffset, bufferLengthToSend));

      //send command to data node
      URI uriDataNode = URI::fromString(uri);
      Str response;
      if (sendRequestToDataNodeKilling(uriDataNode, cmd.output, &response)) {
        //Str response = syncmg(uriDataNode,cmd.output);
        assertResponse(response);

        //result is new chunk name
        StrReader readerDataNode(&response);
        readerDataNode.setPos(4);
        Str localChunkId = readerDataNode.readStrSer();
        this->registerLocalChunkIdToIndex(fileId, uriDataNode, order,
                                          localChunkId);
      }
    } else if (verb == "doappendtochunk") {
      Str uri = reader.readStrSer();
      Str chunkId = reader.readStrSer();
      UI64 bufferOffset = reader.readUInt64();
      UI64 bufferLengthToSend = reader.readUInt64();
      UI64 newTotalSize = reader.readUInt64();
      UI64 newChunkSize = reader.readUInt64();

      URI uriDataNode = URI::fromString(uri);
      ToStrSerializer serDataNode;
      serDataNode.appendRaw("appc");
      serDataNode.append(chunkId);
      serDataNode.appendUInt64(newChunkSize);
      serDataNode.append(Str(appendix, length));


      Str response;
      //Str response = syncmg(uriDataNode,serDataNode.output);
      if (sendRequestToDataNodeKilling(uriDataNode, serDataNode.output,
                                       &response)) {
        assertResponse(response);
      }
    }

  }
  this->triggerIndexBackup();

}

void RemoteFilesystem::storeFile(FILEID fileId, FILEBUFFER content, long length,
                                 CATEGORY c) {
  storeFullBufferFromMemory(fileId, content, length, c);
}

Str getRequestForStorageNewFile(FILEID fileId, long contentLength,
                                CATEGORY category) {
  ToStrSerializer serIndex;
  serIndex.appendRaw("stor");
  serIndex.appendUInt64(contentLength);
  //serIndex.append(contentLength,12);
  serIndex.append(Str(fileId));
  if (category != 0) {
    serIndex.append(Str(category));
  } else {
    serIndex.appendEmptyStr();
  }
  return serIndex.output;
}

void
RemoteFilesystem::storeFullBufferFromMemory(FILEID fileId, FILEBUFFER content,
                                            long contentLength,
                                            CATEGORY category) {

  //delete existing file
  this->deleteFile(fileId);

  debug("storeFileFromLocal begin");
  Str msg = getRequestForStorageNewFile(fileId, contentLength, category);

  debug("storeFileFromLocal send storage request message to index");
  Str r = syncm(msg);
  debug(Str("storeFileFromLocal - result from index node - ").append(r));
  assertResponse(r);

  IndexEntry indexEntry = IndexEntry::deserialize(r.substr(4));

  debug(Str("storeFileFromLocal - fileid ").append(indexEntry.fileId));

  int pic = indexEntry.chunkInfoListLength;
  debug(Str("Anzahl Teile - ").append(pic));

  for (int p = 0; p < pic; p++) {

    ChunkInfo *pci = &indexEntry.chunkInfoList[p];

    long order = pci->order;
    long offset = pci->offsetInFile;
    long length = pci->length;

    debug(Str("order, offsetInFile, length").append(order).append(
      " ").append(offset).append(" ").append(length));

    int locationListLength = pci->chunkLocationListLength;
    for (int d = 0; d < locationListLength; d++) {
      ChunkLocation *pcl = &pci->chunkLocationList[d];
      URI uri = pcl->dataNodeUri;
      debug((Str("save partnumber ").append(order).append(
        "with size ").append(length).append(" at URL ").append(
        uri.toString())));

      //cmd for datanode
      //part<length:ui64><lcat:3stellig>
      ToStrSerializer serIndex;
      serIndex.appendRaw("part");

      if (category != 0) {
        serIndex.append(strlen(category), 3);
        serIndex.appendRaw(category);
      } else {
        serIndex.appendRaw("000");
      }

      //Wir schicken den (Teilinhalt) der Datei an alle Datenknoten
      char *partialContent = new char[length];
      memcpy(partialContent, (void *) (content + offset), length);

      serIndex.appendUInt64(length);
      serIndex.appendRaw(Str(partialContent, length));

      //Str result = syncmg(uri,serIndex.output);
      Str result;
      if (this->sendRequestToDataNodeKilling(uri, serIndex.output, &result)) {
        delete[] partialContent;
        assertResponse(result);

        //wir haben eine Chunk-Kennung erhalten
        StrReader readerStore(&result);
        readerStore.setPos(4);
        Str chunk = readerStore.readStrSer();
        debug(Str("got chunk is ").append(chunk));

        //chunk dem master bekannt machen
        //chun(fileId,chunkId,dataNodeUri,order)
        this->registerLocalChunkIdToIndex(indexEntry.fileId, uri, order, chunk);
      }

    }
  }

  this->triggerIndexBackup();
}

void
RemoteFilesystem::registerLocalChunkIdToIndex(const Str &fileId, const URI &uri,
                                              long order,
                                              const Str &localChunkId) {
  ToStrSerializer ser;
  ser.appendRaw("chun");
  ser.append(fileId);
  ser.append(localChunkId);
  ser.append(uri.toString());
  ser.appendDefaultUnsigned(order);
  syncm(ser.output);
}

void RemoteFilesystem::storeFileFromLocal(FILEID fileId, FILEPATH localPath,
                                          CATEGORY category) {

  this->deleteFile(fileId);

  long contentLength = io::file::fileSize(localPath);
  int bufsize = this->fileCopyBuffer;
  int amountRead = 0;

  FILE *fp = fopen(localPath, "rb");
  char *fileReadBuffer = new char[bufsize];

  // reserve indexentry on index server
  Str msg = getRequestForStorageNewFile(fileId, contentLength, category);
  Str r = syncm(msg);
  IndexEntry indexEntry = IndexEntry::deserialize(r.substr(4));

  int pic = indexEntry.chunkInfoListLength;
  for (int p = 0; p < pic; p++) {

    ChunkInfo *pci = &indexEntry.chunkInfoList[p];

    long order = pci->order;
    long offset = pci->offsetInFile;
    long length = pci->length;

    int locationListLength = pci->chunkLocationListLength;
    for (int d = 0; d < locationListLength; d++) {

      ChunkLocation *pcl = &pci->chunkLocationList[d];
      URI uri = pcl->dataNodeUri;

      bool firstDataToThisLocation = true;
      bool useThisLocation = false;

      // create a new chunk at data node
      Str chunkId;
      if (firstDataToThisLocation) {
        firstDataToThisLocation = false;
        ToStrSerializer ser;
        ser.appendRaw("parr");
        if (category != 0) {
          ser.append(strlen(category), 3);
          ser.appendRaw(category);
        } else {
          ser.appendRaw("000");
        }
        ser.appendUInt64(length);
        Str result;
        if (this->sendRequestToDataNodeKilling(uri, ser.output, &result)) {
          assertResponse(result);
          StrReader readerStore(&result);
          readerStore.setPos(4);
          chunkId = readerStore.readStrSer();
          if (canDebug) debug(Str("got new chunk ").append(chunkId));
          this->registerLocalChunkIdToIndex(indexEntry.fileId, uri, order,
                                            chunkId);
          useThisLocation = true;
        }
      }

      //read content from file and put it as data to the data nodes chunk
      if (useThisLocation) {
        fseek(fp, pci->offsetInFile, SEEK_SET);

        UI64 bytesAlreadyTransfered = 0;
        UI64 bytesToBeTransfered = 0;
        UI64 bufSizeToUse = bufsize;
        if (bufSizeToUse > length) bufSizeToUse = length;

        while ((amountRead = fread(fileReadBuffer, 1, bufSizeToUse, fp)) > 0) {
          bytesToBeTransfered += amountRead;

          UI64 bytesToSend = amountRead;
          if (bytesToBeTransfered > length) {
            bytesToSend = bytesToBeTransfered - length;
          }

          ToStrSerializer ser;
          ser.appendRaw("parc");
          ser.append(chunkId);
          ser.appendUInt64(bytesToSend);
          ser.append(Str(fileReadBuffer, bytesToSend));

          Str result;
          this->sendRequestToDataNodeKilling(uri, ser.output, &result);

          bytesAlreadyTransfered += bytesToSend;
          if (bytesAlreadyTransfered >= length) break;
        }
      }

    }

  }
  fclose(fp);

  delete[] fileReadBuffer;
  this->triggerIndexBackup();
}

void
RemoteFilesystem::appendToFileFromLocalFile(FILEID fileId, FILEPATH localPath) {
  long filesize = io::file::fileSize(localPath);
  char *content = io::file::getFileContent(localPath);
  appendToFile(fileId, content, filesize);
}
