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
 
 01590 Fachpraktikum "Erweiterbare Datenbanksysteme" 
 WS 2011 / 2012

 Svenja Fuhs
 Regine Karg
 Jan Kristof Nidzwetzki
 Michael Teutsch 
 C[ue]neyt Uysal

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of a suffix tree

[TOC]

1 Includes and Defines

*/

#ifndef _SUFFIXTREE_H
#define _SUFFIXTREE_H

#include <vector>
#include <queue>
#include <string>
#include "NestedList.h"
#include "ListUtils.h"
#include "../../Tools/Flob/Flob.h"
#include "../../Tools/Flob/DbArray.h"

using namespace std;

class SuffixTreeEdge;
class SuffixTreeVertex;
class SuffixTreeLoader; 

/*

1 Class ~SuffixTreeVertex~

*/
class SuffixTreeVertex 
{

public:

  friend class SuffixTreeEdge;
  friend std::ostream& operator<<(ostream& out, SuffixTreeVertex& x);

  // Our default constructor
  SuffixTreeVertex(const string *input);

  // Our destructor
  virtual ~SuffixTreeVertex();

  // edges
  bool   HasEdges();
  size_t GetEdgeCount();

  // Get edge by character
  SuffixTreeEdge* GetEdge(const char edgeChar, 
    bool loadDataOnEdgeComplete = true);
  
  // Get edge by position
  SuffixTreeEdge* GetEdgeAt(size_t pos, bool loadDataOnEdgeComplete = true);

  // Load Data for Index
  void LoadTextForIndex(size_t begin, size_t end);
  
  // Insert a new edge
  void InsertEdge(SuffixTreeEdge *edge);

  // Returns our Input
  const string* GetInput() const;

  // Return number of leaves
  size_t GetNumberOfLeaves();

  SuffixTreeEdge *GetParentEdge() const;

  SuffixTreeVertex *GetSuffixLink() const;
  void SetSuffixLink(SuffixTreeVertex *suffixLink);

  // isRoot (cleanup the string reference on destruction)
  bool IsRoot();
 
  // Find a given SearchPattern in our Tree
  bool FindEdgeForSearchPattern(const string searchPattern, 
    const SuffixTreeEdge **resultEdge, int *resultPositionOnEdge);

  static SuffixTreeVertex* CreateSuffixtreeFromListExpr(ListExpr,
      const string*);

  static ListExpr CreateListExprFromSuffixtree(SuffixTreeVertex*);

  // for altering to a generalized S-Tree

  bool GetCov1();
  bool GetCov2();
  size_t GetSDepth();
  void SetCov1(bool b);
  void SetCov2(bool b);
  void SetSDepth(size_t d);

  // Loader
  SuffixTreeLoader* GetLoader() const;
  void SetLoader(SuffixTreeLoader*);
  void TriggerLoadNextData();
  size_t GetVertexId() const;
  void SetVertexId(size_t);

private:

  void SetParentEdge(SuffixTreeEdge *parentEdge);

  // members
  vector<SuffixTreeEdge*> mEdgeVector;
  const string *mInput;

  SuffixTreeEdge *mParentEdge;
  SuffixTreeVertex *mSuffixLink;
  SuffixTreeLoader *mLoader;

  bool cov1, cov2;
  size_t SDepth;
  size_t VertexId;
  static SuffixTreeVertex* CreateVertex(ListExpr, const string*) throw (string);
  static SuffixTreeEdge* CreateEdge(ListExpr, const string*) throw (string);
};

/*

2 Class ~SuffixTreeEdge~

*/
class SuffixTreeEdge 
{

public:

  friend class SuffixTreeVertex;
  friend std::ostream& operator<<(ostream& out, const SuffixTreeEdge& x);

  // Default constructor
  SuffixTreeEdge();

  // First non default constructor
  SuffixTreeEdge(
      size_t startIndex,
      size_t endIndex,
      SuffixTreeVertex *child = NULL);
  
  // Constructor with global end index
  SuffixTreeEdge(
        size_t startIndex,
        SuffixTreeVertex *child = NULL);

  // Desturctor
  virtual ~SuffixTreeEdge();

  // start index
  size_t GetStartIndex() const;
  void   SetStartIndex(size_t startIndex);

  // end index
  size_t GetEndIndex() const;
  void   SetEndIndex(size_t endIndex);

  // length
  size_t GetLength() const;

  // Has our edge a child?
  bool HasVertex() const;

  // vertex
  SuffixTreeVertex* GetChild() const;
  void SetChild(SuffixTreeVertex *vertex);
  SuffixTreeVertex* GetParent() const;

  static void SetGlobalEndIndex(size_t endIndex);

private:

  void SetParent(SuffixTreeVertex *parent);

  // members
  size_t mStartIndex;
  size_t mEndIndex;
  SuffixTreeVertex *mParent;
  SuffixTreeVertex *mChild;

  bool mHasEndIndex;

  static size_t sGlobalEndIndex;
};

/*

2 Class ~SuffixTreeLoader~

*/
class SuffixTreeLoader 
{

  public:
  // constructor
  SuffixTreeLoader(DbArray<size_t> *tree, DbArray<size_t> *index, Flob *input) 
  {
    mSuffixTree = tree;
    mSuffixIndex = index;
    mInput = input;
  }
  
  void TriggerLoadNextData(SuffixTreeVertex *);
  void LoadTextForIndex(SuffixTreeVertex *, size_t begin, size_t end);

  private:

    // member
    DbArray<size_t> *mSuffixTree;
    DbArray<size_t> *mSuffixIndex;
    Flob *mInput;
};


#endif
