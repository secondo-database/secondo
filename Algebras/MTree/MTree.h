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

//[_] [\_]
//characters      [1]   verbatim:   [$]   [$]
//characters      [2]   formula:    [$]   [$]
//characters      [3]   capital:    [\textsc{]  [}]
//characters      [4]   teletype:   [\texttt{]  [}]

2 The M-Tree Datastructure

November/December 2007, Mirko Dibbert

2.1 Overview

TODO enter datastructure description

2.2 Class ~MTree~ (file: MTree.h)

2.2.1 Class description

TODO enter class description

2.2.2 Class definition

*/
#ifndef MTREE_H
#define MTREE_H

#include "MTreeNodeMngr.h"
#include "MTreeSplitpol.h"
#include "MTreeConfig.h"
#include "StandardTypes.h"

namespace MT
{

struct SearchBestPathEntry
{
  SearchBestPathEntry( MT::Entry* entry_, double dist_,
                       unsigned index_ ) :
    entry( entry_ ),
    dist( dist_ ),
    index( index_ )
  {}

  MT::Entry* entry;
  double dist;
  unsigned index;
};
/*
This struct is used in the "insert"[4] method of "mtree"[4] when
searching for the best path to descent the tree.

*/

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
This struct is used in the "rangeSearch"[4] method of "mtree"[4].

*/

class MTree
{

public:
  MTree();
/*
Constructor, creates a new m-tree ("initialize"[4] method
must be called before the tree can be used).

*/

  MTree( const SmiFileId fileid );
/*
Constructor, opens an existing tree.

*/

  MTree( const MTree& mtree );
/*
Copy constructor

*/

  ~MTree();
/*
Destructor

*/

  void initialize( const string& tcName,
                   const string& metricName,
                   const string& configName );
/*
This method initializes a new created m-tree.

*/

  void deleteFile();
/*
This Method deletes the m-tree file.

*/

  inline SmiFileId getFileId()
  {
    return file.GetFileId();
  }
/*
This method returns the file id of the "SmiRecordFile"[4] containing the m-tree.

*/

  inline bool isInitialized()
  {
    return initialized;
  }
/*
Returns true, if the m-tree has been successfully initialized.

*/

  inline int getRoutingCount()
  {
    return header.routingCount;
  }
/*
Returns the count of all routing nodes.

*/

  inline int getLeafCount()
  {
    return header.leafCount;
  }
/*
Retunrs the count of all leafes.

*/

  inline int getEntryCount()
  {
    return header.entryCount;
  }
/*
Returns the count of all entries, stored in the mtree.

*/

  inline int getHeight()
  {
    return header.height;
  }
/*
Returns the height of the mtree.

*/

  void insert( Attribute* attr, TupleId tupleId );
/*
Inserts a new entry into the mtree.

*/

  inline void finalizeInsert()
  {
    nodeMngr->flush();
  }

  void rangeSearch( Attribute* attr, const double& searchRad,
                    list<TupleId>* results );

/*
Returns all entries in the tree, wich have a maximum distance of
"searchRad"[4] to the attribute "attr"[4] in the result list.

*/

void nnSearch( Attribute* attr, int nncount,
               list<TupleId>* results );
/*
Returns the "nncount"[4] nearest neighbours ot
the attribute "attr"[4] in the result list.

*/

  void printMTreeConfig();
/*
Writes the "config"[4] object to "cmsg.info()"[4].

*/

private:
  struct Header
  {
    STRING_T tcName;     // type name of the stored entries
    STRING_T metricName; // name of the used metric
    STRING_T configName; // name of the MTreeConfig object
    SmiRecordId root;    // page of the root node
    unsigned height;     // height of the mtree
    unsigned entryCount; // count of the entries stored in the mtree
    unsigned routingCount; // count of the mtree routing nodes
    unsigned leafCount;    // count of the mtree leafes

    Header() :
      root ( 0 ),
      height( 0 ),
      entryCount( 0 ),
      routingCount( 0 ),
      leafCount( 0 )
    {}
  };
/*
This struct contains all neccesary data, which is needed
to reinitialize a previously stored m-tree.

*/

  bool initialized;   // true, if the mtree has been initialized
  SmiRecordFile file; // contains the mtree file
  Header header;      // contains the header data
  Splitpol* splitpol; // reference to chosen split policy
  NodeMngr* nodeMngr; // reference to node-manager
  TMetric metric;     // reference to chosen metric
  TGetDataFun getDistData; // reference to respectiva getData method
  MTreeConfig config; // chosen MTreeConfig object

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

}; // MTree

} // namespace MTree

#endif
