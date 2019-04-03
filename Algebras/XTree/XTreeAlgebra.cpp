/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

1 Implementation file "XTreeAlgebra.cpp"[4]

January-May 2008, Mirko Dibbert

*/
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "XTree.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

using namespace std;
using namespace gta;

namespace xtreeAlgebra
{

/********************************************************************
1.1 Type constructor ~xtree~

********************************************************************/
static ListExpr XTreeProp()
{
    ListExpr examplelist = nl->TextAtom();
    nl->AppendText(examplelist, "<relation> creatextree [<attrname>]"
                                     " where <attrname> is the key");

    return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
                    nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
                    nl->StringAtom("(let myxtree = pictures "
                                    "creatextree[pic] "))));
}

ListExpr OutXTree(ListExpr type_Info, Word w)
{
    #ifdef __XTREE_OUTFUN_PRINT_STATISTICS
    static_cast<XTree*>(w.addr)->printTreeInfos();
    #endif
    return nl->TheEmptyList();
}

Word InXTree(
        ListExpr type_Info, ListExpr value,
        int errorPos, ListExpr &error_Info, bool &correct)
{
    correct = false;
    return SetWord(Address(0));
}

Word CreateXTree(const ListExpr type_Info)
{ return SetWord(new XTree()); }

void DeleteXTree(const ListExpr type_Info, Word &w)
{
    static_cast<XTree*>(w.addr)->deleteFile();
    delete static_cast<XTree*>(w.addr);
    w.addr = 0;
}

bool OpenXTree(
        SmiRecord &valueRecord, size_t &offset,
        const ListExpr type_Info, Word &w)
{
    SmiFileId fileid;
    valueRecord.Read(&fileid, sizeof(SmiFileId), offset);
    offset += sizeof(SmiFileId);

    XTree *xtree = new XTree(fileid);
    w = SetWord(xtree);
    return true;
}

bool SaveXTree(
        SmiRecord &valueRecord, size_t &offset,
        const ListExpr type_Info, Word &w)
{
    SmiFileId fileId;
    XTree *xtree = static_cast<XTree*>(w.addr);
    fileId = xtree->fileId();
    if (fileId)
    {
        valueRecord.Write(&fileId, sizeof(SmiFileId), offset);
        offset += sizeof(SmiFileId);
        return true;
    }
    else
    {
        return false;
    }
}

void CloseXTree(const ListExpr type_Info, Word &w)
{
    XTree *xtree = (XTree*)w.addr;
    delete xtree;
}

Word CloneXTree(const ListExpr type_Info, const Word &w)
{ return SetWord(Address(0)); }

int SizeOfXTree()
{ return sizeof(XTree); }

bool CheckXTree(ListExpr typeName, ListExpr &error_Info)
{ return nl->IsEqual(typeName, XTree::BasicType()); }

TypeConstructor xtreeTC(
        XTree::BasicType(),       XTreeProp,
        OutXTree,    InXTree,
        0, 0,
        CreateXTree, DeleteXTree,
        OpenXTree,   SaveXTree,
        CloseXTree,  CloneXTree,
        0,
        SizeOfXTree,
        CheckXTree);

