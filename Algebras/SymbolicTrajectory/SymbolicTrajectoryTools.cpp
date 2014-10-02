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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Auxiliary functions of the Symbolic Trajectory Algebra

Started March 2012, Fabio Vald\'{e}s

*/
#include "SymbolicTrajectoryTools.h"
#include "SecondoCatalog.h"
#include "RelationAlgebra.h"
#include "FTextAlgebra.h"

// extern QueryProcessor *qp;

using namespace std;


void MLabelIndex::insert(TrieNode *ptr, string label, set<size_t> pos) {
  if (!ptr) {
    ptr = new TrieNode();
  }
  unsigned char c;
  unsigned int i = 0;
  while (i <= label.length()) {
    c = label[i];
    if (!ptr->content[c].nextNode) { // successor node has to be created
      if (i == label.length()) { // after last character
        ptr->positions.insert(pos.begin(), pos.end());
      }
      else {
        ptr->content[c].nextNode = new TrieNode();
        ptr = ptr->content[c].nextNode;
      }
    }
    else { // path exists
      ptr = ptr->content[c].nextNode;
    }
    i++;
  }
}

set<size_t> MLabelIndex::find(TrieNode *ptr, string label) {
  set<size_t> result;
  if (!ptr) {
    result = findInDbArrays(ptr, label);
    return result;
  }
  if (!label.length()) { // empty label
    return ptr->positions;
  }
  unsigned int i = 0;
  unsigned char c;
  while (i <= label.length()) {
    if (i == label.length()) { // last character
      return ptr->positions;
    }
    c = label[i];
    if (ptr->content[c].nextNode) { // path exists
      ptr = ptr->content[c].nextNode;
    }
    else { // path does not exist
      result = findInDbArrays(ptr, label);
      return result;
    }
    i++;
  }
  return result; // should not occur
}

set<size_t> MLabelIndex::findInDbArrays(TrieNode* ptr, string label) {
  set<size_t> result;
  if (!nodes.Size()) {
    return result;
  }
  NodeRef nRef = getNodeRef(0);
  unsigned int i(0);
  int nodeRefPos(0);
  bool found = false;
  while (i < label.length()) {
    if ((nRef.firstCont == -1) || (nRef.lastCont == -1)) {
      return result;
    }
    found = false;
    if (nRef.lastCont < nodeLinks.Size()) {
      for (int j = nRef.firstCont; (j <= nRef.lastCont && !found); j++) {
        if (getNodeLink(j).character == label[i]) {
          found = true;
          nodeRefPos = getNodeLink(j).nextNode;
        }
      }
      if (!found) {
        return result;
      }
    }
    else {
      cout << "NL Error:" << nRef.lastCont << ">=" << nodeLinks.Size() << endl;
      return result;
    }
    if (nodeRefPos < nodes.Size()) {
      nRef = getNodeRef(nodeRefPos);
    }
    else {
      cout << "Nodes Error: " << nodeRefPos << " >= " << nodes.Size() << endl;
      return result;
    }
    i++;
  }
  if ((nRef.firstIndex > -1) && (nRef.lastIndex > -1)) {
    if (nRef.lastIndex < labelIndex.Size()) {
      for (int k = nRef.firstIndex; k <= nRef.lastIndex; k++) {
        result.insert(getLabelIndex(k));
      }
    }
    else {
      cout << "LI Err:" << nRef.lastIndex << ">=" << labelIndex.Size() << endl;
      return result;
    }
  }
  insert(ptr, label, result);
  return result;
}

void MLabelIndex::makePersistent(TrieNode *ptr) {
  if (!ptr) {
    return;
  }
  stack<unsigned int> nodeIndexes;
  makePersistent(ptr, nodeIndexes);
}

