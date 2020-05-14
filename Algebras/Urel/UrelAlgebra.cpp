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
//[newpage] [\newpage]

[1] Implementation of UrelAlgebra with Memory Pool File

December 2009 Jiamin Lu

[TOC]

[newpage]

1 Includes,  Globals

*/

#include <vector>
#include <iostream>
#include <string>

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Counter.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "LogMsg.h"
#include "Algebras/RTree/RTreeAlgebra.h"
#include "ListUtils.h"


#include "UrelAlgebra.h"

#include "Symbols.h"
using namespace std;

extern NestedList* nl;
extern QueryProcessor* qp;

using namespace temporalalgebra;

/*

2 Implementation of URel1Page

A URel1Page is one page in the set of ~data page~s of an Urel1 object,
it is composed by three parts: (~disk~, ~succ~, ~tupleArray~).
The ~disk~ is used to indicate whether this page is filled or not.
The ~succ~ is used to indicate the closest old filled page of the same moving object,
if the page is the oldest one, then its ~succ~ is zero.
The ~tupleArray~ is used to hold the tuples of updates.

2.1 Constructor of URel1Page

In the constructor of URel1Page, if the update file page is exist,
then read the head parameters out.
The ~maxTupleNum~ is the maximum number of tuples that the ~tupleArray~ can hold.

*/

URel1Page::URel1Page() :
  ufPage(), disk(false), succ(0),
  iniOffset(0), maxTupleNum(-1)
{}

URel1Page::URel1Page(SmiUpdatePage* _page) :
    ufPage(_page), disk(false), succ(0),
    iniOffset(0), maxTupleNum(-1)
{
  SmiSize offset = 0;
  ufPage->Read(&disk, sizeof(bool), offset);
  offset += sizeof(bool);
  ufPage->Read(&succ, sizeof(PageNo_t), offset);
  offset += sizeof(PageNo_t);
  iniOffset = offset;

  maxTupleNum = (ufPage->GetPageSize() - iniOffset)
      / sizeof(URel1Tuple);
}

URel1Page::~URel1Page()
{
  ufPage = 0;
}

URel1Page::URel1Page(const URel1Page& rhs) :
  ufPage(rhs.ufPage), disk(rhs.disk), succ(rhs.succ),
      iniOffset(rhs.iniOffset), maxTupleNum(rhs.maxTupleNum)
{}

/*

2.2 AddTuple method in URel1Page

Append a new tuple to the end of the ~tupleArray~.
Note that when writing the tuple, the ~oid~ of this tuple will be written at last.
Because when reading the tuple, it needs to read the ~oid~ out first
to check whether this tuple is exist.

*/
bool  URel1Page::AddTuple(URel1Tuple newTuple)
{
  //mark the disk after adding newTuple
  bool result = false;
  if ((ufPage->isAvailable()) && !disk)
    {
      int extTupNum = 0;
      while ((extTupNum < maxTupleNum) && (ReadOid(extTupNum) > 0))
          extTupNum++;

      if (extTupNum >= maxTupleNum)
        {
          disk = true;
          result = false;
          //set the ~disk~(full) mark to the page
          ufPage->Write(&disk, sizeof(bool), 0);
        }
      else
        {
          //Write the new tuple to the end of this page
           SmiSize offset = iniOffset
               + extTupNum * sizeof(URel1Tuple);
           ufPage->Write(&(newTuple.updateTime), sizeof(Instant),
               offset + sizeof(Oid_t) + sizeof(UPoint));
           ufPage->Write(&(newTuple.loc),
               sizeof(UPoint), offset + sizeof(Oid_t));
           ufPage->Write(&(newTuple.id), sizeof(Oid_t), offset);
           result = true;
        }
    }
  return result;
}

bool  URel1Page::isFull()
{
  return disk;
}

/*

2.3 ReadTuple and ReadOid method of URel1Page

As mentioned above, every time when reading a tuple out,
it needs to read the ~oid~ out first to ensure that the tuple is exist.

*/
Oid_t  URel1Page::ReadOid(int tupleNo)
{
//  assert(ufPage->isAvailable());
//  assert(tupleNo >= 0);
//  assert(tupleNo <= maxTupleNum);
  Oid_t id;
  ufPage->Read(&id, sizeof(Oid_t),
      (iniOffset + tupleNo * sizeof(URel1Tuple)));
  return id;
}

URel1Tuple  URel1Page::ReadTuple(int tupleNo)
{
  //assert(ufPage->isAvailable());
  //assert(tupleNo >= 0);
  //assert(tupleNo <= maxTupleNum);

  URel1Tuple tuple;
  ufPage->Read(&tuple, sizeof(URel1Tuple), (iniOffset + tupleNo
      * sizeof(URel1Tuple)));

  return tuple;
}

