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

5.4 Implementation of class "MT::Node"[4] (file: MTreeNode.cpp)

*/
#include "MTreeNode.h"

/*
Initialise count of open objects:

*/
unsigned MT::Node::m_objectsOpen = 0;

/*
Destructor :

*/
MT::Node::~Node()
{
  #ifdef __MT_DEBUG
  MT::Node::m_objectsOpen--;
  #endif

  if ( m_modified )
    write();

  for (size_t i = 0; i < m_entries->size(); i++)
    delete (*m_entries)[i];

  delete m_entries;
}

/*
Method ~getNodeId~ :

*/
SmiRecordId
MT::Node::getNodeId()
{
  if ( !m_nodeId )
  {
    SmiRecord record;
    m_file->AppendRecord( m_nodeId, record );
  }

  return m_nodeId;
}

/*
Method ~removeNode~ :

*/
void
MT::Node::removeNode()
{
  // remove extension pages
  list<SmiRecordId>::iterator iter;
  for ( iter = m_extensions.begin();
        iter != m_extensions.end(); iter++ )
  {
    m_file->DeleteRecord( *iter );
  }

  // remove header page
  m_file->DeleteRecord( m_nodeId );
  m_nodeId = 0;

  /* modified flag must be reseted, because the destructor would call
     the write method which would create a new record otherwhise.
  */
  m_modified = false;
}

/*
Method ~removeEntry~ (with iterator):

*/
void MT::Node::removeEntry( vector<Entry*>::iterator iter )
{
  m_curSize -= ( *iter )->size();
  delete *iter;
  *iter = m_entries->back();
  m_entries->pop_back();
  m_modified = true;
}

/*
Method ~removeEntry~ (with position index):

*/
void MT::Node::removeEntry( size_t pos )
{
  #ifdef __MT_DEBUG
  assert ( pos < m_entries->size() );
  #endif

  m_curSize -= (*m_entries)[pos]->size();
  delete (*m_entries)[pos];
  (*m_entries)[pos] = m_entries->back();
  m_entries->pop_back();
  m_modified = true;
}

/*
Method ~update~ :

*/
void MT::Node::update( bool sizeChanged )
{
  m_modified = true;
  if ( !sizeChanged )
    return;

  // recalculate curSize
  m_curSize = emptySize() +
      ( m_extensions.size() * sizeof(SmiRecordId) );

  vector<Entry*>::iterator iter;
  for (iter = m_entries->begin(); iter != m_entries->end(); iter++)
  {
    m_curSize += (*iter)->size();
  }

  // append extension pages, if nessecary
  while (m_curSize > ((m_extensions.size()+1) * NODE_PAGESIZE))
  {
    SmiRecordId rec_no;
    SmiRecord rec;
    m_file->AppendRecord( rec_no, rec );
    m_extensions.push_back( rec_no );
    m_curSize += sizeof(SmiRecordId);
  }
}

/*
Method ~replaceEntry~ :

*/
void MT::Node::replaceEntry( size_t pos, Entry* newValue )
{
  #ifdef __MT_DEBUG
  assert ( pos < m_entries->size() );
  #endif

  m_curSize -= (*m_entries)[pos]->size();
  m_curSize += newValue->size();
  delete (*m_entries)[pos];
  (*m_entries)[pos] = newValue;
  m_modified = true;
}

/*
Method ~insert~ :

*/
bool
MT::Node::insert( Entry* entry )
{
  if ( m_curSize + entry->size() >
      ( (m_extensions.size()+1) * NODE_PAGESIZE ) )
  {
    if ( m_entries->size() < 2 )
    {
      // insert entry, m_curSize curSize
      m_entries->push_back( entry );
      m_curSize += entry->size();
      m_modified = true;

      #ifdef __MT_DEBUG
      assert ( NODE_PAGESIZE > sizeof(SmiRecordId) );
      #endif

      /* Append extension pages until the the node is huge
         enough to store the entry. */
      while ( m_curSize > ((m_extensions.size()+1) * NODE_PAGESIZE) )
      {
        SmiRecordId rec_no;
        SmiRecord rec;
        m_file->AppendRecord( rec_no, rec );
        m_extensions.push_back( rec_no );
        m_curSize += sizeof(SmiRecordId);
      }
      return true;
    }
    else
    { // node contains already at least two entries
      return false;
    }
  }

  // do not insert, if the node contains already m_maxEntries nodes
  if ( m_entries->size() >= m_maxEntries )
    return false;

  if (m_entries->size() == 32)
    m_entries->reserve(
        (NODE_PAGESIZE-emptySize()) / Entry::minSize() + 1);

  // insert entry, m_curSize curSize
  m_entries->push_back( entry );
  m_curSize += entry->size();
  m_modified = true;
  return true;
}

