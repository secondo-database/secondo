
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
#include <cstdio>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <string>
#include "TemporalAlgebra.h"
#include "TemporalUnitAlgebra.h"
#include "SecParser.h"

using namespace std;

struct NFAtransition {
  int oldState;
  int trigger;
  int newState;
};

string int2String(int i);
int str2Int(string const &text);
set<string> stringToSet(string input);
string setToString(set<string> input);
int prefixCount(string str, set<string> strings);
vector<string> splitPattern(string input);
char* convert(string arg);
string eraseQM(string arg); // QM = quotation marks
string addQM(string arg);
int getKey(string type);
string extractVar(string input);
string extendDate(string input, const bool start);
bool checkSemanticDate(const string text, const SecInterval uIv,
                       const bool resultNeeded);
bool checkDaytime(const string text, const SecInterval uIv);
bool labelsMatch(string label, set<string> lbs);
bool timesMatch(Interval<DateTime>* iv, set<string> ivs);
bool checkRewriteSeq(pair<vector<unsigned int>, vector<unsigned int> > seq,
                     unsigned int maxSize, bool print);
Word evaluate(string input);
vector<string> createTrajectory(int size);
void fillML(const MString& source, MString& result, DateTime* duration);
DbArray<NFAtransition> makeNFApersistent(vector<map<int, set<int> > > nfa);
vector<map<int, set<int> > > createNFAfromPersistent(DbArray<NFAtransition> db);

struct TrieNode;

struct NodePointer {
  TrieNode* nextNode;
  int nextDbIndex; // position of the pointed node in Nodes
};

struct TrieNode {
  pair<unsigned int, vector<char> > getChilds();

  NodePointer content[256];
  set<size_t> positions;
};

struct NodeRef {
  NodeRef() {}
  NodeRef(int fc, int lc, int fi, int li) :
          firstCont(fc), lastCont(lc), firstIndex(fi), lastIndex(li) {}

  int firstCont;
  int lastCont;
  int firstIndex;
  int lastIndex;
};

struct NodeLink {
  NodeLink() {}
  NodeLink(int ch, int next) : character((char)ch), nextNode(next) {}
  
  char character;
  unsigned int nextNode;
};

class MLabelIndex {
public:
  MLabelIndex() {}
  
  MLabelIndex(int n) : nodes(0), nodeLinks(0), labelIndex(0) {
    root = 0;
  }
  
  MLabelIndex(DbArray<NodeRef> n, DbArray<NodeLink> nL, DbArray<size_t> lI);

  ~MLabelIndex() {
    removeTrie();
  }

  void initRoot() {root = 0;}
  void cleanDbArrays() {nodes.clean(); nodeLinks.clean(); labelIndex.clean();}
  void insert(string label, set<size_t> pos);
  set<size_t> find(string label); // returns the position(s) of label
  set<size_t> findInDbArrays(string label);
  void makePersistent(); // stores main memory tree structure into DbArrays
  void makePersistent(TrieNode* ptr, stack<unsigned int>& nodeIndexes);
  void removeTrie(); // removes main memory tree structure
  void remove(TrieNode* ptr, unsigned char c); // help function for recursion
  void printDbArrays();
  void printContents(set<string> &labels);
  void appendNodeRef(NodeRef nRef)    {nodes.Append(nRef);}
  void appendNodeLink(NodeLink nLink) {nodeLinks.Append(nLink);}
  void appendLabelIndex(size_t index) {labelIndex.Append(index);}
  int getNodeRefSize() const          {return nodes.Size();}
  int getNodeLinkSize() const         {return nodeLinks.Size();}
  int getLabelIndexSize() const       {return labelIndex.Size();}
  NodeRef getNodeRef(int pos) const;
  NodeLink getNodeLink(int pos) const;
  size_t getLabelIndex(int pos) const;
  DbArray<NodeRef> getNodeRefs() const   {return nodes;}
  DbArray<NodeLink> getNodeLinks() const {return nodeLinks;}
  DbArray<size_t> getLabelIndex() const  {return labelIndex;}
  DbArray<NodeRef>* getNodeRefsPtr()   {return &nodes;}
  DbArray<NodeLink>* getNodeLinksPtr() {return &nodeLinks;}
  DbArray<size_t>* getLabelIndexPtr()  {return &labelIndex;}
  void destroyDbArrays(); // removes persistent structure
  void copyFrom(const MLabelIndex& source);
  
private:
  DbArray<NodeRef> nodes;
  DbArray<NodeLink> nodeLinks;
  DbArray<size_t> labelIndex;
  TrieNode* root;
};