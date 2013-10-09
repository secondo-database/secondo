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


*/


#include "ExternalFileCache.h"
#include "Flob.h"
#include "FlobManager.h"
#include <fstream>

ExternalFileCache::ExternalFileCache(const size_t _ms):
    maxSize(_ms), usedSize(0)
{
  list = new vector<pair<string, ifstream*> >();

  slotSize = sizeof(CachedFileIdEntry);
  assert(maxSize > slotSize);
  tableSize = ((maxSize / slotSize) * 2);
  if (tableSize < 1u){
    tableSize = 1u;
  }

  hashtable = new CachedFileIdEntry*[tableSize];
  for(unsigned int i=0;i<tableSize; i++){
    hashtable[i] = 0;
  }

  hashFile = new SmiRecordFile(true, slotSize, true);
  bool created = hashFile->Create("cachedFileIDTable", "Default");
  assert(created);
  first = last = 0;
}


ExternalFileCache::~ExternalFileCache()
{
  clear();
  if (list)
  {
    list->clear();
    delete list;
    list = 0;
  }

  if (!hashFile->Close()){
    cerr << "Problem in closing cachedFileIDTable" << endl;
  }
  if (!hashFile->Remove()){
    cerr << "Problem in deleting cachedFileIDTable" << endl;
  }
  delete hashFile;
  hashFile = 0;

}

void ExternalFileCache::clear()
{
  // Close all files
  if (list)
  {
    vector<pair<string, ifstream*> >::iterator it;
    for (it = list->begin(); it < list->end(); it++)
    {
      ifstream* pt = it->second;
      pt->close();
      pt = 0;
    }
  }

  //Clean all cached records
  if (hashtable)
  {
    for (size_t i=0; i < tableSize; i++)
    {
      hashtable[i] = 0;
    }
    delete[] hashtable;
    hashtable = 0;
  }
  CachedFileIdEntry* entry = first;
  while (entry){
    CachedFileIdEntry* victim = entry;
    entry = entry->lruNext;
    delete victim;
  }
  first = last = 0;
  usedSize = 0;
}

int ExternalFileCache::getFileId(const string fileName)
{
  if (list)
  {
    vector<pair<string, ifstream*> >::iterator it;
    for (it = list->begin(); it < list->end(); it++)
    {
      if (it->first.compare(fileName) == 0)
      {
        return distance(list->begin(), it);
      }
    }

    //New file
    ifstream* pt = new ifstream(fileName.c_str(), ios::binary);
    list->push_back(pair<string, ifstream*>(fileName, pt));
    return (list->size() - 1);
  }
  return -1;
}

ifstream* ExternalFileCache::getFile(const SmiRecordId& recId)
{
  int cachedFileId = findRecord(recId);
  assert(cachedFileId >= 0);
  if (list){
    return list->at(cachedFileId).second;
  }
  return 0;
}


void ExternalFileCache::cacheRecord(const SmiRecordId& recId,
    const string& flobFile)
{
  size_t index = recId % tableSize;
  CachedFileIdEntry* entry = hashtable[index];

  while (entry && !entry->matches(recId)){
    entry = entry->tableNext;
  }

  if (!entry){
    int fileId = getFileId(flobFile);
    assert(fileId >= 0);

    entry = new CachedFileIdEntry(recId, fileId);
    entry->tableNext = hashtable[index];
    if (hashtable[index]){
      hashtable[index]->tablePrev = entry;
    }
    hashtable[index] = entry;
    putAtFront(entry);

    usedSize += slotSize;
    if ( usedSize > maxSize ){
      reduceTable();
    }
  }
  // the entry has been cached already
}

int ExternalFileCache::findRecord(const SmiRecordId& recId)
{
  size_t index = recId % tableSize;
  CachedFileIdEntry* entry = hashtable[index];

  while (entry && !entry->matches(recId)){
    entry = entry->tableNext;
  }

  if (entry){
    bringToFront(entry);
    return entry->cachedFileId;
  }
  else{
    CachedFileIdEntry* newEntry = new CachedFileIdEntry();
    SmiSize actRead;
    bool ok = hashFile->Read(recId, newEntry, slotSize, 0, actRead);
    if (!ok)
    {
      cerr << " error in getting data from cachedFileIdTable " << endl;
      cerr << " actSize = " << actRead << endl;
      cerr << " try to read = " << slotSize << endl;
      string err;
      SmiEnvironment::GetLastErrorCode(err);
      cerr << " err " << err << endl;
    }
    assert(ok);

    newEntry->tableNext = hashtable[index];
    if (hashtable[index]){
      hashtable[index]->tablePrev = newEntry;
    }
    hashtable[index] = newEntry;
    putAtFront(newEntry);

    usedSize += slotSize;
    if (usedSize > maxSize){
      reduceTable();
    }

    return newEntry->cachedFileId;
  }

  return -1;
}

/*
Put a new entry to the top of the lru-list

*/
void ExternalFileCache::putAtFront(CachedFileIdEntry* newEntry)
{
  assert(newEntry->lruPrev==0);
  assert(newEntry->lruNext==0);
  if(!first){
    assert(!last);
    first = newEntry;
    last = newEntry;
  } else {
    newEntry->lruNext = first;
    first->lruPrev = newEntry;
    first = newEntry;
  }
}

/*
Transfer an existing entry to the top of the lru-list

*/
void ExternalFileCache::bringToFront(CachedFileIdEntry* entry)
{
  assert(first);
  assert(last);

  if (entry == first){
   return;
  }

  if (last == entry){
    assert(last->lruNext==0);
    last = last->lruPrev;
    last->lruNext = 0;
    entry->lruPrev = 0;
    entry->lruNext = first;
    first->lruPrev = entry;
    first = entry;
    return;
  }
  //entry in the middle of the list
  assert(entry->lruPrev);
  assert(entry->lruNext);
  entry->lruPrev->lruNext = entry->lruNext;
  entry->lruNext->lruPrev = entry->lruPrev;
  entry->lruPrev = 0;
  entry->lruNext = first;
  first->lruPrev = entry;
  first = entry;
}

/*
Put the last entry in the lru-list into disk

*/
void ExternalFileCache::reduceTable()
{
  //remove from the lru-list
  CachedFileIdEntry* victim = last;
  assert(victim->lruNext == 0);
  last = victim->lruPrev;
  last->lruNext = 0;
  victim->lruPrev = 0;

  //remove from the hashtable
  size_t index = victim->hashValue(tableSize);
  if (hashtable[index] == victim){
    hashtable[index] = victim->tableNext;
    if (hashtable[index]){
      hashtable[index]->tablePrev = 0;
    }
  } else{
    victim->tablePrev->tableNext = victim->tableNext;
    if (victim->tableNext){
      victim->tableNext->tablePrev = victim->tablePrev;
    }
  }
  victim->tablePrev = 0;
  victim->tableNext = 0;

  SmiSize written;
  bool ok = hashFile->Write(victim->recId, victim, slotSize, 0, written);
  assert(ok);
  delete victim;
  usedSize -= slotSize;
}
