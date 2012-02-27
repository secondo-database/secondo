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

[1] Implementation of a simple algoritm to construct a suffix tree

[TOC]

1 Includes and Defines

*/

#include "SimpleTreeBuilder.h"

using namespace std;


int SimpleTreeBuilder::PosAtEdge (int ESI, int EEI, int SSI, string *t)
{
  bool found = false; 
  int i = 0;
  while (ESI+i<=EEI && !found)       // das erst Zeichen stimmt def. überein
  {
    found = ((*t)[ESI+i] != (*t)[SSI+i]);
    i++;
  }
  if (found)    // rel pos vom edge anfang +1
    return i;
  else
    return -1;

  
}

void SimpleTreeBuilder::PrintTree (SuffixTreeVertex *st, string *t) 
// Nur innere Knoten mit werden ausgegeben!!
{
  queue<SuffixTreeVertex*> vertexQueue;
  SuffixTreeVertex* currentVertex;
  SuffixTreeEdge* currentEdge;
  vertexQueue.push(st);

  int knr=0;
  while (!vertexQueue.empty())
  {
    currentVertex = vertexQueue.front();
    vertexQueue.pop();

    knr++;
  cout<<endl<<"Innerer Knoten "<<knr<<" , Kanten : ";
  
    for (size_t edgeNo = 0; edgeNo < currentVertex->GetEdgeCount(); edgeNo++)
    {
      
    currentEdge = currentVertex->GetEdgeAt(edgeNo);

    cout<<"["<<(currentEdge->GetStartIndex())<<":";
    cout<<(currentEdge->GetEndIndex())<<"]->";
    
    for (size_t i=currentEdge->GetStartIndex(); 
      i<=currentEdge->GetEndIndex(); 
      i++)
    {  
      cout<< (*t)[i];}cout<<"  ";
      
      if (currentEdge->GetChild() != NULL)
      {
        vertexQueue.push(currentEdge->GetChild());
      }
  }
  }
}



/*
1.1 Function ~CreateSuffixTree~

Create a suffix tree from a given text with complexity \(O(n^2)\)

*/
SuffixTreeVertex* SimpleTreeBuilder::CreateSuffixTree(string *text)
{
 
  int TEI = text->length()-1;   // text end index

  SuffixTreeVertex *root = new SuffixTreeVertex(text);
  SuffixTreeEdge   *edge = new SuffixTreeEdge(0,TEI);
  root->InsertEdge(edge);

  // Füge sukzessiv alle Suffixe [i:E] ein
    
  for (int i=1; i<=TEI; i++)
  {
  //  cout<<endl<<endl<<"Einfüge von Suffix["<<i-1<<":E]";PrintTree(root);

    SuffixTreeVertex *curVertex = root;
    SuffixTreeEdge   *curEdge   = root->GetEdge((*text)[i]);

    // Fall 1: Suffix wird an Wurzel eingefügt

    if (curEdge == NULL)
    {
      SuffixTreeEdge * newEdge = new SuffixTreeEdge (i,TEI);
      curVertex->InsertEdge(newEdge);
    }

    else // Position suchen wo Suffix eingefügt wird
    {
      int SASI=i; 
      //Suffix aktueller StartIndex, falls Kanten bei der Suche 
      // durchlaufen muss dieser angepasst werden
      
      bool found=false;
      while (!found)
      {
      
        int pos = PosAtEdge(curEdge->GetStartIndex(), 
          curEdge->GetEndIndex(), SASI,text);       
        if ( pos == -1 ) // Einfügepunkt nicht auf aktueller Kante
        {
          SASI = SASI+curEdge->GetLength();
          curVertex = curEdge->GetChild();
          curEdge   = curVertex->GetEdge((*text)[SASI]);  
          // da Kante durchlaufen startindex anpassen

          if (curEdge == NULL)  //Einfügen an innerem Knoten
          { 
            found = true;
            SuffixTreeEdge * newEdge = new SuffixTreeEdge (SASI,TEI);
            curVertex->InsertEdge(newEdge);
          }
            
        }
        else  // neuen Knoten erzeugen
        {
// cout<<endl<<"Neuen Knoten erzeugen, pos = "<<pos<<"   SASI ="<<SASI<<endl;

          found=true;

          SuffixTreeVertex *newVertex   = new SuffixTreeVertex(text);
          SuffixTreeEdge   *oldEdgePart = new SuffixTreeEdge ( 
            curEdge->GetStartIndex()+pos-1, curEdge->GetEndIndex() );
          SuffixTreeEdge   *newEdge     = new SuffixTreeEdge 
            ( SASI+pos-1, TEI );
          
          oldEdgePart->SetChild( curEdge->GetChild() );
          
          newVertex->InsertEdge(oldEdgePart);
          newVertex->InsertEdge(newEdge);
          
          curEdge->SetChild(newVertex);
          curEdge->SetEndIndex(curEdge->GetStartIndex()+pos-2);
        }
      }
    }
  }

return root;
}

