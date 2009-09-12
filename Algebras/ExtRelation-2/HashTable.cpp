/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
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

1 Implementation File HashTable.cpp

June 2009, Sven Jungnickel. Initial version

2 Includes and defines

*/

#include "stdlib.h"

#include "LogMsg.h"
#include "HashTable.h"

/*
3 Implementation of class ~Bucket~

*/
namespace extrel2
{

ostream& Bucket::Print(ostream& os)
{
  os << "Bucket " << number << endl;

  for(size_t i = 0; i < tupleRefs.size(); i++)
  {
    os << *(tupleRefs[i].tuple) << "(Refs: "
       << tupleRefs[i].tuple->GetNumOfRefs()
       << ")" << endl;
  }

  return os;
}

/*
4 Implementation of class ~BucketIterator~

*/
BucketIterator::BucketIterator(Bucket& b)
: bucket(b)
{
  iter = bucket.tupleRefs.begin();
}

Tuple* BucketIterator::GetNextTuple()
{
  if ( iter != bucket.tupleRefs.end() )
  {
    Tuple* t = (*iter).tuple;
    iter++;
    return t;
  }

  return 0;
}

/*
5 Implementation of class ~HashTable~

*/
HashTable::HashTable( size_t nBuckets,
                      HashFunction* f,
                      JoinTupleCompareFunction* cmp )
: iter(0)
, hashFunc(f)
, cmpFunc(cmp)
{
  for(size_t i = 0; i < nBuckets; i++)
  {
    buckets.push_back( new Bucket(i) );
  }
}

HashTable::~HashTable()
{
  for(size_t i = 0; i < buckets.size(); i++)
  {
    delete buckets[i];
  }
  buckets.clear();

  if ( iter )
  {
    delete iter;
    iter = 0;
  }

  delete hashFunc;
  hashFunc = 0;

  delete cmpFunc;
  cmpFunc = 0;
}

void HashTable::Clear()
{
  // reset iterator
  if ( iter )
  {
    delete iter;
    iter = 0;
  }

  // clear buckets
  for(size_t i = 0; i < buckets.size(); i++)
  {
    buckets[i]->Clear();
  }
}

void HashTable::Insert(Tuple* t)
{
  // calculate bucket number
  size_t h = hashFunc->Value(t);

  // insert tuple into bucket
  buckets[h]->Insert(t);
}

int HashTable::ReadFromStream(Word stream, size_t maxSize, bool& finished)
{
  int read = 0;
  size_t bytes = 0;
  Word wTuple(Address(0));

  // Request first tuple
  qp->Request(stream.addr, wTuple);

  while( qp->Received(stream.addr) )
  {
    read++;
    Tuple *t = static_cast<Tuple*>( wTuple.addr );

    bytes += t->GetSize();

    this->Insert(t);

    if ( bytes > maxSize )
    {
      finished = false;
      return read;
    }

    qp->Request(stream.addr, wTuple);
  }

  finished = true;

  return read;
}

Tuple* HashTable::Probe(Tuple* t)
{
  Tuple* nextTuple = 0;

  // calculate bucket number
  size_t h = hashFunc->Value(t);

  if ( iter == 0 )
  {
    // start bucket scan
    iter = buckets[h]->MakeScan();

    if ( traceMode )
    {
      cmsg.info() << "Start scanning bucket "
                  << h << ".." << endl;
    }
  }
  else
  {
    if ( traceMode )
    {
      cmsg.info() << "Proceeding scanning bucket "
                  << h << ".." << endl;
    }
  }

  while ( (nextTuple = iter->GetNextTuple() ) != 0 )
  {
    if ( traceMode )
    {
      cmsg.info() << "Comparing :" << *t << " and " << *nextTuple;
    }

    if ( cmpFunc->Compare(t, nextTuple) == 0 )
    {
      if ( traceMode )
      {
        cmsg.info() << " -> Match!" << endl;
      }
      return nextTuple;
    }

    if ( traceMode )
    {
      cmsg.info() << ".." << endl;
    }
  }

  delete iter;
  iter = 0;

  if ( traceMode )
  {
    cmsg.info() << "End of scan bucket "
                << h << ".." << endl;
  }

  return 0;
}

vector<Tuple*> HashTable::GetTuples(int bucket)
{
  Tuple* t;
  vector<Tuple*> arr;

  BucketIterator* iter = buckets[bucket]->MakeScan();

  while ( ( t = iter->GetNextTuple() ) != 0 )
  {
    arr.push_back(t);
  }

  return arr;
}

/*
6 Implementation of global operators

*/
ostream& HashTable::Print(ostream& os)
{
  os << "------------- Hash-Table content --------------" << endl;

  for(size_t i = 0; i < buckets.size(); i++)
  {
    buckets[i]->Print(os);
  }

  return os;
}

} // end of namespace extrel2
