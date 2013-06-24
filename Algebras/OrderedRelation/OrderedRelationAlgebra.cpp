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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module Ordered Relation Algebra

Winter 2009 Nicolai Voget

[TOC]

1 Overview

The Ordered Relational Algebra implements the type constructor ~orel~.

For more information about what the functions should do, see the
OrderedRelation.h header file.

2 Defines, includes, and constants

*/
#include "OrderedRelationAlgebra.h"
#include "StandardTypes.h"
#include "ListUtils.h"
#include "Progress.h"
#include "RelationAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include <limits>
#include "BTreeAlgebra.h"
#include "Symbols.h"
#include "LongInt.h"

//#define DEBUG_OREL

#ifdef DEBUG_OREL
#define DEBUG_OREL2
#endif


/*
2.1 Structs and Tools

2.1.1 Priority Queue and Visited Sections for Shortest Path Computations

*/
struct NodeEntry
{
  NodeEntry(){}

  NodeEntry(const int id, const int index, const int before)
    : nodeId(id), arrayIndex(index), beforeNodeId(before)
  {}

  NodeEntry(const NodeEntry& nE)
    : nodeId(nE.nodeId), arrayIndex(nE.arrayIndex),
      beforeNodeId(nE.beforeNodeId)
  {}

  ~NodeEntry()
  {}

  inline int Compare(const NodeEntry nE) const
  {
    return Compare(nE.GetNodeId());
  }

  inline int Compare(const int nId) const
  {
    if (nodeId < nId) return -1;
    if (nodeId > nId) return 1;
    return 0;
  }

  inline void operator=( const NodeEntry nE)
  {
    nodeId = nE.nodeId;
    arrayIndex = nE.arrayIndex;
    beforeNodeId = nE.beforeNodeId;
  }

  ostream& Print(ostream& os) const
  {
    os << "Node Number: " << nodeId;
    os << ", Predecessor Node Number: " << beforeNodeId;
    os << ", array Index: " << arrayIndex;
    os << endl;
    return os;
  }

  inline int GetNodeId() const
  {
    return nodeId;
  }

  inline int GetArrayIndex() const
  {
    return arrayIndex;
  }

  inline int GetBeforeNodeId() const
  {
    return beforeNodeId;
  }

  inline void SetNodeId(const int id)
  {
    nodeId = id;
  }

  inline void SetArrayIndex(const int index)
  {
    arrayIndex = index;
  }

  inline void SetBeforeNodeId (const int id)
  {
    beforeNodeId = id;
  }

  int nodeId;
  int arrayIndex;
  int beforeNodeId;
};

struct TreeEntry
{

  TreeEntry(){};

  TreeEntry(const NodeEntry elem, int l = -1, int r = -1)
    : nEntry(elem), left(l), right(r)
  {}

  TreeEntry(const TreeEntry& te)
    : nEntry(te.GetNodeEntry()), left (te.GetLeft()), right(te.GetRight())
  {}

  ~TreeEntry()
  {}

  inline NodeEntry GetNodeEntry() const
  {
    return nEntry;
  }

  inline int GetLeft() const
  {
    return left;
  }

  inline int GetRight() const
  {
    return right;
  }

  inline void operator=(const TreeEntry nTEntry)
  {
    nEntry = nTEntry.GetNodeEntry();
    left = nTEntry.GetLeft();
    right = nTEntry.GetRight();
  }

  inline void SetNodeEntry(const NodeEntry& ne)
  {
    nEntry = ne;
  }

  inline void SetLeft(const int n)
  {
    left = n;
  }

  inline void SetRight(const int n)
  {
    right = n;
  }

  inline int Compare(const TreeEntry& nE) const
  {
    return GetNodeEntry().Compare(nE.GetNodeEntry());
  }

  inline int Compare(const int nodeId) const
  {
    return GetNodeEntry().Compare(nodeId);
  }

  ostream& Print(ostream& os) const
  {
    os << "Entry: ";
    nEntry.Print(os);
    os << "left: " << left << ", right: " << right << endl;
    return os;
  }

  NodeEntry nEntry;
  int left,right;
};

struct NodeEntryTree
{
  NodeEntryTree()
    : tree(0)
  {
    fFree = 0;
  }

  NodeEntryTree(const int n)
    : tree(n)
  {
    fFree = 0;
  }

  ~NodeEntryTree()
  {}

  inline void Destroy()
  {
    tree.Destroy();
  }

  inline void Remove()
  {
    tree.Destroy();
    delete this;
  };

  int Find(const TreeEntry& te) const
  {
    int i = 0;
    if (tree.Size() < 1) return -1;
    while (i < fFree)
    {
      TreeEntry test = GetTreeEntry(i);
      switch(test.Compare(te))
      {
        case 0:
        {
          return i;
          break;
        }
        case -1:
        {
          if (test.GetRight() != -1) i = test.GetRight();
          else return i;
          break;
        }
        case 1:
        {
          if (test.GetLeft() != -1) i = test.GetLeft();
          else return i;
          break;
        }
        default: // should never been reached
        {
          return -1;
          break;
        }
      }
    }
    return -1; // should never been reached
  };

  int Find(const int nId) const
  {
    int i = 0;
    if (tree.Size() < 1) return -1;
    while (i < fFree)
    {
      TreeEntry test = GetTreeEntry(i);
      switch(test.GetNodeEntry().Compare(nId))
      {
        case 0:
        {
          return i;
          break;
        }
        case 1:
        {
          if (test.GetLeft() != -1) i = test.GetLeft();
          else return i;
          break;
        }
        case -1:
        {
          if (test.GetRight() != -1) i = test.GetRight();
          else return i;
          break;
        }
        default: // should never been reached
        {
          return -1;
          break;
        }
      }
    }
    return -1; // should never been reached
  };

  void Insert(const TreeEntry nE, int& newPos)
  {

    newPos = Find(nE);
    if (newPos < 0 )
    {
      newPos = fFree;
      fFree++;
      tree.Put(newPos, nE);
    }
    else
    {
      if (newPos < fFree)
      {
        TreeEntry test = GetTreeEntry(newPos);
        switch(test.Compare(nE))
        {
          case 1:
          {
            test.SetLeft(fFree);
            tree.Put(newPos,test);
            tree.Put(fFree,nE);
            fFree++;
            break;
          }
          case -1:
          {
            test.SetRight(fFree);
            tree.Put(newPos,test);
            tree.Put(fFree,nE);
            fFree++;
            break;
          }
          case 0:
          {
            tree.Put(newPos,nE);
            break;
          }
          default: //should never been reached
          {
            break;
          }
        }
      }
    }
  };

  void SetIndex (const int pos, const int index)
  {
    assert (pos > -1 && pos < fFree);
    TreeEntry te = GetTreeEntry(pos);
    NodeEntry nE = te.GetNodeEntry();
    nE.SetArrayIndex(index);
    te.SetNodeEntry(nE);
    tree.Put(pos,te);
  }

  inline int GetIndex (const int pos) const
  {
    assert(pos > -1 && pos < fFree);
    return GetTreeEntry(pos).GetNodeEntry().GetArrayIndex();
  }

  void SetBeforeNodeId(const int pos, const int before)
  {
    assert(pos > -1 && pos < fFree);
    TreeEntry te = GetTreeEntry(pos);
    NodeEntry ne = te.GetNodeEntry();
    ne.SetBeforeNodeId(before);
    te.SetNodeEntry(ne);
    tree.Put(pos,te);
  }

  TreeEntry GetTreeEntry(const int pos) const
  {
    assert(pos > -1 && pos < fFree);
    TreeEntry te;
    tree.Get(pos,te);
    return te;
  }

  bool IsNode(const int pos, const int nodeNumber) const
  {
    assert(pos > -1 && pos < fFree);

    if (GetTreeEntry(pos).GetNodeEntry().GetNodeId() == nodeNumber)
      return true;
    else
      return false;
  }

  ostream& Print(ostream& os) const
  {
    os << "Start NodeEntryTree: " << endl;
    TreeEntry tE;
    if (fFree > 0)
    {
      for (int i = 0; i < tree.Size(); i++)
      {
        tree.Get(i,tE);
        tE.Print(os);
      }
    }
    os << "Ende NodeEntryTree" << endl;
    return os;
  }

  DbArray<TreeEntry> tree;
  int fFree;
};


struct PQEntryOrel
{
  PQEntryOrel() {}

  PQEntryOrel(const int node, const int before, const double dist,
              const double prio)
    : nodeNumber(node), beforeNodeNumber(before), distFromStart(dist),
      prioval(prio)
  {}

  PQEntryOrel(const PQEntryOrel& nE)
    : nodeNumber(nE.nodeNumber), beforeNodeNumber(nE.beforeNodeNumber),
      distFromStart(nE.distFromStart), prioval(nE.prioval)
  {}

  ~PQEntryOrel()
  {}

  ostream& Print(ostream& os) const
  {
    os << "Node Number: " << nodeNumber;
    os << ", Predecessor Node Number: " << beforeNodeNumber;
    os << ", Prioval: " << prioval;
    os << ", Distance From Start: " << distFromStart;
    os << endl;
    return os;
  }

  int nodeNumber;
  int beforeNodeNumber;
  double distFromStart;
  double prioval;

};

struct PQueueOrel
{

  PQueueOrel()
    : prioQ(0)
  {
    firstFree = 0;
  }

  PQueueOrel(const int n)
    : prioQ(n)
  {
    firstFree = 0;
  }

  ~PQueueOrel()
  {}

  inline void Destroy()
  {
    prioQ.Destroy();
  }

  inline void Clear()
  {
    prioQ.clean();
    firstFree = 0;
  }

  inline bool IsEmpty() const
  {
    if (firstFree == 0 ) return true;
    else return false;
  }

  ostream& Print(ostream& os) const
  {
    os << "Start PriorityQueue: " << endl;
    PQEntryOrel pE;
    if (firstFree > 0)
    {
      for (int i = 0; i < firstFree; i++)
      {
        prioQ.Get(i,pE);
        pE.Print(os);
      }
    }
    os << "Ende PriorityQueue" << endl;
    return os;
  }

/*
If a node is reached second time and the prioval of the second way is
smaller than on the path found before. The prioval, the valFromStart and the
position in the  priority queue must be corrected.

*/

  void CorrectPosition ( const int actPosIndex, const PQEntryOrel& nEle,
                         NodeEntryTree* pNodeTree )
  {
    int testIndex = actPosIndex;
    int n = actPosIndex;
    PQEntryOrel test;
    bool found = false;
    while ( testIndex > 0 && !found )
    {

      if ( ( testIndex % 2 ) == 0 ) n = ( testIndex-2 ) / 2;
      else n = ( testIndex -1 ) / 2;
      if ( n >= 0 )
      {
        prioQ.Get ( n, test );
        if ( test.prioval > nEle.prioval ||
             (test.prioval == nEle.prioval &&
              test.distFromStart > nEle.distFromStart))
        {
          Swap ( n, nEle, testIndex, test, pNodeTree );
        }
        else
        {
          found = true;
        }
      }
      else
      {
        found = true;
      }
    }
  }

  void Append ( const PQEntryOrel nE, int pNElemPos,
                NodeEntryTree *pNodeTree)
  {
    int actPos = firstFree;
    prioQ.Put(actPos, nE );
    pNodeTree->Insert(TreeEntry(NodeEntry(nE.nodeNumber,
                                          actPos,
                                          nE.beforeNodeNumber),
                                -1,-1),
                      pNElemPos);
    CorrectPosition(actPos, nE, pNodeTree );
    firstFree++;
  }

  void Insert ( const PQEntryOrel nE, NodeEntryTree *pNodeTree,
                DbArray<PQEntryOrel>* spTree)
  {
    int pNElemPos = pNodeTree->Find(nE.nodeNumber);
    spTree->Append(nE);
    if (pNElemPos < 0)
      Append(nE, pNElemPos, pNodeTree);
    else
    {
      if ( pNElemPos > -1 && pNElemPos < pNodeTree->fFree)
      {
        if (!pNodeTree->IsNode(pNElemPos, nE.nodeNumber))
        {
          Append(nE, pNElemPos, pNodeTree);
        }
        else
        {
          int index = pNodeTree->GetIndex(pNElemPos);
          if (index > -1 && index < firstFree)
          {
            PQEntryOrel test;
            prioQ.Get(index, test);
            if (test.prioval > nE.prioval)
            {
              prioQ.Put(index, nE);
              pNodeTree->SetBeforeNodeId(pNElemPos, nE.beforeNodeNumber);
              CorrectPosition(index, nE, pNodeTree);
            }
          }
        }
      }
    }
  }

  void Swap(const int testIndex, const PQEntryOrel& last,
            int& actIndex, const PQEntryOrel& test1,
            NodeEntryTree* pNodeTree)
  {
    prioQ.Put(testIndex, last);
    pNodeTree->SetIndex(pNodeTree->Find(last.nodeNumber),testIndex);
    prioQ.Put(actIndex, test1);
    pNodeTree->SetIndex(pNodeTree->Find(test1.nodeNumber),actIndex);
    actIndex = testIndex;
  }

