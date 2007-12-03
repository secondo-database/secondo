/*
\newpage

2.4.3 Implementation part (file: MTNodeMngr.cpp)

*/
#include "MTNodeMngr.h"

/*
Initialise static member

*/
unsigned MT::NodeMngr::m_tagCntr = 0;

/*
Method ~insert~

*/
void
MT::NodeMngr::insert( MT::Node* node )
{
  #ifdef __MT_PRINT_NODE_CACHE_INFO
  m_misses++;
  #endif

  if ( m_nodes.size() < MAX_CACHED_NODES )
  {
    m_nodes[node->getNodeId()] = TaggedNode( node->copy() );
    return;
  }

  if ( !m_nodes.size() )
    return;

  // m_nodes.size() > m_rotingNodeCacheSize > 0
  unsigned oldest = m_nodes.begin()->second.tag;
  map< SmiRecordId, TaggedNode >::iterator oldestPos = m_nodes.begin();
  map< SmiRecordId, TaggedNode >::iterator iter;
  for(iter = m_nodes.begin(); iter != m_nodes.end(); iter++)
  {
    if ( iter->second.tag < oldest )
    {
      oldest = iter->second.tag;
      oldestPos = iter;
    }
  }

  oldestPos->second.node->deleteIfAllowed();

  m_nodes.erase( oldestPos );
  m_nodes[node->getNodeId()] = TaggedNode ( node->copy() );
}

/*
Method ~getNode~

*/
MT::Node*
MT::NodeMngr::getNode( SmiRecordId nodeId )
{
    map< SmiRecordId, TaggedNode >::iterator

    // search node in routing node cache
    iter = m_nodes.find(nodeId);
    if ( iter != m_nodes.end() )
    {
      #ifdef __MT_PRINT_NODE_CACHE_INFO
      m_hits++;
      #endif

      iter->second.tag = m_tagCntr++;
      return iter->second.node->copy();
    }

    // node not found in cache
    Node* node = new Node( m_file, m_maxNodeEntries, nodeId );
    insert( node );
    return node;
}

/*
Method ~createNode~ :

*/
MT::Node*
MT::NodeMngr::createNode()
{
  Node* node = new Node( m_file, m_maxNodeEntries);
  insert( node );
  return node;
}
