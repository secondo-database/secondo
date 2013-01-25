
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

string int2String(int i);
int str2Int(string const &text);
set<string> stringToSet(string input);
string setToString(set<string> input);
int prefixCount(string str, set<string> strings);
vector<string> splitPattern(string input);
char* convert(string arg);
string eraseQM(string arg);
string addQM(string arg);
int getKey(string type);
string extractVar(string input);
string extendDate(string input, const bool start);
bool checkSemanticDate(const string text, const SecInterval uIv,
                       const bool resultNeeded);
bool checkDaytime(const string text, const SecInterval uIv);
bool checkRewriteSeq(pair<vector<size_t>, vector<size_t> > seq, size_t maxSize,
                     bool print);
Word evaluate(string input);
vector<string> createTrajectory(int size);
void fillML(const MString& source, MString& result, DateTime* duration);

struct TrieNode;

struct NodePointer {
  TrieNode* nextNode;
  int nextDbIndex; // position of the pointed node in Nodes
};

struct TrieNode {
  NodePointer content[256];
  set<unsigned int> positions;
};

struct NodeRef {
  unsigned int firstCont;
  unsigned int lastCont;
  unsigned int firstIndex;
  unsigned int lastIndex;
};

struct NodeContent {
  char character;
  unsigned int nextNodeIndex;
};

class LabelTrie {
public:
  LabelTrie() {}
  LabelTrie(DbArray<NodeRef>* n, DbArray<NodeContent>* nC, DbArray<size_t>* lI);

  ~LabelTrie() {}

  bool insert(string label, size_t pos);
  bool makePersistent(); // stores main memory tree structure into DbArrays
  bool removeTrie(); // removes main memory tree structure
  set<int> find(string label); // returns the position(s) where label occurs

private:
  DbArray<NodeRef> nodes;
  DbArray<NodeContent> nodeContents;
  DbArray<size_t> labelIndex;
};