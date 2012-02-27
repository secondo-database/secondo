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


#include <iostream>
#include <vector>
#include <queue>
#include <assert.h>

#include "SuffixTree.h"

using namespace std;

SuffixTreeVertex*
SuffixTreeVertex::CreateSuffixtreeFromListExpr(ListExpr suffixtree,
  const string *text){
  return CreateVertex(suffixtree, text);
}
/*
 returns a new SuffixTreeEdge incl. the subtree

*/
SuffixTreeEdge*
SuffixTreeVertex::CreateEdge(ListExpr suffixtree, const string *text)
throw (string)
{
  if (nl->ListLength(suffixtree) != 3)
    {
      throw(string) "No suffixtree";
    }
    ListExpr start = nl->First(suffixtree);
    ListExpr end = nl->Second(suffixtree);
    ListExpr vertex = nl->Third(suffixtree);
    if (!(nl->IsAtom(start) && nl->AtomType(start)== IntType && nl->IsAtom(end)
        && nl->AtomType(end)))
    {
      throw(string) "No suffixtree";
    } else {
      SuffixTreeEdge* tempEdge;
      if (nl->IsEmpty(vertex)){
        tempEdge = new SuffixTreeEdge(nl->IntValue(start),
                nl->IntValue(end), NULL);
      } else {
        tempEdge = new SuffixTreeEdge(nl->IntValue(start),
                nl->IntValue(end), CreateVertex(vertex, text));
      }
      return tempEdge;
    }
}




/*
 returns the root of the suffixtree created from the ListExpr ~suffixtree~

*/
SuffixTreeVertex*
SuffixTreeVertex::CreateVertex(ListExpr suffixtree, const string *text)
throw (string)
{
    SuffixTreeVertex* vertex = new SuffixTreeVertex(text);
    while (!nl->IsEmpty(suffixtree))
    {
      SuffixTreeEdge* tempEdge = CreateEdge(nl->First(suffixtree), text);

      vertex->InsertEdge(tempEdge);
      suffixtree = nl->Rest(suffixtree);
    }
    return vertex;
}

SuffixTreeVertex::SuffixTreeVertex(const string *input) :
    mInput(input),
    mParentEdge(NULL),
    mSuffixLink(NULL),
    mLoader(NULL),
    cov1(false),
    cov2(false), 
    SDepth(0),
    VertexId(0)
{
}

size_t SuffixTreeVertex::GetEdgeCount()
{
  TriggerLoadNextData();
  return mEdgeVector.size();
}

/*

1.2 Get loader

*/
SuffixTreeLoader* SuffixTreeVertex::GetLoader() const 
{
  return mLoader;
}

/*

1.2 Set loader

*/
void SuffixTreeVertex::SetLoader(SuffixTreeLoader *loader) 
{
  mLoader = loader;
}

/*

1.2 Is this vertex the root vertex?
  If it is true, we call delete on mInput
  in destructor

*/
bool SuffixTreeVertex::IsRoot() 
{
  return (mParentEdge == NULL);
}

/*

1.1 Get Number of Leafs

*/
size_t SuffixTreeVertex::GetNumberOfLeaves()
{
  TriggerLoadNextData();
  int result = 0;
  
  for (vector<SuffixTreeEdge*>::const_iterator iter = mEdgeVector.begin();
      iter < mEdgeVector.end();
      iter++)
  {

    if(mLoader != NULL)
    {
      mLoader -> LoadTextForIndex(this, (*iter)->GetStartIndex(), 
        (*iter)->GetEndIndex());
    }

    if( ! (*iter) -> HasVertex() ) 
    {
      result++;
    } 
    else 
    {
      SuffixTreeVertex *vertex = (*iter) -> GetChild();
      result = result + (vertex -> GetNumberOfLeaves());
    }
  } 

  return result;
}

SuffixTreeEdge *SuffixTreeVertex::GetParentEdge() const
{
  return mParentEdge;
}

SuffixTreeVertex *SuffixTreeVertex::GetSuffixLink() const
{
  return mSuffixLink;
}

void SuffixTreeVertex::SetSuffixLink(SuffixTreeVertex *suffixLink)
{
  mSuffixLink = suffixLink;
}