  PQEntryOrel* GetAndDeleteMin(NodeEntryTree* pNodeTree)
  {
    if (firstFree <= 0) return 0;
    PQEntryOrel result, last, test1, test2;
    prioQ.Get(0,result);
    PQEntryOrel* retValue = new PQEntryOrel(result);
    int tRet = pNodeTree->Find(result.nodeNumber);
    prioQ.Get(firstFree-1,last);
    prioQ.Put(0,last);
    prioQ.Put(firstFree-1, PQEntryOrel(-1,
                                       -1,
                                       numeric_limits<double>::max(),
                                       numeric_limits<double>::max()));
    firstFree--;
    int pNewPos = pNodeTree->Find(last.nodeNumber);
    pNodeTree->SetIndex(pNewPos,0);
    pNodeTree->SetIndex(tRet,-1);
    int actIndex = 0;
    int testIndex = 0;
    bool found = false;
    while (testIndex < firstFree && !found)
    {
      testIndex = 2*actIndex + 1;
      if (testIndex < firstFree-1)
      {
        prioQ.Get(testIndex, test1);
        prioQ.Get(testIndex+1, test2);
        if (test1.prioval < last.prioval ||
            test2.prioval < last.prioval)
        {
          if (test1.prioval <= test2.prioval)
            Swap(testIndex, last, actIndex, test1, pNodeTree);
          else
            Swap(testIndex+1, last, actIndex, test2, pNodeTree);
        }
        else
        {
          if(test1.prioval == last.prioval &&
             test1.distFromStart < last.distFromStart)
            Swap(testIndex, last, actIndex, test1, pNodeTree);
          else
          {
            if (test2.prioval == last.prioval &&
                test2.distFromStart < last.distFromStart)
              Swap(testIndex+1, last, actIndex, test2, pNodeTree);
            else
              found = true;
          }
        }
      }
      else
      {
        if (testIndex > 0 && testIndex == firstFree-1)
        {
          prioQ.Get(testIndex,test1);
          if (test1.prioval < last.prioval ||
              (test1.prioval == last.prioval &&
               test1.distFromStart < last.distFromStart))
            Swap(testIndex, last, actIndex, test1, pNodeTree);
          else
            found = true;
        }
        else
        {
          found = true;
        }
      }
    }
    return retValue;
  }

  DbArray<PQEntryOrel> prioQ;
  int firstFree;
};

struct  OShortestPathInfo
{
  OShortestPathInfo()
  {
    resTuples = 0;
    iter = 0;
    counter = 0;
    seqNoAttrIndex = 0;
    resultSelect = 0;
  }

  ~OShortestPathInfo(){};

  TupleBuffer* resTuples;
  GenericRelationIterator* iter;
  int counter;
  int seqNoAttrIndex;
  int resultSelect;
};


/*
3 Implementation
3.1 OrderedRelationIterator

*/
OrderedRelationIterator::OrderedRelationIterator(const OrderedRelation* orel,
                                                TupleType* newType /*=0*/,
                                                const CompositeKey& from,
                                                const CompositeKey& to):
              tupleType(orel->tupleType), outtype(newType),
              tupleFile(orel->tupleFile), lobFileId(orel->lobFileId),
              appendix(-1) {
#ifdef DEBUG_OREL
cout << "Konstruktor_OrelIter" << endl;
#endif
  tupleType->IncReference();
  if(outtype!=0) outtype->IncReference();
  endOfScan = true;
  if(from.IsDefined() || to.IsDefined()) {
    if(from.IsDefined()) {
      if(to.IsDefined()) {
        it = orel->tupleFile->SelectRangePrefetched(from.GetSmiKey(),
                                                    to.GetSmiKey());
      } else {
        it = orel->tupleFile->SelectRightRangePrefetched(from.GetSmiKey());
      }
    } else {
      it = orel->tupleFile->SelectLeftRangePrefetched(to.GetSmiKey());
    }
  } else {
    it = orel->tupleFile->SelectAllPrefetched();
  }
  if (it!=0) {
    endOfScan = false;
  }
}

OrderedRelationIterator::~OrderedRelationIterator() {
  tupleType->DeleteIfAllowed();
  if(outtype!=0) outtype->DeleteIfAllowed();
  if(it!=0) delete it;
  return;
}

Tuple* OrderedRelationIterator::GetNextTuple() {
#ifdef DEBUG_OREL
cout << "GetNextTuple_OrelIter" << endl;
#endif
  if(!Advance()) {
    return 0;
  }
  Tuple* t = new Tuple(tupleType);
  if(t->OpenOrel(lobFileId, it, appendix)) {
    return t;
  } else {
    delete t;
    return 0;
  }
}

Tuple* OrderedRelationIterator::GetNextTuple(const list<int>& attrList) {
  if(!Advance()) {
    return 0;
  }
  Tuple* t = new Tuple(tupleType);
  if(t->OpenPartialOrel(outtype, attrList, lobFileId, it, appendix)) {
    return t;
  } else {
    delete t;
    return 0;
  }
}

TupleId OrderedRelationIterator::GetTupleId() const {
#ifdef DEBUG_OREL
cout << "GetTupleId_OrelIter" << endl;
#endif
  return appendix;
}

const CompositeKey& OrderedRelationIterator::GetKey() const {
#ifdef DEBUG_OREL
cout << "GetKey" << endl;
#endif
  return key;
}

bool OrderedRelationIterator::Advance() {
  if(endOfScan) return false;
  if(!it->Next()) {
    key = CompositeKey();
    endOfScan = true;
    return false;
  }
  SmiKey k;
  it->CurrentKey(k);
  key = CompositeKey(k);

  appendix = key.GetAppendix();
  return (appendix>=MIN_TUPLE_ID);
}

/*
3.2 OrderedRelation

*/
OrderedRelation::OrderedRelation(ListExpr typeInfo, bool createFiles/*=true*/):
  tupleType(new TupleType(nl->Second(typeInfo))),
  attrExtSize(tupleType->GetNoAttributes()),
  attrSize(tupleType->GetNoAttributes()) {
#ifdef DEBUG_OREL
cout << "Konstruktor_Orel(typeInfo)" << endl;
#endif
  GetKeyStructure(typeInfo, keyElement, keyElemType);
  if(createFiles) {
    tupleFile = new SmiBtreeFile(SmiKey::Composite);
    tupleFile->Create();
    tupleFileId = tupleFile->GetFileId();
    hasLobs = false;
    lobFileId = 0;
    if (tupleType->NumOfFlobs() > 0) {
      hasLobs = true;
      SmiRecordFile rf(false, 0, false);
      if(!rf.Create()) {
        assert(false);
      }
      lobFileId = rf.GetFileId();
      rf.Close();
    }
  }
  noTuples = 0;
  maxId = MIN_TUPLE_ID;
}

OrderedRelation::~OrderedRelation() {
#ifdef DEBUG_OREL
cout << "Destruktor_Orel" << endl;
#endif
  tupleType->DeleteIfAllowed();
  if(tupleFile!=0) {
    tupleFile->Close();
    delete tupleFile;
  }
}

ListExpr OrderedRelation::Out(const ListExpr typeInfo, Word value) {
#ifdef DEBUG_OREL
cout << "Out_Orel" << endl;
#endif
  OrderedRelation* orel = static_cast<OrderedRelation*>(value.addr);
#ifdef DEBUG_OREL2
cout << orel->noTuples << endl;
#endif
  ListExpr result = nl->TheEmptyList();
  ListExpr last = result, tupleList = result;

  GenericRelationIterator* rit = orel->MakeScan();
  Tuple* t = 0;
  ListExpr tupleTypeInfo = nl->TwoElemList(
      nl->Second(typeInfo),
      nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));

#ifdef DEBUG_OREL2
cout << "Out:\tvor Iteration" << endl;
cout << nl->ToString(tupleTypeInfo) << endl;
int count = 0;
#endif
  while ((t = rit->GetNextTuple()) !=0) {
    tupleList = t->Out(tupleTypeInfo);
    t->DeleteIfAllowed();
#ifdef DEBUG_OREL2
cout << "Tuple No. " << count++ << endl;
cout << nl->ToString(tupleList) << endl;
#endif
    if (result == nl->TheEmptyList()) {
      result = nl->Cons(tupleList, nl->TheEmptyList());
      last = result;
    } else {
      last = nl->Append(last,tupleList);
    }
  }
  delete rit;
  return result;
}

Word OrderedRelation::In(const ListExpr typeInfo, const ListExpr value,
                    const int errorPos, ListExpr& errorInfo, bool& correct) {
#ifdef DEBUG_OREL
cout << "In_Orel" << endl;
#endif
#ifdef DEBUG_OREL2
cout << nl->ToString(typeInfo) << endl;
#endif
  OrderedRelation* orel = new OrderedRelation(typeInfo);
  int tupleno = 0;
  Tuple* t;
  ListExpr list = value;
  ListExpr first;
  correct=true;

  ListExpr tupleTypeInfo = nl->TwoElemList(nl->Second(typeInfo),
      nl->IntAtom(nl->ListLength(nl->Second(nl->Second(typeInfo)))));

  while(!nl->IsEmpty(list)) {
    first = nl->First(list);
    list = nl->Rest(list);
    tupleno++;
    t = Tuple::In(tupleTypeInfo, first, tupleno, errorInfo, correct);
    if(correct) {
      orel->AppendTuple(t);
      t->DeleteIfAllowed();
    } else {
      delete orel;
      return SetWord(Address(0));
    }
  }
  return SetWord(orel);
}

Word OrderedRelation::Create(const ListExpr typeInfo) {
#ifdef DEBUG_OREL
cout << "Create_Orel" << endl;
cout << nl->ToString(typeInfo) << endl;
#endif
  return SetWord(new OrderedRelation(typeInfo));
}

void OrderedRelation::Delete(const ListExpr typeInfo, Word& value) {
#ifdef DEBUG_OREL
cout << "Delete_Orel" << endl;
#endif
  OrderedRelation* orel = static_cast<OrderedRelation*>(value.addr);
  orel->tupleFile->Close();
  orel->tupleFile->Drop();
  delete orel->tupleFile;
  orel->tupleFile = 0; //to prevent ~OrderedRelation from closing tupleFile
  if (orel->hasLobs) {
    SmiRecordFile rf(false, 0, false);
    rf.Open(orel->lobFileId);
    rf.Close();
    rf.Drop();
  }
  delete orel;
  value.addr = 0;
}

bool OrderedRelation::Open(SmiRecord& valueRecord, size_t& offset,
#include "../../include/AlgebraTypes.h"
                      const ListExpr typeInfo, Word& value) {
#ifdef DEBUG_OREL
cout << "Open_Orel" << endl;
#endif
//STUB
  OrderedRelation* orel = new OrderedRelation(typeInfo, false);

  valueRecord.SetPos(offset);

  valueRecord.Read(orel->noTuples);
  valueRecord.Read(orel->maxId);

  valueRecord.Read(orel->tupleFileId);
  valueRecord.Read(orel->lobFileId);

  valueRecord.Read(orel->totalExtSize);
  valueRecord.Read(orel->totalSize);

  for(int i=0;i<orel->tupleType->GetNoAttributes();i++) {
    valueRecord.Read(orel->attrExtSize[i]);
  }
  for(int i=0;i<orel->tupleType->GetNoAttributes();i++) {
    valueRecord.Read(orel->attrSize[i]);
  }

  orel->tupleFile = new SmiBtreeFile(SmiKey::Composite);
  orel->tupleFile->Open(orel->tupleFileId);

  orel->hasLobs = (orel->lobFileId!=0);

  offset = valueRecord.GetPos();
  value = SetWord(orel);
  return true;
}


bool OrderedRelation::Save(SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value) {
#ifdef DEBUG_OREL
cout << "Save_Orel" << endl;
#endif
  OrderedRelation* orel = static_cast<OrderedRelation*>(value.addr);

  valueRecord.SetPos(offset);
  valueRecord.Write(orel->noTuples);
  valueRecord.Write(orel->maxId);

  valueRecord.Write(orel->tupleFileId);
  valueRecord.Write(orel->lobFileId);

  valueRecord.Write(orel->totalExtSize);
  valueRecord.Write(orel->totalSize);

  for(int i=0;i<orel->tupleType->GetNoAttributes();i++) {
    valueRecord.Write(orel->attrExtSize[i]);
  }
  for(int i=0;i<orel->tupleType->GetNoAttributes();i++) {
    valueRecord.Write(orel->attrSize[i]);
  }

  offset = valueRecord.GetPos();
  return true;
}


void OrderedRelation::Close(const ListExpr typeInfo, Word& value) {
#ifdef DEBUG_OREL
cout << "Close_Orel" << endl;
#endif
   delete (static_cast<OrderedRelation*>(value.addr));
}


