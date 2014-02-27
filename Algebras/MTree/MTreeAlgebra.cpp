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

January-May 2008, Mirko Dibbert

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
#include "ListUtils.h"
#include "Symbols.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

namespace mtreeAlgebra {

/********************************************************************
1.1 Type constructor ~mtree~

********************************************************************/
static ListExpr MTreeProp()
{
    ListExpr examplelist = nl->TextAtom();
    nl->AppendText(examplelist, "<relation> createmtree [<attrname>]"
                                     " where <attrname> is the key");

    return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
                    nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
                    nl->StringAtom("(let mymtree = pictures "
                                    "createmtree[pic] "))));
}

ListExpr OutMTree(ListExpr type_Info, Word w)
{
#ifdef __MTREE_OUTFUN_PRINT_STATISTICS
    static_cast<MTree*>(w.addr)->printTreeInfos();
#endif
    return nl->TheEmptyList();
}

Word InMTree(
        ListExpr type_Info, ListExpr value,
        int errorPos, ListExpr &error_Info, bool &correct)
{
    correct = false;
    return SetWord(Address(0));
}

Word Createmtree(const ListExpr type_Info)
{ return SetWord(new MTree()); }

void DeleteMTree(const ListExpr type_Info, Word &w)
{
    static_cast<MTree*>(w.addr)->deleteFile();
    delete static_cast<MTree*>(w.addr);
    w.addr = 0;
}

bool OpenMTree(SmiRecord &valueRecord, size_t &offset,
          const ListExpr type_Info, Word &w)
{
    SmiFileId fileid;
    valueRecord.Read(&fileid, sizeof(SmiFileId), offset);
    offset += sizeof(SmiFileId);

    MTree *mtree = new MTree(fileid);
    w = SetWord(mtree);
    return true;
}

bool SaveMTree(SmiRecord &valueRecord, size_t &offset,
          const ListExpr type_Info, Word &w)
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

void CloseMTree(const ListExpr type_Info, Word &w)
{
    MTree *mtree = (MTree*)w.addr;
    delete mtree;
}

Word CloneMTree(const ListExpr type_Info, const Word &w)
{ return SetWord(Address(0)); }

int SizeOfMTree()
{ return sizeof(MTree); }

bool CheckMTree(ListExpr typeName, ListExpr &error_Info)
{ return nl->IsEqual(typeName, MTree::BasicType()); }

TypeConstructor
mtreeTC(MTree::BasicType(),       MTreeProp,
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

1.1.1 Value mappings

1.1.1.1 createmtreeRel[_]VM

This value mapping function is used for all "createmtree"[4] operators, which expect a relation and non-distdata attributes.

It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt> int
        createmtreeRel_VM(Word *args, Word &result, int message,
                          Word &local, Supplier s)
{
    result = qp->ResultStorage(s);

    MTree *mtree =
        static_cast<MTree*>(result.addr);

    Relation *relation =
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

    DistDataId id = DistDataReg::getId(typeName, dataName);
    mtree->initialize(id, distfunName, configName);

    Tuple *tuple;
    GenericRelationIterator *iter = relation->MakeScan();
    while ((tuple = iter->GetNextTuple()) != 0)
    {
        Attribute *attr = tuple->GetAttribute(attrIndex);
        if(attr->IsDefined())
        {
            mtree->insert(attr, tuple->GetTupleId());
        }
        tuple->DeleteIfAllowed();
    }
    delete iter;

#ifdef __MTREE_PRINT_INSERT_INFO
    cout << endl;
#endif

    return 0;
}

/********************************************************************
1.1.1.1 createmtreeStream[_]VM

This value mapping function is used for all "createmtree"[4] operators, which expect a tupel stream and non-distdata attributes.

It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt>
int createmtreeStream_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    MTree *mtree = static_cast<MTree*>(result.addr);

    void *stream =
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

    DistDataId id = DistDataReg::getId(typeName, dataName);
    mtree->initialize(id, distfunName, configName);

    Word wTuple;
    qp->Open(stream);
    qp->Request(stream, wTuple);
    while (qp->Received(stream))
    {
        Tuple *tuple = static_cast<Tuple*>(wTuple.addr);
        Attribute *attr = tuple->GetAttribute(attrIndex);
        if(attr->IsDefined())
        {
            mtree->insert(attr, tuple->GetTupleId());
        }
        tuple->DeleteIfAllowed();
        qp->Request(stream, wTuple);
    }
    qp->Close(stream);

#ifdef __MTREE_PRINT_INSERT_INFO
    cout << endl;
#endif

    return 0;
}

/********************************************************************
1.1.1.1 createmtreeDDRel[_]VM

This value mapping function is used for all "createmtree"[4] operators, which expect a relation and distdata attributes.

It is designed as template function, which expects the count of arguments as template paremeter.

********************************************************************/
template<unsigned paramCnt>
int createmtreeDDRel_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    MTree *mtree = static_cast<MTree*>(result.addr);

    Relation *relation =
        static_cast<Relation*>(args[0].addr);

    int attrIndex =
        static_cast<CcInt*>(args[paramCnt].addr)->GetIntval();

    string distfunName =
        static_cast<CcString*>(args[paramCnt+1].addr)->GetValue();

    string configName =
        static_cast<CcString*>(args[paramCnt+2].addr)->GetValue();

    Tuple *tuple;
    GenericRelationIterator *iter = relation->MakeScan();

    DistDataId id;
    while ((tuple = iter->GetNextTuple()))
    {
        DistDataAttribute *attr = static_cast<DistDataAttribute*>(
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
            char* tdata = attr->getData();
            DistData *data = new DistData(attr->size(), tdata);
            delete[] tdata;
            mtree->insert(data, tuple->GetTupleId());
        }
        tuple->DeleteIfAllowed();
    }
    delete iter;

#ifdef __MTREE_PRINT_INSERT_INFO
    cout << endl;
#endif

    return 0;
}

