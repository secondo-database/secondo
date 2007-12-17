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

November/December 2007, Mirko Dibbert

5.3 Implementation of class "MT::NodeMngr"[4] (file: MTreeNodeMngr.cpp)

*/
#include "MTreeNodeMngr.h"

/*
Initialise static member

*/
unsigned MT::NodeMngr::TaggedNode::m_tagCntr = 0;

/*
Method ~flush~ :

*/
void
MT::NodeMngr::flush()
{
  if (!m_nodes.empty())
  {
    map< SmiRecordId, TaggedNode >::iterator iter;

    #ifdef __MT_PRINT_NODE_CACHE_INFO
    cmsg.info() << endl << "Writing cached nodes to disc... ";
    cmsg.send();
    #endif

    for(iter = m_nodes.begin(); iter != m_nodes.end(); iter++)
    {
      iter->second.node->deleteIfAllowed();
    }
    m_nodes.clear();
    m_curSize = 0;

    #ifdef __MT_PRINT_NODE_CACHE_INFO
    cmsg.info() << " [DONE]" << endl;
    cmsg.send();
    #endif

    #ifdef __MT_PRINT_NODE_CACHE_INFO
    cmsg.info() << endl << "node-cache hits   : " << m_hits
                << endl << "node-cache misses : " << m_misses
                << endl << endl;
    cmsg.send();
    #endif
  } // if
} // flush

/*
Method ~insert~ :


*/
void
MT::NodeMngr::insert( MT::Node* node )
{
  #ifdef __MT_PRINT_NODE_CACHE_INFO
  m_misses++;
  #endif

  if ( m_useCache )
  {
    // insert new node into cache
    node->setCached( true );
    m_curSize += node->size();
    m_nodes[node->getNodeId()] = TaggedNode ( node->copy() );

    cleanup();
  }
}

/*
Method ~getNode~ :

*/
MT::Node*
MT::NodeMngr::getNode( SmiRecordId nodeId )
{
  map< SmiRecordId, TaggedNode >::iterator
      iter = m_nodes.find( nodeId );

  // search node in cache
  if ( iter != m_nodes.end() )
  {
    #ifdef __MT_PRINT_NODE_CACHE_INFO
    m_hits++;
    #endif

    iter->second.update();
    return iter->second.node->copy();
  }

  // node not found in cache
  Node* node = 0;
  try
  {
    node = new Node( m_file, m_maxNodeEntries, nodeId );
    insert( node );
  }
  catch (bad_alloc&)
  {
    cmsg.error() << "Not enough memory, disabling node cache... "
                 << endl;
    cmsg.send();
    disableCache();
    try
    {
      node = new Node( m_file, m_maxNodeEntries, nodeId );
    }
    catch (bad_alloc&)
    {
      cmsg.error() << "Not enough memory to create new node!"
                   << endl;
      cmsg.send();
    }
  }
  return node;
}

/*
Method ~createNode~ :

*/
MT::Node*
MT::NodeMngr::createNode()
{
  Node* node = 0;
  try
  {
    node = new Node( m_file, m_maxNodeEntries);
    insert( node );
  }
  catch (bad_alloc&)
  {
    cmsg.error() << "Not enough memory to create new node!"
                 << endl;
    cmsg.send();
    disableCache();
    try
    {
      node = new Node( m_file, m_maxNodeEntries);
    }
    catch (bad_alloc&)
    {
      cmsg.error() << "Not enough memory to create new node!";
      cmsg.send();
    }
  }
  return node;
}

/*
Method ~update~ :

*/
void
MT::NodeMngr::update( Node* node, bool sizeChanged )
{
  if ( node->isCached() )
  {
    m_curSize -= node->size();
    node->update( sizeChanged );
    m_curSize += node->size();
  }
  else
  {
    node->update( sizeChanged );
  }
}

/*
Method ~insert~ :

*/
bool
MT::NodeMngr::insert( Node* node, Entry *entry )
{
  if ( node->isCached() )
  {
    if ( node->insert( entry ) )
    {
      m_curSize += entry->size();
      cleanup();
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return node->insert( entry );
  }
}

/*
Method ~remove~ (with iterator) :

*/
void
MT::NodeMngr::remove( Node* node, vector<Entry*>::iterator iter )
{
  if ( node->isCached() )
  {
    m_curSize -= node->size();
    node->removeEntry( iter );
    m_curSize += node->size();
  }
  else
  {
    node->removeEntry( iter );
  }
}

/*
Method ~remove~ (with position index):

*/
void
MT::NodeMngr::remove( Node* node, size_t pos )
{
  if ( node->isCached() )
  {
    m_curSize -= node->size();
    node->removeEntry( pos );
    m_curSize += node->size();
  }
  else
  {
    node->removeEntry( pos );
  }
}

/*
Method ~initPath~ :

*/
void
MT::NodeMngr::initPath( SmiRecordId rootId )
{
  while ( !path.empty() )
    path.pop();

  while ( !indizes.empty() )
    indizes.pop();

  if ( m_parent )
    m_parent->deleteIfAllowed();

  if ( m_curNode )
    m_curNode->deleteIfAllowed();

  m_parent = 0;
  m_curNode = getNode( rootId );
}

/*
Method ~getChield~ :

*/
void
MT::NodeMngr::getChield( unsigned pos )
{
  path.push( m_curNode->getNodeId() );
  indizes.push ( pos );

  if ( m_parent )
    m_parent->deleteIfAllowed();

  m_parent = m_curNode;
  m_curNode = getNode( (*m_curNode->getEntries())[pos]->chield() );
}

/*
Method ~getParent~ :

*/
void
MT::NodeMngr::getParent()
{
  m_parentIndex = indizes.top();
  indizes.pop();
  path.pop();

  m_curNode->deleteIfAllowed();
  m_curNode = m_parent;

  if ( !path.empty() )
    m_parent = getNode( path.top() );
  else
    m_parent = 0;

}

/*
Method ~replacePromotedEntry~ :

*/
void
MT::NodeMngr::replacePromotedEntry( Entry* newEntry )
{
  if ( m_curNode->isCached() )
  {
    m_curSize -= m_curNode->size();
    m_curNode->replaceEntry( m_parentIndex, newEntry );
    m_curSize += m_curNode->size();
    cleanup();
  }
  else
  {
    m_curNode->replaceEntry( indizes.top(), newEntry );
  }
}

/*
Method ~hasChield~ :

*/
bool
MT::NodeMngr::hasChield()
{
  vector<Entry*>* entries = m_curNode->getEntries();
  if ( entries->empty() )
    return false;
  else if ( (*entries)[ 0 ]->chield() == 0 )
    return false;
  else
    return true;
}

/*
Method ~cleanup~ :

*/
void
MT::NodeMngr::cleanup()
{
  // delete old nodes from cache, if neccesary
  while ( m_curSize > NODE_CACHE_SIZE )
  {
    map< SmiRecordId, TaggedNode >::iterator
        oldest = m_nodes.begin();
    map< SmiRecordId, TaggedNode >::iterator iter;
    oldest = m_nodes.begin();
    for( iter = m_nodes.begin(); iter != m_nodes.end(); iter++ )
    {
      if ( iter->second.tag < oldest->second.tag)
      {
        oldest = iter;
      }
    }

    oldest->second.node->setCached( false );
    m_curSize -= oldest->second.node->size();
    oldest->second.node->deleteIfAllowed();
    m_nodes.erase( oldest );
  }
}
