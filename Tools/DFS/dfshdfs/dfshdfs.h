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
#ifndef DFS_DFSHDFS_H
#define DFS_DFSHDFS_H

#include "../dfs/dfs.h"
#include "../shared/log.h"
#include "../shared/str.h"
#include <hdfs.h>

namespace dfs {

  namespace hdfs {

    /**
     * simple filesystem implementation using HDFS
     */
    class HDFS : public SimpleFilesystem {
    private:

      hdfsFS fs;
      bool isOpen;
      dfs::log::Logger *pLogger;

      void debug(const Str &msg);

      void error(const Str &msg);

      void open();

      void close();

    public:
      HDFS();

      virtual ~HDFS();

      virtual void deleteFile(FILEID fileId);

      virtual void receiveFileToLocal(FILEID fileId, FILEPATH localPath);

      virtual void
      storeFile(FILEID fileId, FILEBUFFER content, long length, CATEGORY c = 0);

      virtual void
      appendToFile(FILEID fileId, FILEBUFFER appendix, long length);

      virtual UI64 nextWritePosition(FILEID fileId);

      virtual void
      receiveFilePartially(FILEID fileId, NUMBER startIndex, NUMBER length,
                           char *targetBuffer,
                           NUMBER targetBufferStartIndex = 0);

      virtual bool hasFile(FILEID fileId);

      void setLogger(dfs::log::Logger *logger);

    };
  };

};

#endif
