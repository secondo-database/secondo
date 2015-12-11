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

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Page Hash Table in UrelAlgebra

December 2009 Jiamin Lu

1 Includes

*/

#include "PageHash.h"


using namespace std;

/*

2 Implementation of PageHashTable

2.1 Constructor of PageHashTable

If the memory pool file is created for the first time,
allocate continuous ~bucketsNum~ pages in the memory pool file
for holding the hash table, and record the first page number.

*/
PageHashTable::PageHashTable(SmiUpdateFile* ufp,
    int _bucketPageNum, int startPageNo) :
  upFile(ufp), bucketsNum(_bucketPageNum),
  firstPageNo(startPageNo)
{

  if (!startPageNo)
    {
      for (int i = 0; i < bucketsNum; i++)
        {
          SmiUpdatePage *nPage;
          if (upFile->AppendNewPage(nPage))
            {
              if (i == 0)
                firstPageNo = nPage->GetPageNo();
            }
          else
            {
              cerr << "Update Relation: Append new page for"
                  << " hash table in update file failed." << endl;
            }
        }
    }
}

PageHashTable::PageHashTable(PageHashTable& rhs):
    upFile(rhs.upFile), bucketsNum(rhs.bucketsNum),
    firstPageNo(rhs.firstPageNo)
{}

PageHashTable::~PageHashTable()
{
  upFile = 0;
}

/*

2.3 The FindEntry method of PageHashTable

Find the entry of the object with ~oid~ in the hash table.
If the entry doesn't exist when inserting the first unit of an object,
as well as there is still have space, then insert a new entry.

*/
bool PageHashTable::FindEntry(const Oid_t _oID,
    PageHashEntry& objEntry, bool wantAdd)
{
  //assert(upFile != 0);

  try
    {
      const string errMsgPrefix("PageHashTable::FindEntry: ");

      Oid_t oID = _oID;
      PageHashBucket *bucket;
      int firstBucketNo = oID % bucketsNum;
      //Get the first possible bucket number
      int bucketNo = firstBucketNo;
      bool found = false;

      int bCounter = 1;
      PageNo_t bucketPageNo;
      bool isBucketFull;
      while (1)
        {
          bucketPageNo = firstPageNo + bucketNo;
          SmiUpdatePage *bPage;
          if (!upFile->GetPage(bucketPageNo, bPage))
            throw SecondoException(errMsgPrefix
                + "Can't get bucket page" +int2Str(bucketPageNo)
                + " in update file. Line " + int2Str(__LINE__));

          bucket = new PageHashBucket(bPage);
          found = bucket->FindEntry(oID, objEntry);
          isBucketFull = bucket->disk;

/*

There are three results after finding an entry in this bucket:

  1 If found, then the entry has already copied into ~objEntry~.

  2 If not found, but this bucket is not full filled, then add a new entry
to this bucket with ascendant order on OID.

  3 If not found, and this bucket is full filled, then keep looking in
 the next bucket.

*/

          if ((!found) && wantAdd && (!isBucketFull))
            {
              //Append a new page, and add a new entry
              SmiUpdatePage *newPage;
              if (!upFile->AppendNewPage(newPage))
                throw SecondoException(errMsgPrefix
                    + "Can't append a new page in update file. Line "
                    + int2Str(__LINE__));

              objEntry.oID = _oID;
              objEntry.pageNo = newPage->GetPageNo();
              objEntry.pageAddr = newPage->GetPageAddr();

              if (!bucket->AddEntry(oID, objEntry))
                throw SecondoException(errMsgPrefix
                    + "Can't add new entry to the bucket. Line "
                    + int2Str(__LINE__));
              found = true;
            }
          delete bucket;

          if (found)
            break;

          //Get the next bucketNo
          int newBucketNo = firstBucketNo + bCounter;
          bucketNo = newBucketNo < bucketsNum ? newBucketNo :
            abs(newBucketNo - bucketsNum);

          //there is no other possible buckets
          if (++bCounter > bucketsNum)
            {
              cerr << "Through all buckets, still can't find "
                  "the entry in the hash table" << endl;
              break;
            }
        }

      return found;
    }
  catch (SecondoException e)
    {
      cerr << e.msg() << endl;
      return false;
    }
}

