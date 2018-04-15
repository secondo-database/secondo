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

#include "DataNodeClientHandler.h"
#include "../shared/io.h"
#include <iostream>
#include <stdio.h>

using namespace dfs;
using namespace dfs::io::file;

bool doCrc = false;

void DataNodeClientHandler::onStart() {
  current = 0;
  state = 2;
}

Str DataNodeClientHandler::onReceived(Str *s, int *resultFlags) {

  bool canDebug = this->logger->canDebug;

  if (canDebug) {
    _dr("DataNodeClientHandler.onReceived");
    _dr(Str("value <current> is ").append(current));
    _dr(Str("got buffer ").append(s->len()));
  }

  //wir befinden uns bereits in einer inhaltsuebertragung
  if (current == 1 || current == 2) {
    return "0000";
  }

  if (s->len() < 4) {
    return "8888";
  }

  Str cmd = s->substr(0, 4);
  _dr("cmd " + cmd);

  if (canDebug) {
    _dr("************");
    _dr(Str("*** ").append(cmd).append(" ***"));
    _dr("************");
  }

  if (cmd == "echo") {
    return Str("0000").append(s->substr(4));
  } else if (cmd == "quit") {
    *resultFlags = 2;
    dataNode->state()->markAsDirty();
    return Str("0000");
  } else if (cmd == "conf") {
    ToStrSerializer ser;
    ser.appendRaw("0000");
    return Str("0000");
  } else if (cmd == "stat") {
    ToStrSerializer ser;
    ser.appendRaw("0000");
    int used = dataNode->state()->usedChunkCount();
    ser.appendUInt64(used);
    ser.appendUInt64(dataNode->state()->sizeOfAllChunks());

    for (int i = 0; i < used; i++) {
      Chunk *pChunk = dataNode->state()->findChunkByIndex(i);
      if (pChunk) {
        ser.append(pChunk->chunkname);
        ser.append(pChunk->category);
      }
    }

    return ser.output;
  }

    //get bytes from chunk
  else if (cmd == "byte") {

    StrReader reader(s);
    reader.setPos(4);

    Str chunkId = reader.readStrSer();
    UI64 offset = reader.readUInt64();
    UI64 length = reader.readUInt64();

    Chunk *pChunk = dataNode->state()->findChunkByName(chunkId);
    if (pChunk == 0) return ("1000 chunk not found");

    Str dataFilePath = pChunk->mapToDataPath(dataNode->config.dataDir);
    _dr(Str("loading data from this file ").append(dataFilePath));
    _dr(Str("starting index ").append(offset).append(" bytes to read ").append(
      length));


    UI64 lastIndex = offset + length;
    if (lastIndex > pChunk->chunksize) {
      length = pChunk->chunksize - offset;
      _dr(Str("new bytes to read is ").append(length));
    }

    FILE *fp = fopen(CStr(dataFilePath).cstr(), "r");
    char *buffer = new char[length];
    memset(buffer, 0, length);
    fseek(fp, offset, SEEK_SET);
    fread(buffer, length, 1, fp);
    fclose(fp);

    Str strBuffer(buffer, length);
    delete[] buffer;
    ToStrSerializer ser;
    ser.appendRaw("0000");
    ser.append(strBuffer);
    return ser.output;

  }

    //appends data to a given chunk
    //appd<chunkid><newChunkSize><content>
  else if (cmd == "appc") {

    if (canDebug) _dr("chunk should be appended");

    StrReader reader(s);
    reader.setPos(4);
    Str chunkId = reader.readStrSer();
    UI64 newChunkSize = reader.readUInt64();

    if (canDebug) {
      _dr(Str("information for appending - chunkId,newChunkSize - ").append(
        chunkId).append(",").append(",").append(newChunkSize));
    }

    Str content = reader.readStrSer();
    if (canDebug) {
      _dr(Str("appendix length is - ").append(content.len()));
    }

    if (canDebug) _dr("obtaining chunk");
    Chunk *pChunk = dataNode->state()->findChunkByName(chunkId);
    if (pChunk == 0) return Str("4444ChunkNotFound-").append(chunkId);
    Str filepath = pChunk->mapToDataPath(dataNode->config.dataDir);
    if (canDebug) _dr(Str("got file path - ").append(filepath));
    dfs::io::file::Writer writer(filepath, true);
    writer.append(content);
    writer.close();
    if (canDebug) _dr("appended data to file");
    pChunk->chunksize = newChunkSize;

    if (canDebug) _dr("updated meta info");
    this->dataNode->state()->markAsDirty();

    return Str("0000");
  }

    //deleting single chunk, needs chunkid
  else if (cmd == "dele") {
    StrReader reader(s);
    reader.setPos(4);
    Str chunkId = reader.readStrSer();

    if (canDebug) _dr(Str("deleting chunk ").append(chunkId));
    Chunk *pChunk = dataNode->state()->findChunkByName(chunkId);
    if (pChunk != 0) {

      Str filepath = pChunk->mapToDataPath(dataNode->config.dataDir);
      if (canDebug) _dr(Str("deleting file ").append(filepath));
      dfs::io::file::deleteFile(filepath);

      if (pChunk->category.len() > 0) {
        dfs::io::file::deleteEmptyDirOnlySafe(
          pChunk->mapTargetDirToDataPath(filepath));
      }

      if (canDebug) _dr(Str("deleting chunk from state ").append(chunkId));
      dataNode->state()->deleteChunkById(chunkId);


      if (canDebug) _dr(Str("done deleting"));

    } else {
      if (canDebug) _dr("chunk not found. ignore.");
    }
    return Str("0000");
  } else if (cmd == "dela") {
    if (canDebug) _dr("deleting of all chunks");

    State *state = dataNode->state();

    int chunkCount = state->usedChunkCount();
    Chunk *chunks = state->getChunkList();

    for (int i = 0; i < chunkCount; i++) {
      Chunk *pChunk = chunks + i;
      Str filepath = pChunk->mapToDataPath(dataNode->config.dataDir);
      if (canDebug) _dr(Str("deleting file ").append(filepath));
      dfs::io::file::deleteFile(filepath);

      if (pChunk->category.len() > 0) {
        dfs::io::file::deleteEmptyDirOnlySafe(
          pChunk->mapTargetDirToDataPath(filepath));
      }

    }

    dataNode->state()->deleteAllChunks();
    return Str("0000");
  } else if (cmd == "hash") {
    State *state = dataNode->state();

    int chunkCount = state->usedChunkCount();
    Chunk *chunks = state->getChunkList();
    UI64 hash = chunkCount;
    for (int i = 0; i < chunkCount; i++) {
      int fac = i + 1;
      Chunk *cur = chunks + i;
      hash += fac * (cur->chunksize);
    }

    ToStrSerializer ser;
    ser.appendRaw("0000");
    ser.appendUInt64(hash);
    return ser.output;
  }

    /**
     * parr
     * part register
     * prepare a chunk
     * no data incoming
     * parr<lcat3><cat><lcontent>
     */
  else if (cmd == "parr") {

    StrReader reader(s);
    reader.setPos(4);

    int lcat = reader.readInt(3);
    Str cat = reader.readStr(lcat);
    UI64 lcontent = reader.readUInt64();

    if (canDebug) {
      _dr(Str(
        "parr - part register - prepare new chunk - "
          "(cat,contentLength) - ").append(
        cat).append(",").append(lcontent));
    }

    Chunk *pchunk = dataNode->state()->nextChunk();
    pchunk->chunksize = lcontent;
    pchunk->category = cat;

    ToStrSerializer ser;
    ser.appendRaw(Str("0000"));
    ser.append(pchunk->chunkname);
    if (canDebug) _dr(Str("Str parr - chunk id is ").append(pchunk->chunkname));
    return ser.output;
  }

    /**
     * part content
     * just append content to file identified by chunkid
     * parc<chunkid><lcontent><content>
     */
  else if (cmd == "parc") {
    StrReader reader(s);
    reader.setPos(4);
    Str chunkId = reader.readStrSer();
    UI64 lengthOfContent = reader.readUInt64();

    if (canDebug) {
      _dr(Str("parc - appending data to chunk ").append(chunkId).append(
        " - added bytes length ").append(lengthOfContent));
    }

    Chunk *pChunk = dataNode->state()->findChunkByName(chunkId);
    createDir(pChunk->mapTargetDirToDataPath(dataNode->config.dataDir));
    Str targetFile = pChunk->mapToDataPath(dataNode->config.dataDir);
    if (canDebug) _dr(Str("append the data to file ").append(targetFile));

    dfs::io::file::Writer writer(targetFile, true);
    Str content = reader.readStrSer();
    writer.append(content);
    writer.close();

  }

    //store file
    //part<lcat3><cat><lcontent><content>
  else if (cmd == "part") {

    StrReader reader(s);
    reader.setPos(4);

    int lcat = reader.readInt(3);
    Str cat = reader.readStr(lcat);
    UI64 lcontent = reader.readUInt64();

    int posdata = reader.getPos();

    if (canDebug) {
      _dr(
        "lengthCat " + Str(lcat) + " lengthContent " + Str(lcontent) + " cat " +
        cat);
    }

    //dateiname reservieren
    if (canDebug) _dr("try to get next chunk");
    Chunk *pchunk = dataNode->state()->nextChunk();
    pchunk->chunksize = lcontent;
    pchunk->category = cat;
    if (canDebug) _dr("got next chunk and filled it with meta data");

    Str dataDir = dataNode->config.dataDir;
    if (canDebug) _dr("got datadir from config");

    //verzeichnis mappen
    Str filepath;
    if (lcat > 0) {
      filepath = combinePath(dataDir, cat);
      if (canDebug) _dr("create directory " + filepath);
      createDir(filepath);
      filepath = combinePath(filepath, pchunk->chunkname);
    } else {
      filepath = dfs::io::file::combinePath(dataDir, pchunk->chunkname);
    }

    if (canDebug) _dr("target chunk file path is " + filepath);

    //datei anlegen
    if (canDebug) _dr("writing content to local file system");
    char *csfile = filepath.cstr();
    FILE *fp = fopen(csfile, "w");
    delete[] csfile;
    fwrite(s->buf() + posdata, lcontent, 1, fp);
    fclose(fp);
    if (canDebug) _dr("content has been written to local file system");

    ToStrSerializer ser;
    ser.appendRaw(Str("0000"));
    ser.append(pchunk->chunkname);
    return ser.output;

  } else {
    return "9999unknownCommand";
  }
}

void DataNodeClientHandler::onEnd() {
  current = 0;
  state = 0;
  if (this->dataNode->state()->dirtyToggle()) {
    this->dataNode->state()->dumpToFile(this->dataNode->mapToDir("state"));
  }
}
