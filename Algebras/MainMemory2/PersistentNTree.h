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

*/

#ifndef PERSISTENTNTREE_H
#define PERSISTENTNTREE_H

#include "NTree.h"

namespace mm2algebra {
  
template<class T, class DistComp, int variant>
class PersistentNTree {
 public:
  typedef NTreeLeafNode<MTreeEntry<T>, DistComp, variant> leafnode_t;
  typedef NTreeNode<MTreeEntry<T>, DistComp, variant> node_t;
  typedef NTree<MTreeEntry<T>, DistComp, variant> ntree_t;
  typedef NTreeInnerNode<MTreeEntry<T>, DistComp, variant> innernode_t;

  // This constructor is applied for ~exportntree~
  PersistentNTree(ntree_t* n, std::vector<Tuple*>* tuples,
             ListExpr relTypeList, std::string& prefix, const int firstId,
             const int suffix) :
          status(false), treeInfoType(0), nodeInfoType(0), nodeDistType(0), 
          pivotInfoType(0), firstNodeId(firstId), nodeInfoPos(0),
          nodeDistPos(0), pivotInfoPos(0), srcTuples(tuples), nodeInfoTuples(0),
          nodeDistTuples(0), pivotInfoTuples(0), ntree(n) {
    sc = SecondoSystem::GetCatalog();
    std::vector<std::string> relNames = getRelNames(prefix, suffix);
    if (tuples->empty()) {
      return;
    }
    if (!createTypeLists(relTypeList)) {
      return;
    }
    if (!initRelations(relNames)) {
      return;
    }
    if (!processNTree(ntree, relTypeList)) {
      return;
    }
    if (!storeRelation(treeInfoTypeList, treeInfoRel, relNames[0])) {
      return;
    }
    if (!storeRelation(nodeInfoTypeList, nodeInfoRel, relNames[1])) {
      return;
    }
    if (!storeRelation(nodeDistTypeList, nodeDistRel, relNames[2])) {
      return;
    }
    if (!storeRelation(pivotInfoTypeList, pivotInfoRel, relNames[3])) {
      return;
    }
    status = true;
  }
  
  // This constructor is applied for ~importntree~
  PersistentNTree(std::string& prefix, const int suffix) : status(false), 
       treeInfoType(0), nodeInfoType(0), nodeDistType(0), pivotInfoType(0), 
       nodeInfoPos(0), nodeDistPos(0), pivotInfoPos(0), srcTuples(0), ntree(0) {
    sc = SecondoSystem::GetCatalog();
    std::vector<std::string> relNames = getRelNames(prefix, suffix);
    std::string nodeInfoRelName = prefix + "NodeInfo";
    ListExpr srcRelTypeList = getNodeInfoRelTypeList(nodeInfoRelName);
    if (!createTypeLists(srcRelTypeList)) {
      return;
    }
    if (!checkRelationType(relNames[0], prefix, treeInfoTypeList)) {
      return;
    }
    if (!checkRelationType(relNames[1], prefix, srcRelTypeList)) {
      return;
    }
    if (!checkRelationType(relNames[2], prefix, nodeDistTypeList)) {
      return;
    }
    if (!checkRelationType(relNames[3], prefix, pivotInfoTypeList)) {
      return;
    }
    if (!initNTree()) {
      return;
    }
    if (!buildNTree()) {
      return;
    }
    status = true;
  }
  
  ~PersistentNTree() {
    if (treeInfoType != 0) {
      treeInfoType->DeleteIfAllowed();
    }
    if (nodeInfoType != 0) {
      nodeInfoType->DeleteIfAllowed();
    }
    if (nodeDistType != 0) {
      nodeDistType->DeleteIfAllowed();
    }
    if (pivotInfoType != 0) {
      pivotInfoType->DeleteIfAllowed();
    }
  }
  