/*

2.4 The ChangeEntry method of PageHashTable

Change an exist entry in the hash table.

*/

bool PageHashTable::ChangeEntry(const Oid_t _oID,
    SmiUpdatePage* _newPage)
{
  //assert(upFile != 0);

  try
    {
      const string errMsgPrefix("PageHashTable::ChangeEntry: ");

      PageHashBucket* bucket;
      int firstBucketNo = _oID % bucketsNum;
      PageNo_t bucketNo = firstBucketNo;
      bool found = false;
      PageNo_t bucketPageNo;
      PageHashEntry tgtEntry;
      bool isBucketFull;

      int bCounter = 1;
      while (bCounter <= bucketsNum)
        {
          bucketPageNo = firstPageNo + bucketNo;
          SmiUpdatePage *bPage;
          if (!upFile->GetPage(bucketPageNo, bPage))
            throw SecondoException(errMsgPrefix
                + "Can't get hash page in update file. Line "
                + int2Str(__LINE__));

          bucket = new PageHashBucket(bPage);
          isBucketFull = bucket->disk;
          found = bucket->FindEntry(_oID, tgtEntry);

          if (found)
            {
              //if found, replace the old entry
              PageHashEntry newEntry;
              newEntry.oID = _oID;
              newEntry.pageNo = _newPage->GetPageNo();
              newEntry.pageAddr = _newPage->GetPageAddr();
              bucket->ChangeEntry(_oID, newEntry);
            }

          delete bucket;

          if (found)
            break;
          else if (!isBucketFull)
            {
              //Can't find an entry in an unfilled page
              cerr << "Can't find page entry with oID: "
                  << _oID << endl;
              break;
            }
          else
            {
              //search the next bucket
              int newBucketNo = firstBucketNo + bCounter;
              bucketNo = newBucketNo < bucketsNum ?
                  newBucketNo : abs(newBucketNo - bucketsNum);
              bCounter++;
            }
        }
      return found;
    }
  catch (SecondoException e)
    {
      cerr << e.msg() << endl;
      return false;
    }
}

int PageHashTable::GetBucketPagesNum()
{
  return bucketsNum;
}

/*

3 Implementation of PageHashBucket

3.1 The constructor of the PageHashBucket

Initialize the bucket by reading the head parameter in this page.

*/
PageHashBucket::PageHashBucket(SmiUpdatePage* page)
  : disk(false), extEntryNum(0),
    pagePt(page), isInitialized(false)
{
//  assert(pagePt->isAvailable());

  SmiSize offset = 0;
  pagePt->Read(&disk, sizeof(bool), offset);
  offset += sizeof(bool);
  pagePt->Read(&extEntryNum, sizeof(int), offset);
  offset += sizeof(int);
  headOffset = offset;

  maxBucketsNum = (pagePt->GetPageSize() -
      headOffset) / sizeof(PageHashEntry);
  isInitialized = true;
}

PageHashBucket::~PageHashBucket()
{
  pagePt = 0;
}