Word OrderedRelation::Clone(const ListExpr typeInfo, const Word& value) {
#ifdef DEBUG_OREL
cout << "Clone_Orel" << endl;
#endif
  OrderedRelation* clone = new OrderedRelation(typeInfo);
  OrderedRelation* orel = static_cast<OrderedRelation*>(value.addr);
  Tuple* t;
  OrderedRelationIterator* iter = (OrderedRelationIterator*)orel->MakeScan();
  while((t = iter->GetNextTuple()) != 0) {
#ifdef DEBUG_OREL2
cout << "GotNextTuple" << endl;
#endif
    clone->AppendTuple(t);
    t->DeleteIfAllowed();
  }
  delete iter;
  return SetWord(clone);
}


void* OrderedRelation::Cast(void* addr) {
#ifdef DEBUG_OREL
cout << "Cast_Orel" << endl;
#endif
  return 0;
}


bool OrderedRelation::CheckKind(const ListExpr typeInfo,
                                        ListExpr& errorInfo) {
#ifdef DEBUG_OREL
cout << "CheckKind_Orel" << endl;
#endif
#ifdef DEBUG_OREL2
cout << nl->ToString(typeInfo) << endl;
cout << nl->SymbolValue(nl->First(typeInfo)) << endl;
#endif
  if ( listutils::isOrelDescription(typeInfo) ) {
    return true;
  }
  else {
    errorInfo = nl->Append(errorInfo,
                           nl->ThreeElemList(nl->IntAtom(80),
                                             nl->SymbolAtom(OREL),
                                             typeInfo));
    return false;
  }
}


int OrderedRelation::GetNoTuples() const {
#ifdef DEBUG_OREL
cout << "GetNoTuples_Orel" << endl;
#endif
  return noTuples;
}


double OrderedRelation::GetTotalRootSize() const {
#ifdef DEBUG_OREL
cout << "GetTotalRootSize_Orel" << endl;
#endif
  return noTuples*tupleType->GetCoreSize();
}


double OrderedRelation::GetTotalRootSize(int i) const {
#ifdef DEBUG_OREL
cout << "GetTotalRootSize[i]_Orel" << endl;
#endif
  return noTuples*tupleType->GetAttributeType(i).coreSize;
}


double OrderedRelation::GetTotalExtSize() const {
#ifdef DEBUG_OREL
cout << "GetTotalExtSize_Orel" << endl;
#endif
  return totalExtSize;
}


double OrderedRelation::GetTotalExtSize(int i) const {
#ifdef DEBUG_OREL
cout << "GetTotalExtSize[i]_Orel" << endl;
#endif
  return attrExtSize[i];
}


double OrderedRelation::GetTotalSize() const {
#ifdef DEBUG_OREL
cout << "GetTotalSize_Orel" << endl;
#endif
  return totalSize;
}


double OrderedRelation::GetTotalSize(int i) const {
#ifdef DEBUG_OREL
cout << "GetTotalSize[i]_Orel" << endl;
#endif
  return attrSize[i];
}


void OrderedRelation::Clear() {
#ifdef DEBUG_OREL
cout << "Clear_Orel" << endl;
#endif
  noTuples=0;
  maxId = MIN_TUPLE_ID;
  tupleFile->Truncate();
  if(hasLobs) {
    SmiRecordFile rf(false, 0, false);
    rf.Open(lobFileId);
    rf.Truncate();
    rf.Close();
  }
  for(int i = 0;i<tupleType->GetNoAttributes();i++) {
    attrSize[i] = 0.0;
    attrExtSize[i] = 0.0;
  }
  totalExtSize = 0.0;
  totalSize = 0.0;
}


void OrderedRelation::AppendTuple(Tuple* t) {
#ifdef DEBUG_OREL
cout << "AppendTuple_Orel" << endl;
#endif
  SmiRecord record;
  TupleId extension = maxId++;
  bool rc = tupleFile->InsertRecord(GetKey(t, true, extension).GetSmiKey(),
                                    record);
  assert(rc==true);
  t->SaveOrel(&record, lobFileId, totalExtSize, totalSize, attrExtSize,
              attrSize, false, extension);
  record.Finish();
  noTuples++;
}


Tuple* OrderedRelation::GetTuple(const TupleId& id,
                                 const bool dontReportError) const {
#ifdef DEBUG_OREL
cout << "GetTuple_Orel" << endl;
#endif
  return 0;
}


Tuple* OrderedRelation::GetTuple(const TupleId& id, const int attrIndex,
                const vector<pair<int, int> >& intervals,
                const bool dontReportError) const {
  return GetTuple(id, dontReportError);
}


Tuple* OrderedRelation::GetTuple(const CompositeKey& key) const {
  Tuple* t = 0;
  SmiRecord record;
  if(tupleFile->SelectRecord(key.GetSmiKey(), record)) {
      t = new Tuple(tupleType);
      t->OpenOrel(lobFileId, record, key.GetAppendix());
  }
  return t;
}


Tuple* OrderedRelation::GetTuple(const CompositeKey& key, const int attrIndex,
                const vector<pair<int, int> >& intervals) const {
  Tuple* t = 0;
  if((t=GetTuple(key))!=0)
    t->GetAttribute(attrIndex)->Restrict(intervals);
  return t;
}

bool OrderedRelation::DeleteTuple(Tuple* tuple) {
  return DeleteTuple(tuple, true);
}

bool OrderedRelation::DeleteTuple(Tuple* tuple, bool deleteComplete) {
  CompositeKey key = GetKey(tuple, true, tuple->GetTupleId());
  SmiKeyedFileIterator iter;
  bool ok = false;
  SmiRecord record;
  if(tupleFile->SelectRecord(key.GetSmiKey(), iter, SmiFile::Update))
    ok = iter.Next(record) && iter.DeleteCurrent();
  if(ok) {
    Attribute* nextAttr;
    Flob* nextFlob;

    noTuples--;
    totalExtSize -= tuple->GetRootSize();
    totalSize -= tuple->GetRootSize();

    for (int i = 0; i < tuple->GetNoAttributes(); i++)
    {
      nextAttr = tuple->GetAttribute(i);
      if(deleteComplete) nextAttr->Finalize();
      for (int j = 0; j < nextAttr->NumOfFLOBs(); j++)
      {
        nextFlob = nextAttr->GetFLOB(j);
        SmiSize fsz = nextFlob->getSize();

        assert( i >= 0 &&
                (size_t)i < attrSize.size() );
        attrSize[i] -= fsz;
        totalSize -= fsz;

        if( fsz < Tuple::extensionLimit )
        {
          assert( i >= 0 &&
                  (size_t)i < attrExtSize.size() );
          attrExtSize[i] -= fsz;
          totalExtSize -= fsz;
        }

        if(deleteComplete) nextFlob->destroy();
      }
    }
  }
  return ok;
}

void OrderedRelation::UpdateTuple( Tuple *tuple,
                                   const vector<int>& changedIndices,
                                   const vector<Attribute *>& newAttrs ) {
  DeleteTuple(tuple, false);
  tuple->UpdateAttributesOrel(changedIndices, newAttrs );

  SmiRecord record;
  TupleId extension = tuple->GetTupleId();
  bool rc = tupleFile->InsertRecord(GetKey(tuple, true, extension).GetSmiKey(),
                                    record);
  assert(rc==true);
  tuple->SaveOrel(&record, lobFileId, totalExtSize, totalSize, attrExtSize,
              attrSize, false, extension);
  record.Finish();
  noTuples++;
}

GenericRelationIterator* OrderedRelation::MakeScan() const {
#ifdef DEBUG_OREL
cout << "MakeScan_Orel" << endl;
#endif
  return new OrderedRelationIterator(this);
}

GenericRelationIterator* OrderedRelation::MakeScan(TupleType* tt) const {
#ifdef DEBUG_OREL
cout << "MakeScan(tt)_Orel" << endl;
#endif
  return new OrderedRelationIterator(this, tt);
}

GenericRelationIterator* OrderedRelation::MakeRangeScan(
                                                  const CompositeKey& from,
                                                  const CompositeKey& to) const{
#ifdef DEBUG_OREL
cout << "MakeRangeScan_Orel" << endl;
#endif
  return new OrderedRelationIterator(this, 0, from, to);
}


GenericRelationIterator* OrderedRelation::MakeRangeScan(TupleType* tt,
                                                  const CompositeKey& from,
                                                  const CompositeKey& to) const{
#ifdef DEBUG_OREL
cout << "MakeRangeScan(tt)_Orel" << endl;
#endif
  return new OrderedRelationIterator(this, tt, from, to);
}


bool OrderedRelation::GetTupleFileStats(SmiStatResultType& result) {
#ifdef DEBUG_OREL
cout << "GetTupleFileStats_Orel" << endl;
#endif
  result = tupleFile->GetFileStatistics(SMI_STATS_EAGER);
  std::stringstream fileid;
  fileid << tupleFileId;
  result.push_back(pair<string,string>("FilePurpose",
            "OrderedRelationTupleCoreFile"));
  result.push_back(pair<string,string>("FileId",fileid.str()));
  return true;
}


bool OrderedRelation::GetLOBFileStats(SmiStatResultType& result) {
#ifdef DEBUG_OREL
cout << "GetLOBFileStats_Orel" << endl;
#endif
  if( !hasLobs ){
    return true;
  }
  SmiRecordFile lobFile(false);
  if( !lobFile.Open( lobFileId ) ){
    return false;
  } else {
    result = lobFile.GetFileStatistics(SMI_STATS_EAGER);
    result.push_back(pair<string,string>("FilePurpose",
                                         "OrderedRelationTupleLOBFile"));
    std::stringstream fileid;
    fileid << lobFileId;
    result.push_back(pair<string,string>("FileId",fileid.str()));
  }
  if( !lobFile.Close() )
    return false;
  return true;
}

CompositeKey OrderedRelation::GetKey(const Tuple* t, const bool appendNumber,
                                     const TupleId appendix) {
  return CompositeKey(t, keyElement, keyElemType, appendNumber, appendix);
}

CompositeKey OrderedRelation::GetRangeKey(Word& arg, int length, bool upper) {
  Word val;
  Supplier son;
  vector<void*> attributes(length);
  vector<SmiKey::KeyDataType> attrTypes(length);
  for(SmiSize i = 0; (i < (SmiSize)length); i++) {
    son = qp->GetSupplier(arg.addr, i);
    qp->Request(son, val);
    attributes[i] = val.addr;
    attrTypes[i] = keyElemType[i];
  }
  return CompositeKey(attributes, attrTypes, upper);
}

CompositeKey OrderedRelation::GetLowerRangeKey(Word& arg, int length) {
#ifdef DEBUG_OREL
cout << "GetLowerRangeKey" << endl;
#endif
  return GetRangeKey(arg, length, false);
}

CompositeKey OrderedRelation::GetUpperRangeKey(Word& arg, int length) {
#ifdef DEBUG_OREL
cout << "GetUpperRangeKey" << endl;
#endif
  return GetRangeKey(arg, length, true);
}

bool OrderedRelation::GetKeyStructure(ListExpr typeInfo,
                                 vector<int>& keyElement,
                                 vector<SmiKey::KeyDataType>& keyElemType) {
#ifdef DEBUG_OREL
cout << "GetKeytype_Orel" << endl;
cout << nl->ToString(typeInfo) << endl;
#endif
  if(nl->ListLength(typeInfo)!=3 || nl->IsAtom(nl->Second(typeInfo))
      || nl->ListLength(nl->Second(typeInfo))!=2) return false;
  //(orel(tuple((a1 t1)...(an tn)) (ai1 ai2 ai3))) or
  //(orel(tuple((a1 t1)...(an tn)) ai)) expected

  ListExpr tupleInfo = nl->Second(nl->Second(typeInfo));
  ListExpr keyInfo = nl->Third(typeInfo);
  int keyCount = 0;
  if(nl->IsAtom(keyInfo)) {
    keyCount = 1;
  } else {
    keyCount = nl->ListLength(keyInfo);
  }
  keyElement.resize(keyCount);
  keyElemType.resize(keyCount);
  int algId, typeId;
  for(int count = 0; count < keyCount; count++) {

#ifdef DEBUG_OREL2
cout << nl->IsAtom(keyInfo) << endl;
cout << nl->ToString(keyInfo) << endl;
if(nl->IsAtom(keyInfo)) {
cout << nl->SymbolValue(keyInfo) << endl;
}
#endif

    string id;
    if(nl->IsAtom(keyInfo)) {
      id = nl->SymbolValue(keyInfo);
    } else {
      id = nl->SymbolValue(nl->First(keyInfo));
      keyInfo = nl->Rest(keyInfo);
    }
    ListExpr tempInfo = tupleInfo;
    bool found = false;
    for(int i=0;!(nl->IsEmpty(tempInfo)||found);i++) {
      ListExpr current = nl->First(tempInfo);
      if(nl->SymbolValue(nl->First(current)) == id) {
        found = true;

#ifdef DEBUG_OREL2
cout << count << '\t' << id << '\t' << i << endl;
cout << nl->ToString(current) << endl;
#endif

        keyElement[count] = i;
        algId = nl->IntValue(nl->First(nl->Second(current)));
        typeId = nl->IntValue(nl->Second(nl->Second(current)));
        string keyTypeString = am->GetTC(algId, typeId)->Name();

        if (keyTypeString == CcInt::BasicType()) {
          keyElemType[count] = SmiKey::Integer;
        } else if(keyTypeString == LongInt::BasicType()){
          keyElemType[count] = SmiKey::Longint;
        } else if(keyTypeString == CcString::BasicType()) {
          keyElemType[count] = SmiKey::String;
        } else if(keyTypeString == CcReal::BasicType()) {
          keyElemType[count] = SmiKey::Float;
        } else {
          keyElemType[count] = SmiKey::Composite;
        }
      }
      tempInfo = nl->Rest(tempInfo);
    }
    if(!found) {
      return false;
    }
  }
  return true;
}