/********************************************************************
1.1.1.1 createmtreeDDStream[_]VM

This value mapping function is used for all "createmtree"[4] operators, which expect a tupel stream and distdata attributes.

It is designed as template function, which expects the count of arguments as template paremeter.


********************************************************************/
template<unsigned paramCnt>
int createmtreeDDStream_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
    result = qp->ResultStorage(s);
    MTree *mtree = static_cast<MTree*>(result.addr);

    void *stream =
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
        Tuple *tuple = static_cast<Tuple*>(wTuple.addr);
        DistDataAttribute *attr = static_cast<DistDataAttribute*>(
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
            char* flobData = attr->getData();
            DistData *data = new DistData(attr->size(), flobData);
            delete[] flobData;
            mtree->insert(data, tuple->GetTupleId());
        }
        tuple->DeleteIfAllowed();
        qp->Request(stream, wTuple);
    }

    qp->Close(stream);

#ifdef __MTREE_PRINT_INSERT_INFO
    cout << endl;
#endif

    return 0;
}

/********************************************************************
1.1.1.1 Search[_]LI

Local Info object for the search value mappings.

********************************************************************/
struct search_LI
{
    Relation *relation;
    list<TupleId> *results;
    list<TupleId>::iterator iter;
    bool defined;

    search_LI(Relation *rel)
        : relation(rel),
          results(new list<TupleId>),
          defined(false)
    {}

    void initResultIterator()
    {
        iter = results->begin();
        defined = true;
    }

    ~search_LI()
    { delete results; }

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
1.1.1.1 rangesearch[_]VM

********************************************************************/
int rangesearch_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  search_LI *info = (search_LI*) local.addr;

  switch (message)
  {
    case OPEN :
    {
      if(info){ delete info; }
      MTree *mtree = static_cast<MTree*>(args[0].addr);
      info = new search_LI( static_cast<Relation*>(args[1].addr));
      local = SetWord(info);
      Attribute *attr = static_cast<Attribute*>(args[2].addr);
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
      if(!info){ return CANCEL; }
      if(!info->defined)
        return CANCEL;

      TupleId tid = info->next();
      if(tid)
      {
        Tuple *tuple = info->relation->GetTuple(tid, false);
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
      if(info){
         delete info;
         local.addr=0;
      }
      return 0;
    }
  }

  return 0;
}

/********************************************************************
1.1.1.1 rangesearchDD[_]VM

********************************************************************/
int rangesearchDD_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  search_LI * info = (search_LI*)local.addr;

