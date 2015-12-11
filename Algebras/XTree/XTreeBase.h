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

1 Headerfile "XTreeBase.h"[4]

January-May 2008, Mirko Dibbert

1.1 Overview

This file contains the declarations of the xtree nodes and node entries.

1.1 includes and defines

*/
#ifndef __XTREE_BASE_H__
#define __XTREE_BASE_H__

#include "RelationAlgebra.h"
#include "XTreeAlgebra.h"

namespace xtreeAlgebra {

using gtree::NodePtr;

/********************************************************************
1.1 Class "SplitHist"[4]

This class implements the split history for internal entries, which is stored in one bit per dimension.

********************************************************************/
class SplitHist
{
public:
/*
Contstructor (creates an empty split history)

*/
    inline SplitHist(unsigned dim)
        : m_len((dim/8)+1), m_hist(new char[m_len])
    { memset(m_hist, 0, m_len); }

/*
Default copy constructor.

*/
    inline SplitHist(const SplitHist &e)
         : m_len(e.m_len), m_hist(new char[m_len])
    { memcpy(m_hist, e.m_hist, m_len); }

/*
Constructor (reads the histoory from buffer and increases offset)

*/
    inline SplitHist(const char *const buffer, int &offset)
    { read(buffer, offset); }

/*
Destructor.

*/
    inline ~SplitHist()
    { delete[] m_hist; }

/*
Sets bit n in the Split history.

*/
    inline void set(unsigned n)
    {
        #ifdef __XTREE_DEBUG
        assert(n < 8*m_len);
        #endif

        unsigned i = n/8;
        m_hist[i] |= (1 << (n - 8*i));
    }

/*
Logical OR operator.

*/
    inline void operator |= (const SplitHist &rhs)
    {
        for (unsigned i=0; i<m_len; ++i)
            m_hist[i] |= rhs.m_hist[i];
    }

/*
Logical AND operator.

*/
    inline void operator &= (const SplitHist &rhs)
    {
        for (unsigned i=0; i<m_len; ++i)
            m_hist[i] &= rhs.m_hist[i];
    }

/*
Access operator (returns "true"[4] if bit n is set)

*/
    inline bool operator [] (unsigned n)
    {
        #ifdef __XTREE_DEBUG
        assert(n < 8*m_len);
        #endif

        int i = static_cast<int>(n/8);
        return (m_hist[i] & (1 << (n - 8*i)));
    }

/*
Writes the history to buffer and increases offset.

*/
    inline void write(char *const buffer, int &offset) const
    {
        memcpy(buffer+offset, &m_len, sizeof(unsigned));
        offset += sizeof(unsigned);

        memcpy(buffer+offset, m_hist, m_len);
        offset += m_len;
    }

/*
Returns the size of the history in bytes.

*/
    inline size_t size()
    { return sizeof(unsigned) + m_len*sizeof(char); }

private:
/*
Reads the history from buffer and increases offset.

*/
    inline void read(const char* const buffer, int &offset)
    {
        memcpy(&m_len, buffer+offset, sizeof(unsigned));
        offset += sizeof(unsigned);

        m_hist = new char[m_len];
        memcpy(m_hist, buffer+offset, m_len);
        offset += m_len;
    }

