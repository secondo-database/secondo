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

#ifndef STATE_H
#define STATE_H

#include "../shared/str.h"
#include "../shared/FigureSystem.h"
#include "Chunk.h"

const short CHUNK_ALLOC = 128;

namespace dfs {

  class State {
  public:
    State();

    virtual ~State();

    /*
     * returns amount of chunks within the data node
     * @return
     */
    int getChunksLength() { return chunksLength; }

    /*
     * returns list of chunks
     * do not modify!
     * @return
     */
    Chunk *getChunkList() { return chunks; }

    /*
     * returns next free chunk
     * @return
     */
    Chunk *nextChunk();

    /*
     * created deep copy of state
     * memory need to be freed by caller
     */
    Chunk *copyState(int *outLength);

    /*
     * returns a chunk by id
     * @param name
     * @return
     */
    Chunk *findChunkByName(const Str &name);

    /*
     * deletes chunk by id
     * if not found, nothing happens
     * @param id
     */
    void deleteChunkById(const Str &id);

    /*
     * deletes all chunks and reorganized internal structures like
     * on a new data node
     */
    void deleteAllChunks();

    /*
     * returns chunk by the index within the internal list
     * @param index
     * @return
     */
    Chunk *findChunkByIndex(int index);

    /*
     * indicates whether dirty
     * if read out, dirty is unset
     * @return
     */
    bool dirtyToggle() {
      bool b = isDirty;
      if (b) isDirty = false;
      return b;
    }

    void markAsDirty() { isDirty = true; };

    /*
     * dumps state to file
     * @param filename
     */
    void dumpToFile(const Str &filename);

    /*
     * restores state from file
     * @param filename
     */
    void restoreFromFile(const Str &filename);

    /*
     * returns amount of used chunks in this data node
     * @return
     */
    int usedChunkCount();

    /*
     * returns size of all chunks
     * @return
     */
    UI64 sizeOfAllChunks();

  private:
    FigureSystem *fsc;
    int nextChunkIndex;
    bool isDirty;
    Chunk *chunks;
    int chunksLength;

    void reset();
  };
};

#endif /* STATE_H */
