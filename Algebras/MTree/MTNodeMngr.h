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

2.3  Class ~NodeMngr~

December 2007, Mirko Dibbert

2.3.1 Class description

TODO enter class description

2.3.1 Definition part (file: MTNodeMngr.h)

*/

#ifndef __MTREE_NODE_MNGR_H
#define __MTREE_NODE_MNGR_H

#include "MTNode.h"

namespace MT
{

class NodeMngr
{
  struct TaggedNode
  {
    TaggedNode()
    : node ( 0 ), tag ( 0 ) {}

    TaggedNode( Node* node_ )
    : node ( node_ ), tag ( NodeMngr::m_tagCntr++ ) {}

    Node* node;
    unsigned tag;
  };

  map< SmiRecordId, TaggedNode > m_nodes;
  SmiRecordFile* m_file;
  unsigned m_maxNodeEntries;
  unsigned m_hits, m_misses;
  static unsigned m_tagCntr;

  void insert( MT::Node* node );
/*
Inserts the node into the cache. If neccesary, the oldest cached node will be
replaced.

*/

public:
  NodeMngr( SmiRecordFile* file, unsigned maxNodeEntries )
  : m_file ( file ), m_maxNodeEntries ( maxNodeEntries ),
    m_hits( 0 ), m_misses( 0 )
  {}
/*
Constructor.

*/

  ~NodeMngr()
  {
    map< SmiRecordId, TaggedNode >::iterator iter;

    for(iter = m_nodes.begin(); iter != m_nodes.end(); iter++)
      delete iter->second.node;
    m_nodes.clear();

    #ifdef __MT_PRINT_NODE_CACHE_INFO
    cout << "\nnode-cache hits: " << m_hits
         << ", node-cache misses: " << m_misses << endl;
    #endif
  }
/*
Destructor.

*/

  MT::Node* getNode( SmiRecordId nodeId );
/*
Returns the specified node and increases its ref-count.

*/

  MT::Node* createNode();
/*
Returns a new node and stores it into cache.

*/

}; // class NodeMngr

} // namespace MTree

#endif
