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
//paragraph [23]  table4columns:    [\begin{quote}\begin{tabular}{lll}][\end{tabular}\end{quote}]
//[--------]      [\hline]

2.3  Class ~MT::NodeMngr~ (file: MTNodeMngr.h)

November/December 2007, Mirko Dibbert

2.3.1 Class description

This class provides a node cache and methods, which the constructors and some
of the methods of MT::Node. If no cache is needed, the methods of
MT::Node could be used directly.
\\[3ex]
This class provides the following methods:

[23]  navigate/build mtree & wrapper methods      & muscellaneous \\
[--------]
  initPath                 & replacePromotedEntry & getNode       \\
  curNode                  & update               & createNode    \\
  parent                   & insert               & usedCacheSize \\
  parentEntry              & remove               & disableCache  \\
  getChield                                                       \\
  getParent                                                       \\
  hasChield                                                       \\
  hasParent

2.3.2 Class definition

*/

#ifndef MTREE_NODE_MNGR_H
#define MTREE_NODE_MNGR_H

#include "MTreeNode.h"
#include <stack>

namespace MT
{

class NodeMngr
{

public:
  NodeMngr( SmiRecordFile* file, unsigned maxNodeEntries )
  : m_file ( file ), m_maxNodeEntries ( maxNodeEntries ),
    m_hits( 0 ), m_misses( 0 ), m_curSize( 0 ),
    m_useCache( NODE_CACHE_SIZE ), m_curNode( 0 ), m_parent( 0 )
  {}
/*
Constructor.

*/

  ~NodeMngr()
  {
    if ( m_parent )
      m_parent->deleteIfAllowed();

    if ( m_curNode )
      m_curNode->deleteIfAllowed();

    flush();
  }
/*
Destructor.

*/

  void flush();
/*
Removes all nodes from cache.

*/

  inline void disableCache()
  {
    flush();
    m_useCache = false;
  }
/*
Disables the node cache.

*/

  MT::Node* getNode( SmiRecordId nodeId );
/*
Returns the specified node and increases its ref-count.

*/

  MT::Node* createNode();
/*
Returns a new node and stores it into cache.

*/

  void update( Node* node, bool sizeChanged);
/*
Like "MT::Node::update"[4], but additionally updates current cache size.

*/

  bool insert( Node* node, Entry *entry );
/*
Like "MT::Node::insert"[4], but additionally updates current cache size.

*/

  void remove( Node* node, vector<Entry*>::iterator iter );
/*
Like "MT::Node::remove"[4], but additionally updates current cache size.

*/

  void remove( Node* node, size_t pos );
/*
Like "MT::Node::remove"[4], but additionally updates current cache size.

*/

  inline unsigned usedCacheSize()
  {
    return m_curSize;
  }
/*
Returns current cache size.

*/

  void initPath( SmiRecordId rootId );
/*
Initiates a new path from root "rootId"[4].

*/

  inline MT::Node* curNode()
  {
    return m_curNode;
  }
/*
Returns the pointer to the last node in "path"[4].

*/

  inline MT::Node* parent()
  {
    return m_parent;
  }
/*
Returns the pointer to the parent of "curNode()"[4].

*/

  inline MT::Entry* parentEntry()
  {
    vector<Entry*>* entries = m_parent->getEntries();
    return (*entries)[indizes.top()];
  }
/*
Returns the pointer the routing entry to "curEntry()"[4].

*/

  void getChield( unsigned pos );
/*
Replaces "m[_]parent"[4] with "m[_]curNode"[4] and "m[_]curNode"[4] with its chield
at position "pos"[4].

*/

  void getParent();
/*
Replaces "m[_]curNode"[4] with its parent.


*/

  void replacePromotedEntry( Entry* newEntry );
/*
Replaces the last routing entry with "newEntry"[4].

*/

  bool hasChield();
/*
True, if "curNode()"[4] is not a leaf.

*/

  inline bool hasParent()
  {
    return ( m_parent != 0 );
  }
/*
True, if "curNode()"[4] is not the root.

*/

private:
  struct TaggedNode
  {
    TaggedNode()
    : node ( 0 ), tag ( 0 ) {}

    TaggedNode( Node* node_ )
    : node ( node_ ), tag ( m_tagCntr++ ) {}

    Node* node;
    unsigned tag;
    static unsigned m_tagCntr;

    inline void update()
    {
      tag = m_tagCntr++;
    }
  };
/*

This struct is needed to store a node together with a tag, wich is used to
determine the oldest nodes if a node in the cache must be replaced with a
newer one.

*/

  map< SmiRecordId, TaggedNode > m_nodes; // the node cache
  SmiRecordFile* m_file;     // reference to the m-tree file
  unsigned m_maxNodeEntries; // maximum entries per node
  unsigned m_hits, m_misses; // statistic infos
  unsigned m_curSize;        // size of all cached nodes
  bool m_useCache; // true, if the node cache should be used

/*
The following members are needed when navigating through the mtree with the
getChield and getParent methods (used by the insert and split method of
"MT::MTree"[4]).

*/
  stack<SmiRecordId> path;
  stack<unsigned> indizes;
  MT::Node* m_curNode;
  MT::Node* m_parent;
  unsigned m_parentIndex;

  void insert( MT::Node* node );
/*
Inserts the node into the cache. If neccesary,
the oldest cached node will be replaced.

*/

  void cleanup();
/*
Removes at least one node from cache, if a cachesize-overflow has been occured.

*/

}; // class NodeMngr

} // namespace MTree

#endif
