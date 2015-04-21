/*
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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

[1] Definition of TupleQueueHP

February 2014 Jiamin Lu

[newpage]

1 TupleQueueHP

*/

#include "TupleQueueHP.h"
#include "Counter.h"

TupleQueueHP::TupleQueueHP(const size_t maxMemorySize):
  MAX_MEMORY_SIZE(maxMemorySize),
  diskBuffer(0), inMemory(true), totalMemSize(0), transportedTuples(0)
{}

TupleQueueHP::~TupleQueueHP(){
  if (inMemory){
    for(TupleList::iterator it = memoryBuffer.begin();
        it != memoryBuffer.end(); it++){
      (*it)->DeleteIfAllowed();
      (*it) = 0;
    }
    memoryBuffer.clear();
  }
 
  if (!inMemory && transportedTuples){
    cleanDiskBuffer();
    
    diskBuffer->DeleteAndTruncate();
    diskBuffer = 0;
  }
}

void TupleQueueHP::AppendTuple(Tuple* t)
{
  if (inMemory)
  {
    if (totalMemSize + t->GetMemSize() <= MAX_MEMORY_SIZE)
    {
      t->IncReference();
      memoryBuffer.push_back(t);
      totalMemSize += t->GetMemSize();
      return;
    }
    else
    {
      diskBuffer = new Relation(t->GetTupleType(), true);
      inMemory = false;
    }
  }
  
  t->PinAttributes();
  diskBuffer->AppendTupleNoLOBs(t);
}

Tuple* TupleQueueHP::PopTuple(bool& bulkLoaded){
  if (memoryBuffer.empty()){
    if (!inMemory){
      if (!bulkLoadQueue()){
        return 0;
      } else {
        bulkLoaded = true;
      }
    } else {
      return 0;
    }
  }

  Tuple* result = memoryBuffer.front();
  memoryBuffer.pop_front();
  totalMemSize -= result->GetMemSize();
  result->DeleteIfAllowed();
  return result;
}


size_t TupleQueueHP::GetNoTuples() const {
  if (inMemory){
    return memoryBuffer.size();
  }
  else {
    return (memoryBuffer.size() + diskBuffer->GetNoTuples()
      - transportedTuples->size());
  }
}

bool TupleQueueHP::bulkLoadQueue(){
  assert(!inMemory);
  assert(totalMemSize == 0);

  //First clean up the cached transported tuples. 
  cleanDiskBuffer();

  if (diskBuffer->GetNoTuples() == 0){
    diskBuffer->DeleteAndTruncate();
    diskBuffer = 0;
    return false;
  }

  transportedTuples = new vector<Tuple*>();
  //Load tuples from the diskBuffer to the memoryBuffer until it becomes full
  GenericRelationIterator *diskIterator = diskBuffer->MakeScan();
  Tuple* tuple = diskIterator->GetNextTuple();
  while (tuple && totalMemSize + tuple->GetMemSize() <= MAX_MEMORY_SIZE){
    assert(tuple);
    tuple->IncReference();
    memoryBuffer.push_back(tuple);
    totalMemSize += tuple->GetMemSize();
      
    tuple->IncReference();
    transportedTuples->push_back(tuple);

    tuple = diskIterator->GetNextTuple();
  }
  return true;
}

Tuple* TupleQueueHP::getMemoryCachedTuple(size_t i)
{
  assert(i < memoryBuffer.size());
  size_t count = 0;
  TupleList::iterator it = memoryBuffer.begin();
  while (count++ < i){
    it++;
  }
  return *it;
}

TupleQueueHPIterator::TupleQueueHPIterator(TupleQueueHP& q):
  queue(q), diskIterator(0), currentTuple(0)
{}

Tuple* TupleQueueHPIterator::GetNextTuple()
{
  Tuple* result;
  if (currentTuple < queue.memoryBuffer.size()){
    result = queue.getMemoryCachedTuple(currentTuple);

  } else {
    if (diskIterator == 0){
      diskIterator = queue.diskBuffer->MakeScan();
    }
    result = diskIterator->GetNextTuple();
  }
  result->IncReference();
  currentTuple++;
  return result;
}

void TupleQueueHP::IncFlobReference(const Flob& flob)
{
  assert(flob.getMode() == 1);
  SmiRecordId recId = flob.getRecordId();
  if (FlobRecIdRefs.find(recId) == FlobRecIdRefs.end()){
    FlobRecIdRefs.insert(make_pair(recId, 1));
  }
  else {
    FlobRecIdRefs.find(recId)->second++;
  }
}

bool TupleQueueHP::deleteIfAllowed(const Flob& flob)
{
  if (flob.getMode() == 1)
  {
    SmiRecordId recId = flob.getRecordId();
    map<SmiRecordId, size_t>::iterator mit = FlobRecIdRefs.find(recId);
    if (mit != FlobRecIdRefs.end()){
      if (mit->second > 0){
        mit->second--;
        return (mit->second == 0);
      }
    }
  }
  return true;
}

void TupleQueueHP::cleanDiskBuffer()
{
  if (transportedTuples)
  {
    for (vector<Tuple*>::iterator it = transportedTuples->begin();
          it != transportedTuples->end(); it++){
      Tuple* tuple = (*it);
      if (tuple)
      {
        for (int i = 0; i < tuple->GetNoAttributes(); i++ ){
          Attribute* attr = tuple->GetAttribute(i);
          for (int k = 0; k < attr->NumOfFLOBs(); k++){
            Flob* flob = attr->GetFLOB(k);
            if (!deleteIfAllowed(*flob)){
              //avoid deleting the same flob repeatedly
              flob->changeMode((char)3);
            }
          }
        }
        diskBuffer->DeleteTuple(tuple);
      }
    }

    transportedTuples->clear();
    delete transportedTuples;
    transportedTuples = 0;
  }
}

