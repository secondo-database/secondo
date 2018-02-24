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
#ifndef REMOTEMETADATA_H
#define REMOTEMETADATA_H

#include "str.h"
#include "uri.h"
#include "../dfs/dfs.h"
#include "numberUtils.h"

namespace dfs {

  struct ChunkLocation : SerializeAble {

    dfs::URI dataNodeUri;

    Str chunkId;

    static ChunkLocation deserialize(StrReader &r) {
      ChunkLocation location;
      location.dataNodeUri = dfs::URI::fromString(r.readStrSer());
      location.chunkId = r.readStrSer();
      return location;
    }

    virtual void serializeTo(ToStrSerializer &serializer) const {
      serializer.append(dataNodeUri.toString());
      serializer.append(chunkId);
    }

  };

  class ChunkInfo : SerializeAble {

  public:

    NUMBER offsetInFile;
    NUMBER length;
    NUMBER order;

    ChunkLocation *chunkLocationList;

    int chunkLocationListLength;

    ChunkInfo() {
      chunkLocationListLength = 0;
      chunkLocationList = 0;
    }

    ChunkInfo(const ChunkInfo &other) {
      offsetInFile = other.offsetInFile;
      length = other.length;
      order = other.order;

      chunkLocationListLength = other.chunkLocationListLength;
      chunkLocationList = 0;
      if (other.chunkLocationList != 0) {
        chunkLocationList = new ChunkLocation[chunkLocationListLength];
        for (int i = 0; i < chunkLocationListLength; i++) {
          chunkLocationList[i] = other.chunkLocationList[i];
        }
      }
    }

    ChunkInfo &operator=(const ChunkInfo &other) {
      if (this != &other) {
        delete[] chunkLocationList;
        length = other.length;
        order = other.order;
        offsetInFile = other.offsetInFile;
        chunkLocationListLength = other.chunkLocationListLength;
        chunkLocationList = 0;
        if (other.chunkLocationListLength > 0) {
          chunkLocationList = new ChunkLocation[chunkLocationListLength];
          for (int i = 0; i < chunkLocationListLength; i++)
            chunkLocationList[i] = other.chunkLocationList[i];
        }
      }
      return *this;
    }

    ~ChunkInfo() {
      delete[] chunkLocationList;
      chunkLocationList = 0;
    }

    void allocateLocations(int locationCount) {
      delete[] chunkLocationList;
      chunkLocationList = new ChunkLocation[locationCount];
      chunkLocationListLength = locationCount;
    }

    void markDataNodeAsBroken(const Str &uri) {

      int foundCount = 0;
      for (int i = 0; i < chunkLocationListLength; i++) {
        if ((chunkLocationList + i)->dataNodeUri.toString() == uri) {
          foundCount++;
          break;
        }
      }

      if (foundCount == 0) return;

      int newLocLength = chunkLocationListLength - foundCount;

      if (newLocLength == 0) {
        delete[] chunkLocationList;
        chunkLocationList = 0;
        chunkLocationListLength = 0;
      } else {
        ChunkLocation *newList = new ChunkLocation[newLocLength];
        int targetIndex = 0;
        for (int i = 0; i < chunkLocationListLength; i++) {
          if ((chunkLocationList + i)->dataNodeUri.toString() == uri) {
          } else {
            newList[targetIndex++] = chunkLocationList[i];
          }
        }
        delete[] chunkLocationList;
        chunkLocationList = newList;
        chunkLocationListLength = newLocLength;
      }

    }

    virtual void serializeTo(ToStrSerializer &serializer) const {
      serializer.appendDefaultUnsigned(order);
      serializer.appendDefaultUnsigned(offsetInFile);
      serializer.appendDefaultUnsigned(length);
      serializer.appendDefaultUnsignedShort(chunkLocationListLength);
      for (int i = 0; i < chunkLocationListLength; i++) {
        ChunkLocation *pl = &chunkLocationList[i];
        pl->serializeTo(serializer);
      }
    }

    static ChunkInfo deserialize(StrReader &r) {
      ChunkInfo i;
      i.order = r.readLong(12);
      i.offsetInFile = r.readLong(12);
      i.length = r.readLong(12);
      i.allocateLocations(r.readInt(5));
      for (int j = 0; j < i.chunkLocationListLength; j++) {
        i.chunkLocationList[j] = ChunkLocation::deserialize(r);
      }

      return i;
    }

  };