const SmiBtreeFile* OrderedRelation::GetTupleFile() const {
  return tupleFile;
}

const TupleType* OrderedRelation::GetTupleType() const {
  return tupleType;
}

OrderedRelation::OrderedRelation() {
#ifdef DEBUG_OREL
cout << "Konstruktor_Orel" << endl;
#endif
}

ostream& OrderedRelation::Print(ostream& os) const
{
  GenericRelationIterator* itORel = MakeScan();
  Tuple* actTuple = itORel->GetNextTuple();
  while (actTuple != 0)
  {
    actTuple->Print(os);
    actTuple->DeleteIfAllowed();
    actTuple = itORel->GetNextTuple();
  }
  return os;
}
/*
4 Operators

4.1 orange, oleftrange \& orightrange

*/
enum RangeKind {
  LeftRange,
  RightRange,
  Range
};

/*
4.1.1 Type Mapping function for orange operators

*/
template<RangeKind rk> ListExpr ORangeTypeMap(ListExpr args) {
#ifdef DEBUG_OREL
cout << "RangeTypeMap" << endl;
#endif
#ifdef DEBUG_OREL2
cout << nl->ToString(args) << endl;
#endif
  string op= rk==LeftRange?"oleftrange":(rk==RightRange?"orightrange":"orange");
  int length = rk==Range?3:2;
  string typelist = "(orel (tuple (a1:t1...an:tn)) (ai1...ain)) x (ti1...tik)";
  if(rk==Range) typelist += " x (ti1...til)";
  if(nl->ListLength(args) != length) {
    ErrorReporter::ReportError("Operator " + op + " expects a list of type "
                                + typelist);
    return nl->TypeError();
  }
  if(!listutils::isOrelDescription(nl->First(args))) {
    ErrorReporter::ReportError("Operator " + op + " expects " + OREL +
                                " as first argument");
    return nl->TypeError();
  }
  bool ok = true;
  ListExpr keyList = nl->Third(nl->First(args));
  int t_size = nl->ListLength(keyList);
  vector<string> keyTypes(t_size);
  int count = 0;
  while (!nl->IsEmpty(keyList)) {
    ListExpr attrType;
    ListExpr current = nl->First(keyList);
    keyList = nl->Rest(keyList);
    listutils::findAttribute(nl->Second(nl->Second(nl->First(args))),
                              nl->SymbolValue(current), attrType);
    keyTypes[count++] = nl->SymbolValue(attrType);
  }
  ListExpr keyInfo = nl->Second(args);
  if(nl->IsAtom(keyInfo))
    return nl->TypeError();
  length = nl->ListLength(keyInfo);
  if((length==0) || (length > t_size)) {
    ErrorReporter::ReportError("Zero length or too long key list!");
    return nl->TypeError();
  }
  for (int i=0;ok && i<length;i++) {
    ListExpr current = nl->First(keyInfo);
    keyInfo = nl->Rest(keyInfo);
    if(!nl->IsAtom(current))
      return nl->TypeError();
    ok = ok && (keyTypes[i] == nl->SymbolValue(current));
  }
  if(!ok) {
    string tmpStr = rk==Range?"first ":"";
    ErrorReporter::ReportError("The " + tmpStr +
                                "range has to follow the typeorder of the " +
                                OREL + " key");
    return nl->TypeError();
  }
  if(rk==Range) {
    keyInfo = nl->Third(args);
    if(nl->IsAtom(keyInfo))
      return nl->TypeError();
    length = nl->ListLength(keyInfo);
    if((length==0) || (length > t_size)) {
      ErrorReporter::ReportError("Zero length or too long key list!");
      return nl->TypeError();
    }
    for (int i=0;ok && i<length;i++) {
      ListExpr current = nl->First(keyInfo);
      keyInfo = nl->Rest(keyInfo);
      if(!nl->IsAtom(current))
        return nl->TypeError();
      ok = ok && (keyTypes[i] == nl->SymbolValue(current));
    }
    if(!ok) {
      ErrorReporter::ReportError("The second range has to follow the "
                                  "typeorder of the " + OREL + " key");
      return nl->TypeError();
    }
  }
  ListExpr appList;
  if(rk==Range) {
    appList = nl->TwoElemList(nl->IntAtom(nl->ListLength(nl->Second(args))),
                              nl->IntAtom(nl->ListLength(nl->Third(args))));
  } else {
    appList = nl->OneElemList(nl->IntAtom(nl->ListLength(nl->Second(args))));
  }
  return nl->ThreeElemList(nl->SymbolAtom(Symbol::APPEND()), appList,
        nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                        nl->Second(nl->First(args))));
}

#ifndef USE_PROGRESS
/*
4.1.2 Value Mapping function of orange operators without progress estimation

*/

template<RangeKind rk> int ORangeValueMap(Word* args, Word& result, int message,
                                          Word& local, Supplier s) {
  GenericRelationIterator* rit;
  OrderedRelation* r;
  switch(message) {
    case OPEN: {

      CompositeKey fromKey;
      CompositeKey toKey;
      int l1 = 0;
      int l2 = 0;
      if(rk==Range) {
        l1 = ((CcInt*)args[3].addr)->GetIntval();
        l2 = ((CcInt*)args[4].addr)->GetIntval();
      } else {
        l1 = ((CcInt*)args[2].addr)->GetIntval();
      }
      r = (OrderedRelation*)args[0].addr;
      if(rk==LeftRange) {
        toKey = r->GetUpperRangeKey(args[1],l1);
      } else if(rk==RightRange) {
        fromKey = r->GetLowerRangeKey(args[1],l1);
      } else if(rk==Range) {
        fromKey = r->GetLowerRangeKey(args[1],l1);
        toKey = r->GetUpperRangeKey(args[2],l2);
      }
      rit = r->MakeRangeScan(fromKey,toKey);
      local.addr = rit;
      return 0;
    }
    case REQUEST:
      rit = (GenericRelationIterator*)local.addr;
      Tuple* t;
      if((t = rit->GetNextTuple())) {
        result.setAddr(t);
        return YIELD;
      }
      return CANCEL;
      break;
    case CLOSE:
      if(local.addr) {
        rit = (GenericRelationIterator*)local.addr;
        delete rit;
      }
      return 0;
  }
  return 0;
}

#else

struct ORangeLocalInfo: public ProgressLocalInfo
{
  GenericRelationIterator* iter;
  CompositeKey fromKey, toKey;
  bool first;
  int Card;
  int completeCalls;
  int completeReturned;
};

/*
4.1.3 Value Mapping function of orange operators with progress estimation

*/
template<RangeKind rk> int ORangeValueMap(Word* args, Word& result, int message,
                                          Word& local, Supplier s) {

  ORangeLocalInfo* linfo = (ORangeLocalInfo*)local.addr;

  switch(message) {
    case OPEN: {
      OrderedRelation* orel = (OrderedRelation*)args[0].addr;
      if(!linfo) {
        linfo = new ORangeLocalInfo;
        linfo->completeCalls = 0;
        linfo->completeReturned = 0;
        linfo->iter = 0;
        local = SetWord(linfo);

        //initialization of sizes
        linfo->total = orel->GetNoTuples();
        linfo->defaultValue = 50;
        linfo->Size = 0;
        linfo->SizeExt = 0;
        linfo->noAttrs = orel->GetTupleType()->GetNoAttributes();
        linfo->attrSize = new double[linfo->noAttrs];
        linfo->attrSizeExt = new double[linfo->noAttrs];
        for(int i=0; i < linfo->noAttrs; i++) {
          linfo->attrSize[i] = orel->GetTotalSize(i) / (linfo->total + 0.001);
          linfo->attrSizeExt[i] = orel->GetTotalExtSize(i) /
                                 (linfo->total + 0.001);
          linfo->Size += linfo->attrSize[i];
          linfo->SizeExt += linfo->attrSizeExt[i];
        }
        linfo->sizesInitialized = true;
        linfo->sizesChanged = true;
      }
      int l1 = 0;
      int l2 = 0;
      if(rk==Range) {
        l1 = ((CcInt*)args[3].addr)->GetIntval();
        l2 = ((CcInt*)args[4].addr)->GetIntval();
      } else {
        l1 = ((CcInt*)args[2].addr)->GetIntval();
      }
      if(rk==LeftRange) {
        linfo->toKey = orel->GetUpperRangeKey(args[1],l1);
      } else if(rk==RightRange) {
        linfo->fromKey = orel->GetLowerRangeKey(args[1],l1);
      } else if(rk==Range) {
        linfo->fromKey = orel->GetLowerRangeKey(args[1],l1);
        linfo->toKey = orel->GetUpperRangeKey(args[2],l2);
      }

      SmiKeyRange fromRange, toRange;
      if(rk==RightRange || Range) {
        orel->GetTupleFile()->KeyRange(linfo->fromKey.GetSmiKey(),fromRange);
        linfo->Card = (int)(linfo->total*(1-fromRange.less));
        if(rk==Range) {
          orel->GetTupleFile()->KeyRange(linfo->toKey.GetSmiKey(), toRange);
          linfo->Card = (int)(linfo->total*(1-toRange.greater -
                                              fromRange.less));
        }
      } else {
        orel->GetTupleFile()->KeyRange(linfo->toKey.GetSmiKey(), toRange);
        linfo->Card = (int)(linfo->total*(1-toRange.greater));
      }
      if(linfo->iter){
        delete linfo->iter;
      }
      linfo->iter = orel->MakeRangeScan(linfo->fromKey,linfo->toKey);
      return 0;
    }
    case REQUEST:
      if(!linfo){
        return CANCEL;
      }
      if(!linfo->iter){
        return CANCEL;
      }
      Tuple* t;
      if((t = linfo->iter->GetNextTuple())) {
        linfo->returned++;
        result.setAddr(t);
        return YIELD;
      }
      return CANCEL;

    case CLOSE:
      if(linfo) {
        if(linfo->iter){
           delete linfo->iter;
           linfo->iter=0;
        }
        linfo->completeCalls++;
        linfo->completeReturned += linfo->returned;
        linfo->returned = 0;

        linfo->Card = linfo->completeReturned/linfo->completeCalls;
      }
      return 0;

    case REQUESTPROGRESS: {
      const double uORange = 0.08;  //ms per search
      const double vORange = 0.009; //ms per result tuple
      if(!linfo) return CANCEL;

      ProgressInfo* p;
      p = (ProgressInfo*)result.addr;
      p->CopySizes(linfo);
      if(linfo->returned > linfo->Card) {
        linfo->Card = (int)(linfo->returned * 1.1);
        if (linfo->Card > linfo->total) {
          linfo->Card = linfo->total;
        }
      }
      p->Card = linfo->Card;
      p->Time = uORange + p->Card*vORange;
      p->Progress = (uORange + linfo->returned*vORange)/p->Time;

      return YIELD;
    }
    case CLOSEPROGRESS:
      if(linfo) {
        if(linfo->iter){
           delete linfo->iter;
           linfo->iter = 0;
        }
        delete linfo;
        local = SetWord(Address(0));
      }
      return 0;
  }
  return 0;
}
#endif

/*
4.1.4 Specifications of orange operators

*/
const string OLeftRangeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(orel (tuple(a1:t1 ... an:tn)) (ai1 ai2 ... ain)) x (ti1 ti2) -> "
  "(stream (tuple(a1:t1 ... an:tn)))</text--->"
  "<text>_ oleftrange [key]</text--->"
  "<text>Returns a stream of tuples where each tuple's key is smaller than or "
  "equal as the given key.</text--->"
  "<text>query cities feed oconsume [BevT,Name] oleftrange[100] count</text--->"
  ") )";

const string ORightRangeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(orel (tuple(a1:t1 ... an:tn)) (ai1 ai2 ... ain)) x (ti1 ti2) -> "
  "(stream (tuple(a1:t1 ... an:tn)))</text--->"
  "<text>_ orightrange [key]</text--->"
  "<text>Returns a stream of tuples where each tuple's key is greater than or "
  "equal as the given key.</text--->"
  "<text>query cities feed oconsume [BevT,Name] orightrange[1000] count"
  "</text--->) )";

