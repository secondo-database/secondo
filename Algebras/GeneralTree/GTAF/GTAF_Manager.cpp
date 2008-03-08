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

1.1 Implementation file "GTAF[_]Manager.cpp"[4]

January-February 2008, Mirko Dibbert
\\[3ex]
This file implements the "TreeManger"[4] class.

*/
#include "GTAF_Config.h"
#include "GTAF_Manager.h"

using namespace gtaf;

/*
Method ~createNode~:

*/
NodePtr
TreeManager::createNode(NodeTypeId type, unsigned level)
{
    if (useCache)
    {
        ++m_misses;

        NodePtr node;

        try
        {
            node = new FileNode(nodeSupp, type);
            node->setLevel(level);
            putToCache(node);
        }
        catch (bad_alloc&)
        {
            disableCache();

            try
            {
                node = new FileNode(nodeSupp, type);
            }
            catch (bad_alloc&)
            {
                cmsg.error()
                << "Not enough memory to create new node!" << endl;
                cmsg.send();
            }
        }

        return node;
    }
    else
    {
        NodePtr node;

        try
        {
            node = new FileNode(nodeSupp, type);
        }
        catch (bad_alloc&)
        {
            cmsg.error()
            << "Not enough memory to create new node!" << endl;
            cmsg.send();
        }

        return node;
    }
}

/*
Method ~getNode~:

*/
NodePtr
TreeManager::getNode(SmiRecordId nodeId, unsigned level)
{
    if (useCache)
    {
        // search node in cache
        iterator iter = cache.find(nodeId);

        if (iter != cache.end())
        {
            ++m_hits;
            return iter->second;
        }

        // node not found in cache
        ++m_misses;

        // create new node
        NodePtr node;

        try
        {
            node = new FileNode(nodeSupp, nodeId);
            node->setLevel(level);
            putToCache(node);
        }
        catch (bad_alloc&)
        {
            disableCache();

            try
            {
                node = new FileNode(nodeSupp, nodeId);
            }
            catch (bad_alloc&)
            {
                cmsg.error()
                << "Not enough memory to read node!" << endl;
                cmsg.send();
            }
        }

        return node;
    }
    else
    {
        NodePtr node;

        try
        {
            node = new FileNode(nodeSupp, nodeId);
        }
        catch (bad_alloc&)
        {
            cmsg.error()
            << "Not enough memory to read node!" << endl;
            cmsg.send();
        }

        return node;
    }
}

/*
Method ~replaceParentEntry~:

*/
void
TreeManager::replaceParentEntry(InternalEntry* entry)
{
    if (m_parentNode->isCached())
    {
        curSize -= m_parentNode->memSize();
        m_parentNode->replace(path.back().parentIndex, entry);
        curSize += m_parentNode->memSize();

        if (curSize > maxNodeCacheSize)
            cleanupCache();
    }
    else
        m_parentNode->replace(path.back().parentIndex, entry);
}

/*
Method ~insert~:

*/
bool
TreeManager::insert(NodePtr node, EntryBase* entry)
{
    if (node->isCached())
    {
        curSize -= node->memSize();
        bool result = node->insert(entry);
        curSize += node->memSize();

        if (curSize > maxNodeCacheSize)
            cleanupCache();

        return result;
    }
    else
        return node->insert(entry);
}

/*
Method ~insertCopy~:

*/
bool
TreeManager::insertCopy(NodePtr node, EntryBase* e)
{
    if (node->isCached())
    {
        curSize -= node->memSize();
        bool result = node->insertCopy(e);
        curSize += node->memSize();

        if (curSize > maxNodeCacheSize)
            cleanupCache();

        return result;
    }
    else
        return node->insertCopy(e);
}

/*
Method ~clear~:

*/
void
TreeManager::clear(NodePtr node)
{
    if (node->isCached())
    {
        curSize -= node->memSize();
        node->clear();
        curSize += node->memSize();
    }
    else
        node->clear();
}

/*
Method ~remove~:

*/
void
TreeManager::remove(NodePtr node, unsigned i)
{
    if (node->isCached())
    {
        curSize -= node->memSize();
        node->remove(i);
        curSize += node->memSize();
    }
    else
        node->remove(i);
}

/*
Method ~recomputeSize~:

*/
void
TreeManager::recomputeSize(NodePtr node)
{
    if (node->isCached())
    {
        curSize -= node->memSize();
        node->recomputeSize();
        curSize += node->memSize();
    }
    else
        node->recomputeSize();
}

/*
Method ~drop~:

*/
void
TreeManager::drop(NodePtr node)
{
    if (node->isCached())
    {
        iterator iter = cache.find(node->getNodeId());
        iter->second->resetCached();
        curSize -= iter->second->memSize();
        cache.erase(iter);

        curSize -= node->memSize();
        node->drop();
    }
    else
        node->drop();
}

/*
Method ~putToCache~:

*/
void
TreeManager::putToCache(NodePtr node)
{
    if (useCache)
    {
        if (node->isCacheable())
        {
            unsigned p = getPriority(node);

            if (p > minPriority)
            {
                node->setCached();
                curSize += node->memSize();
                cache[node->getNodeId()] = node;

                if (curSize > maxNodeCacheSize)
                    cleanupCache();
            }
        }
    }
}

/*
Method ~cleanupCache~:

*/
void
TreeManager::cleanupCache()
{
    // delete old nodes from cache, if neccesary
    list<PriorityEntry> priorities;

    for (iterator iter = cache.begin(); iter != cache.end(); ++iter)
    {
        priorities.push_back(PriorityEntry(
                                 getPriority(iter->second), iter));
    }

    priorities.sort();

    while (curSize > minNodeCacheSize)
    {
        iterator iter = priorities.front().iter;
        iter->second->resetCached();
        curSize -= iter->second->memSize();
        cache.erase(iter);
        priorities.pop_front();
    }
}

/*
Method ~initPath~:

*/
void
TreeManager::initPath(SmiRecordId rootId, unsigned rootLevel)
{
    m_curNode.reset(); /* writes the node, if iter is not stored
                        anywhere else (e.g. in the cache), to avoid
                        a write afer read error, if curNode already
                        points to the root node */
    path.clear();
    m_parentNode.reset();
    m_level = rootLevel;
    m_curNode = getNode(rootId, m_level);
}

/*
Method ~getChield~

*/
void
TreeManager::getChield(unsigned i)
{
    path.push_back(PathEntry(m_curNode->getNodeId(), i));
    m_parentNode = m_curNode;
    m_curNode = getNode(m_curNode->chield(i), --m_level);
}


/*
Method ~getParent~

*/
void
TreeManager::getParent()
{
#ifdef GT_DEBUG
    if (!hasParent)
        noParentNode_Error();
#endif

    m_curNode = m_parentNode;
    path.pop_back();

    if (!path.empty())
        m_parentNode = getNode(path.back().nodeId, ++m_level);
    else
        m_parentNode.reset();
        /* writes the node, if iter is not stored anywhere else
           (e.g. in the cache) */
}