void URel1Page::WriteSuccessor(PageNo_t succNo)
{
  //assert(ufPage->isAvailable());

  ufPage->Write(&succNo, sizeof(PageNo_t), sizeof(bool));
  succ = succNo;
}

int  URel1Page::GetSuccessor()
{
 // assert(ufPage->isAvailable());

  ufPage->Read(&succ, sizeof(PageNo_t), sizeof(bool));
  return succ;
}

/*

3 Implementation of  type Urel1

3.1 Constructors of Urel1

There are three different constructors of the Urel1:

  1 Basic constructor. Used to create a new Urel1 object, that is to create the smi update file,
initiate the page hash table, and write catalog information.

  2 Copy constructor. Used to create a new Urel1 object from an exist one. Two objects sharing
a same smi update file.

  3 Reopen constructor. Similar like copy constructor, used to reopen a exist smi update file,
but need to check the catalog information to ensure that the file is same.

*/
Urel1::Urel1(int _rootPageNum, SmiSize _pageSize) :
	mpName(""), suf(0), urc(), pht(0), fileId(0), cataPageNum(1)
{
  try
    {
      const string errMsgPrefix("URel constructor: ");

      if (_rootPageNum <= 0)
        throw SecondoException(errMsgPrefix
            + "Root page Number must be positive. Line "
            + int2Str(__LINE__));
      if(_pageSize <= 0)
        throw SecondoException(errMsgPrefix
            + "Page size must be positive. Line "
            + int2Str(__LINE__));
      else
        {
          suf = new SmiUpdateFile(_pageSize);
          if (suf->Create())
            {
              mpName = suf->GetName();
              fileId = suf->GetFileId();
              SmiUpdatePage* catalogPage;

              urc.fileID = fileId;
              urc.pageSize = suf->GetPoolPageSize();
              urc.rootPageNum = _rootPageNum;
              if (suf->AppendNewPage(catalogPage))
                {
                  pht = new PageHashTable(suf, _rootPageNum);
                  urc.hashTableStartNo = pht->firstPageNo;
                  urc.isInitialized = true;
                  if (catalogPage->
                      Write(&urc, sizeof(URelCatalogInfo)) <= 0)
                    throw SecondoException(errMsgPrefix
                        + "Can't write catalog page in "
                        "update file. Line " + int2Str(__LINE__));
                }
              else
                throw SecondoException(errMsgPrefix
                    + "Can't append new page in file. Line "
                    + int2Str(__LINE__));
            }
          else
            throw SecondoException(errMsgPrefix
                + "Can't create update file. Line "
                + int2Str(__LINE__));
        }
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
      assert(false);
    }
}

Urel1::Urel1(const Urel1& rhs) :
	suf(0), urc(), pht(0), fileId(rhs.fileId), cataPageNum(1)
{
  try
    {
      const string errMsgPrefix("URel copy constructor: ");

      SmiSize pageSize = rhs.suf->GetPoolPageSize();
      mpName = rhs.suf->GetName();
      urc = rhs.urc;

      suf = new SmiUpdateFile(pageSize);
      if (suf->Open(fileId, pageSize, rhs.suf->GetIniStatus()))
        {
          //Catalog info checking
          SmiUpdatePage *catalogPage;
          if (suf->GetPage(1, catalogPage))
            {
              URelCatalogInfo extUrc;
              catalogPage->Read(&extUrc, sizeof(URelCatalogInfo));

              if (extUrc == urc)
              {
                pht = new PageHashTable(*(rhs.pht));
              }
              else
                throw SecondoException(errMsgPrefix
                    + "Catalog info doesn't match. Line "
                    + int2Str(__LINE__));
            }
          else
            throw SecondoException(errMsgPrefix
                + "Can't get the catalog page. Line "
                + int2Str(__LINE__));
        }
      else
        throw SecondoException(errMsgPrefix
            + "Can't reopen an exist update file. Line "
            + int2Str(__LINE__));
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
      assert(false);
    }
}

