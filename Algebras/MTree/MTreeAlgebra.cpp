/*
\newpage

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

1 Implementation file "MTreeAlgebra.cpp"[4]

January-March 2008, Mirko Dibbert

1.1 Overview

This file contains the implementation of the mtree algebra.

1.1 Includes and defines

*/
#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "MTreeAlgebra.h"
#include "MTree.h"

extern NestedList* nl;
extern QueryProcessor* qp;
extern AlgebraManager* am;

namespace mtreeAlgebra {

/********************************************************************
1.1 Type constructor "MTREE"[4]

********************************************************************/
static ListExpr
MTreeProp()
{
    ListExpr examplelist = nl->TextAtom();
    nl->AppendText(examplelist, "<relation> createmtree [<attrname>]"
                                     " where <attrname> is the key");

    return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
                    nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
                    nl->StringAtom("(let mymtree = images "
                                    "createmtree[pic] "))));
}

ListExpr
OutMTree(ListExpr type_Info, Word w)
{
    MTree* mtree = static_cast<MTree*>(w.addr);
    if (mtree->isInitialized())
    {
        NList assignments(
            NList("assignments:"),
            NList(
                NList(NList("type constructor:"),
                    NList(mtree->typeName(), true)),
                NList(NList("metric:"),
                    NList(mtree->distfunName(), true)),
                NList(NList("distdata type:"),
                    NList(mtree->dataName(), true)),
                NList(NList("mtree-config:"),
                    NList(mtree->configName(), true))));

        NList statistics(
            NList("statistics:"),
            NList(
                NList(NList("height:"),
                    NList((int)mtree->height())),
                NList(NList("# of internal nodes:"),
                    NList((int)mtree->internalCount())),
                NList(NList("# of leaf nodes:"),
                    NList((int)mtree->leafCount())),
                NList(NList("# of leaf entries:"),
                    NList((int)mtree->entryCount()))));

        NList result(assignments, statistics);
        return result.listExpr();
    }
    else
        return nl->SymbolAtom( "undef" );
}

Word
InMTree(ListExpr type_Info, ListExpr value,
        int errorPos, ListExpr &error_Info, bool &correct)
{
    correct = false;
    return SetWord(0);
}

Word
Createmtree(const ListExpr type_Info)
{ return SetWord(new MTree()); }

void
DeleteMTree(const ListExpr type_Info, Word& w)
{
    static_cast<MTree*>(w.addr)->deleteFile();
    delete static_cast<MTree*>(w.addr);
    w.addr = 0;
}

bool
OpenMTree(SmiRecord &valueRecord, size_t &offset,
          const ListExpr type_Info, Word& w)
{
    SmiFileId fileid;
    valueRecord.Read(&fileid, sizeof(SmiFileId), offset);
    offset += sizeof(SmiFileId);

    MTree* mtree = new MTree(fileid);
    w = SetWord(mtree);
    return true;
}

