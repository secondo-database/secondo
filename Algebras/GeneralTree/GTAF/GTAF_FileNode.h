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

SECONDO is free software; you can redistribute iter and/or modify
iter under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that iter will be useful,
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

1.1 Headerfile "GTAF[_]FileNode.h"[4]

January-February 2008, Mirko Dibbert

1.1.1 Includes and defines

*/
#ifndef __GTAF_FILE_NODE_H
#define __GTAF_FILE_NODE_H

#include "GTAF_Base.h"
#include "GTAF_NodeSupp.h"

namespace gtaf
{

/********************************************************************
1.1.1 Class "FileNode"[4]

********************************************************************/
class FileNode
{

public:
/*
Constructor (creates a new node).

*/
    inline FileNode(NodeSupp* supp, NodeTypeId type);


/*
Constructor (reads the node from page "nodeId"[4] in file).

*/
    inline FileNode(NodeSupp* supp, SmiRecordId nodeId);

/*
Default copy constructor.

*/
    inline FileNode(const FileNode& node);

/*
Destructor (writes the node to file, if neccesary).

*/
    inline ~FileNode();


/*
Returns a copy of the "FileNode"[4] Object.

*/
    inline NodePtr clone() const;

/*
Returns the record id of the node. If no record exist, a new record will be appended to the file.

*/
    SmiRecordId getNodeId();

/*
Creates a new node. If there already exists a node reference, that node would be automatically written to file.

*/
    inline void createNode(NodeTypeId type);

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
    inline SmartPtr<NodeBase> ptr();

/*
Casts the node pointer to the node class, which is specified in the template parameter. In debug mode, an error message is shown and the dbms will terminate, if the cast was not allowed.

*/
    template<class NodeT>
    inline SmartPtr<NodeT> cast();

private:
    SmartPtr<NodeBase> m_node;        // Pointer to the referred node
    NodeSupp*          m_supp;        // ref. to support object
    SmiRecordId        m_nodeId;      // header record of the node
    SmiRecordId        m_extensionId; // id of first extension page
    unsigned           m_extPageCnt;  // count of extension pages

public:

/*
The following methods wraps the respective methods of the "NodeBase"[4] class:

*/
    inline unsigned memSize(bool recompute = true) const;
    inline bool isCached() const;
    inline void setCached();
    inline void resetCached();
    inline bool isModified() const;
    inline void setModified();
    inline void resetModified();
    inline bool isCacheable() const;
    inline bool isLeaf() const;
    inline NodeTypeId typeId() const;
    inline SmiRecordId chield(unsigned i) const;
    inline void recomputeSize();
    inline bool insert(EntryBase* e);
    inline bool insertCopy(EntryBase* e);
    inline unsigned entryCount() const;
    inline void clear();
    inline void remove(unsigned i);
    inline void replace(unsigned i, EntryBase* newEntry);
    inline unsigned pagecount() const;
    inline unsigned level() const;
    inline void incLevel();
    inline void decLevel();
    inline void setLevel(unsigned level);
    inline unsigned priority() const;
    inline EntryBase* baseEntry(unsigned i) const;
}; // class FileNode

/********************************************************************
1.1.1 Implementation of class "FileNode"[4] (inline methods)

********************************************************************/
/*
Constructor (creates a new node):

*/
FileNode::FileNode(NodeSupp* supp, NodeTypeId type)
        : m_supp(supp), m_nodeId(0),
          m_extensionId(0), m_extPageCnt(0)
{ createNode(type); }

/*
Constructor (read the node from page "nodeId"[4] in file):

*/
FileNode::FileNode(NodeSupp* supp, SmiRecordId nodeId)
        : m_supp(supp)
{ get(nodeId); }

/*
Default copy constructor.

*/
FileNode::FileNode(const FileNode& node)
        : m_node(node.m_node->clone()), m_supp(node.m_supp),
          m_nodeId(node.m_nodeId), m_extensionId(node.m_extensionId),
          m_extPageCnt(node.m_extPageCnt)
{}

/*
Destructor (writes the node to file, if neccesary).

*/
FileNode::~FileNode()
{ put(); }

/*
Method ~clone~:

*/
NodePtr
FileNode::clone() const
{ return NodePtr(new FileNode(*this)); }

/*
Method ~createNode~:

*/
void
FileNode::createNode(NodeTypeId type)
{ m_node = m_supp->createNode(type); }

/*
Method ~ptr~:

*/
SmartPtr<NodeBase>
FileNode::ptr()
{ return m_node; }

/*
Method ~cast~:

*/
template<class NodeT>
SmartPtr<NodeT>
FileNode::cast()
{
#ifdef GTAF_DEBUG
    SmartPtr<NodeT> result = smart_pointer::dynamicCast<NodeBase,
            NodeT, SmartPtr<NodeBase>::counterType> (m_node);

    if (!result.defined())
        Msg::invalidNodeCast_Error();

    return result;

#else
    return  smart_pointer::staticCast<NodeBase,
            NodeT, SmartPtr<NodeBase>::counterType> (m_node);

#endif
}

/*
Method ~memSize~:

*/
unsigned
FileNode::memSize(bool recompute) const
{ return m_node->memSize(recompute); }

/*
Method ~isCached~:

*/
bool
FileNode::isCached() const
{ return m_node->isCached(); }

/*
Method ~setCached~:

*/
void
FileNode::setCached()
{ m_node->setCached(); }

/*
Method ~resetCached~:

*/
void
FileNode::resetCached()
{ m_node->resetCached(); }

/*
Method ~isModified~:

*/
bool
FileNode::isModified() const
{ return m_node->isModified(); }

/*
Method ~setModifed~:

*/
void
FileNode::setModified()
{ m_node->setModified(); }

/*
Method ~resetModified~:

*/
void
FileNode::resetModified()
{ m_node->resetModified(); }

/*
Method ~isCacheable~:

*/
bool
FileNode::isCacheable() const
{ return m_node->isCacheable(); }

/*
Method ~isLeaf~:

*/
bool
FileNode::isLeaf() const
{ return m_node->isLeaf(); }

/*
Method ~typeId~:

*/
NodeTypeId
FileNode::typeId() const
{ return m_node->typeId(); }

/*
Method ~chield~:

*/
SmiRecordId
FileNode::chield(unsigned i) const
{ return m_node->chield(i); }

/*
Method ~recomputeSize~:

*/
void
FileNode::recomputeSize()
{ m_node->recomputeSize(); }

/*
Method ~insert~:

*/
bool
FileNode::insert(EntryBase* e)
{ return m_node->insert(e); }

/*
Method ~insertCopy~:

*/
bool
FileNode::insertCopy(EntryBase* e)
{ return m_node->insertCopy(e); }

/*
Method ~entryCount~:

*/
unsigned
FileNode::entryCount() const
{ return m_node->entryCount(); }

/*
Method ~clear~:

*/
void
FileNode::clear()
{ m_node->clear(); }

/*
Method ~remove~:

*/
void
FileNode::remove(unsigned i)
{ m_node->remove(i); }

/*
Method ~replace~:

*/
void
FileNode::replace(unsigned i, EntryBase* newEntry)
{ m_node->replace(i, newEntry); }

/*
Method ~pagecount~:

*/
unsigned
FileNode::pagecount() const
{ return m_node->pagecount(); }

/*
Method ~level~:

*/
unsigned
FileNode::level() const
{ return m_node->level(); }

/*
Method ~incLevel~:

*/
void
FileNode::incLevel()
{ return m_node->incLevel(); }

/*
Method ~decLevel~:

*/
void
FileNode::decLevel()
{ return m_node->decLevel(); }

/*
Method ~setLevel~:

*/
void
FileNode::setLevel(unsigned level)
{ return m_node->setLevel(level); }

/*
Method ~priority~:

*/
unsigned
FileNode::priority() const
{ return m_node->priority(); }

/*
Method ~baseEntry~:

*/
EntryBase*
FileNode::baseEntry(unsigned i) const
{ return m_node->baseEntry(i); }

} // namespace gtaf
#endif // #ifndef __GTAF_FILE_NODE_H
