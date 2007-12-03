/*
\newpage

2.4.3 Implementation part (file: MTNode.cpp)

*/
#include "MTNode.h"

#ifdef __MT_DEBUG
size_t MT::Node::_created = 0;
size_t MT::Node::_deleted = 0;
#endif

/*
Destructor :

*/
MT::Node::~Node()
{
#ifdef __MT_DEBUG
  MT::Node::_deleted++;
  MT::Node::printDebugInfo( true );
#endif

  if ( m_modified )
    write();

  for (size_t i=0; i<m_entries->size(); i++)
    delete (*m_entries)[i];

  delete m_entries;
}

/*
Method ~removeNode~ :

*/
void MT::Node::removeNode()
{
  list<SmiRecordId>::iterator iter;
  for ( iter = _extensions.begin();
        iter != _extensions.end(); iter++ )
  {
    m_file->DeleteRecord( *iter );
  }

  m_file->DeleteRecord( m_nodeId );
  m_nodeId = 0;
  m_modified = false;
}

/*
Method ~remove~ :

*/
void MT::Node::remove( vector<Entry*>::iterator iter )
{
  m_curNodeSize -= ( *iter )->size();
  delete *iter;
  *iter = m_entries->back();
  m_entries->pop_back();
  m_modified = true;
}

void MT::Node::remove( size_t pos )
{
#ifdef __MT_DEBUG
  assert ( pos < m_entries->size() );
#endif
  m_curNodeSize -= (*m_entries)[pos]->size();
  delete (*m_entries)[ pos ];
  (*m_entries)[pos] = m_entries->back();
  m_entries->pop_back();
  m_modified = true;
}

/*
Method ~modified~ :

*/
void MT::Node::modified( bool sizeChanged )
{
  m_modified = true;

  if ( !sizeChanged )
    return;

  // recalculate curSize
  m_curNodeSize = emptySize();
  vector<Entry*>::iterator iter;
  for (iter = m_entries->begin(); iter != m_entries->end(); iter++)
  {
    m_curNodeSize += (*iter)->size();
  }

  // append extension pages, if nessecary
  while (m_curNodeSize > ((_extensions.size()+1) * NODE_PAGESIZE))
  {
    SmiRecordId rec_no;
    SmiRecord rec;
    m_file->AppendRecord( rec_no, rec );
    _extensions.push_back( rec_no );
    m_curNodeSize += sizeof( SmiRecordId );
  }
}

/*
Method ~insert~ :

*/
bool
MT::Node::insert( Entry* entry )
{
  if ( m_entries->size() >= m_maxEntries )
    return false;

  size_t newSize = m_curNodeSize + entry->size();
  if ( newSize > ((_extensions.size()+1) * NODE_PAGESIZE) )
  {
    if ( m_entries->size() < 2 )
    {
      /* Append extension nodeId(s) until the the node is huge
         enough to store the entry. */
#ifdef __MT_DEBUG
      assert ( NODE_PAGESIZE > sizeof( SmiRecordId ) );
#endif
      while (newSize > ((_extensions.size()+1) * NODE_PAGESIZE))
      {
        SmiRecordId rec_no;
        SmiRecord rec;
        m_file->AppendRecord( rec_no, rec );
        _extensions.push_back( rec_no );
        newSize += sizeof( SmiRecordId );
      }

      // insert entry, update curSize
      m_entries->push_back( entry );
      m_curNodeSize = newSize;
      m_modified = true;
      return true;
    }
    else
    {
      return false;
    }
  }

  // insert entry, update curSize
  m_entries->push_back( entry );
  m_curNodeSize = newSize;
  m_modified = true;
  return true;
}

/*
Method ~update~ :

*/
void MT::Node::update( size_t pos, Entry* newValue )
{
#ifdef __MT_DEBUG
  assert ( pos < m_entries->size() );
#endif
  m_curNodeSize -= (*m_entries)[ pos ]->size();
  m_curNodeSize += newValue->size();
  delete (*m_entries)[ pos ];
  (*m_entries)[ pos ] = newValue;
  m_modified = true;
}