bool
SaveMTree(SmiRecord &valueRecord, size_t &offset,
          const ListExpr type_Info, Word& w)
{
    SmiFileId fileId;
    MTree *mtree = static_cast<MTree*>(w.addr);
    fileId = mtree->fileId();
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

void
CloseMTree(const ListExpr type_Info, Word& w)
{
    MTree *mtree = (MTree*)w.addr;
    delete mtree;
}

Word
CloneMTree(const ListExpr type_Info, const Word& w)
{
    MTree* src = static_cast<MTree*>(w.addr);
    MTree* cpy = new MTree(*src);
    return SetWord(cpy);
}

int
SizeOfMTree()
{ return sizeof(MTree); }

bool
CheckMTree(ListExpr typeName, ListExpr &error_Info)
{ return nl->IsEqual(typeName, MTREE); }

TypeConstructor
mtreeTC(MTREE,       MTreeProp,
        OutMTree,    InMTree,
        0, 0,
        Createmtree, DeleteMTree,
        OpenMTree,   SaveMTree,
        CloseMTree,  CloneMTree,
        0,
        SizeOfMTree,
        CheckMTree);

/********************************************************************
1.1 Operators

1.3.1 Value mappings

1.3.1.1 createmtreeRel[_]VM

This value mapping function is used for all "createmtree"[4] operators, which expect a relation and non-distdata attributes.

It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt> int
        createmtreeRel_VM(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
    result = qp->ResultStorage(s);

    MTree* mtree =
        static_cast<MTree*>(result.addr);

    Relation* relation =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string typeName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    string distfunName =
        static_cast<CcString*>(args[paramCnt+2].addr)->GetValue();

    string dataName =
        static_cast<CcString*>(args[paramCnt+3].addr)->GetValue();

    string configName =
        static_cast<CcString*>(args[paramCnt+4].addr)->GetValue();

    DistDataId id = DistDataReg::getDataId(typeName, dataName);
    mtree->initialize(id, distfunName, configName);

    Tuple* tuple;
    GenericRelationIterator* iter = relation->MakeScan();
    while ((tuple = iter->GetNextTuple()) != 0)
    {
        Attribute* attr = tuple->GetAttribute(attrIndex);
        if(attr->IsDefined())
        {
            mtree->insert(attr, tuple->GetTupleId());
        }
        tuple->DeleteIfAllowed();
    }
    delete iter;

    #ifdef MTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.3.1.2 createmtreeStream[_]VM

This value mapping function is used for all "createmtree"[4] operators, which expect a tupel stream and non-distdata attributes.

It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt> int
createmtreeStream_VM(Word* args, Word& result, int message,
                     Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    MTree* mtree = static_cast<MTree*>(result.addr);

    void* stream =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string typeName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    string distfunName =
        static_cast<CcString*>(args[paramCnt+2].addr)->GetValue();

    string dataName =
        static_cast<CcString*>(args[paramCnt+3].addr)->GetValue();

    string configName =
        static_cast<CcString*>(args[paramCnt+4].addr)->GetValue();

    DistDataId id = DistDataReg::getDataId(typeName, dataName);
    mtree->initialize(id, distfunName, configName);

    Word wTuple;
    qp->Open(stream);
    qp->Request(stream, wTuple);
    while (qp->Received(stream))
    {
        Tuple* tuple = static_cast<Tuple*>(wTuple.addr);
        Attribute* attr = tuple->GetAttribute(attrIndex);
        if(attr->IsDefined())
        {
            mtree->insert(attr, tuple->GetTupleId());
        }
        tuple->DeleteIfAllowed();
        qp->Request(stream, wTuple);
    }
    qp->Close(stream);

    #ifdef MTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.3.1.3 createmtreeDDRel[_]VM

This value mapping function is used for all "createmtree"[4] operators, which expect a relation and distdata attributes.

It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt> int
createmtreeDDRel_VM(Word* args, Word& result, int message,
                    Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    MTree* mtree = static_cast<MTree*>(result.addr);

    Relation* relation =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string distfunName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    string configName =
        static_cast<CcString*>(args[paramCnt+2].addr)->GetValue();

    Tuple* tuple;
    GenericRelationIterator* iter = relation->MakeScan();

    DistDataId id;
    while ((tuple = iter->GetNextTuple()))
    {
        DistDataAttribute* attr = static_cast<DistDataAttribute*>(
                tuple->GetAttribute(attrIndex));

        if(attr->IsDefined())
        {
            if (mtree->isInitialized())
            {
                if(attr->distdataId() != id)
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator createmtree: " << endl
                        << "Got distdata attributes of different "
                        << "types!" << endl << "(type constructor "
                        << "or distdata type are not equal)"
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    delete iter;
                    return CANCEL;
                }
            }
            else
            {  // initialize mtree
                id = attr->distdataId();
                DistDataInfo info = DistDataReg::getInfo(id);
                string dataName = info.name();
                string typeName = info.typeName();

                if (distfunName == DFUN_DEFAULT)
                {
                    distfunName = DistfunReg::defaultName(typeName);
                }

                // check if distance function is defined
                if (!DistfunReg::isDefined(distfunName, id))
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator createmtree: " << endl
                        << "Distance function \"" << distfunName
                        << "\" for type \"" << typeName
                        << "\" is not defined!" << endl
                        << "Defined distance functions: " << endl
                        << endl
                        << DistfunReg::definedNames(typeName)
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    delete iter;
                    return CANCEL;
                }

                if (!DistfunReg::getInfo(
                        distfunName, typeName, dataName).isMetric())
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator createmtree: " << endl
                        << "Distance function \"" << distfunName
                        << "\" with \"" << dataName
                        << "\" data for type" << endl
                        << "\"" << typeName << "\" is no metric!"
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    delete iter;
                    return CANCEL;
                }

                mtree->initialize(
                        attr->distdataId(), distfunName, configName);
            }

            // insert attribute into mtree
            DistData* data =
                    new DistData(attr->size(), attr->value());
            mtree->insert(data, tuple->GetTupleId());
        }
        tuple->DeleteIfAllowed();
    }
    delete iter;

    #ifdef MTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.3.1.4 createmtreeDDStream[_]VM

This value mapping function is used for all "createmtree"[4] operators, which expect a tupel stream and distdata attributes.

It is designed as template function, which expects the count of arguments as template paremeter.


********************************************************************/
template<unsigned paramCnt> int
createmtreeDDStream_VM(Word* args, Word& result, int message,
                       Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    MTree* mtree = static_cast<MTree*>(result.addr);

    void* stream =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string distfunName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    string configName =
        static_cast<CcString*>(args[paramCnt+2].addr)->GetValue();

    DistDataId id;
    Word wTuple;
    qp->Open(stream);
    qp->Request(stream, wTuple);
    while (qp->Received(stream))
    {
        Tuple* tuple = static_cast<Tuple*>(wTuple.addr);
        DistDataAttribute* attr = static_cast<DistDataAttribute*>(
                                     tuple->GetAttribute(attrIndex));
        if(attr->IsDefined())
        {
            if (mtree->isInitialized())
            {
                if(attr->distdataId() != id)
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator createmtree: " << endl
                        << "Got distdata attributes of different "
                        << "types!" << endl << "(type constructor "
                        << "or distdata type are not equal)"
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    return CANCEL;
                }
            }
            else
            {  // initialize mtree
                id = attr->distdataId();
                DistDataInfo info = DistDataReg::getInfo(id);
                string dataName = info.name();
                string typeName = info.typeName();

                if (distfunName == DFUN_DEFAULT)
                {
                    distfunName = DistfunReg::defaultName(typeName);
                }

                // check if distance function is defined
                if (!DistfunReg::isDefined(distfunName, id))
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator createmtree: " << endl
                        << "Distance function \"" << distfunName
                        << "\" for type \"" << typeName
                        << "\" is not defined!" << endl
                        << "Defined distance functions: " << endl
                        << endl
                        << DistfunReg::definedNames(typeName)
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    return CANCEL;
                }

                if (!DistfunReg::getInfo(
                        distfunName, typeName, dataName).isMetric())
                {
                    const string seperator =
                            "\n" + string(70, '-') + "\n";
                    cmsg.error()
                        << seperator
                        << "Operator createmtree: " << endl
                        << "Distance function \"" << distfunName
                        << "\" with \"" << dataName
                        << "\" data for type" << endl
                        << "\"" << typeName << "\" is no metric!"
                        << seperator << endl;
                    cmsg.send();
                    tuple->DeleteIfAllowed();
                    return CANCEL;
                }

                mtree->initialize(
                        attr->distdataId(), distfunName, configName);
            }

            // insert attribute into mtree
            DistData* data =
                    new DistData(attr->size(), attr->value());
            mtree->insert(data, tuple->GetTupleId());
        }
        tuple->DeleteIfAllowed();
        qp->Request(stream, wTuple);
    }

    qp->Close(stream);

    #ifdef MTREE_PRINT_INSERT_INFO
    cout << endl;
    #endif

    return 0;
}

