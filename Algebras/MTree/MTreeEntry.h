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
//paragraph [24]  table4columns:    [\begin{quote}\begin{tabular}{llll}][\end{tabular}\end{quote}]
//[--------]      [\hline]

2.5 Class ~MT::Entry~ (file: MTreeEntry.h)

November/December 2007, Mirko Dibbert

2.5.1 Class description

This class implements the entries, stored in the m-tree nodes.
\\[3ex]
This class provides the following constructors:

----
Entry( const TupleId tid, DistData* data )
----
Creates a new entry object with given tuple id and DistData object.

----
Entry( const char* const buffer, int& offset )
----
Reads a previously stored entry from "buffer"[4].

----
Entry( const Entry& e )
----
Copy constructor.
\\[3ex]
This class provides the following methods:

[24]  getter  & setter    & I/O   & info         \\
[--------]
      dist    & setDist   & write & minSize       \\
      rad     & setRad    & read  & size          \\
      chield  & setChield &       & objectsOpen   \\
      tid     & setTid    &       &               \\
      data    &           &       &

The write method is used in the write method of the "node"[4] class to write
the entry into a buffer. The read method could only be used by calling the
appropriate constructor. The size method is used in the "node"[4] class to
calculate the current size of the node. The minSize method is needed to reserve
an adequate amount of memory for the entry vector of "MT::Node"[4]
(used in "MT::Node::insert()"[4]).

2.5.2 Class definition

*/
#ifndef MT_ENTRY_H
#define MT_ENTRY_H

#include "RelationAlgebra.h"
#include "MTreeAlgebra.h"
#include "DistData.h"
#include "RelationAlgebra.h"

namespace MT
{

class Entry
{

public:
  inline Entry( const TupleId tid, DistData* data )
  : m_tid( tid ), m_dist( 0 ), m_rad ( 0 ), m_chield( 0 ),
    m_data( data )
  {
    #ifdef __MT_DEBUG
    m_objectsOpen++;
    assert ( data );
    #endif

    updateSize();
  }
/*
Constructor.

*/

  inline Entry( const char* const buffer, int& offset )
  {
    #ifdef __MT_DEBUG
    m_objectsOpen++;
    #endif

    read( buffer, offset );
    updateSize();
  }
/*
Constructor.

*/

  inline Entry( const Entry& e )
  : m_tid( e.m_tid ), m_dist( e.m_dist ), m_rad( e.m_rad ),
    m_chield ( e.m_chield ), m_data( e.m_data->copy() ),
    m_size( e.m_size )
  {
    #ifdef __MT_DEBUG
    m_objectsOpen++;
    #endif
  }
/*
Copy constructor.

*/

  inline ~Entry()
  {
    m_data->deleteIfAllowed();

    #ifdef __MT_DEBUG
    m_objectsOpen--;
    #endif
  }
/*
Destructor.

*/

  inline const DistData* data() const
  {
    return m_data;
  }
/*
Returns the "DistData"[4] object of the entry.

*/

  inline TupleId tid() const
  {
    return m_tid;
  }
/*
Returns the tuple id if the entry.

*/

  inline double dist() const
  {
    return m_dist;
  }
/*
Returns distance of the entry to the parent node.

*/

  inline double rad() const
  {
    return m_rad;
  }
/*
Returns the covering radius of the entry.

*/

  inline SmiRecordId chield() const
  {
    return m_chield;
  }
/*
Returns record id of the chield node.

*/

  inline void setTid( TupleId tid )
  {
    m_tid = tid;
  }
/*
Sets a new tuple id.

*/

  inline void setDist( const double& dist )
  {
    m_dist = dist;
  }
/*
Sets distance to parent node.

*/
  inline void setRad( const double& rad )
  {
    m_rad = rad;
  }
/*
Sets a new covering radius.

*/

  void setChield( const SmiRecordId chield )
  {
    m_chield = chield;
  }
/*
Sets a new chield node.

*/

  Entry& operator=( const Entry& e );
/*
Assignment operator.

*/

  static size_t minSize();
/*
Returns the minimal size of an entry on disc.

*/

  inline size_t size() const
  {
    return m_size;
  }
/*
Returns the actual size of the entry on disc
(= minSize() + size of the data object).

*/

  void write( char* const buffer, int& offset ) const;
/*
Writes the entry to "buffer"[4] and increases "offset"[4].

*/

  static inline size_t objectsOpen()
  {
    return m_objectsOpen;
  }
/*
Returns the count of open objects (if "[_][_]DISTDATA[_]DEBUG"[4] is
not defined, this method will allways return 0).

*/

private:
  void read( const char* const buffer, int& offset );
/*
Reads the entry from "buffer"[4] and increases "offset"[4].

*/

  void updateSize();
/*
Computes the actual size of the node (stored in "m[_]size"[4]).

*/

  TupleId         m_tid;         // tuple-id of the entry
  double          m_dist;        // distance to parent node
  double          m_rad;         // covering radius
  SmiRecordId     m_chield;      // pointer to chield node
  DistData*       m_data;        // data obj. for dist. computations
  size_t          m_size;        // actual size of the entry
  static unsigned m_objectsOpen; // currently open objects

}; // class Entry

} // namespace MTree

#endif
