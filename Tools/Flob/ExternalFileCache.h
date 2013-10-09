/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Department of Computer Science,
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

//paragraph [10] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[ae] [\"a]
//[ue] [\"u]
//[oe] [\"o]

[10] Header File of Module PersistentFlobCache

October 2013, Jiamin Lu

1  Overview

The ~ExternalFileCache~ manages the file pointers of external data files
that contain Flob data.
It is mainly prepared for the mode 2 in Flob management.
Tuples and their Flob are kept together in files created by operator like ~fconsume~.
Only tuples are fetched from these files, but leaves their Flob data untouched.
The Flob is read only when they are really needed.

In order to avoid repeatedly open and close the same data file,
this class is built up.
With it, the Flob can get access to the opened file pointer directly.

*/

#include "SecondoSMI.h"
#include "Flob.h"

class CachedFileIdEntry
{
public:
  CachedFileIdEntry(const SmiRecordId& _rid, const int& _cfid):
    recId(_rid), cachedFileId(_cfid){
    tableNext = tablePrev = 0;
    lruNext = lruPrev = 0;
  }

  CachedFileIdEntry(){
    recId = 0;
    cachedFileId = -1;
    tableNext = tablePrev = 0;
    lruNext = lruPrev = 0;
  }

  inline size_t hashValue(size_t tableSize){
    return (recId % tableSize);
  }

  inline bool matches(const SmiRecordId& rid){
    return (rid == recId);
  }

  SmiRecordId recId;
  int cachedFileId;
  CachedFileIdEntry* tableNext;
  CachedFileIdEntry* tablePrev;
  CachedFileIdEntry* lruNext;
  CachedFileIdEntry* lruPrev;
};

class ExternalFileCache{
public:

  ExternalFileCache(const size_t _maxSize);

  ~ExternalFileCache();

/*
Get the input file stream pointer.

*/
  ifstream* getFile(const SmiRecordId& recId);

  void cacheRecord(const SmiRecordId& recId, const string& flobFile);
  int findRecord(const SmiRecordId& recId);

private:

  void clear();

/*
According to the file name, create the input file stream and return the fildId.
The fileId is the serial number of the file in the vector.

*/
  int getFileId(const string fileName);

  void putAtFront(CachedFileIdEntry* newEntry);
  void bringToFront(CachedFileIdEntry* entry);
  void reduceTable();

  vector<pair<string, ifstream*> >* list;

  //In-Memory Hashtable for cachedFileIds (records)
  size_t maxSize, tableSize, slotSize;
  CachedFileIdEntry** hashtable;
  CachedFileIdEntry* first;
  CachedFileIdEntry* last;
  size_t usedSize;

  //Disk Cache file, prepared for the above hashtable
  SmiRecordFile* hashFile;
};