/********************************************************************
1.3.1.5 rangesearchLocalInfo

********************************************************************/
struct rangesearchLocalInfo
{
  Relation* relation;
  list<TupleId>* results;
  list<TupleId>::iterator iter;
  bool defined;

  rangesearchLocalInfo(Relation* rel) :
    relation(rel),
    results(new list<TupleId>),
    defined(false)
    {}

  void initResultIterator()
  {
    iter = results->begin();
    defined = true;
  }

  ~rangesearchLocalInfo()
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
1.3.1.6 rangesearch[_]VM

********************************************************************/
int
rangesearch_VM(Word* args, Word& result, int message,
               Word& local, Supplier s)
{
  rangesearchLocalInfo* info;

  switch (message)
  {
    case OPEN :
    {
      MTree* mtree =
          static_cast<MTree*>(args[0].addr);

      info = new rangesearchLocalInfo(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(info);

      Attribute* attr =
          static_cast<Attribute*>(args[2].addr);

      double searchRad =
            static_cast<CcReal*>(args[3].addr)->GetValue();

      string typeName =
          static_cast<CcString*>(args[4].addr)->GetValue();

      if (mtree->typeName() != typeName)
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator rangesearch:" << endl
            << "Got an \"" << typeName << "\" attribute, but the "
            << "mtree contains \"" << mtree->typeName()
            << "\" attriubtes!" << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      mtree->rangeSearch(attr, searchRad, info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      info = (rangesearchLocalInfo*)local.addr;
      if(!info->defined)
        return CANCEL;

      TupleId tid = info->next();
      if(tid)
      {
        Tuple *tuple = info->relation->GetTuple(tid);
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
      info = (rangesearchLocalInfo*)local.addr;
      delete info;
      return 0;
    }
  }

  return 0;
}

/********************************************************************
1.3.1.7 rangesearchDD[_]VM

********************************************************************/
int
rangesearchDD_VM(Word* args, Word& result, int message,
                 Word& local, Supplier s)
{
  rangesearchLocalInfo* info;

  switch (message)
  {
    case OPEN :
    {
      MTree* mtree = static_cast<MTree*>(args[0].addr);

      info = new rangesearchLocalInfo(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(info);

      DistDataAttribute* attr =
          static_cast<DistDataAttribute*>(args[2].addr);

      double searchRad =
            static_cast<CcReal*>(args[3].addr)->GetValue();

      string typeName =
          static_cast<CcString*>(args[4].addr)->GetValue();

      if (attr->distdataId() != mtree->dataId())
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
          << "Operator rangesearch:" << endl
          << "Distdata attribute type does not match the type of "
          << "the mtree!" << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      DistData* data = new DistData(attr->size(), attr->value());
      mtree->rangeSearch(data, searchRad, info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      info = (rangesearchLocalInfo*)local.addr;
      if(!info->defined)
        return CANCEL;

      TupleId tid = info->next();
      if(tid)
      {
        Tuple *tuple = info->relation->GetTuple(tid);
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
      info = (rangesearchLocalInfo*)local.addr;
      delete info;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.3.1.8 nnsearchLocalInfo

********************************************************************/
struct nnsearchLocalInfo
{
  Relation* relation;
  list<TupleId>* results;
  list<TupleId>::iterator iter;
  bool defined;

  nnsearchLocalInfo(Relation* rel)
  : relation(rel),
    results(new list<TupleId>),
    defined(false)
    {}

  void initResultIterator()
  {
    iter = results->begin();
    defined = true;
  }

  ~nnsearchLocalInfo()
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
1.3.1.9 nnsearch[_]VM

********************************************************************/
int
nnsearch_VM(Word* args, Word& result, int message,
            Word& local, Supplier s)
{
  nnsearchLocalInfo* info;

  switch (message)
  {
    case OPEN :
    {
      MTree* mtree =
          static_cast<MTree*>(args[0].addr);

      info = new nnsearchLocalInfo(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(info);

      Attribute* attr =
          static_cast<Attribute*>(args[2].addr);

      int nncount= ((CcInt*)args[3].addr)->GetValue();

      string typeName =
          static_cast<CcString*>(args[4].addr)->GetValue();

      if (mtree->typeName() != typeName)
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator nnsearch:" << endl
            << "Got an \"" << typeName << "\" attribute, but the "
            << "mtree contains \"" << mtree->typeName()
            << "\" attriubtes!" << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      mtree->nnSearch(attr, nncount, info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      info = (nnsearchLocalInfo*)local.addr;
      if(!info->defined)
        return CANCEL;

      TupleId tid = info->next();
      if(tid)
      {
        Tuple *tuple = info->relation->GetTuple(tid);
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
      info = (nnsearchLocalInfo*)local.addr;
      delete info;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.3.1.10 nnsearchDD[_]VM

********************************************************************/
int
nnsearchDD_VM(Word* args, Word& result, int message,
              Word& local, Supplier s)
{
  nnsearchLocalInfo* info;
  switch (message)
  {
    case OPEN :
    {
      MTree* mtree = static_cast<MTree*>(args[0].addr);

      info = new nnsearchLocalInfo(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(info);

      DistDataAttribute* attr =
          static_cast<DistDataAttribute*>(args[2].addr);

      int nncount = ((CcInt*)args[3].addr)->GetValue();


      if (attr->distdataId() != mtree->dataId())
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
          << "Operator nnsearch:" << endl
          << "Distdata attribute type does not match the type of "
          << "the mtree!" << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      DistData* data = new DistData(attr->size(), attr->value());
      mtree->nnSearch(data, nncount, info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      info = (nnsearchLocalInfo*)local.addr;
      if(!info->defined)
         return CANCEL;

      TupleId tid = info->next();
      if(tid)
      {
        Tuple *tuple = info->relation->GetTuple(tid);
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
      info = (nnsearchLocalInfo*)local.addr;
      delete info;
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.3.2 Type mappings

1.3.2.1 createmtree[_]TM

relation/tuple stream x attribute name -> mtree (op. createmtree)

relation/tuple stream x attribute name x
config name x distfun name -> mtree (op. createmtree2)

relation/tuple stream x attribute name x
config name x distfun name x distdata type -> mtree (op. createmtree3)

********************************************************************/
template<unsigned paramCnt>
ListExpr createmtree_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    stringstream paramCntErr;
    string errmsg;
    bool cond;
    NList nl_args(args);

    paramCntErr << "Expecting " << paramCnt << " arguments.";
    cond = nl_args.length() == paramCnt;
    CHECK_COND(cond, paramCntErr.str());

    NList arg1 = nl_args.first();
    NList arg2 = nl_args.second();

    // check first argument (should be relation or tuple stream)
    NList attrs;
    cond = (arg1.checkRel(attrs) || arg1.checkStreamTuple(attrs));
    errmsg = "Expecting a relation or tuple stream as first "
             "argument, but got a list with structure '" +
              arg1.convertToString() + "'.";
    CHECK_COND(cond, errmsg);

    // check, if second argument is the name of an existing attribute
    errmsg = "Expecting the name of an existing attribute as second "
             "argument, but got '" + arg2.convertToString() + "'.";
    CHECK_COND(arg2.isSymbol(), errmsg);

    // check, if attribute can be found in attribute list
    string attrName = arg2.str();
    errmsg = "Attribute name '" + attrName + "' is not known.\n"
             "Known Attribute(s):\n" + attrs.convertToString();
    ListExpr attrTypeLE;
    int attrIndex = FindAttribute(
            attrs.listExpr(), attrName, attrTypeLE);
    CHECK_COND(attrIndex > 0, errmsg);
    NList attrType (attrTypeLE);
    string typeName = attrType.str();

    // select config name
    string configName;
    if (paramCnt >= 3)
    {  // type mapping for createmtree2 and createmtree3
        NList arg3 = nl_args.third();
        errmsg = "Expecting the name of an existing mtree config "
                 "or '" + CONFIG_DEFAULT + "\" as third argument.";
        CHECK_COND(arg3.isSymbol(), errmsg);
        configName = arg3.str();
    }
    else
    {  // type mapping for createmtree
        configName = CONFIG_DEFAULT;
    }

    // check, if selected config name is defined
    if (configName == CONFIG_DEFAULT)
    {
        configName = MTreeConfigReg::defaultName();
        errmsg = "Default config (\"" + configName +
                 "\") not defined!";
    }
    else
    {
        errmsg = "Config \"" + configName + "\" not defined!";
    }
    CHECK_COND(MTreeConfigReg::isDefined(configName), errmsg);

    // select distfun name
    string distfunName;
    if (paramCnt >= 4)
    {  // type mapping for createmtree2 and createmtree3
        NList arg4 = nl_args.fourth();
        errmsg = "Expecting the name of an existing distance function "
                 "or '" + DFUN_DEFAULT + "\" as fourth argument.";
        CHECK_COND(arg4.isSymbol(), errmsg);
        distfunName = arg4.str();
    }
    else
    {  // type mapping for createmtree
        distfunName = DFUN_DEFAULT;
    }

    if (typeName == DISTDATA)
    {
        NList res1(APPEND);
        NList res2;
        res2.append(NList(attrIndex - 1));
        res2.append(NList(distfunName, true));
        res2.append(NList(configName, true));
        NList res3(MTREE);
        NList result(res1, res2, res3);
        return result.listExpr();
    }

    // *** typeName != DISTDATA ***

    // select distdata type
    string dataName;
    if (paramCnt == 5)
    { // type mapping for createmtree3
        NList arg5 = nl_args.fifth();
        errmsg = "Expecting the name of an existing distdata type "
                 "or '" + DDATA_DEFAULT + "\" as fifth argument.";
        CHECK_COND(arg5.isSymbol(), errmsg);
        dataName = arg5.str();
    }
    else
    { // type mapping for createmtree1 and createmtree2
        dataName = DistDataReg::defaultName(typeName);
    }

    // check, if selected distance function with selected distdata
    // type is defined
    if (dataName == DDATA_DEFAULT)
    {
        errmsg = "No default distdata type defined for type \"" +
                 typeName + "\"!";
        dataName = DistDataReg::defaultName(typeName);
        CHECK_COND(dataName != DDATA_UNDEFINED, errmsg);
    }
    else if(!DistDataReg::isDefined(typeName, dataName))
    {
        errmsg = "Distdata type \"" + dataName + "\" for type \"" +
                 typeName + "\" is not defined! Defined names: \n\n" +
                 DistDataReg::definedNames(typeName);
        CHECK_COND(false, errmsg);
    }

    if (distfunName == DFUN_DEFAULT)
    {
        distfunName = DistfunReg::defaultName(typeName);
        errmsg = "No default distance function defined for type \""
            + typeName + "\"!";
        CHECK_COND(distfunName != DFUN_UNDEFINED, errmsg);
    }
    else
    { // search distfun
        if (!DistfunReg::isDefined(
                distfunName, typeName, dataName))
        {
            errmsg = "Distance function \"" + distfunName +
                     "\" not defined for type \"" +
                     typeName + "\" and data type \"" +
                     dataName + "\"! Defined names: \n\n" +
                     DistfunReg::definedNames(typeName);
            CHECK_COND(false, errmsg);
        }
    }

    // check if selected distance function is a metric
    errmsg = "Distance function \"" + distfunName +
             "\" with \"" + dataName + "\" data for type \"" +
             typeName + "\" is no metric!";
    cond = DistfunReg::getInfo(
            distfunName, typeName, dataName).isMetric();
    CHECK_COND(cond, errmsg);

    // generate result list
    NList res1(APPEND);
    NList res2;
    res2.append(NList(attrIndex - 1));
    res2.append(NList(typeName, true));
    res2.append(NList(distfunName, true));
    res2.append(NList(dataName, true));
    res2.append(NList(configName, true));
    NList res3(MTREE);
    NList result(res1, res2, res3);

    return result.listExpr();
}

/********************************************************************
1.3.2.4 rangesearch[_]TM

********************************************************************/
ListExpr
rangesearch_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

  string errmsg;
  NList nl_args(args);

  errmsg = "Operator rangesearch expects four arguments(mtree x "
           "relation x search_attribute x search_range)";
  CHECK_COND(nl_args.length() == 4, errmsg);

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();
  NList arg3 = nl_args.third();
  NList arg4 = nl_args.fourth();

  // check first argument (should be a mtree)
  errmsg = "Expecting a mtree as first argument!";
  CHECK_COND(arg1.isEqual(MTREE), errmsg);

  // check second argument (should be relation)
  NList attrs;
  errmsg = "Expecting a relation as second argument, but got a "
           "list with structure '" + arg2.convertToString() + "'.";
  CHECK_COND(arg2.checkRel(attrs), errmsg);

  // check fourth argument
  errmsg = "Expecting an int value as fourth argument, but got '" +
           arg4.convertToString() + "'.";
  CHECK_COND(arg4.isEqual(REAL), errmsg);

  NList append(APPEND);
  NList result (
      append,
      NList(arg3.convertToString(), true).enclose(),
      NList(NList(STREAM), arg2.second()));
  return result.listExpr();
}

/********************************************************************
1.3.2.5 nnsearch[_]TM

********************************************************************/
ListExpr
nnsearch_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

  string errmsg;
  NList nl_args(args);

  errmsg = "Operator nnsearch expects four arguments(mtree x "
           "relation x search_attribute x nncount)";
  CHECK_COND(nl_args.length() == 4, errmsg);

  NList arg1 = nl_args.first();
  NList arg2 = nl_args.second();
  NList arg3 = nl_args.third();
  NList arg4 = nl_args.fourth();

  // check first argument (should be a mtree)
  errmsg = "Expecting a mtree as first argument!";
  CHECK_COND(arg1.isEqual(MTREE), errmsg);

  // check second argument (should be relation)
  NList attrs;
  errmsg = "Expecting a list with structure\n"
           "   rel (tuple ((a1 t1)...(an tn)))\n"
           "as second argument, but got a list with structure '" +
       arg2.convertToString() + "'.";
  CHECK_COND(arg2.checkRel(attrs), errmsg);

  // check fourth argument
  errmsg = "Expecting an int value as fourth argument, but got '" +
           arg4.convertToString() + "'.";
  CHECK_COND(arg4.isEqual(INT), errmsg);

  NList append(APPEND);
  NList result (
      append,
      NList(arg3.convertToString(), true).enclose(),
      NList(NList(STREAM), arg2.second()));
  return result.listExpr();
}

/********************************************************************
1.3.3 Selection functions

********************************************************************/
int
createmtree_Select(ListExpr args)
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

    if (arg1.first().isEqual(REL))
    {
        if(attrType.isEqual(DISTDATA))
            return 2;
        else
            return 0;
    }
    else if (arg1.first().isEqual(STREAM))
    {
        if(attrType.isEqual(DISTDATA))
            return 3;
        else
            return 1;
    }
    else
        return -1;
}

int
createmtree3_Select(ListExpr args)
{
    NList argsNL(args);
    NList arg1 = argsNL.first();

    if (arg1.first().isEqual(REL))
        return 0;
    else if (arg1.first().isEqual(STREAM))
        return 1;
    else
        return -1;
}

int
search_Select(ListExpr args)
{
    NList argsNL(args);
    NList arg3 = argsNL.third();
    if (arg3.isEqual(DISTDATA))
        return 1;
    else
        return 0;
}

/********************************************************************
1.3.4 Value mapping arrays

********************************************************************/
ValueMapping createmtree_Map[] = {
    createmtreeRel_VM<2>,
    createmtreeStream_VM<2>,
    createmtreeDDRel_VM<2>,
    createmtreeDDStream_VM<2>
};

ValueMapping createmtree2_Map[] = {
    createmtreeRel_VM<4>,
    createmtreeStream_VM<4>,
    createmtreeDDRel_VM<4>,
    createmtreeDDStream_VM<4>
};

ValueMapping createmtree3_Map[] = {
    createmtreeRel_VM<5>,
    createmtreeStream_VM<5>,
    createmtreeDDRel_VM<5>,
    createmtreeDDStream_VM<5>
};

ValueMapping rangesearch_Map[] = {
    rangesearch_VM,
    rangesearchDD_VM
};

ValueMapping nnsearch_Map[] = {
    nnsearch_VM,
    nnsearchDD_VM
};

/********************************************************************
1.3.5 Operator infos

********************************************************************/
struct createmtree_Info : OperatorInfo
{
    createmtree_Info()
    {
        name = "createmtree";
        signature =
        "relation/tuple stream x attribute -> mtree";
        syntax = "_ createmtree [_]";
        meaning =
            "creates a new mtree from relation or tuple stream in arg1\n"
            "arg2 must be the name of the attribute in arg1, "
            "which should be indexed by the mtree";
        example = "pictures createmtree [Pic]";
    }
};

struct createmtree2_Info : OperatorInfo
{
    createmtree2_Info()
    {
        name = "createmtree2";
        signature =
        "relation/tuple stream x attribute x config_name x "
        "metric_name -> mtree";
        syntax = "_ createmtree2 [_, _, _]";
        meaning =
            "creates a new mtree from relation or tuple stream in arg1\n"
            "arg2 must be the name of the attribute in arg1, "
            "which should be indexed by the mtree\n"
            "arg3 must be the name of a registered mtree-config\n"
            "arg4 must be the name of a registered metric";
        example = "pictures createmtree2 [Pic, mlbdistHP, quadr]";
    }
};

struct createmtree3_Info : OperatorInfo
{
    createmtree3_Info()
    {
        name = "createmtree3";
        signature =
        "relation/tuple stream x attribute x config_name x "
        "metric_name x distdata_name -> mtree";
        syntax = "_ createmtree3 [_, _, _, _]";
        meaning =
            "creates a new mtree from relation or tuple stream in arg1\n"
            "arg2 must be the name of the attribute in arg1, "
            "which should be indexed by the mtree\n"
            "arg3 must be the name of a registered mtree-config\n"
            "arg4 must be the name of a registered metric\n"
            "arg5 must be the name of a registered distdata type";
        example = "pictures createmtree [lab, mlbdistHP, quadr, lab256]";
    }
};

struct rangesearch_Info : OperatorInfo
{
    rangesearch_Info()
    {
        name = "rangesearch";
        signature = "mtree x relation x data x real -> tuple stream";
        syntax = "_ _ rangesearch [_, _]";
        meaning =
            "data must be of the same type as the type of the "
            "attributes which are indized in the mtree and is used "
            "as reference attribute for the search - the real value "
            "sets the search radius";
        example = "pictree rangesearch [pictures, pic1, 0.2]";
    }
};

struct nnsearch_Info : OperatorInfo
{
    nnsearch_Info()
    {
        name = "nnsearch";
        signature = "mtree x relation x data x int -> tuple stream";
        syntax = "_ _ nnsearch [_, _]";
        meaning =
            "data must be of the same type as the type of the "
            "attributes which are indized in the mtree and is used "
            "as reference attribute for the search - the int value "
            "sets the count of nearest neighbours to data in the result";
        example = "pictree rangesearch [pictures, pic1, 5]";
    }
};

/********************************************************************
1.4 Create and initialize the Algebra

********************************************************************/
class MTreeAlgebra : public Algebra
{

public:
    MTreeAlgebra() : Algebra()
    {
        AddTypeConstructor(&mtreeTC);

        AddOperator(createmtree_Info(),
                    createmtree_Map,
                    createmtree_Select,
                    createmtree_TM<2>);

        AddOperator(createmtree2_Info(),
                    createmtree2_Map,
                    createmtree_Select,
                    createmtree_TM<4>);

        AddOperator(createmtree3_Info(),
                    createmtree3_Map,
                    createmtree_Select,
                    createmtree_TM<5>);

        AddOperator(rangesearch_Info(),
                    rangesearch_Map,
                    search_Select,
                    rangesearch_TM);

        AddOperator(nnsearch_Info(),
                    nnsearch_Map,
                    search_Select,
                    nnsearch_TM);
    }

    ~MTreeAlgebra() {};
};

} // namespace mtreeAlgebra

mtreeAlgebra::MTreeAlgebra mtreeAlg;

extern "C"
Algebra* InitializeMTreeAlgebra(
    NestedList *nlRef, QueryProcessor *qpRef)
{
    nl = nlRef;
    qp = qpRef;
    return (&mtreeAlg);
}