const string ORangeSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(orel (tuple(a1:t1 ... an:tn)) (ai1 ai2 ... ain)) x (ti1 ti2) x "
  "(ti1 ti2 ti3) -> (stream (tuple(a1:t1 ... an:tn)))</text--->"
  "<text>_ orange [leftkey, rightkey]</text--->"
  "<text>Returns a stream of tuples where each tuple's key is between the two "
  "given keys.</text--->"
  "<text>query cities feed oconsume [BevT,Name] orange[500;800] count</text--->"
  ") )";

Operator oleftrange (
                      "oleftrange",              // name
                      OLeftRangeSpec,            // specification
                      ORangeValueMap<LeftRange>, // value mapping
                      Operator::SimpleSelect,    // trivial selection function
                      ORangeTypeMap<LeftRange>   // type mapping
                      );


Operator orightrange (
                      "orightrange",              // name
                      ORightRangeSpec,            // specification
                      ORangeValueMap<RightRange>, // value mapping
                      Operator::SimpleSelect,     // trivial selection function
                      ORangeTypeMap<RightRange>   // type mapping
                      );


Operator orange (
                "orange",               // name
                ORangeSpec,             // specification
                ORangeValueMap<Range>,  // value mapping
                Operator::SimpleSelect, // trivial selection function
                ORangeTypeMap<Range>    // type mapping
                );

/*
4.2 Operator ~oshortestpathd~

We want to use the ordered relation as graph representation. And use this
specialised ordered relation for shortest path computing.

An ordered relation representing the edges of a graph contains for each edge of
the graph one tuple. Each tuple consists at least of two integer attributes
identifying the start and end node of the edge, and not less than one
double attribute representing the costs of the edge.
The tuples may contain further attributes giving additonal cost informations
or arbitrary informations for each edge.

For the shortest path operation the input relation should be ordered by the
numbers of the starting nodes at the first and by the end node numbers at the
second level to reduce the number of page accesses.

The operation ~oshortestpathd~ expects an ordered relation with at least three
attributes (two integer and one double) like described before. Two integers
identifying the start and the end node, an integer selecting the result
representation and a cost function mapping the tuple
values to an real number describing the costs of the edge.

The following results can be selected by integer:
0 shortest path
1 remaining priority queue at end of computation
2 visited sections of shortest path search
3 shortest path tree

In case of function result 3 the parameter end node is ignored. And the
complete shortest path tree from the start node will be computed.

In all cases the operation ~oshortestpathd~ extends the original tuples by an
new attribute seqNo of type int. Describing the sequence number of the edge
within the path from the start node to the end node (case 0), the number of
the edges in the priority queue (case 1), the sequence of visiting the edges
(case 2), and the sequence number the edge was inserted into the shortest path
tree (case 3).

For the path computation Dijkstras Algorithm is used.

*/

ListExpr OShortestPathDTypeMap(ListExpr args)
{
  #ifdef DEBUG_OREL
    cout << "OShortestPathDTypeMap" << endl;
  #endif
  #ifdef DEBUG_OREL2
    cout << nl->ToString(args) << endl;
  #endif
  if(nl->ListLength(args) != 5)
  {
    return listutils::typeError("oshortestpathd expects 5 arguments");
  }
  ListExpr orelList = nl->First(args);
  ListExpr startNodeList = nl->Second(args);
    ListExpr endNodeList = nl->Third(args);
    ListExpr resultSelect = nl->Fourth(args);
  ListExpr functionMap = nl->Fifth(args);

  //Check of first argument
  if(!listutils::isOrelDescription(orelList))
  {
    return listutils::typeError("oshortestpathd expects orel as 1. argument");
  }

  ListExpr orelTuple = nl->Second(orelList);

  if (!listutils::isTupleDescription(orelTuple))
  {
    return listutils::typeError("second value of orel is not of type tuple");
  }

  ListExpr orelAttrList(nl->Second(orelTuple));

  if (!listutils::isAttrList(orelAttrList))
  {
    return listutils::typeError("Error in orel attrlist.");
  }

  if (nl->ListLength(orelAttrList) >= 3)
  {
    ListExpr firstAttr = nl->First(orelAttrList);

    if (nl->ListLength(firstAttr) != 2 ||
        nl->SymbolValue(nl->Second(firstAttr)) != CcInt::BasicType())
    {
      return listutils::typeError("First attribute of orel should be int");
    }

    ListExpr secondAttr = nl->Second(orelAttrList);
    if (nl->ListLength(secondAttr) != 2 ||
        nl->SymbolValue(nl->Second(secondAttr)) != CcInt::BasicType())
    {
      return listutils::typeError("Second attribute of orel should be int");
    }
  }
  else
  {
    return listutils::typeError("orel has less than 3 attributes.");
  }

  //Check of second argument
  if (!listutils::isSymbol(startNodeList,CcInt::BasicType()))
  {
    return listutils::typeError("Second argument should be int");
  }

  //Check of third argument
  if (!listutils::isSymbol(endNodeList,CcInt::BasicType()))
  {
    return listutils::typeError("Third argument should be int");
  }

  //Check of fourth argument
  if (!listutils::isSymbol(resultSelect,CcInt::BasicType()))
  {
    return listutils::typeError("Fourth argument should be int");
  }

  //Check of fifth argument
  if (!listutils::isMap<1>(functionMap))
  {
    return listutils::typeError("Fourth argument should be a map");
  }

  ListExpr mapTuple = nl->Second(functionMap);

  if (!nl->Equal(orelTuple,mapTuple))
  {
    return listutils::typeError("Tuple of map function must match orel tuple");
  }

  ListExpr mapres = nl->Third(functionMap);

  if(!listutils::isSymbol(mapres,CcReal::BasicType()))
  {
    return listutils::typeError("Wrong mapping result type for oshortestpathd");
  }

  //returns stream of tuples like in orel
  NList extendAttrList(nl->TwoElemList(nl->SymbolAtom("SeqNo"),
                                       nl->SymbolAtom(CcInt::BasicType())));
  NList extOrelAttrList(nl->TheEmptyList());


  for (int i = 0; i < nl->ListLength(orelAttrList); i++)
  {
    NList attr(nl->Nth(i+1,orelAttrList));
    extOrelAttrList.append(attr);
  }

  extOrelAttrList.append(extendAttrList);

  ListExpr outlist = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                     nl->TwoElemList(
                                      nl->SymbolAtom(Tuple::BasicType()),
                                      extOrelAttrList.listExpr()));

  return outlist;
}