Urel1::Urel1(SmiFileId _fileID, SmiSize _pageSize, int _rootPageNum):
    suf(0), urc(), pht(0), fileId(_fileID), cataPageNum(1)
{
  try
    {
      const string errMsgPrefix("URel re-constructor: ");

      urc.pageSize = _pageSize;
      urc.rootPageNum = _rootPageNum;
      suf = new SmiUpdateFile(_pageSize);
      if (suf->Open(_fileID, _pageSize))
        {
          mpName = suf->GetName();

          SmiUpdatePage *catalogPage;
          if (suf->GetPage(1, catalogPage))
            {
              URelCatalogInfo extUrc;
              catalogPage->Read(&extUrc, sizeof(URelCatalogInfo));

              if (urc.pageSize == extUrc.pageSize && urc.rootPageNum
                  == extUrc.rootPageNum)
                {
                  urc.hashTableStartNo = extUrc.hashTableStartNo;
                  pht = new PageHashTable(suf, urc.rootPageNum,
                      urc.hashTableStartNo);
                }
              else
                throw SecondoException(errMsgPrefix
                    + "Catalog basic info doesn't match. Line "
                    + int2Str(__LINE__));
            }
          else
            throw SecondoException(errMsgPrefix
                + "Can't get the catalog page. Line "
                + int2Str(__LINE__));
        }
      else
        throw SecondoException(errMsgPrefix
            + "Can't reopen an exist update file. Line "
            + int2Str(__LINE__));
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
      assert(false);
    }
}

Urel1::~Urel1()
{
  delete pht;
  if (suf != 0)
    {
      suf->Close();
      delete suf;
      suf = 0;
    }
}

/*

3.1 AppendTuple method in Urel1

When append a new tuple to the Urel1 object, it follows below steps:

  1 Search the PageHashTable to see whether there is a page hash entry.

  2 If the entry exist, according to the ~pageNo~ of the entry to get the
corresponding ~data page~. If that page is not full, add the new tuple to
the end of its ~tupleArray~. If the page is full, then mark its ~disk~ as true,
create a new ~data page~, set its ~succ~ as the ~pageNo~, change the
hash entry, and add the tuple to the new page.

  3 If the entry is not exist, then create a new data page, add a new hash entry,
and add the tuple to the new data page.

*/
bool Urel1::AppendTuple(URel1Tuple newTuple)
{
  //assert(suf != 0);
  //assert(pht != 0);

  try
    {
      const string errMsgPrefix("URel AppendTuple: ");

      PageHashEntry pageEntry;
      if (!pht->FindEntry(newTuple.id, pageEntry, true))
        throw SecondoException(errMsgPrefix
            + "Can't find page entry in hash table. Line "
            + int2Str(__LINE__));
      PageNo_t firstPageNo = pageEntry.pageNo;

      SmiUpdatePage *upFilePage;
      if (!suf->GetPage(firstPageNo, upFilePage))
        throw SecondoException(errMsgPrefix
            + "Can't get data page in update file. Line "
            + int2Str(__LINE__));
      URel1Page *urPage = new URel1Page(upFilePage);

      if (!urPage->AddTuple(newTuple))
        {
          if (urPage->isFull())
            {
              suf->PutPage(firstPageNo);

              SmiUpdatePage *newFilePage;
              //Append a new page.
              if (!suf->AppendNewPage(newFilePage))
                throw SecondoException(errMsgPrefix
                    + "Can't append new page in update file. Line "
                    + int2Str(__LINE__));
              URel1Page *newUrPage = new URel1Page(newFilePage);

              //set the successor
              newUrPage->WriteSuccessor(firstPageNo);

              //change the entry info in hash ...
              if (!pht->ChangeEntry(newTuple.id, newFilePage))
                throw SecondoException(errMsgPrefix
                    + "Can't change the hash entry. Line "
                    + int2Str(__LINE__));

              if(!newUrPage->AddTuple(newTuple))
                throw SecondoException(errMsgPrefix
                    + "Can't add tuple to a new page. Line "
                    + int2Str(__LINE__));

              delete newUrPage;
            }
          else
            throw SecondoException(errMsgPrefix
                + "Can't add more new tuple into "
                  "the unfilled page. Line " + int2Str(__LINE__));
        }

      delete urPage;
      return true;
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
      return false;
    }
}

/*

3.2 GetTuplesInPage in Urel1

Get all tuple in one ~data page~ before the ~desTime~.
Put the all tuples into the vector ~tupleList~.

*/
int Urel1::GetTuplesInPage(const PageNo_t pageNo,
    const Instant desTime, vector<URel1Tuple> &tupleList)
{

  try
    {
      const string errMsgPrefix("URel GetTuplesInPage: ");
      SmiUpdatePage* filePage;
      int oldNum = tupleList.size();

      if (!suf->GetPage(pageNo, filePage))
        throw SecondoException(errMsgPrefix
            + "Can't get data page in update file. Line "
            + int2Str(__LINE__));

      URel1Page urPage(filePage);
      int extTupNum = 0;
      while ((extTupNum < urPage.maxTupleNum)
          && (urPage.ReadOid(extTupNum) > 0))
        {
          URel1Tuple tuple = urPage.ReadTuple(extTupNum);
          if (tuple.updateTime < desTime)
            {
              tupleList.push_back(tuple);
              extTupNum++;
            }
          else
            break;
        }
      return tupleList.size() - oldNum;
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
      return -1;
    }
}

