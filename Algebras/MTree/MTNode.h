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

2.4 Class ~MTreeNode~

November 2007, Mirko Dibbert

2.4.1 Class description

TODO enter class description

2.4.2 Definition part (file: MTNode.h)

*/
#ifndef __MTREE_NODE_H
#define __MTREE_NODE_H

#include "MTEntry.h"
namespace MT
{

class Node
{

public:
  SmiRecordFile* m_file;  // reference to the m-tree file
  bool m_modified;      // true, if the node has been modified
  unsigned m_maxEntries;  // maximum count of entries per node
  unsigned m_curNodeSize; // current size of the node
  SmiRecordId m_nodeId; // record-id of the node in the m-tree file
  list<SmiRecordId> _extensions; // id's of the extension pages
  vector<Entry*>* m_entries;      // entries stored in this node.
  unsigned char m_refs;

  inline Node( SmiRecordFile* file, size_t maxEntries )
  : m_file ( file ), m_modified( true ), m_maxEntries ( maxEntries ),
    m_curNodeSize( emptySize() ), m_nodeId ( 0 ), _extensions (),
    m_entries( new vector<Entry*>() ), m_refs( 1 )
  {
    #ifdef __MT_DEBUG
    MT::Node::_created++;
    MT::Node::printDebugInfo( true );
    #endif

    m_entries->reserve(
        (NODE_PAGESIZE-emptySize()) / Entry::minSize() + 1 );
  }
/*
Constructor, creates a new node.

*/

  inline Node( SmiRecordFile* file, size_t maxEntries,
               SmiRecordId nodeId )
    : m_file ( file ), m_modified( false ),
      m_maxEntries ( maxEntries ), _extensions (),
      m_entries( new vector<Entry*>() ), m_refs( 1 )
  {
    #ifdef __MT_DEBUG
    MT::Node::_created++;
    MT::Node::printDebugInfo( true );
    #endif

    read( nodeId );
  }
/*
Constructor, reads the node from the record "nodeId"[4].

*/

  ~Node();
/*
Destructor.

*/

  SmiRecordId getNodeId()
  {
    if ( !m_nodeId )
    {
      SmiRecord record;
      m_file->AppendRecord( m_nodeId, record );
    }
    return m_nodeId;
  }

  void removeNode();
/*
This method deletes all records of the node from the m-tree file.

*/

  void remove( vector<Entry*>::iterator iter );
/*
Removes entry at position iter from the node.

*/

  void remove( size_t pos );
/*
Removes entry at position pos from the node.

*/

  inline Node* copy()
  {
    if( m_refs == numeric_limits<unsigned char>::max() )
    {
      return new Node( *this );
    }

    m_refs++;
    return this;
  }

  inline void deleteIfAllowed()
  {
#ifdef __MT_DEBUG
    assert( m_refs > 0 );
#endif
    --m_refs;
    if ( !m_refs )
      delete this;
  }

  void update( size_t pos, Entry* newValue );

  inline vector<Entry*>* getEntries()
  {
    return m_entries;
  }
/*
Returns the entry vector (used during split)

*/

  inline void swapEntries ( vector<Entry*>* entries )
  {
    m_entries->swap( *entries );
    m_entries->reserve(
        (NODE_PAGESIZE-emptySize()) / Entry::minSize() + 1);
  }
/*
Sets new entry vector and returns a pointer to the old vector.

*/

   inline size_t getEntryCount()
   {
     return m_entries->size();
   }
/*
Returns the count of the currently stored entries.

*/

  void modified( bool sizeChanged = false );
/*
Sets the modified flag to true. If "changedSize == true"[4], the node size will
be recalculated and if nessecary, new extension pages will be added.

*/

  bool insert( Entry *entry );
/*
Tries to inserts an entry into the node and returns true, if succeed. If the
method returns false, the node must be splitted.

If entries->size() < 2, the node will allways insert the entry into the node. If
neccesary, extension pages will be appended to the m-tree file, to get enough
space to store the entry.

*/


  void read( SmiRecordId nodeId );
/*
Reads the node from page "nodeId"[4] in the m-tree file.

*/

  void write();
/*
Writes the node to page "nodeId"[4] in the m-tree file.

*/

  inline static size_t emptySize()
  {
    return sizeof( size_t ) + // m_entries->size()
           sizeof( size_t ) + // _extensions.size()
           sizeof( bool );    // _leaf
  }
/*
Returns the size of an empty node (used to initialize "curNodeSize"[4] )

*/

/*
The following methods are implemented for debugging purposes:

*/
#ifdef __MT_DEBUG
private:
  static unsigned _created, _deleted;

public:
  static void printDebugInfo( bool detailed = false )
  {
    #ifdef __MT_PRINT_NODE_INFO
    cmsg.info() << "DEBUG_INFO <MT::NODE> : ";
    if ( detailed )
    {
      cmsg.info() << "objects created : "  << MT::Node::_created
                  << " - objects deleted: " << MT::Node::_deleted
                  << " - ";
    }
    cmsg.info() << "open objects : "  << MT::Node::objectsOpen()
                << endl;
    cmsg.send();
    #endif
  }

  static inline size_t objectsOpen()
  { return ( _created - _deleted ); }
#endif

}; // class Node

} // namespace MTree

#endif
