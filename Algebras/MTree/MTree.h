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

2 The M-Tree Datastructure

December 2007, Mirko Dibbert

2.1 Overview

TODO enter datastructure description

2.2 Class ~MTree~

2.2.1 Class description

TODO enter class description

2.2.2 Definition part (file: MTree.h)

*/
#ifndef __MTREE_H
#define __MTREE_H

#include "MTNodeMngr.h"
#include "MTSplitpol.h"
#include "MTreeConfig.h"

namespace MT
{

class MTree
{
  struct Header
  {
    STRING_T tcName;     // type name of the stored entries
    STRING_T metricName; // name of the used metric
    STRING_T configName; // name of the MTreeConfig object
    SmiRecordId root;    // page of the root node
    unsigned height;
    unsigned entryCount;
    unsigned routingCount;
    unsigned leafCount;

    Header() :
      root ( 0 ),
      height( 0 ),
      entryCount( 0 ),
      routingCount( 0 ),
      leafCount( 0 )
    {}
  }; // struct Header
/*
This struct contains all neccesary data to reinitialize a previously stored
m-tree.

*/

  bool initialized;
  SmiRecordFile file;
  Header header;
  Splitpol* splitpol;
  NodeMngr* nodeMngr;

  TMetric metric;
  MTreeConfig config;
  vector<SmiRecordId> path;
  vector<unsigned> indizes;
  Node* nodePtr;

struct RemainingNodesEntry
{
  SmiRecordId nodeId;
  unsigned deepth;
  double dist;

  RemainingNodesEntry( SmiRecordId nodeId_, size_t deepth_,
                      double dist_ ) :
    nodeId( nodeId_ ),
    deepth( deepth_ ),
    dist ( dist_ )
  {}

};
/*
This struct is used in the "rangeSearch"[4] method as path entry

*/

struct SearchBestPathEntry
{
  SearchBestPathEntry( Entry* entry_, double dist_,
                       unsigned index_ ) :
    entry( entry_ ),
    dist( dist_ ),
    index( index_ )
  {}

  Entry* entry;
  double dist;
  unsigned index;
};
/*
This struct is used in the "insert"[4] method when searching for the best path
to descent the tree.

*/

  void readHeader();
/*
Reads the header from file.

*/

  void writeHeader();
/*
Writes the header to file.

*/


  void split(Entry* entry );
/*
Splits an node by applying the split policy defined in the MTreeConfing object.

*/

public:
  MTree();
/*
Constructor, creates a new m-tree ("initialize"[4] method must be called before
the tree can be used).

*/

  MTree( const SmiFileId fileid );
/*
Constructor, opens an existing tree.

*/

  ~MTree();
/*
Destructor

*/

  void initialize( const Attribute* attr,
                   const string tcName,
                   const string metricName,
                   const string configName );
/*
This method initializes a new created m-tree.

*/

  void deleteFile();
/*
This Method deletes the m-tree file.

*/

  DistData* getDistData( Attribute* attr );
/*
Returns a new DistData object which will be created from a CcInt, CcReal or
CcString object or obtained from the getDistData method of attr.

*/

  inline SmiFileId getFileId()
  {
    return file.GetFileId();
  }
/*
This method returns the file id of the "SmiRecordFile"[4] containing the m-tree.

*/

  inline bool isInitialized()
  { return initialized; }
/*
Returns true, if the m-tree has been successfully initialized.

*/

  void insert( Attribute* attr, TupleId tupleId );
/*
Inserts a new entry into the tree.

*/

  void rangeSearch( Attribute* attr, const double& searchRad,
                    list<TupleId>* results );

/*
Returns all entries in the tree, wich have a maximum distance of "searchRad"[4]
to the attribute "attr"[4] in the result list.

*/

void nnSearch( Attribute* attr, int nncount,
               list<TupleId>* results );
/*
k-nearest-neighbour search

*/

}; // MTree

} // namespace MTree

#endif