/********************************************************************
1.1 Operators

1.1.1 Value mappings

1.1.1.1 creatextreeHPointRel[_]VM

This value mapping function is used for all "creatextree"[4] operators, which expects a relation and the name of a hpoint attribute as parameter. It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt>
int creatextreeHPointRel_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    XTree *xtree = static_cast<XTree*>(result.addr);

    Relation *relation =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string configName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    Tuple *tuple;
    GenericRelationIterator *iter = relation->MakeScan();

    unsigned dim=0;
    while ((tuple = iter->GetNextTuple()))
    {
        HPointAttr *attr = static_cast<HPointAttr*>(
                tuple->GetAttribute(attrIndex));

        if(attr->IsDefined())
        {
            if (xtree->isInitialized())
            {
                if(attr->dim() != dim)
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator creatextree: " << endl
                        << "Got hpoint attributes of variing "
                        << "dimensions!"
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    delete iter;
                    return CANCEL;
                }
            }
            else
            {  // initialize xtree
                dim = attr->dim();
                xtree->initialize(
                        dim, configName, "hpoint", 0,
                        HPointReg::defaultName("hpoint"));
            }

            // insert attribute into xtree
            xtreeAlgebra::LeafEntry *e =
                    new xtreeAlgebra::LeafEntry(
                            tuple->GetTupleId(), attr->hpoint());
            xtree->insert(e);
        }
        tuple->DeleteIfAllowed();
    }
    delete iter;

    #ifdef __XTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.1.1.1 creatextreeHRectRel[_]VM

This value mapping function is used for all "creatextree"[4] operators, which expects a relation and the name of a hrect attribute as parameter. It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt>
int creatextreeHRectRel_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    XTree *xtree = static_cast<XTree*>(result.addr);

    Relation *relation =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string configName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    Tuple *tuple;
    GenericRelationIterator *iter = relation->MakeScan();

    unsigned dim=0;
    while ((tuple = iter->GetNextTuple()))
    {
        HRectAttr *attr = static_cast<HRectAttr*>(
                tuple->GetAttribute(attrIndex));

        if(attr->IsDefined())
        {
            if (xtree->isInitialized())
            {
                if(attr->dim() != dim)
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator creatextree: " << endl
                        << "Got hrect attributes of variing "
                        << "dimensions!"
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    delete iter;
                    return CANCEL;
                }
            }
            else
            {  // initialize xtree
                dim = attr->dim();
                xtree->initialize(
                        dim, configName, "hrect", 0,
                        BBoxReg::defaultName("hrect"));
            }

            // insert attribute into xtree
            xtreeAlgebra::LeafEntry *e =
                    new xtreeAlgebra::LeafEntry(
                            tuple->GetTupleId(), attr->hrect());
            xtree->insert(e);
        }
        tuple->DeleteIfAllowed();
    }
    delete iter;

    #ifdef __XTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.1.1.1 creatextreeRel[_]VM

This value mapping function is used for all "creatextree"[4] operators, which expects a relation and the name of a non hpoint and non hrect attribute as parameter. It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt>
int creatextreeRel_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    XTree *xtree = static_cast<XTree*>(result.addr);

    Relation *relation =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string configName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    string typeName =
        static_cast<CcString*>(args[paramCnt+2].addr)->GetValue();

    int getdataType =
        static_cast<CcInt*>(args[paramCnt+3].addr)->GetIntval();

    string getdataName =
        static_cast<CcString*>(args[paramCnt+4].addr)->GetValue();

    HPointInfo hpoint_info;
    BBoxInfo bbox_info;

    // init hpoint_info or bbox_info
    if (getdataType == 0)
        hpoint_info = HPointReg::getInfo(typeName, getdataName);
    else
        bbox_info = BBoxReg::getInfo(typeName, getdataName);

    Tuple *tuple;
    GenericRelationIterator *iter = relation->MakeScan();

    while ((tuple = iter->GetNextTuple()))
    {
        Attribute *attr = tuple->GetAttribute(attrIndex);
        if(attr->IsDefined())
        {
            xtreeAlgebra::LeafEntry *e;
            if (getdataType == 0)
            {
                e = new xtreeAlgebra::LeafEntry(
                        tuple->GetTupleId(),
                        hpoint_info.getHPoint(attr));
            }
            else
            {
                e = new xtreeAlgebra::LeafEntry(
                        tuple->GetTupleId(),
                        bbox_info.getBBox(attr));
            }

            if (!xtree->isInitialized())
            {  // initialize xtree
                xtree->initialize(
                        e->bbox()->dim(), configName, typeName,
                        getdataType, getdataName);
            }

            xtree->insert(e);
        }
        tuple->DeleteIfAllowed();
    }
    delete iter;

    #ifdef __XTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.1.1.1 creatextreeHPointStream[_]VM

This value mapping function is used for all "creatextree"[4] operators, which expects a tuple stream and the name of a hpoint attribute as parameter. It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt>
int creatextreeHPointStream_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    XTree *xtree = static_cast<XTree*>(result.addr);

    void *stream =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string configName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    Word wTuple;
    qp->Open(stream);
    qp->Request(stream, wTuple);

    unsigned dim=0;
    while (qp->Received(stream))
    {
        Tuple *tuple = static_cast<Tuple*>(wTuple.addr);
        HPointAttr *attr = static_cast<HPointAttr*>(
                tuple->GetAttribute(attrIndex));

        if(attr->IsDefined())
        {
            if (xtree->isInitialized())
            {
                if(attr->dim() != dim)
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator creatextree: " << endl
                        << "Got hpoint attributes of variing "
                        << "dimensions!"
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    qp->Close(stream);
                    return CANCEL;
                }
            }
            else
            {  // initialize xtree
                dim = attr->dim();
                xtree->initialize(
                        dim, configName, "hpoint", 0,
                        HPointReg::defaultName("hpoint"));
            }

            // insert attribute into xtree
            xtreeAlgebra::LeafEntry *e =
                    new xtreeAlgebra::LeafEntry(
                            tuple->GetTupleId(), attr->hpoint());
            xtree->insert(e);
        }
        tuple->DeleteIfAllowed();
        qp->Request(stream, wTuple);
    }
    qp->Close(stream);

    #ifdef __XTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.1.1.1 creatextreeHRectStream[_]VM

This value mapping function is used for all "creatextree"[4] operators, which expects a tuple stream and the name of a hrect attribute as parameter. It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt>
int creatextreeHRectStream_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    XTree *xtree = static_cast<XTree*>(result.addr);

    void *stream =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string configName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    Word wTuple;
    qp->Open(stream);
    qp->Request(stream, wTuple);

    unsigned dim=0;
    while (qp->Received(stream))
    {
        Tuple *tuple = static_cast<Tuple*>(wTuple.addr);
        HRectAttr *attr = static_cast<HRectAttr*>(
                tuple->GetAttribute(attrIndex));

        if(attr->IsDefined())
        {
            if (xtree->isInitialized())
            {
                if(attr->dim() != dim)
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator creatextree: " << endl
                        << "Got hrect attributes of variing "
                        << "dimensions!"
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    qp->Close(stream);
                    return CANCEL;
                }
            }
            else
            {  // initialize xtree
                dim = attr->dim();
                xtree->initialize(
                        dim, configName, "hrect", 0,
                        BBoxReg::defaultName("hrect"));
            }

            // insert attribute into xtree
            xtreeAlgebra::LeafEntry *e =
                    new xtreeAlgebra::LeafEntry(
                            tuple->GetTupleId(), attr->hrect());
            xtree->insert(e);
        }
        tuple->DeleteIfAllowed();
        qp->Request(stream, wTuple);
    }
    qp->Close(stream);

    #ifdef __XTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.1.1.1 creatextreeStream[_]VM

This value mapping function is used for all "creatextree"[4] operators, which expects a tuple stream and the name of a non hpoint and non hrect attribute as parameter. It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt>
int creatextreeStream_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    XTree *xtree = static_cast<XTree*>(result.addr);

    void *stream =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string configName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    string typeName =
        static_cast<CcString*>(args[paramCnt+2].addr)->GetValue();

    int getdataType =
        static_cast<CcInt*>(args[paramCnt+3].addr)->GetIntval();

    string getdataName =
        static_cast<CcString*>(args[paramCnt+4].addr)->GetValue();

    HPointInfo hpoint_info;
    BBoxInfo bbox_info;

    // init hpoint_info or bbox_info
    if (getdataType == 0)
        hpoint_info = HPointReg::getInfo(typeName, getdataName);
    else
        bbox_info = BBoxReg::getInfo(typeName, getdataName);

    Word wTuple;
    qp->Open(stream);
    qp->Request(stream, wTuple);

    while (qp->Received(stream))
    {
        Tuple *tuple = static_cast<Tuple*>(wTuple.addr);
        Attribute *attr = tuple->GetAttribute(attrIndex);

        if(attr->IsDefined())
        {
            xtreeAlgebra::LeafEntry *e;
            if (getdataType == 0)
            {
                e = new xtreeAlgebra::LeafEntry(
                        tuple->GetTupleId(),
                        hpoint_info.getHPoint(attr));
            }
            else
            {
                e = new xtreeAlgebra::LeafEntry(
                        tuple->GetTupleId(),
                        bbox_info.getBBox(attr));
            }

            if (!xtree->isInitialized())
            {  // initialize xtree
                xtree->initialize(
                        e->bbox()->dim(), configName, typeName,
                        getdataType, getdataName);
            }

            xtree->insert(e);
        }
        tuple->DeleteIfAllowed();
        qp->Request(stream, wTuple);
    }
    qp->Close(stream);

    #ifdef __XTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.1.1.1 Search[_]LI

Local Info object for the search value mappings.

********************************************************************/
struct Search_LI
{
  Relation *relation;
  list<TupleId> *results;
  list<TupleId>::iterator iter;
  bool defined;

