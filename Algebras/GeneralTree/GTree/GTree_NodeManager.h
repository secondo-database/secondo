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

1.1 Headerfile "GTree[_]NodeManager.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_NODE_MANAGER_H__
#define __GTREE_NODE_MANAGER_H__

#include <map>
#include "GTree_NodeBase.h"

namespace gtree
{

/********************************************************************
Class ~NodeManager~

This class manages the node prototypes and could create new nodes of a specific type by calling the clone method of the respective prototype node.

********************************************************************/
class NodeManager
{
  public:

    typedef std::map<NodeTypeId, NodeBase*>::iterator iterator;

/*
Constructor.

*/
    NodeManager(SmiRecordFile *treeFile)
        : m_file(treeFile)
    {}

/*
Destructor.

*/
    ~NodeManager()
    {
        for (iterator iter = m_prototypes.begin();
                iter != m_prototypes.end(); ++iter)
        {
            delete iter->second;
        }
    }

/*
Adds a new prototype node.

*/
    inline void addPrototype(NodeBase* node)
    { m_prototypes[node->typeId()] = node; }

/*
Returns a copy of the prototype node with id "type"[4]. If such a node does not exist, it prints an error message and halts the dbms or returns 0 (if assertions are disabled).

*/
    NodeBase *createNode(NodeTypeId type)
    {
        iterator iter = m_prototypes.find(type);

        if (iter != m_prototypes.end())
        {
            return iter->second->clone();
        }
        else // node type not found in prototype map
        {
            Msg::undefinedNodeType_Error(type);
            return 0;
        }
    }

/*
Adds all prototype nodes from the "src"[4] objects to this object (used in the "Tree"[4] copy constructor).

*/
    void copyPrototypes(NodeManager *src)
    {
        for (size_t i = 0; i < src->m_prototypes.size(); ++i)
            addPrototype(src->m_prototypes[i]->clone());
    }

/*
Returns a reference to the tree file.

*/
    inline SmiRecordFile *file() const
    { return m_file; }

  private:
    SmiRecordFile *m_file;
    // reference to the tree file

    std::map<NodeTypeId, NodeBase*> m_prototypes;
    // prototypes for all registered nodes
}; // class NodeManager

} // namespace gtree
#endif // #define __GTREE_NODE_MANAGER_H__
