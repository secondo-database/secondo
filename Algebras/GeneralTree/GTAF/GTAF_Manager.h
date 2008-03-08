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

1.1 Headerfile "GTAF[_]Manager.h"[4]

January-February 2008, Mirko Dibbert

1.1.1 Includes and defines

*/
#ifndef __GTAF_MANAGER_H
#define __GTAF_MANAGER_H

#include <limits>
#include "GTAF_Config.h"
#include "GTAF_Entries.h"

namespace gtaf
{

/********************************************************************
1.1.1 Struct CachePriorityFun:

********************************************************************/

struct DefaultCachePriority
{
    inline static unsigned getPriority(NodePtr &node)
    {
        return 10*node->priority() +
               5*node->level() +
               node->pagecount();
    }
};

/********************************************************************
1.1.1 Class TreeManager

********************************************************************/

class TreeManager
{

public:
    typedef map<SmiRecordId, NodePtr>::iterator iterator;

/*
Default constructor:

*/
    inline TreeManager(NodeSupp* supp);

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
    inline ~TreeManager();

/*
Sets a new priority function, which replaces the default priority function.

*/
    inline void setPriorityFun(PriorityFun pf);
/*
Enables the node cache.

*/
    inline void enableCache();

/*
Removes all nodes from the node cache and disables it.

*/
    inline void disableCache();

/*
Removes all nodes from cache.

(methods needs to resist in headerfile, otherwhise the defines are not set)

*/
    void clearCache()
    {
        if (!cache.empty())
            {
                #ifdef GTAF_SHOW_CACHE_INFOS
                cmsg.info() << endl << "Writing cached nodes to disc... ";
                cmsg.send();
                #endif

                cache.clear();
                curSize = 0;

                #ifdef GTAF_SHOW_CACHE_INFOS
                cmsg.info() << " [OK]" << endl;
                cmsg.send();
                #endif

                #ifdef GTAF_SHOW_CACHE_INFOS
                cmsg.info() << endl << "node-cache m_hits   : " << m_hits
                << endl << "node-cache m_misses : " << m_misses
                << endl << endl;
                cmsg.send();
                #endif
            }
    }

/*
Returns the current size of the cache.

*/
    inline unsigned cacheSize();

/*
Creates a new node of the specified type. The optional "level"[4] value specifies the level of the node in the tree (whereas leaf level = 0) and should be used, if the node cache has been enabled, since iter is used to determine the priority of the nodes in the cache.

If the node level of a cached node has been changed (e.g. if a new chield has been inserted), iter could be updated with the respective node methods ("incLevel"[4], "decLevel"[4], "setLevel"[4]), otherwhise the cache priority mechanism would possibly not work as expected.

*/
    NodePtr createNode(NodeTypeId type, unsigned level = 0);

/*
Like ~createNode~, but uses current node level as level for the new node.

*/
    inline NodePtr createNeighbourNode(NodeTypeId type);

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
    inline bool hasParent();

/*
Returns a smart pointer to the current node.

*/
    inline NodePtr curNode();

/*
Returns a smart pointer to the parent node.

*/
    inline NodePtr parentNode();

/*
Returns the level of the current node in path (should be equal to "curNode()->level()"[4]).

*/
    inline unsigned curLevel();

/*
Returns a pointer to the parent entry. The node type of the parent is excepted as template parameter.

*/
    template<class NodeT>
    inline typename NodeT::entryType* parentEntry();

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
    NodeSupp* nodeSupp;   // reference to the NodeSupp object
    list<PathEntry> path; // path to current node
    unsigned m_level;     // tree level of current node
    bool useCache;        // true, if cache should be used
    unsigned curSize;     // current size of the cache
    unsigned minPriority; /* maximal minimum priority of a node, that
                             has been removed from cache (only nodes
                             with a priority greater than this value
                             will be inserted into the cache) */
    unsigned m_hits, m_misses; // statistic infos
    map<SmiRecordId, NodePtr> cache; // node cache
    PriorityFun getPriority;
}; // TreeManager


/********************************************************************
1.1.1 Implementation of class "TreeManager"[4] (inline methods)

********************************************************************/
/*
Default constructor:

*/
TreeManager::TreeManager(NodeSupp* supp)
        : m_curNode(), m_parentNode(), nodeSupp(supp), m_level(0),
        useCache(false), curSize(0), minPriority(0), m_hits(0),
        m_misses(0), getPriority(DefaultCachePriority::getPriority)
{}

/*
Destructor:

*/
TreeManager::~TreeManager()
{ clearCache(); }

/*
Method ~setPriorityFun~:

*/
void
TreeManager::setPriorityFun(PriorityFun pf)
{ getPriority = pf; }

/*
Method ~enableCache~:

*/
void
TreeManager::enableCache()
{ useCache = true; }

/*
Method ~disableCache~:

*/
void
TreeManager::disableCache()
{
    clearCache();
    useCache = false;
}

/*
Method ~cacheSize~

*/
unsigned
TreeManager::cacheSize()
{ return curSize; }

/*
Method ~createNeighbourNode~

*/
NodePtr
TreeManager::createNeighbourNode(NodeTypeId type)
{ return createNode(type, m_level); }

/*
Method ~hasParent~

*/
bool
TreeManager::hasParent()
{ return !path.empty(); }

/*
Method ~curNode~

*/
NodePtr
TreeManager::curNode()
{ return m_curNode; }

/*
Method ~parentNode~

*/
NodePtr
TreeManager::parentNode()
{ return m_parentNode; }

/*
Method ~curLevel~

*/
unsigned
TreeManager::curLevel()
{ return m_level; }

/*
Method ~parentEntry~:

*/
template<class NodeT>
typename NodeT::entryType*
TreeManager::parentEntry()
{
    // could not use the cast method of m_parerntNode, since calling
    // template member methods from template classes does not work
    // under windows
    NodeBase* parent = m_parentNode->ptr().operator->();
    NodeT* node = 0;
#ifdef GTAF_DEBUG
    node = dynamic_cast<NodeT*>(parent);
    if (!node)
        Msg::invalidNodeCast_Error();
#else
    node = static_cast<NodeT*>(parent);
#endif
    return node->entry(path.back().parentIndex);
}

} // namespace gtaf
#endif // #ifndef __GTAF_MANAGER_H
