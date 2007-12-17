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
//paragraph [25]  table4columns:    [\begin{quote}\begin{tabular}{lllll}][\end{tabular}\end{quote}]
//[--------]      [\hline]

2.4 Class ~MT::MTreeNode~ (file: MTNode.h)

November/December 2007, Mirko Dibbert

2.4.1 Class description

This class implements the nodes of the m-tree.

If the node cache should not be used, the constructors and
methods of this class could be used directly. Otherwhise
the methods of MT::NodeMngr should be used instead.
\\[3ex]
This class provides the following constructors:

----
Node( SmiRecordFile* file, size_t maxEntries )
----
Creates an empty node.

----
Node( SmiRecordFile* file, size_t maxEntries, SmiRecordId nodeId )
----
Reads the node from the record "nodeId"[4].
\newpage

This class provides the following methods:

[25]  info      & manipulate    & getter       & I/O   & create/delete    \\
[--------]
  emptySize     & insert        & getEntries   & write & copy             \\
  size          & removeNode    & getNodeId    & read  & deleteIfAllowed  \\
  getEntryCount & removeEntry   &              &       &                  \\
  isCached      & replaceEntry  &              &       &                  \\
  objectsOpen   & update        &              &       &                  \\
                & swapEntries   &              &       &                  \\
                & setCached     &              &

2.4.2 Class definition

*/
#ifndef MTREE_NODE_H
#define MTREE_NODE_H

#include "MTreeEntry.h"
namespace MT
{

class Node
{
public:
  inline Node( SmiRecordFile* file, size_t maxEntries )
  : m_file ( file ), m_modified( true ), m_maxEntries ( maxEntries ),
    m_curSize( emptySize() ), m_nodeId ( 0 ), m_extensions (),
    m_entries( new vector<Entry*>() ), m_refs( 1 ),
    m_cached( false )
  {
    #ifdef __MT_DEBUG
    MT::Node::m_objectsOpen++;
    #endif

    m_entries->reserve( 32 );
  }
/*
Constructor.

*/

  inline Node( SmiRecordFile* file, size_t maxEntries,
               SmiRecordId nodeId )
    : m_file ( file ), m_modified( false ),
      m_maxEntries ( maxEntries ), m_extensions (),
      m_entries( new vector<Entry*>() ), m_refs( 1 ),
      m_cached( false )
  {
    #ifdef __MT_DEBUG
    MT::Node::m_objectsOpen++;
    #endif

    read( nodeId );
  }
/*
Constructor.

*/

  ~Node();
/*
Destructor.

*/

  SmiRecordId getNodeId();
/*
This method returns the "SmiRecordId"[4] of the node. If no id exist, the id of
a new page in "m[_]file"[4] will be returned.

*/

  void removeNode();
/*
This method deletes all records of the node from the m-tree file.

*/

  void removeEntry( vector<Entry*>::iterator iter );
/*
This method removes the entry at position "iter"[4] from the node.

*/

  void removeEntry( size_t pos );
/*
This method removes the entry at position "pos"[4] from the node.

*/

  inline Node* copy()
  {
    if( m_refs < numeric_limits<unsigned char>::max() )
    {
      m_refs++;
      return this;
    }
    else
    {
      return new Node( *this );
    }
  }
/*
Returns a pointer to the current object and increases the reference counter.
In case of counter overflow, a new copy of the object would be returned instead.

*/

  inline void deleteIfAllowed()
  {
    #ifdef __MT_DEBUG
    assert( m_refs > 0 );
    #endif

    m_refs--;
    if ( !m_refs )
      delete this;
  }
/*
Decreases the reference counter and deletes the object
if no more references exist.

*/

  void update( bool sizeChanged = false );
/*
Sets the modified flag to true. If "changedSize == true"[4], the node size will
be recalculated and if nessecary, new extension pages will be added.

*/

  void replaceEntry( size_t pos, Entry* newValue );
/*
Replaces the entry at position "pos"[4] with "newValue"[4].

*/

  inline vector<Entry*>* getEntries()
  {
    return m_entries;
  }
/*
Returns the entry vector.

*/

  inline void swapEntries ( vector<Entry*>* entries )
  {
    entries->reserve( m_entries->capacity() );
    m_entries->swap( *entries );
  }
/*
Swaps the entry vector with a new one and reserves sufficient memory for it.

*/

   inline size_t getEntryCount()
   {
     return m_entries->size();
   }
/*
Returns the count of the currently stored entries.

*/

  bool insert( Entry *entry );
/*
Tries to insert an entry into the node and returns true, if succeed. Otherwhise
the entry will not be inserted and the node should be splitted.

If the node contains at most two entries it will allways insert the entry into the node. If
neccesary, extension pages will be appended to the m-tree file, to get enough
space to store the entry.

*/


  void read( SmiRecordId nodeId );
/*
Reads the node from page "nodeId"[4] in the m-tree file.

*/

  void write();
/*
Writes the node to page "nodeId"[4] in the m-tree file. If "nodeId"[4] = 0, a
new record will be appended to the file.

*/

  inline static size_t emptySize()
  {
    return sizeof( size_t ) + // m_entries->size()
           sizeof( size_t );  // m_extensions.size()
  }
/*
Returns the size of an empty node (used to initialize "m[_]curSize"[4]).

*/

  inline size_t size()
  {
    return ( sizeof(Node) + m_curSize - emptySize() +
           ( m_entries->capacity() * sizeof(Entry*) ) );
  }
/*
This method is used by "NodeMngr"[4] to update the size of the cached nodes.

*/

  inline bool isCached()
  {
    return m_cached;
  }
/*
This method returns true, if the node is stored in the cache of "NodeMngr"[4].

*/

  inline void setCached( bool cached )
  {
    m_cached = cached;
  }
/*
This method is used by "NodeMngr"[4] to indicate that the node is cached.

*/

  static inline size_t objectsOpen()
  {
    return m_objectsOpen;
  }
/*
This method returns the count of open objects (if "[_][_]DISTDATA[_]DEBUG"[4] is
not defined, this method will allways return 0).

*/

private:
  SmiRecordFile*    m_file;        // reference to the m-tree file
  bool              m_modified;    // modified flag
  unsigned          m_maxEntries;  // max. count of entries per node
  unsigned          m_curSize;     // current size of the node
  SmiRecordId       m_nodeId;      // record-id of the node in m_file
  list<SmiRecordId> m_extensions;  // id's of the extension pages
  vector<Entry*>*   m_entries;     // entries stored in this node.
  unsigned char     m_refs;        // reference counter
  bool              m_cached;      // true, if node is in the cache
  static unsigned   m_objectsOpen; // currently open objects

}; // class Node

} // namespace MTree

#endif
