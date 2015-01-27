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

1.1 Headerfile "GTree[_]GenericNodeBase.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_GENERIC_NODE_BASE_H__
#define __GTREE_GENERIC_NODE_BASE_H__

#include <vector>
#include "GTree_NodeBase.h"

namespace gtree
{

/********************************************************************
Class ~GenericNodeBase~

Base class for "InternalNode"[4] and "LeafNode"[4] - implements the "NodeBase"[4] class, using a vector.

The following methods hat been added additionally to the "NodeBase"[4] methods:

  * "begin"[4], "end"[4], "entryIt"[4], "reserve"[4] (access to some methods of the entry vector)

  * "remove"[4] for iterators

  * "fastRemove"[4] methods (faster than remove, but ignores entry order)

********************************************************************/
template<class TEntry>
class GenericNodeBase
    : public gtree::NodeBase
{

public:
    typedef typename vector<TEntry*>::iterator iterator;
    typedef TEntry entryType;

/*
Constructor

*/
    inline GenericNodeBase(
            NodeConfigPtr config, unsigned emptySize)
        : NodeBase(config, emptySize + sizeof(size_t)),
          m_entries(new vector<TEntry*>())
    {}

/*
Default copy constructor.

*/
    inline GenericNodeBase(const GenericNodeBase& node)
        : NodeBase(node), m_entries(new vector<TEntry*>())
    {
        // copy entry vector
        for (iterator it = node.begin(); it != node.end(); ++it)
            m_entries->push_back(new TEntry(**it));
    }

/*
Virtual destructor.

*/
    virtual ~GenericNodeBase()
    {
        for (iterator it = begin(); it != end(); ++it)
            delete *it;

        delete m_entries;
    }

/*
Returns an iterator to the begin of the entry vector.

*/
    inline iterator begin() const
    { return m_entries->begin(); }

/*
Returns an iterator to the end of the entry vector.

*/
    inline iterator end() const
    { return m_entries->end(); }

/*
Returns an iterator to the i-th entry.

*/
    inline iterator entryIt(unsigned i) const
    {
#ifdef __GTREE_DEBUG
        assert(i < entryCount());
#endif

        return m_entries->begin() + i;
    }

/*
Returns the entry vector for direct access. If entries has been added or removed from the vector, the "recomputeSize"[4] method has to be called.

*/
    inline vector<TEntry*> *entries() const
    { return m_entries; }

/*
Reserves space for "n"[4] entries in the entry vector.

*/
    inline void reserve(size_t n)
    { m_entries->reserve(n); }

/*
Sets the modified flag and recomputes "m[_]curSize" and "m[_]pagesize", which is nessesary, if the entry vector has been directly manipulated.

*/
    virtual void recomputeSize();

/*
Inserts a new entry into the node and returns "true"[4], if the node should been splitted.

*/
    virtual bool insert(EntryBase *newEntry);

/*
Like ~insert~, but inserts a copy of "e"[4], using the respective copy constructor.

*/
    virtual bool insertCopy(EntryBase *newEntry);

/*
Returns the count of all entries in the node.

*/
    virtual inline unsigned entryCount() const
    { return m_entries->size(); }

/*
Removes all entries from the node.

*/
    virtual inline void clear()
    {
        m_curSize = m_emptySize;
        m_entries->clear();
        setModified();
    }

/*
Removes the i-th entry from the node. Since the entries are hold in a vector, all entries after the i-th position have to be moved to the new position, wich could be a performance problem, in particular for nodes with much entries. However this shouldn't be a problem in most cases, since the vector contains only pointers to the entry. If the order of the entries doesen't matter, "fastRemove"[4] could be used instead.

*/
    virtual void remove(unsigned i);

/*
Like above remove method, but expects an iterator to the entry.

*/
    virtual void remove(iterator it);

/*
Removes the i-th entry from the node and moves the last entry to that position. This is faster than the default remove method, but destroyes the entry order.

*/
    virtual void fastRemove(unsigned i);

/*
Like above fastRemove method, but expects an iterator to the entry.

*/
    virtual void fastRemove(iterator it);

/*
Replaced the i-th entry with "newEntry"[4].

Warning: Do not replace an entry with itself, since this would lead to a crash!

*/
    virtual void replace(unsigned i, EntryBase *newEntry);

/*
Returns a reference to the i-th entry.

*/
    inline TEntry *entry(unsigned i) const;

/*
Like above entry method, but returns an "EntryBase"[4] pointer (e.g. used in the "Tree"[4] copy constructor).

*/
    virtual inline EntryBase *baseEntry(unsigned i) const;

/*
Returns the (approximately) size of the entry im memory.

*/
    virtual inline unsigned memSize(bool recompute = true) const;

protected:
/*
Reads the node from buffer and increses offset.

*/
    virtual void read(const char *const buffer, int &offset);

/*
Writes the node to buffer and increses offset.

*/
    virtual void write(char *const buffer, int &offset) const;

    vector<TEntry*> *m_entries; // entry vector
}; // GenericNodeBase



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/*
Method ~recomputeSize~:

*/
template<class TEntry>
void GenericNodeBase<TEntry>::recomputeSize()
{
    setModified();

    // recompute size
    m_curSize = m_emptySize;
    for (iterator it = begin(); it != end(); ++it)
        m_curSize += (*it)->size();

    // recompute pagecount
    m_pagecount = 1;
    while (m_curSize > m_pagecount * PAGESIZE)
        ++m_pagecount;
}

/*
Method ~insert~:

*/
template<class TEntry>
bool GenericNodeBase<TEntry>::insert(EntryBase *newEntry)
{
#ifdef __GTREE_DEBUG
    TEntry *e = dynamic_cast<TEntry*>(newEntry);
    if (!e) Msg::wrongEntryType_Error();

#else
    TEntry *e = static_cast<TEntry*>(newEntry);
#endif

    // insert entry and update node size
    m_entries->push_back(e);
    m_curSize += e->size();
    setModified();

    // increase pagecount, if neccessary
    while (m_curSize > (m_pagecount * PAGESIZE))
        ++m_pagecount;

    if ((
            (entryCount() <= m_config->maxEntries()) &&
            (m_pagecount <= m_config->maxPages())
        ) ||
        (entryCount() <= m_config->minEntries()))
    {   // node should not be splitted
        return false;
    }

    // node should be splitted
    return true;
}

/*
Method ~insertCopy~:

*/
template<class TEntry>
bool GenericNodeBase<TEntry>::insertCopy(EntryBase *newEntry)
{
#ifdef __GTREE_DEBUG
    TEntry *e = dynamic_cast<TEntry*>(newEntry);
    if (!e) Msg::wrongEntryType_Error();

#else
    TEntry *e = static_cast<TEntry*>(newEntry);
#endif

    // insert entry and update node size
    m_entries->push_back(new TEntry(*e));
    m_curSize += e->size();
    setModified();

    // increase pagecount, if neccessary
    while (m_curSize > (m_pagecount * PAGESIZE))
        ++m_pagecount;

    if ((
            (entryCount() <= m_config->maxEntries()) &&
            (m_pagecount <= m_config->maxPages())
        ) ||
        (entryCount() <= m_config->minEntries()))
    {   // node should not be splitted
        return false;
    }

    // node should be splitted
    return true;
}

/*
Method ~remove~ (position):

*/
template<class TEntry>
void GenericNodeBase<TEntry>::remove(unsigned i)
{
#ifdef __GTREE_DEBUG
    assert(i < entryCount());
#endif

    m_curSize -= entry(i)->size();
    delete entry(i);
    entries()->erase(entryIt(i));

    setModified();
}

/*
Method ~remove~ (iterator):

*/
template<class TEntry>
void GenericNodeBase<TEntry>::remove(iterator it)
{
    m_curSize -= (*it)->size();
    delete *it;
    entries()->erase(it);

    setModified();
}

/*
Method ~fastRemove~ (position):

*/
template<class TEntry>
void GenericNodeBase<TEntry>::fastRemove(unsigned i)
{
#ifdef __GTREE_DEBUG
    assert(i < entryCount());
#endif

    m_curSize -= entry(i)->size();
    delete entry(i);
    *entryIt(i) = m_entries->back();
    m_entries->pop_back();

    setModified();
}

/*
Method ~fastRemove~ (iterator):

*/
template<class TEntry>
void GenericNodeBase<TEntry>::fastRemove(iterator it)
{
    m_curSize -= (*it)->size();
    delete *it;
    *it = m_entries->back();
    m_entries->pop_back();

    setModified();
}

/*
Method ~replace~:

*/
template<class TEntry>
void GenericNodeBase<TEntry>::replace(
        unsigned i, EntryBase *newEntry)
{
#ifdef __GTREE_DEBUG
    assert(i < entryCount());
    TEntry *e = dynamic_cast<TEntry*>(newEntry);
    if (!e) Msg::wrongEntryType_Error();

#else
    TEntry *e = static_cast<TEntry*>(newEntry);
#endif

    m_curSize -= entry(i)->size();
    m_curSize += e->size();
    delete entry(i);
    *entryIt(i) = e;
    setModified();
}

/*
Method ~entry~:

*/
template<class TEntry>
TEntry *GenericNodeBase<TEntry>::entry(unsigned i) const
{
#ifdef __GTREE_DEBUG
    assert(i < entryCount());
#endif
    return (*m_entries)[i];
}

/*
Method ~baseEntry~:

*/
template<class TEntry>
EntryBase *GenericNodeBase<TEntry>::baseEntry(unsigned i) const
{
#ifdef __GTREE_DEBUG
    assert(i < entryCount());
#endif
    return (*m_entries)[i];
}

/*
Method ~memSize~:

*/
template<class TEntry>
unsigned GenericNodeBase<TEntry>::memSize(bool recompute) const
{
    if (recompute)
    {
        m_memSize = m_curSize +
            sizeof(GenericNodeBase<TEntry>) + // size of all members
            entryCount() * sizeof(TEntry*); // size of entry pointers
    }
    return m_memSize;
}

/*
Method ~read~:

*/
template<class TEntry>
void GenericNodeBase<TEntry>::read(
        const char *const buffer, int &offset)
{
    m_curSize = m_emptySize;

    // read count of stored entries
    size_t count;
    memcpy(&count, buffer+offset, sizeof(size_t));
    offset += sizeof(size_t);

    // read entry vector
    m_entries->reserve(count);
    int old_offset = offset;

    for (size_t i = 0; i < count; ++i)
    {
        m_entries->push_back(new TEntry());
        m_entries->back()->read(buffer, offset);
    }

    // add  size of the entry vector to m_curSize
    m_curSize += (offset - old_offset);
}

/*
Method ~write~:

*/
template<class TEntry>
void GenericNodeBase<TEntry>::write(
        char *const buffer, int &offset) const
{
    // write count of stored entries
    size_t count = m_entries->size();
    memcpy(buffer + offset, &count, sizeof(size_t));
    offset += sizeof(size_t);

    // write the entry array
    for (iterator it = begin(); it != end(); ++it)
    {
        (*it)->write(buffer, offset);
    }
}



} // namespace gtree
#endif // #define __GTREE_GENERIC_NODE_BASE_H__
