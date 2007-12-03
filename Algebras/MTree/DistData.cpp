/*
\newpage

3.4.3 Implementation part (file: DistData.cpp)

*/
#include "DistData.h"

/*
Initialisation of static members :

*/
#ifdef __DEBUG_DISTDATA
size_t DistData::m_created = 0;
size_t DistData::m_deleted = 0;
#endif

/*
Assignment Operator :

*/
DistData&
DistData::operator=( const DistData& e )
{
  m_size = e.m_size;
  char* newValue = new char[ e.m_size ];

  memcpy( newValue, e.m_value, e.m_size );
  delete m_value;
  m_value = newValue;

  return* this;
}

/*
Method ~write~ :

*/
void
DistData::write( char* buffer, int& offset ) const
{
  // write m_size
  memcpy( buffer + offset, &m_size, sizeof( size_t ) );
  offset += sizeof( size_t );

  // write m_value
  memcpy( buffer + offset, m_value, m_size );
  offset += m_size;
}