/*

3.2 AddEntry method of PageHashBucket

Insert a newEntry to the array on this bucket by ascending order.
After adding, update the ~extEntryNum~,
and if the bucket is full, mark the ~disk~ as true,
so there will be no more entries want to be added into this bucket.

*/
bool PageHashBucket::AddEntry(const Oid_t _oID, PageHashEntry& newEntry)
{
//  assert(pagePt->isAvailable());

  if (isInitialized)
    {
      int eCounter = extEntryNum;
      SmiSize entrySize = sizeof(PageHashEntry);

      //Move all entries have larger ~oID~ to the back of the array
      size_t offset = headOffset + extEntryNum * entrySize;
      while (eCounter-- > 0)
        {
        PageHashEntry lmEntry;//The last maximum entry
          pagePt->Read(&lmEntry, entrySize, offset - entrySize);
          if (lmEntry.oID > _oID)
            pagePt->Write(&lmEntry, entrySize, offset);
          else
            break;
          offset -= entrySize;
        }

      //Put the new Entry into the suitable place
      pagePt->Write(&newEntry, entrySize, offset);

      //Update the ~exist entry number~, if this bucket is full,
      //mark it with the ~disk~
      extEntryNum++;
      pagePt->Write(&extEntryNum, sizeof(int), sizeof(bool));
      if (extEntryNum == maxBucketsNum)
        {
          disk = true;
          pagePt->Write(&disk, sizeof(bool));
        }
      return true;
    }
  else
    {
      cerr << "The Bucket is not initialized for adding entry yet !" << endl;
      return false;
    }
}

/*

3.3 The FindEntry method  of PageHashBucket

Use binary search to find the entry to the data page in this bucket,
if the entry is found, its value will be returned by ~objEntry~.

*/
bool PageHashBucket::FindEntry(const Oid_t _oID, PageHashEntry &objEntry)
{
 // assert(pagePt->isAvailable());

  if (isInitialized)
    {
      if (extEntryNum == 0)
          return false;
      else
        {
          SmiSize stOffset = headOffset;
          SmiSize endOffset = headOffset
              + (extEntryNum - 1) * sizeof(PageHashEntry);
          SmiSize tgtOffset = binSearchEntryAddr(_oID, stOffset, endOffset);
          if (tgtOffset)
            {
              pagePt->Read(&objEntry, sizeof(PageHashEntry), tgtOffset);
              return true;
            }
          else
              return false;
        }
    }
  else
    {
      cerr << "The Bucket is not initialized for "
          "searching entry yet !" << endl;
      return false;
    }
}

/*

3.4 The ChangeEntry method  of PageHashBucket

Change an exist entry in this bucket, if the entry doesn't exist, return false.

*/
bool PageHashBucket::ChangeEntry(const Oid_t _oID, PageHashEntry newEntry)
{
//  assert(pagePt->isAvailable());
  if (isInitialized)
    {
      if (extEntryNum == 0)
        return false;
      else
        {
        SmiSize stOffset = headOffset;
        SmiSize endOffset = headOffset
            + (extEntryNum -1) * sizeof(PageHashEntry);
        SmiSize tgtOffset = binSearchEntryAddr(_oID, stOffset, endOffset);
        if (tgtOffset)
          {
            //Found, and change the entry
            pagePt->Write(&newEntry, sizeof(PageHashEntry), tgtOffset);
            return true;
          }
          else
            {
              cerr << "Can't find the entry in this bucket." << endl;
              return false;
            }
        }
    }
  else
    {
      cerr << "The Bucket is not initialized "
          "for searching entry yet !" << endl;
      return false;
    }
}

SmiSize PageHashBucket::binSearchEntryAddr(const Oid_t _oID,
    SmiSize _LPt, SmiSize _HPt)
{
 // assert(pagePt->isAvailable());

  if (_LPt > _HPt)
    return 0;
  else
    {
      SmiSize entrySize = sizeof(PageHashEntry);
      int EntryNum = (_HPt - _LPt) / entrySize + 1;
      int mid = EntryNum / 2;
      Oid_t midID;
      pagePt->Read(&midID, sizeof(Oid_t), _LPt + mid * entrySize);
      if (_oID == midID)
        return (_LPt + mid * entrySize);
      else if (mid == 0)
        return 0;
      else if (_oID > midID)
        return binSearchEntryAddr(_oID,
            _LPt + (mid + 1) * entrySize, _HPt);
      else
            return binSearchEntryAddr(_oID, _LPt, _LPt + (mid - 1) * entrySize);
    }
}