int OShortestPathDValueMap(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
#ifdef DEBUG_OREL
cout << "OShortestPathDValuMap" << endl;
#endif
  OShortestPathInfo* spi = (OShortestPathInfo*) local.addr;

  switch(message)
  {
    case OPEN:
    {
      //Create localinfo
      if (spi != 0) delete spi;
      spi = new OShortestPathInfo();
      ListExpr tupleType = GetTupleResultType( s );
      spi->resultSelect = ((CcInt*)args[3].addr)->GetIntval();
      if (spi->resultSelect < 0 || spi->resultSelect > 3)
      {
        cout << "Selected result value does not exist. Enter 0 for shortest";
        cout << " path, 1 for remaining priority queue elements, 2 for visited";
        cout << " edges, 3 for shortest path tree." << endl;
        return 0;
      }
      TupleType* rtt = new TupleType( nl->Second( tupleType ) );
      spi->resTuples = new TupleBuffer((size_t) 64*1024*1024);
      local.setAddr(spi);
      // Check for simplest Case
      int startNode = ((CcInt*)args[1].addr)->GetIntval();
      int endNode = ((CcInt*)args[2].addr)->GetIntval();
      if (spi->resultSelect < 3)
      {
        if (startNode == endNode)
        {
          //source and target node are equal no path
          rtt->DeleteIfAllowed();
          rtt = 0;
          return 0;
        }
      }
      //Shortest Path Evaluation
      //Get edge Tuples for StartNode
      OrderedRelation* orel = (OrderedRelation*)args[0].addr;
      OrderedRelationIterator* orelIt = 0;
      vector<void*> attributes(2);
      vector<SmiKey::KeyDataType> kElems(2);
      SmiKey test((int32_t) 0);
      kElems[0] = test.GetType();
      kElems[1] = test.GetType();
      int toNode = startNode;
      //Init priority Queue
      DbArray<PQEntryOrel>* spTree = new DbArray<PQEntryOrel>(0);
      NodeEntryTree* visitedNodes = new NodeEntryTree(0);
      PQueueOrel* prioQ = new PQueueOrel(0);
      prioQ->Insert(PQEntryOrel(startNode,
                                -1,
                                0.0,
                                0.0),
                    visitedNodes, spTree);
      bool found = false;
      PQEntryOrel* actPQEntry = 0;
      double dist = 0.0;
      Tuple* actTuple = 0;
      CcInt* minNodeId = new CcInt(true,0);
      CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
      //Search shortest path
      while(!prioQ->IsEmpty() && !found)
      {
        actPQEntry = prioQ->GetAndDeleteMin(visitedNodes);
        if (spi->resultSelect < 3 && actPQEntry->nodeNumber == endNode)
        {
          found = true;
        }
        else
        {
          CcInt* actNodeInt = new CcInt(true,actPQEntry->nodeNumber);

          attributes[0] = actNodeInt;
          attributes[1] = minNodeId;
          CompositeKey actNodeLower(attributes,kElems,false);
          attributes[1] = maxNodeId;
          CompositeKey actNodeUpper(attributes,kElems,true);
          orelIt =
            (OrderedRelationIterator*) orel->MakeRangeScan(actNodeLower,
                                                           actNodeUpper);
          actTuple = orelIt->GetNextTuple();
          while(actTuple != 0)
          {
            toNode = ((CcInt*)actTuple->GetAttribute(1))->GetIntval();
            if (actPQEntry->nodeNumber != toNode)
            {
              ArgVectorPointer funArgs = qp->Argument(args[4].addr);
              Word funResult;
              ((*funArgs)[0]).setAddr(actTuple);
              qp->Request(args[4].addr,funResult);
              double edgeCost = ((CcReal*)funResult.addr)->GetRealval();
              if (edgeCost < 0.0)
              {
                cerr << "Found negativ edge cost computation aborted." << endl;
                actTuple->DeleteIfAllowed();
                actTuple = 0;
                actNodeInt->DeleteIfAllowed();
                delete orelIt;
                orelIt = 0;
                delete actPQEntry;
                actPQEntry = 0;
                minNodeId->DeleteIfAllowed();
                maxNodeId->DeleteIfAllowed();
                attributes.clear();
                kElems.clear();
                prioQ->Clear();
                prioQ->Destroy();
                delete prioQ;
                visitedNodes->Destroy();
                delete visitedNodes;
                spTree->Destroy();
                delete spTree;
                rtt->DeleteIfAllowed();
                rtt = 0;
                return 0;
              }
              dist = actPQEntry->distFromStart + edgeCost;
              prioQ->Insert(PQEntryOrel(toNode,
                                        actPQEntry->nodeNumber,
                                        dist,
                                        dist),
                            visitedNodes,spTree);
            }
            actTuple->DeleteIfAllowed();
            actTuple = 0;
            actTuple = orelIt->GetNextTuple();
          }
          if (actTuple != 0)
          {
            actTuple->DeleteIfAllowed();
            actTuple = 0;
          }
          actNodeInt->DeleteIfAllowed();
          delete orelIt;
          orelIt = 0;
          delete actPQEntry;
          actPQEntry = 0;
        }
      }
      minNodeId->DeleteIfAllowed();
      maxNodeId->DeleteIfAllowed();
      if (spi->resultSelect < 3 && !found) //no path exists
      {
        cout << "no path exists" << endl;
        attributes.clear();
        kElems.clear();
        prioQ->Clear();
        prioQ->Destroy();
        delete prioQ;
        visitedNodes->Destroy();
        delete visitedNodes;
        spTree->Destroy();
        delete spTree;
        rtt->DeleteIfAllowed();
        rtt = 0;
        return 0;
      }
      else //Shortest Path found write result Relation
      {
        switch(spi->resultSelect)
        {
          case 0: // shortest path
          {
            int actEntryPos = visitedNodes->Find(actPQEntry->nodeNumber);
            TreeEntry te = visitedNodes->GetTreeEntry(actEntryPos);
            NodeEntry nE = te.GetNodeEntry();
            delete actPQEntry;
            actPQEntry = 0;
            CcInt* startNodeInt = new CcInt(true,nE.beforeNodeId);
            CcInt* endNodeInt = new CcInt(true,nE.nodeId);
            attributes[0] = startNodeInt;
            attributes[1] = endNodeInt;
            CompositeKey actNodeKeyLower(attributes,kElems,false);
            CompositeKey actNodeKeyUpper(attributes,kElems,true);
            Tuple *newTuple = 0;
            orelIt =
              (OrderedRelationIterator*) orel->MakeRangeScan(actNodeKeyLower,
                                                            actNodeKeyUpper);
            actTuple = orelIt->GetNextTuple();
            found = false;
            while (!found && actTuple != 0)
            {
              newTuple = new Tuple( rtt );
              int i = 0;
              while( i < actTuple->GetNoAttributes())
              {
                newTuple->CopyAttribute(i, actTuple,i);
                i++;
              }
              CcInt* noOfNodes = new CcInt(true, 0);
              spi->seqNoAttrIndex = i;
              newTuple->PutAttribute(i,noOfNodes);
              spi->resTuples->AppendTuple(newTuple);
              if (newTuple != 0)
              {
                newTuple->DeleteIfAllowed();
                newTuple = 0;
              }
              if (nE.GetBeforeNodeId() != startNode)
                actEntryPos = visitedNodes->Find(nE.GetBeforeNodeId());
              else
                found = true;
              if (!found)
              {
                te = visitedNodes->GetTreeEntry(actEntryPos);
                nE = te.GetNodeEntry();
                startNodeInt->DeleteIfAllowed();
                endNodeInt->DeleteIfAllowed();
                startNodeInt = new CcInt(true,nE.beforeNodeId);
                endNodeInt = new CcInt(true,nE.nodeId);
                attributes[0] = startNodeInt;
                attributes[1] = endNodeInt;
                CompositeKey actNodeKeyLower(attributes,kElems,false);
                CompositeKey actNodeKeyUpper(attributes,kElems,true);
                delete orelIt;
                orelIt = (OrderedRelationIterator*)
                  orel->MakeRangeScan(actNodeKeyLower, actNodeKeyUpper);
                if (actTuple != 0)
                {
                  actTuple->DeleteIfAllowed();
                  actTuple = 0;
                }
                actTuple = orelIt->GetNextTuple();
              }
            }
            if (startNodeInt != 0)
            {
              startNodeInt->DeleteIfAllowed();
              startNodeInt = 0;
            }
            if (endNodeInt != 0)
            {
              endNodeInt->DeleteIfAllowed();
              endNodeInt = 0;
            }
            if (actTuple != 0)
            {
              actTuple->DeleteIfAllowed();
              actTuple = 0;
            }
            delete orelIt;
            orelIt = 0;
            break;
          }

          case 1: //Remaining elements in priority queue
          {
            delete actPQEntry;
            actPQEntry = 0;
            for (int i = 0; i < prioQ->firstFree; i++)
            {
              actPQEntry = prioQ->GetAndDeleteMin(visitedNodes);
              if (actPQEntry == 0) break;
              CcInt* startNodeInt =
                new CcInt(true,actPQEntry->beforeNodeNumber);
              CcInt* endNodeInt = new CcInt(true,actPQEntry->nodeNumber);
              attributes[0] = startNodeInt;
              attributes[1] = endNodeInt;
              CompositeKey actNodeKeyLower(attributes,kElems,false);
              CompositeKey actNodeKeyUpper(attributes,kElems,true);
              orelIt =
                (OrderedRelationIterator*) orel->MakeRangeScan(actNodeKeyLower,
                                                               actNodeKeyUpper);
              actTuple = orelIt->GetNextTuple();
              Tuple* newTuple = new Tuple( rtt );
              int j = 0;
              while( j < actTuple->GetNoAttributes())
              {
                newTuple->CopyAttribute(j, actTuple,j);
                j++;
              }
              CcInt* noOfNodes = new CcInt(true, i+1);
              newTuple->PutAttribute(j, noOfNodes);
              spi->resTuples->AppendTuple(newTuple);
              if (newTuple != 0)
              {
                newTuple->DeleteIfAllowed();
                newTuple = 0;
              }
              if (startNodeInt != 0)
              {
                startNodeInt->DeleteIfAllowed();
                startNodeInt = 0;
              }
              if (endNodeInt != 0)
              {
                endNodeInt->DeleteIfAllowed();
                endNodeInt = 0;
              }
              if (actTuple != 0)
              {
                actTuple->DeleteIfAllowed();
                actTuple = 0;
              }
              delete orelIt;
              orelIt = 0;
              if (actPQEntry != 0)
              {
                delete actPQEntry;
                actPQEntry = 0;
              }
            }
            break;
          }

          case 2: //visited sections
          {
            PQEntryOrel pqElem;
            for (int i = 1; i < spTree->Size(); i++)
            {
              spTree->Get(i,pqElem);
              CcInt* startNodeInt = new CcInt(true,pqElem.beforeNodeNumber);
              CcInt* endNodeInt = new CcInt(true,pqElem.nodeNumber);
              attributes[0] = startNodeInt;
              attributes[1] = endNodeInt;
              CompositeKey actNodeKeyLower(attributes,kElems,false);
              CompositeKey actNodeKeyUpper(attributes,kElems,true);
              orelIt =
                (OrderedRelationIterator*) orel->MakeRangeScan(actNodeKeyLower,
                                                               actNodeKeyUpper);
              actTuple = orelIt->GetNextTuple();
              Tuple* newTuple = new Tuple( rtt );
              int j = 0;
              while( j < actTuple->GetNoAttributes())
              {
                newTuple->CopyAttribute(j, actTuple,j);
                j++;
              }
              CcInt* noOfNodes = new CcInt(true, i);
              newTuple->PutAttribute(j, noOfNodes);
              spi->resTuples->AppendTuple(newTuple);
              if (newTuple != 0)
              {
                newTuple->DeleteIfAllowed();
                newTuple = 0;
              }
              if (startNodeInt != 0)
              {
                startNodeInt->DeleteIfAllowed();
                startNodeInt = 0;
              }
              if (endNodeInt != 0)
              {
                endNodeInt->DeleteIfAllowed();
                endNodeInt = 0;
              }
              if (actTuple != 0)
              {
                actTuple->DeleteIfAllowed();
                actTuple = 0;
              }
              delete orelIt;
              orelIt = 0;
            }
            break;
          }

          case 3: //shortest path tree
          {
            NodeEntry actElem;
            for (int i = 1; i < visitedNodes->fFree; i++)
            {
              actElem = visitedNodes->GetTreeEntry(i).GetNodeEntry();
              CcInt* startNodeInt = new CcInt(true,actElem.GetBeforeNodeId());
              CcInt* endNodeInt = new CcInt(true,actElem.GetNodeId());
              attributes[0] = startNodeInt;
              attributes[1] = endNodeInt;
              CompositeKey actNodeKeyLower(attributes,kElems,false);
              CompositeKey actNodeKeyUpper(attributes,kElems,true);
              orelIt =
                (OrderedRelationIterator*) orel->MakeRangeScan(actNodeKeyLower,
                                                               actNodeKeyUpper);
              actTuple = orelIt->GetNextTuple();
              Tuple* newTuple = new Tuple( rtt );
              int j = 0;
              while( j < actTuple->GetNoAttributes())
              {
                newTuple->CopyAttribute(j, actTuple,j);
                j++;
              }
              CcInt* noOfNodes = new CcInt(true, i);
              newTuple->PutAttribute(j, noOfNodes);
              spi->resTuples->AppendTuple(newTuple);

              if (newTuple != 0)
              {
                newTuple->DeleteIfAllowed();
                newTuple = 0;
              }
              if (startNodeInt != 0)
              {
                startNodeInt->DeleteIfAllowed();
                startNodeInt = 0;
              }
              if (endNodeInt != 0)
              {
                endNodeInt->DeleteIfAllowed();
                endNodeInt = 0;
              }
              if (actTuple != 0)
              {
                actTuple->DeleteIfAllowed();
                actTuple = 0;
              }
              delete orelIt;
              orelIt = 0;
            }
            break;
          }

          default: //should never been reached;
          {
            break;
          }
        }
      }
      if (actPQEntry != 0)
      {
        delete actPQEntry;
        actPQEntry = 0;
      }
      if (actTuple != 0)
      {
         actTuple->DeleteIfAllowed();
         actTuple = 0;
      }
      rtt->DeleteIfAllowed();
      rtt = 0;
      attributes.clear();
      kElems.clear();
      prioQ->Clear();
      prioQ->Destroy();
      delete prioQ;
      visitedNodes->Destroy();
      delete visitedNodes;
      spTree->Destroy();
      delete spTree;
      spTree = 0;
      spi->counter = spi->resTuples->GetNoTuples();
      spi->iter = spi->resTuples->MakeScan();
      return 0;
      break;
    }

    case REQUEST:
    {
      if (spi != 0 && spi->resTuples != 0 && spi->iter != 0)
      {
        Tuple* res = spi->iter->GetNextTuple();
        if (res != 0)
        {
          if (spi->resultSelect == 0)
          {
            res->PutAttribute( spi->seqNoAttrIndex,
                               new CcInt (true, spi->counter));
            spi->counter--;
            result.setAddr(res);
            return YIELD;
            break;
          }
          else
          {
            if (spi->resultSelect > 0 && spi->resultSelect < 4)
            {
              result.setAddr(res);
              return YIELD;
              break;
            }
            else
              return CANCEL;
          }
        }
      }
      return CANCEL;
      break;
    }

    case CLOSE:
    {
      if (spi != 0)
      {
        delete spi->iter;
        spi->iter = 0;
        delete spi->resTuples;
        spi->resTuples = 0;
        delete spi;
        spi = 0;
        local.setAddr(0);
      }
      return 0;
      break;
    }

  }
  return 0;
}

const string OShortestPathDSpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(orel(tuple(a1:t1 ... an:tn))(ai1 ai2 ... ain)) x ti1 x ti2 x int"
  " x (tuple->real) -> "
  "(stream (tuple(a1:t1 ... an:tn SeqNo: int)))</text--->"
  "<text>_ oshortestpathd [int,int,int;fun]</text--->"
  "<text>The operation ~oshortestpathd~ expects an ordered relation "
  "representing the edges of a graph. Each edge of the graph is "
  "represented by one tuple consisting at least of two integer attributes "
  "identifying the start and end node of the edge, and not less than one real "
  "attribute representing the costs of the edge. "
  "The tuples may contain further "
  "attributes giving additional cost information or arbitrary informations for "
  "each edge. Besides this ordered relation, the operator expects the start and"
  " end node identifiers and a selecting int value choosing the return value."
  " In case of 0 the operator"
  " returns the shortest path from node ti1 to ti2 as a stream of tuples"
  " where each tuple is extended by a sequence number telling which is the"
  " place of the tuple in the shortest path. In case of 1 the operator returns"
  " the stream of tuples which are in the priority queue at the end of the"
  " operation extended by their number in the queue. In case of 2 the operator"
  " returns the stream of tuples which have been touched by the operator. In"
  " case of 3 the operator returns a stream of tuples giving the shortest path"
  " tree from the start node (The target node will be ignored in case 3)."
  " The last parameter is a cost function for the edges."
  " For the path computation Dijkstras Algorithm is used.</text--->"
  "<text>query otest oshortestpathd[1, 1, 0; .Cost] consume</text--->) )";

Operator oshortestpathd (
                "oshortestpathd",               // name
                OShortestPathDSpec,             // specification
                OShortestPathDValueMap,  // value mapping
                Operator::SimpleSelect, // trivial selection function
                OShortestPathDTypeMap    // type mapping
                );

/*
4.2 Operator ~oshortestpatha~

We want to use the ordered relation as graph representation. And use this
specialised ordered relation for shortest path computing.

An ordered relation representing the edges of a graph contains for each edge of
the graph one tuple. Each tuple consists at least of two integer attributes
identifying the start and end node of the edge, and not less than two
double attributes representing the costs of the edge, and the distance function
to the target. For weight computation in Astar-Algorithm.
The tuples may contain further attributes giving additonal cost informations
or arbitrary informations for each edge.

For the shortest path operation the input relation should be ordered by the
numbers of the starting nodes at the first and by the end node numbers at the
second level to reduce the number of page accesses.

The operation ~oshortestpatha~ expects an ordered relation with at least four
attributes (two integer and two double) like described before. Two integers
identifying the start and the end node, an integer selecting the result
representation, a cost function mapping the tuple values to an real number
describing the costs of the edge, and a cost function mapping the tuple values
to an real number describing the costs from the end of the edge to the target.

The following results can be selected by integer:
0 shortest path
1 remaining priority queue at end of computation
2 visited sections of shortest path search
3 shortest path tree detected before target node was reached.

In all cases the operation ~oshortestpatha~ extends the original tuples by an
new attribute seqNo of type int. Describing the sequence number of the edge
within the path from the start node to the end node (case 0), the number of
the edges in the priority queue (case 1), the sequence of visited edges
(case 2), and the sequence number the edge was inserted into the shortest path
tree (case 3).

For the path computation AStar-Algorithm is used.

*/

