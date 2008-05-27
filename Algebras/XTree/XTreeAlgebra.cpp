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
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"
#include "XTree.h"

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

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
    return SetWord(0);
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
{ return SetWord(0); }

int SizeOfXTree()
{ return sizeof(XTree); }

bool CheckXTree(ListExpr typeName, ListExpr &error_Info)
{ return nl->IsEqual(typeName, XTREE); }

TypeConstructor xtreeTC(
        XTREE,       XTreeProp,
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

1.1.1.1 creatextreeRel[_]VM

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

    unsigned dim;
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
                xtree->initialize(dim, configName);
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

    unsigned dim;
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
                xtree->initialize(dim, configName);
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

    unsigned dim;
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
                xtree->initialize(dim, configName);
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

    unsigned dim;
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
                xtree->initialize(dim, configName);
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
1.1.1.1 rangesearch[_]VM

********************************************************************/
int rangesearch_VM(
        Word *args, Word &result, int message,
        Word &local, Supplier s)
{
  Search_LI *info;
  switch (message)
  {
    case OPEN :
    {
      XTree *xtree = static_cast<XTree*>(args[0].addr);

      info = new Search_LI(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(info);

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
            << attr->dim() << ", but the xtree contains"
            << xtree->dim() << "-dimensional data!"
            << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      xtree->rangeSearch(attr->hpoint(), searchRad, info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      info = (Search_LI*)local.addr;
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
      info = (Search_LI*)local.addr;
      delete info;
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
  Search_LI *info;

  switch (message)
  {
    case OPEN :
    {
      XTree *xtree = static_cast<XTree*>(args[0].addr);

      info = new Search_LI(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(info);

      HPointAttr *attr =
          static_cast<HPointAttr*>(args[2].addr);

      int nnCount =
            static_cast<CcInt*>(args[3].addr)->GetValue();

      if (attr->dim() != xtree->dim())
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator nnsearch:" << endl
            << "The given hpoint has the dimension "
            << attr->dim() << ", but the xtree contains"
            << xtree->dim() << "-dimensional data!"
            << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      xtree->nnSearch(attr->hpoint(), nnCount , info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      info = (Search_LI*)local.addr;
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
      info = (Search_LI*)local.addr;
      delete info;
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
  Search_LI *info;

  switch (message)
  {
    case OPEN :
    {
      XTree *xtree = static_cast<XTree*>(args[0].addr);

      info = new Search_LI(
          static_cast<Relation*>(args[1].addr));
      local = SetWord(info);

      HRectAttr *attr =
          static_cast<HRectAttr*>(args[2].addr);

      if (attr->dim() != xtree->dim())
      {
        const string seperator = "\n" + string(70, '-') + "\n";
        cmsg.error() << seperator
            << "Operator windowintersects:" << endl
            << "The given hrect has the dimension "
            << attr->dim() << ", but the xtree contains"
            << xtree->dim() << "-dimensional data!"
            << seperator << endl;
        cmsg.send();
        return CANCEL;
      }

      xtree->windowIntersects(attr->hrect(), info->results);
      info->initResultIterator();

      assert(info->relation != 0);
      return 0;
    }

    case REQUEST :
    {
      info = (Search_LI*)local.addr;
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
      info = (Search_LI*)local.addr;
      delete info;
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

    CHECK_LIST_LENGTH(paramCnt, args_NL)

    NList attrs;
    NList relStream_NL = args_NL.first();
    NList attr_NL = args_NL.second();

    CHECK_REL_OR_STREAM(relStream_NL, attrs, 1)

    // check, if the specified attr. can be found in attribute list
    CHECK_SYMBOL(attr_NL, 2)
    string attrName = attr_NL.str();
    string typeName;
    int attrIndex;
    CHECK_ATTRIBUTE(attrs, attrName, typeName, attrIndex, 2)

    string error = "The specified attributes are not of type "
                   "\"hpoint\" or \"hrect\"!";
    CHECK_COND(((typeName == "hpoint") ||
                (typeName == "hrect")), error)

    // get config name
    string configName;
    if (paramCnt == 3)
        GET_CONFIG_NAME(args_NL.third(), configName, 3)
    else
        configName = CONFIG_DEFAULT;

    if (!XTreeConfigReg::isDefined(configName))
    {
        string errmsg;
        errmsg = "Config \"" + configName +
                 "\" not defined, defined names:\n\n" +
                 XTreeConfigReg::definedNames();
        CHECK_COND(false, errmsg);
    }

    NList res1(APPEND);
    NList res2;
    res2.append(NList(attrIndex));
    res2.append(NList(configName, true));
    NList res3(XTREE);
    NList result(res1, res2, res3);
    return result.listExpr();
}

/********************************************************************
1.1.1.1 rangesearch[_]TM

********************************************************************/
ListExpr rangesearch_TM(ListExpr args)
{
    NList args_NL(args);

    CHECK_LIST_LENGTH(4, args_NL);

    NList attrs;
    NList xtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList hpoint_NL = args_NL.third();
    NList searchRad_NL = args_NL.fourth();

    CHECK_COND(
            xtree_NL.isEqual(XTREE),
            "First argument must be a xtree!");
    CHECK_REL(rel_NL, attrs, 2);
    CHECK_SYMBOL(hpoint_NL, 3);
    CHECK_REAL(searchRad_NL, 4);

    string typeName = hpoint_NL.str();
    string error = "Expecting a \"hpoint\" attribute as third "
                   "parameter!";
    CHECK_COND(typeName == "hpoint", error);

    NList stream_NL(STREAM);
    NList result(stream_NL, rel_NL.second());
    return result.listExpr();
}

/********************************************************************
1.1.1.1 nnsearch[_]TM

********************************************************************/
ListExpr nnsearch_TM(ListExpr args)
{
    NList args_NL(args);

    CHECK_LIST_LENGTH(4, args_NL);

    NList attrs;
    NList xtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList hpoint_NL = args_NL.third();
    NList nnCount_NL = args_NL.fourth();

    CHECK_COND(
            xtree_NL.isEqual(XTREE),
            "First argument must be a xtree!");
    CHECK_REL(rel_NL, attrs, 2);
    CHECK_SYMBOL(hpoint_NL, 3);
    CHECK_INT(nnCount_NL, 4);

    string typeName = hpoint_NL.str();
    string error = "Expecting a \"hpoint\" attribute as third "
                   "parameter!";
    CHECK_COND(typeName == "hpoint", error);

    NList stream_NL(STREAM);
    NList result(stream_NL, rel_NL.second());
    return result.listExpr();
}

/********************************************************************
1.1.1.1 windowintersects[_]TM

********************************************************************/
ListExpr windowintersects_TM(ListExpr args)
{
    NList args_NL(args);

    CHECK_LIST_LENGTH(3, args_NL);

    NList attrs;
    NList xtree_NL = args_NL.first();
    NList rel_NL = args_NL.second();
    NList hrect_NL = args_NL.third();

    CHECK_COND(
            xtree_NL.isEqual(XTREE),
            "First argument must be a xtree!");
    CHECK_REL(rel_NL, attrs, 2);
    CHECK_SYMBOL(hrect_NL, 3);

    string typeName = hrect_NL.str();
    string error = "Expecting a \"hrect\" as third parameter!";
    CHECK_COND(typeName == "hrect", error);

    NList stream_NL(STREAM);
    NList result(stream_NL, rel_NL.second());
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

    if (arg1.first().isEqual(REL))
    {
        if (attrType.isEqual("hpoint"))
            return 0;
        else
            return 2;
    }
    else if (arg1.first().isEqual(STREAM))
    {
        if (attrType.isEqual("hpoint"))
            return 1;
        else
            return 3;
    }
    else
        return -1;
}

/********************************************************************
1.1.1 Value mapping arrays

********************************************************************/
ValueMapping creatextree_Map[] = {
    creatextreeHPointRel_VM<2>,
    creatextreeHPointStream_VM<2>,
    creatextreeHRectRel_VM<2>,
    creatextreeHRectStream_VM<2>
};

ValueMapping creatextree2_Map[] = {
    creatextreeHPointRel_VM<3>,
    creatextreeHPointStream_VM<3>,
    creatextreeHRectRel_VM<3>,
    creatextreeHRectStream_VM<3>
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
        "relation x attribute -> xtree";
        syntax = "_ creatextree [_]";
        meaning =
            "Creates a new xtree from the relation "
            "in argument 1. Argument 2 must be the name of the "
            "attribute in the relation/tuple stream, that should "
            "be indexed in the xtree. "
            "This operator uses the default xtree config and the "
            "default distdata type for the type constructor of "
            "the specified attribute.";
        example = "strassen creatextree[geoData]";
    }
};

struct creatextree2_Info : OperatorInfo
{
    creatextree2_Info()
    {
        name = "creatextree2";
        signature =
        "relation x attribute x xtree-config -> xtree";
        syntax = "_ creatextree [_, _]";
        meaning =
            "Creates a new xtree from the relation "
            "in argument 1. Argument 2 must be the name of the "
            "attribute in the relation/tuple stream, that should "
            "be indexed in the xtree. "
            "This operator uses the default xtree config and the "
            "default gethpoint or getbbox function the type constructor of "
            "the specified attribute.";
        example = "strassen creatextree2[geoData, limit80e]";
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
        example = "xt rel rangesearch [p, 0.1]";
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
        example = "xt rel nnsearch [p, 5]";
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
        example = "xt rel windowintersects [r]";
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

        AddOperator(rangesearch_Info(),
                    rangesearch_VM,
                    rangesearch_TM);

        AddOperator(nnsearch_Info(),
                    nnsearch_VM,
                    nnsearch_TM);

        AddOperator(windowintersects_Info(),
                    windowintersects_VM,
                    windowintersects_TM);
    }

    ~XTreeAlgebra()
    {};
};

} // namespace xtreeAlgebra

xtreeAlgebra::XTreeAlgebra xtreeAlg;

extern "C"
Algebra *InitializeXTreeAlgebra(
    NestedList *nlRef, QueryProcessor *qpRef)
{
    nl = nlRef;
    qp = qpRef;
    return (&xtreeAlg);
}
