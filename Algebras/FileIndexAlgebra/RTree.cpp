/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[->] [$\rightarrow$]
//[TOC] [    ableofcontents]
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
#include "RelationAlgebra.h"
#include "DateTime.h"
#include "TupleIdentifier.h"
#include "Progress.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "NList.h"
#include "LongInt.h"
#include "RectangleAlgebra.h"
#include "RTree.h"
#include "Cache/NoCache.h"
#include "Cache/LruCache.h"

#include "Cache/CacheBase.h"
#include <fstream>               // ofstream


#include <string>

#include "BPTree.h"
#include "WinUnix.h"
#include <vector>
#include <stack>
#include <math.h>
#include <stdexcept>

using namespace std;
using fialgebra::cache::LruCache;
using fialgebra::cache::CacheBase;
using fialgebra::cache::NoCache;

//Implementation of R-Tree for portable Index

namespace fialgebra{
template<int dim>
RTree<dim>::RTree(RTreeHeader* header, CacheBase* cache){
    m_treeHeader = header;
    m_treeCache = cache;
}

template<int dim>
RTree<dim>* RTree<dim>::Create(const char* fileName, size_t cacheSize,
                               size_t minEntries){
  size_t pageSize = WinUnix::getPageSize(),
         maxEntries = RTreeNode<dim>::GetMax(pageSize);
  if (maxEntries == 0){
    throw out_of_range("RTree<dim>::Create(const char*, size_t, size_t): "
                       "Systems pageSize too small for a entry");
  }
  else{
    size_t fixedMinEntries = minEntries;
    if (fixedMinEntries == 0){
      fixedMinEntries = max<size_t>(maxEntries / 2, 1);
    }
    else{
      fixedMinEntries = min(fixedMinEntries, max<size_t>(maxEntries / 2, 1));
    }

    ofstream stream(fileName, ofstream::binary | ofstream::app);
    if (stream.good())
    {
      if (stream.tellp() == 0)
      {
        RTreeHeader* header = new RTreeHeader(pageSize, SizeOfRectangle<dim>(),
                                              dim, fixedMinEntries, 0, 0);

        stream.write(header->GetBytes(), header->GetPageSize());
        stream.flush();
        stream.close();

        CacheBase* cache = new LruCache(const_cast<char*>(fileName),
                                        header->GetPageSize(), cacheSize);

        return new RTree<dim>(header, cache);
      }
      else
      {
        stream.close();
        throw runtime_error("File allready existed!");
      }
    }
    else
    {
      stream.close();
      throw runtime_error("File couldn't be created!");
    }
  }
}

template<int dim>
RTree<dim>* RTree<dim>::Open(const char * fileName, size_t cacheSize){
  ifstream stream(fileName, ifstream::binary | ifstream::ate);
  if (!stream.good())
  {
    stream.close();
    throw runtime_error( "File couldn't be opened: " + string(fileName) );
  }
  else
  {
    //cout << "\n Position " << stream.tellg() << "\n";
    //cout << "\n PageSize " << WinUnix::getPageSize() << "\n";

    size_t pageSize = WinUnix::getPageSize(),
           length = min<size_t>(pageSize, stream.tellg());

    stream.seekg(0, ios::beg);

    char* bytes = new char[length];

    stream.read(bytes, length);
    stream.close();

    RTreeHeader* header = new RTreeHeader(bytes, length, pageSize);

    if (header->GetMarker() != TreeHeaderMarker::Rtree){
      delete(header);
      throw runtime_error("File isn't a valid R-Tree!");
    }

    if (header->GetDimension() != dim){
      delete(header);
      throw runtime_error("File contains a R-Tree "
                          "with a different dimension!");
    }

    CacheBase* cache = NULL;
    if (cacheSize == 0){
      cache = new NoCache(fileName, header->GetPageSize(), 0);
    }
    else{
      cache = new LruCache(fileName, header->GetPageSize(), cacheSize);
    }

    return new RTree<dim>(header, cache);
  }
}


/* This function is exclusively used by the 
 * rebuildfrtree operator. The main purpose is
 * to check whether the first file that has been
 * passed to the operator actually contains a valid
 * RTree */

template<int dim>
RTree<dim>* RTree<dim>::OpenRebuild(const char * fileName, size_t cacheSize){
  ifstream stream(fileName, ifstream::binary | ifstream::ate);
  if (!stream.good())
  {
    stream.close();
    throw runtime_error( "File couldn't be opened: " + string(fileName) );
  }
  else
  {
    //cout << "\n Position " << stream.tellg() << "\n";
    //cout << "\n PageSize " << WinUnix::getPageSize() << "\n";

    size_t pageSize = WinUnix::getPageSize(),
           length = min<size_t>(pageSize, stream.tellg());

    stream.seekg(0, ios::beg);

    char* bytes = new char[length];

    stream.read(bytes, length);
    stream.close();

    RTreeHeader* header = new RTreeHeader(bytes, length, pageSize);

    if (header->GetMarker() != TreeHeaderMarker::Rtree){
      delete(header);
      throw runtime_error("File isn't a valid R-Tree!");
    }

    CacheBase* cache = NULL;
    if (cacheSize == 0){
      cache = new NoCache(fileName, header->GetPageSize(), 0);
    }
    else{
      cache = new LruCache(fileName, header->GetPageSize(), cacheSize);
    }

    return new RTree<dim>(header, cache);
  }
}


template<int dim>
void RTree<dim>::Insert( const Rectangle<dim>& box, size_t id ) {

  RTreeNode<dim>* node = NULL;
  // This variable saves the path (ids) from root to target leaf
  vector <size_t> path;
  
  // 1. start with the root node
  //    First we need to check if there is already a root node available
  //    in case there is no existing root let´s create it and put the
  //    ID into the root. Then we´re already done.
  if ( m_treeHeader->GetRoot() == 0 ) {
    // generate root
    node = CreateRTReeNode( true );
    // write page number of root node into header
    m_treeHeader->SetRoot( node->GetNodeID() );
  }
  else {
    // 2. There is already an existing root.
    // 2.1. We're requesting it.
    node = ReadNode( m_treeHeader->GetRoot() );

    while ( !node->IsLeaf() ) {
      // 3.1. In order to find the target leaf we can use
      //      the function "BestSonSearch"
      size_t nextID = BestSonIDSearch(*node, box);

      size_t numberOfEntries = node->GetNumberOfEntries();
      for ( size_t i = 0; i < numberOfEntries; i++ ) {
        if ( node->GetIDAt( i ) == nextID ) {
          node->SetValueAt( i, node->GetValueAt( i ).Union( box ) );
          break;
        } // if
      } // for
      
      path.push_back( node->GetNodeID() );
      WriteNode( *node );
      
      delete node;
      node = ReadNode(nextID);
    } // while
  } // else
  
  node->AddEntry( box, id );
  
  while( node->GetNumberOfEntries() > node->GetMax() ) {
    RTreeNode<dim>* parent = NULL;
    if ( path.size() > 0 ) {
      parent = ReadNode( path.back() );
      path.pop_back();
    } // if
    
    SplitNode( node, parent );

    if ( parent != NULL ) {
      WriteNode( *node );
      delete node;
      node = parent;
    }
    else {
      break;
    } // else
  } // while

  WriteNode( *node );
  
  delete node;
  node = 0;
} // end of Insert

template<int dim>
bool RTree<dim>::Delete(const Rectangle<dim>& box, size_t id){
  stack<pair<size_t, size_t>> path;
  RTreeNode<dim>* node = NULL;
  size_t index = 0;

  if (m_treeHeader->GetRoot() != 0){
    path.push(pair<size_t, size_t>(m_treeHeader->GetRoot(), 0));
  }

  //Find node and index
  while (node == NULL && !path.empty()){
    RTreeNode<dim>* currentNode = ReadNode(path.top().first);
    size_t currentIndex = path.top().second;
    path.pop();

    bool found = false;
    if (!currentNode->IsLeaf()){
      while (!found && currentIndex < currentNode->GetNumberOfEntries()){
        if (currentNode->GetValueAt(currentIndex).Contains(box)){
          path.push(pair<size_t, size_t>(currentNode->GetNodeID(),
                                         currentIndex));
          path.push(pair<size_t, size_t>(currentNode->GetIDAt(currentIndex),
                                        0));
          found = true;
        }
        else{
          currentIndex++;
        }
      }

      delete(currentNode);
      currentNode = NULL;
    }
    else{
      while (!found && currentIndex < currentNode->GetNumberOfEntries()){
        if (currentNode->GetValueAt(currentIndex) == box
            && currentNode->GetIDAt(currentIndex) == id){
          node = currentNode;
          index = currentIndex;

          found = true;
        }
        else{
          currentIndex++;
        }
      }

      if (node == NULL){
        delete(currentNode);
        currentNode = NULL;
      }
    }

    if(!found && !path.empty()){
      path.top() = pair<size_t, size_t>(path.top().first,
                                        path.top().second + 1);
    }
  }

  //Remove entry if found
  if (node != NULL){
    node->RemoveEntryAt(index);

    //Not root and underflow?
    while (!path.empty()
           && node->GetNumberOfEntries() < m_treeHeader->GetMinEntries()){
      //Remove node from parent
      RTreeNode<dim>* parent = ReadNode(path.top().first);
      parent->RemoveEntryAt(path.top().second);
      path.pop();

      //Redistribute all entries
      for (size_t i = node->GetNumberOfEntries(); i > 0; i--){
        const Rectangle<dim>& value = node->GetValueAt(i - 1);
        size_t id = node->GetIDAt(i - 1);

        RTreeNode<dim>* bestSon = BestSonSearch(*parent, value);
        bestSon->AddEntry(value, id);

        //Find best son's index and update value in parent
        for (size_t j = 0; j < parent->GetNumberOfEntries(); j++){
          if (parent->GetIDAt(j) == bestSon->GetNodeID()){
            parent->SetValueAt(j, bestSon->GetBox());
            break;
          }
        }

        if (bestSon->GetNumberOfEntries() > bestSon->GetMax()){
          SplitNode( bestSon, parent );
        }

        WriteNode(*bestSon);
        delete(bestSon);
        bestSon = NULL;

        node->RemoveEntryAtEnd();
      }

      RecycleNode(*node);
      WriteNode(*node);
      delete(node);
      node = parent;
    }

    //Overflow?
    if (node->GetNumberOfEntries() > node->GetMax()){
      do{
        RTreeNode<dim>* parent = path.empty() ?
                                 NULL : ReadNode(path.top().first);
        path.pop();

        SplitNode( node, parent );

        WriteNode(*node);
        delete(node);
        node = parent;
      }
      while (node->GetNumberOfEntries() > node->GetMax());
    }
    //Root?
    else if (path.empty()){
      //Only one child?
      if (!node->IsLeaf() && node->GetNumberOfEntries() == 1){
        m_treeHeader->SetRoot(node->GetIDAt(0));
        RecycleNode(*node);
      }
      //Were empty
      else if (node->GetNumberOfEntries() == 0){
        m_treeHeader->SetRoot(0);
        RecycleNode(*node);
      }
    }

    WriteNode(*node);
    delete(node);
    node = NULL;

    return true;
  }
  else{
    return false;
  }
}

template<int dim>
RTreeNode<dim>* RTree<dim>::ReadNode( size_t id ) {
  char* bytes = (char*)m_treeCache->Read( id, m_treeHeader->GetPageSize() );
  return new RTreeNode<dim>( bytes, m_treeHeader->GetPageSize(), id );
}

template<int dim>
void RTree<dim>::WriteNode( RTreeNode<dim>& myNode, size_t page ) {
  char* bytes = myNode.GetBytes();
  m_treeCache->Write( page, (size_t)bytes, m_treeHeader->GetPageSize() );
}

template<int dim>
void RTree<dim>::RecycleNode(RTreeNode<dim>& node) {
  node.ClearEntries();
  node.AddEntry(Rectangle<dim>(false), m_treeHeader->GetEmptyPage());
  m_treeHeader->SetEmptyPage(node.GetNodeID());
}