  ListExpr getNodeInfoRelTypeList(std::string& relName) const {
    if (!sc->IsObjectName(relName)) {
      return false;
    }
    return nl->TwoElemList(nl->SymbolAtom(mm2algebra::Mem::BasicType()),
                           sc->GetObjectTypeExpr(relName));
  }
  
  std::vector<std::string> getRelNames(std::string& prefix, 
                                       const int suffix = -1) {
    std::string suffixstr = (suffix == -1 ? "" : "_" + std::to_string(suffix));
    std::vector<std::string> result{prefix + "TreeInfo" + suffixstr,
               prefix + "NodeInfo" + suffixstr, prefix + "NodeDist" + suffixstr,
               prefix + "PivotInfo" + suffixstr};
    return result;
  }
  
  bool checkRelationType(std::string& relName, std::string& prefix,
                         ListExpr relType) {
    Word relWord;
    bool defined;
    if (!sc->IsObjectName(relName)) {
      cout << "relation " << relName << " does not exist" << endl;
      return false;
    }
    if (!sc->GetObject(relName, relWord, defined)) {
      cout << "relation " << relName << " could not be read" << endl;
      return false;
    }
    if (!defined) {
      cout << "relation " << relName << " undefined" << endl;
      return false;
    }
    if (relName.find("TreeInfo", prefix.size()) != std::string::npos) {
      if (!nl->Equal(relType, treeInfoTypeList)) {
        cout << "relation " << relName << " has wrong type" << endl;
        return false;
      }
      treeInfoRel = (Relation*)(relWord.addr);
    }
    if (relName.find("NodeInfo", prefix.size()) != std::string::npos) {
      ListExpr attrList1 = nl->Second(nl->Second(nl->Second(relType)));
      ListExpr attrList2 = nl->Second(nodeInfoTypeList);
      int listLength = nl->ListLength(attrList1);
      for (int i = 1; i <= listLength; i++) {
        if (!nl->Equal(nl->Nth(i, attrList1), nl->Nth(i, attrList2))) {
          cout << "relation " << relName << " has wrong type" << endl;
          return false;
        }
      }
      firstAttrNo -= 4;
      nodeInfoRel = (Relation*)(relWord.addr);
      int nodeInfoRelSize = nodeInfoRel->GetNoTuples();
      nodeInfoTuples = new std::vector<Tuple*>(nodeInfoRelSize);
      for (int i = 1; i <= nodeInfoRelSize; i++) {
        (*nodeInfoTuples)[i - 1] = nodeInfoRel->GetTuple(i, false);
      }
    }
    if (relName.find("NodeDist", prefix.size()) != std::string::npos) {
      if (!nl->Equal(relType, nodeDistTypeList)) {
        cout << "relation " << relName << " has wrong type" << endl;
        return false;
      }
      nodeDistRel = (Relation*)(relWord.addr);
      int nodeDistRelSize = nodeDistRel->GetNoTuples();
      nodeDistTuples = new std::vector<Tuple*>(nodeDistRelSize);
      for (int i = 1; i <= nodeDistRelSize; i++) {
        (*nodeDistTuples)[i - 1] = nodeDistRel->GetTuple(i, false);
      }
    }
    if (relName.find("PivotInfo", prefix.size()) != std::string::npos) {
      if (!nl->Equal(relType, pivotInfoTypeList)) {
        cout << "relation " << relName << " has wrong type" << endl;
        return false;
      }
      pivotInfoRel = (Relation*)(relWord.addr);
      int pivotInfoRelSize = pivotInfoRel->GetNoTuples();
      pivotInfoTuples = new std::vector<Tuple*>(pivotInfoRelSize);
      for (int i = 1; i <= pivotInfoRelSize; i++) {
        (*pivotInfoTuples)[i - 1] = pivotInfoRel->GetTuple(i, false);
      }
    }
    return true;
  }
  
