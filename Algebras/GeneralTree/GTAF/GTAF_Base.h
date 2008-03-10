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

1.1 Headerfile "GTAF[_]Base.h"[4]

January-February 2008, Mirko Dibbert

1.1.1 Includes and defines

*/
#ifndef __GTAF_BASE_H
#define __GTAF_BASE_H

#include "SecondoSMI.h"
#include "ObjCounter.h"
#include "GTAF_NodeConfig.h"

namespace gtaf
{
/********************************************************************
1.1.1 Class EntryBase

Base class for gtaf-entries.

********************************************************************/
class EntryBase : public ObjCounter<generalTreeEntries>
{

public:
    inline EntryBase()
    {}

    inline EntryBase(const EntryBase& source)
    {}

    // virtual destructor needed since dynamic_cast is used instead
    // of static_cast, if GTAF_DEBUG is defined
#ifdef GTAF_DEBUG
    inline virtual ~EntryBase() {}
#else
    inline ~EntryBase() {}
#endif
};

/********************************************************************
1.1.1 Class NodeBase

Base class for gtaf-nodes.

********************************************************************/
class NodeBase : public ObjCounter<generalTreeNodes>
{
friend class FileNode;

public:
/*
Default constructor.

*/
    inline NodeBase(NodeConfigPtr config, unsigned emptySize);

/*
Default copy constructor.

*/
    inline NodeBase(const NodeBase& node);

/*
Virtual destructor.

*/
    inline virtual ~NodeBase();

/*
Returns the type-id of the node.

*/
    inline NodeTypeId typeId() const;

/*
Returns state of the cached flag.

*/
    inline bool isCached() const;

/*
Returns true, if the node could be stored in the node cache.

*/
    inline bool isCacheable() const;

/*
Sets the cached flag to "true"[4].

*/
    inline void setCached();

/*
Sets the cached flag to "false"[4].

*/
    inline void resetCached();

/*
Returns state of the modified flag.

*/
    inline bool isModified() const;

/*
Sets the modified flag to "true"[4].

*/
    inline void setModified();

/*
Sets the modified flag to "false"[4].

*/
    inline void resetModified();

/*
Returns the count of pages, which this node would need in the file.

*/
    inline unsigned pagecount() const;

/*
Returns the level of the node in the tree.

*/
    inline unsigned level() const;

/*
Increases the level of the node in the tree.

*/
    inline void incLevel();

/*
Decreases the level of the node in the tree.

*/
    inline void decLevel();

/*
Sets the level of the node in the tree.

*/
    inline void setLevel(unsigned level);

/*
Returns the priority of the node.

*/
    inline unsigned priority() const;

/*
Should return "true"[4] for leaf nodes and "false"[4] for internal nodes.

*/
    virtual bool isLeaf() const = 0;

/*
Should read the node from buffer and increase offset.

*/
    virtual void read(const char* const buffer, int& offset) = 0;

/*
Should write the node to buffer and increase offset.

*/
    virtual void write(char* const buffer, int& offset) const = 0;

/*
Should return a copy of the node.

*/
    virtual NodeBase* clone() const = 0;

/*
Should return the "SmiRecordId" of the i-th subtree of the node, if the node is an internal node and 0 for leaf nodes.

*/
    virtual SmiRecordId chield(unsigned i) const = 0;

/*
Should recompute the size of the node and set the modified flag (needed, if the entry representation has been manimpulated directly)

*/
    virtual void recomputeSize() = 0;

/*
Inserts a new entry into the node. Should return "true"[4], if the node needs to be splitted after insert (the entry should be inserted in any case).

*/
    virtual bool insert(EntryBase* e) = 0;

/*
Like ~insert~, but inserts a copy of "e"[4] by calling the copy constructor of the respective entry class.

*/
    virtual bool insertCopy(EntryBase* e) = 0;

/*
Should return the count of all contained entries.

*/
    virtual unsigned entryCount() const = 0;

/*
Should remove all entries.

*/
    virtual void clear() = 0;

/*
Should remove the i-th entry.

*/
    virtual void remove(unsigned i) = 0;

/*
Should replace the i-th emtry with "newEntry"[4].

*/
    virtual void replace(unsigned i, EntryBase* newEntry) = 0;

/*
Should return the size of the entry im memory, which is needed to update the size of the node-cache, if used. Since this size is only needed to check, if the node cache has reached it's maximum size, it is sufficient to compute an approximately value to avoid complex computations.

If the "recompute"[4] value is false, the last value (stored in m[_]memSize) should be returned. Otherwhise the memory size should be recomputed first. A call with parameter "recompute = false"[4] is needed from the tree manager in the "recomputeSize"[4] method, which calls respective method of this class. The method needs the last size, since the size could have been changed before calling the method.

*/
    virtual unsigned memSize(bool recompute = true) const = 0;

/*
Returns the i-th entry in the node, needed it the respective node class is not known.

*/
    virtual EntryBase* baseEntry(unsigned i) const = 0;

protected:
    NodeConfigPtr m_config;    // ref. to the NodeConfig object
    unsigned      m_emptySize; // size of an empty node
    unsigned      m_curSize;   // curent size of the node
    bool          m_modified;  // modified flag
    bool          m_cached;    // cached flag
    unsigned      m_pagecount; // count of extension pages
    unsigned      m_level;     // level of the node
    mutable unsigned m_memSize;// current size of the node in memory
};

/********************************************************************
1.1.1 Implementation of class "NodeBase"[4] (inline methods)

********************************************************************/
/*
Default Constructor:

*/
NodeBase::NodeBase(NodeConfigPtr config, unsigned emptySize)
        : m_config(config),
        m_emptySize(emptySize +
                    sizeof(NodeTypeId) + // type-id
                    sizeof(size_t)), // count of extension records
        m_curSize(m_emptySize),
        m_modified(true),
        m_cached(false),
        m_pagecount(1),
        m_level(0)
{}

/*
Default copy constructor:

*/
NodeBase::NodeBase(const NodeBase& node)
        : m_config(node.m_config),
        m_emptySize(node.m_emptySize),
        m_curSize(node.m_curSize),
        m_modified(node.m_modified),
        m_cached(node.m_cached),
        m_pagecount(node.m_pagecount),
        m_level(node.m_level)
{}

/*
Destructor:

*/
NodeBase::~NodeBase()
{}

/*
Method ~typeId~:

*/
NodeTypeId
NodeBase::typeId() const
{ return m_config->type(); }

/*
Method ~isCached~:

*/
bool
NodeBase::isCached() const
{ return m_cached; }

/*
Method ~isCachable~:

*/
bool
NodeBase::isCacheable() const
{ return m_config->cacheable(); }

/*
Method ~setCached~:

*/
void
NodeBase::setCached()
{ m_cached = true; }

/*
Method ~resetCached~:

*/
void
NodeBase::resetCached()
{ m_cached = false; }

/*
Method ~isModified~:

*/
bool
NodeBase::isModified() const
{ return m_modified; }

/*
Method ~setModified~:

*/
void
NodeBase::setModified()
{ m_modified = true; }

/*
Method ~resetModified~:

*/
void
NodeBase::resetModified()
{ m_modified = false; }

/*
Method ~pagecount~:

*/
unsigned
NodeBase::pagecount() const
{ return m_pagecount; }

/*
Method ~level~:

*/
unsigned
NodeBase::level() const
{ return m_level; }

/*
Method ~incLevel~:

*/
void
NodeBase::incLevel()
{ ++m_level; }

/*
Method ~decLevel~:

*/
void
NodeBase::decLevel()
{ --m_level; }

/*
Method ~setLevel~:

*/
void
NodeBase::setLevel(unsigned level)
{ m_level = level; }

/*
Method ~priority~:

*/
unsigned
NodeBase::priority() const
{ return m_config->priority(); }

}; // namespace gtaf
#endif // #ifndef __GTAF_BASE_H