  //destructor saves changes that have been done within the tree
  //using delete(m_treeCache)
template<int dim>
RTree<dim>::~RTree(){
  m_treeCache->Write(0, (size_t)m_treeHeader->GetBytes(),
                     RTreeHeader::GetHeaderSize());

  delete(m_treeCache);
  m_treeCache = NULL;

  delete(m_treeHeader);
  m_treeHeader = NULL;
}

  template<int dim>
  void RTree<dim>::WriteNode(RTreeNode<dim>& node) {
    m_treeCache->Write(node.GetNodeID(),
                       (size_t)node.GetBytes(),
                       m_treeHeader->GetPageSize());
  }

  template<int dim>
  RTreeHeader* RTree<dim>::GetHeader(){
    return  m_treeHeader;
  }

  template<int dim>
  RTreeNode<dim>* RTree<dim>::BestSonSearch( const RTreeNode<dim>& actualNode,
                                             const Rectangle<dim>& rect ) {
    RTreeNode<dim>* bestSon = ReadNode( actualNode.GetIDAt( 0 ) );
    Rectangle<dim>  box     = actualNode.GetValueAt( 0 );
    
    double bestArea     = box.Area( NULL );
    double bestIncrease = box.Union( rect ).Area( NULL ) - bestArea;

    for ( size_t i = 1; i < actualNode.GetNumberOfEntries(); i++ ) {
      box = actualNode.GetValueAt( i );
      double area     = box.Area( NULL );
      double increase = box.Union( rect ).Area( NULL ) - area;

      if ( increase <= bestIncrease ) {
        RTreeNode<dim>* son = NULL;

        if ( increase < bestIncrease ) {
          son = ReadNode( actualNode.GetIDAt( i ) );
        }
        else if ( area <= bestArea ) {
          son = ReadNode( actualNode.GetIDAt( i ) );

          if ( area == bestArea &&
               son->GetNumberOfEntries() >= bestSon->GetNumberOfEntries() ) {
            
            delete son;
            son = NULL;
            
            continue;
          } // if
        }
        else {
          continue;
        } // else

        delete bestSon;
        bestSon = son;
        bestArea = area;
        bestIncrease = increase;
      } // if
    } // for

    return bestSon;
  }

