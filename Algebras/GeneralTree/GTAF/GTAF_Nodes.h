/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This file belongs to the GeneralTreeAlgebra framework (GTAF)           %
% Class descriptions and usage details could be found in gtaf.pdf        %
%                                                                        %
% (if this file does not exist, use "make docu" in the parent directory) %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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

1.1 Headerfile "GTAF[_]Nodes.h"[4]

January-February 2008, Mirko Dibbert

1.1.1 Includes and defines

*/
#ifndef __GTAF_NODES_H
#define __GTAF_NODES_H

#include "GTAF_Base.h"

namespace gtaf
{

/********************************************************************
1.1.1 Class GenericVectorNode

********************************************************************/
template<class TEntry>

class GenericVectorNode : public gtaf::NodeBase
{

public:
    typedef typename vector<TEntry*>::iterator iterator;
    typedef TEntry entryType;

/*
Constructor

Adding "sizeof(size[_]t)"[4] bytes to emptySize to store the count of entries.

*/
    inline GenericVectorNode(
            NodeConfigPtr config, unsigned emptySize);

/*
Default copy constructor.

*/
    inline GenericVectorNode(const GenericVectorNode& node);

/*
Virtual destructor.

*/
    virtual ~GenericVectorNode();

/*
Returns an iterator to the begin of the entry vector.

*/
    inline iterator begin() const;

/*
Returns an iterator to the end of the entry vector.

*/
    inline iterator end() const;

/*
Returns an iterator to the i-th entry.

*/
    inline iterator entryIt(unsigned i) const;

/*
Returns the entry vector for direct access. If entries has been added or removed from the vector, the "recomputeSize"[4] method has to be called.

*/
    inline vector<TEntry*>* entries() const;

/*
Reserves space for "n"[4] entries in the entry vector.

*/
    inline void reserve(size_t n);

/*
Sets the modified flag and recomputes "m[_]curSize" and "m[_]pagesize", which is nessesary, if the entry vector has been directly manipulated.

*/
    virtual void recomputeSize();

/*
Inserts a new entry into the node and returns "true"[4], if the node should been splitted.

*/
    virtual bool insert(EntryBase* newEntry);

/*
Like ~insert~, but inserts a copy of "e"[4], using the respective copy constructor.

*/
    virtual bool insertCopy(EntryBase* newEntry);

/*
Returns the count of all entries in the node.

*/
    virtual inline unsigned entryCount() const;

/*
Removes all entries from the node.

*/
    virtual inline void clear();

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

Warning: Do not replace an entry with itself, since this case would lead to a crash!

*/
    virtual void replace(unsigned i, EntryBase* newEntry);

/*
Returns a reference to the i-th entry.

*/
    inline TEntry* entry(unsigned i) const;

/*
Like above entry method, but returns an EntryBase pointer (e.g. used in the "Tree"[4] copy constructor.

*/
    virtual inline EntryBase* baseEntry(unsigned i) const;

/*
Returns the (approximately) size of the entry im memory.

*/
    virtual inline unsigned memSize() const;

protected:
/*
Reads the node from buffer and increses offset.

*/
    virtual void read(const char* const buffer, int& offset);

/*
Writes the node to buffer and increses offset.

*/
    virtual void write(char* const buffer, int& offset) const;

