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
 ----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the Ukkonen algorithm to create suffix trees

[TOC]

1 Includes and Defines

*/

#include <iostream>

#include "UkkonenTreeBuilder.h"

/*
1.1 Function ~CreateSuffixTree~

Creates a suffix tree using the Ukkonen algorithm.
Uses the following tree extension rules:
Rule 1: insertion point is at a leaf
        -> update end index
Rule 2: character at the insertion point does not match
        -> create new edge / subdivide edge
Rule 3: suffix is already present
        -> do nothing

*/
SuffixTreeVertex* UkkonenTreeBuilder::CreateSuffixTree(string *nText)
{
  //phase 0: initialize tree
  SuffixTreeVertex *root = new SuffixTreeVertex(nText);
  SuffixTreeEdge   *edge = new SuffixTreeEdge(0);
  root->InsertEdge(edge);

  //initialize edge vector to properly set end indexes
  vector<SuffixTreeEdge*> edgeVector;
  edgeVector.push_back(edge);


  SuffixTreeVertex *curVertex = root;
  size_t curVertexDepth = 0;
  SuffixTreeEdge *curEdge;
  SuffixTreeVertex *lastVertex = NULL;
  //start at extension 1, as extension 0 is always present
  size_t curExtension = 1;
  for (size_t curPhase = 1; curPhase <= nText->length(); curPhase++)
  {
    //rule 1 (implicit)
    SuffixTreeEdge::SetGlobalEndIndex(curPhase);

    bool isSuffixPresent;
    do
    {
      isSuffixPresent = false;

      //traverse the tree until the update index lies on the current edge
      while (curExtension + curVertexDepth < curPhase)
      {
        curEdge = curVertex->GetEdge((*nText)[curExtension + curVertexDepth]);
        curVertexDepth += curEdge->GetLength();
        curVertex = curEdge->GetChild();
      }

      if (curExtension + curVertexDepth == curPhase)
      {
        //update at a vertex / edge start --------------------------------
        curEdge = curVertex->GetEdge((*nText)[curExtension + curVertexDepth]);
        if (curEdge != NULL)
        {
          //rule 3
          isSuffixPresent = true;
        }
        else
        {
          //rule 2
          edge = new SuffixTreeEdge(curPhase);
          edgeVector.push_back(edge);
          curVertex->InsertEdge(edge);
        }
        //set suffix link
        if (lastVertex != NULL)
        {
          lastVertex->SetSuffixLink(curVertex);
          lastVertex = NULL;
        }
      }
      else
      {
        //update inside an edge ----------------------------------------

        //set current vertex back to edge start
        curVertex = curEdge->GetParent();
        curVertexDepth -= curEdge->GetLength();

        //calculate inter-edge update index
        size_t interEdgeIdx = curPhase - curExtension - curVertexDepth;
        size_t absUpdateIdx = curEdge->GetStartIndex() + interEdgeIdx;
        if ((*nText)[curPhase] == (*nText)[absUpdateIdx])
        {
          //rule 3
          isSuffixPresent = true;
        }
        else
        {
          //rule 2

          //create new vertex and edges
          curVertex = new SuffixTreeVertex(nText);
          //edge leading to leaf -> no preset end index
          if (curEdge->GetChild() == NULL)
          {
            edge = new SuffixTreeEdge(
                absUpdateIdx,
                curEdge->GetChild());
          }
          else
          {
            edge = new SuffixTreeEdge(
                absUpdateIdx,
                curEdge->GetEndIndex(),
                curEdge->GetChild());
          }
          edgeVector.push_back(edge);
          curVertex->InsertEdge(edge);
          edge = new SuffixTreeEdge(curPhase);
          edgeVector.push_back(edge);
          curVertex->InsertEdge(edge);
          //update current edge
          curEdge->SetEndIndex(absUpdateIdx - 1);
          curEdge->SetChild(curVertex);
          //set suffix link
          if (lastVertex != NULL)
          {
            lastVertex->SetSuffixLink(curVertex);
          }
          lastVertex = curVertex;
          //update depth in tree
          curVertexDepth += interEdgeIdx;
        }
      }

      //compute extensions as long as rule 3 does not apply
      if (!isSuffixPresent)
      {
        //if no suffix link exists from the current vertex (not root),
        //the parent vertex must have it
        if ((curVertex->GetSuffixLink() == NULL) && (curVertex != root))
        {
          SuffixTreeEdge *parentEdge = curVertex->GetParentEdge();
          curVertex = parentEdge->GetParent();
          curVertexDepth -= parentEdge->GetLength();
        }
        if (curVertex->GetSuffixLink() != NULL)
        {
          curVertex = curVertex->GetSuffixLink();
          curVertexDepth--;
        }

        curExtension++;
      }
    } while (!isSuffixPresent && (curExtension <= curPhase));
  }

  //set absolute end indexes to leaf edges
  for (vector<SuffixTreeEdge*>::iterator iter = edgeVector.begin();
       iter != edgeVector.end(); iter++)
  {
    if ((*iter)->GetChild() == NULL)
    {
      (*iter)->SetEndIndex(nText->length()-1);
    }
  }
  return root;
}