  template<int dim>
  size_t RTree<dim>::BestSonIDSearch( const RTreeNode<dim>& actualNode,
                                             const Rectangle<dim>& rect ) {
    size_t sonNumber = 0;
    Rectangle<dim> sonBox = actualNode.GetValueAt(sonNumber);
    Rectangle<dim> tempBox = sonBox;
    double bestArea = sonBox.Area();
    double area = bestArea;
    double bestIncrease = sonBox.Union(rect).Area() - bestArea;
    double increase = bestIncrease;
    int numberOfEntries = actualNode.GetNumberOfEntries();

    for(int i = 1; i < numberOfEntries; i++){
      tempBox = actualNode.GetValueAt(i);
      area = tempBox.Area();
      bestArea = sonBox.Area();
      increase = tempBox.Union(rect).Area() - area;
      if (increase < bestIncrease){
         bestIncrease = increase;
         sonNumber = i;
         sonBox = tempBox;
      }//end of  if (increase < bestIncrease)
      else{
        if (increase == bestIncrease){
          if (area < bestArea){
             sonNumber = i;
             sonBox = tempBox;
          }// end of if (r.Area() < sonBox.Area())
          else{
            if(area == bestArea){
              RTreeNode<dim>* tempNode = ReadNode(actualNode.GetIDAt(i));
              RTreeNode<dim>* tempBestSon = ReadNode(sonNumber);
              if (tempBestSon->GetNumberOfEntries() >
                  tempNode->GetNumberOfEntries()){
                 sonNumber = i;
                 sonBox = tempBox;
              }// end of if (son->GetNumberOfEntries() > ...
              delete (tempNode);
              delete (tempBestSon);
            }// end of if(r.Area() == sonBox.Area())
          }
        }// end of if (increase == bestIncrease)
      }
    }// end of for

    return actualNode.GetIDAt(sonNumber);
  }
  
  
  template<int dim>
  void RTree<dim>::SplitNode( RTreeNode<dim>*& node, RTreeNode<dim>* parent ) {
    size_t numberOfEntries = node->GetNumberOfEntries(),
           seed1 = 0,
           seed2 = numberOfEntries - 1;

    double bestUnionArea = 0;
    for( size_t i = 0; i < numberOfEntries; i++ ) {
      const Rectangle<dim>& value1 = node->GetValueAt( i );

      for( int j = i + 1; j < numberOfEntries; j++ ) {
        double unionArea = value1.Union( node->GetValueAt( j ) ).Area();

        if ( unionArea > bestUnionArea ) {
          seed1 = i;
          seed2 = j;
          bestUnionArea = unionArea;
        } // if
      } // for
    } // for

    // I. now we need to generate two new nodes and assign rectangle and id
    RTreeNode<dim>* node1 = new RTreeNode<dim>( *node );
    node1->ClearEntries();
    node1->AddEntry( node->GetValueAt( seed1 ), node->GetIDAt( seed1 ) );

    RTreeNode<dim>* node2 = CreateRTReeNode( node->IsLeaf() );
    node2->AddEntry( node->GetValueAt( seed2 ), node->GetIDAt( seed2 ) );

    // Remove seed1 and seed2 from the current node
    node->RemoveEntryAt( seed1 );
    node->RemoveEntryAt( seed2 > seed1 ? seed2 - 1 : seed2 );
    numberOfEntries -= 2;

    size_t minEntries = m_treeHeader->GetMinEntries();
    // in this step all remaining entries of the passed node
    // get distributed to the son nodes
    while ( numberOfEntries > 0 ) {
      if ( numberOfEntries + node1->GetNumberOfEntries()
         == minEntries ) {
        while ( numberOfEntries > 0 ) {
          node1->AddEntry( node->GetValueAt( numberOfEntries - 1 ),
                           node->GetIDAt( numberOfEntries - 1 ) );
          node->RemoveEntryAtEnd();
          numberOfEntries--;
        } // while
      }
      else if ( numberOfEntries + node2->GetNumberOfEntries()
              == minEntries ){
        while ( numberOfEntries > 0 ) {
          node2->AddEntry( node->GetValueAt( numberOfEntries - 1 ),
                           node->GetIDAt( numberOfEntries - 1 ) );
          node->RemoveEntryAtEnd();
          numberOfEntries--;
        } // while
      }
      else {
        // identifies where the next entry of the father node
        // needs to be inserted
        // initialize the bounding boxes of the two sons
        Rectangle<dim> box1 = node1->GetBox(),
                       box2 = node2->GetBox();

        double a1 = box1.Area();
        double a2 = box2.Area();

        // identify pair with largest delta (between bounding boxes)
        // calculates the amount the bounding box should be extended
        RTreeNode<dim>* target = NULL;
        unsigned int index;
        double d;

        for ( int i = 0; i < numberOfEntries; i++ ) {
          const Rectangle<dim>& value = node->GetValueAt( i );
          double d1 = value.Union(box1).Area() - a1,
                 d2 = value.Union(box2).Area() - a2,
                 d3 = abs(d1 - d2);

          if(target == NULL || d3 > d){
            d = d3;
            index = i;

            if(d1 < d2){
              target = node1;
            }
            else if(d2 < d1){
              target = node2;
            }
            else if(a1 < a2){
              target = node1;
            }
            else if(a2 < a1){
              target = node2;
            }
            else if(node1->GetNumberOfEntries() < node2->GetNumberOfEntries()){
              target = node1;
            }
            else if(node2->GetNumberOfEntries() < node1->GetNumberOfEntries()){
              target = node2;
            }
            else{
              // all criterions failed
              target = node1;
            }
          }
        } // for

        target->AddEntry( node->GetValueAt( index ), node->GetIDAt( index ) );
        node->RemoveEntryAt( index );
        numberOfEntries--;
      } // else 
    } // while

    // Were splitting the root node
    if ( parent == NULL ) {
      RTreeNode<dim>* root = CreateRTReeNode( false );
      root->AddEntry( node1->GetBox(), node1->GetNodeID() );
      root->AddEntry( node2->GetBox(), node2->GetNodeID() );

      m_treeHeader->SetRoot( root->GetNodeID() );

      WriteNode( *root, root->GetNodeID() );
      delete root;
    }
    else {
      for ( size_t i = 0; i < parent->GetNumberOfEntries(); i++ ) {
        if ( parent->GetIDAt( i ) == node1->GetNodeID() ) {
          parent->SetValueAt( i, node1->GetBox() );
        } // if
      } // for

      parent->AddEntry( node2->GetBox(), node2->GetNodeID() );
    } // else

    WriteNode( *node2 );
    delete node2;
    node2 = NULL;
    
    delete node;
    node = node1;
  }
  
