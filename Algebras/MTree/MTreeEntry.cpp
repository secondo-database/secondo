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

5.5 Implementation of class "MT::Entry"[4] (file: MTreeEntry.cpp)

November/December 2007, Mirko Dibbert

*/
#include "MTreeEntry.h"

/*
Initialise count of open objects:

*/
size_t MT::Entry::m_objectsOpen = 0;

/*
Assignment Operator :

*/
MT::Entry&
MT::Entry::operator=( const MT::Entry& e )
{
  m_tid    = e.m_tid;
  m_dist   = e.m_dist;
  m_rad    = e.m_rad;
  m_chield = e.m_chield;

  // copy e.m_data
  DistData* tmp = new DistData( *e.m_data );
  delete m_data;
  m_data = tmp;

  return* this;
}

/*
\newpage
Method ~minSize~ :

*/
size_t
MT::Entry::minSize()
{
  return sizeof( TupleId )     + // tid
         sizeof( double )      + // dist
         sizeof( double )      + // rad
         sizeof( SmiRecordId ) + // chield
         sizeof( size_t );       // size of data object
}

/*
Method ~updateSize~ :

*/
void
MT::Entry::updateSize()
  {
    m_size = minSize() + m_data->size();
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

  // write data object
  m_data->write( buffer, offset );
}

/*
\newpage
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

  // read data object
  m_data = new DistData( buffer, offset );
}
