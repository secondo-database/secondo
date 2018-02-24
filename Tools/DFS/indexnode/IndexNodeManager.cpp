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
#include "IndexNodeManager.h"
#include "../shared/io.h"

using namespace dfs;
using namespace std;

void IndexNodeManager::storeFile(const IndexEntry &e) {
  IIT pos = fileIndex.begin();
  fileIndex.insert(pos, std::pair<Str, IndexEntry>(e.fileId, e));
}

void IndexNodeManager::deleteFile(const Str &s) {
  fileIndex.erase(s);
  triggerImportantStateChange();
}

int IndexNodeManager::countFiles() const {
  return fileIndex.size();
}

IndexEntry *IndexNodeManager::findFile(const Str &fileId) {
  std::map<Str, IndexEntry>::iterator x = fileIndex.find(fileId);
  if (x != fileIndex.end()) {
    return &x->second;
  }
  return 0;
}

bool IndexNodeManager::hasFile(const Str &fileId) {
  return this->findFile(fileId) != 0;
}

void IndexNodeManager::deleteAllFiles() {
  fileIndex.clear();
  triggerImportantStateChange();
}

int IndexNodeManager::deleteAllFilesOfCategory(const Str &category) {
  IIT x = fileIndex.begin();
  int amountDeleted = 0;
  while (x != fileIndex.end()) {
    if (x->second.category == category) {
      IIT toBeDeleted = x;
      ++x;
      fileIndex.erase(toBeDeleted);
      amountDeleted++;
    } else {
      ++x;
    }
  }
  triggerImportantStateChange();
  return amountDeleted;
}

void IndexNodeManager::registerDataNode(const Str &uri) {
  dataNodeIndex.add(uri);
  triggerImportantStateChange();
}

void IndexNodeManager::unregisterDataNode(const Str &uri) {
  dataNodeIndex.remove(uri);
  triggerImportantStateChange();
}

bool IndexNodeManager::markDataNodeAsBroken(const Str &uri) {
  if (!dataNodeIndex.hasNode(uri)) return false;
  dataNodeIndex.remove(uri);
  for (IIT kv = fileIndex.begin(); kv != fileIndex.end(); kv++) {
    kv->second.markDataNodeAsBroken(uri);
  }
  triggerImportantStateChange();
  return true;
}

int IndexNodeManager::countDataNodes() const {
  return dataNodeIndex.count();
}

std::vector<DataNodeEntry> IndexNodeManager::needDataNodes() {
  return needDataNodes(this->config.replicaCount);
}

std::vector<DataNodeEntry> IndexNodeManager::needDataNodes(int amount) {
  if (amount > dataNodeIndex.count()) {
    throw BaseException("not enough data nodes present");
  }
  return dataNodeIndex.need(amount);
}

Str IndexNodeManager::getDefaultStateFile() {
  return dfs::io::file::combinePath(dataPath, "state");
}

void IndexNodeManager::backupState() {
  Str filename = this->getDefaultStateFile();
  this->dumpStateToFile(filename);
}

void IndexNodeManager::tryToRestoreState() {
  this->restoreStateFromFile(getDefaultStateFile());
}

bool IndexNodeManager::restoreStateFromFile(const Str &filename) {
  io::file::Reader reader(filename);

  if (!reader.open()) return false;

  int version = reader.readInt();
  int amountEntries = reader.readInt();

  for (int i = 0; i < amountEntries; i++) {
    int len = reader.readInt();
    Str serString = reader.readStr(len);
    IndexEntry indexEntry = IndexEntry::deserialize(serString);
    IIT pos = fileIndex.begin();
    fileIndex.insert(pos,
                     std::pair<Str, IndexEntry>(indexEntry.fileId, indexEntry));
  }

  int amountOfDataNodeEntries = reader.readInt();
  for (int i = 0; i < amountOfDataNodeEntries; i++) {
    int len = reader.readInt();
    Str serString = reader.readStr(len);
    DataNodeEntry dataNodeEntry = DataNodeEntry::deserialize(serString);
    dataNodeIndex.addRaw(dataNodeEntry);
  }


  reader.close();

  return true;
}

void IndexNodeManager::dumpStateToFile(const Str &filename) {
  io::file::Writer writer(filename);

  //fileformat
  //<version:int><amountIndexEntries:int><indexEntries>
  //<indexEntries> := <indexEntries><lengthOfString><stringIndexEntrySerialized>

  //version
  writer.append(1);

  //amount of index entries
  writer.append(this->fileIndex.size());

  //write each index entry
  for (IIT kv = fileIndex.begin(); kv != fileIndex.end(); kv++) {
    ToStrSerializer ser;
    kv->second.serializeTo(ser);
    int length = ser.output.len();
    writer.append(length);
    writer.append(ser.output);
  }

  //write datanodes
  int dnSize = this->dataNodeIndex.index.size();
  writer.append(dnSize);
  for (std::map<Str, DataNodeEntry>::iterator kv =
      this->dataNodeIndex.index.begin();
       kv != this->dataNodeIndex.index.end(); kv++) {

    ToStrSerializer ser;
    kv->second.serializeTo(ser);
    int length = ser.output.len();
    writer.append(length);
    writer.append(ser.output);
  }

  writer.close();

}

UI64 IndexNodeManager::totalSize() {
  UI64 totalSize = 0;
  for (IIT kv = fileIndex.begin(); kv != fileIndex.end(); kv++) {
    totalSize += kv->second.calculateFileSize();
  }
  return totalSize;
}

bool
IndexNodeManager::associateChunkToFileId(const Str &fileId, const Str &chunkId,
                                         long order,
                                         const Str &uriOfContaingDataNode) {

  IIT entryIt = this->fileIndex.find(fileId);
  if (entryIt != this->fileIndex.end()) {
    IndexEntry *pEntry = &entryIt->second;
    for (int i = 0; i < pEntry->chunkInfoListLength; i++) {
      if (pEntry->chunkInfoList[i].order == order) {
        ChunkInfo *pci = &pEntry->chunkInfoList[i];
        for (int j = 0; j < pci->chunkLocationListLength; j++) {
          if (pci->chunkLocationList[j].dataNodeUri.toString() ==
              uriOfContaingDataNode) {
            pci->chunkLocationList[j].chunkId = chunkId;
            return true;
          }
        }
      }
    }
  }
  return false;

}


void IndexNodeManager::triggerImportantStateChange() {
  backupState();
}
