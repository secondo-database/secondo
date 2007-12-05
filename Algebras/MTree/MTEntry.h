/*
//paragraph [24]  table4columns:    [\begin{quote}\begin{tabular}{llll}][\end{tabular}\end{quote}]
//[--------]      [\hline]

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

2.5 Class ~MT::Entry~

November 2007, Mirko Dibbert

2.5.1 Class description

This class manages the entries, stored in the m-tree nodes and contains the
following member functions:

[24]  getter    & setter      & I/O     & miscellaneous \\
[--------]
      dist()    & setDist()   & write() & minSize()     \\
      rad()     & setRad()    & read()  & size()        \\
      chield()  & setChield() &         &               \\
      tid()     & setTid()    &         &               \\
      data()    &             &         &

The write method is used in the write method of the "node"[4] class to write
the entry into a buffer. The read method could only be used by calling the
appropriate constructor. The size method is used in the "node"[4] class to
calculate the current size of the node. The minSize method is only needed to
reserve enough space for the entry vector in the "node"[4] class.

Furthermore this class offers two constructors: The first one is used to create
a new entry. The second constructor reads the entry from a buffer and is used by
the read function of the "node"[4] class.

2.5.2 Definition part (file: MTEntry.h)

*/
#ifndef __MT_ENTRY_H
#define __MT_ENTRY_H

#include "MTreeAlgebra.h"

namespace MT
{

class Entry
{
private:
  TupleId m_tid;        // tuple-id of the entry
  double m_dist;        // distance to parent node
  double m_rad;         // covering radius
  SmiRecordId m_chield; // pointer to chield node
  DistData* m_data;     // data string for distance computations
  unsigned m_size;      // actual size of the entry

  void read( const char* const buffer, int& offset );
/*
This method reads the entry from "buffer"[4] and increases "offset"[4].

*/

  inline void updateSize()
  {
    m_size = sizeof( TupleId ) +    // tid
             sizeof( double )  +    // dist
             sizeof( double )  +    // rad
             sizeof( SmiRecordId )+ // chield
             sizeof( size_t ) +      // size of data string
             m_data->size();         // data string
  }
/*
This method calculates the actual size of the node.

*/

public:
  inline Entry( const TupleId tid, DistData* data )
  : m_tid( tid ), m_dist( 0 ), m_rad ( 0 ), m_chield( 0 ),
    m_data( data )
  {
    #ifdef __MT_DEBUG
    MT::Entry::m_created++;
    MT::Entry::printDebugInfo( true );
    assert ( data );
    #endif

    updateSize();
  }
/*
Constructor, creates a new entry object with given tuple id and DistData object.

*/

  inline Entry( const char* const buffer, int& offset )
  {
    #ifdef __MT_DEBUG
    MT::Entry::m_created++;
    MT::Entry::printDebugInfo( true );
    #endif

    read( buffer, offset );
    updateSize();
  }
/*
Constructor, reads a previously stored entry from "buffer"[4].

*/

  inline Entry( const Entry& e )
  : m_tid( e.m_tid ), m_dist( e.m_dist ), m_rad ( e.m_rad ),
    m_chield ( e.m_chield ), m_data( e.m_data->copy() ),
    m_size( e.m_size )
  {
    #ifdef __MT_DEBUG
    MT::Entry::m_created++;
    MT::Entry::printDebugInfo( true );
    #endif
  }
/*
Copy constructor.

*/

  inline ~Entry()
  {
    m_data->deleteIfAllowed();

    #ifdef __MT_DEBUG
    MT::Entry::m_deleted++;
    MT::Entry::printDebugInfo( true );
    #endif
  }
/*
Destructor.

*/

  inline const DistData* data() const
  { return m_data; }
/*
Returns the data object.

*/

  inline TupleId tid() const
  { return m_tid; }
/*
Returns the tid value.

*/

  inline double dist() const
  { return m_dist; }
/*
Returns distance to parent node.

*/

  inline double rad() const
  { return m_rad; }
/*
Returns the covering radius.

*/

  inline SmiRecordId chield() const
  { return m_chield; }
/*
Returns record id of the chield node.

*/

  inline void setTid( TupleId tid )
  { m_tid = tid; }
/*
Sets a new value for the tuple id.

*/

  inline void setDist( const double& dist )
  { m_dist = dist; }
/*
Sets distance to parent node.

*/
  inline void setRad( const double& rad )
  { m_rad = rad; }
/*
Sets a new covering radius.

*/

  void setChield( const SmiRecordId chield )
  { m_chield = chield; }
/*
Sets a new chield node.

*/

  Entry& operator=( const Entry& e );
/*
Assignment operator.

*/

  static size_t minSize();
/*
This method returns the minimal size of an entry on disc and is used in the
constructors of the "Node"[4] class to reserve an adequate amount of memory for
the entry vector.

*/

  size_t size() const;
/*
This method returns the actual size of the entry on disc.

*/

  void write( char* const buffer, int& offset ) const;
/*
This method writes the entry to "buffer"[4] and increases "offset"[4].

*/

/*
The following methods are implemented for debugging purposes:

*/
#ifdef __MT_DEBUG
private:
  static unsigned m_created, m_deleted;

public:
  static void printDebugInfo( bool detailed = false )
  {
  #ifdef __MT_PRINT_ENTRY_INFO
    cmsg.info() << "DEBUG_INFO <MT::ENTRY> : ";
    if ( detailed )
    {
      cmsg.info() << "objects created : "  << MT::Entry::m_created
                  << " - objects deleted: " << MT::Entry::m_deleted
                  << " - ";
    }
    cmsg.info() << "open objects : "  << MT::Entry::objectsOpen()
                << endl;
    cmsg.send();
  #endif
  }

  static inline size_t objectsOpen()
  { return ( m_created - m_deleted ); }
#endif

}; // class Entry

} // namespace MTree

#endif
