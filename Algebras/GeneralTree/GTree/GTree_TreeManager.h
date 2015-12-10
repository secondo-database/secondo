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

1.1 Headerfile "GTree[_]TreeManager.h"[4]

January-May 2008, Mirko Dibbert

*/
#ifndef __GTREE_TREE_MANAGER_H__
#define __GTREE_TREE_MANAGER_H__

#include <list>
#include "GTree_FileNode.h"
#include "GTree_InternalEntry.h"
#include "GTree_LeafEntry.h"

namespace gtree
{


typedef unsigned (*PriorityFun)(NodePtr& node);

/********************************************************************
Class ~TreeManager~

********************************************************************/
class TreeManager
{

public:
    typedef std::map<SmiRecordId, NodePtr>::iterator iterator;

/*
Default constructor:

*/
    inline TreeManager(NodeManager* mngr)
        : m_curNode(), m_parentNode(), nodeSupp(mngr), m_level(0),
          useCache(false), curSize(0), minPriority(0), m_hits(0),
          m_misses(0), getPriority(TreeManager::priorityFun)
    {}

private:
/*
Default copy constructor and assignment operator (not implemented).

*/
    TreeManager(const TreeManager&);
    void operator = (const TreeManager&);

public:
/*
Destructor:

*/
    inline ~TreeManager()
    { clearCache(); }

/*
Default priority function - could be replaced with another one with the "setPriorityFun"[4] method.

*/
    inline static unsigned priorityFun(NodePtr &node)
    {
        return 10*node->priority() +
               5*node->level() +
               node->pagecount();
    }

/*
Sets a new priority function, which replaces the default priority function.

*/
    inline void setPriorityFun(PriorityFun pf)
    { getPriority = pf; }

/*
Enables the node cache.

*/
    inline void enableCache()
    { useCache = true; }

/*
Removes all nodes from the node cache and disables it.

*/
    inline void disableCache()
    {
        clearCache();
        useCache = false;
    }

/*
Removes all nodes from cache.

*/
    void clearCache();

/*
Returns the current size of the cache.

*/
    inline unsigned cacheSize()
    { return curSize; }

/*
Creates a new node of the specified type. The optional "level"[4] value specifies the level of the node in the tree (whereas leaf level = 0) and should be used, if the node cache has been enabled, since iter is used to determine the priority of the nodes in the cache.

If the node level of a cached node has been changed (e.g. if a new chield has been inserted), iter could be updated with the respective node methods ("incLevel"[4], "decLevel"[4], "setLevel"[4]), otherwhise the cache priority mechanism would possibly not work as expected.

*/
    NodePtr createNode(NodeTypeId type, unsigned level = 0);

/*
Like ~createNode~, but uses current node level as level for the new node.

*/
    inline NodePtr createNeighbourNode(NodeTypeId type)
    { return createNode(type, m_level); }

/*
The optional "level"[4] value specifies the level of the node in the tree (whereas leaf level = 0) and should be used, if the node cache has been enabled, since iter is used to determine the priority of the nodes in the cache.

If the node level of a cached node has been changed (e.g. if a new chield has been inserted), iter could be updated with the respective node methods ("incLevel"[4], "decLevel"[4], "setLevel"[4]), otherwhise the cache priority mechanism would possibly not work as expected.

*/
    NodePtr getNode(SmiRecordId nodeId, unsigned level = 0);

/*
Initiates a new path from the specified root. The curNode pointer will be set to the root node.

*/
    void initPath(SmiRecordId rootId, unsigned rootLevel);

/*
Sets the curNode pointer to the i-th chield of current node.

*/
    void getChield(unsigned i);

/*
Sets the curNode pointer to the parent of current node.

*/
    void getParent();

/*
Returns "true", if the current node is not the root.

*/
    inline bool hasParent()
    { return !path.empty(); }

/*
Returns a smart pointer to the current node.

*/
    inline NodePtr curNode()
    { return m_curNode; }

/*
Returns a smart pointer to the parent node.

*/
    inline NodePtr parentNode()
    { return m_parentNode; }

/*
Returns the level of the current node in path (should be equal to "curNode()->level()"[4]).

*/
    inline unsigned curLevel()
    { return m_level; }

/*
Returns a pointer to the parent entry. The node type of the parent is excepted as template parameter.

*/
    template<class TNode>
    inline typename TNode::entryType* parentEntry()
    {
        // could not use the cast method of m_parerntNode, since
        // calling template member methods from template classes does
        //  not work under windows
        NodeBase* parent = m_parentNode->ptr().operator->();
        TNode* node = 0;
#ifdef __GTREE_DEBUG
        node = dynamic_cast<TNode*>(parent);
        if (!node) Msg::invalidNodeCast_Error();

#else
        node = static_cast<TNode*>(parent);
#endif
        return node->entry(path.back().parentIndex);
    }

/*
Replaces the parent entry in parent node with "entry"[4] and updates the used cache size.

*/
    void replaceParentEntry(InternalEntry* entry);

/*
Calls "node->insert"[4] and updates the used cache size.

*/
    bool insert(NodePtr node, EntryBase* entry);

/*
Inserts a copy of "entry"[4] into "node"[4] and updates used cache size.

*/
    bool insertCopy(NodePtr node, EntryBase* e);

/*
Removes all entries from node and updates used cache size.

*/
    void clear(NodePtr node);

/*
Removes the i-th entry from node and updates used cache size.

*/
    void remove(NodePtr node, unsigned i);

/*
Calls "node->recomputeSize"[4] and updates the used cache size.

*/
    void recomputeSize(NodePtr node);

/*
Removes the node from the tree file and the node cache, clears all entries in the node.

*/
    void drop(NodePtr node);

protected:
/*
This struct is used to store the id of the parent node together with the respective parent entry index.

*/
    struct PathEntry
    {
        PathEntry(SmiRecordId _nodeId, unsigned _parentIndex)
                : nodeId(_nodeId), parentIndex(_parentIndex)
        {}

        SmiRecordId nodeId;
        unsigned parentIndex;
    };

/*
This struct is used in the "cleanupCache"[4] method and stores the priority of the node together with an iterator to the node position in the node cache.

*/
    struct PriorityEntry
    {
        PriorityEntry(unsigned _priority, iterator _iter)
                : priority(_priority), iter(_iter)
        {}

        unsigned priority;
        iterator iter;

        bool operator<(const PriorityEntry rhs) const
        { return priority < rhs.priority; }
    };

/*
Inserts "node"[4] into the cache, if the priority of "node"[4] is greater than "minPriority"[4].

*/
    void putToCache(NodePtr node);

/*
Removes at least one node from the cache, if the used cache size has grown greater than "maxNode-"[4] "CacheSize"[4]. In general, many nodes would be removed in one burst, which could be done more efficient than removing only one node per time.

*/
    void cleanupCache();

    NodePtr m_curNode;    // ref. to current node in path
    NodePtr m_parentNode; // ref. to parent node in path
    NodeManager* nodeSupp;   // reference to the NodeManager object
    std::list<PathEntry> path; // path to current node
    unsigned m_level;     // tree level of current node
    bool useCache;        // true, if cache should be used
    unsigned curSize;     // current size of the cache
    unsigned minPriority; /* maximal minimum priority of a node, that
                             has been removed from cache (only nodes
                             with a priority greater than this value
                             will be inserted into the cache) */
    unsigned m_hits, m_misses; // statistic infos
    std::map<SmiRecordId, NodePtr> cache; // node cache
    PriorityFun getPriority;
}; // TreeManager

} // namespace gtree
#endif // #define __GTREE_TREE_MANAGER_H__