/*
1.1 Function ~FindEdgeForSearchPattern~

 Find position for a given searchPattern in our SuffixTree

 Returns true if searchPattern is found in our suffix tree
 returns false otherweise
 
 If found, resultEdge contains the reference to the edge in our 
 tree, resultPositionOnEdge contains a offset value for the edge
 
*/
bool SuffixTreeVertex::FindEdgeForSearchPattern(
  const string searchPattern, const SuffixTreeEdge **resultEdge, 
  int *resultPositionOnEdge) 
{
  
  SuffixTreeEdge *suffixTreeEdge = this->GetEdge(searchPattern[0], false);
       
  // Not Found
  if(suffixTreeEdge == NULL) 
  {
      return false;
  }
  
  SuffixTreeVertex *nextVertex = suffixTreeEdge -> GetChild();
  
  // Offset for current edge
  int currentEdgeOffset = 0;

  // Process our seachPattern
  for(size_t searchPatternOffset = 0; 
     searchPatternOffset < searchPattern.length(); 
     searchPatternOffset++) 
  {
    int startPosition = suffixTreeEdge -> GetStartIndex();
    int textOffset = startPosition + currentEdgeOffset;
    
    LoadTextForIndex(textOffset, textOffset);

    const string *text = this -> GetInput();
    
    // Text not found on edge
    if((*text)[textOffset] != searchPattern[searchPatternOffset]) 
    {
      return false;
    }

    int endPosition = suffixTreeEdge -> GetEndIndex();
    
    currentEdgeOffset++;
 
    // Is Edge completely processed?
    if(startPosition + currentEdgeOffset > endPosition) 
    {
      // Is our search pattern not completely preocessed?
      if(searchPatternOffset + 1 < searchPattern.length()) 
      {
        // Find Edge
        suffixTreeEdge = nextVertex->
          GetEdge(searchPattern[searchPatternOffset + 1]);
        //cout << ">> Switching to edge " <<  *suffixTreeEdge << endl;

        // Not Found
        if(suffixTreeEdge == NULL) 
        {
          return false;
        }

        nextVertex = suffixTreeEdge -> GetChild();
        currentEdgeOffset = 0;
      }
    }
 }
  
  // Return values
  *resultEdge = suffixTreeEdge;
  *resultPositionOnEdge = currentEdgeOffset;

  return true;
}


/*
1.1 Function ~appendEdge~

Append a new Edge to our ~Vertex~ at position pos

*/
void SuffixTreeVertex::InsertEdge(SuffixTreeEdge *myEdge)
{
  bool wasInserted = false;
  size_t myEdgeStartIdx = myEdge->GetStartIndex();
  for (vector<SuffixTreeEdge*>::iterator iter = mEdgeVector.begin();
      iter < mEdgeVector.end();
      iter++)
  {
     size_t curEdgeStartIdx = (*iter)->GetStartIndex();
     if ((*mInput)[myEdgeStartIdx] < (*mInput)[curEdgeStartIdx])
     {
       mEdgeVector.insert(iter, myEdge);
       wasInserted = true;
       break;
     }
  }
  if (!wasInserted)
  {
    mEdgeVector.push_back(myEdge);
  }
  myEdge->SetParent(this);
}



/*
1.2 Function ~hasEdges~

retuns true if this ~Vertex~ has edges, false else

*/
bool SuffixTreeVertex::HasEdges()
{
  TriggerLoadNextData();
  return ! mEdgeVector.empty();
}

SuffixTreeEdge* SuffixTreeVertex::GetEdge(const char edgeChar, 
  bool loadDataOnEdgeComplete)
{
  TriggerLoadNextData();
  SuffixTreeEdge *edge = NULL;
  for (vector<SuffixTreeEdge*>::const_iterator iter = mEdgeVector.begin();
        iter < mEdgeVector.end();
        iter++)
  {
    size_t curEdgeStartIdx = (*iter)->GetStartIndex();
    if (edgeChar == (*mInput)[curEdgeStartIdx])
    {
      edge = *iter;

      if(mLoader != NULL)
      {
        if(loadDataOnEdgeComplete)
        {
          mLoader -> LoadTextForIndex(this, edge->GetStartIndex(), 
            edge->GetEndIndex());
        } 
        else
        {
          mLoader -> LoadTextForIndex(this, edge->GetStartIndex(), 
            edge->GetStartIndex());
        }
      }

      break;
    }
  }
  return edge;
}