    vector<TEntry*>* m_entries; // entry vector
};

/********************************************************************
1.1.1 Class InternalNode

********************************************************************/
template<class TEntry>

class InternalNode : public GenericVectorNode<TEntry>
{

public:
/*
Default constructor.

*/
    inline InternalNode(NodeConfigPtr config, unsigned emptySize = 0)
            : GenericVectorNode<TEntry>(config, emptySize)
    {}

/*
Default copy constructor.

*/
    inline InternalNode(const InternalNode& node)
            : GenericVectorNode<TEntry>(node)
    {}

/*
Virtual destructor.

*/
    inline virtual ~InternalNode()
    {}

/*
Returns a reference to a copy of the node.

*/
    virtual InternalNode* clone() const
    { return new InternalNode(*this); }

/*
Returns the record id of the i-th chield node.

*/
    virtual SmiRecordId chield(unsigned i) const
    { return (GenericVectorNode<TEntry>::entry(i))->chield(); }

/*
Returns false, to indicate that this node is an internal node.

*/
    virtual bool isLeaf() const
    { return false; }
};

/********************************************************************
1.1.1 Class LeafNode

This class should be used as base class for all leaf node types.

********************************************************************/
template<class TEntry>

class LeafNode : public GenericVectorNode<TEntry>
{

public:
/*
Default constructor.

*/
    inline LeafNode(NodeConfigPtr config, unsigned emptySize = 0)
            : GenericVectorNode<TEntry>(config, emptySize)
    {}

/*
Default copy constructor.

*/
    inline LeafNode(const LeafNode& node)
            : GenericVectorNode<TEntry>(node)
    {}

/*
Virtual destructor.

*/
    inline virtual ~LeafNode()
    {}

/*
Returns a reference to a copy of the node.

*/
    virtual LeafNode* clone() const
    { return new LeafNode(*this); }

/*
Returns 0, since leaf does not have any chield nodes.

*/
    virtual SmiRecordId chield(unsigned i) const
    { return 0; }

/*
Returns true, to indicate that this node is a leaf node.

*/
    virtual bool isLeaf() const
    { return true; }
};

/********************************************************************
1.1.1 Implementation of class "GenericVectorNode"[4]

********************************************************************/
/*
Constructor:

*/
template<class TEntry>
GenericVectorNode<TEntry>::
        GenericVectorNode(NodeConfigPtr config, unsigned emptySize)
        : NodeBase(config, emptySize + sizeof(size_t)),
        m_entries(new vector<TEntry*>())
{}

/*
Constructor:

*/
template<class TEntry>
GenericVectorNode<TEntry>::
        GenericVectorNode(const GenericVectorNode& node)
        : NodeBase(node), m_entries(new vector<TEntry*>())
{
    // copy entry vector
    for (iterator it = node.begin(); it != node.end(); ++it)
        m_entries->push_back(new TEntry(**it));
}

/*
Destructor:

*/
template<class TEntry>
GenericVectorNode<TEntry>::~GenericVectorNode()
{
    iterator it;

    for (it = begin(); it != end(); ++it)
        delete *it;

    delete m_entries;
}

/*
Method ~begin~;

*/
template<class TEntry>
typename GenericVectorNode<TEntry>::iterator
GenericVectorNode<TEntry>::begin() const
{ return m_entries->begin(); }


/*
Method ~end~;

*/
template<class TEntry>
typename GenericVectorNode<TEntry>::iterator
GenericVectorNode<TEntry>::end() const
{ return m_entries->end(); }

/*
Method ~entryIt~;

*/
template<class TEntry>
typename GenericVectorNode<TEntry>::iterator
GenericVectorNode<TEntry>::entryIt(unsigned i) const
{
#ifdef GTAF_DEBUG
    assert(i < entryCount());
#endif

    return m_entries->begin() + i;
}

/*
Method ~entries~;

*/
template<class TEntry>
vector<TEntry*>*
GenericVectorNode<TEntry>::entries() const
{ return m_entries; }

/*
Method ~reserve~;

*/
template<class TEntry>
void
GenericVectorNode<TEntry>::reserve(size_t n)
{ entries->reserve(n); }

/*
Method ~recomputeSize~:

*/
template<class TEntry>
void
GenericVectorNode<TEntry>::recomputeSize()
{
    setModified();
    m_pagecount = 1;
    m_curSize = m_emptySize;

    // recompute size

    for (iterator it = begin(); it != end(); ++it)
        m_curSize += (*it)->size();

    // increment pagecount, if nessecary
    while (m_curSize > m_pagecount * PAGESIZE)
        ++m_pagecount;
}

/*
Method ~insert~:

*/
template<class TEntry>
bool
GenericVectorNode<TEntry>::insert(EntryBase* newEntry)
{
#ifdef GTAF_DEBUG
    TEntry* e = dynamic_cast<TEntry*>(newEntry);

    if (!e)
        Msg::wrongEntryType_Error();

#else
    TEntry* e = static_cast<TEntry*>(newEntry);

#endif

    // insert entry and update node size
    m_entries->push_back(e);

    m_curSize += e->size();

    setModified();

    // node must not be splitted, if the new entry fits into the
    // node without adding new pages and if it does not contain more
    // than m_config->maxEntries() entries
    if ((m_curSize <= m_pagecount * PAGESIZE) &&
            (entryCount() > m_config->maxEntries()))
        return false;

    // Increase pagecount, if neccessary
    while (m_curSize > (m_pagecount * PAGESIZE))
        ++m_pagecount;

    // node should never be splitted, if it does not contain at least
    // m_config->minEntries() entries or if the node is smaller than
    // m_config->minPages() pages
    if ((entryCount() < m_config->minEntries()) ||
            (m_curSize < m_config->minPages()))
        return false;

    // node should be splittet, if it contains more than
    // m_config->maxEntries() entries
    if (entryCount() > m_config->maxEntries())
        return true;

    // node should be splittet, if it needs more than
    // m_config->maxPages() pages
    if (m_pagecount > m_config->maxPages())
        return true;

    // minEntries <= entryCount <= maxEntries
    // minPages <= m_pagecount <= maxPages
    return false;
}

/*
Method ~insertCopy~:

*/
template<class TEntry>
bool
GenericVectorNode<TEntry>::insertCopy(EntryBase* newEntry)
{
#ifdef GTAF_DEBUG
    TEntry* e = dynamic_cast<TEntry*>(newEntry);

    if (!e)
        Msg::wrongEntryType_Error();

#else
    TEntry* e = static_cast<TEntry*>(newEntry);

#endif

    // insert entry and update node size
    m_entries->push_back(new TEntry(*e));

    m_curSize += e->size();

    setModified();

    // node must not be splitted, if the new entry fits into the
    // node without adding new pages and if it does not contain more
    // than m_config->maxEntries() entries
    if ((m_curSize <= m_pagecount * PAGESIZE) &&
            (entryCount() > m_config->maxEntries()))
        return false;

    // Increase pagecount, if neccessary
    while (m_curSize > (m_pagecount * PAGESIZE))
        ++m_pagecount;

    // node should never be splitted, if it does not contain at least
    // m_config->minEntries() entries or if the node is smaller than
    // m_config->minPages() pages
    if ((entryCount() < m_config->minEntries()) ||
            (m_pagecount < m_config->minPages()))
        return false;

    // node should be splittet, if it contains more than
    // m_config->maxEntries() entries
    if (entryCount() > m_config->maxEntries())
        return true;

    // node should be splittet, if it needs more than
    // m_config->maxPages() pages
    if (m_pagecount > m_config->maxPages())
        return true;

    // minEntries <= entryCount <= maxEntries
    // minPages <= m_pagecount <= maxPages
    return false;
}

/*
Method ~entryCount~:

*/
template<class TEntry>
unsigned
GenericVectorNode<TEntry>::entryCount() const
{ return m_entries->size(); }


/*
Method ~clear~:

*/
template<class TEntry>
void
GenericVectorNode<TEntry>::clear()
{
    m_curSize = m_emptySize;
    m_entries->clear();
    setModified();
}

/*
Method ~remove~ (position):

*/
template<class TEntry>
void
GenericVectorNode<TEntry>::remove(unsigned i)
{
#ifdef GTAF_DEBUG
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
void
GenericVectorNode<TEntry>::remove(iterator it)
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
void
GenericVectorNode<TEntry>::fastRemove(unsigned i)
{
#ifdef GTAF_DEBUG
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
void
GenericVectorNode<TEntry>::fastRemove(iterator it)
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
template<class TEntry> void
GenericVectorNode<TEntry>::replace(unsigned i, EntryBase* newEntry)
{
#ifdef GTAF_DEBUG
    assert(i < entryCount());
#endif

#ifdef GTAF_DEBUG
    TEntry* e = dynamic_cast<TEntry*>(newEntry);

    if (!e)
        Msg::wrongEntryType_Error();

#else
    TEntry* e = static_cast<TEntry*>(newEntry);

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
TEntry*
GenericVectorNode<TEntry>::entry(unsigned i) const
{
#ifdef GTAF_DEBUG
    assert(i < entryCount());
#endif
    return (*m_entries)[i];
}

/*
Method ~baseEntry~:

*/
template<class TEntry>
EntryBase*
GenericVectorNode<TEntry>::baseEntry(unsigned i) const
{
#ifdef GTAF_DEBUG
    assert(i < entryCount());
#endif
    return (*m_entries)[i];
}

/*
Method ~memSize~:

*/
template<class TEntry>
unsigned
GenericVectorNode<TEntry>::memSize() const
{
    return
        m_curSize +
        sizeof(GenericVectorNode<TEntry>) + // size of all members
        entryCount() * sizeof(TEntry*);    // size of entry pointers
}

/*
Method ~read~:

*/
template<class TEntry>
void
GenericVectorNode<TEntry>::read(const char* const buffer,
                                int& offset)
{
    m_curSize = m_emptySize;

    // read count of stored entries
    size_t count;
    memcpy(&count, buffer + offset, sizeof(size_t));
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
void
GenericVectorNode<TEntry>::write(char* const buffer,
                                 int& offset) const
{
    // write count of stored entries
    size_t count = m_entries->size();
    memcpy(buffer + offset, &count, sizeof(size_t));
    offset += sizeof(size_t);

    // write the entry array

    for (iterator it = begin(); it != end(); ++it)
        (*it)->write(buffer, offset);
}
}; // namepspace gtree
#endif // #ifndef __GTAF_NODES_H
