/*
//[_] [\_]

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

3.4 Class ~DistData~

December 2007, Mirko Dibbert

3.4.1 Class description

This class contains a data array, which contains all neccecary data for
distance computations. For each metric, the respective objects will be
created with the "getDistData"[4] method of the corresponding attribute class.

3.4.2 Definition part (file: DistData.h)

*/
#ifndef __DISTDATA_H
#define __DISTDATA_H

// #define __DEBUG_DISTDATA

#include <iostream>
#include <string>
#include "assert.h"
#include "LogMsg.h"

class DistData
{
  size_t m_size;
  char* m_value;
  unsigned char m_refs;

public:
  inline DistData( size_t size, const void* value )
  : m_size( size ), m_value( new char[size] ), m_refs( 1 )
  {
    memcpy( m_value, value , m_size );

    #ifdef __DEBUG_DISTDATA
    DistData::m_created++;
    #endif
  }
/*
Constructor, creates a new object with length "size"[4] and read it's value
from "value"[4].

*/

  inline DistData( const char* buffer, int& offset )
  : m_refs ( 1 )
  {
    memcpy( &m_size, buffer + offset, sizeof(size_t) );
    offset += sizeof(size_t);

    m_value = new char[m_size];
    memcpy( m_value, buffer + offset, m_size );
    offset += m_size;

#ifdef __DEBUG_DISTDATA
    DistData::m_created++;
#endif
  }
/*
Read constructor, creates a new object and read it's size and value from
buffer, starting at position offset - offset is increased.

*/

  inline DistData( const string value )
  : m_size( value.size() ), m_value( new char[m_size] ), m_refs( 1 )
  {
    memcpy( m_value, value.c_str(), m_size );

#ifdef __DEBUG_DISTDATA
    DistData::m_created++;
#endif
  }
/*
Constructor, creates a new object from a string.

*/

  inline DistData( const DistData& e )
  : m_size ( e.m_size ), m_value( new char[e.m_size] ), m_refs( 1 )
  {
    memcpy( m_value, e.m_value, e.m_size );

#ifdef __DEBUG_DISTDATA
    DistData::m_created++;
#endif
  }
/*
Copy constructor.

*/

  inline ~DistData()
  {
    delete m_value;

#ifdef __DEBUG_DISTDATA
    assert( !m_refs );
    DistData::m_deleted++;
#endif
  }
/*
The Destructor.

*/

  inline DistData* copy()
  {
    if( m_refs == numeric_limits<unsigned char>::max() )
      return new DistData( *this );

    m_refs++;
    return this;
  }

  inline void deleteIfAllowed()
  {
    --m_refs;
    if ( !m_refs )
      delete this;
  }

  inline const void* value() const
  { return m_value; }
/*
Returns "m[_]value"[4].

*/

  inline size_t size() const
  { return m_size; }
/*
Returns "m[_]size"[4].

*/

  DistData& operator=( const DistData& e );
/*
Assignment Operator.

*/

  void write( char* buffer, int& offset ) const;
/*
Writes the data string to the buffer at position offset. Offset is increased.

*/

#ifdef __DEBUG_DISTDATA
/*
The following methods are implemented for debugging purposes:

*/
private:
  static size_t m_created, m_deleted;

public:
  static inline size_t created() const
  { return m_created; }

  static inline size_t deleted() const
  { return m_deleted; }

  static inline size_t openObjects() const
  { return ( m_created - m_deleted ); }
#endif

}; // class DistData

#endif
