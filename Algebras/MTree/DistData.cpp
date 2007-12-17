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

5.9 Implementation of class "DistData"[4] (file: DistData.cpp)

November/December 2007, Mirko Dibbert

*/
#include "DistData.h"

/*
Initialise count of open objects:

*/
unsigned DistData::m_objectsOpen = 0;

/*
Assignment Operator :

*/
DistData&
DistData::operator=( const DistData& e )
{
  m_size = e.m_size;

  // copy m_value
  char* newValue = new char[e.m_size];
  memcpy( newValue, e.m_value, e.m_size );
  delete m_value;
  m_value = newValue;

  return *this;
}

/*
\newpage
Method ~write~ :

*/
void
DistData::write( char* buffer, int& offset ) const
{
  // write m_size
  memcpy( buffer+offset, &m_size, sizeof(size_t) );
  offset += sizeof(size_t);

  // write m_value
  memcpy( buffer+offset, m_value, m_size );
  offset += m_size;
}