  bool initNTree() {
    Tuple *treeInfoTuple = treeInfoRel->GetTuple(1, false);
    int degree = ((CcInt*)(treeInfoTuple->GetAttribute(1)))->GetValue();
    int maxLeafSize = ((CcInt*)(treeInfoTuple->GetAttribute(2)))->GetValue();
    Geoid *geoid = (Geoid*)(treeInfoTuple->GetAttribute(5));
    DistComp dc(geoid);
    attrNo = ((CcInt*)(treeInfoTuple->GetAttribute(4)))->GetValue();
    PartitionMethod pMethod = (variant == 8 ? RANDOMOPT : RANDOMONLY);
    ntree = new ntree_t(degree, maxLeafSize, dc, pMethod, attrNo);
    delete treeInfoTuple;
    return true;
  }
  
  node_t* buildNextNode() {
    Tuple *nodeInfoTuple = (*nodeInfoTuples)[nodeInfoPos];
    int nodeId = 
               ((CcInt*)(nodeInfoTuple->GetAttribute(firstAttrNo)))->GetValue();
    int entry(-1), attr0(firstAttrNo), currentNodeId(nodeId);
    std::vector<double> maxDist;
    std::vector<int> subnodeIds, entries;
    std::vector<MTreeEntry<T>* > objects;
    int subnodeId =
                   ((CcInt*)nodeInfoTuple->GetAttribute(attr0 + 2))->GetValue();
    bool isLeaf = (subnodeId == 0);
    node_t *result;
    if (isLeaf) {
      result = new leafnode_t(ntree->getDegree(), ntree->getMaxLeafSize(),
                              ntree->getCandOrder(), ntree->getPruningMethod());
    }
    else {
      result = new innernode_t(ntree->getDegree(), ntree->getMaxLeafSize(),
                              ntree->getCandOrder(), ntree->getPruningMethod());
    }
    cout << "process " << (isLeaf ? "LEAF node #" : "INNER node #") << nodeId 
         << endl;
//     std::queue<int> subnodeIds;
    while (currentNodeId == nodeId) {
      entry = ((CcInt*)nodeInfoTuple->GetAttribute(attr0 + 1))->GetValue();
      cout << "begin iteration, entry = " << entry << endl;
      entries.push_back(entry);
      if (isLeaf) {
        if (entry == 0) {
          maxDist.push_back( 
                 ((CcReal*)nodeInfoTuple->GetAttribute(attr0 + 3))->GetValue());
        }
      }
      else { // inner node
        subnodeId =
                   ((CcInt*)nodeInfoTuple->GetAttribute(attr0 + 2))->GetValue();
        subnodeIds.push_back(subnodeId);
        maxDist.push_back( 
                 ((CcReal*)nodeInfoTuple->GetAttribute(attr0 + 3))->GetValue());
      }
      T *obj = (T*)((T*)(nodeInfoTuple->GetAttribute(attrNo))->Clone());
      objects.push_back(new MTreeEntry<T>(*obj, nodeInfoTuple->GetTupleId()));
      nodeInfoPos++;
      nodeInfoTuple = (*nodeInfoTuples)[nodeInfoPos];
      currentNodeId= ((CcInt*)(nodeInfoTuple->GetAttribute(attr0)))->GetValue();
    }
    result->setCenters(nodeId, entries, objects, maxDist);
    Tuple *nodeDistTuple = (*nodeDistTuples)[nodeDistPos];
    currentNodeId = ((CcInt*)(nodeDistTuple->GetAttribute(0)))->GetValue();
    int entry1, entry2, distMatrixSize(0);
    double dist;
    std::vector<std::tuple<int, int, double> > distEntries;
    while (currentNodeId == nodeId) {
      entry1 = ((CcInt*)nodeDistTuple->GetAttribute(1))->GetValue();
      if (entry1 + 1 >= distMatrixSize) {
        distMatrixSize = entry1 + 1;
      }
      entry2 = ((CcInt*)nodeDistTuple->GetAttribute(2))->GetValue();
      dist = ((CcReal*)nodeDistTuple->GetAttribute(3))->GetValue();
      distEntries.push_back(std::make_tuple(entry1, entry2, dist));
      nodeDistPos++;
      nodeDistTuple = (*nodeDistTuples)[nodeDistPos];
      currentNodeId = ((CcInt*)(nodeDistTuple->GetAttribute(0)))->GetValue();
    }
    double** distMatrix = new double*[distMatrixSize];
    for (int i = 0; i < distMatrixSize; i++) {
      distMatrix[i] = new double[distMatrixSize];
      std::fill_n(distMatrix[i], distMatrixSize, -1.0);
      distMatrix[i][i] = 0.0;
    }
    for (unsigned int i = 0; i < distEntries.size(); i++) {
      auto distEntry = distEntries[i];
      distMatrix[std::get<0>(distEntry)][std::get<1>(distEntry)] = 
                                                         std::get<2>(distEntry);
    }
    result->setDistMatrix(distMatrix);
    Tuple *pivotInfoTuple = (*pivotInfoTuples)[pivotInfoPos];
    currentNodeId = ((CcInt*)(pivotInfoTuple->GetAttribute(0)))->GetValue();
    std::vector<std::tuple<int, double, double> > pivotEntries;
    double dist1, dist2;
    bool isPivot;
    std::vector<int> refDistPos;
    while (currentNodeId == nodeId) {
      entry = ((CcInt*)pivotInfoTuple->GetAttribute(1))->GetValue();
      dist1 = ((CcReal*)pivotInfoTuple->GetAttribute(2))->GetValue();
      dist2 = ((CcReal*)pivotInfoTuple->GetAttribute(3))->GetValue();
      isPivot = ((CcBool*)pivotInfoTuple->GetAttribute(4))->GetValue();
      if (isPivot) {
        refDistPos.push_back(entry);
      }
      pivotEntries.push_back(std::make_tuple(entry, dist1, dist2));
      pivotInfoPos++;
      pivotInfoTuple = (*pivotInfoTuples)[pivotInfoPos];
      currentNodeId = ((CcInt*)(pivotInfoTuple->GetAttribute(0)))->GetValue();
    }
    std::pair<double, double>* distances2d =
                             new std::pair<double, double>[pivotEntries.size()];
    for (unsigned int i = 0; i < pivotEntries.size(); i++) {
      auto pivotEntry = pivotEntries[i];
      distances2d[std::get<0>(pivotEntry)] = 
               std::make_pair(std::get<1>(pivotEntry), std::get<2>(pivotEntry));
    }
    result->setPivotInfo(refDistPos, distances2d);
    return result;
  }
  