void MLabelIndex::makePersistent(TrieNode* ptr,
                                 stack<unsigned int>& nodeIndexes) {
  pair<unsigned int, vector<char> > childs = ptr->getChilds();
  NodeRef nRef;
  nRef.firstCont = (childs.first ? nodeLinks.Size() : -1);
  nRef.lastCont = (childs.first ? nodeLinks.Size() + childs.first - 1 : -1);
  nRef.firstIndex = (ptr->positions.size() ? labelIndex.Size() : -1);
  nRef.lastIndex = (ptr->positions.size() ?
                   labelIndex.Size() + ptr->positions.size() - 1 : -1);
  nodes.Append(nRef);
  NodeLink nLink;
  if (childs.first) {
    nLink.character = childs.second.back();
    nLink.nextNode = nodes.Size();
    nodeLinks.Append(nLink);
    childs.second.pop_back();
    stack<unsigned int> missingIndexes;
    for (int i = nRef.firstCont + 1; i <= nRef.lastCont; i++) {
      NodeLink nLink;
      nLink.character = childs.second.back();
      nodeLinks.Append(nLink);
      missingIndexes.push(nodeLinks.Size() - 1);
      childs.second.pop_back();
    }
    while (missingIndexes.size()) {
      nodeIndexes.push(missingIndexes.top());
      missingIndexes.pop();
    }
  }
  else if (nodeIndexes.size()) {
    nodeLinks.Get(nodeIndexes.top(), nLink);
    nLink.nextNode = nodes.Size();
    nodeLinks.Put(nodeIndexes.top(), nLink);
    nodeIndexes.pop();
  }
  for (set<size_t>::iterator it = ptr->positions.begin();
                                   it != ptr->positions.end(); it++) {
    labelIndex.Append(*it);
  }
  for (int i = 0; i < 256; i++) {
    if (ptr->content[i].nextNode) {
      makePersistent(ptr->content[i].nextNode, nodeIndexes);
    }
  }
}

void MLabelIndex::removeTrie(TrieNode* ptr) {
  if (!ptr) {
    return;
  }
  for (int i = 0; i < 256; i++) {
    if (ptr->content[i].nextNode) {
      remove(ptr, i);
    }
  }
  delete ptr;
}

void MLabelIndex::remove(TrieNode* ptr1, unsigned char c) {
  TrieNode* ptr2 = ptr1->content[c].nextNode;
  for (int i = 0; i < 256; i++) {
    if (ptr2->content[i].nextNode) { // successor of ~i~ exists
      remove(ptr2, i);
    }
  } // all successors removed now
  delete ptr2;
}

pair<unsigned int, vector<char> > TrieNode::getChilds() {
  unsigned int number = 0;
  vector<char> characters;
  for (unsigned int i = 0; i < 256; i++) {
    if (content[255 - i].nextNode) {
      number++;
      characters.push_back((char)(255 - i));
    }
  }
  return make_pair(number, characters);
}

void MLabelIndex::printDbArrays() {
  stringstream ss;
  ss << "Nodes" << endl << "=====" << endl;
  NodeRef nRef;
  for (int i = 0; i < nodes.Size(); i++) {
    nodes.Get(i, nRef);
    ss << i << "  [" << nRef.firstCont << ", " << nRef.lastCont << "] ["
       << nRef.firstIndex << ", " << nRef.lastIndex << "]" << endl;
  }
  ss << endl << "NodeLinks" << endl << "=========" << endl;
  NodeLink nLink;
  for (int i = 0; i < nodeLinks.Size(); i++) {
    nodeLinks.Get(i, nLink);
    ss << i << "  \'" << nLink.character << "\'  " << nLink.nextNode << endl;
  }
  ss << endl << "LabelIndexes" << endl << "============" << endl;
  size_t index;
  for (int i = 0; i < labelIndex.Size(); i++) {
    labelIndex.Get(i, index);
    ss << i << "  " << index << endl;
  }
  cout << ss.str() << endl;
}

void MLabelIndex::printContents(TrieNode *ptr, set<string> &labels) {
  for (set<string>::iterator it1 = labels.begin(); it1 != labels.end(); it1++) {
    set<size_t> p = find(ptr, *it1);
    cout << "\"" << *it1 << "\" found at pos: ";
    for (set<size_t>::iterator it = p.begin(); it != p.end(); it++) {
      cout << *it << " ";
    }
    cout << endl;
  }
  cout << endl;
}

NodeRef MLabelIndex::getNodeRef(int pos) const {
  assert((0 <= pos) && (pos < getNodeRefSize()));
  NodeRef nRef;
//   cout << "try to get nodeRef #" << pos << ", size is " << nodes.Size();
  nodes.Get(pos, nRef);
//   cout << "   .............. successful." << endl;
  return nRef;
}

NodeLink MLabelIndex::getNodeLink(int pos) const {
  assert((0 <= pos) && (pos < getNodeLinkSize()));
  NodeLink nLink;
  nodeLinks.Get(pos, nLink);
  return nLink;
}

size_t MLabelIndex::getLabelIndex(int pos) const {
  assert((0 <= pos) && (pos < getLabelIndexSize()));
  size_t index;
  labelIndex.Get(pos, index);
  return index;
}

void MLabelIndex::destroyDbArrays() {
  nodes.Destroy();
  nodeLinks.Destroy();
  labelIndex.Destroy();
}

void MLabelIndex::copyFrom(const MLabelIndex& source) {
  nodes.copyFrom(source.getNodeRefs());
  nodeLinks.copyFrom(source.getNodeLinks());
  labelIndex.copyFrom(source.getLabelIndex());
}
