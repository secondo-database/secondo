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

[1] Declaration of UrelAlgebra with Memory Pool File

December 2009 Jiamin Lu

1 Overview

The class Urel1 provides an effective way to access fix schema
update tuples of moving objects. Different from the regular way
that use Berkeley DB database file to save the tuples of a relation,
Urel1 save the tuples in a smi update file, i.e., put the data into
main memory first, not to the disk file. Compare to the disk file,
accessing memory will be more effective. And different processes
can share the Urel1 object, write and read tuples nearly at same time.
Detailed explanation can refer to the smiUpdateFile part of SecondoSMI.h.

The whole Urel1 is composed by two parts:

  * a page hash table

  * a set of data pages

The page hash table is an array of entries embedded in the pages
for searching the latest page of an object, the details of this structure
is explained in ~PageHash.h~.

The data pages is used to hold the update tuples.


2 Includes and Type Definitions

*/

#ifndef URELALGE_H
#define URELALGE_H

#include "DateTime.h"
#include "CharTransform.h"
#include "PageHash.h"
#include "../Temporal/TemporalAlgebra.h"
#include "SecondoSMI.h"
#include <time.h>
#include <sys/timeb.h>

#define TRACE_ON
#include "LogMsg.h"

typedef u_int32_t Oid_t;
typedef db_pgno_t PageNo_t;

/*

3 Definition of URelCatalogInfo

The URelCatalogInfo structure is used to store some catalog information
of the Urel1 object.

*/
struct URelCatalogInfo
{
  URelCatalogInfo() :
    isInitialized(false), fileID(-1), pageSize(-1), rootPageNum(-1),
        hashTableStartNo(-1)
  {}

  bool  operator ==(URelCatalogInfo& rhs)
  {
    bool result = true;
    result &= (isInitialized == rhs.isInitialized);
    result &= (fileID == rhs.fileID);
    result &= (pageSize == rhs.pageSize);
    result &= (rootPageNum == rhs.rootPageNum);
    result &= (hashTableStartNo == rhs.hashTableStartNo);

    return result;
  }

  URelCatalogInfo& operator =(const URelCatalogInfo& rhs)
  {
    if (this == &rhs)
      return *this;

    isInitialized = rhs.isInitialized;
    fileID = rhs.fileID;
    pageSize = rhs.pageSize;
    rootPageNum = rhs.rootPageNum;
    hashTableStartNo = rhs.hashTableStartNo;

    return *this;
  }

  bool isInitialized; //Does this update file is initialized.
  SmiFileId fileID;
  SmiSize pageSize;
  int rootPageNum;
  PageNo_t hashTableStartNo;
};

/*

4 Definition of URel1Tuple

This class defines the data structure for holding one update tuple
of moving objects. Since the Urel1 is used to access fix schema tuple,
it only contains three attributes in this tuple:
(~oid~:int, ~unit~:upoint, ~time~:instant).

*/

class URel1Tuple
{
public:
    URel1Tuple():
      id(-1), loc(), updateTime()
    {}

    URel1Tuple(int _id, temporalalgebra::UPoint _loc, Instant _ut) :
      id(_id), loc(_loc), updateTime(_ut)
    {}

    URel1Tuple(const URel1Tuple& rhs)/*:
      id(rhs.id), loc(rhs.loc), updateTime(rhs.updateTime)*/
    {
      id = rhs.id;
      loc = rhs.loc;
      updateTime = rhs.updateTime;
    }

    ~URel1Tuple()
    {
    }

    int GetID() { return id; }
    temporalalgebra::UPoint GetLoc() { return loc; }
private:
	Oid_t id;
	temporalalgebra::UPoint loc;
	Instant updateTime;

	friend class Urel1;
	friend class URel1Iterator;
	friend class URel1Page;
};

/*

5 Definition of URel1Page

The class of URel1Page provides the methods to access tuples
contained in one page of the smi update file. The main
methods of this class are listed as follow:

  * AddTuple

  * ReadOid

  * ReadTuple

The AddTuple is used to add a new tuple to the end of the page.
The ReadTuple is used to read a tuple out.
The ReadOid is used before query a tuple to ensure that the tuple is exist.

*/

class URel1Page
{
public:
  URel1Page();
  URel1Page(SmiUpdatePage* _page);
  URel1Page(const URel1Page& rhs);
  ~URel1Page();

  bool  AddTuple(URel1Tuple newTuple);
  Oid_t  ReadOid(int tupleNo);
  URel1Tuple  ReadTuple(int tupleNo);

private:
  SmiUpdatePage *ufPage;
  bool disk; //indicate the full-filled status
  PageNo_t succ; //the pointer to the next page
  SmiSize iniOffset;
  int maxTupleNum;

  bool  isFull();
  void  WriteSuccessor(PageNo_t succNo);
  int  GetSuccessor();

  friend class Urel1;
};

/*

6 Definition of  Urel1IteratorMark

Iterator to query the tuples of an urel1 object.

*/

struct Urel1IteratorMark
{
  bool moreTuple;       //To indicate whether exist more tuples
  PageNo_t pageNo;    //The page no. of the next tuple
  int indexInPage;       //The index of the tuple in the page
  Instant desTime;      //The time limit of the query
};

/*

7 Declaration of class Urel1

The class of Urel1 defines the the type of Urel1 in UrelAlgebra.
It provides two basic methods:

  * AppendTuple

  * GetNextTuple

AppendTuple is used to add a new tuple to the urel1 object.
GetNextTuple is used to get the next tuple by the Urel1IteratorMark.

*/
class PageHashTable;

class Urel1
{
public:
  Urel1(int _rootPageNum, SmiSize _pageSize = -1);
  Urel1(SmiFileId _fileID, SmiSize _pageSize = -1, int _rootPageNum = -1);
  Urel1(const Urel1& rhs);
  ~Urel1();

  bool  AppendTuple(URel1Tuple newTuple);
  URel1Tuple*  GetNextTuple(Urel1IteratorMark& itm, const Oid_t _oID = 0);

  static Word  In(const ListExpr typeInfo, const ListExpr instance,
      const int errorPos, ListExpr& errorInfo, bool& correct);
  static ListExpr  Out(ListExpr typeInfo, Word value);

  //status methods
  static void  Delete(const ListExpr typeInfo, Word& w);

  static void  Close(const ListExpr typeInfo, Word& w);

  static bool  Open(SmiRecord& valueRecord, size_t& offset,
      const ListExpr typeInfo, Word& value);

  static bool  Save(SmiRecord& valueRecord, size_t& offset,
      const ListExpr typeInfo, Word& w);

  //auxiliary functions
  int  GetStartPageNo();
  int  GetExtDataPageNum();
  PageNo_t  GetLatestPageNo(const Oid_t _oID);
  PageNo_t  GetSuccessor(const PageNo_t pageno);

private:
  Urel1() :
    cataPageNum(1)
  {}

  std::string mpName; //The memory pool name
  SmiUpdateFile *suf; //The SmiUpdateFile handle
  URelCatalogInfo urc;
  //The catalog record for this type, keep in the first page.
  PageHashTable *pht; //The PageHashTable handle
  SmiFileId fileId; //The fileID of the update file.
  const int cataPageNum;

  URel1Tuple*  GetTuple(const Urel1IteratorMark& _itm);
  int  GetTuplesInPage(const PageNo_t pageNo, const Instant desTime,
      std::vector<URel1Tuple> &tupleList);
  std::vector<URel1Tuple>  GetAllTuples(const Instant desTime);

  friend class ConstructorFunctions<Urel1> ;
};

#endif //URELALGE_H