  switch (message)
  {
    case OPEN :
    {
      if(info){
        delete info;
      }
      MTree *mtree = static_cast<MTree*>(args[0].addr);
      info = new search_LI( static_cast<Relation*>(args[1].addr));
      local = SetWord(info);
      DistDataAttribute *attr =
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
      char* flobData = attr->getData();
      DistData *data = new DistData(attr->size(), flobData);
      delete[] flobData;
      mtree->rangeSearch(data, searchRad, info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      if(!info){
        return CANCEL;
      }
      if(!info->defined)
        return CANCEL;

      TupleId tid = info->next();
      if(tid)
      {
        Tuple *tuple = info->relation->GetTuple(tid, false);
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
       if(info){
         delete info;
         local.addr = 0;
       }
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
  search_LI * info = (search_LI*)local.addr;

  switch (message)
  {
    case OPEN :
    { if(info){
        delete info;
      }
      MTree *mtree = static_cast<MTree*>(args[0].addr);
      info = new search_LI( static_cast<Relation*>(args[1].addr));
      local = SetWord(info);
      Attribute *attr = static_cast<Attribute*>(args[2].addr);
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
      if(!info){
        return CANCEL;
      }
      if(!info->defined)
        return CANCEL;

      TupleId tid = info->next();
      if(tid)
      {
        Tuple *tuple = info->relation->GetTuple(tid, false);
        result = SetWord(tuple);
        return YIELD;
      }
      else
      {
        return CANCEL;
      }
    }

    case CLOSE :
    { if(info){
        delete info;
        local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.1.1.1 nnsearchDD[_]VM

********************************************************************/
int nnsearchDD_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  search_LI *info = (search_LI*)local.addr;
  switch (message)
  {
    case OPEN :
    {
      if(info){
       delete info;
      }
      MTree *mtree = static_cast<MTree*>(args[0].addr);

      info = new search_LI(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(info);

      DistDataAttribute *attr =
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
      char* flobdata = attr->getData();
      DistData *data = new DistData(attr->size(), flobdata);
      delete[] flobdata;
      mtree->nnSearch(data, nncount, info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      if(!info){
         return CANCEL;
      }
      if(!info->defined)
         return CANCEL;

      TupleId tid = info->next();
      if(tid)
      {
        Tuple *tuple = info->relation->GetTuple(tid, false);
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
      if(info){
         delete info;
         local.addr = 0;
      }
      return 0;
    }
  }
  return 0;
}

/********************************************************************
1.1.1 Type mappings

1.1.1.1 createmtree[_]TM

********************************************************************/
template<unsigned paramCnt>
ListExpr createmtree_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    NList args_NL(args);

    if(!args_NL.hasLength(paramCnt)){
      stringstream err;
      err << "Expecting " << paramCnt << " argument(s)!";
      return listutils::typeError(err.str());
    }

    NList attrs;
    NList relStream_NL = args_NL.first();
    NList attr_NL = args_NL.second();

    if(!relStream_NL.checkRel(attrs) && !relStream_NL.checkStreamTuple(attrs)){
      return listutils::typeError(
                    "Argument 1 must be a relation or tuple stream!");
    }

    // check, if the specified attribute can be found in attribute list
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

    if (paramCnt > 3)
    {
        string errmsg = "No distdata attributes allowed for operator"
        "createmtree3 - use createmtree or createmtree2 instead)!";
        if(typeName == "distdata"){
          return listutils::typeError(errmsg);
        }
    }

    // get config name
    string configName;
    if (paramCnt >= 3){
      if(!args_NL.third().isSymbol()){
        listutils::typeError(
        "Argument 3 must be the name of an existing config object!");
      }
      configName = args_NL.third().str();
    }
    else
        configName = CONFIG_DEFAULT;

    if (!MTreeConfigReg::isDefined(configName))
    {
        string errmsg;
        errmsg = "Config \"" + configName +
                 "\" not defined, defined names:\n\n" +
                 MTreeConfigReg::definedNames();
        return listutils::typeError(errmsg);
    }

    // select distfun name
    string distfunName;
    if (paramCnt >= 4){
      if(!args_NL.fourth().isSymbol()){
        stringstream err;
        err << "Argument 4 must be the name of an existing "
        << "distance function or \"" + DFUN_DEFAULT + "\"!";
        return listutils::typeError(err.str());
      }
      distfunName = args_NL.fourth().str();
    }
    else
        distfunName = DFUN_DEFAULT;

    if (typeName == "distdata")
    {
        NList res1(Symbol::APPEND());
        NList res2;
        res2.append(NList(attrIndex));
        res2.append(NList(distfunName, true));
        res2.append(NList(configName, true));
        NList res3(MTree::BasicType());
        NList result(res1, res2, res3);
        return result.listExpr();
    }

    // *** typeName != "distdata" ***

    // select distdata type
    string dataName;
    if (paramCnt >= 5){
      if(!args_NL.fifth().isSymbol()){
        stringstream err;
        err << "Argument 5 must be the name of an existing "
        << "distdata type or \"" + DDATA_DEFAULT + "\"!";
        return listutils::typeError(err.str());
      }
      dataName = args_NL.fifth().str();
    }
    else
        dataName = DistDataReg::defaultName(typeName);

    // check, if selected distdata type is defined
    if (dataName == DDATA_DEFAULT){
      dataName = DistDataReg::defaultName(typeName);
      if(dataName == DDATA_UNDEFINED){
        string errmsg;
        errmsg = "No default distdata type defined for type "
                 "constructor \"" + typeName + "\"!";
        return listutils::typeError(errmsg);
      }
    } else if(!DistDataReg::isDefined(typeName, dataName)){
      string errmsg;
      errmsg = "Distdata type \"" + dataName + "\" for "
               "type constructor \"" + typeName +
               "\" is not defined! Defined names: \n\n" +
               DistDataReg::definedNames(typeName);
      return listutils::typeError(errmsg);
    }

    // Returs a type error, if the specified distance function is not defined.
    string errmsg;
    if (distfunName == DFUN_DEFAULT){
      distfunName = DistfunReg::defaultName(typeName);
      if(distfunName == DFUN_UNDEFINED){
        errmsg = "No default distance function defined for type \""
        + typeName + "\"!";
        return listutils::typeError(errmsg);
      }
    } else {
      if (!DistfunReg::isDefined(distfunName, typeName, dataName)){
        errmsg = "Distance function \"" + distfunName +
          "\" not defined for type \"" +
          typeName + "\" and data type \"" +
          dataName + "\"! Defined names: \n\n" +
          DistfunReg::definedNames(typeName);
        return listutils::typeError(errmsg);
      }
    }

    // check if selected distance function is a metric
    if(!DistfunReg::getInfo(distfunName, typeName, dataName).isMetric()){
      errmsg = "Distance function \"" + distfunName +
                "\" with \"" + dataName + "\" data for type \"" +
                typeName + "\" is no metric!";
      return listutils::typeError(errmsg);
    }

    // generate result list
    NList res1(Symbol::APPEND());
    NList res2;
    res2.append(NList(attrIndex));
    res2.append(NList(typeName, true));
    res2.append(NList(distfunName, true));
    res2.append(NList(dataName, true));
    res2.append(NList(configName, true));
    NList res3(MTree::BasicType());
    NList result(res1, res2, res3);

    return result.listExpr();
}

/********************************************************************
1.3.2.4 rangesearch[_]TM

********************************************************************/
ListExpr rangesearch_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    NList args_NL(args);

    if(!args_NL.hasLength(4)){
      return listutils::typeError("Expecting 4 argument(s)!");
    }

    NList attrs;
    NList mtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList data_NL = args_NL.third();
    NList searchRad_NL = args_NL.fourth();

    if(!mtree_NL.isEqual(MTree::BasicType())){
      return listutils::typeError("First argument must be a mtree!");
    }

    if(!rel_NL.checkRel(attrs)){
      return listutils::typeError("Argument 2 must be a relation!");
    }

    if(!data_NL.isSymbol()){
      return listutils::typeError(
      "Argument 3 must be a symbol or an atomar type!");
    }

    if(!searchRad_NL.isEqual(CcReal::BasicType())){
      return listutils::typeError("Argument 4 must be a \"real\" value!");
    }


    /* Further type checkings for the data parameter will be done
       in the value mapping function, since the type of the mtree
       is not yet known. */

    NList append(Symbol::APPEND());
    NList result (
        append,
        NList(data_NL.str(), true).enclose(),
        NList(NList(Symbol::STREAM()), rel_NL.second()));

    return result.listExpr();
}

/********************************************************************
1.3.2.5 nnsearch[_]TM

********************************************************************/
ListExpr nnsearch_TM(ListExpr args)
{
    // initialize distance functions and distdata types
    if (!DistfunReg::isInitialized())
        DistfunReg::initialize();

    NList args_NL(args);

    if(!args_NL.hasLength(4)){
      stringstream err;
      return listutils::typeError("Expecting 4 arguments!");
    }

    NList attrs;
    NList mtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList data_NL = args_NL.third();
    NList nnCount_NL = args_NL.fourth();

    if(!mtree_NL.isEqual(MTree::BasicType())){
      return listutils::typeError("First argument must be a mtree!");
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

    /* Further type checkings for the data parameter will be done
       in the value mapping function, since the type of the mtree
       is not yet known. */

    NList append(Symbol::APPEND());
    NList result (
        append,
        NList(data_NL.str(), true).enclose(),
        NList(NList(Symbol::STREAM()), rel_NL.second()));

    return result.listExpr();
}

/********************************************************************
1.3.3 Selection functions

********************************************************************/
int createmtree_Select(ListExpr args)
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
    {
        if(attrType.isEqual("distdata"))
            return 2;
        else
            return 0;
    }
    else if (arg1.first().isEqual(Symbol::STREAM()))
    {
        if(attrType.isEqual("distdata"))
            return 3;
        else
            return 1;
    }
    else
        return -1;
}

int createmtree3_Select(ListExpr args)
{
    NList argsNL(args);
    NList arg1 = argsNL.first();

    if (arg1.first().isEqual(Relation::BasicType()))
        return 0;
    else if (arg1.first().isEqual(Symbol::STREAM()))
        return 1;
    else
        return -1;
}

int search_Select(ListExpr args)
{
    NList argsNL(args);
    NList arg3 = argsNL.third();
    if (arg3.isEqual("distdata"))
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
    createmtreeRel_VM<3>,
    createmtreeStream_VM<3>,
    createmtreeDDRel_VM<3>,
    createmtreeDDStream_VM<3>
};

ValueMapping createmtree3_Map[] = {
    createmtreeRel_VM<5>,
    createmtreeStream_VM<5>
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
            "Creates a new mtree from the relation or tuple stream "
            "in argument 1. Argument 2 must be the name of the "
            "attribute in the relation/tuple stream, that should "
            "be indexed in the mtree. "
            "This operator uses the default mtree config and the "
            "default metric for the type constructor of the "
            "specified attribute (the distdata type is specified "
            "by the expected distdata type of the default metric).";
        example = "Orte createmtree[Ort]";
    }
};

struct createmtree2_Info : OperatorInfo
{
    createmtree2_Info()
    {
        name = "createmtree2";
        signature =
        "relation/tuple stream x attribute x config_name -> mtree";
        syntax = "_ createmtree2 [_, _, _]";
        meaning =
            "Creates a new mtree from the relation or tuple stream "
            "in argument 1. Argument 2 must be the name of the "
            "attribute in the relation/tuple stream, that should "
            "be indexed in the mtree. config_name must be the name "
            "of a registered mtree configuration."
            "This operator uses the default distdata type for the "
            "type constructor of the specified attribute.";
        example = "Orte createmtree2 [Ort, limit80e]";
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
            "Creates a new mtree from the relation or tuple stream "
            "in argument 1. Argument 2 must be the name of the "
            "attribute in the relation/tuple stream, that should "
            "be indexed in the mtree. config_name must be the name "
            "of a registered mtree configuration. metric_name must "
            "be the name of a defined metric and distdata_name must "
            "be the name of a registered distdata type.";
        example =
            "Orte createmtree3 [Ort, limit80e, edit, native]";
    }
};

struct rangesearch_Info : OperatorInfo
{
    rangesearch_Info()
    {
        name = "rangesearch";
        signature = "mtree x relation x DATA x real -> tuple stream";
        syntax = "_ _ rangesearch [_, _]";
        meaning =
            "Returns a tuple stream, which contains all attributes, "
            "that could be found within the search radius "
            "(argument 4) to the query data (argument 3). The "
            "relation must contain at least the same tuples, that "
            "has been used to create the mtree. ";
        example = "pictree pictures rangesearch [pic1, 0.2]";
    }
};

struct nnsearch_Info : OperatorInfo
{
    nnsearch_Info()
    {
        name = "nnsearch";
        signature = "mtree x relation x DATA x int -> tuple stream";
        syntax = "_ _ nnsearch [_, _]";
        meaning =
            "Returns a tuple stream, which contains the k nearest "
            "neighbours of the query data (argument 3), whereas "
            "k is specified in argument 4. The relation must "
            "contain at least the same tuples, that has been used "
            "to create the mtree. ";
        example = "pictree pictures nnsearch [pic1, 5]";
    }
};

/********************************************************************
1.4 Create and initialize the Algebra

********************************************************************/
class MTreeAlgebra
    : public Algebra
{

public:
    MTreeAlgebra()
        : Algebra()
    {
        AddTypeConstructor(&mtreeTC);

        AddOperator(createmtree_Info(),
                    createmtree_Map,
                    createmtree_Select,
                    createmtree_TM<2>);

        AddOperator(createmtree2_Info(),
                    createmtree2_Map,
                    createmtree_Select,
                    createmtree_TM<3>);

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

    ~MTreeAlgebra()
    {};
};

} // namespace mtreeAlgebra


extern "C"
Algebra *InitializeMTreeAlgebra(
    NestedList *nlRef, QueryProcessor *qpRef)
{
    nl = nlRef;
    qp = qpRef;
    return new mtreeAlgebra::MTreeAlgebra();
}
