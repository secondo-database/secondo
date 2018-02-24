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

#include "State.h"
#include "../shared/io.h"

State::State() {
  isDirty = false;
  fsc = new dfs::FigureSystem(36, 10);
  nextChunkIndex = 0;
  chunksLength = 0;
  chunks = 0;
}

State::~State() {
  delete fsc;
}

void State::deleteAllChunks() {
  isDirty = true;
  this->reset();
}

void State::deleteChunkById(const Str &id) {
  Chunk *pChunk = this->findChunkByName(id);
  if (pChunk == 0) return;
  isDirty = true;

  Chunk *newList = new Chunk[chunksLength - 1];
  int targetIndex = 0;

  for (int i = 0; i < chunksLength; i++) {
    Chunk *cur = chunks + i;
    if (cur->chunkname == id) {
    } else {
      newList[targetIndex] = chunks[i];
      targetIndex++;
    }
  }

  delete[] chunks;
  chunks = newList;
  chunksLength--;
  nextChunkIndex--;

}

Chunk *State::nextChunk() {
  isDirty = true;

  //chunkmeta erzeugen
  Chunk c;
  c.chunkname = fsc->toStr();
  fsc->inc();

  //chunkmeta speichern
  if (nextChunkIndex >= chunksLength) {
    int l = chunksLength + CHUNK_ALLOC;
    Chunk *tmp = new Chunk[l];

    for (int i = 0; i < chunksLength; i++) {
      tmp[i] = chunks[i];
    }
    delete[] chunks;
    chunksLength = l;
    chunks = tmp;
  }
  int index = nextChunkIndex++;
  chunks[index] = c;
  return &chunks[index];
}

void State::reset() {
  if (chunksLength > 0) {
    delete[] chunks;
    chunks = 0;
    chunksLength = 0;
    isDirty = true;
    nextChunkIndex = 0;
  }
  fsc->resetToZero();
}

void State::dumpToFile(const Str &filename) {

  //format
  //<nextChunkIndex><chunksLength>
  //
  dfs::io::file::Writer w(filename);

  w.append(this->nextChunkIndex);
  w.append(this->chunksLength);

  for (int i = 0; i < chunksLength; i++) {
    Chunk *c = &chunks[i];
    w.appendWithLengthInfo(4, c->chunkname);
    w.append(c->chunksize);
    w.appendWithLengthInfo(4, c->category);
  }
  w.close();
}

void State::restoreFromFile(const Str &filename) {
  dfs::io::file::Reader reader(filename);

  if (!reader.open()) return;
  this->reset();

  nextChunkIndex = reader.readInt();
  chunksLength = reader.readInt();

  chunks = new Chunk[chunksLength];
  for (int i = 0; i < chunksLength; i++) {
    Chunk *c = chunks + i;
    c->chunkname = reader.readWithLengthInfo(4);
    c->chunksize = reader.readInt();
    c->category = reader.readWithLengthInfo(4);
  }
  reader.close();
}

Chunk *State::copyState(int *outLength) {
  int l = chunksLength;
  *outLength = l;
  Chunk *res = new Chunk[l];
  for (int i = 0; i < l; i++) res[i] = chunks[i];
  return res;
}

Chunk *State::findChunkByName(const Str &name) {
  for (int i = 0; i < chunksLength; i++) {
    Chunk *pChunk = &chunks[i];
    if (pChunk->chunkname == name) return pChunk;
  }
  return 0;
}

Chunk *State::findChunkByIndex(int index) {
  return &chunks[index];
}

UI64 State::sizeOfAllChunks() {
  UI64 sum = 0;
  for (int i = 0; i < chunksLength; i++) {
    sum += chunks[i].chunksize;
  }
  return sum;
}

int State::usedChunkCount() {
  int sum = 0;
  for (int i = 0; i < chunksLength; i++) {
    if (chunks[i].isUsed()) sum++;
  }
  return sum;
}