/*
Method ~Write~ :

*/
void MT::Node::write()
{
  if( !m_modified )
    return;

  // open record, if needed append a new page
  SmiRecord record;
  if ( m_nodeId )
    m_file->SelectRecord( m_nodeId, record, SmiFile::Update);
  else
    m_file->AppendRecord( m_nodeId, record);

  // remove unneccesary extension nodeIds
  while ( m_curNodeSize < ( _extensions.size() * NODE_PAGESIZE ) )
  {
    m_file->DeleteRecord( _extensions.back() );
    _extensions.pop_back();
  }

  // create write buffer
  int offset = 0;
  int bufferSize =
      ( record.Size() * ( _extensions.size()+1 ) ) +
      ( sizeof( SmiRecordId ) * _extensions.size() );
  char buffer[ bufferSize ];

  // write number of extension nodeIds
  unsigned count = _extensions.size();
  memcpy( buffer+offset, &count, sizeof( unsigned ) );
  offset += sizeof( unsigned );

  // write extension pointer list
  list<SmiRecordId>::iterator extIter;
  for (extIter = _extensions.begin();
      extIter != _extensions.end(); extIter++ )
  {
    memcpy( buffer+offset, &(*extIter), sizeof( SmiRecordId ) );
    offset += sizeof( SmiRecordId );
  }

  // write number of stored  entries
  count = m_entries->size();
  memcpy( buffer+offset, &count, sizeof( size_t ) );
  offset += sizeof( size_t );

  // write the entry array
  vector<Entry*>::iterator entryIter;
  for ( entryIter = m_entries->begin();
        entryIter != m_entries->end(); entryIter++)
  {
    (*entryIter)->write( buffer, offset );
  }

  record.Write( buffer, record.Size(), 0 );

  // write extensions, if exist
  offset = record.Size();
  for ( extIter = _extensions.begin();
        extIter != _extensions.end(); extIter++ )
  {
    m_file->SelectRecord( *extIter, record, SmiFile::Update );
    record.Write(buffer+offset, record.Size(), 0);
    offset += record.Size();
  }

  // update modified flag
  m_modified = false;
}

/*
Method ~Read~ :

*/
void
MT::Node::read( SmiRecordId nodeId  )
{
  if ( m_modified )
    write();

  m_curNodeSize = emptySize();

  SmiRecord record;
  m_nodeId = nodeId;

  // read node (header nodeId)
  char extensionsCountBuf[sizeof( size_t )];
  m_file->SelectRecord( m_nodeId, record, SmiFile::ReadOnly );
  record.Read( extensionsCountBuf, sizeof( size_t ), 0 );

  // read number of extension nodeIds
  size_t extensionsCount;
  memcpy( &extensionsCount, extensionsCountBuf, sizeof( size_t ) );

  int offset = sizeof( size_t );
  int bufferSize =
      ( record.Size() * ( extensionsCount+1 ) ) +
      ( sizeof( SmiRecordId ) * extensionsCount );
  char buffer[ bufferSize ];

  // read node (header nodeId)
  record.Read( buffer, record.Size(), 0 );

  // read extensions, if exist
  int nodeIdoffset = record.Size();
  _extensions.clear();
  for ( size_t i = 0; i < extensionsCount; i++ )
  {
    SmiRecordId rec_no;
    memcpy( &rec_no, buffer+offset, sizeof( SmiRecordId ) );
    _extensions.push_back( rec_no );
    offset += sizeof( SmiRecordId );

    m_file->SelectRecord( rec_no, record, SmiFile::ReadOnly );
    record.Read( buffer+nodeIdoffset, record.Size(), 0 );
    nodeIdoffset += record.Size();
  }

  // read number of stored entries
  unsigned count;
  memcpy( &count, buffer+offset, sizeof( unsigned ) );
  offset += sizeof( unsigned );

  // delete currently stored entries
  for (size_t i=0; i<m_entries->size(); i++)
    delete (*m_entries)[i];
  m_entries->clear();

  // read the entry array.
  int old_offset = offset;
  m_entries->reserve(
      (NODE_PAGESIZE-emptySize()) / Entry::minSize() );

  unsigned pos = 0;
  while( pos < count )
  {
    pos++;
    m_entries->push_back(new Entry( buffer, offset ) );
  }

  // update size and modified flag
  m_curNodeSize += ( offset - old_offset );
  m_modified = false;

} // read
