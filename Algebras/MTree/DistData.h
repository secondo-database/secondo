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
//paragraph [24]  table3columns:    [\begin{quote}\begin{tabular}{llll}][\end{tabular}\end{quote}]
//[--------]      [\hline]

4.3 Class ~DistData~ (file: DistData.h)

November/December 2007, Mirko Dibbert

4.3.1 Class Description

This class provides a data array, which contains all neccecary data for distance
computations with the metrics defined in class "MetricRegistry"[4].
\\[3ex]
This class provides the following constructors:

----
DistData( size_t size, const void* value )
----
Creates a new object with length "size"[4] and read it's value from "value"[4].

----
DistData( const string value )
----
Creates a new object from the string.

----
DistData( const char* buffer, int& offset )
----
Creates a new object and read it's size and value from
buffer, starting at position offset - offset will be increased.

----
DistData( const DistData& e )
----
Copy constructor.
\\[3ex]
This class provides the following methods:

[24]  getter    & I/O     & create/delete    & miscellaneous   \\
[--------]
  value         & write   & copy             & operator =      \\
  size          &         & deleteIfAllowed  & objectsOpen

4.3.2 Class definition

*/
#ifndef DISTDATA_H
#define DISTDATA_H

// #define __DISTDATA_DEBUG

#include <iostream>
#include <string>
#include "assert.h"

using namespace std;

class DistData
{

public:
  inline DistData( size_t size, const void* value )
  : m_size( size ), m_value( new char[m_size] ), m_refs( 1 )
  {
    memcpy( m_value, value , m_size );

    #ifdef __DISTDATA_DEBUG
    m_objectsOpen++;
    #endif
  }
/*
Constructor.

*/

  inline DistData( const string value )
  : m_size( value.size() ), m_value( new char[m_size] ), m_refs( 1 )
  {
    memcpy( m_value, value.c_str(), m_size );

    #ifdef __DISTDATA_DEBUG
    m_objectsOpen++;
    #endif
  }
/*
Constructor.

*/

  inline DistData( const char* buffer, int& offset )
  : m_refs ( 1 )
  {
    // read m_size
    memcpy( &m_size, buffer+offset, sizeof(size_t) );
    offset += sizeof(size_t);

    // read m_value
    m_value = new char[m_size];
    memcpy( m_value, buffer+offset, m_size );
    offset += m_size;

    #ifdef __DISTDATA_DEBUG
    m_objectsOpen++;
    #endif
  }
/*
Constructor.

*/

  inline DistData( const DistData& e )
  : m_size ( e.m_size ), m_value( new char[e.m_size] ), m_refs( 1 )
  {
    memcpy( m_value, e.m_value, e.m_size );

    #ifdef __DISTDATA_DEBUG
    m_objectsOpen++;
    #endif
  }
/*
Copy constructor.

*/

  inline ~DistData()
  {
    delete m_value;

    #ifdef __DISTDATA_DEBUG
    assert( !m_refs );
    m_objectsOpen--;
    #endif
  }
/*
Destructor.

*/

  inline const void* value() const
  {
    return m_value;
  }
/*
Returns a reference to the data array stored in m[_]value.

*/

  inline size_t size() const
  {
    return m_size;
  }
/*
Returns the size of the data array

*/

  inline DistData* copy()
  {
    if( m_refs < numeric_limits<unsigned char>::max() )
    {
      m_refs++;
      return this;
    }
    else
    {
      return new DistData( *this );
    }
  }
/*
Returns a pointer to the current object and increases the reference counter.
In case of counter overflow, a new copy of the object would be returned instead.

*/

  inline void deleteIfAllowed()
  {
    #ifdef __DISTDATA_DEBUG
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

  DistData& operator=( const DistData& e );
/*
Assignment Operator, copies the values from "data"[4] to the current object.

*/

  void write( char* buffer, int& offset ) const;
/*
Writes the object to the buffer at position offset. Offset is increased.

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
  size_t          m_size;        // length of the data array
  char*           m_value;       // contains the data array
  unsigned char   m_refs;        // reference counter
  static unsigned m_objectsOpen; // currently open objects

}; // class DistData

#endif
