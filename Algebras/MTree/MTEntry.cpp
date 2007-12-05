/*
\newpage

2.5.3 Implementation part (file: MTEntry.cpp)

*/
#include "MTEntry.h"

/*
Initialise static members :

*/

#ifdef m__MT_DEBUG
size_t MT::Entry::m_created = 0;
size_t MT::Entry::m_deleted = 0;
#endif

/*
Assignment Operator :

*/
MT::Entry&
MT::Entry::operator=( const MT::Entry& e )
{
  m_tid = e.m_tid;
  m_dist = e.m_dist;
  m_rad = e.m_rad;
  m_chield = e.m_chield;

  // copy e.m_data
  DistData* tmp = new DistData( *e.m_data );
  if ( m_data )
    delete m_data;
  m_data = tmp;

  return* this;
}

/*
Method ~minSize~ :

*/
size_t
MT::Entry::minSize()
{
  return sizeof( TupleId ) +    // tid
         sizeof( double )  +    // dist
         sizeof( double )  +    // rad
         sizeof( SmiRecordId ); // chield
}

/*
Method ~size~ :

*/
size_t
MT::Entry::size() const
{
  return m_size;
}

/*
Method ~write~ :

*/
void
MT::Entry::write( char* const buffer, int& offset ) const
{
  // write tuple-id
  memcpy( buffer+offset, &m_tid, sizeof( TupleId ) );
  offset += sizeof( TupleId );

  // write distance to parent node
  memcpy( buffer+offset, &m_dist, sizeof( double ) );
  offset += sizeof( double );

  // write covering radius
  memcpy( buffer+offset, &m_rad, sizeof( double ) );
  offset += sizeof( double );

  // write pointer to chield node
  memcpy( buffer+offset, &m_chield, sizeof( SmiRecordId ) );
  offset += sizeof( SmiRecordId );

  // write data string
 m_data->write( buffer, offset );
}

/*
Method ~read~ :

*/
void
MT::Entry::read( const char* const buffer, int& offset )
{
  // read tuple-id
  memcpy( &m_tid, buffer+offset, sizeof( TupleId ) );
  offset += sizeof( TupleId );

  // read distance to parent node
  memcpy( &m_dist, buffer+offset, sizeof( double ) );
  offset += sizeof( double );

  // read covering radius
  memcpy( &m_rad, buffer+offset, sizeof( double ) );
  offset += sizeof( double );

  // read pointer to chield node
  memcpy( &m_chield, buffer+offset, sizeof( SmiRecordId ) );
  offset += sizeof( SmiRecordId );

  // read data string
  m_data = new DistData( buffer, offset );
}
