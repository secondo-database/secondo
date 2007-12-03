/*
\newpage

2.5.3 Implementation part (file: MTEntry.cpp)

*/
#include "MTEntry.h"

/*
Initialise static members :

*/

#ifdef __MT_DEBUG
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
  // write tid, dist, rad and chield
  memcpy( buffer+offset, this, MT_STATIC_ENTRY_SIZE );
  offset += MT_STATIC_ENTRY_SIZE;

  // write data string
  m_data->write( buffer, offset );
}

/*
Method ~read~ :

*/
void
MT::Entry::read( const char* const buffer, int& offset )
{
  // read tid, dist, rad and chield
  memcpy( this, buffer+offset, MT_STATIC_ENTRY_SIZE );
  offset += MT_STATIC_ENTRY_SIZE;

  // read data string
  m_data = new DistData( buffer, offset );
}