/*

3.3  GetTuple method in Urel1

Get a tuple inside the smiUpdateFile by the ~Urel1IteratorMark~.
If there is no more tuples before the query instant, return 0.

*/

URel1Tuple* Urel1::GetTuple(const Urel1IteratorMark& _itm)
{
  try
    {
      const string errMsgPrefix("URel::GetTuple: ");
      SmiUpdatePage* filePage;
      URel1Page *urPage;
      URel1Tuple *tuple = 0;

      if (!suf->GetPage(_itm.pageNo, filePage))
        throw SecondoException(errMsgPrefix
            + "Can't get data page in update file. Line "
            + int2Str(__LINE__));
      urPage = new URel1Page(filePage);

      if ((_itm.indexInPage < urPage->maxTupleNum)
          && (urPage->ReadOid(_itm.indexInPage) > 0))
        {
          //The tuple is exist because its ~oid~ is readable.
         URel1Tuple ttp = urPage->ReadTuple(_itm.indexInPage);
          if (ttp.updateTime > _itm.desTime)
            {
              //delete tuple;
              tuple = 0;
            }
          tuple = new URel1Tuple(ttp);
        }

      delete urPage;
      //delete filePage;
      return tuple;
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
      return 0;
    }
}

/*

3.4 GetNextTuple method in Urel1

Get the next tuple by the Urel1IteratorMark ~itm~. If get one tuple, return it.
The way to get the next page is different with different operators.

  * In ~feedObject~ operator, if doesn't get, but there exist successive
page it according to the\_oID, then move to the successive page.

  * In ~feed~ operator, if doesn't get, but there still be more pages,
move to the next page,  and query the tuple again.

*/

URel1Tuple* Urel1::GetNextTuple(
    Urel1IteratorMark& itm, const Oid_t _oID)
{
  while (itm.moreTuple)
    {
      URel1Tuple *tuple = GetTuple(itm);
      itm.indexInPage++;
      PageNo_t succ;
      PageNo_t cPageNo = itm.pageNo;

      if (tuple != 0)
          return tuple;
      else
        {
          if ((_oID > 0) && ((succ = GetSuccessor(cPageNo)) > 0))
            {
              //Put the useless page back to the disk file
              suf->PutPage(cPageNo, false);
              itm.pageNo = succ;
              itm.indexInPage = 0;

            }
          else if ((_oID == 0)
              && ((int)cPageNo < suf->GetExistPageNum()))
            {
              suf->PutPage(cPageNo, false);
              itm.pageNo++;
              itm.indexInPage = 0;
            }
          else
            {
              // no tuples at all
              itm.moreTuple = false;
              return 0;
            }
        }
    }
  return 0;
}

PageNo_t Urel1::GetSuccessor(const PageNo_t pageno)
{
  try
    {
      const string errMsgPrefix("URel: GetSuccessor");
      SmiUpdatePage *filePage;

      if (!suf->GetPage(pageno, filePage))
        throw SecondoException(errMsgPrefix
            + "Can't get data page in update file. Line "
            + int2Str(__LINE__));

      URel1Page urPage(filePage);
      PageNo_t succ = urPage.GetSuccessor();
      return succ;
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
      assert(false);
      return 0;
    }
}

PageNo_t Urel1::GetLatestPageNo(const Oid_t _oID)
{
  try
    {
      const string errMsgPrefix("URel: GetFirstPageNo");

      PageHashEntry pageEntry;
      if (!pht->FindEntry(_oID, pageEntry))
        throw SecondoException(errMsgPrefix
            + "Can't find page entry in hash table. Line "
            + int2Str(__LINE__));
      return pageEntry.pageNo;
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
      return 0;
    }
}

int Urel1::GetStartPageNo()
{
  return cataPageNum + pht->GetBucketPagesNum() + 1;
}

int Urel1::GetExtDataPageNum()
{
  return suf->GetExistPageNum()
      - cataPageNum - pht->GetBucketPagesNum();
}

vector<URel1Tuple> Urel1::GetAllTuples(const Instant desTime)
{
  vector<URel1Tuple> tupleList;

  try
    {
      const string errMsgPrefix("URel GetAllTuples: ");

      //TODO ---JIAMIN--- better if output can order by Oid
      PageNo_t maxPageNo = suf->GetExistPageNum();
      int startPageNo = cataPageNum + pht->GetBucketPagesNum() + 1;
      for (PageNo_t pageNo = startPageNo;
          pageNo <= maxPageNo; pageNo++)
        {
          if (GetTuplesInPage(pageNo, desTime, tupleList) < 0)
            throw SecondoException(errMsgPrefix
                + "Can't get tuples from page '"
                + int2Str(pageNo) + "'. Line "
                + int2Str(__LINE__));
        }
    }
  catch (SecondoException& e)
    {
      cerr << e.msg() << endl;
    }

  return tupleList;
}