/*
  1.3 Returns our Input

*/
const string* SuffixTreeVertex::GetInput() const {
   return mInput;
}

/*
1.3 Function ~getEdge~

Returns edge on pos ~pos~

*/
SuffixTreeEdge* SuffixTreeVertex::GetEdgeAt(size_t pos, 
  bool loadDataOnEdgeComplete) 
{
  TriggerLoadNextData();
  // Request for non existing edge
  if (mEdgeVector.size() < pos) 
  {
    return NULL;
  }
  vector<SuffixTreeEdge*>::const_iterator iter = mEdgeVector.begin();
  for(size_t i = 0; i < pos; i++)
  {
    iter++;
  }
 
  if(mLoader != NULL)
  {
    if(loadDataOnEdgeComplete)
    {
    mLoader -> LoadTextForIndex(this, (*iter)->GetStartIndex(), 
      (*iter)->GetEndIndex());
    }
    else
    {
    mLoader -> LoadTextForIndex(this, (*iter)->GetStartIndex(), 
      (*iter)->GetStartIndex());
    }
  }

  return *iter;
}

/*
1.3 Function ~LoadTextForIndex~


*/
void SuffixTreeVertex::LoadTextForIndex(size_t begin, size_t end)
{
  if(mLoader != NULL)
  {
     mLoader -> LoadTextForIndex(this, begin, end);
  }
}

/*
 returns the suffixtree as nested list

*/
ListExpr
SuffixTreeVertex::CreateListExprFromSuffixtree(SuffixTreeVertex* tree)
{
  ListExpr list;

    tree -> TriggerLoadNextData();

    // sonList speichert die Kanten eines Knotens (incl. der dazugehörigen
    // Teilbäume) in NestedList-Form
    vector<ListExpr> *sonVector = new vector<ListExpr> ();
    for (size_t i = 0; i < tree->GetEdgeCount(); i++)
    {
      SuffixTreeEdge* edge_i = tree->GetEdgeAt(i);
      ListExpr firstElement = nl->IntAtom(edge_i->GetStartIndex());
      ListExpr secondElement = nl->IntAtom(edge_i->GetEndIndex());
      ListExpr thirdElement;
      if (edge_i->GetChild()==NULL){
        thirdElement=nl->TheEmptyList();
      } else {
        thirdElement= CreateListExprFromSuffixtree(edge_i->GetChild());
      }
      ListExpr edge = nl->ThreeElemList(firstElement, secondElement,
          thirdElement);
      sonVector->push_back(edge);
    }
    // Die Kanten eines Knotens werden in einer List gespeichert
    if (!sonVector->empty()){
    list = nl->OneElemList(sonVector->at(0));
    ListExpr last = list;
    for (size_t i = 1; i < sonVector->size(); i++)
    {
      nl->Append(last, sonVector->at(i));
      last = nl->Rest(last);
    }
    }
  // cleanup
  delete sonVector;
  sonVector = NULL;

  return list;
}

/*
1.4 Internal method to set the parent edge (only used by SuffixTreeEdge)

*/
void SuffixTreeVertex::SetParentEdge(SuffixTreeEdge *parentEdge)
{
  mParentEdge = parentEdge;
}

bool SuffixTreeVertex::GetCov1()
{
  return cov1;
}

bool SuffixTreeVertex::GetCov2()
{
  return cov2;
}

size_t SuffixTreeVertex::GetSDepth()
{
  return SDepth;
}


void SuffixTreeVertex::SetCov1(bool b)
{
  cov1=b;
}
void SuffixTreeVertex::SetCov2(bool b)
{
  cov2=b;
}

void SuffixTreeVertex::SetSDepth(size_t d)
{
  SDepth=d;
}

void SuffixTreeVertex::TriggerLoadNextData()  
{
  if(mLoader != NULL)
  {
    // Load next data 
    if(mEdgeVector.empty()) { 
       mLoader -> TriggerLoadNextData(this);
    }
  }
}

