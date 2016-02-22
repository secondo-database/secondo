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
 //[->] [$\rightarrow$]
 //[TOC] [\tableofcontents]
 //[_] [\_]

 BPTree-Class

*/

#include "BPTree.h"
#include <chrono>
#include <sstream>
#include <fstream>               // ofstream
#include <stdexcept>

#include "AlgebraManager.h"
#include "WinUnix.h"

#include "Cache/LruCache.h"
#include "Cache/NoCache.h"


using namespace std;
using fialgebra::cache::LruCache;
using fialgebra::cache::NoCache;

extern AlgebraManager *am;

//Implementation of B+ Tree for portable Index

namespace fialgebra {
BPTree* BPTree::Create(const char* fileName, unsigned int algebraId,
                       unsigned int typeId, size_t cacheSize){
    ofstream stream(fileName, ofstream::binary | ofstream::app);
    if (stream.good())
    {

      if (stream.tellp() == 0)
      {
        size_t valueSize = am->SizeOfObj(algebraId, typeId)();
        BPTreeHeader* header = new BPTreeHeader(WinUnix::getPageSize(),
                                                valueSize,
                                                algebraId,
                                                typeId,
                                                0, 0);//No freepage-list or root

        stream.write(header->GetBytes(), header->GetPageSize());
        stream.flush();
        stream.close();


        cache::CacheBase* cache = NULL;
        // cacheSize <= 0 : Cache is not used
        if ( cacheSize <= 0 )
            cache = new NoCache( fileName, header->GetPageSize(), 0 );
        else
            cache = new LruCache( fileName, header->GetPageSize(), cacheSize );

        return new BPTree( header, cache );
      }
      else
      {
        stream.close();
        throw runtime_error("File allready existed!");
      } // else
    }
    else
    {
      stream.close();
      throw runtime_error("File couldn't be created!");
    } // else
}

BPTree* BPTree::Open(const char * fileName, size_t cacheSize){
  ifstream stream(fileName, ifstream::binary | ifstream::ate);
  if (!stream.good())
  {
    stream.close();
    throw runtime_error( "File couldn't be opened! fileName: " +
        std::string( fileName ) );
  }
  else
  {
    size_t pageSize = WinUnix::getPageSize(),
           length = min<size_t>(pageSize, stream.tellg());

    stream.seekg(0, ios::beg);

    char* bytes = new char[length];
    stream.read(bytes, length);
    stream.close();

    BPTreeHeader* header = new BPTreeHeader(bytes, length, pageSize);

    if (header->GetMarker() != TreeHeaderMarker::BPlusTree){
      delete(header);
      throw runtime_error("File couldn't be opened! Not a valid B+-Tree!");
    } // of

    cache::CacheBase* cache = NULL;
    // cacheSize <= 0: Cache is not used
    if ( cacheSize <= 0 )
        cache = new NoCache( fileName, header->GetPageSize(), 0 );
    else
        cache = new LruCache( fileName, header->GetPageSize(), cacheSize );

    return new BPTree( header, cache );
  }
}

BPTree::~BPTree() {
  m_treeCache->Write( 0,
    (size_t)m_treeHeader->GetBytes(),
    m_treeHeader->GetHeaderSize() );

  delete m_treeCache;
  m_treeCache = NULL;

  delete m_treeHeader;
  m_treeHeader = NULL;

  //cout << "Search: " << search << "\nInsert: " << insert;
  //cout << "\nBalance: " << balance << "\nMoove: " << mooveLeft + mooveRight;
  //cout << "\nMooveLeft" << mooveLeft << "\nMooveRight: " << mooveRight;
  //cout << "\n\nTotal: " << total << '\n';
}

BPTree::BPTree(BPTreeHeader* header, cache::CacheBase* cache){
  m_treeHeader = header;
  m_treeCache = cache;

  m_valueCast = am->Cast(header->GetAlgebraID(), header->GetTypeID());
}

bool BPTree::DeleteValue(Attribute& value, TupleId tupleId) {
  stack<PathEntry>* path = GetDeletePath(value, tupleId);

  if (!path->empty()) {
    BPTreeNode* node = path->top().node;
    size_t index = path->top().index;
    path->pop();

    node->RemoveIDAt(index);
    node->RemoveValueAt(index);

    while (node->GetValueCount() < node->GetMaxValueCount() / 2) {
      if (path->empty()){
        if (node->GetValueCount() == 0){
          m_treeHeader->SetRoot(node->IsLeaf() ? 0 : node->GetIDAt(0));

          DeleteNode(*node);
        }

        break;
      }
      else{
        BPTreeNode* parentNode = path->top().node;
        size_t nodeIndex = path->top().index;
        pair<BPTreeNode*, BPTreeNode*> siblings = GetBrothers(*node,
                                                              *parentNode,
                                                              nodeIndex);

        //balance, if possible
        if (BalanceNode(node, siblings, parentNode, nodeIndex)){
          WriteNode(*parentNode);
          parentNode = NULL;
        }
        else{
          MergeNode(*node, siblings, *parentNode, nodeIndex);

          DeleteNode(*node);
          WriteNode(*node);
          delete(node);
          node = parentNode;
          parentNode = NULL;

          path->pop();
        }

        if(siblings.first != NULL){
          WriteNode(*siblings.first);
          delete(siblings.first);
          siblings.first = NULL;
        }

        if(siblings.second != NULL){
          WriteNode(*siblings.second);
          delete(siblings.second);
          siblings.second = NULL;
        }
      }
    }

    // Save and delete
    WriteNode(*node);
    delete(node);
    node = NULL;

    while(!path->empty()){
      delete(path->top().node);
      path->pop();
    }

    delete(path);

    return true;
  }

  delete(path);

  return false;
}

void BPTree::Rebuild(const char* filename) {
  BPTreeHeader& header1 = this->GetHeader();
  // Remove target file if it exists
  fstream file2(filename);
  remove(filename);
  // Create target tree
  BPTree* bpt2 = Create(filename, header1.GetAlgebraID(),
                                header1.GetTypeID(), 50);
  BPTreeHeader& header2 = bpt2->GetHeader();
  // Copy header data
  header2.SetPageSize(header1.GetPageSize());
  // Root node will be written to page 1
  header2.SetRoot(1);
  // Pointer to list of empty pages is null pointer
  header2.SetEmptyPage(0);
  // First, copy root node to target file
  BPTreeNode* yy1 = this->GetNodefromPageNumber(GetHeader().GetRoot());
  yy1->SetPageNumber(1);
  size_t page = bpt2->m_treeCache->NewPage();
  bpt2->WriteNode(*yy1);
  delete yy1;
  // Second, copy the other nodes to target file
  unsigned int numbNodesToRead = 1;
  size_t pageReadNext = 1;
  unsigned int counter1 = 0;
  size_t yy3 = 0;

  BPTreeNode* yy2;
  while ((bpt2->GetNodefromPageNumber(pageReadNext))->IsLeaf() == false) {
    // for each node already written to target file fetch children
    // from source file and write them to target file; if leaf level
    // reached, end since no children exist.
    for (unsigned int i = 0; i < numbNodesToRead; i++) {
      // fetch next node which is already written to target file
      yy2 = bpt2->GetNodefromPageNumber(pageReadNext);
      yy3 = yy2->GetValueCount() + 1;
      // fetch node's child nodes from source file
      for (unsigned int k = 0; k < yy3; k++) {
        yy1 = this->GetNodefromPageNumber(yy2->GetIDAt(k));
        page = bpt2->m_treeCache->NewPage();
        // if child node is leaf, set PrevLeaf and NextLeaf
        if (yy1->IsLeaf() == true) {
          if (yy1->GetPrevLeaf() !=0 ) {
            yy1->SetPrevLeaf( page - 1);
          }
          if (yy1->GetNextLeaf() !=0 ) {
            yy1->SetNextLeaf( page + 1);
          }
        }
        // set child node's new page number
        yy1->SetPageNumber(page);
        // write child node to target file
        bpt2->WriteNode(*yy1);
        // Set child node's ID to new page number in node's
        // IDs array.
        yy2->SetIDAt(k, page);
        delete yy1;
        counter1++;
      } // for (unsigned int k = 0; k < yy3; k++)
      // write changed node back to target file
      bpt2->WriteNode(*yy2);
      delete yy2;
      pageReadNext++;
    } // for (unsigned int i = 0; i < numbNodesToRead; i++)
    // End of copying leaves
    numbNodesToRead = counter1;
    counter1 = 0;
  } // while ((bpt2->GetNodefromPageNumber(pageReadNext))->IsLeaf() == false)
  // End of coyping source tree, tree in target file is ordered
  // show source and target tree
  //cout << this->ToString() << '\n';
  //cout << bpt2->ToString() << '\n';
  delete bpt2;
}

stack<BPTree::PathEntry>* BPTree::GetInsertPath(const Attribute& attribute) {
  stack<PathEntry>* path = new stack<PathEntry>();

  unsigned long root = m_treeHeader->GetRoot();
  BPTreeNode* node = root != 0 ? GetNodefromPageNumber(root) : NULL;

  while (node != NULL){
    size_t index = LookupInsertIndex(*node, attribute);
    path->push(PathEntry(node, index));

    if (!node->IsLeaf()){
      node = GetNodefromPageNumber(node->GetIDAt(index));
    }
    else{
      node = NULL;
    }
  }

  return path;
}
stack<BPTree::PathEntry>* BPTree::GetDeletePath(const Attribute& attribute,
                                                TupleId tupleId){
  stack<PathEntry>* path = GetInsertPath(attribute);

  while (!path->empty()){
    BPTreeNode* node = path->top().node;

    if (path->top().index == node->GetValueCount()){
      path->top().index--;
    }

    for ( size_t index = path->top().index + 1; index > 0; index-- ) {
      if ( node->GetIDAt( index - 1 ) == tupleId ) {
        path->top().index = index - 1;
        return path;
      } // if
    } // for

    //Try to search in prev leafs
    size_t prevLeaf = node->GetPrevLeaf();
    if (prevLeaf != 0 && node->GetValueAt(0).Compare(&attribute) >= 0){
      //Fix the path
      do{
        path->pop();

        delete(node);
        node = path->top().node;
      }
      while(path->top().index == 0);

      path->top().index--;

      node = GetNodefromPageNumber(node->GetIDAt(path->top().index));

      path->push(PathEntry(node, node->GetValueCount()));

      while(!node->IsLeaf()){
        node = GetNodefromPageNumber(node->GetIDAt(node->GetValueCount()));
        path->push(PathEntry(node, node->GetValueCount()));
      }
    }
    else{
      node = NULL;

      while(!path->empty()){
        delete(path->top().node);
        path->pop();
      }

      *path = stack<PathEntry>();
    }

  }

  return path;
}

void BPTree::InsertValue(Attribute& value, TupleId tupleId) {
  //look, if root exists
  //auto start = std::chrono::steady_clock::now();

  if (m_treeHeader->GetRoot() == 0) {
    BPTreeNode* rootNode = CreateNode(true);

    rootNode->InsertValue(value, tupleId, 0);

    // update the header with the actual page number
    m_treeHeader->SetRoot(rootNode->GetPageNumber());

    // Write and delete
    WriteNode(*rootNode);
    delete(rootNode);
    rootNode = NULL;
  }
  else{
    //find the leaf to insert
    //auto start2 = std::chrono::steady_clock::now();
    stack<PathEntry>* path = GetInsertPath(value);
    //auto end2 = std::chrono::steady_clock::now();
    //search += (end2 - start2).count();

    //auto start0 = std::chrono::steady_clock::now();
    BPTreeNode* targetNode = path->top().node;

    targetNode->InsertValue(value, tupleId, path->top().index);
    path->pop();

    //auto end0 = std::chrono::steady_clock::now();
    //insert += (end0 - start0).count();

    //auto start1 = std::chrono::steady_clock::now();

    //node overflow?
    while (targetNode->GetValueCount() > targetNode->GetMaxValueCount()){
      BPTreeNode* parentNode = NULL;

      if (!path->empty()) {
        parentNode = path->top().node;
        size_t nodeIndex = path->top().index;

        //get brothers
        pair<BPTreeNode*, BPTreeNode*> siblings = GetBrothers(*targetNode,
                                                              *parentNode,
                                                              nodeIndex);

        //balance, if possible
        if (BalanceNode(targetNode, siblings, parentNode, nodeIndex)){
          if(siblings.first != NULL){
            WriteNode(*siblings.first);
          }

          if(siblings.second != NULL){
            WriteNode(*siblings.second);
          }

          WriteNode(*parentNode);
          parentNode = NULL;
        }
        else{
          SplitNode(*targetNode, *parentNode, nodeIndex);

          WriteNode(*targetNode);
          delete(targetNode);
          targetNode = parentNode;
          parentNode = NULL;

          path->pop();
        }

        if(siblings.first != NULL){
          delete(siblings.first);
          siblings.first = NULL;
        }

        if(siblings.second != NULL){
          delete(siblings.second);
          siblings.second = NULL;
        }
      }
      //root treatment
      else {
        //split targetNode and generate new root
        parentNode = CreateNode(false);
        parentNode->SetIDAt(0, targetNode->GetPageNumber());

        m_treeHeader->SetRoot(parentNode->GetPageNumber());

        SplitNode(*targetNode, *parentNode, 0);

        WriteNode(*parentNode);
        delete(parentNode);
        parentNode = NULL;
      }
    }

    //auto end1 = std::chrono::steady_clock::now();
    //balance += (end1 - start1).count();

    // overflow is fixed, task completed
    WriteNode(*targetNode);
    delete(targetNode);
    targetNode = NULL;

    while(!path->empty()){
      delete(path->top().node);
      path->pop();
    }

    delete(path);
  }

  //auto end = std::chrono::steady_clock::now();
  //total += (end - start).count();
}

pair<BPTreeNode*, BPTreeNode*> BPTree::GetBrothers(BPTreeNode& node,
                                                   BPTreeNode& parent,
                                                   size_t nodeIndex) {
  BPTreeNode* leftSibling = NULL,
            * rightSibling = NULL;

  if (nodeIndex > 0){
    leftSibling = GetNodefromPageNumber(parent.GetIDAt(nodeIndex - 1));
  }

  if (nodeIndex < parent.GetValueCount()){
    rightSibling = GetNodefromPageNumber(parent.GetIDAt(nodeIndex + 1));
  }

  return pair<BPTreeNode*, BPTreeNode*>(leftSibling, rightSibling);
}

void BPTree::SplitNode(BPTreeNode& node, BPTreeNode& parentNode,
                       size_t nodeIndex) {
  BPTreeNode* newNode = CreateNode(node.IsLeaf());

  parentNode.InsertValue(node.GetValueAt(0), newNode->GetPageNumber(),
                         nodeIndex);

  size_t newNodeValueCount = newNode->GetValueCount();

  if (!node.IsLeaf()){
    newNode->SetIDAt(newNodeValueCount, node.GetIDAt(0));

    node.RemoveIDAt(0);
    node.RemoveValueAt(0);
  }
  else{
    newNode->SetPrevLeaf(node.GetPrevLeaf());
    newNode->SetNextLeaf(node.GetPageNumber());
    node.SetPrevLeaf(newNode->GetPageNumber());

    if (newNode->GetPrevLeaf() != 0){
      BPTreeNode* previousLeaf = GetNodefromPageNumber(newNode->GetPrevLeaf());
      previousLeaf->SetNextLeaf(newNode->GetPageNumber());

      WriteNode(*previousLeaf);
      delete(previousLeaf);
      previousLeaf = NULL;
    }
  }

  //move half of the entries to the left node
  MoveEntriesToLeft(node, *newNode, parentNode, nodeIndex + 1,
                    (node.GetValueCount() - newNodeValueCount + 1) / 2);

  WriteNode(*newNode);
  delete(newNode);
  newNode = NULL;
}

bool BPTree::MergeNode(BPTreeNode& node,
                       pair<BPTreeNode*, BPTreeNode*>& siblings,
                       BPTreeNode& parentNode, size_t nodeIndex){
  //If possible, merge with left sibling
  if (siblings.first != NULL && siblings.first->GetMaxValueCount() -
      siblings.first->GetValueCount() >= node.GetValueCount()){
    BPTreeNode& leftNode = *siblings.first;
    //Let's move all values and ids to leftNode
    MoveEntriesToLeft(node, leftNode, parentNode, nodeIndex,
                      node.GetValueCount());

    if (!node.IsLeaf()){
      //We are a inner node, so we have one more id to go
      leftNode.InsertValue(parentNode.GetValueAt(nodeIndex - 1),
                           leftNode.GetValueCount());
      leftNode.SetIDAt(leftNode.GetValueCount(), node.GetIDAt(0));
    }
    else if (siblings.second != NULL){
      leftNode.SetNextLeaf(siblings.second->GetPageNumber());
      siblings.second->SetPrevLeaf(leftNode.GetPageNumber());
    }
    else{
      leftNode.SetNextLeaf(node.GetNextLeaf());

      if (leftNode.GetNextLeaf() != 0){
        BPTreeNode* nextNode = GetNodefromPageNumber(leftNode.GetNextLeaf());
        nextNode->SetPrevLeaf(leftNode.GetPageNumber());

        WriteNode(*nextNode);
        delete(nextNode);
        nextNode = NULL;
      }
    }

    parentNode.RemoveIDAt(nodeIndex);
    parentNode.RemoveValueAt(nodeIndex - 1);
    return true;
  }
  //If possible, merge with right sibling
  else if (siblings.second != NULL && siblings.second->GetMaxValueCount() -
           siblings.second->GetValueCount() >= node.GetValueCount()){
    BPTreeNode& rightNode = *siblings.second;
    //Let's move all values and ids to rightNode
    MoveEntriesToRight(node, rightNode, parentNode, nodeIndex,
                       node.GetValueCount());

    if (!node.IsLeaf()){
      //We are a inner node, so we have one more id to go
      rightNode.InsertValue(parentNode.GetValueAt(nodeIndex),
                            node.GetIDAt(0), 0);
    }
    else if (siblings.first != NULL){
      rightNode.SetPrevLeaf(siblings.first->GetPageNumber());
      siblings.first->SetNextLeaf(rightNode.GetPageNumber());
    }
    else{
      rightNode.SetPrevLeaf(node.GetPrevLeaf());

      if (rightNode.GetPrevLeaf() != 0){
        BPTreeNode* prevNode = GetNodefromPageNumber(rightNode.GetPrevLeaf());
        prevNode->SetNextLeaf(rightNode.GetPageNumber());

        WriteNode(*prevNode);
        delete(prevNode);
        prevNode = NULL;
      }
    }

    parentNode.RemoveIDAt(nodeIndex);
    parentNode.RemoveValueAt(nodeIndex);
    return true;
  }
  else{
    return false;
  }
}

bool BPTree::BalanceNode(BPTreeNode* targetNode,
                         pair<BPTreeNode*, BPTreeNode*>& siblings,
                         BPTreeNode* parentNode, size_t nodeIndex) {
  //if possible, balance with left, else with the right one.
  return (siblings.first != NULL && BalanceWithLeft(*targetNode,
                                                    *siblings.first,
                                                    *parentNode, nodeIndex))
         || (siblings.second != NULL && BalanceWithRight(*targetNode,
                                                         *siblings.second,
                                                         *parentNode,
                                                         nodeIndex));
}

bool BPTree::BalanceWithLeft(BPTreeNode& node, BPTreeNode& leftNode,
                             BPTreeNode& parentNode, size_t nodeIndex){
  size_t nodeValueCount = node.GetValueCount(),
         leftNodeValueCount = leftNode.GetValueCount();

  if (nodeValueCount > leftNodeValueCount){

    if (nodeValueCount > node.GetMaxValueCount() / 2
        && leftNodeValueCount < leftNode.GetMaxValueCount()){
      MoveEntriesToLeft(node, leftNode, parentNode, nodeIndex,
                        (nodeValueCount - leftNodeValueCount + 1) / 2);
      return true;
    }
  }
  else if (nodeValueCount < leftNodeValueCount){
    return BalanceWithRight(leftNode, node, parentNode, nodeIndex - 1);
  }
  return false;
}

bool BPTree::BalanceWithRight(BPTreeNode& node, BPTreeNode& rightNode,
                              BPTreeNode& parentNode, size_t nodeIndex){
  size_t nodeValueCount = node.GetValueCount(),
         rightNodeValueCount = rightNode.GetValueCount();

  if (nodeValueCount > rightNodeValueCount){
    if (nodeValueCount > node.GetMaxValueCount() / 2
        && rightNodeValueCount < rightNode.GetMaxValueCount()){
      MoveEntriesToRight(node, rightNode, parentNode, nodeIndex,
                         (nodeValueCount - rightNodeValueCount + 1) / 2);
      return true;
    }
  }
  else if (nodeValueCount < rightNodeValueCount){
    return BalanceWithLeft(rightNode, node, parentNode, nodeIndex + 1);
  }

  return false;
}

void BPTree::MoveEntriesToLeft(BPTreeNode& node, BPTreeNode& leftNode,
                               BPTreeNode& parentNode, size_t nodeIndex,
                               size_t count){
  //auto start = std::chrono::steady_clock::now();

  if (node.IsLeaf()){
    Attribute** values = new Attribute*[count];
    unsigned long* ids = new unsigned long[count];

    for (size_t i = 0; i < count; i++){
      values[i] = &node.GetValueAt(i);
      ids[i] = node.GetIDAt(i);
    }

    leftNode.InsertIds(leftNode.GetValueCount(), ids, count);
    leftNode.InsertValues(leftNode.GetValueCount(), values, count);

    delete[](values);
    delete[](ids);

    node.RemoveIds(0, count);
    node.RemoveValues(0, count);

    //If we are empty this behaves quite odd because we don't fix the parent
    if(node.GetValueCount() > 0){
      parentNode.SetValueAt(nodeIndex - 1, node.GetValueAt(0));
    }
  }
  else{
    Attribute** values = new Attribute*[count];
    unsigned long* ids = new unsigned long[count];

    values[0] = &parentNode.GetValueAt(nodeIndex - 1);

    for (size_t i = 0; i < count - 1; i++){
      values[i + 1] = &node.GetValueAt(i);
      ids[i] = node.GetIDAt(i);
    }

    ids[count - 1] = node.GetIDAt(count - 1);

    leftNode.InsertIds(leftNode.GetValueCount() + 1, ids, count);
    leftNode.InsertValues(leftNode.GetValueCount(), values, count);

    delete[](values);
    delete[](ids);

    parentNode.SetValueAt(nodeIndex - 1, node.GetValueAt(count - 1));

    node.RemoveIds(0, count);
    node.RemoveValues(0, count);
  }

  //auto end = std::chrono::steady_clock::now();
  //mooveLeft += (end - start).count();
}

void BPTree::MoveEntriesToRight(BPTreeNode& node, BPTreeNode& rightNode,
                                BPTreeNode& parentNode, size_t nodeIndex,
                                size_t count){
  //auto start = std::chrono::steady_clock::now();

  if (node.IsLeaf()){
    Attribute** values = new Attribute*[count];
    unsigned long* ids = new unsigned long[count];

    for (size_t i = 0; i < count; i++){
      size_t index = node.GetValueCount() - count + i;

      values[i] = &node.GetValueAt(index);
      ids[i] = node.GetIDAt(index);
    }

    rightNode.InsertIds(0, ids, count);
    rightNode.InsertValues(0, values, count);

    delete[](values);
    delete[](ids);

    size_t index = node.GetValueCount() - count;
    node.RemoveIds(index, count);
    node.RemoveValues(index, count);

    parentNode.SetValueAt(nodeIndex, rightNode.GetValueAt(0));
  }
  else{
    Attribute** values = new Attribute*[count];
    unsigned long* ids = new unsigned long[count];

    for (size_t i = 0; i < count - 1; i++){
      size_t index = node.GetValueCount() - (count - 1) + i;

      values[i] = &node.GetValueAt(index);
      ids[i] = node.GetIDAt(index);
    }

    values[count - 1] = &parentNode.GetValueAt(nodeIndex);
    ids[count - 1] = node.GetIDAt(node.GetValueCount());

    rightNode.InsertIds(0, ids, count);
    rightNode.InsertValues(0, values, count);

    delete[](values);
    delete[](ids);

    parentNode.SetValueAt(nodeIndex,
                          node.GetValueAt(node.GetValueCount() - count));

    size_t index = node.GetValueCount() - count;
    node.RemoveIds(index + 1, count);
    node.RemoveValues(index, count);
  }

  //auto end = std::chrono::steady_clock::now();
  //mooveRight += (end - start).count();
}

void BPTree::WriteNode(BPTreeNode& node) {
  m_treeCache->Write(node.GetPageNumber(), (size_t)node.GetBytes());
}

BPTreeNode* BPTree::GetNodefromPageNumber(size_t page) {
  //auto start = std::chrono::steady_clock::now();

  char* bytes = (char*)m_treeCache->Read(page);
  auto r = new BPTreeNode(bytes, m_treeHeader->GetPageSize(),
                        m_treeHeader->GetValueSize(), page, m_valueCast);

  //auto end = std::chrono::steady_clock::now();
  //getNode += (end - start).count();

  return r;
}

BPTreeNode* BPTree::CreateNode(bool isLeaf){
  size_t page = m_treeHeader->GetEmptyPage();

  if (page == 0){
    page = m_treeCache->NewPage();
  }
  else{
    BPTreeNode* node = GetNodefromPageNumber(page);
    m_treeHeader->SetEmptyPage(node->GetNextLeaf());
    delete(node);
  }

  return new BPTreeNode(m_treeHeader->GetPageSize(),
                        m_treeHeader->GetValueSize(),
                        page, m_valueCast, isLeaf);
}

void BPTree::DeleteNode(BPTreeNode& node){
  node.SetNextLeaf(m_treeHeader->GetEmptyPage());
  m_treeHeader->SetEmptyPage(node.GetPageNumber());
}


void BPTree::StartBulkload(){
  if (m_treeHeader->GetRoot() != 0){
    throw runtime_error("StartBulkload(Attribute&): Tree is not empty!");
  }

  BPTreeNode* rootNode = CreateNode(true);
  m_treeHeader->SetRoot(rootNode->GetPageNumber());
  WriteNode(*rootNode);

  m_bulkLoadLeaf = rootNode;

  m_bulkLoadPath = new stack<BPTreeNode*>();
}

void BPTree::InsertBulkload(Attribute& value, TupleId tupleId) {
  if (m_bulkLoadLeaf == NULL){
    throw runtime_error("InsertBulkload(Attribute&, TupleId): "
                        "Bulkload wasn't started properly!");
  }

  if (m_bulkLoadLeaf->GetValueCount() >= m_bulkLoadLeaf->GetMaxValueCount()){
    BPTreeNode* newBrother = CreateNode(true);

    newBrother->InsertValue(value, tupleId, 0);

    newBrother->SetPrevLeaf(m_bulkLoadLeaf->GetPageNumber());
    m_bulkLoadLeaf->SetNextLeaf(newBrother->GetPageNumber());

    WriteNode(*m_bulkLoadLeaf);
    delete(m_bulkLoadLeaf);
    m_bulkLoadLeaf = newBrother;

    size_t depth = 0;

    while(!m_bulkLoadPath->empty()
          && m_bulkLoadPath->top()->GetValueCount()
             == m_bulkLoadPath->top()->GetMaxValueCount()){
      WriteNode(*m_bulkLoadPath->top());
      delete(m_bulkLoadPath->top());
      m_bulkLoadPath->pop();
      depth++;
    }

    //New root?
    if (m_bulkLoadPath->empty()){
      BPTreeNode* root = CreateNode(false);

      root->SetIDAt(0, m_treeHeader->GetRoot());

      m_treeHeader->SetRoot(root->GetPageNumber());

      m_bulkLoadPath->push(root);
    }

    m_bulkLoadPath->top()->InsertValue(value,
                                       m_bulkLoadPath->top()->GetValueCount());

    //Insert inner nodes until depth is restored
    if (depth > 0){
      BPTreeNode* node = CreateNode(false);

      m_bulkLoadPath->top()->SetIDAt(m_bulkLoadPath->top()->GetValueCount(),
                                     node->GetPageNumber());

      m_bulkLoadPath->push(node);
      depth--;

      while(depth > 0){
        node = CreateNode(false);
        m_bulkLoadPath->top()->SetIDAt(0, node->GetPageNumber());

        m_bulkLoadPath->push(node);
        depth--;
      }

      m_bulkLoadPath->top()->SetIDAt(0, m_bulkLoadLeaf->GetPageNumber());
    }
    else
    {
      m_bulkLoadPath->top()->SetIDAt(m_bulkLoadPath->top()->GetValueCount(),
                                     m_bulkLoadLeaf->GetPageNumber());
    }
  }
  else {
    m_bulkLoadLeaf->InsertValue(value, tupleId,
                                m_bulkLoadLeaf->GetValueCount());
  }
}

void BPTree::EndBulkload() {
  if (m_bulkLoadLeaf == NULL){
    throw runtime_error("EndBulkload(): Bulkload wasn't started properly!");
  }

  WriteNode(*m_bulkLoadLeaf);
  delete(m_bulkLoadLeaf);
  m_bulkLoadLeaf = NULL;

  while(!m_bulkLoadPath->empty()){
    WriteNode(*m_bulkLoadPath->top());
    delete(m_bulkLoadPath->top());
    m_bulkLoadPath->pop();
  }

  delete(m_bulkLoadPath);
  m_bulkLoadPath = NULL;

  BPTreeNode* node = GetNodefromPageNumber(m_treeHeader->GetRoot());

  //It's so empty in here
  if (node->GetValueCount() == 0){
    DeleteNode(*node);
    WriteNode(*node);

    m_treeHeader->SetRoot(0);
  }
  //Check balancing in rightmost path
  else{
    while (!node->IsLeaf()){
      BPTreeNode* parent = node;

      node = GetNodefromPageNumber(parent->GetIDAt(parent->GetValueCount()));

      //Underflow?
      if (node->GetValueCount() < node->GetMaxValueCount() / 2){
        BPTreeNode* leftNode =
          GetNodefromPageNumber(parent->GetIDAt(parent->GetValueCount() - 1));

        BalanceWithLeft(*node, *leftNode, *parent, parent->GetValueCount());

        WriteNode(*node);

        WriteNode(*leftNode);
        delete(leftNode);
        leftNode = NULL;

        WriteNode(*parent);
      }

      delete(parent);
      parent = NULL;
    }
  }

  delete(node);
  node = NULL;

}

/*
 ~printAsTree~ und ~ToString~

*/

void BPTree::PrintAsTree(ostream& o, BPTreeNode& node, size_t depth) {
  for (size_t i = 0; i < depth; i++){
    o << "    ";
  }

  o << "(";
  node.Print(o);
  o << "(Id: " << node.GetPageNumber();

  if (node.IsLeaf()){
    o << " Prev: " << node.GetPrevLeaf() << " Next: " << node.GetNextLeaf();
  }

  o << " ValueCount: " << node.GetValueCount() << ")";

  if (!node.IsLeaf()) { // not a leaf node
    for (size_t i = 0; i < (node.GetValueCount()) + 1; i++) {
      BPTreeNode* child = GetNodefromPageNumber(node.GetIDAt(i));

      o << endl;
      PrintAsTree(o, *child, depth + 1);

      delete(child);
      child = NULL;
    }
  }

  o << ")";
}


string BPTree::ToString() {
  ostringstream o;
  o << "( tree " << endl;

  if (m_treeHeader->GetRoot() != 0){
    BPTreeNode* root = GetNodefromPageNumber(m_treeHeader->GetRoot());
    PrintAsTree(o, *root, 0);
    delete(root);
    root = NULL;
  }

  o << ")";

  return o.str();
}

/* end ~printAsTree~ */

int BPTree::GetAlgebraId(){
  return m_treeHeader->GetAlgebraID();
}

int BPTree::GetTypeId(){
  return m_treeHeader->GetTypeID();
}

BPTreeHeader& BPTree::GetHeader()
{
  return *m_treeHeader;
}

//
// Search for keys in the tree
//
// internal searches
//
// returns the position (index) at which the search-value
// should be placed. If the node doesn't contain the search-value,
// the return value is the most fitting index.
long BPTree::LookupSearchIndex(
    BPTreeNode* node, const Attribute& searchValue ) {

  size_t count = node->GetValueCount();
  if ( count == 0 ) return 0;

  long l = 0;
  long r = count - 1;
  long m = 0;
  int comp = 0; // compare
  Attribute* curValue = NULL;

  // binary search
  while ( l <= r ) {
    m = l + ( ( r - l ) / 2 );

    curValue = &( node->GetValueAt( m ) );
    // -1 : curValue < searchValue
    //  0 : curValue = searchValue
    //  1 : curValue > searchValue
    comp = curValue->Compare( &searchValue );

    // Exact match
    if ( comp == 0 ) break;

    if ( comp > 0 ) r = m - 1;
    else l = m + 1;
  } // while

  // Binary search has the problem, that the search is not stable. We don't
  // know if the found index is the first index if the search-value, if the
  // node contains the search-value multiple times.
  // To make it stable, we look for the left-most index of the search-value.
  if ( comp == 0 ) {
    while ( m > 0 ) {
      // Value left to the found one
      curValue = &( node->GetValueAt( m - 1 ) );

      comp = curValue->Compare( &searchValue );
      if ( comp != 0 ) break;

      m--;
    } // while
  }
  else {
    // If the search-value was not found exactly, the last index (m)
    // is located +1 or -1 to the theoretical index.
    // We need the index +1 after that index.
    //
    // -1 : curValue < searchValue
    if ( comp < 0 ) m++;
  } // else

  return m;
}
//
// returns the position (index) at which the searched
// value has to be inserted
size_t BPTree::LookupInsertIndex(BPTreeNode& node,
                                 const Attribute& insertValue) {
  size_t l = 0,
         r = node.GetValueCount();

  // binary search
  while (r != l) {
    size_t m = l + ((r - l) / 2);

    // -1 : curValue < insertValue
    //  0 : curValue = insertValue
    //  1 : curValue > insertValue
    if (node.GetValueAt(m).Compare(&insertValue) > 0){
      //Are we lucky?
      //if (m > l && node.GetValueAt(m - 1).Compare(&insertValue) < 0){
      //  l = r = m;
      //}
      //Too bad...
      //else{
      r = m;
      //}
    }
    else{
      l = m + 1;
    }
  } // while

  return l;
}

// returns the node which should contain the searched key
BPTreeNode* BPTree::SearchNode( const Attribute& key ) {
  BPTreeNode* currentNode = GetRootNode();
  if ( currentNode == 0 ) return 0;

  while ( !currentNode->IsLeaf() ) {
    // looking for matching position
    size_t matchIx = LookupSearchIndex( currentNode, key );

    // ID (page) of the "best fitting" node
    size_t matchNodeId = currentNode->GetIDAt( matchIx );

    // Cleanup
    delete currentNode;
    currentNode = NULL;

    currentNode = GetNodefromPageNumber( matchNodeId );
  } // while

  return currentNode;
}

// searches for all elements with the given key
BPTreeSearchEnumerator* BPTree::SearchKeys( const Attribute* key ) {
  return SearchKeys( key, key );
}
// searches for all elements within a range  [min,max], including min and max
BPTreeSearchEnumerator* BPTree::SearchKeys(
  const Attribute* minKey,
  const Attribute* maxKey ) {

  BPTreeNode* startNode = 0;

  // If minKey = [null], start with the leftmost node
  if ( !minKey ) startNode = GetLeftMostLeaf();
  else startNode = SearchNode( *minKey );

  BPTreeSearchEnumerator* e = new BPTreeSearchEnumerator(
    this, startNode, minKey, maxKey );
  return e;
}

// delivers the leftmost leaf of the tree
BPTreeNode* BPTree::GetLeftMostLeaf() {
  BPTreeNode* currentNode = GetRootNode();
  if ( currentNode == 0 ) return 0;

  while ( !currentNode->IsLeaf() && currentNode->GetValueCount() > 0 ) {
    // ID (Page) of the left (first) child node
    size_t leftNodeId = currentNode->GetIDAt( 0 );

    // cleanup
    delete currentNode;
    currentNode = GetNodefromPageNumber( leftNodeId );
  } // while

  return currentNode;
}
// delivers the root node or 0 if no root exists
BPTreeNode* BPTree::GetRootNode() {
  size_t id = m_treeHeader->GetRoot();

  if ( id == 0 ) return 0;
  else return GetNodefromPageNumber( id );
}

// delivers the height of the tree
size_t BPTree::GetHeight() {
  // Tree is balanced, so all partial trees are of equal height,
  // so we take the height of the leftmost path to a leaf

  size_t h = 0;

  BPTreeNode* currentNode = GetRootNode();
  if ( currentNode == 0 ) return h;

  while ( !currentNode->IsLeaf() && currentNode->GetValueCount() > 0 ) {
    // ID (Page) of the left (first) child node
    size_t leftNodeId = currentNode->GetIDAt( 0 );

    delete currentNode;
    currentNode = GetNodefromPageNumber( leftNodeId );

    h++;
  } // while

  // cleanup
  delete currentNode;
  currentNode = NULL;

  return h;
}

BPTree::PathEntry::PathEntry(BPTreeNode* node, size_t index){
  this->node = node;
  this->index = index;
}
} // end of namespace fialgebra






