/*

3.5 In and Out methods in Urel1

  * In method is used to create a new urel1 object, according to the ~instant~ value list.

  * Out method is used to print all tuples in this object out with nested list.

*/
Word Urel1::In(const ListExpr typeInfo, const ListExpr instance,
    const int errorPos, ListExpr& errorInfo, bool& correct)
{
  correct = false;
  Word result = SetWord(Address(0));
  const string errMsg = "Urel expect value expression as: "
    "(pageSize:int, rootPageNum:int, tuples )";
  Urel1* uRel;
  NList valList(instance);

  if (valList.length() != 3)
    {
      errorInfo = nl->Append(errorInfo,
          nl->ThreeElemList(nl->IntAtom(70),
          nl->SymbolAtom(errMsg), instance));
      return result;
    }

  NList first = valList.first();
  NList second = valList.second();
  NList third = valList.third();

  if (first.isInt() && second.isInt() && third.isList())
    {
      SmiSize pageSize = first.intval();
      int rootPageNum = second.intval();

      uRel = new Urel1(rootPageNum, pageSize);
      correct = true;

      if (third.length() > 0)
        {
          //append the tuples into the value;
          NList tupleList;
          NList restList = third;

          while (!restList.isEmpty())
            {
              tupleList = restList.first();
              restList.rest();

              UPoint *loc =
                  (UPoint*) InUPoint(nl->TheEmptyList(),
                      tupleList.second().listExpr(),
                      errorPos, errorInfo, correct).addr;
              Instant *updateTime =
                  new datetime::DateTime(tupleList.third().realval());

              if (tupleList.length() == 3
                  && tupleList.first().isInt()
                  && loc->IsDefined() && loc->IsValid()
                  && (!updateTime->IsZero()))
                {
                  int tID = tupleList.first().intval();

                  URel1Tuple newTuple(tID, *loc, *updateTime);
                  if (!(uRel->AppendTuple(newTuple)))
                    {
                      // Append new Tuple error!
                      correct = false;
                      break;
                    }
                }
              else
                {
                  // Tuple list check error!
                  correct = false;
                  break;
                }

              delete loc;
              delete updateTime;
            }
        }

      if (correct)
        result.addr = uRel;
      else
        errorInfo = nl->Append(errorInfo,
            nl->ThreeElemList(nl->IntAtom(70),
            nl->SymbolAtom(errMsg), instance));
    }
  else
    {
      errorInfo = nl->Append(errorInfo,
          nl->ThreeElemList(nl->IntAtom(70),
          nl->SymbolAtom(errMsg), instance));
    }

  return result;
}

ListExpr Urel1::Out(ListExpr typeInfo, Word value)
{
  Urel1* rel = static_cast<Urel1*> (value.addr);

  struct timeval _now;
  gettimeofday(&_now, NULL);
  Instant NOW(int64_t(_now.tv_sec));
  vector<URel1Tuple> tuples;
  tuples = rel->GetAllTuples(NOW);

  NList resultList(nl);
  resultList.append(NList((int) rel->suf->GetPoolPageSize()));
  resultList.append(NList(rel->pht->GetBucketPagesNum()));

  NList tupleList(nl);
  for (size_t i = 0; i < tuples.size(); i++)
    {
      int ID = tuples[i].id;
      UPoint loc = tuples[i].loc;
      Instant upTime = tuples[i].updateTime;

      NList aTuple(NList(ID), NList(
          OutUPoint(nl->TheEmptyList(), SetWord(&loc)), nl),
          NList(upTime.ToListExpr(false), nl));

      tupleList.append(aTuple);
    }
  resultList.append(tupleList);

  return resultList.listExpr();
}

void Urel1::Delete(const ListExpr typeInfo, Word& w)
{
  Urel1* r = static_cast<Urel1*>( w.addr );
  delete r;
  w.addr = 0;
}

void Urel1::Close(const ListExpr typeInfo, Word& w)
{
  delete static_cast<Urel1*>(w.addr);
  w.addr = 0;
}