ListExpr OShortestPathATypeMap(ListExpr args)
{
  #ifdef DEBUG_OREL
    cout << "OShortestPathATypeMap" << endl;
  #endif
  #ifdef DEBUG_OREL2
    cout << nl->ToString(args) << endl;
  #endif
  if(nl->ListLength(args) != 6)
  {
    return listutils::typeError("oshortestpatha expects 5 arguments");
  }
  ListExpr orelList = nl->First(args);
  ListExpr startNodeList = nl->Second(args);
  ListExpr endNodeList = nl->Third(args);
  ListExpr selectFunList = nl->Fourth(args);
  ListExpr functionDistMap = nl->Fifth(args);
  ListExpr functionWeightMap = nl->Sixth(args);

  //Check of first argument
  if(!listutils::isOrelDescription(orelList))
  {
    return listutils::typeError("oshortestpatha expects orel as 1. argument");
  }

  ListExpr orelTuple = nl->Second(orelList);

  if (!listutils::isTupleDescription(orelTuple))
  {
    return listutils::typeError("second value of orel is not of type tuple");
  }

  ListExpr orelAttrList(nl->Second(orelTuple));

  if (!listutils::isAttrList(orelAttrList))
  {
    return listutils::typeError("Error in orel attrlist.");
  }

  if (nl->ListLength(orelAttrList) >= 4)
  {
    ListExpr firstAttr = nl->First(orelAttrList);

    if (nl->ListLength(firstAttr) != 2 ||
        nl->SymbolValue(nl->Second(firstAttr)) != CcInt::BasicType())
    {
      return listutils::typeError("First attribute of orel should be int");
    }

    ListExpr secondAttr = nl->Second(orelAttrList);
    if (nl->ListLength(secondAttr) != 2 ||
        nl->SymbolValue(nl->Second(secondAttr)) != CcInt::BasicType())
    {
      return listutils::typeError("Second attribute of orel should be int");
    }
  }
  else
  {
    return listutils::typeError("orel has less than 4 attributes.");
  }

  //Check of second argument
  if (!listutils::isSymbol(startNodeList,CcInt::BasicType()))
  {
    return listutils::typeError("Second argument should be int");
  }

  //Check of third argument
  if (!listutils::isSymbol(endNodeList,CcInt::BasicType()))
  {
    return listutils::typeError("Third argument should be int");
  }

  //Check of fourth argument
  if (!listutils::isSymbol(selectFunList,CcInt::BasicType()))
  {
    return listutils::typeError("Fourth argument should be int");
  }

  // check of fifth argument
  if (!listutils::isMap<1>(functionDistMap))
  {
    return listutils::typeError("Fifth argument should be a map");
  }

  ListExpr mapTuple1 = nl->Second(functionDistMap);

  if (!nl->Equal(orelTuple,mapTuple1))
  {
    return listutils::typeError("Tuple of map function must match orel tuple");
  }

  ListExpr mapres1 = nl->Third(functionDistMap);

  if(!listutils::isSymbol(mapres1,CcReal::BasicType()))
  {
    return listutils::typeError("Wrong mapping result type for oshortestpatha");
  }

  // check of sixth argument
  if (!listutils::isMap<1>(functionWeightMap))
  {
    return listutils::typeError("Sixth argument should be a map");
  }

  ListExpr mapTuple2 = nl->Second(functionWeightMap);

  if (!nl->Equal(orelTuple,mapTuple2))
  {
    return listutils::typeError("Tuple of map function must match orel tuple");
  }

  ListExpr mapres2 = nl->Third(functionWeightMap);

  if(!listutils::isSymbol(mapres2,CcReal::BasicType()))
  {
    return listutils::typeError("Wrong mapping result type for oshortestpath");
  }

  //returns stream of tuples like in orel
  NList extendAttrList(nl->TwoElemList(nl->SymbolAtom("SeqNo"),
                                       nl->SymbolAtom(CcInt::BasicType())));
  NList extOrelAttrList(nl->TheEmptyList());


  for (int i = 0; i < nl->ListLength(orelAttrList); i++)
  {
    NList attr(nl->Nth(i+1,orelAttrList));
    extOrelAttrList.append(attr);
    #ifdef DEBUG_OREL2
    cout << "extOrelAttrList: ";
    extOrelAttrList.writeAsStringTo(cout);
    cout << endl;
    #endif
  }

  extOrelAttrList.append(extendAttrList);


  ListExpr outlist = nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                                     nl->TwoElemList(
                                      nl->SymbolAtom(Tuple::BasicType()),
                                      extOrelAttrList.listExpr()));

  return outlist;
}

int OShortestPathAValueMap(Word* args, Word& result, int message,
                          Word& local, Supplier s)
{
#ifdef DEBUG_OREL
cout << "OShortestPathAValuMap" << endl;
#endif
  OShortestPathInfo* spi = (OShortestPathInfo*) local.addr;

  switch(message)
  {
    case OPEN:
    {
      //Create localinfo
      if (spi != 0) delete spi;
      spi = new OShortestPathInfo();
      spi->resultSelect = ((CcInt*)args[3].addr)->GetIntval();
      if (spi->resultSelect < 0 || spi->resultSelect > 3)
      {
        cout << "Selected result value does not exist. Enter 0 for shortest";
        cout << " path, 1 for remaining priority queue elements, 2 for visited";
        cout << " edges, 3 for shortest path tree till target reached." << endl;
        return 0;
      }
      ListExpr tupleType = GetTupleResultType( s );
      TupleType* rtt = new TupleType( nl->Second( tupleType ) );
      spi->resTuples = new TupleBuffer((size_t) 64*1024*1024);
      local.setAddr(spi);
      // Check for simplest Case
      int startNode = ((CcInt*)args[1].addr)->GetIntval();
      int endNode = ((CcInt*)args[2].addr)->GetIntval();
      if (startNode == endNode)
      {
        //source and target node are equal no path
        rtt->DeleteIfAllowed();
        rtt = 0;
        return 0;
      }

      //Shortest Path Evaluation
      //Get edge Tuples for StartNode
      OrderedRelation* orel = (OrderedRelation*)args[0].addr;
      OrderedRelationIterator* orelIt = 0;
      vector<void*> attributes(2);
      vector<SmiKey::KeyDataType> kElems(2);
      SmiKey test((int32_t) 0);
      kElems[0] = test.GetType();
      kElems[1] = test.GetType();
      int toNode = startNode;
      //Init priority Queue
      DbArray<PQEntryOrel>* spTree = new DbArray<PQEntryOrel>(0);
      NodeEntryTree* visitedNodes = new NodeEntryTree(0);
      PQueueOrel* prioQ = new PQueueOrel(0);
      prioQ->Insert(PQEntryOrel(startNode,
                                -1,
                                0.0,
                                0.0),
                    visitedNodes,spTree);
      bool found = false;
      PQEntryOrel* actPQEntry = 0;
      double dist = 0.0;
      Tuple* actTuple = 0;
      CcInt* minNodeId = new CcInt(true,0);
      CcInt* maxNodeId = new CcInt(true,numeric_limits<int>::max());
      //Search shortest path
      while(!prioQ->IsEmpty() && !found)
      {
        actPQEntry = prioQ->GetAndDeleteMin(visitedNodes);
        if (actPQEntry->nodeNumber == endNode)
        {
          found = true;
        }
        else
        {
          CcInt* actNodeInt = new CcInt(true,actPQEntry->nodeNumber);

          attributes[0] = actNodeInt;
          attributes[1] = minNodeId;
          CompositeKey actNodeLower(attributes,kElems,false);
          attributes[1] = maxNodeId;
          CompositeKey actNodeUpper(attributes,kElems,true);
          orelIt =
            (OrderedRelationIterator*) orel->MakeRangeScan(actNodeLower,
                                                           actNodeUpper);
          actTuple = orelIt->GetNextTuple();
          while(actTuple != 0)
          {
            toNode = ((CcInt*)actTuple->GetAttribute(1))->GetIntval();
            if (actPQEntry->nodeNumber != toNode)
            {
              ArgVectorPointer funArgs1 = qp->Argument(args[4].addr);
              Word funResult1;
              ((*funArgs1)[0]).setAddr(actTuple);
              qp->Request(args[4].addr,funResult1);
              double edgeCost = ((CcReal*)funResult1.addr)->GetRealval();
              if (edgeCost < 0.0)
              {
                cerr << "Found negativ edge cost computation aborted." << endl;
                actTuple->DeleteIfAllowed();
                actTuple = 0;
                actNodeInt->DeleteIfAllowed();
                delete orelIt;
                orelIt = 0;
                delete actPQEntry;
                actPQEntry = 0;
                minNodeId->DeleteIfAllowed();
                maxNodeId->DeleteIfAllowed();
                attributes.clear();
                kElems.clear();
                prioQ->Clear();
                prioQ->Destroy();
                delete prioQ;
                visitedNodes->Destroy();
                delete visitedNodes;
                spTree->Destroy();
                delete spTree;
                rtt->DeleteIfAllowed();
                rtt = 0;
                return 0;
              }
              dist = actPQEntry->distFromStart + edgeCost;
              ArgVectorPointer funArgs2 = qp->Argument(args[5].addr);
              Word funResult2;
              ((*funArgs2)[0]).setAddr(actTuple);
              qp->Request(args[5].addr,funResult2);
              double restCost = ((CcReal*)funResult2.addr)->GetRealval();
              if (restCost < 0) restCost = 0;
              double prioVal = dist + restCost;
              prioQ->Insert(PQEntryOrel(toNode,
                                        actPQEntry->nodeNumber,
                                        dist,
                                        prioVal),
                            visitedNodes,spTree);
            }
            actTuple->DeleteIfAllowed();
            actTuple = 0;
            actTuple = orelIt->GetNextTuple();
          }
          if (actTuple != 0)
          {
            actTuple->DeleteIfAllowed();
            actTuple = 0;
          }
          actNodeInt->DeleteIfAllowed();
          delete orelIt;
          orelIt = 0;
          delete actPQEntry;
          actPQEntry = 0;
        }
      }
      minNodeId->DeleteIfAllowed();
      maxNodeId->DeleteIfAllowed();
      if (!found) //no path exists
      {
        cout << "no path exists" << endl;
        attributes.clear();
        kElems.clear();
        prioQ->Clear();
        prioQ->Destroy();
        delete prioQ;
        visitedNodes->Destroy();
        delete visitedNodes;
        spTree->Destroy();
        delete spTree;
        rtt->DeleteIfAllowed();
        if (actPQEntry != 0)
        {
          delete actPQEntry;
          actPQEntry = 0;
        }
        rtt = 0;
        return 0;
      }
      else //Shortest Path found write result Relation
      {
        switch(spi->resultSelect)
        {
          case 0: // shortest path
          {
            int actEntryPos = visitedNodes->Find(actPQEntry->nodeNumber);
            TreeEntry te = visitedNodes->GetTreeEntry(actEntryPos);
            NodeEntry nE = te.GetNodeEntry();
            delete actPQEntry;
            actPQEntry = 0;
            CcInt* startNodeInt = new CcInt(true,nE.beforeNodeId);
            CcInt* endNodeInt = new CcInt(true,nE.nodeId);
            attributes[0] = startNodeInt;
            attributes[1] = endNodeInt;
            CompositeKey actNodeKeyLower(attributes,kElems,false);
            CompositeKey actNodeKeyUpper(attributes,kElems,true);
            Tuple *newTuple = 0;
            orelIt =
              (OrderedRelationIterator*) orel->MakeRangeScan(actNodeKeyLower,
                                                            actNodeKeyUpper);
            actTuple = orelIt->GetNextTuple();
            found = false;
            while (!found && actTuple != 0)
            {
              newTuple = new Tuple( rtt );
              int i = 0;
              while( i < actTuple->GetNoAttributes())
              {
                newTuple->CopyAttribute(i, actTuple,i);
                i++;
              }
              CcInt* noOfNodes = new CcInt(true, 0);
              spi->seqNoAttrIndex = i;
              newTuple->PutAttribute(i,noOfNodes);
              spi->resTuples->AppendTuple(newTuple);
              if (newTuple != 0)
              {
                newTuple->DeleteIfAllowed();
                newTuple = 0;
              }
              if (nE.GetBeforeNodeId() != startNode)
                actEntryPos = visitedNodes->Find(nE.GetBeforeNodeId());
              else
                found = true;
              if (!found)
              {
                te = visitedNodes->GetTreeEntry(actEntryPos);
                nE = te.GetNodeEntry();
                startNodeInt->DeleteIfAllowed();
                endNodeInt->DeleteIfAllowed();
                startNodeInt = new CcInt(true,nE.beforeNodeId);
                endNodeInt = new CcInt(true,nE.nodeId);
                attributes[0] = startNodeInt;
                attributes[1] = endNodeInt;
                CompositeKey actNodeKeyLower(attributes,kElems,false);
                CompositeKey actNodeKeyUpper(attributes,kElems,true);
                delete orelIt;
                orelIt = (OrderedRelationIterator*)
                  orel->MakeRangeScan(actNodeKeyLower, actNodeKeyUpper);
                if (actTuple != 0)
                {
                  actTuple->DeleteIfAllowed();
                  actTuple = 0;
                }
                actTuple = orelIt->GetNextTuple();
              }
            }
            if (startNodeInt != 0)
            {
              startNodeInt->DeleteIfAllowed();
              startNodeInt = 0;
            }
            if (endNodeInt != 0)
            {
              endNodeInt->DeleteIfAllowed();
              endNodeInt = 0;
            }
            if (actTuple != 0)
            {
              actTuple->DeleteIfAllowed();
              actTuple = 0;
            }
            delete orelIt;
            orelIt = 0;
            break;
          }

          case 1: //Remaining elements in priority queue
          {
            PQEntryOrel* actElem;
            for (int i = 0; i < prioQ->firstFree; i++)
            {
              actElem = prioQ->GetAndDeleteMin(visitedNodes);
              if (actElem == 0) break;
              CcInt* startNodeInt = new CcInt(true,actElem->beforeNodeNumber);
              CcInt* endNodeInt = new CcInt(true,actElem->nodeNumber);
              attributes[0] = startNodeInt;
              attributes[1] = endNodeInt;
              CompositeKey actNodeKeyLower(attributes,kElems,false);
              CompositeKey actNodeKeyUpper(attributes,kElems,true);
              orelIt =
                (OrderedRelationIterator*) orel->MakeRangeScan(actNodeKeyLower,
                                                               actNodeKeyUpper);
              actTuple = orelIt->GetNextTuple();
              Tuple* newTuple = new Tuple( rtt );
              int j = 0;
              while( j < actTuple->GetNoAttributes())
              {
                newTuple->CopyAttribute(j, actTuple,j);
                j++;
              }
              CcInt* noOfNodes = new CcInt(true, i+1);
              newTuple->PutAttribute(j, noOfNodes);
              spi->resTuples->AppendTuple(newTuple);
              if (newTuple != 0)
              {
                newTuple->DeleteIfAllowed();
                newTuple = 0;
              }
              if (startNodeInt != 0)
              {
                startNodeInt->DeleteIfAllowed();
                startNodeInt = 0;
              }
              if (endNodeInt != 0)
              {
                endNodeInt->DeleteIfAllowed();
                endNodeInt = 0;
              }
              if (actTuple != 0)
              {
                actTuple->DeleteIfAllowed();
                actTuple = 0;
              }
              delete orelIt;
              orelIt = 0;
              if (actElem != 0)
              {
                delete actElem;
                actElem = 0;
              }
            }
            break;
          }

          case 2: //visited sections
          {
            PQEntryOrel pqElem;
            for (int i = 1; i < spTree->Size(); i++)
            {
              spTree->Get(i,pqElem);
              CcInt* startNodeInt = new CcInt(true,pqElem.beforeNodeNumber);
              CcInt* endNodeInt = new CcInt(true,pqElem.nodeNumber);
              attributes[0] = startNodeInt;
              attributes[1] = endNodeInt;
              CompositeKey actNodeKeyLower(attributes,kElems,false);
              CompositeKey actNodeKeyUpper(attributes,kElems,true);
              orelIt =
                (OrderedRelationIterator*) orel->MakeRangeScan(actNodeKeyLower,
                                                               actNodeKeyUpper);
              actTuple = orelIt->GetNextTuple();
              Tuple* newTuple = new Tuple( rtt );
              int j = 0;
              while( j < actTuple->GetNoAttributes())
              {
                newTuple->CopyAttribute(j, actTuple,j);
                j++;
              }
              CcInt* noOfNodes = new CcInt(true, i);
              newTuple->PutAttribute(j, noOfNodes);
              spi->resTuples->AppendTuple(newTuple);
              if (newTuple != 0)
              {
                newTuple->DeleteIfAllowed();
                newTuple = 0;
              }
              if (startNodeInt != 0)
              {
                startNodeInt->DeleteIfAllowed();
                startNodeInt = 0;
              }
              if (endNodeInt != 0)
              {
                endNodeInt->DeleteIfAllowed();
                endNodeInt = 0;
              }
              if (actTuple != 0)
              {
                actTuple->DeleteIfAllowed();
                actTuple = 0;
              }
              delete orelIt;
              orelIt = 0;
            }
            break;
          }

          case 3: //shortest path tree
          {
            NodeEntry actElem;
            for (int i = 1; i < visitedNodes->fFree; i++)
            {
              actElem = visitedNodes->GetTreeEntry(i).GetNodeEntry();
              CcInt* startNodeInt = new CcInt(true,actElem.GetBeforeNodeId());
              CcInt* endNodeInt = new CcInt(true,actElem.GetNodeId());
              attributes[0] = startNodeInt;
              attributes[1] = endNodeInt;
              CompositeKey actNodeKeyLower(attributes,kElems,false);
              CompositeKey actNodeKeyUpper(attributes,kElems,true);
              orelIt =
                (OrderedRelationIterator*) orel->MakeRangeScan(actNodeKeyLower,
                                                               actNodeKeyUpper);
              actTuple = orelIt->GetNextTuple();
              Tuple* newTuple = new Tuple( rtt );
              int j = 0;
              while( j < actTuple->GetNoAttributes())
              {
                newTuple->CopyAttribute(j, actTuple,j);
                j++;
              }
              CcInt* noOfNodes = new CcInt(true, i);
              newTuple->PutAttribute(j, noOfNodes);
              spi->resTuples->AppendTuple(newTuple);

              if (newTuple != 0)
              {
                newTuple->DeleteIfAllowed();
                newTuple = 0;
              }
              if (startNodeInt != 0)
              {
                startNodeInt->DeleteIfAllowed();
                startNodeInt = 0;
              }
              if (endNodeInt != 0)
              {
                endNodeInt->DeleteIfAllowed();
                endNodeInt = 0;
              }
              if (actTuple != 0)
              {
                actTuple->DeleteIfAllowed();
                actTuple = 0;
              }
              delete orelIt;
              orelIt = 0;
            }
            break;
          }

          default: //should never been reached;
          {
            break;
          }
        }
      }
      if (actPQEntry != 0)
      {
        delete actPQEntry;
        actPQEntry = 0;
      }
      if (actTuple != 0)
      {
         actTuple->DeleteIfAllowed();
         actTuple = 0;
      }
      rtt->DeleteIfAllowed();
      rtt = 0;
      attributes.clear();
      kElems.clear();
      prioQ->Clear();
      prioQ->Destroy();
      delete prioQ;
      visitedNodes->Destroy();
      delete visitedNodes;
      spTree->Destroy();
      delete spTree;
      spi->counter = spi->resTuples->GetNoTuples();
      spi->iter = spi->resTuples->MakeScan();
      return 0;
      break;
    }

    case REQUEST:
    {
      if (spi != 0 && spi->resTuples != 0 && spi->iter != 0)
      {
        Tuple* res = spi->iter->GetNextTuple();
        if (res != 0)
        {
          if (spi->resultSelect == 0)
          {
            res->PutAttribute( spi->seqNoAttrIndex,
                               new CcInt (true, spi->counter));
            spi->counter--;
            result.setAddr(res);
            return YIELD;
            break;
          }
          else
          {
            if (spi->resultSelect > 0 && spi->resultSelect < 4)
            {
              result.setAddr(res);
              return YIELD;
              break;
            }
            else
              return CANCEL;
          }
        }
      }
      return CANCEL;
      break;
    }

    case CLOSE:
    {
      if (spi != 0)
      {
        delete spi->iter;
        spi->iter = 0;
        delete spi->resTuples;
        spi->resTuples = 0;
        delete spi;
        spi = 0;
        local.setAddr(0);
      }
      return 0;
      break;
    }

  }
  return 0;
}

