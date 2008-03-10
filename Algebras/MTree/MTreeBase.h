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

January-February 2008, Mirko Dibbert

1.1 Overview

This headerfile implements entries and nodes of the mtree datastructure.

1.1 Includes and defines

*/
#ifndef __MTREE_BASE_H
#define __MTREE_BASE_H

#include "RelationAlgebra.h"
#include "MTreeAlgebra.h"

namespace mtreeAlgebra {
using gtaf::NodePtr;

/********************************************************************
1.1 Deklaration part

1.1.1 Class "LeafEntry"[4]

********************************************************************/
class LeafEntry : public gtaf::LeafEntry
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
    inline LeafEntry(TupleId _tid, DistData* _data,
                    DFUN_RESULT _dist = 0) :
            m_tid(_tid), m_data(_data), m_dist(_dist)
    {
        #ifdef MTREE_DEBUG
        assert(m_data);
        #endif
    }

/*
Default copy constructor.

*/
    inline LeafEntry(const LeafEntry& e) :
            m_tid(e.m_tid), m_data(new DistData(*e.m_data)),
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
    inline DFUN_RESULT rad() const
    { return 0; }

/*
Returns the tuple id of the entry.

*/
    inline TupleId tid() const
    { return m_tid; }

/*
Returns distance of the entry to the parent node.

*/
    inline DFUN_RESULT dist() const
    { return m_dist; }

/*
Sets a new distance to parent.

*/
    inline void setDist(DFUN_RESULT dist)
    { m_dist = dist; }

/*
Returns a reference to the "DistData"[4] object.

*/
    inline DistData* data()
    {
        #ifdef MTREE_DEBUG
        assert(m_data);
        #endif

        return m_data;
    }

/*
Writes the entry to buffer and increses offset (defined inline, since this method is called only once from Node::write).

*/
    inline void write(char* const buffer, int& offset) const
    {
        gtaf::LeafEntry::write(buffer, offset);

        // write tuple-id
        memcpy(buffer+offset, &m_tid, sizeof(TupleId));
        offset += sizeof(TupleId);

        // write distance to parent node
        memcpy(buffer+offset, &m_dist, sizeof(DFUN_RESULT));
        offset += sizeof(DFUN_RESULT);

        // write m_data object
        m_data->write(buffer, offset);
    }

/*
Reads the entry from buffer and increses offset (defined inline, since this method is called only once from Node::read).

*/
    inline void read(const char* const buffer, int& offset)
    {
        gtaf::LeafEntry::read(buffer, offset);

        // read tuple-id
        memcpy(&m_tid, buffer+offset, sizeof(TupleId));
        offset += sizeof(TupleId);

        // read distance to parent node
        memcpy(&m_dist, buffer+offset, sizeof(DFUN_RESULT));
        offset += sizeof(DFUN_RESULT);

        // read m_data object
        m_data = new DistData(buffer, offset);
    }

/*
Returns the size of the entry on disc.

*/
    inline size_t size()
    {
        return gtaf::LeafEntry::size() +
            sizeof(TupleId) + // m_tid
            sizeof(DFUN_RESULT) +  // m_dist
            sizeof(size_t) +  // size of DistData object
            m_data->size();   // m_data of DistData object
    }

private:
    TupleId     m_tid;  // tuple-id of the entry
    DistData*   m_data; // m_data obj. for m_dist. computations
    DFUN_RESULT m_dist; // distance to parent node
};

/********************************************************************
1.1.1 Class "InternalEntry"[4]

********************************************************************/
class InternalEntry : public gtaf::InternalEntry
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
    inline InternalEntry(const InternalEntry& e, DFUN_RESULT _rad,
                        SmiRecordId _chield) :
        gtaf::InternalEntry(_chield),
        m_dist(e.m_dist), m_rad(_rad),
        m_data(new DistData(*e.m_data))
    {}

/*
Constructor (creates a new internal entry from a leaf entry).

*/
    inline InternalEntry(const LeafEntry& e, DFUN_RESULT _rad,
                        SmiRecordId _chield) :
            gtaf::InternalEntry(_chield),
            m_dist(e.m_dist), m_rad(_rad),
            m_data(new DistData(*e.m_data))
    {}

/*
Destructor.

*/
    inline ~InternalEntry()
    { delete m_data; }

/*
Returns distance of the entry to the parent node.

*/
    inline DFUN_RESULT dist() const
    { return m_dist; }

/*
Returns the covering radius of the entry.

*/
    inline DFUN_RESULT rad() const
    { return m_rad; }

/*
Returns a reference to the "DistData"[4] object.

*/
    inline DistData* data()
    {
        #ifdef MTREE_DEBUG
        assert(m_data);
        #endif

        return m_data;
    }

/*
Sets a new distance to parent.

*/
    inline void setDist(DFUN_RESULT dist)
    { m_dist = dist; }

/*
Sets a new covering radius.

*/
    inline void setRad(DFUN_RESULT rad)
    { m_rad = rad; }

/*
Writes the entry to buffer and increses offset (defined inline, since this method is called only once from Node::read).

*/
    inline void write(char* const buffer, int& offset) const
    {
        gtaf::InternalEntry::write(buffer, offset);

        // write distance to parent node
        memcpy(buffer+offset, &m_dist, sizeof(DFUN_RESULT));
        offset += sizeof(DFUN_RESULT);

        // write covering radius
        memcpy(buffer+offset, &m_rad, sizeof(DFUN_RESULT));
        offset += sizeof(DFUN_RESULT);

        // write m_data object
        m_data->write(buffer, offset);
    }

/*
Reads the entry from buffer and increses offset (defined inline, since this method is called only once from Node::read).

*/
    void read(const char* const buffer, int& offset)
    {
        gtaf::InternalEntry::read(buffer, offset);

        // read distance to parent node
        memcpy(&m_dist, buffer+offset, sizeof(DFUN_RESULT));
        offset += sizeof(DFUN_RESULT);

        // read covering radius
        memcpy(&m_rad, buffer+offset, sizeof(DFUN_RESULT));
        offset += sizeof(DFUN_RESULT);

        // read m_data object
        m_data = new DistData(buffer, offset);
    }

/*
Returns the size of the entry on disc.

*/
    inline size_t size()
    {
        return gtaf::InternalEntry::size() +
            2*sizeof(DFUN_RESULT) + // m_dist, m_rad
            sizeof(size_t) +   // size of DistData object
            m_data->size();    // m_data of DistData object
    }

private:
    DFUN_RESULT m_dist; // distance to parent node
    DFUN_RESULT m_rad;  // covering radius
    DistData*   m_data; // m_data obj. for m_dist. computations
};

/********************************************************************
1.1.1 M-Tree basic typedefs

********************************************************************/
typedef gtaf::LeafNode<LeafEntry> LeafNode;
typedef gtaf::InternalNode<InternalEntry> InternalNode;

typedef SmartPtr<LeafNode> LeafNodePtr;
typedef SmartPtr<InternalNode> InternalNodePtr;

} // namespace mtee_alg
#endif // #ifndef __MTREE_BASE_H