bool Urel1::Open(SmiRecord& valueRecord, size_t& offset,
		const ListExpr typeInfo, Word& value)
{
  SmiFileId _fd = 0; //fileID
  SmiSize _ps = 0; //pageSize
  int _bn = -1; //buckets number | root Pages number

  bool ok = true;
  ok = ok && valueRecord.Read(&_fd, sizeof(SmiFileId), offset);
  offset += sizeof(SmiFileId);
  ok = ok && valueRecord.Read(&_ps, sizeof(SmiSize), offset);
  offset += sizeof(SmiSize);
  ok = ok && valueRecord.Read(&_bn, sizeof(int), offset);
  offset += sizeof(int);

  Urel1 *rel = new Urel1(_fd, _ps, _bn);
  value.setAddr(rel);
  return ok;
}

bool Urel1::Save(SmiRecord& valueRecord, size_t& offset,
		const ListExpr typeInfo, Word& w)
{
  Urel1* r = static_cast<Urel1*> (w.addr);

  //Save necessary parameters into the ~catalog~ record
  SmiFileId _fd = r->suf->GetFileId();
  SmiSize _ps = r->suf->GetPoolPageSize();
  int _bn = r->pht->GetBucketPagesNum();

  bool ok = true;
  ok = ok && valueRecord.Write(&_fd, sizeof(SmiFileId), offset);
  offset += sizeof(SmiFileId);
  ok = ok && valueRecord.Write(&_ps, sizeof(SmiSize), offset);
  offset += sizeof(SmiSize);
  ok = ok && valueRecord.Write(&_bn, sizeof(int), offset);
  offset += sizeof(int);

  return ok;
}

struct urel1Info: ConstructorInfo
{
	urel1Info()
	{
		name = "urel1";
		signature = "TUPLE->REL";
		typeExample = "urel1";
		listRep = "(pageSize, rootPageNum, (tuplelist))";
		valueExample = "(1024, 10, ())";
		remarks = "let myUrel = [const urel1 value (1024 10 ())];";
	}
};

struct urel1Functions: ConstructorFunctions<Urel1>
{
	urel1Functions()
	{
		in = Urel1::In;
		out = Urel1::Out;

		deletion = Urel1::Delete;
		close = Urel1::Close;
		open = Urel1::Open;
		save = Urel1::Save;
	}
};

urel1Info ur1i;
urel1Functions ur1f;
TypeConstructor urel1TC(ur1i, ur1f);

/*

4 Operators

4.1 Operator ~uinsert~

The ~uinsert~ operator of UrelAlgebra is used to append all tuples in a
tuple stream to a urel1 object. This operator maps

---- stream(tuple((_ int) (_ upoint) (_ instant))) x urel1 -> int
----

*/

struct urelInsertInfo : OperatorInfo {

	urelInsertInfo()
  {
    name      = "uinsert";
    signature = Symbol::STREAM() + " x urel1 -> " + CcInt::BasicType();
    syntax    = "_ _ uinsert";
    meaning   = "Insert a stream of tuples into urel1";
  }

};

/*

4.1.1 Type mapping

*/
ListExpr UInsertTypeMap(ListExpr args)
{
  NList type(args);

  if (type.length() != 2)
    return type.typeError("Expect two arguments.");

  NList streamList = type.first();
  NList urelList = type.second();

  if (!listutils::isTupleStream(streamList.listExpr()))
      return type.typeError(
          "Expect the first argument is a tuple stream.");

  if (!urelList.isSymbol("urel1"))
    return type.typeError(
        "Expect the second argument is urel1 type");

  NList tuplesList = streamList.second().second();

  if (tuplesList.length() != 3)
    return type.typeError(
        "Expect three attributes inside one tuple.");

  NList _1Attr = tuplesList.first().second();
  NList _2Attr = tuplesList.second().second();
  NList _3Attr = tuplesList.third().second();

  if (!_1Attr.isSymbol(CcInt::BasicType()) ||
      !_2Attr.isSymbol(UPoint::BasicType()) ||
      !_3Attr.isSymbol(Instant::BasicType()) )
    return type.typeError("Expect (int, upoint, instant) tuple");

  return nl->SymbolAtom(CcInt::BasicType());
}

/*

4.1.2 Value mapping

*/
int UInsertValueMap(Word* args, Word& result,
		int message, Word& local, Supplier s)
{
  Word actual;

  //Get the storage space for the result
  CcInt* ir = static_cast<CcInt*> ((qp->ResultStorage(s)).addr);

  int addTuples = 0;
  Urel1 *urel = static_cast<Urel1*> (args[1].addr);

  qp->Open(args[0].addr);
  qp->Request(args[0].addr, actual);
  while (qp->Received(args[0].addr))
    {
      Tuple* tuple = (Tuple*) actual.addr;
      int oid = ((CcInt*) tuple->GetAttribute(0))->GetIntval();
      UPoint unit = *((UPoint*) tuple->GetAttribute(1));
      Instant time = *((Instant*) tuple->GetAttribute(2));

      URel1Tuple new1tuple(oid, unit, time);
      if (urel->AppendTuple(new1tuple))
          addTuples++;
      tuple->DeleteIfAllowed();
      qp->Request(args[0].addr, actual);
    }

  ir->Set(true, addTuples);
  result.addr = ir;

  return 0;
}

