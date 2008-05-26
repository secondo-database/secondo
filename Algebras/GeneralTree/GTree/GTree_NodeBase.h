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

1.1 Headerfile "GTree[_]NodeBase.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_NODE_BASE_H__
#define __GTREE_NODE_BASE_H__


#include "SmartPtr.h"
#include "GTree_EntryBase.h"
#include "GTree_NodeConfig.h"


namespace gtree
{



class FileNode; // forward declaration
typedef SmartPtr<FileNode> NodePtr;



/********************************************************************
Class ~NodeBase~

This is the base class for all nodes in the framework.

********************************************************************/
class NodeBase
    : public ObjCounter<generalTreeNodes>
{

friend class FileNode;

public:
/*
Default constructor.

*/
    inline NodeBase(NodeConfigPtr config, unsigned emptySize)
        : m_config(config),
          m_emptySize(emptySize +
                sizeof(NodeTypeId) +  // type-id
                sizeof(size_t) +      // count of extension records
                sizeof(SmiRecordId)), // first extension page
          m_curSize(m_emptySize),
          m_modified(true),
          m_cached(false),
          m_pagecount(1),
          m_level(0)
    {}

/*
Default copy constructor.

*/
    inline NodeBase(const NodeBase& node)
        : m_config(node.m_config),
          m_emptySize(node.m_emptySize),
          m_curSize(node.m_curSize),
          m_modified(node.m_modified),
          m_cached(node.m_cached),
          m_pagecount(node.m_pagecount),
          m_level(node.m_level)
    {}

/*
Virtual destructor.

*/
    inline virtual ~NodeBase()
    {}

/*
Returns the type-id of the node.

*/
    inline NodeTypeId typeId() const
    { return m_config->type(); }

/*
Returns state of the cached flag.

*/
    inline bool isCached() const
    { return m_cached; }

/*
Returns true, if the node could be stored in the node cache.

*/
    inline bool isCacheable() const
    { return m_config->cacheable(); }

/*
Sets the cached flag to "true"[4].

*/
    inline void setCached()
    { m_cached = true; }

/*
Sets the cached flag to "false"[4].

*/
    inline void resetCached()
    { m_cached = false; }

/*
Returns state of the modified flag.

*/
    inline bool isModified() const
    { return m_modified; }

/*
Sets the modified flag to "true"[4].

*/
    inline void setModified()
    { m_modified = true; }

/*
Sets the modified flag to "false"[4].

*/
    inline void resetModified()
    { m_modified = false; }

/*
Returns the count of pages, which this node would need in the file.

*/
    inline unsigned pagecount() const
    { return m_pagecount; }

/*
Returns the level of the node in the tree.

*/
    inline unsigned level() const
    { return m_level; }

/*
Increases the level of the node in the tree.

*/
    inline void incLevel()
    { ++m_level; }

/*
Decreases the level of the node in the tree.

*/
    inline void decLevel()
    { --m_level; }

/*
Sets the level of the node in the tree.

*/
    inline void setLevel(unsigned level)
    { m_level = level; }

/*
Returns the priority of the node.

*/
    inline unsigned priority() const
    { return m_config->priority(); }

/*
Should return "true"[4] for leaf nodes and "false"[4] for internal nodes.

*/
    virtual bool isLeaf() const = 0;

/*
Should read the node from buffer and increase offset.

*/
    virtual void read(const char *const buffer, int& offset) = 0;

/*
Should write the node to buffer and increase offset.

*/
    virtual void write(char *const buffer, int& offset) const = 0;

/*
Should return a copy of the node.

*/
    virtual NodeBase *clone() const = 0;

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
    virtual bool insert(EntryBase *e) = 0;

/*
Like ~insert~, but inserts a copy of "e"[4] by calling the copy constructor of the respective entry class.

*/
    virtual bool insertCopy(EntryBase *e) = 0;

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
    virtual void replace(unsigned i, EntryBase *newEntry) = 0;

/*
Should return the size of the entry im memory, which is needed to update the size of the node-cache, if used. Since this size is only needed to check, if the node cache has reached it's maximum size, it is sufficient to compute an approximately value to avoid complex computations.

If the "recompute"[4] value is false, the last value (stored in m[_]memSize) should be returned. Otherwhise the memory size should be recomputed first. A call with parameter "recompute = false"[4] is needed from the tree manager in the "recomputeSize"[4] method, which calls respective method of this class.

*/
    virtual unsigned memSize(bool recompute = true) const = 0;

/*
Returns the i-th entry in the node, needed it the respective node class is not known.

*/
    virtual EntryBase *baseEntry(unsigned i) const = 0;

protected:
    NodeConfigPtr m_config;     // ref. to the NodeConfig object
    unsigned      m_emptySize;  // size of an empty node
    unsigned      m_curSize;    // curent size of the node
    bool          m_modified;   // modified flag
    bool          m_cached;     // cached flag
    unsigned      m_pagecount;  // count of extension pages
    unsigned      m_level;      // level of the node
    mutable unsigned m_memSize; // current size of the node in memory
}; // class NodeBase

} // namespace gtree
#endif // #define __GTREE_NODE_BASE_H__
