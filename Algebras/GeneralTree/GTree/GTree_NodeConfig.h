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

1.1 Headerfile "GTree[_]NodeConfig.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_NODE_CONFIG_H__
#define __GTREE_NODE_CONFIG_H__

namespace gtree
{



class NodeConfig; // forward declaration
typedef SmartPtr<NodeConfig> NodeConfigPtr;
typedef unsigned char NodeTypeId;



/*
Class ~NodeConfig~

This class contains the node type id and some data to configurate the node.

The "minEntries"[4] value could be used to enforce a minimum node size, before "Node::insert"[4] is allowed to return "true"[4], which means that the node should be split (if not, further entries will be inserted nevertheless). The default value guaranties, that the tree does not degenerate to a linked list.

The "maxEntries"[4] and "maxPages"[4] values limit the maximum entrie count for the node. If one of these values is exceeded and the node contains at least "minEntries"[4] entries, the "insert" method will return "true"[4] (split node).

Setting minEntries to 0 would lead to nodes of (potentially) unlimited size (e.g. used for the supernodes in the xtree).

The "userext"[4] pointer is provides a reference to any possible structure, which is avaliable in all nodes (e.g. some data from the tree header). To access this ref. it is neccesary to define individual node types, which then could access the reference with "m[_]config->userext()"[4] and a respective typecast.

Warning: modifying the config object of a node would affect all nodes of the same type, since all these nodes use a reference to the same config object.

*/
class NodeConfig
{

public:

/*
Constructor

*/
    NodeConfig(
        NodeTypeId type,
        unsigned priority = 0,
        unsigned minEntries = 2,
        unsigned maxEntries = std::numeric_limits<unsigned>::max(),
        unsigned maxPages = 1,
        bool cacheable = true,
        void *userext = 0)
            : m_type(type), m_priority(priority),
            m_minEntries(minEntries),
            m_maxEntries(maxEntries),
            m_maxPages(maxPages),
            m_cacheable(cacheable),
            m_userext(userext)
    {
        if (m_minEntries == 0)
            m_minEntries = std::numeric_limits<unsigned>::max();

        if (m_maxEntries < m_minEntries)
            m_maxEntries = m_minEntries;
    }

/*
Destructor

*/
    inline ~NodeConfig() {}

/*
Returns the id of the node type.

*/
    inline NodeTypeId type() const
    { return m_type; }

/*
Returns the priority of the node.

*/
    inline unsigned priority() const
    { return m_priority; }

/*
Returns the minimum count of entries in the node.

*/
    inline unsigned minEntries() const
    { return m_minEntries; }

/*
Returns the maximum count of entries in node.

*/
    inline unsigned maxEntries() const
    { return m_maxEntries; }

/*
Returns the minimum count of pages of the node.

*/
    inline unsigned maxPages() const
    { return m_maxPages; }

/*
Returns "true"[4] if the node could be cached.

*/
    inline bool cacheable() const
    { return m_cacheable; }

/*
Returns a reference to the user extension object.

*/
    inline void *userext() const
    { return m_userext; }

/*
Sets a new user extension object.

*/
    inline void setUserext(void* ext)
    { m_userext = ext; }

private:
    NodeTypeId m_type;     // id of the node type
    unsigned m_priority;   // priority of the node
    unsigned m_minEntries; // minimum count of entries per node
    unsigned m_maxEntries; // maximum count of entries per node
    unsigned m_maxPages;   // min. size of the node in memory pages
    bool m_cacheable;      // cacheable flag
    void *m_userext;       // reference to user defined object
}; // class NodeConfig

} // namespace gtree
#endif // #define __GTREE_NODE_CONFIG_H__