    unsigned m_len;
    char *m_hist;
};

/********************************************************************
1.1 Class ~LeafEntry~

********************************************************************/
class LeafEntry
        : public gtree::LeafEntry
{

public:
/*
Default constructor.

*/
    inline LeafEntry()
    {}

/*
Constructor (creates a new leaf entry with given values).

*/
    inline LeafEntry(TupleId tid, gta::HPoint *p)
        : m_tid(tid), m_isPoint(true), m_bbox(p->bbox()), m_point(p)
    {
        #ifdef __XTREE_DEBUG
        assert(p);
        #endif
    }

/*
Constructor (creates a new leaf entry with given values).

*/
    inline LeafEntry(TupleId tid, gta::HRect *bbox)
        : m_tid(tid), m_isPoint(false), m_bbox(bbox), m_point(0)
    {
        #ifdef __XTREE_DEBUG
        assert(m_bbox);
        #endif
    }

/*
Default copy constructor.

*/
    inline LeafEntry(const LeafEntry &e)
        : m_tid(e.m_tid), m_isPoint(e.m_isPoint),
          m_bbox(e.m_bbox), m_point(e.m_point)
    {}

/*
Destructor.

*/
    inline ~LeafEntry()
    {
        delete m_bbox;
        if (m_point)
            delete m_point;
    }

/*
Returns the tuple id of the entry.

*/
    inline TupleId tid() const
    { return m_tid; }

/*
Returns a reference to the "HRect"[4] object.

*/
    inline gta::HRect *bbox()
    { return m_bbox; }

/*
Writes the entry to buffer and increases offset (defined inline, since this method is called only once from Node::write).

*/
    inline void write(char *const buffer, int &offset) const
    {
        gtree::LeafEntry::write(buffer, offset);

        // write tuple-id
        memcpy(buffer+offset, &m_tid, sizeof(TupleId));
        offset += sizeof(TupleId);

        // read m_isPoint
        memcpy(buffer+offset, &m_isPoint, sizeof(bool));
        offset += sizeof(bool);

        if (m_isPoint)
            m_point->write(buffer, offset);
        else
            m_bbox->write(buffer, offset);
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

        // read m_isPoint
        memcpy(&m_isPoint, buffer+offset, sizeof(bool));
        offset += sizeof(bool);

        if (m_isPoint)
        {
            m_point = new gta::HPoint(buffer, offset);
            m_bbox = m_point->bbox();
        }
        else
        {
            m_point = 0;
            m_bbox = new gta::HRect(buffer, offset);
        }
    }

/*
Returns the size of the entry on disc.

*/
    inline size_t size()
    {
        if (m_isPoint)
        {
            return gtree::LeafEntry::size() +
                    sizeof(TupleId) + sizeof(bool) + m_point->size();
        }
        else
        {
            return gtree::LeafEntry::size() + sizeof(bool) +
                    sizeof(TupleId) + sizeof(bool) + m_bbox->size();
        }
    }

/*
Returns the square of the Euclidean distance between "p"[4] and the entry data (if the data is no point, the distance to the center of the bounding box is returned).

*/
    double dist(gta::HPoint *p)
    {
        if (m_isPoint)
            return gta::SpatialDistfuns::euclDist2(p, m_point);
        else
        {
            gta::HPoint c = m_bbox->center();
            return gta::SpatialDistfuns::euclDist2(p, &c);
        }
    }


private:
    TupleId   m_tid;      // tuple-id of the entry
    bool      m_isPoint;  // true, if data is point data
    gta::HRect     *m_bbox;    // bounding box of the entry
    gta::HPoint    *m_point;   // used for point data
}; // class LeafEntry

/********************************************************************
1.1 Class ~InternalEntry~

********************************************************************/
class InternalEntry
        : public gtree::InternalEntry
{
public:
/*
Default constructor.

*/
    inline InternalEntry()
    {}

/*
Constructor.

*/
    inline InternalEntry(gta::HRect *bbox, SmiRecordId _chield)
        : gtree::InternalEntry(_chield), m_bbox(bbox),
          m_history(new SplitHist(m_bbox->dim()))
    {}

/*
Constructor.

*/
    inline InternalEntry(
            gta::HRect *bbox, SmiRecordId _chield, SplitHist *_hist)
        : gtree::InternalEntry(_chield), m_bbox(bbox),
          m_history(new SplitHist(*_hist))
    {}

/*
Default copy constructor.

*/
    inline InternalEntry(const InternalEntry &e)
        : gtree::InternalEntry(e), m_bbox(new gta::HRect(*e.m_bbox)),
          m_history(e.history())
    {}

/*
Destructor.

*/
    inline ~InternalEntry()
    {
        delete m_bbox;
        delete m_history;
    }

/*
Returns a reference to the "HRect"[4] object.

*/
    inline gta::HRect *bbox()
    { return m_bbox; }

/*
Replaces the bounding box with new one (used during split).

*/
    inline void replaceHRect(gta::HRect* _bbox)
    {
        delete m_bbox;
        m_bbox = _bbox;

        #ifdef __XTREE_DEBUG
        assert(m_bbox);
        #endif
    }

/*
Returns the split history.

*/
    inline SplitHist *history() const
    { return m_history; }

/*
Writes the entry to buffer and increases offset (defined inline, since this method is called only once from Node::write).

*/
    inline void write(char *const buffer, int &offset) const
    {
        gtree::InternalEntry::write(buffer, offset);
        m_bbox->write(buffer, offset);
        m_history->write(buffer, offset);
    }

/*
Reads the entry from buffer and increases offset (defined inline, since this method is called only once from Node::read).

*/
    inline void read(const char *const buffer, int &offset)
    {
        gtree::InternalEntry::read(buffer, offset);
        m_bbox = new gta::HRect(buffer, offset);
        m_history = new SplitHist(buffer, offset);
    }

/*
Returns the size of the entry on disc.

*/
    inline size_t size()
    {
        return gtree::InternalEntry::size() +
            m_bbox->size() + m_history->size();
    }

private:
    gta::HRect     *m_bbox;    // bounding box of the entry
    SplitHist *m_history; // split history
}; // class DirEntry

/********************************************************************
1.1 Class  ~LeafNode~

********************************************************************/
class LeafNode
        : public gtree::LeafNode<LeafEntry>
{
public:
/*
Default constructor.

*/
    inline LeafNode(gtree::NodeConfigPtr config)
        : gtree::LeafNode<LeafEntry>(config, 0)
    {}

/*
Default copy constructor.

*/
    inline LeafNode(const LeafNode& node)
        : gtree::LeafNode<LeafEntry>(node)
    {}

/*
Virtual destructor.

*/
    inline virtual ~LeafNode()
    {}

/*
Returns a reference to a copy of the node.

*/
    virtual LeafNode *clone() const
    { return new LeafNode(*this); }

/*
Returns the union of all contained bounding boxes.

*/
    inline gta::HRect *bbox()
    {
        iterator iter = begin();
        gta::HRect *new_bbox = new gta::HRect(*(*iter)->bbox());
        while(++iter != end())
            new_bbox->unite((*iter)->bbox());

        return new_bbox;
    }

/*
Returns the union of the given entry vector.

*/
    static gta::HRect *bbox(std::vector<LeafEntry*> *entries)
    {
        iterator iter = entries->begin();
        gta::HRect *new_bbox = new gta::HRect(*(*iter)->bbox());
        while(++iter != entries->end())
            new_bbox->unite((*iter)->bbox());

        return new_bbox;
    }
}; // class "LeafNode"[4]

/********************************************************************
1.1 Class ~InternalNode~

********************************************************************/
class InternalNode
        : public gtree::InternalNode<InternalEntry>
{
public:
/*
Default constructor.

*/
    inline InternalNode(
            gtree::NodeConfigPtr config,
            gtree::NodeConfigPtr defaultConfig,
            gtree::NodeConfigPtr supernodeConfig)
    : gtree::InternalNode<InternalEntry>(config, 0),
      m_defaultConfig(defaultConfig),
      m_supernodeConfig(supernodeConfig)
    {}

/*
Default copy constructor.

*/
    inline InternalNode(const InternalNode &node)
    : gtree::InternalNode<InternalEntry>(node),
      m_defaultConfig(node.m_defaultConfig),
      m_supernodeConfig(node.m_supernodeConfig)
    {}

/*
MemSize (used to update used node cache size).

*/
    unsigned memSize(bool recompute) const
    {
        return gtree::InternalNode<InternalEntry>::
            memSize(recompute) + 2*sizeof(gtree::NodeConfigPtr);
    }

/*
Virtual destructor.

*/
    inline virtual ~InternalNode()
    {}

/*
Returns a reference to a copy of the node.

*/
    virtual InternalNode *clone() const
    { return new InternalNode(*this); }

/*
Sets supernode state.

*/
    inline void setSupernode()
    { m_config = m_supernodeConfig; }

/*
Resets supernode state.

*/
    inline void resetSupernode()
    { m_config = m_defaultConfig; }

/*
Returns "true"[4] if "this"[4] is a supernode.

*/
    inline bool isSupernode()
    { return this->m_config->type() == SUPERNODE; }

/*
Returns the union of all contained bounding boxes.

*/
    inline gta::HRect *bbox()
    {
        iterator iter = begin();
        gta::HRect *new_bbox = new gta::HRect(*(*iter)->bbox());
        while(++iter != end())
            new_bbox->unite((*iter)->bbox());

        return new_bbox;
    }

/*
Returns the union of the given entry vector.

*/
    static gta::HRect *bbox(std::vector <InternalEntry*> *entries)
    {
        iterator iter = entries->begin();
        gta::HRect *new_bbox = new gta::HRect(*(*iter)->bbox());
        while(++iter != entries->end())
            new_bbox->unite((*iter)->bbox());

        return new_bbox;
    }

private:
/*
This config objects are needed to change a node into a supernode and vice versa.

*/
    gtree::NodeConfigPtr m_defaultConfig;
    gtree::NodeConfigPtr m_supernodeConfig;
}; // class "InternalNode"[4]

/********************************************************************
1.1 Typedefs

********************************************************************/
typedef LeafNode* LeafNodePtr;
typedef InternalNode* InternalNodePtr;

} // namespace xtreeAlgebra
#endif // #ifndef __XTREE_BASE_H__