struct uFeedLocalInfo
{
  TupleType *resultTupleType;
  Urel1IteratorMark *itm;
};

/*

4.2 Operator ~ufeed~

This operator is used to get all tuples which ~updateTime~ is earlier
than an instant to a tuple stream. This operator maps

---- urel1 x instant -> stream(tuple([id:int, loc upoint]))
----

*/

struct urelFeedInfo : OperatorInfo {

	urelFeedInfo()
  {
    name      = "ufeed";
    signature = "urel1 x Instant -> " + Symbol::STREAM();
    syntax    = "_ ufeed [ _ ] ";
    meaning   = "Get all tuples before an instant";
  }

};

/*

4.2.1 Type mapping

*/
ListExpr UFeedTypeMap(ListExpr args)
{
  if (nl->ListLength(args) != 2)
    return listutils::typeError(
      "Operator feed expects a list of length two.");
  NList l(args);
  NList first = l.first();
  NList second = l.second();

  if (!first.isSymbol("urel1"))
    return l.typeError(
        "Expect the first argument is urel1 type.");

  if (!second.isSymbol(Instant::BasicType()))
      return l.typeError(
          "Expect the second argument is instant type.");

  ListExpr attrList = nl->OneElemList(nl->TwoElemList(
      nl->StringAtom("oid", false), nl->SymbolAtom(CcInt::BasicType())));
  nl->Append(attrList, nl->TwoElemList(
      nl->StringAtom(UInt::BasicType(), false),
      nl->SymbolAtom(UPoint::BasicType())));

  NList AttrList(attrList, nl);
  NList outList = NList(NList().tupleStreamOf(AttrList));
  return outList.listExpr();
}

/*

4.2.2 Value mapping

*/
int UFeedValueMap(Word* args, Word& result,
		int message, Word& local, Supplier s)
{
  Urel1* urel = 0;
  Urel1IteratorMark *itm = 0;
  ListExpr resultType;
  TupleType *resultTupleType = 0;
  uFeedLocalInfo *localInfo = 0;
  URel1Tuple* urt;

  switch (message)
    {
  case OPEN:
    urel = (Urel1*) args[0].addr;

    localInfo = new uFeedLocalInfo;

    localInfo->itm = new Urel1IteratorMark;
    localInfo->itm->pageNo = urel->GetStartPageNo();
    localInfo->itm->indexInPage = 0;
    localInfo->itm->moreTuple = true;
    localInfo->itm->desTime = *((Instant*) args[1].addr);

    resultType = GetTupleResultType(s);
    localInfo->resultTupleType = new TupleType(nl->Second(resultType));

    local = SetWord(localInfo);

    return 0;
  case REQUEST:
    urel = (Urel1*) args[0].addr;
    Tuple *newTuple;

    if (local.addr)
      {
        localInfo = (uFeedLocalInfo*)local.addr;
        itm = localInfo->itm;
        resultTupleType = localInfo->resultTupleType;
        newTuple = new Tuple(resultTupleType);
      }
    else
      {
        return CANCEL;
      }

    urt = urel->GetNextTuple(*itm);
    if (urt != 0)
      {
        newTuple->PutAttribute(0, new CcInt(true, urt->GetID()));
        newTuple->PutAttribute(1, new UPoint(urt->GetLoc()));
        delete urt;
        result = SetWord(newTuple);
        return YIELD;
      }
    else
      {
        return CANCEL;
      }
  case CLOSE:
    if (local.addr)
      {
        localInfo = (uFeedLocalInfo*)local.addr;
        delete localInfo->itm;
        delete localInfo->resultTupleType;
        delete localInfo;
      }
    return 0;
    }

  return 0;
}

/*

4.3 Operator ~ufeedObject~

This operator is used to get all tuples which are with a same ~oid~,
and their ~updateTime~ is earlier than an instant to a tuple stream.
This operator maps

---- urel1 x instant x int -> stream(tuple([id:int, loc upoint]))
----

*/


struct urelUFeedObjectInfo : OperatorInfo
{
  urelUFeedObjectInfo()
  {
    name = "ufeedobject";
    signature = "urel1 x Instant x " + CcInt::BasicType() + " -> "
                + Symbol::STREAM();
    syntax = "_ ufeedobject [ _ , _ ] ";
    meaning = "Get all tuples before an instant with a same id";
  }
};

