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
#include "IndexNode.h"

#include <iostream>
#include <unistd.h>
#include "IndexNodeManager.h"
#include "../commlayer/comm.h"
#include "../shared/log.h"
#include "DataNodeEntry.h"

using namespace std;
using namespace dfs;

IndexNodeManager man;
dfs::log::Logger *globalLogger;
dfs::log::Logger *auditLogger;

Str result(const char *code, const Str &x) { return Str(code, 4).append(x); }

Str success(const Str &x) { return result("0000", x); }

Str success() { return result("0000", ""); }

void debug(const Str &s) {
  globalLogger->debug(Str("IndexNode.handleMessage ").append(s));
}

void audit(const Str &s) {
  auditLogger->info(s);
}

Str handleMessage(const Str *i, int *flags) {

  bool canDebug = globalLogger->canDebug;

  *flags = 0;

  if (i == 0 || i->len() < 4) {
    return "8888malformed command";
  }

  //sleep(1);

  Str cmd = i->substr(0, 4);
  Str netto = i->substr(4);

  if (canDebug) {
    debug("************");
    debug(Str("*** ").append(cmd).append(" ***"));
    debug("************");
    debug(Str("indexnode.handleMessage: (command) (total length) ").append(
      cmd).append(" ").append(i->len()));
  }

  //store a file
  //s   $size$fileid
  if (cmd == "stor") {
    int cd = man.countDataNodes();
    if (cd < 1) {
      return "7777noDataNodes";
    }

    StrReader reader(&netto);
    UI64 size = reader.readUInt64();
    Str fid = reader.readStrSer();
    Str category = reader.readStrSer();

    if (canDebug) {
      debug(Str("stor - file save (id,size) ").append(fid).append(" ").append(
        size));
    }

    IndexEntry e;

    if (size > man.config.maxChunkSize) {

      if (canDebug) debug("file needs to be split in severals chunks");
      long maxChunkSize = man.config.maxChunkSize;
      long amountOfChunks = size / maxChunkSize + (size % maxChunkSize > 0);

      if (canDebug)
        debug(Str("amount of needed chunks ").append(amountOfChunks));

      e = IndexEntry::create(fid, amountOfChunks, category);

      long bytesLeft = size;
      long currentStartByte = 0;
      long order = 0;

      while (bytesLeft > 0) {
        ChunkInfo *pci = &e.chunkInfoList[order];
        pci->order = order++;
        pci->offsetInFile = currentStartByte;
        pci->length = bytesLeft >= maxChunkSize ? maxChunkSize : bytesLeft;
        currentStartByte += pci->length;
        bytesLeft -= maxChunkSize;
        if (canDebug)
          debug(Str("Chunk registered Order,OffsetInFile,Laenge,BytesLeft")
                  .append(pci->order)
                  .append(",")
                  .append(pci->offsetInFile)
                  .append(",")
                  .append(pci->length)
                  .append(",")
                  .append(bytesLeft)
          );

        if (canDebug) debug("accquire information about data nodes");

        //we need some dataNode for saving chunks
        vector<DataNodeEntry> vn = man.needDataNodes();
        int vns = vn.size();
        if (canDebug) debug("DataNodes fetched");

        pci->allocateLocations(vns);
        if (canDebug) debug("added data nodes to indexEntry");

        for (int i = 0; i < vns; i++) {
          pci->chunkLocationList[i].dataNodeUri = vn[i].uri;
          e.chunkInfoList[0].chunkLocationList[i].dataNodeUri = vn[i].uri;
        }
      }

      man.storeFile(e);

      ToStrSerializer ser;
      e.serializeTo(ser);
      Str eser = ser.output;
      if (canDebug) debug("index entry serialized");
      audit(Str("file has been added - size and name ").append(size).append(
        " - ").append(fid));

      return success(eser);
    } else {

      if (canDebug) debug("file fits within one chunk");

      //we need some dataNode for saving chunks
      vector<DataNodeEntry> vn = man.needDataNodes();
      int vns = vn.size();

      if (canDebug) debug("creating indexEntry...");
      e = IndexEntry::createForOneChunkFile(fid, category);
      e.chunkInfoList[0].allocateLocations(vns);
      e.chunkInfoList[0].length = size;
      e.chunkInfoList[0].offsetInFile = 0;
      e.chunkInfoList[0].order = 0;

      for (int i = 0; i < vns; i++) {
        e.chunkInfoList[0].chunkLocationList[i].dataNodeUri = vn[i].uri;
        if (canDebug)
          debug(Str("using uri ").append(
            e.chunkInfoList[0].chunkLocationList[i].dataNodeUri.toString()));
      }

      if (canDebug) debug("index entry created");

      man.storeFile(e);
    }

    ToStrSerializer ser;
    e.serializeTo(ser);
    Str eser = ser.output;

    if (canDebug) debug(Str("indexEntry serialized - ").append(eser));
    audit(Str("file has been added - size and name ").append(size).append(
      " - ").append(fid));

    return success(eser);
  }

    //returns list of all data nodes
  else if (cmd == "node") {
    if (canDebug) debug("list of uris of all data nodes wanted");
    vector<URI> uris = man.dataNodeIndex.allURIs();
    ToStrSerializer ser;
    ser.appendRaw("0000");
    ser.appendUInt64(uris.size());
    for (UI64 i = 0; i < uris.size(); i++) {
      ser.append(uris.at(i).toString());
    }
    return ser.output;
  }

    //marks a node as broken
  else if (cmd == "kill") {
    StrReader reader(&netto);
    Str dataNodeUriStr = reader.readStrSer();
    if (canDebug) debug(Str("killing data node ").append(dataNodeUriStr));
    bool result = man.markDataNodeAsBroken(dataNodeUriStr);

    if (result)
      globalLogger->warn(
        Str("data node has been killed ").append(dataNodeUriStr));
    else
      globalLogger->warn(
        Str("data node already been killed ").append(dataNodeUriStr));

    return result ? Str("0000") : Str("0004");
  } else if (cmd == "appd") {
    if (canDebug) debug("file content needs to be appended");
    StrReader reader(&netto);
    Str fileId = reader.readStrSer();

    const UI64 lengthToAdd = reader.readUInt64();

    if (canDebug) {
      debug(Str("file id is - ").append(fileId));
      debug(Str("content length to be appended - ").append(lengthToAdd));
    }
    if (lengthToAdd < 1) return Str("9999");

    //get index entry
    IndexEntry *pEntry = man.findFile(fileId);
    if (pEntry == 0) return Str("4444"); //not found FIXME
    const UI64 totalFileSize = pEntry->calculateFileSize();
    const UI64 newTotalFileSize = totalFileSize + lengthToAdd;

    //get the last chunk
    bool appendToLastChunk = false;

    int newChunksNeeded = 0;
    long bytesLeft = lengthToAdd;
    const long maxChunkSize = man.config.maxChunkSize;
    long deltaLastChunk = 0;

    ChunkInfo *lastChunk = pEntry->lastChunk();
    const unsigned long orderOfLastChunk = lastChunk->order;

    if (canDebug)
      debug(Str("last chunk's length is ").append(lastChunk->length));
    deltaLastChunk = maxChunkSize - lastChunk->length;
    if (deltaLastChunk > 0) {
      appendToLastChunk = true;
      bytesLeft -= deltaLastChunk;
      long oldLengthLastChunk = lastChunk->length;
      //update last chunks length
      if (deltaLastChunk < lengthToAdd)
        lastChunk->length = maxChunkSize;
      else
        lastChunk->length += lengthToAdd;
      if (canDebug)
        debug(Str("length of last chunk has been changed from ").append(
          oldLengthLastChunk).append(" to ").append(lastChunk->length));
    }
    while (bytesLeft > 0) {
      newChunksNeeded++;
      bytesLeft -= maxChunkSize;
    }

    if (canDebug) {
      if (appendToLastChunk)
        debug(Str("last chunk will be appended with bytes - ").append(
          deltaLastChunk));
      debug(Str("amount of new chunks needed - ").append(newChunksNeeded));
    }

    //building result manual
    UI64 currentBufferOffset = 0;
    UI64 amountActions = 0;
    ToStrSerializer serActions;

    bytesLeft = lengthToAdd;

    //action: append to last chunk
    if (appendToLastChunk) {
      if (canDebug) debug("building action for append last chunk");
      amountActions += lastChunk->chunkLocationListLength;
      for (int i = 0; i < lastChunk->chunkLocationListLength; i++) {

        Str uri = lastChunk->chunkLocationList[i].dataNodeUri.toString();
        serActions.append("doappendtochunk");
        serActions.append(uri);
        serActions.append(lastChunk->chunkLocationList[i].chunkId);
        serActions.appendUInt64(currentBufferOffset);
        //how many bytes of the appendix needs to be added here
        if (deltaLastChunk >= lengthToAdd) {
          serActions.appendUInt64(lengthToAdd);
          currentBufferOffset = lengthToAdd;
          if (i == 0) bytesLeft -= lengthToAdd;
        } else {
          serActions.appendUInt64(deltaLastChunk);
          currentBufferOffset = deltaLastChunk;
          if (i == 0) bytesLeft -= deltaLastChunk;
        }
        serActions.appendUInt64(newTotalFileSize);
        serActions.appendUInt64(lastChunk->length);
      }
    }

    //action: new chunk
    if (newChunksNeeded > 0) {

      unsigned long currentOffsetInFile =
        lastChunk->offsetInFile + lastChunk->length;

      if (canDebug)
        debug(
          Str("building action for adding new chunks - amount needed ").append(
            newChunksNeeded));
      int startIndexOfNewChunkInfo = pEntry->reallocateForAdditionalChunkInfo(
        newChunksNeeded);
      for (int i = 0; i < newChunksNeeded; i++) {

        if (canDebug) debug(Str("handlung new chunk number ") + i);

        vector<DataNodeEntry> nodes = man.needDataNodes();
        ChunkInfo *pChunk =
          pEntry->chunkInfoList + startIndexOfNewChunkInfo + i;

        UI64 len = maxChunkSize;
        if (len > bytesLeft) len = bytesLeft;
        bytesLeft -= len;

        pChunk->length = len;
        pChunk->order = orderOfLastChunk + i + 1;
        pChunk->offsetInFile = currentOffsetInFile;

        currentOffsetInFile += len;

        int nodeSize = nodes.size();
        if (canDebug) debug(Str("needed ") + nodeSize + " for this chunk");
        pChunk->allocateLocations(nodeSize);
        for (int j = 0; j < nodeSize; j++) {

          const Str uriStr = nodes[j].uri.toString();
          if (canDebug) debug(Str("handling uri ").append(uriStr));

          (pChunk->chunkLocationList + j)->dataNodeUri = nodes[j].uri;


          if (canDebug) {
            debug(
              Str("new chunk starting at appendix offset and length ").append(
                currentBufferOffset).append(" ").append(len));
          }

          serActions.append("donewchunk");
          serActions.append(uriStr);
          serActions.appendUInt64(currentBufferOffset);
          serActions.appendUInt64(len);
          serActions.appendUInt64(pChunk->order);
          serActions.append(pEntry->category);
          amountActions++;
        }
      }
    }

    if (canDebug) {
      debug(
        Str("amount of actions to be done for append ").append(amountActions));
    }

    ToStrSerializer ser;
    ser.appendRaw("0000");
    ser.appendUInt64(amountActions);
    ser.appendRaw(serActions.output);
    return ser.output;

  } else if (cmd == "read") {

    if (canDebug) debug("getting information for reading file");
    StrReader reader(&netto);
    Str fileId = reader.readStrSer();
    if (canDebug) debug(Str("fileId is ").append(fileId));

    IndexEntry *pie = man.findFile(fileId);
    if (!pie) {
      return "1000file not found";
    }

    if (canDebug)
      debug("file index object was found - now serialize to output");
    ToStrSerializer ser;
    pie->serializeTo(ser);
    if (canDebug) debug("index entry is serialized");
    return success(ser.output);
  }

    //associcate a chunk with a fileid
  else if (cmd == "chun") {
    if (canDebug) debug("associate fileId and chunk");
    StrReader reader(&netto);

    Str fileId = reader.readStrSer();
    Str chunk = reader.readStrSer();
    Str uriStr = reader.readStrSer();
    long order = reader.readInt(12);

    if (canDebug) {
      debug(Str("fileId ").append(fileId));
      debug(Str("chunk ").append(chunk));
      debug(Str("uriStr ").append(uriStr));
      debug(Str("order ").append(order));
    }

    return man.associateChunkToFileId(fileId, chunk, order, uriStr) ? success()
                                                                    : "7777";
  }

    //echo - returns the input as output
  else if (cmd == "echo") {
    return success(i->substr(4));
  }

    //returns amount of files of the system
  else if (cmd == "cofi") {
    return success(man.countFiles());
  } else if (cmd == "sizt") {
    UI64 totalSize = man.totalSize();
    ToStrSerializer ser;
    ser.appendRaw("0000");
    ser.appendUInt64(totalSize);
    return ser.output;
  } else if (cmd == "fsiz") {
    StrReader reader(&netto);
    Str fileId = reader.readStrSer();
    IndexEntry *pEntry = man.findFile(fileId);
    if (pEntry == 0) return Str("00040");
    UI64 fileSize = pEntry->calculateFileSize();
    ToStrSerializer ser;
    ser.appendRaw("0000");
    ser.appendUInt64(fileSize);
    return ser.output;
  } else if (cmd == "hasf") {
    StrReader reader(&netto);
    Str fileId = reader.readStrSer();
    if (canDebug) debug(Str("check if we have file - ").append(fileId));
    bool hasFile = man.hasFile(fileId);
    return Str(hasFile ? "00001" : "00000");
  } else if (cmd == "conf") {
    Str result;
    result = result.append("chunksize=").append(man.config.maxChunkSize);
    return success(result);
  } else if (cmd == "sett") {
    if (canDebug)
      debug(Str("change settings ").append("netto input ").append(netto));
    StrReader reader(&netto);
    Str key = reader.readStrSer();
    Str value = reader.readStrSer();

    if (key == "chunksize") {
      int newChunkSize = value.toInt();
      audit(Str("chunk size changed to ").append(value));
      man.config.maxChunkSize = newChunkSize;
      return success();
    } else {
      return "9998unknown settings key";
    }

  }
    //deletes a file by id
  else if (cmd == "dele") {

    StrReader reader(&netto);
    Str fileId = reader.readStrSer();
    if (canDebug)
      debug(Str("file is wanted to be deleted - <").append(fileId).append(">"));

    IndexEntry *pEntry = man.findFile(fileId);
    if (!pEntry) return Str("0004");

    ToStrSerializer ser;
    ser.appendRaw("0000");
    pEntry->serializeTo(ser);
    man.deleteFile(fileId);
    audit(Str("file has been deleted - ").append(fileId));
    return ser.output;

  }
    //deletes all files
  else if (cmd == "dela") {
    audit("all files deleted");
    if (canDebug) debug("deleted all files");
    man.deleteAllFiles();

    std::vector<URI> uris = man.dataNodeIndex.allURIs();

    ToStrSerializer ser;
    ser.appendRaw("0000");
    int countNodes = uris.size();

    if (canDebug) {
      debug(Str("all uris gotten - count ").append(countNodes));
    }

    ser.appendUInt64(countNodes);
    for (int i = 0; i < countNodes; i++) {
      Str uriStr = uris.at(i).toString();
      if (canDebug) debug(uriStr);
      ser.append(uriStr);
    }

    return ser.output;
  }
    //deletes all files of category
  else if (cmd == "dfac") {
    StrReader reader(&netto);
    Str cat = reader.readStrSer();
    Str log = Str("all files of category ").append(cat).append(" deleted");
    audit(log);
    if (canDebug) debug(log);
    int amountDeleted = man.deleteAllFilesOfCategory(cat);
    ToStrSerializer ser;
    ser.appendRaw("0000");
    ser.appendUInt64(amountDeleted);
    return ser.output;
  }
    //registers a data node
  else if (cmd == "dnrg") {
    StrReader reader(&netto);
    Str dataNodeUri = reader.readStrSer();
    if (canDebug) debug(Str("registered datanode ").append(dataNodeUri));
    man.registerDataNode(dataNodeUri);
    if (canDebug)
      debug(Str("amount known datanodes: ").append(man.countDataNodes()));
    audit(Str("added new data node ").append(dataNodeUri));
    return success();
  }
    //unregisters a data node
  else if (cmd == "dnug") {
    man.unregisterDataNode(netto);
    audit(Str("data not has been removed ").append(netto));
    return success();
  }

    //lists all files in system
  else if (cmd == "list") {
    int count = man.countFiles();
    Str r = Str(count).prepend(14, '0');
    for (IIT kv = man.fileIndex.begin(); kv != man.fileIndex.end(); kv++) {
      Str id = kv->first;
      r = r.append(id.serialize());
    }
    return success(r);
  }

    //trigger backup of state
  else if (cmd == "back") {
    if (canDebug) debug("backup of state forced");
    man.backupState();
  } else {
    globalLogger->error("unknown command");
    return Str("9999", 4);
  }

}

void IndexNode::setLogger(dfs::log::Logger *l) {
  globalLogger = l;
}

void IndexNode::setAuditLogger(dfs::log::Logger *l) {
  auditLogger = l;
}

IndexNodeManager *IndexNode::getManager() {
  return &man;
}

void IndexNode::run() {

  dfs::comm::PFUNC_SMH *smh = &handleMessage;


  //wir brauchen nun den Endpunkt
  dfs::comm::Endpoint ep("indexnode");
  ep.port = port;
  ep.bufsize = 4 * 1024 * 1024;
  ep.setLogger(globalLogger);
  ep.setSingleMessageHandler(smh);

  cout << "DFS-INDEXNODE at port " << port
       << " is now ready and accepting connections" << endl;
  cout << "id of node is " << man.id << endl;

  ep.listen();

}


