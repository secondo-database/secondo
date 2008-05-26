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

1.1 Headerfile "GTree[_]FileNode.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_FILE_NODE_H__
#define __GTREE_FILE_NODE_H__

#include "SecondoSMI.h"
#include "GTree_NodeManager.h"

namespace gtree
{



class FileNode; // forward declaration
typedef SmartPtr<FileNode> NodePtr;



/*
Class ~FileNode~

This class adds some persitence abilities to the "NodeBase"[4] class, all methods of "NodeBase"[4] are wrapped within respective methods of this class, thus "NodePtr"[4] (="SmartPtr<FileNode>"[4]) could be used similar to "NodeBase"[4], but allows further filemanagement methods for the node,\\e.g. "getNodeId"[4], "get"[4], "put"[4] or "drop"[4].

*/
class FileNode
{

public:
/*
Constructor (creates a new node).

*/
    inline FileNode(NodeManager *mngr, NodeTypeId type)
        : m_nodeMngr(mngr), m_nodeId(0),
          m_extensionId(0), m_extPageCnt(0)
    { createNode(type); }

/*
Constructor (reads the node from page "nodeId"[4] in file).

*/
    inline FileNode(NodeManager *mngr, SmiRecordId nodeId)
        : m_nodeMngr(mngr)
    { get(nodeId); }


/*
Default copy constructor.

*/
    inline FileNode(const FileNode &node)
        : m_node(node.m_node->clone()), m_nodeMngr(node.m_nodeMngr),
          m_nodeId(node.m_nodeId), m_extensionId(node.m_extensionId),
          m_extPageCnt(node.m_extPageCnt)
    {}

/*
Destructor (writes the node to file, if neccesary).

*/
    inline ~FileNode()
    { put(); }


/*
Returns a copy of the "FileNode"[4] Object.

*/
    inline NodePtr clone() const
    { return NodePtr(new FileNode(*this)); }


/*
Returns the record id of the node. If no record exist, a new record will be appended to the file.

*/
    SmiRecordId getNodeId();

/*
Creates a new node. If there already exists a node reference, that node would be automatically written to file.

*/
    inline void createNode(NodeTypeId type)
    { m_node = m_nodeMngr->createNode(type); }

/*
Writes the node to file.

*/
    void put();

/*
Reads the node from file. If there already exists a node reference, that node would be automatically written to file.

*/
    void get(SmiRecordId nodeId);

/*
Removes the refered node from file.

*/
    void drop();

/*
Returns "m[_]node"[4].

*/
    inline SmartPtr<NodeBase> ptr()
    { return m_node; }

/*
Casts the node pointer to the node class, which is specified in the template parameter. In debug mode, an error message is shown and the dbms will terminate, if the cast was not allowed.

*/
    template<class TNode>
    inline TNode* cast()
    {
        #ifdef __GTREE_DEBUG
        TNode* result = dynamic_cast<TNode*>(m_node.operator->());
        if (!result)
            Msg::invalidNodeCast_Error();
        return result;
        #else
        return static_cast<TNode*>(m_node.operator->());
        #endif
    }

private:
    SmartPtr<NodeBase> m_node;        // Pointer to the referred node
    NodeManager        *m_nodeMngr;   // ref. to node manager
    SmiRecordId        m_nodeId;      // header record of the node
    SmiRecordId        m_extensionId; // id of first extension page
    unsigned           m_extPageCnt;  // count of extension pages

public:

/*
The following methods wraps the respective methods of the "NodeBase"[4] class:

*/
    inline unsigned memSize(bool recompute = true) const
    { return m_node->memSize(recompute); }

    inline bool isCached() const
    { return m_node->isCached(); }

    inline void setCached()
    { m_node->setCached(); }

    inline void resetCached()
    { m_node->resetCached(); }

    inline bool isModified() const
    { return m_node->isModified(); }

    inline void setModified()
    { m_node->setModified(); }

    inline void resetModified()
    { m_node->resetModified(); }

    inline bool isCacheable() const
    { return m_node->isCacheable(); }

    inline bool isLeaf() const
    { return m_node->isLeaf(); }

    inline NodeTypeId typeId() const
    { return m_node->typeId(); }

    inline SmiRecordId chield(unsigned i) const
    { return m_node->chield(i); }

    inline void recomputeSize()
    { m_node->recomputeSize(); }

    inline bool insert(EntryBase *e)
    { return m_node->insert(e); }

    inline bool insertCopy(EntryBase *e)
    { return m_node->insertCopy(e); }

    inline unsigned entryCount() const
    { return m_node->entryCount(); }

    inline void clear()
    { m_node->clear(); }

    inline void remove(unsigned i)
    { m_node->remove(i); }

    inline void replace(unsigned i, EntryBase *newEntry)
    { m_node->replace(i, newEntry); }

    inline unsigned pagecount() const
    { return m_node->pagecount(); }

    inline unsigned level() const
    { return m_node->level(); }

    inline void incLevel()
    { return m_node->incLevel(); }

    inline void decLevel()
    { return m_node->decLevel(); }

    inline void setLevel(unsigned level)
    { return m_node->setLevel(level); }

    inline unsigned priority() const
    { return m_node->priority(); }

    inline EntryBase *baseEntry(unsigned i) const
    { return m_node->baseEntry(i); }
}; // class FileNode

} // namespace gtree
#endif // #define __GTREE_FILE_NODE_H__