size_t SuffixTreeVertex::GetVertexId() const
{
  return VertexId;
}

void SuffixTreeVertex::SetVertexId(size_t id)
{
  VertexId = id;
}

/* 
1.5 Destructor for ~Vertex~

*/
SuffixTreeVertex::~SuffixTreeVertex()
{
  for(vector<SuffixTreeEdge*>::iterator iter = mEdgeVector.begin();
      iter != mEdgeVector.end();
      iter++)
  {
    SuffixTreeEdge *myEdge = *iter;
    delete myEdge;
  }

  mEdgeVector.clear();

  if( IsRoot() ) 
  {
    delete mInput;
    mInput = NULL;

    if(mLoader != NULL) 
    {
      delete mLoader;
      mLoader = NULL;
    }
  }
}

/*
2.1 Default constructor of ~Edge~

*/
SuffixTreeEdge::SuffixTreeEdge() :
    mStartIndex(0),
    mEndIndex(0),
    mParent(NULL),
    mChild(NULL),
    mHasEndIndex(true)
{
}

/*
2.2 Constructors of ~Edge~

*/
SuffixTreeEdge::SuffixTreeEdge(
    size_t myStartIndex,
    size_t myEndIndex,
    SuffixTreeVertex *child) :
    mStartIndex(myStartIndex),
    mEndIndex(myEndIndex),
    mParent(NULL),
    mChild(NULL),
    mHasEndIndex(true)
{
  SetChild(child);
}
  

SuffixTreeEdge::SuffixTreeEdge(
    size_t myStartIndex,
    SuffixTreeVertex *child) :
    mStartIndex(myStartIndex),
    mEndIndex(0),
    mParent(NULL),
    mChild(NULL),
    mHasEndIndex(false)
{
  SetChild(child);
}

/*
2.3 Set a start and end Index for this edge

*/
void SuffixTreeEdge::SetStartIndex(size_t newStart)
{
  mStartIndex = newStart;
}

void SuffixTreeEdge::SetEndIndex(size_t newEnd)
{
  mEndIndex = newEnd;
  mHasEndIndex = true;
}

/*
2.4 Returns the start Index of this edge

*/
size_t SuffixTreeEdge::GetStartIndex() const
{
  return mStartIndex;
}

/*
2.5 Returns the end Index of this edge

*/
size_t SuffixTreeEdge::GetEndIndex() const
{
  return mHasEndIndex ? mEndIndex : sGlobalEndIndex;
}

/*
2.6 Get Length 

*/
size_t SuffixTreeEdge::GetLength() const
{
  return GetEndIndex() - mStartIndex + 1;
}

/*
2.6 Has our edge a child vertex?

*/

bool SuffixTreeEdge::HasVertex() const
{
  return mChild != NULL;
}

/*
2.6 Set the ~vertex~ for this edge

*/
void SuffixTreeEdge::SetChild(SuffixTreeVertex *myVertex) 
{
  if ((mChild != NULL) && (mChild->GetParentEdge() == this))
  {
    mChild->SetParentEdge(NULL);
  }
  if (myVertex != NULL)
  {
    myVertex->SetParentEdge(this);
    
    if(GetParent() != NULL) 
    {
      myVertex->SetLoader(GetParent()->GetLoader());
    }
  }
  mChild = myVertex;
}

/*
2.7 Get the ~vertex~ for this edge

*/
SuffixTreeVertex* SuffixTreeEdge::GetChild() const
{
  return mChild;
}

SuffixTreeVertex* SuffixTreeEdge::GetParent() const
{
  return mParent;
}

void SuffixTreeEdge::SetParent(SuffixTreeVertex *parent)
{
  mParent = parent;
}

/*
2.8 Destructor for ~Edge~

*/
SuffixTreeEdge::~SuffixTreeEdge() 
{
   if(mChild != NULL) 
   {
      delete mChild;
   }
}