  bool buildNTree() {
    if (nodeInfoRel->GetNoTuples() == 0) {
      return false;
    }
    node_t* root = buildNextNode();
    ntree->setRoot((innernode_t*)root);
    ntree->getRoot()->print(cout, true, ntree->getDistComp());
    cout << *((ntree->getRoot()->getCenter(0))->getKey()) << endl;
    return true;
  }
  
  bool createTypeLists(ListExpr relTypeList) {
    treeInfoTypeList = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
      nl->SixElemList(nl->TwoElemList(nl->SymbolAtom("Variant"),
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Degree"), 
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("MaxLeafSize"),
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("RelType"), 
                                      nl->SymbolAtom(FText::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("AttrNo"), 
                                      nl->SymbolAtom(CcInt::BasicType())),
                      nl->TwoElemList(nl->SymbolAtom("Geoid"),
                                      nl->SymbolAtom(Geoid::BasicType()))));
    ListExpr numTreeInfoTypeList = sc->NumericType(treeInfoTypeList);
    treeInfoType = new TupleType(numTreeInfoTypeList);
    firstAttrNo = nl->ListLength(nl->Second(nl->Second(nl->Second(
                                                                relTypeList))));
    ListExpr curList = nl->OneElemList(nl->First(nl->Second(nl->Second(
                                                    nl->Second(relTypeList)))));
    ListExpr restAttrList = nl->Rest(nl->Second(nl->Second(nl->Second(
                                                                relTypeList))));
    ListExpr oneAttrList = nl->First(restAttrList);
    ListExpr curList2 = curList;
    while (!nl->IsEmpty(restAttrList)) {
      oneAttrList = nl->First(restAttrList);
      restAttrList = nl->Rest(restAttrList);
      curList2 = nl->Append(curList2, oneAttrList);
    }
    curList2 = nl->Append(curList2, nl->TwoElemList(nl->SymbolAtom("NodeId"),
                                           nl->SymbolAtom(CcInt::BasicType())));
    curList2 = nl->Append(curList2, nl->TwoElemList(nl->SymbolAtom("Entry"),
                                           nl->SymbolAtom(CcInt::BasicType())));
    curList2 = nl->Append(curList2, nl->TwoElemList(nl->SymbolAtom("Subtree"),
                                           nl->SymbolAtom(CcInt::BasicType())));
    curList2 = nl->Append(curList2, nl->TwoElemList(nl->SymbolAtom("MaxDist"),
                                          nl->SymbolAtom(CcReal::BasicType())));
    nodeInfoTypeList = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
                                       curList);
    ListExpr numNodeInfoTypeList = sc->NumericType(nodeInfoTypeList);
    nodeInfoType = new TupleType(numNodeInfoTypeList);
    nodeDistTypeList = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
      nl->FourElemList(nl->TwoElemList(nl->SymbolAtom("NodeId"), 
                                       nl->SymbolAtom(CcInt::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("Entry1"),
                                       nl->SymbolAtom(CcInt::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("Entry2"),
                                       nl->SymbolAtom(CcInt::BasicType())),
                       nl->TwoElemList(nl->SymbolAtom("Distance"),
                                       nl->SymbolAtom(CcReal::BasicType()))));
    ListExpr numNodeDistTypeList = sc->NumericType(nodeDistTypeList);
    nodeDistType = new TupleType(numNodeDistTypeList);
    pivotInfoTypeList = nl->TwoElemList(nl->SymbolAtom(Tuple::BasicType()),
       nl->FiveElemList(nl->TwoElemList(nl->SymbolAtom("NodeId"),
                                        nl->SymbolAtom(CcInt::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("Entry"), 
                                        nl->SymbolAtom(CcInt::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("PivotDist1"),
                                        nl->SymbolAtom(CcReal::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("PivotDist2"),
                                        nl->SymbolAtom(CcReal::BasicType())),
                        nl->TwoElemList(nl->SymbolAtom("IsPivot"),
                                        nl->SymbolAtom(CcBool::BasicType()))));
    ListExpr numPivotInfoTypeList = sc->NumericType(pivotInfoTypeList);
    pivotInfoType = new TupleType(numPivotInfoTypeList);
    return true;
  }
  
  bool initRelations(std::vector<std::string>& relNames) {
    for (unsigned int i = 0; i < relNames.size(); i++) {
      std::string errMsg;
      if (!sc->IsValidIdentifier(relNames[i], errMsg, true)) {
        if (sc->IsObjectName(relNames[i])) {
          if (!sc->DeleteObject(relNames[i])) {
            cout << "object " << relNames[i] << " could not be deleted" << endl;
            return false;
          }
          cout << "previous object \"" << relNames[i] << "\" deleted" << endl;
        }
      }
      if (sc->IsSystemObject(relNames[i])) {
        cout << relNames[i] << " is a reserved name" << endl;
        return false;
      }
    }
    treeInfoRel = new Relation(treeInfoType, false);
    nodeInfoRel = new Relation(nodeInfoType, false);
    nodeDistRel = new Relation(nodeDistType, false);
    pivotInfoRel = new Relation(pivotInfoType, false);
    return true;
  }
  
  bool processNTree(ntree_t* ntree, ListExpr relTypeList) {
    Tuple *treeInfoTuple = new Tuple(treeInfoType);
    treeInfoTuple->PutAttribute(0, new CcInt(true, ntree->getVariant()));
    treeInfoTuple->PutAttribute(1, new CcInt(true, ntree->getDegree()));
    treeInfoTuple->PutAttribute(2, new CcInt(true, ntree->getMaxLeafSize()));
    treeInfoTuple->PutAttribute(3, new FText(true, nl->ToString(relTypeList)));
    treeInfoTuple->PutAttribute(4, new CcInt(true, ntree->getAttrNo()));
    auto distComp = ntree->getDistComp();
    Geoid *geoid = distComp.getGeoid();
    Geoid *newGeoid = 0;
    if (geoid == 0) {
      newGeoid = new Geoid(false);
    }
    else if (geoid->IsDefined()) {
      newGeoid = new Geoid(*geoid);
    }
    else {
      newGeoid = new Geoid(false);
    }
    treeInfoTuple->PutAttribute(5, newGeoid);
    treeInfoRel->AppendTuple(treeInfoTuple);
    treeInfoTuple->DeleteIfAllowed();
    processNode(ntree->getRoot());
    return true;
  }
  
  void processNode(node_t* node) {
    int nodeId = node->getNodeId() + firstNodeId;
    Tuple *nodeInfoTuple(0), *nodeDistTuple(0), *pivotInfoTuple(0),*srcTuple(0);
    TupleId tid;
    std::tuple<int, int, int> refDistPos = node->getRefDistPos();
    for (int i = 0; i < node->getCount(); i++) {
      if (node->isLeaf()) {
        tid = ((leafnode_t*)node)->getObject(i)->getTid();
      }
      else { // inner node
        tid = ((innernode_t*)node)->getCenter(i)->getTid();
      }
      int subtreeNodeId = (node->isLeaf() ? 0 :
                  ((innernode_t*)node)->getChild(i)->getNodeId() + firstNodeId);
      double maxDist = (node->isLeaf() ? ((leafnode_t*)node)->getMaxDist() : 
                                         ((innernode_t*)node)->getMaxDist(i));
//       nodeInfo.push_back(std::make_tuple(node->getNodeId() + firstNodeId, i, 
//                                          subtreeNodeId, maxDist, tid - 1));
      nodeInfoTuple = new Tuple(nodeInfoType);
      srcTuple = (*srcTuples)[tid - 1];
      for (int j = 0; j < srcTuple->GetNoAttributes(); j++) {
        nodeInfoTuple->CopyAttribute(j, srcTuple, j);
      }
      nodeInfoTuple->PutAttribute(firstAttrNo, new CcInt(true, nodeId));
      nodeInfoTuple->PutAttribute(firstAttrNo + 1, new CcInt(true, i));
      nodeInfoTuple->PutAttribute(firstAttrNo + 2, new CcInt(true, 
                                                             subtreeNodeId));
      nodeInfoTuple->PutAttribute(firstAttrNo + 3, new CcReal(true, maxDist));
      nodeInfoRel->AppendTuple(nodeInfoTuple); 
      delete nodeInfoTuple;
      for (int j = 0; j < i; j++) {
//         nodeDist.push_back(std::make_tuple(node->getNodeId() + firstNodeId, 
//                       i, j, node->getPrecomputedDist(i, j, node->isLeaf())));
        nodeDistTuple = new Tuple(nodeDistType);
        nodeDistTuple->PutAttribute(0, new CcInt(true, nodeId));
        nodeDistTuple->PutAttribute(1, new CcInt(true, i));
        nodeDistTuple->PutAttribute(2, new CcInt(true, j));
        nodeDistTuple->PutAttribute(3, new CcReal(true, 
                               node->getPrecomputedDist(i, j, node->isLeaf())));
        nodeDistRel->AppendTuple(nodeDistTuple);
        nodeDistTuple->DeleteIfAllowed();
      }
      std::vector<double> pivotDists = node->getPivotDistances(i);
      bool isPivot = (i == std::get<0>(refDistPos) || 
                      i == std::get<1>(refDistPos));
//       pivotInfo.push_back(std::make_tuple(node->getNodeId() + firstNodeId, i,
//                                      pivotDists[0], pivotDists[1], isPivot));
      pivotInfoTuple = new Tuple(pivotInfoType);
      pivotInfoTuple->PutAttribute(0, new CcInt(true, nodeId));
      pivotInfoTuple->PutAttribute(1, new CcInt(true, i));
      pivotInfoTuple->PutAttribute(2, new CcReal(true, pivotDists[0]));
      pivotInfoTuple->PutAttribute(3, new CcReal(true, pivotDists[1]));
      pivotInfoTuple->PutAttribute(4, new CcBool(true, isPivot));
      pivotInfoRel->AppendTuple(pivotInfoTuple);
      pivotInfoTuple->DeleteIfAllowed();
    }
    if (!node->isLeaf()) {
      for (int i = 0; i < node->getCount(); i++) {
        processNode(((innernode_t*)node)->getChild(i));
      }
    }
  }
  
  bool storeRelation(ListExpr typeList, Relation *rel, std::string& relName) {
    ListExpr relType = nl->TwoElemList(nl->SymbolAtom(Relation::BasicType()), 
                                       typeList);
    Word relWord;
    relWord.setAddr(rel);
    sc->InsertObject(relName, "", relType, relWord, true);
    return true;
  }
  
  bool getStatus() const {
    return status;
  }
  
  ntree_t* getNTree() {
    return ntree;
  }
  
 private:
  bool status;
  SecondoCatalog* sc;
  ListExpr treeInfoTypeList, nodeInfoTypeList, nodeDistTypeList, 
           pivotInfoTypeList;
  TupleType *treeInfoType, *nodeInfoType, *nodeDistType, *pivotInfoType;
  Relation *treeInfoRel, *nodeInfoRel, *nodeDistRel, *pivotInfoRel;
  int attrNo, firstAttrNo, firstNodeId, nodeInfoPos, nodeDistPos, pivotInfoPos;
  std::vector<Tuple*> *srcTuples, *nodeInfoTuples, *nodeDistTuples, 
                      *pivotInfoTuples;
  ntree_t* ntree;
};

template <class T, class DistComp, int variant>
class MemoryNtreeObject : public MemoryObject {

 public:
  typedef std::pair<T, TupleId> treeentry_t;
  typedef NTree<MTreeEntry<T>, DistComp, variant> tree_t;

  MemoryNtreeObject(tree_t* _ntreeX, size_t _memSize, 
                    const std::string& _objectTypeExpr, bool _flob, 
                    const std::string& _database) {
    ntreeX = _ntreeX;
    memSize = _memSize;
    objectTypeExpr =_objectTypeExpr;
    flob = _flob;
    database = _database;
  };

  tree_t* getNtreeX() {
    return ntreeX;
  };

  static std::string BasicType() {
    return "ntree" + (variant > 1 ? std::to_string(variant) : "");
  }

  static bool checkType(ListExpr list) {
    if (!nl->HasLength(list, 2)) {
      return false;
    }
    if (!listutils::isSymbol(nl->First(list), BasicType())) {
      return false;
    }
    return T::checkType(nl->Second(list));
  }
    
  MemoryObject* clone() {
    return new MemoryNtreeObject<T, DistComp, variant>(ntreeX->clone(), 
                                       memSize, objectTypeExpr, flob, database);
  }


 private:
  tree_t* ntreeX;
  MemoryNtreeObject();
  
 protected:
  ~MemoryNtreeObject() {
    if (ntreeX) {
      delete ntreeX;
    }
  };
};
  
}

#endif