  class IndexEntry : SerializeAble {
  public:

    static IndexEntry
    create(const Str &fileId, int chunkInfoCount, const Str &category) {
      IndexEntry e;
      e.fileId = fileId;
      e.chunkInfoList = new ChunkInfo[chunkInfoCount];
      e.chunkInfoListLength = chunkInfoCount;
      e.category = category;
      return e;
    }

    static IndexEntry
    createForOneChunkFile(const Str &fileId, const Str &category) {
      return create(fileId, 1, category);
    }


    /**
     * optional category of index entry
     */
    Str category;


    /**
     * the fileid
     */
    Str fileId;

    /**
     * a list of chunk info
     */
    ChunkInfo *chunkInfoList;

    /**
     * the length of chunkInfo
     */
    int chunkInfoListLength;

    /**
     * indicates whether the file is deleted
     */
    bool isDeleted;

    IndexEntry() {
      isDeleted = false;
      chunkInfoList = 0;
      chunkInfoListLength = 0;
    }

    IndexEntry(const IndexEntry &other) {

      fileId = other.fileId;
      category = other.category;
      isDeleted = other.isDeleted;

      this->chunkInfoListLength = other.chunkInfoListLength;
      if (this->chunkInfoListLength > 0) {
        this->chunkInfoList = new ChunkInfo[this->chunkInfoListLength];
        for (int i = 0; i < chunkInfoListLength; i++) {
          this->chunkInfoList[i] = other.chunkInfoList[i];
        }
      } else {
        this->chunkInfoList = 0;
      }
    }

    void markDataNodeAsBroken(const Str &uri) {
      for (int i = 0; i < chunkInfoListLength; i++) {
        (this->chunkInfoList + i)->markDataNodeAsBroken(uri);
      }
    }

    int reallocateForAdditionalChunkInfo(int amountToAdd) {
      ChunkInfo *newList = new ChunkInfo[chunkInfoListLength + amountToAdd];
      for (int i = 0; i < chunkInfoListLength; i++) {
        newList[i] = chunkInfoList[i];
      }
      delete[] chunkInfoList;
      int startingIndex = chunkInfoListLength;
      chunkInfoList = newList;
      chunkInfoListLength += amountToAdd;
      return startingIndex;
    }

    ChunkInfo *lastChunk() {
      return chunkInfoListLength == 0 ? 0 : &chunkInfoList[chunkInfoListLength -
                                                           1];
    }

    IndexEntry &operator=(const IndexEntry &other) {
      if (this != &other) {

        fileId = other.fileId;
        category = other.category;
        isDeleted = other.isDeleted;
        chunkInfoListLength = other.chunkInfoListLength;

        delete[] chunkInfoList;
        chunkInfoList = 0;
        if (chunkInfoListLength > 0) {
          chunkInfoList = new ChunkInfo[chunkInfoListLength];
          for (int i = 0; i < chunkInfoListLength; i++)
            chunkInfoList[i] = other.chunkInfoList[i];
        }
      }
      return *this;
    }

    ~IndexEntry() {
      delete[] chunkInfoList;
      chunkInfoList = 0;
    }

    bool isSinglePartFile() {
      return chunkInfoListLength == 1;
    }

    unsigned long long calculateFileSize() const {
      unsigned long long fileSize = 0;
      for (int i = 0; i < chunkInfoListLength; i++) {
        fileSize += chunkInfoList[i].length;
      }
      return fileSize;
    }

    void serializeTo(ToStrSerializer &ser) const {

      ser.append(fileId);
      ser.append(chunkInfoListLength, 4);
      ser.append(category);

      for (int iChunkInfoIndex = 0;
           iChunkInfoIndex < chunkInfoListLength; iChunkInfoIndex++) {
        ChunkInfo *pci = &chunkInfoList[iChunkInfoIndex];
        pci->serializeTo(ser);
      }
    }

    static IndexEntry deserialize(const Str &s) {
      StrReader r(&s);

      Str fileId = r.readStrSer();
      int infoLength = r.readInt(4);
      Str category = r.readStrSer();

      IndexEntry e = IndexEntry::create(fileId, infoLength, category);
      for (int i = 0; i < infoLength; i++) {
        e.chunkInfoList[i] = ChunkInfo::deserialize(r);
      }

      return e;
    }


  };
};
#endif