/*
2.9 Load input text on demand

*/
void SuffixTreeLoader::LoadTextForIndex
  (SuffixTreeVertex *rootVertex, size_t begin, size_t end) 
{
  //cout << "-------------> Trigger text load" << begin << " / " << end << endl;
  const string *ourInput = rootVertex -> GetInput();
  string *myInput = const_cast<string*>(ourInput);
  
  //cout << "-------------> Old Text is" << *myInput << endl;

  // load text
  size_t size = end - begin + 1;
  
  //cout << "-------------> Persistence lengt is: " 
    //<< mInput->getSize() << endl;
  //cout << "-------------> Size is: " << size << endl;
  //cout << "-------------> Pos is: " << begin << endl;

  // +1 for our \0
  char *buffer = (char*) malloc((size+1) * sizeof(char));
  bool ok = mInput->read(buffer, size, begin);
  buffer[size] = '\0';
  assert(ok);
  //cout << "------------> Read from persistence: " << buffer << endl;

  myInput -> replace(begin, size, buffer);
  free(buffer);
  buffer = NULL;

  //cout << "-------------> New Text is" << *myInput << endl << endl;
  //cout << "-------------> Length is" << myInput->length() << endl << endl;
}

/*
2.9 Trigger load of a new level

*/
void SuffixTreeLoader::TriggerLoadNextData(SuffixTreeVertex *rootVertex)
{
  // vertexIndex stores the index on the next node to be processed. The
  // element in mSuffixtree[vertexIndex] stores the node's number of
  // outgoing edges.

  size_t rootVertexId = rootVertex -> GetVertexId();

  //cout << "-----------> Trigger load of new childs" << endl;
  //cout << "----------> Vertex " << rootVertexId << endl;

  size_t prevEdges = 0;
  mSuffixIndex -> Get(rootVertexId - 1, prevEdges);
  
  size_t offset = (2 * rootVertexId) + (3 * prevEdges);
  //cout << "So our offset is" << offset << endl;

    SuffixTreeLoader *loader = rootVertex -> GetLoader();

    size_t edgeNo, vertexId;
    mSuffixTree -> Get(offset++, vertexId);
    mSuffixTree -> Get(offset++, edgeNo);

    //cout << "Read vertex id is: " << vertexId << endl;
    assert(vertexId == rootVertexId); // we got the correct position in index

    //cout << "----------> Adding: " << edgeNo << "edges " << endl;
    for (size_t edgeIndex = 0; edgeIndex < edgeNo; edgeIndex++)
    {   
      size_t start, end, destVertexId; 
      mSuffixTree -> Get((int) offset++, start);
      mSuffixTree -> Get((int) offset++, end);
      mSuffixTree -> Get((int) offset++, destVertexId);
      
      loader -> LoadTextForIndex(rootVertex, start, start);
     // cout <<  "------> Added Edge: " << edgeIndex << " start/end " 
     //  << start << " / " << end << endl;
      
      SuffixTreeEdge* newEdge = new SuffixTreeEdge(start, end);
      rootVertex->InsertEdge(newEdge);
      
      if (destVertexId != 0)
      {    
         //cout <<  "------> Edge: " << edgeIndex << " got empty vertex " 
         // << endl; 
        // inner edge
        SuffixTreeVertex *newVertex = 
          new SuffixTreeVertex(rootVertex -> GetInput());
        newVertex -> SetVertexId(destVertexId);
        newEdge->SetChild(newVertex);
      }    
    }    
}

/*
2.9 global end index setter

*/

void SuffixTreeEdge::SetGlobalEndIndex(size_t endIndex)
{
  sGlobalEndIndex = endIndex;
}

/*
2.9 global end index initialization

*/
size_t SuffixTreeEdge::sGlobalEndIndex = 0;

/*
3.0 Overloaded operator

*/
std::ostream& operator<<(ostream& out, SuffixTreeVertex& x) 
{
   x.TriggerLoadNextData();
   out << "( Vertex size: " << x.mEdgeVector.size() << " )" << endl;

   for(vector<SuffixTreeEdge*>::const_iterator iter = x.mEdgeVector.begin();
       iter != x.mEdgeVector.end();
       iter++)
   {
     out << "Ref: " << **iter << endl;
   }

   return out;
}

std::ostream& operator<<(ostream& out, const SuffixTreeEdge& x) 
{
   out << "( Edge vertex: ( " << x.mStartIndex << " "
    << x.mEndIndex << " ) ";
   out <<  x.mChild << " )" << endl;

   return out;
}