    // delivers the used cache
  template<int dim>
CacheBase* RTree<dim>::GetCache() {
  return m_treeCache;
}

template<int dim>
  void RTree<dim>::RebuildR(const char* filename) {
    
  
  RTreeHeader* header1 = this->GetHeader();
  
  // Remove target file if it exists
  fstream file2(filename);
  
  remove(filename);
  
  
  
  // Create target tree
  
  RTree<dim>* rt2 = Create(filename, 512,
                                header1->GetMinEntries());
                               
  RTreeHeader* header2 = rt2->GetHeader();
  
  // Copy header data
  header2->SetPageSize(header1->GetPageSize());
  
  // Root node will be written to page 1
  header2->SetRoot(1);
  
  // Pointer to list of empty pages is null pointer
  header2->SetEmptyPage(0);
  
  // First, copy root node to target file
  RTreeNode<dim>* yy1 = this->ReadNode(GetHeader()->GetRoot());
  
  yy1->SetNodeID(1);
  
  size_t page = (rt2->GetCache())->NewPage();
  
  
  
  RTreeNode<dim> yy11 = *yy1;
  rt2->WriteNode(yy11, page);
  delete yy1;
  // Second, copy the other nodes to target file
 
  unsigned int numbNodesToRead = 1;
  size_t pageReadNext = 1;
  unsigned int counter1 = 0;
  size_t yy3 = 0, yy4 = 0;

  bool endRebuild;

  RTreeNode<dim>* yy2;
  yy2 = rt2->ReadNode( pageReadNext );
  endRebuild = yy2->IsLeaf();
  delete yy2;

  while( !endRebuild ) {
    
  
    // for each node already written to target file fetch children
    // from source file and write them to target file; if leaf level
    // reached, end since no children exist.
    for (unsigned int i = 0; i < numbNodesToRead; i++) {
      // fetch next node which is already written to target file
      yy2 = rt2->ReadNode(pageReadNext);
  //    yy3 = yy2->GetValueCount() + 1;  Steffen fragen
      yy3 = yy2->GetNumberOfEntries();
      yy4 = yy2->GetNodeID();
      endRebuild = yy2->IsLeaf();

      if(!endRebuild){    
        // fetch node's child nodes from source file
        for (unsigned int k = 0; k < yy3; k++) {
          yy1 = this->ReadNode(yy2->GetIDAt(k));
          page = (rt2->GetCache())->NewPage();
          // if child node is leaf, set PrevLeaf and NextLeaf
          
          // set child node's new page number
          yy1->SetNodeID(page);
          // set child node's new parent number
          yy1->SetParentNodeID(yy4);
          // write child node to target file
          rt2->WriteNode(*yy1, page);
          // Set child node's ID to new page number in node's
          // IDs array.
          yy2->SetIDAt(k, page);
          delete yy1;
          counter1++;
        } // for (unsigned int k = 0; k < yy3; k++)
        // write changed node back to target file
        rt2->WriteNode(*yy2);
        delete yy2;
        pageReadNext++;
      }
      else{
        delete yy2;
      }
    } // for (unsigned int i = 0; i < numbNodesToRead; i++)
    // End of copying leaves
    numbNodesToRead = counter1;
    counter1 = 0;
  } // while
  
 delete rt2;
 
  
}

  
  