/*

4.3.1 Type mapping

*/
ListExpr UFeedObjectTypeMap(ListExpr args)
{

  if (nl->ListLength(args) != 3)
      return listutils::typeError(
          "Operator feed expects a list of length three.");

  NList l(args);
  NList first = l.first();
  NList second = l.second();
  NList third = l.third();

  if (!first.isSymbol("urel1"))
    return l.typeError("Expect the first argument is urel1 type.");

  if (!second.isSymbol(Instant::BasicType()))
    return l.typeError(
        "Expect the second argument is instant type.");

  if (!third.isSymbol(CcInt::BasicType()))
    return l.typeError("Expect the third argument is int type");

  ListExpr attrList = nl->OneElemList(
      nl->TwoElemList(nl->StringAtom("oid", false),
          nl->SymbolAtom(CcInt::BasicType())));
  nl->Append(attrList, nl->TwoElemList(
      nl->StringAtom(UInt::BasicType(), false),
      nl->SymbolAtom(UPoint::BasicType())));

  ListExpr outList = nl->TwoElemList(
      nl->SymbolAtom(Symbol::STREAM()),
      nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()), attrList));

  return outList;
}

/*

4.3.2 Value mapping

*/
int UFeedObjectValueMap(Word* args, Word& result,
		int message, Word& local, Supplier s)
{

  Urel1* urel;
  Urel1IteratorMark *itm;
  ListExpr resultType;
  TupleType *resultTupleType;
  uFeedLocalInfo *localInfo = 0;
  Oid_t oID;
  PageNo_t firstPageNo;
  URel1Tuple* urt;

  switch (message)
    {
  case OPEN:
    urel = (Urel1*) args[0].addr;
    oID = ((CcInt*) args[2].addr)->GetIntval();

    firstPageNo = urel->GetLatestPageNo(oID);

    localInfo = new uFeedLocalInfo;

    localInfo->itm = new Urel1IteratorMark;
    localInfo->itm->pageNo = firstPageNo;
    localInfo->itm->indexInPage = 0;
    localInfo->itm->moreTuple = true;
    localInfo->itm->desTime = *((Instant*) args[1].addr);

    resultType = GetTupleResultType(s);
    localInfo->resultTupleType = new TupleType(nl->Second(resultType));

    local = SetWord(localInfo);

    return 0;

  case REQUEST:
    urel = (Urel1*) args[0].addr;
    oID = ((CcInt*) args[2].addr)->GetIntval();
    Tuple *newTuple;

    if (local.addr)
      {
        //itm = (Urel1IteratorMark*) local.addr;
        localInfo = (uFeedLocalInfo*) local.addr;
        itm = localInfo->itm;
        resultTupleType = localInfo->resultTupleType;
        newTuple = new Tuple(resultTupleType);
      }
    else
      {
        return CANCEL;
      }

    urt = urel->GetNextTuple(*itm, oID);
    if (urt != 0)
      {
        newTuple->PutAttribute(0, new CcInt(true, urt->GetID()));
        newTuple->PutAttribute(1, new UPoint(urt->GetLoc()));
        delete urt;
        result = SetWord(newTuple);
        return YIELD;
      }
    else
      {
        return CANCEL;
      }

  case CLOSE:
    if (local.addr)
      {
        localInfo = (uFeedLocalInfo*) local.addr;
        delete localInfo->itm;
        delete localInfo->resultTupleType;
        delete localInfo;
      }
    return 0;
    }

  return 0;
}

/*

3 Class ~UrelAlgebra~

A new subclass ~UrelAlgebra~ of class ~Algebra~ is declared. The only
specialization with respect to class ~Algebra~ takes place within the
constructor: all type constructors and operators are registered at the actual algebra.

After declaring the new class, its only instance ~extendedRelationAlgebra~ is defined.

*/

class UrelAlgebra: public Algebra
{
public:
  UrelAlgebra() :
    Algebra()
  {
    AddTypeConstructor(&urel1TC);

    AddOperator(urelInsertInfo(), UInsertValueMap, UInsertTypeMap);
    AddOperator(urelFeedInfo(), UFeedValueMap, UFeedTypeMap);
    AddOperator(urelUFeedObjectInfo(), UFeedObjectValueMap, UFeedObjectTypeMap);
  }
  ~UrelAlgebra()
  {};
};

/*

4 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C" Algebra*
InitializeUrelAlgebra(NestedList* nlRef, QueryProcessor* qpRef)
{
	nl = nlRef;
	qp = qpRef;
	return (new UrelAlgebra());
}





