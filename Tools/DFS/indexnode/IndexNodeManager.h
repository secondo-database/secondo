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
#ifndef INDEXNODEMANAGER_H
#define INDEXNODEMANAGER_H

#include "../shared/str.h"
#include "../shared/remotemetadata.h"
#include "DataNodeIndex.h"
#include "Configuration.h"

namespace dfs {

  typedef std::map<Str, IndexEntry>::iterator IIT;

  class IndexNodeManager {
  private:
    void triggerImportantStateChange();

  public:

    Configuration config;

    DataNodeIndex dataNodeIndex;

    Str id;

    Str dataPath;

    //the state of the index, the relation fileId to IndexEntry
    std::map<Str, IndexEntry, StrComparer> fileIndex;

    //operation on the file index
    Str getDefaultStateFile();

    void dumpStateToFile(const Str &filename);

    bool restoreStateFromFile(const Str &filename);

    void backupState();

    void tryToRestoreState();

    //datanode management methods
    void registerDataNode(const Str &uri);

    void unregisterDataNode(const Str &uri);

    int countDataNodes() const;

    UI64 totalSize();

    std::vector<DataNodeEntry> needDataNodes();

    std::vector<DataNodeEntry> needDataNodes(int amount);

    bool markDataNodeAsBroken(const Str &uri);

    //methods for the file system
    IndexEntry *findFile(const Str &fileId);

    void deleteFile(const Str &fileId);

    void storeFile(const IndexEntry &indexEntry);

    bool hasFile(const Str &fileId);

    int countFiles() const;

    void deleteAllFiles();

    int deleteAllFilesOfCategory(const Str &category);

    bool
    associateChunkToFileId(const Str &fileId, const Str &chunkId, long order,
                           const Str &uriOfContaingDataNode);

  };
};

#endif /* INDEXNODEMANAGER_H */