  template<int dim>
  RTreeNode<dim>* RTree<dim>::
  GetChildNodeByIndex(RTreeNode<dim>* node, size_t index){

    int i = 0;

    while (i< this -> AllNodesWithinRTree.size())
    {
   //   if (this -> AllNodesWithinRTree(i).nodeID == myNodeID)
   //   { break;
   //     return this -> AllNodesWithinRTree(i);
   // }
    i++;

  }
   return 0;
  }




template<int dim> size_t
RTree<dim>::GetChildNodeID(RTreeNode<dim> *parent, RTreeNode<dim> *child){
    size_t childNodeID = child->GetNodeID();

    int i = 0;
    int mylength = parent->GetNumberOfEntries();

    while (i < mylength) {
      if (parent->GetIDAt(i) == childNodeID){
        return parent->GetIDAt(i);
      }
    }

    return 0;
}

template<int dim>
RTreeNode<dim>* RTree<dim>::
 GetNodeByID(const Rectangle<dim>& box, const unsigned long id){

    RTree<dim> rtr = *this;
    int mylength = rtr.AllNodesWithinRTree.size();
    int i = 0;

    while (i < mylength){

      RTreeNode<dim> rtn  = rtr.AllNodesWithinRTree.at(i);

      if (id == rtn.GetNodeID()) {
    break;
    RTreeNode<dim>* result = &rtn;
    return result;
      }
      i++;
    }
    return 0;
}




template<int dim>
RTreeNode<dim>* RTree<dim>::GetParentNode(RTreeNode<dim>* node){

  // unsigned long* pid = node -> parentNodeID;
  size_t pid = node->GetParentNodeID();
    RTreeNode<dim>* myParentNode = this -> ReadNode(pid);
    return myParentNode;
}





template<int dim>
void RTree<dim>::RemoveNode(RTreeNode<dim> *toDelete){
    delete toDelete;
}




template<int dim>
void RTree<dim>::InsertNode(RTreeNode<dim>* parent, RTreeNode<dim>* child){

      int son_position = parent -> GetNumberOfEntries();
      parent -> SetIDAt(son_position, child->GetNodeID());

}




template<int dim>
void RTree<dim>::RemoveNodeFromRAM (RTreeNode<dim>* node){
}


template<int dim>
RTreeNode<dim>* RTree<dim>::CreateRTReeNode(bool isLeaf){
  size_t id = m_treeHeader->GetEmptyPage();

  if (id != 0){
    RTreeNode<dim>* node = ReadNode(m_treeHeader->GetEmptyPage());
    m_treeHeader->SetEmptyPage(node->GetIDAt(0));
    delete(node);
  }
  else{
    id = m_treeCache->NewPage();
  }

  return new RTreeNode<dim>(m_treeHeader->GetPageSize(), id, isLeaf);
}

template<int dim>
void RTree<dim>::ReinsertNodes(RTreeNode<dim> *node){
   
  // node is a leaf and gets inserted 
    if(node->IsLeaf()){
        Insert(node->GetBox(), node->GetNodeID());
    }

    /* node is not a leaf -> iterate through all 
     * its children calling ReinsertNodes for each
     * of them */
    else{
        for(int i = 0; i<node->GetNumberOfEntries(); i++){
            ReinsertNodes(GetChildNodeByIndex(node,i));
        }
    }

    RemoveNode(node);
}


template<int dim>
void RTree<dim>::CondenseTree(RTreeNode<dim> *parent){
    //current node
    RTreeNode<dim> *x = parent;

    //saves all nodes to be inserted
    std::vector<RTreeNode<dim>*> n;

    //if x is root we´re done, otherwise:
    while(x != root){

        //Parent node of x
        RTreeNode<dim> *px = GetParentNode(x);

    /* x has not enough entries, hence remove x
     * from px and save this in n */
        if(x->GetNumberOfEntries() < m_treeHeader->GetMinEntries()){
            int index = GetChildNodeID(px,x);
            px->RemoveEntryAt(index);
            n.push_back(x);
        }
        else{

        }

        
        // go up one level within the tree
        // setting x to its parent node
        x=px;
    }

    /* Re-Insert all leaves that were previously
     * removed into the tree */
    for(auto it=n.begin(); it!= n.end(); it++){
        ReinsertNodes(*it);
    }

}


// 
// Bulkload
// 
// Start bulkload-mode
template<int dim>
void RTree<dim>::BeginBulkload( const double maxDist ) {
  // Initialize bulkload-process
  
  // Check for empty tree
  if ( m_treeHeader->GetRoot() != 0 )
    throw runtime_error( "Tree-file is not empty" );
  
  // Create root node
  _curBulkNode = CreateRTReeNode( true );
  // update header
  m_treeHeader->SetRoot( _curBulkNode->GetNodeID() );
  
  if ( !_bulkStack.empty() )
    throw runtime_error( "internal error" );
  
  _bulkMaxHeight = 0;
  _bulkMaxDist = maxDist;
}
// Insert value in bulkload-mode
template<int dim>
void RTree<dim>::Bulkload( const Rectangle<dim>& box, const size_t id ) {
  // Undefined rectangles will be skipped
  if ( !box.IsDefined() ) return;
  
  bool needNewNode = false;
  // If the current node is completely filled, we have to start a new one
  if ( _curBulkNode->GetNumberOfEntries() >= _curBulkNode->GetMax() )
    needNewNode = true;
  
  // Check max. distance
  if ( !needNewNode && 
       _bulkMaxDist > 0.0 && 
       _curBulkNode->GetNumberOfEntries() > 0 ) {
    Rectangle<dim> b = _curBulkNode->GetBox();
    double dist = box.Distance( b );
    if ( dist > _bulkMaxDist ) needNewNode = true;
  } // if
  
  if ( needNewNode ) {
    // The stack contains all inner nodes on the path to the
    // current bulk node (leaf)
    
    if ( _bulkStack.size() < _bulkMaxHeight ) {
      // If the current path to the leaf is not as height as the most height
      // path, we can add another level of (inner) nodes.
      
      RTreeNode<dim>* newInterNode = CreateRTReeNode( false );
      newInterNode->AddEntry( _curBulkNode->GetBox(),
        _curBulkNode->GetNodeID() );
      
      RTreeNode<dim>* lastInnerNode = _bulkStack.top();
      lastInnerNode->SetIDAt( lastInnerNode->GetNumberOfEntries() - 1,
        newInterNode->GetNodeID() );
      lastInnerNode->SetValueAt( lastInnerNode->GetNumberOfEntries() - 1,
        newInterNode->GetBox() );
      
      // Make persisten
      WriteNode( *newInterNode );
      WriteNode( *lastInnerNode );
      
      // Push the new "inter-node" to the stack
      _bulkStack.push( newInterNode );
    } // if
    
    // The inner node, to which the new child-node will be added
    RTreeNode<dim>* innerNode = 0;
    
    while ( _bulkStack.size() > 0 ) {
      // Get and remove top element
      RTreeNode<dim>* stackNode = _bulkStack.top();
      _bulkStack.pop();
      
      // The bbox of the last child node has changed, so we update the
      // box here.
      size_t rightMostChildNodeId = 
        stackNode->GetIDAt( stackNode->GetNumberOfEntries() - 1 );
      RTreeNode<dim>* rightMostChildNode =
        ReadNode( rightMostChildNodeId );
      stackNode->SetValueAt( stackNode->GetNumberOfEntries() - 1,
        rightMostChildNode->GetBox() );
      
      // Make persisten
      WriteNode( *stackNode );
      
      delete rightMostChildNode;
      rightMostChildNode = NULL;
      
      // If the node has space available, we can add more elements.
      // The node goes to the stack again.
      if ( stackNode->GetNumberOfEntries() < stackNode->GetMax() ) {
        innerNode = stackNode;
        _bulkStack.push( stackNode );
        break;
      } else {
        delete stackNode;
        stackNode = NULL;
      } // else
    } // while

    // If we haven't found a node on the stack, that can handle more elements,
    // we have to create a new root node.
    if ( !innerNode ) {
      // Create new root node and attach old root node as child
      innerNode = CreateRTReeNode( false );
      
      RTreeNode<dim>* oldRootNode =
        ReadNode( m_treeHeader->GetRoot() );
      // Attach root node
      innerNode->AddEntry( oldRootNode->GetBox(),
        oldRootNode->GetNodeID() );
      // Set new root node in header
      m_treeHeader->SetRoot( innerNode->GetNodeID() );
      
      delete oldRootNode;
      oldRootNode = NULL;
      
      // Push the new root node to the stack
      _bulkStack.push( innerNode );
      // Every new root node increments the max. heigth of the tree
      _bulkMaxHeight++;
    } // if
    
    // Add new child node to the found or created node
    RTreeNode<dim>* newLeaf = CreateRTReeNode( true );
    innerNode->AddEntry( newLeaf->GetBox(), newLeaf->GetNodeID() );
    // Make persisten
    WriteNode( *innerNode );
    
    delete _curBulkNode;
    _curBulkNode = NULL;
    
    // The new created leaf is now the current leaf for adding
    // new elements.
    _curBulkNode = newLeaf;
  } // if
  
  // Adding the element to the current node
  _curBulkNode->AddEntry( box, id );
  // Make persisten
  WriteNode( *_curBulkNode );
}
// End bulkload-mode
template<int dim>
void RTree<dim>::EndBulkload() {  
  // traversing stack and adapt boxes
  while ( _bulkStack.size() > 0 ) {
    RTreeNode<dim>* lastNode = _bulkStack.top();
    RTreeNode<dim>* rightChild =
      ReadNode( 
        lastNode->GetIDAt( lastNode->GetNumberOfEntries() - 1 ) );
    
    lastNode->SetValueAt( lastNode->GetNumberOfEntries() - 1, 
      rightChild->GetBox() );
    // Make persisten
    WriteNode( *lastNode );
    
    // Remove node from stack an delete it
    _bulkStack.pop();
    
    delete rightChild;
    rightChild = NULL;
    
    delete lastNode;
    lastNode = NULL;
  } // while 

  if ( _curBulkNode ) {
    delete _curBulkNode;
    _curBulkNode = NULL;
  } // if
}



template<int dim>
void RTree<dim>::ToString(RTreeNode<dim>* node){
  bool printed = false;
  int i = 0;
  if(!node->IsLeaf()){
    while(i < node->GetNumberOfEntries()){
      if(printed == false){
        node->PrintNodeToString();
        printed = true;
      }

      ToString(ReadNode(node->GetIDAt(i)));
      cout<<"i. "<<i<<"  \n";
      i++;
    }
  }
  else{
    node->PrintNodeToString();
  }
}

template<int dim>
string RTree<dim>::ToString() {
  ostringstream o;
  o << "( tree " << endl;

  if (m_treeHeader->GetRoot() != 0){
    RTreeNode<dim>* root = ReadNode(m_treeHeader->GetRoot());
    PrintAsTree(o, *root, 0);
    delete(root);
    root = NULL;
  }
  else{
  o << "is empty " << endl;
  }

  o << ")";

  return o.str();
}

template<int dim>
void RTree<dim>::PrintAsTree(ostream& o, RTreeNode<dim>& node, size_t depth) {
  for (size_t i = 0; i < depth; i++){
    o << "    ";
  }

  o << "(";
  o << "' (values:(";
  for(size_t i = 0; i < node.GetNumberOfEntries(); i++) {
    if(i != 0) {
       o << ", ";
    }

    o << "(";
    for (size_t d = 0; d < dim; d++){
      o << node.GetValueAt(i).MinD(d) << ' ' << node.GetValueAt(i).MaxD(d);

      if (d + 1 < dim){
        o << ' ';
      }
    }
    o << ")";
  }

  o << "), ids:(";
  for(size_t i = 0; i < node.GetNumberOfEntries(); i++) {
    if(i != 0) {
       o << ", ";
    }

    o << node.GetIDAt(i);
  }

  o << ")) '";
  o << "(Id: " << node.GetNodeID() << " Box: ";
  o << "(";
  for (size_t i = 0; i < dim; i++){
    o << node.GetBox().MinD(i) << ' ' << node.GetBox().MaxD(i);

    if (i + 1 < dim){
      o << ' ';
    }
  }
  o << ")";
  o << " ValueCount: " << node.GetNumberOfEntries() << ")";

  if (!node.IsLeaf()) { // not a leaf node
    for (size_t i = 0; i < (node.GetNumberOfEntries()); i++) {
      RTreeNode<dim>* child = ReadNode(node.GetIDAt(i));

      o << endl;
      PrintAsTree(o, *child, depth + 1);

      delete(child);
      child = NULL;
    }
  }

  o << ")";
}

template class RTree<1>;
template class RTree<2>;
template class RTree<3>;
template class RTree<4>;
template class RTree<8>;

}// end of namespace fialgebra
























