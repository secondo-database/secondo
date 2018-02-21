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

//paragraph [1] Title: [{\Large \bf ] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Declaration of Page Hash Table in UrelAlgebra

Jiamin Lu, December 2009

1 Overview

The Page Hash Table is the first part of an Urel1 type object,
holds the top several root pages in the memory pool file.
It is a static hash table, each bucket takes one page.
Every entry in this table with the form
(~object identifier~, ~page number~, ~address~),
point to a data page contains an object's latest updates.

This file declares two classes: *PageHashTable* and *PageHashBucket*,
and a structure *PageHashEntry*.

The PageHashTable provides following methods:

  * FindEntry

  * ChageEntry

The FindEntry is used to find an exist entry in this table,
and add a new entry when the first update unit of an object
is added in. The ChangeEntry is used to change the hash entry
when the latest data page of an object is full.

The PageHashBucket provides following methods:

  * FindEntry


  * AddEntry


  * ChangeEntry

The FindEntry here is used to find the entry in this bucket with
binary search method. The AddEntry is used to insert a new
entry with ascending order. And the ChangeEntry is used to change
an exist entry in this bucket.

2 Includes, Type definitions

*/

#ifndef PAGEHASH_H
#define PAGEHASH_H

#include <vector>
#include <iostream>
#include <string>

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "Algebras/RTree/RTreeAlgebra.h"
#include "ListUtils.h"
#include "UrelAlgebra.h"

#define TRACE_ON
#include "LogMsg.h"

typedef u_int32_t Oid_t;
typedef db_pgno_t PageNo_t;

/*

3 Definition of PageHash Entry

Each entry indicates the latest data page of an object.
It is composed by three parts: ~oid~, ~pageNo~ and ~pageAddr~.
The ~oid~ is the unique identifier of a moving object,
the ~pageNo~ is the page number of the latest data page contains
the update of that object, and the ~pageAddr~ records the memory
address of that page.

*/

class Urel1;

struct PageHashEntry
{
  Oid_t oID;
  PageNo_t pageNo;
  void* pageAddr;
};

/*

4 Definition of PageHashBucket

Each bucket holds one page in the memory pool file,
it's composed by three parts: ~disk~, ~ExtEntryNum~
and the array of entries.

The ~disk~ is a bool value, it is true when the bucket is full.
The ~ExtEntryNum~ is the number of exist entries in this bucket.
In the entry array, entries are ascending ordered by the objects ~oid~s.

*/

class PageHashBucket
{
public:

  PageHashBucket(SmiUpdatePage* page);
  ~PageHashBucket();

  bool  FindEntry(const Oid_t oID, PageHashEntry& objEntry);
  bool  AddEntry(const Oid_t oID, PageHashEntry& newEntry);
  bool  ChangeEntry(const Oid_t oID, PageHashEntry newEntry);

private:
  bool disk;
  //Head value, whether this bucket is full
  int extEntryNum;
  //Head value, number of exist entries inside this bucket

  SmiUpdatePage* pagePt;
  bool isInitialized;
  int headOffset;
  //The offset before the entry array
  int maxBucketsNum;
  //The maximum entries that this bucket can hold

  SmiSize  binSearchEntryAddr(const Oid_t _oID,
      const SmiSize _LPt, const SmiSize _HPt);

  friend class PageHashTable;
};

/*

5 Definition of PageHashTable

The PageHashTable decides the bucket number in this table,
and which page the table is start from.

The hash function in this table is *mod*, when the application
want to find the entry of an object indicated by ~oid~,
the application will first try to search the entry in the
(~oid~ mod ~bucketNum~) page.
If the entry doesn't exist in this bucket, and the bucket is full,
then it will try the next bucket unless all buckets in this table
are full.

*/

class PageHashTable
{
public:
  PageHashTable(SmiUpdateFile* ufp, int _bucketPageNum,
      int startPageNo = 0);
  PageHashTable(PageHashTable& rhs);
  ~PageHashTable();

  bool FindEntry(const Oid_t oID, PageHashEntry& objEntry,
      bool wantAdd = false);
  bool ChangeEntry(const Oid_t oID, SmiUpdatePage* _newPage);
  int  GetBucketPagesNum();

private:
  SmiUpdateFile* upFile;
  int bucketsNum;
  PageNo_t firstPageNo;

  friend class Urel1;
};

#endif //PAGEHASH_H