const string OShortestPathASpec =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>(orel(tuple(a1:t1 ... an:tn))(ai1 ai2 ... ain)) x ti1 x ti2 x "
  " int x (tuple->real) -> "
  "(stream (tuple(a1:t1 ... an:tn SeqNo: int)))</text--->"
  "<text>_ oshortestpatha [int,int, int;fun, fun]</text--->"
  "<text>The operator uses a specialised ordered relation as graph"
  " representation for shortest path computing. Each tuple represents an edge"
  " of the graph consisting of at least two integer attributes identifying"
  " the start node and the end node of the edge, and not less than two real"
  " attributes representing the costs of the edge, and the distance function"
  " to the target. The tuples may"
  " contain further attributes giving additional cost informations or arbitrary"
  " informations for each edge. The ordered relation should be ordered by the"
  " numbers of the starting nodes at the first and by the number of the end "
  " nodes at the second level to reduce the number of page accesses."
  " The first two parameters select the start and the end node. The third "
  " parameter selects the return value. In case of 0 the operator"
  " returns the shortest path from node ti1 to ti2 as a stream of tuples"
  " where each tuple is extended by a sequence number telling which is the"
  " place of the tuple in the shortest path. In case of 1 the operator returns"
  " the stream of tuples which are in the priority queue at the end of the"
  " operation extended by their number in the queue. In case of 2 the operator"
  " returns the stream of tuples which have been touched by the operator. In"
  " case of 3 the operator returns a stream of tuples giving the shortest path"
  " tree from the start node (up to the time the target node is reached)."
  " The fourth and fifth parameter define two cost functions mapping the tuple"
  " values to real numbers describing the costs of the edge, and the costs from"
  " the end of the edge to the target node. Uses A Star Algorithm.</text--->"
  "<text>query otest oshortestpatha[1,1,0;.Cost,.Weight] consume</text--->) )";

Operator oshortestpatha (
                "oshortestpatha",               // name
                OShortestPathASpec,             // specification
                OShortestPathAValueMap,  // value mapping
                Operator::SimpleSelect, // trivial selection function
                OShortestPathATypeMap    // type mapping
                );


/*
5. Type constructors
5.1 orel

*/
ListExpr ORelProperty() {
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"((\"Hagen\" 193)(\"Solingen\" 163))");
  ListExpr listrep = nl->TextAtom();
  nl->AppendText(listrep, "(<tuple>*) where <tuple> is "
  "(<attr1> <attr2> <attr3> .."
  ". <attrn>)");
  return nl->TwoElemList(
      nl->FourElemList( nl->StringAtom("Signature"),
                        nl->StringAtom("ExampleTypeList"),
                        nl->StringAtom("List Rep"),
                        nl->StringAtom("Example List")),
      nl->FourElemList( nl->StringAtom("TUPLE -> REL"),
                        nl->StringAtom("("+OREL+" (tuple "
                                         "((city string)(pop int))) city)"),
                        listrep,
                        examplelist));
}



TypeConstructor cpporel( OREL, ORelProperty,
                          OrderedRelation::Out, OrderedRelation::In,
                          OrderedRelation::Out, OrderedRelation::In,
                          OrderedRelation::Create, OrderedRelation::Delete,
                          OrderedRelation::Open, OrderedRelation::Save,
                          OrderedRelation::Close, OrderedRelation::Clone,
                          OrderedRelation::Cast, OrderedRelation::SizeOf,
                          OrderedRelation::CheckKind);


/*
5.2 compkey
The implementation of CompositeKey is found in CompositeKey.h/cpp

*/
ListExpr CompKeyProperty() {
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"no listrepresentation");
  ListExpr listrep = nl->TextAtom();
  nl->AppendText(listrep, "no listrepresentation");
  return nl->TwoElemList(
      nl->FourElemList( nl->StringAtom("Signature"),
                        nl->StringAtom("ExampleTypeList"),
                        nl->StringAtom("List Rep"),
                        nl->StringAtom("Example List")),
      nl->FourElemList( nl->StringAtom("-> DATA"),
                        nl->StringAtom("compkey"),
                        listrep,
                        examplelist));
}



TypeConstructor cppcompkey( "compkey", CompKeyProperty,
                          CompositeKey::Out, CompositeKey::In,
                          0, 0,
                          CompositeKey::Create, CompositeKey::Delete,
                          CompositeKey::Open, CompositeKey::Save,
                          CompositeKey::Close, CompositeKey::Clone,
                          CompositeKey::Cast, CompositeKey::SizeOf,
                          CompositeKey::CheckKind);



/*
6. OrderedRelationAlgebra definition

*/
class OrderedRelationAlgebra : public Algebra {
  public:
    OrderedRelationAlgebra() : Algebra() {
      AddTypeConstructor(&cpporel);
      cpporel.AssociateKind(Kind::REL());

      AddTypeConstructor(&cppcompkey);
      cppcompkey.AssociateKind(Kind::DATA());

      AddOperator(&oleftrange);
      AddOperator(&orightrange);
      AddOperator(&orange);
      AddOperator(&oshortestpathd);
      AddOperator(&oshortestpatha);
      #ifdef USE_PROGRESS
      oleftrange.EnableProgress();
      orightrange.EnableProgress();
      orange.EnableProgress();
      #endif
    };

    ~OrderedRelationAlgebra() {};
};


/*
7. Registration of the OrderedRelationAlgebra

*/
extern "C" Algebra* InitializeOrderedRelationAlgebra(NestedList* nlRef,
                                                  QueryProcessor* qpRef) {
  nl = nlRef;
  qp = qpRef;
  am = SecondoSystem::GetAlgebraManager();
  return new OrderedRelationAlgebra();
}


