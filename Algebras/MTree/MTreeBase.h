/*
\newpage

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

1 Headerfile "MTreeBase.h"[4]

January-May 2008, Mirko Dibbert

1.1 Overview

This headerfile implements entries and nodes of the mtree datastructure.

1.1 Includes and defines

*/
#ifndef __MTREE_BASE_H__
#define __MTREE_BASE_H__

#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "MTreeAlgebra.h"
#include "Algebras/GeneralTree/DistDataReg.h"

namespace mtreeAlgebra
{


/********************************************************************
1.1 Class ~LeafEntry~

********************************************************************/
class LeafEntry
        : public gtree::LeafEntry
{

friend class InternalEntry;

public:
/*
Default constructor.

*/
    inline LeafEntry()
    {}

/*
Constructor (creates a new leaf entry with given values).

*/
    inline LeafEntry(
            TupleId _tid, gta::DistData *_data, double _dist = 0)
        : m_tid(_tid), m_data(_data), m_dist(_dist)
    {
        #ifdef __MTREE_DEBUG
        assert(m_data);
        #endif
    }

/*
Default copy constructor.

*/
    inline LeafEntry(const LeafEntry &e)
        : m_tid(e.m_tid), m_data(new gta::DistData(*e.m_data)),
          m_dist(e.m_dist)
    {}

/*
Destructor.

*/
    inline ~LeafEntry()
    { delete m_data; }

/*
Returns the covering raidius of the entry (always 0 for leafes).

*/
    inline double rad() const
    { return 0; }

/*
Returns the tuple id of the entry.

*/
    inline TupleId tid() const
    { return m_tid; }

/*
Returns distance of the entry to the parent node.

*/
    inline double dist() const
    { return m_dist; }

/*
Sets a new distance to parent.

*/
    inline void setDist(double dist)
    { m_dist = dist; }

/*
Returns a reference to the "DistData"[4] object.

*/
    inline gta::DistData *data()
    {
        #ifdef __MTREE_DEBUG
        assert(m_data);
        #endif

        return m_data;
    }

/*
Writes the entry to buffer and increases offset (defined inline, since this method is called only once from Node::write).

*/
    inline void write(char *const buffer, int &offset) const
    {
        gtree::LeafEntry::write(buffer, offset);

        // write tuple-id
        memcpy(buffer+offset, &m_tid, sizeof(TupleId));
        offset += sizeof(TupleId);

        // write distance to parent node
        memcpy(buffer+offset, &m_dist, sizeof(double));
        offset += sizeof(double);

        // write m_data object
        m_data->write(buffer, offset);
    }

/*
Reads the entry from buffer and increases offset (defined inline, since this method is called only once from Node::read).

*/
    inline void read(const char *const buffer, int &offset)
    {
        gtree::LeafEntry::read(buffer, offset);

        // read tuple-id
        memcpy(&m_tid, buffer+offset, sizeof(TupleId));
        offset += sizeof(TupleId);

        // read distance to parent node
        memcpy(&m_dist, buffer+offset, sizeof(double));
        offset += sizeof(double);

        // read m_data object
        m_data = new gta::DistData(buffer, offset);
    }

/*
Returns the size of the entry on disc.

*/
    inline size_t size()
    {
        return gtree::LeafEntry::size() +
            sizeof(TupleId) + // m_tid
            sizeof(double) +  // m_dist
            sizeof(size_t) +  // size of DistData object
            m_data->size();   // m_data of DistData object
    }

private:
    TupleId     m_tid;   // tuple-id of the entry
    gta::DistData    *m_data; // m_data obj. for m_dist. computations
    double m_dist;  // distance to parent node
};

/********************************************************************
1.1 Class ~InternalEntry~

********************************************************************/
class InternalEntry
        : public gtree::InternalEntry
{
public:
/*
Default constructor (used to read the entry).

*/
  inline InternalEntry()
  {}

/*
Constructor (creates a new internal entry with given values).

*/
    inline InternalEntry(
            const InternalEntry &e, double _rad,
            SmiRecordId _chield)
        : gtree::InternalEntry(_chield),
          m_dist(e.m_dist), m_rad(_rad),
          m_data(new gta::DistData(*e.m_data))
    {}

/*
Constructor (creates a new internal entry from a leaf entry).

*/
    inline InternalEntry(
            const LeafEntry &e, double _rad,
            SmiRecordId _chield)
        : gtree::InternalEntry(_chield),
          m_dist(e.m_dist), m_rad(_rad),
          m_data(new gta::DistData(*e.m_data))
    {}

/*
Destructor.

*/
    inline ~InternalEntry()
    { delete m_data; }

/*
Returns distance of the entry to the parent node.

*/
    inline double dist() const
    { return m_dist; }

/*
Returns the covering radius of the entry.

*/
    inline double rad() const
    { return m_rad; }

/*
Returns a reference to the "DistData"[4] object.

*/
    inline gta::DistData *data()
    {
        #ifdef __MTREE_DEBUG
        assert(m_data);
        #endif

        return m_data;
    }

/*
Sets a new distance to parent.

*/
    inline void setDist(double dist)
    { m_dist = dist; }

/*
Sets a new covering radius.

*/
    inline void setRad(double rad)
    { m_rad = rad; }

/*
Writes the entry to buffer and increases offset (defined inline, since this method is called only once from Node::read).

*/
    inline void write(char *const buffer, int &offset) const
    {
        gtree::InternalEntry::write(buffer, offset);

        // write distance to parent node
        memcpy(buffer+offset, &m_dist, sizeof(double));
        offset += sizeof(double);

        // write covering radius
        memcpy(buffer+offset, &m_rad, sizeof(double));
        offset += sizeof(double);

        // write m_data object
        m_data->write(buffer, offset);
    }

/*
Reads the entry from buffer and increases offset (defined inline, since this method is called only once from Node::read).

*/
    void read(const char *const buffer, int &offset)
    {
        gtree::InternalEntry::read(buffer, offset);

        // read distance to parent node
        memcpy(&m_dist, buffer+offset, sizeof(double));
        offset += sizeof(double);

        // read covering radius
        memcpy(&m_rad, buffer+offset, sizeof(double));
        offset += sizeof(double);

        // read m_data object
        m_data = new gta::DistData(buffer, offset);
    }

/*
Returns the size of the entry on disc.

*/
    inline size_t size()
    {
        return gtree::InternalEntry::size() +
            2*sizeof(double) + // m_dist, m_rad
            sizeof(size_t) +   // size of DistData object
            m_data->size();    // m_data of DistData object
    }

private:
    double m_dist;  // distance to parent node
    double m_rad;   // covering radius
    gta::DistData    *m_data; // m_data obj. for m_dist. computations
};

/********************************************************************
1.1 Typedefs

********************************************************************/
typedef gtree::LeafNode<LeafEntry> LeafNode;
typedef gtree::InternalNode<InternalEntry> InternalNode;

typedef LeafNode* LeafNodePtr;
typedef InternalNode* InternalNodePtr;

} // namespace mtreeAlgebra
#endif // #ifndef __MTREE_BASE_H__