  Search_LI(Relation *rel)
  : relation(rel),
    results(new list<TupleId>),
    defined(false)
    {}

  void initResultIterator()
  {
    iter = results->begin();
    defined = true;
  }

  ~Search_LI()
  {
    delete results;
  }

  TupleId next()
  {
    if (iter != results->end())
    {
      TupleId tid = *iter;
      *iter++;
      return tid;
    }
    else
    {
      return 0;
    }
  }
};

/********************************************************************
1.1.1.1 rangesearchHPoint[_]VM

********************************************************************/
int rangesearchHPoint_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  Search_LI *li;
  switch (message)
  {
    case OPEN :
    {
      XTree *xtree = static_cast<XTree*>(args[0].addr);

      li = new Search_LI(static_cast<Relation*>(args[1].addr));
      local = SetWord(li);

      HPointAttr *attr =
          static_cast<HPointAttr*>(args[2].addr);

      double searchRad =
            static_cast<CcReal*>(args[3].addr)->GetValue();

      if (attr->dim() != xtree->dim())
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator rangesearch:" << endl
            << "The given hpoint has the dimension "
            << attr->dim() << ", but the xtree contains "
            << xtree->dim() << "-dimensional data!"
            << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      // compute square of searchRad, since the current
      // implementation of the xtree uses the square of the eucledean
      // metric as distance function
      searchRad *= searchRad;

      xtree->rangeSearch(attr->hpoint(), searchRad, li->results);
      li->initResultIterator();

      assert(li->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      li = (Search_LI*)local.addr;
      if(!li->defined)
        return CANCEL;

      TupleId tid = li->next();
      if(tid)
      {
        Tuple *tuple = li->relation->GetTuple(tid, false);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      li = (Search_LI*)local.addr;
      delete li;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.1.1.1 rangesearch[_]VM

********************************************************************/
int rangesearch_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  Search_LI *li;
  switch (message)
  {
    case OPEN :
    {
      XTree *xtree = static_cast<XTree*>(args[0].addr);

      li = new Search_LI(static_cast<Relation*>(args[1].addr));
      local = SetWord(li);

      Attribute *attr = static_cast<Attribute*>(args[2].addr);

      double searchRad =
            static_cast<CcReal*>(args[3].addr)->GetValue();

      string typeName =
           static_cast<CcString*>(args[4].addr)->GetValue();

      // compute square of searchRad, since the current
      // implementation of the xtree uses the square of the eucledean
      // metric as distance function
      searchRad *= searchRad;

      if (xtree->typeName() != typeName)
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator rangesearch:" << endl
            << "Got an \"" << typeName << "\" attribute, but the "
            << "xtree contains \"" << xtree->typeName()
            << "\" attriubtes!" << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      HPoint *p;
      if (xtree->getdataType() == 0)
      { // use gethpoint function
        HPointInfo info = HPointReg::getInfo(
                typeName, xtree->getdataName());
        p = info.getHPoint(attr);
      }
      else
      { // use getbbox function
        BBoxInfo info = BBoxReg::getInfo(
                typeName, xtree->getdataName());
        HRect *r = info.getBBox(attr);
        p = new HPoint(r->center());
        delete r;
      }

      xtree->rangeSearch(p, searchRad, li->results);
      li->initResultIterator();

      assert(li->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      li = (Search_LI*)local.addr;
      if(!li->defined)
        return CANCEL;

      TupleId tid = li->next();
      if(tid)
      {
        Tuple *tuple = li->relation->GetTuple(tid, false);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      li = (Search_LI*)local.addr;
      delete li;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.1.1.1 nnsearchHPoint[_]VM

********************************************************************/
int nnsearchHPoint_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  Search_LI *li;

  switch (message)
  {
    case OPEN :
    {
      XTree *xtree = static_cast<XTree*>(args[0].addr);

      li = new Search_LI(static_cast<Relation*>(args[1].addr));
      local = SetWord(li);

      HPointAttr *attr = static_cast<HPointAttr*>(args[2].addr);
      int nnCount = static_cast<CcInt*>(args[3].addr)->GetValue();

      if (attr->dim() != xtree->dim())
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator nnsearch:" << endl
            << "The given hpoint has the dimension "
            << attr->dim() << ", but the xtree contains "
            << xtree->dim() << "-dimensional data!"
            << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      xtree->nnSearch(attr->hpoint(), nnCount , li->results);
      li->initResultIterator();

      assert(li->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      li = (Search_LI*)local.addr;
      if(!li->defined)
        return CANCEL;

      TupleId tid = li->next();
      if(tid)
      {
        Tuple *tuple = li->relation->GetTuple(tid, false);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      li = (Search_LI*)local.addr;
      delete li;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.1.1.1 nnsearch[_]VM

********************************************************************/
int nnsearch_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  Search_LI *li;

  switch (message)
  {
    case OPEN :
    {
      XTree *xtree = static_cast<XTree*>(args[0].addr);

      li = new Search_LI(static_cast<Relation*>(args[1].addr));
      local = SetWord(li);

      Attribute *attr = static_cast<Attribute*>(args[2].addr);
      int nnCount = static_cast<CcInt*>(args[3].addr)->GetValue();

      string typeName =
           static_cast<CcString*>(args[4].addr)->GetValue();

      if (xtree->typeName() != typeName)
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator nnsearch:" << endl
            << "Got an \"" << typeName << "\" attribute, but the "
            << "xtree contains \"" << xtree->typeName()
            << "\" attriubtes!" << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      HPoint *p;
      if (xtree->getdataType() == 0)
      { // use gethpoint function
        HPointInfo info = HPointReg::getInfo(
                typeName, xtree->getdataName());
        p = info.getHPoint(attr);
      }
      else
      { // use getbbox function
        BBoxInfo info = BBoxReg::getInfo(
                typeName, xtree->getdataName());
        HRect *r = info.getBBox(attr);
        p = new HPoint(r->center());
        delete r;
      }

      xtree->nnSearch(p, nnCount , li->results);
      li->initResultIterator();

      assert(li->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      li = (Search_LI*)local.addr;
      if(!li->defined)
        return CANCEL;

      TupleId tid = li->next();
      if(tid)
      {
        Tuple *tuple = li->relation->GetTuple(tid, false);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      li = (Search_LI*)local.addr;
      delete li;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.1.1.1 nnscan[_]LI

Local Info object for the nnscan value mapping.

********************************************************************/
struct nnscan_LI
{
  XTree *xtree;
  Relation *relation;
};

/********************************************************************
1.1.1.1 nnscanHPoint[_]VM

********************************************************************/
int nnscanHPoint_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  nnscan_LI *li;
  li = static_cast<nnscan_LI*>(local.addr);

  switch (message)
  {
    case OPEN :
    { if(li){
        delete li;
      }
      li = new nnscan_LI();
      li->xtree = static_cast<XTree*>(args[0].addr);
      li->relation = static_cast<Relation*>(args[1].addr);
      local = SetWord(li);

      HPointAttr *attr = static_cast<HPointAttr*>(args[2].addr);
      string typeName = static_cast<CcString*>(args[3].addr)->GetValue();

      if (attr->dim() != li->xtree->dim())
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator nnscan:" << endl
            << "The given hpoint has the dimension "
            << attr->dim() << ", but the xtree contains "
            << li->xtree->dim() << "-dimensional data!"
            << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      li->xtree->nnscan_init(attr->hpoint());

      assert(li->relation != 0);
      return 0;
    }

    case REQUEST :
    { if(!li){
         return CANCEL;
      }
      TupleId tid = li->xtree->nnscan_next();
      if(tid)
      {
        Tuple *tuple = li->relation->GetTuple(tid, false);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      li->xtree->nnscan_cleanup();
      delete li;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.1.1.1 nnscan[_]VM

********************************************************************/
int nnscan_VM(
        Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  nnscan_LI *li;

  switch (message)
  {
    case OPEN :
    {
      li = new nnscan_LI();
      li->xtree = static_cast<XTree*>(args[0].addr);
      li->relation = static_cast<Relation*>(args[1].addr);
      local = SetWord(li);

      Attribute *attr = static_cast<Attribute*>(args[2].addr);
      string typeName = static_cast<CcString*>(args[3].addr)->GetValue();

      if (li->xtree->typeName() != typeName)
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator nnscan:" << endl
            << "Got an \"" << typeName << "\" attribute, but the "
            << "xtree contains \"" << li->xtree->typeName()
            << "\" attriubtes!" << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      HPoint *p;
      if (li->xtree->getdataType() == 0)
      { // use gethpoint function
        HPointInfo info = HPointReg::getInfo(
                typeName, li->xtree->getdataName());
        p = info.getHPoint(attr);
      }
      else
      { // use getbbox function
        BBoxInfo info = BBoxReg::getInfo(
                typeName, li->xtree->getdataName());
        HRect *r = info.getBBox(attr);
        p = new HPoint(r->center());
        delete r;
      }

      li->xtree->nnscan_init(p);
      return 0;
    }

    case REQUEST :
    {
      li = static_cast<nnscan_LI*>(local.addr);
      TupleId tid = li->xtree->nnscan_next();
      if(tid)
      {
        Tuple *tuple = li->relation->GetTuple(tid, false);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      li = static_cast<nnscan_LI*>(local.addr);
      li->xtree->nnscan_cleanup();
      delete li;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}


/********************************************************************
1.1.1.1 windowintersects[_]VM

********************************************************************/
int windowintersects_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  Search_LI *li;

  switch (message)
  {
    case OPEN :
    {
      XTree *xtree = static_cast<XTree*>(args[0].addr);

      li = new Search_LI(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(li);

      HRectAttr *attr =
          static_cast<HRectAttr*>(args[2].addr);

      if (attr->dim() != xtree->dim())
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator windowintersects:" << endl
            << "The given hrect has the dimension "
            << attr->dim() << ", but the xtree contains "
            << xtree->dim() << "-dimensional data!"
            << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      xtree->windowIntersects(attr->hrect(), li->results);
      li->initResultIterator();

      assert(li->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      li = (Search_LI*)local.addr;
      if(!li->defined)
        return CANCEL;

      TupleId tid = li->next();
      if(tid)
      {
        Tuple *tuple = li->relation->GetTuple(tid, false);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    {
      li = (Search_LI*)local.addr;
      delete li;
      local.addr = 0;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.1.1 Type mappings

1.1.1.1 creatextree[_]TM

********************************************************************/
template<unsigned paramCnt>
ListExpr creatextree_TM(ListExpr args)
{
    NList args_NL(args);

    if(!args_NL.hasLength(paramCnt)){
      stringstream err;
      err << "Expected " << paramCnt << " arguments!";
      return listutils::typeError(err.str());
    }

    NList attrs;
    NList relStream_NL = args_NL.first();
    NList attr_NL = args_NL.second();

    if(!relStream_NL.checkRel(attrs) && !relStream_NL.checkStreamTuple(attrs)){
      return listutils::typeError(
      "Argument 1 must be a relation or tuple stream!");
    }

    // check, if the specified attr. can be found in attribute list
    if(!attr_NL.isSymbol()){
      return listutils::typeError(
      "Argument 2 must be a symbol or an atomar type!");
    }

    string attrName = attr_NL.str();
    string typeName;
    int attrIndex;

    ListExpr attrTypeLE;
    attrIndex = FindAttribute(attrs.listExpr(), attrName, attrTypeLE);
    if(attrIndex <= 0){
      stringstream err;
      err << "Attribute name \"" << attrName << "\" is not known.\n"
      << "Known Attribute(s):\n" << attrs.convertToString();
      return listutils::typeError(err.str());
    }
    --attrIndex;
    NList attrType (attrTypeLE);
    typeName = attrType.str();

    // get config name
    string configName;
    if (paramCnt > 2){
      if(!args_NL.third().isSymbol()){
        listutils::typeError(
        "Argument 3 must be the name of an existing config object!");
      }
      configName = args_NL.third().str();
    }
    else
        configName = CONFIG_DEFAULT;

    if (!XTreeConfigReg::isDefined(configName))
    {
        string errmsg;
        errmsg = "Config \"" + configName +
                 "\" not defined, defined names:\n\n" +
                 XTreeConfigReg::definedNames();
        return listutils::typeError(errmsg);
    }

    if ((typeName == "hpoint") || (typeName == "hrect"))
    {
        NList res1(Symbol::APPEND());
        NList res2;
        res2.append(NList(attrIndex));
        res2.append(NList(configName, true));
        NList res3(XTree::BasicType());
        NList result(res1, res2, res3);
        return result.listExpr();
    }

    // attribut is no hpoint and no hrect, try to find getpoint or
    // getbbox function:

    // get name of getdata-function
    string getdataName;
    int getdataType; // 0 = gethpoint, 1 = gethrect
    if (paramCnt > 3) // creatextree3
    {
        NList getdataName_NL = args_NL.fourth();
        string errmsg;;
        errmsg = "Argument 4 must be the name of an existing "
                 "gethrect or getdata function!";
        if(!getdataName_NL.isSymbol()){
          return listutils::typeError(errmsg);
        }
        getdataName = getdataName_NL.str();
        getdataType = 0;
        if (!HPointReg::isDefined(typeName, getdataName))
        { // getdataName is no defined gethpoint function
          // search in HRectReg
            getdataType = 1;
            string errmsg;
            errmsg = "No gethpoint or getbbox function with name \""
                     + getdataName + "\" for type constructor \"" +
                     typeName + "\" found! Possible names:"
                     "\ngethpoint: " +
                     HPointReg::definedNames(typeName) +
                     "\ngetbbox  : " +
                     BBoxReg::definedNames(typeName);
            if(!BBoxReg::isDefined(typeName, getdataName)){
              return listutils::typeError(errmsg);
            }
        }
    }
    else // creatextree, creatextree2
    {   // try default gethpoint function
        getdataName = HPointReg::defaultName(typeName);
        getdataType = 0;
        if (getdataName == HPOINT_UNDEFINED)
        { // no default gethpoint function found
          // try default getbbox function
            getdataName = BBoxReg::defaultName(typeName);
            getdataType = 1;
            string errmsg;
            errmsg = "No default gethpoint or getbbox function "
                     "defined for type constructor \"" +
                     typeName + "\"!";
            if(getdataName == BBOX_UNDEFINED){
              return listutils::typeError(errmsg);
            }
        }
    }

    NList res1(Symbol::APPEND());
    NList res2;
    res2.append(NList(attrIndex));
    res2.append(NList(configName, true));
    res2.append(NList(typeName, true));
    res2.append(NList(getdataType));
    res2.append(NList(getdataName, true));
    NList res3(XTree::BasicType());
    NList result(res1, res2, res3);
    return result.listExpr();


}

/********************************************************************
1.1.1.1 rangesearch[_]TM

********************************************************************/
ListExpr rangesearch_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    NList args_NL(args);

    if(!args_NL.hasLength(4)){
      return listutils::typeError("Expected 4 arguments!");
    }

    NList attrs;
    NList mtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList data_NL = args_NL.third();
    NList searchRad_NL = args_NL.fourth();

    if(!mtree_NL.isEqual(XTree::BasicType())){
      return listutils::typeError("First argument must be a xtree!");
    }

    if(!rel_NL.checkRel(attrs)){
      return listutils::typeError("Argument 2 must be a relation!");
    }

    if(!data_NL.isSymbol()){
      return listutils::typeError(
      "Argument 3 must be a symbol or an atomar type!");
    }

    if(!searchRad_NL.isEqual(CcReal::BasicType())){
      return listutils::typeError("Argument 4 must be an \"real\" value!");
    }

    /* further type checkings for the data parameter will be done
       in the value mapping function, since that needs some data from
       the xtree object */

    NList append(Symbol::APPEND());
    NList result (
        append,
        NList(data_NL.str(), true).enclose(),
        NList(NList(Symbol::STREAM()), rel_NL.second()));

    return result.listExpr();
}

/********************************************************************
1.1.1.1 nnsearch[_]TM

********************************************************************/
ListExpr nnsearch_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    NList args_NL(args);

    if(!args_NL.hasLength(4)){
      return listutils::typeError("Expected 4 arguments!");
    }

    NList attrs;
    NList mtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList data_NL = args_NL.third();
    NList nnCount_NL = args_NL.fourth();

    if(!mtree_NL.isEqual(XTree::BasicType())){
      return listutils::typeError("First argument must be a xtree!");
    }

    if(!rel_NL.checkRel(attrs)){
      return listutils::typeError("Argument 2 must be a relation!");
    }

    if(!data_NL.isSymbol()){
      return listutils::typeError(
      "Argument 3 must be a symbol or an atomar type!");
    }

    if(!nnCount_NL.isEqual(CcInt::BasicType())){
      return listutils::typeError("Argument 4 must be an \"int\" value!");
    }

    /* further type checkings for the data parameter will be done
       in the value mapping function, since that needs some data from
       the xtree object */

    NList append(Symbol::APPEND());
    NList result (
        append,
        NList(data_NL.str(), true).enclose(),
        NList(NList(Symbol::STREAM()), rel_NL.second()));

    return result.listExpr();
}

/********************************************************************
1.1.1.1 windowintersects[_]TM

********************************************************************/
ListExpr windowintersects_TM(ListExpr args)
{
    NList args_NL(args);

    if(!args_NL.hasLength(3)){
      return listutils::typeError("Expected 3 arguments!");
    }

    NList attrs;
    NList xtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList hrect_NL = args_NL.third();

    if(!xtree_NL.isEqual(XTree::BasicType())){
      return listutils::typeError("First argument must be a xtree!");
    }

    if(!rel_NL.checkRel(attrs)){
      return listutils::typeError("Argument 2 must be a relation!");
    }

    if(!hrect_NL.isSymbol()){
      return listutils::typeError(
      "Argument 3 must be a symbol or an atomar type!");
    }

    string typeName = hrect_NL.str();
    string error = "Expecting a \"hrect\" as third parameter!";
    if(typeName != "hrect"){
      return listutils::typeError(error);
    }

    NList stream_NL(Symbol::STREAM());
    NList result(stream_NL, rel_NL.second());
    return result.listExpr();
}

/********************************************************************
1.1.1.1 nnscan[_]TM

********************************************************************/
ListExpr nnscan_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    NList args_NL(args);

    if(!args_NL.hasLength(3)){
      return listutils::typeError("Expected 3 arguments!");
    }

    NList attrs;
    NList mtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList data_NL = args_NL.third();

    if(!mtree_NL.isEqual(XTree::BasicType())){
      return listutils::typeError("First argument must be a xtree!");
    }

    if(!rel_NL.checkRel(attrs)){
      return listutils::typeError("Argument 2 must be a relation!");
    }

    if(!data_NL.isSymbol()){
      return listutils::typeError(
      "Argument 3 must be a symbol or an atomar type!");
    }

    /* further type checkings for the data parameter will be done
       in the value mapping function, since that needs some data from
       the xtree object */

    NList append(Symbol::APPEND());
    NList result (
        append,
        NList(data_NL.str(), true).enclose(),
        NList(NList(Symbol::STREAM()), rel_NL.second()));

    return result.listExpr();
}

/********************************************************************
1.1.1 Selection functions

********************************************************************/
int creatextree_Select(ListExpr args)
{
    NList argsNL(args);
    NList arg1 = argsNL.first();
    NList attrs = arg1.second().second();
    NList arg2 = argsNL.second();

    // get type of selected attribute
    string attrName = arg2.str();
    ListExpr attrTypeLE;
    FindAttribute(attrs.listExpr(), attrName, attrTypeLE);
    NList attrType(attrTypeLE);

    if (arg1.first().isEqual(Relation::BasicType()))
    { // relation
        if (attrType.isEqual("hpoint"))
            return 0;
        else if (attrType.isEqual("hrect"))
            return 1;
        else
            return 2;
    }
    else
    { // stream
        if (attrType.isEqual("hpoint"))
            return 3;
        else if (attrType.isEqual("hrect"))
            return 4;
        else
            return 5;
    }
}

int creatextree3_Select(ListExpr args)
{
    NList argsNL(args);
    NList arg1 = argsNL.first();
    if (arg1.first().isEqual(Relation::BasicType()))
        return 0;
    else
        return 1;
}

int search_Select(ListExpr args)
{
    NList argsNL(args);
    NList arg3 = argsNL.third();
    if (arg3.isEqual("hpoint"))
        return 0;
    else
        return 1;
}

/********************************************************************
1.1.1 Value mapping arrays

********************************************************************/
ValueMapping creatextree_Map[] = {
    creatextreeHPointRel_VM<2>,
    creatextreeHRectRel_VM<2>,
    creatextreeRel_VM<2>,
    creatextreeHPointStream_VM<2>,
    creatextreeHRectStream_VM<2>,
    creatextreeStream_VM<2>,
};

ValueMapping creatextree2_Map[] = {
    creatextreeHPointRel_VM<3>,
    creatextreeHRectRel_VM<3>,
    creatextreeRel_VM<3>,
    creatextreeHPointStream_VM<3>,
    creatextreeHRectStream_VM<3>,
    creatextreeStream_VM<3>,
};

ValueMapping creatextree3_Map[] = {
    creatextreeRel_VM<4>,
    creatextreeStream_VM<4>,
};

ValueMapping rangesearch_Map[] = {
    rangesearchHPoint_VM,
    rangesearch_VM,
};

ValueMapping nnsearch_Map[] = {
    nnsearchHPoint_VM,
    nnsearch_VM,
};

ValueMapping nnscan_Map[] = {
    nnscanHPoint_VM,
    nnscan_VM,
};

/********************************************************************
1.1.1 Operator infos

********************************************************************/
struct creatextree_Info : OperatorInfo
{
    creatextree_Info()
    {
        name = "creatextree";
        signature =
        "rel/tuple stream x attribute -> xtree";
        syntax = "_ creatextree [_]";
        meaning =
            "Creates a new xtree from the relation or tuple stream "
            "in argument 1. Argument 2 must be the name of the "
            "attribute in the relation/tuple stream, that should "
            "be indexed in the xtree. "
            "This operator uses the default xtree config.";
        example = "strassen creatextree[geoData]";
    }
};

struct creatextree2_Info : OperatorInfo
{
    creatextree2_Info()
    {
        name = "creatextree2";
        signature =
        "rel/tuple stream x attribute x config-name -> xtree";
        syntax = "_ creatextree2 [_, _]";
        meaning =
            "Like creatextree, but additionaly allows to specify "
            "another than the default xtree config.";
        example = "strassen creatextree2[geoData, limit80e]";
    }
};

struct creatextree3_Info : OperatorInfo
{
    creatextree3_Info()
    {
        name = "creatextree3";
        signature =
        "rel/tuple stream x attribute x config-name x getdatafun-name -> xtree";
        syntax = "_ creatextree3 [_, _, _]";
        meaning =
        "Like creatextree2, but additionaly allows to specify "
        "another than the default gethpoint or getbbox function "
        "(if a gethpoint and a getbbox function with the same name "
       "are defined, the gethpoint function will be used).";
       example = "strassen creatextree3[geoData, limit80e, native]";
    }
};

struct rangesearch_Info : OperatorInfo
{
    rangesearch_Info()
    {
        name = "rangesearch";
        signature = "xtree x relation x hpoint x real -> tuple stream";
        syntax = "_ _ rangesearch [_, _]";
        meaning =
            "Returns a tuple stream, which contains all attributes, "
            "that could be found within the search radius "
            "(argument 4, using the Euclidean metric) to the query "
            "point. The relation must contain at least the same "
            "tuples, that had been used to create the xtree.";
        example = "xt strassen rangesearch [p, 5000.0]";
    }
};

struct nnsearch_Info : OperatorInfo
{
    nnsearch_Info()
    {
        name = "nnsearch";
        signature = "xtree x relation x hpoint x int -> tuple stream";
        syntax = "_ _ nnsearch [_, _]";
        meaning =
            "Returns a tuple stream, which contains the k nearest "
            "neighbours of the query point. The relation must "
            "contain at least the same tuples, that had been used "
            "to create the xtree.";
        example = "xt strassen nnsearch [p, 20]";
    }
};

struct windowintersects_Info : OperatorInfo
{
    windowintersects_Info()
    {
        name = "windowintersects";
        signature = "xtree x relation x hrect -> tuple stream";
        syntax = "_ _ rangesearch [_]";
        meaning =
            "Returns a tuple stream, which contains all attributes, "
            "that intersects the search windows. The relation must "
            "contain at least the same tuples, that had been used "
            "to create the xtree.";
        example = "xt strassen windowintersects [r]";
    }
};

struct nnscan_Info : OperatorInfo
{
    nnscan_Info()
    {
        name = "nnscan";
        signature = "xtree x relation x fun x hpoint x int -> tuple stream";
        syntax = "_ _ nnscan [_]";
        meaning =
            "Returns a tuple stream, which contains the ranking of the indized"
            "elements, based on the distance to the query point. The relation "
            "must contain at least the same tuples, that had been used "
            "to create the xtree.";
        example = "xt strassen nnscan [p] head[30] count";
    }
};

/********************************************************************
1.1 Create and initialize the Algebra

********************************************************************/
class XTreeAlgebra
    : public Algebra
{

public:
    XTreeAlgebra()
        : Algebra()
    {
        AddTypeConstructor(&xtreeTC);

        AddOperator(creatextree_Info(),
                    creatextree_Map,
                    creatextree_Select,
                    creatextree_TM<2>);

        AddOperator(creatextree2_Info(),
                    creatextree2_Map,
                    creatextree_Select,
                    creatextree_TM<3>);

        AddOperator(creatextree3_Info(),
                    creatextree3_Map,
                    creatextree3_Select,
                    creatextree_TM<4>);

        AddOperator(rangesearch_Info(),
                    rangesearch_Map,
                    search_Select,
                    rangesearch_TM);

        AddOperator(nnsearch_Info(),
                    nnsearch_Map,
                    search_Select,
                    nnsearch_TM);

        AddOperator(windowintersects_Info(),
                    windowintersects_VM,
                    windowintersects_TM);

        AddOperator(nnscan_Info(),
                    nnscan_Map,
                    search_Select,
                    nnscan_TM);
    }

    ~XTreeAlgebra()
    {};
};

} // namespace xtreeAlgebra


extern "C"
Algebra *InitializeXTreeAlgebra(
    NestedList *nlRef, QueryProcessor *qpRef)
{
    nl = nlRef;
    qp = qpRef;
    return (new xtreeAlgebra::XTreeAlgebra());
}