/*
Method ~Write~ :

*/
void MT::Node::write()
{
  if( !m_modified )
    return;

  // open record - if no record exist, append a new record
  SmiRecord record;
  if ( m_nodeId )
    m_file->SelectRecord( m_nodeId, record, SmiFile::Update );
  else
    m_file->AppendRecord( m_nodeId, record );

  // remove unneccesary extension pages
  while ( m_curSize < ( m_extensions.size() * NODE_PAGESIZE ) )
  {
    m_file->DeleteRecord( m_extensions.back() );
    m_extensions.pop_back();
  }

  // create write buffer
  int offset = 0;
  int bufferSize =
      ( record.Size() * ( m_extensions.size()+1 ) ) +
      ( sizeof( SmiRecordId ) * m_extensions.size() );
  char buffer[ bufferSize ];

  // write number of extension nodeIds
  size_t count = m_extensions.size();
  memcpy( buffer+offset, &count, sizeof(size_t) );
  offset += sizeof(size_t);

  // write extension pointer list
  list<SmiRecordId>::iterator extIter;
  for (extIter = m_extensions.begin();
      extIter != m_extensions.end(); extIter++ )
  {
    memcpy( buffer+offset, &(*extIter), sizeof(SmiRecordId) );
    offset += sizeof(SmiRecordId);
  }

  // write number of stored  entries
  count = m_entries->size();
  memcpy( buffer+offset, &count, sizeof(size_t) );
  offset += sizeof(size_t);

  // write the entry array
  vector<Entry*>::iterator entryIter;
  for ( entryIter = m_entries->begin();
        entryIter != m_entries->end(); entryIter++)
  {
    (*entryIter)->write( buffer, offset );
  }

  // write header page
  record.Write( buffer, record.Size(), 0 );

  // write extensions, if exist
  offset = record.Size();
  for ( extIter = m_extensions.begin();
        extIter != m_extensions.end(); extIter++ )
  {
    m_file->SelectRecord( *extIter, record, SmiFile::Update );
    record.Write( buffer+offset, record.Size(), 0 );
    offset += record.Size();
  }

  // reset modified flag
  m_modified = false;
}

/*
Method ~Read~ :

*/
void
MT::Node::read( SmiRecordId nodeId  )
{
  // write old node data, if neccesary
  if ( m_modified )
    write();

  // reset node
  for (size_t i=0; i<m_entries->size(); i++)
    delete (*m_entries)[i];
  m_entries->clear();
  m_extensions.clear();
  m_curSize = emptySize();

  m_nodeId = nodeId;
  int offset = 0;

  // read header page
  SmiRecord record;
  m_file->SelectRecord( m_nodeId, record, SmiFile::ReadOnly );
  char headerBuf[record.Size()];
  record.Read( headerBuf, record.Size(), 0 );

  // read number of extension nodeIds
  size_t count;
  memcpy( &count, headerBuf, sizeof(size_t) );
  offset += sizeof(size_t);

  // create read buffer
  int bufferSize = ( record.Size() * ( count+1 ) ) +
                   ( sizeof(SmiRecordId) * count );
  char buffer[bufferSize];

  // copy header buffer into read buffer
  memcpy( buffer, headerBuf, record.Size() );

  // read extensions, if exist
  for ( size_t i = 1; i <= count; i++ )
  {
    SmiRecordId rec_no;
    memcpy( &rec_no, buffer+offset, sizeof(SmiRecordId) );
    m_extensions.push_back( rec_no );
    offset += sizeof(SmiRecordId);

    m_file->SelectRecord( rec_no, record, SmiFile::ReadOnly );
    record.Read( buffer+(i*record.Size()), record.Size(), 0 );
  }

  // read number of stored entries
  memcpy( &count, buffer+offset, sizeof( unsigned ) );
  offset += sizeof( unsigned );

  // read entry vector
  int old_offset = offset;
  m_entries->reserve( count );
  for (size_t pos = 0; pos < count; pos++)
  {
    m_entries->push_back( new Entry( buffer, offset ) );
  }

  // add  size of the entry vector to m_curSize
  m_curSize += ( offset - old_offset );
}
