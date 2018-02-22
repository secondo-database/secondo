
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

RTree-Class Implementation

*/
#include <iostream>
#include <string>
#include <deque>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <typeinfo>
#include <utility>
#include "Algebra.h"
#include "SecondoSystem.h"
#include "SecondoCatalog.h"
#include "NestedList.h"
#include "NList.h"
#include "QueryProcessor.h"
#include "AlgebraManager.h"
#include "StandardTypes.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "DateTime.h"
#include "Algebras/TupleIdentifier/TupleIdentifier.h"
#include "Progress.h"
#include "Algebras/FText/FTextAlgebra.h"
#include "ListUtils.h"
#include "NList.h"
#include "Algebras/Standard-C++/LongInt.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "RTreeIterator.h"

#include "BPTree/BPTree.h"
#include "WinUnix.h"
#include <vector>
#include <stack>
#include <math.h>

using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

//Implementation of R-Tree for portable Index

namespace fialgebra{

  template<int dim>
  RTreeIterator<dim>::RTreeIterator(RTree<dim>* rtree, Rectangle<dim> rectArg){
    rt = rtree;
    rect = rectArg;
    pathStack.clear();
  }

  template<int dim>
  RTreeIterator<dim>::~RTreeIterator(){
    if (rt) delete rt;
    pathStack.clear();
    }

  template<int dim>
  size_t RTreeIterator<dim>::Search(){
    
  RTreeNode<dim> *actualNode;
  unsigned int sonPos;
  size_t id;
  actualNode = NULL;

    //fangen am Wurzel des Baums an
    if(pathStack.empty()){
        actualNode = rt->ReadNode(rt->GetHeader()->GetRoot());
        sonPos = 0;
    }
    //oder am letzten Element in Stack
    else{
      //hole den Element und die Position des zuletz betrachteten Sohnes
      id = pathStack.back().first;
      actualNode = rt->ReadNode(id);
      sonPos = pathStack.back().second;
      //löschen Eintrag aus dem Stack
      pathStack.pop_back();
      //schauen den nächsten Eintrag an
      sonPos++;
    }

    //Suchen, bis der letzte Sohn des Wurzels erreicht wird
    while(true){

      if(sonPos < actualNode->GetNumberOfEntries()){
        //falls eine Schneidung gibt's
        if(actualNode->GetValueAt(sonPos).Intersects(rect)){
          //speichere die Knote und Position des Sohnes im Stack
          id = actualNode->GetNodeID();
          pathStack.push_back(std::make_pair(id, sonPos));
          id = actualNode->GetIDAt(sonPos);
          bool isLeaf = actualNode->IsLeaf();
          delete actualNode;
          if(isLeaf){
          //Falls es ein Blatt ist, gebe TID zurück  
            return id;
          }
          else{
          //gehe ein Level runter
            actualNode = rt->ReadNode(id);
            sonPos = 0;
          }    
        }
        else{
          //es gibt keine Schneidung
          sonPos++;          
        }
      }
      //Falls alle Einträge in Knote untersucht wurden
      else{
        if(pathStack.empty()){
          //Falls es ein Wurzel war, Suche ist beendet
          delete actualNode;
          return 0;
        }
        else{
          //Sonst gehe Level höher und suche weiter
          //hole den Element und die Position des Sohnes von Elternknote
          delete actualNode;
          id = pathStack.back().first;
          actualNode = rt->ReadNode(id);
          sonPos = pathStack.back().second;
          //löschen Eintrag aus dem Stack
          pathStack.pop_back();
          //schauen den nächsten Eintrag an
          sonPos++;
        }
      }
    }
  delete actualNode;
  return 0;

  }

  template class RTreeIterator<1>;
  template class RTreeIterator<2>;
  template class RTreeIterator<3>;
  template class RTreeIterator<4>;
  template class RTreeIterator<8>;

}//end of namespace fialgebra

