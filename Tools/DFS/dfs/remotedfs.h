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

#pragma once

#include "dfs.h"
#include "../shared/str.h"
#include "../shared/uri.h"
#include "../shared/log.h"
#include "../shared/remotemetadata.h"
#include "../shared/io.h"

namespace dfs {

  namespace remote {

    class DataNodeCommandBuilder {
    public:
      Str getBytes(const Str &chunkId, NUMBER offsetInChunk, NUMBER length);
    };

    class RemoteFilesystem : public Filesystem {
    private:

      URI indexURI;
      log::Logger *logger;
      bool canDebug;

      void debug(const Str &s);

      void debugMsg(const Str &s);

      /**
       * sends synchronous request to index node
       * @param s
       * @return
       */
      Str syncm(const Str &s);

      /**
       * sends message to custom URI
       * @param uri
       * @param s
       * @return
       */
      Str syncMessageToURI(const URI &uri, const Str &s);

      /**
       * performs response errors
       * @param s
       */
      void assertResponse(const Str &s);

      void
      storeFullBufferFromMemory(FILEID fileId, FILEBUFFER content, long length,
                                CATEGORY c = 0);

      void loadChunkContentFromDataNode(const ChunkInfo *chunkInfo,
                                        io::file::Writer &fileWriter);

      void
      appendToExistingFile(FILEID fileId, FILEBUFFER appendix, long length);

      void
      registerLocalChunkIdToIndex(const Str &fileId, const URI &uri, long order,
                                  const Str &localChunkId);

      bool receiveBytesFromDataNodeDirectly(const URI &dataNodeUri,
                                            const Str &localChunkId,
                                            UI64 offsetInChunk, UI64 length,
                                            char *buffer);

      /**
       * marks data node as errornous
       * will not be used anymore than
       * @param dataNodeUri
       */
      void markDataNodeAsErrornous(const URI &dataNodeUri);

      /**
       * receives bytes from a file
       * chooses data node randomly
       * if error occur, use next data node and mark one as broken
       * @param info
       * @param offsetInChunk
       * @param length
       * @param buffer
       */
      void receiveBytesFromDataNodeFailSafe(ChunkInfo *info, UI64 offsetInChunk,
                                            UI64 length, char *buffer);

      /**
       * sends request to data node
       * falls back to other node if error occurs
       * @param info
       * @param request
       * @return
       */
      Str sendChunkAffectingRequestToDataNodeFailsafe(ChunkInfo *info,
                                                      const Str &request);

      /**
       * sends request to data node
       * if it fails, it does not care
       * data node will be marked as broken in error case
       * @param dataNodeUri
       * @param request
       * @return
       */
      bool sendRequestToDataNodeDontCareKilling(const URI &dataNodeUri,
                                                const Str &request);

      /**
       * sends request to data node
       * if it fails, data node is marked as broken (killed) in error case
       * @param dataNodeUri
       * @param request
       * @param response
       * @return
       */
      bool
      sendRequestToDataNodeKilling(const URI &dataNodeUri, const Str &request,
                                   Str *response);

      /**
       * sends a request to index which let indexnode save its state
       */
      void triggerIndexBackup();

    public:

      /*
       * some parameters
       */
      int fileCopyBuffer;

      RemoteFilesystem(URI indexURI, log::Logger *logger);

      virtual ~RemoteFilesystem();

      virtual void deleteFile(FILEID fileId);

      virtual void receiveFileToLocal(FILEID fileId, FILEPATH localPath);

      virtual void
      storeFile(FILEID fileId, FILEBUFFER content, long length, CATEGORY c = 0);

      virtual void
      appendToFile(FILEID fileId, FILEBUFFER appendix, long length);

      void appendToFileFromLocalFile(FILEID fileId, FILEPATH localPath);

      virtual bool hasFile(FILEID fileId);

      virtual void
      storeFileFromLocal(FILEID fileId, FILEPATH localPath, CATEGORY c = 0);

      virtual bool hasFeature(FEATURE name);

      virtual int countFiles();

      virtual UI64 totalSize();

      virtual void deleteAllFiles();

      virtual std::vector<std::string> listFileNames();

      virtual UI64 deleteAllFilesOfCategory(CATEGORY c);

      virtual void
      receiveFilePartially(const char *fileId, unsigned long startIndex,
                           unsigned long length, char *targetBuffer,
                           unsigned long targetBufferStartIndex = 0);

      virtual UI64 fileSize(const char *fileId);

      virtual UI64 nextWritePosition(const char *fileId);

      /*
       admin methods
       */
      void registerDataNode(const URI &uri);

      virtual void changeSetting(const char *key, const char *value);

      virtual void changeChunkSize(int newSize);

      /*
       other methods
       */
      Str echo(const Str &msg);

    };

  };

};