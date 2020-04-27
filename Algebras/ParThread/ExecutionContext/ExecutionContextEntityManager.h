/*
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

//paragraph    [10]    title:           [{\Large \bf ] [}]
//paragraph    [21]    table1column:    [\begin{quote}\begin{tabular}{l}]     [\end{tabular}\end{quote}]
//paragraph    [22]    table2columns:   [\begin{quote}\begin{tabular}{ll}]    [\end{tabular}\end{quote}]
//paragraph    [23]    table3columns:   [\begin{quote}\begin{tabular}{lll}]   [\end{tabular}\end{quote}]
//paragraph    [24]    table4columns:   [\begin{quote}\begin{tabular}{llll}]  [\end{tabular}\end{quote}]
//[--------]    [\hline]
//characters    [1]    verbatim:   [$]    [$]
//characters    [2]    formula:    [$]    [$]
//characters    [3]    capital:    [\textsc{]    [}]
//characters    [4]    teletype:   [\texttt{]    [}]
//[ae] [\"a]
//[oe] [\"o]
//[ue] [\"u]
//[ss] [{\ss}]
//[<=] [\leq]
//[#]  [\neq]
//[tilde] [\verb|~|]
//[Contents] [\tableofcontents]

1 Header File: ExecutionContextEntityManager

September 2019, Fischer Thomas

1.1 Overview

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_ENTITY_MANAGER_H
#define SECONDO_PARTHREAD_ENTITY_MANAGER_H

#include "ExecutionContextEntity.h"
#include "QueryProcessor.h"
#include "boost/thread/shared_mutex.hpp"
#include "boost/thread/condition_variable.hpp"
#include <map>
#include <bitset>

namespace parthread
{

class ExecutionContextEntityManager
{
public: //types and static members
    typedef std::bitset<12> MessageFilter;
    static long AllMessages;
    static int AllEntityIndices;

public: //methods
    /*
    The entity manager has controll over a fixed number of entities.
    
    */
    ExecutionContextEntityManager(size_t numEntities, OpTree partialTree,
                                  QueryProcessor *queryProcessor,
                                  int executionContextId,
                                  const ExecutionContextSetting &settings);

    virtual ~ExecutionContextEntityManager();

    //returns true if an entity is available
    bool PinSingleEntity(ExecutionContextEntity *&entity,
                         MessageFilter allowedLastMessages = AllMessages,
                         int entityIndexFilter = AllEntityIndices);

    //reserves all entities and return them as vector. The method will block
    //as long as all entitys are available and can be allocated
    bool PinAllEntities(std::vector<ExecutionContextEntity *> &entitys,
                        MessageFilter allowedLastMessages = AllMessages);

    //returns the passed entity to the pool
    void UnpinSingleEntity(ExecutionContextEntity *entity);

    //returns true if all entities managed by this class achieved a given state
    bool AllEntitiesAchievedLastMessage(MessageFilter allowedLastMessages);

    //returns true if at least one entity is available
    inline bool HasUnpinnedEntities()
    {
        boost::shared_lock<boost::shared_mutex> lock(m_access);
        return m_numOfAvailableEntities < m_entityPool.size();
    };

    inline size_t NumberOfAvailableEntities()
    {
        boost::shared_lock<boost::shared_mutex> lock(m_access);
        return m_numOfAvailableEntities;
    };

    //number of all entities managed by this pool
    inline size_t NumberOfEntities()
    {
        boost::shared_lock<boost::shared_mutex> lock(m_access);
        return m_entityPool.size();
    };

private: //methods
    //not threadsave
    ExecutionContextEntity *CreateAndInitEntity(bool copyTemplate);

    //not threadsave
    //iterate through the tree and search for par nodes to set the current
    //entity in local2 space. CopyNodeInfo indicates that the object must be
    //copied for the par- node in the current entity
    void FindParAndSetLocal2(OpTree node, ExecutionContextEntity *entity,
                             bool copyNodeInfo);

    //not threadsave
    //non blocking, not threadsave, returns true if an entity is available
    bool PinAvailableEntity(ExecutionContextEntity *&entity,
                            MessageFilter allowedLastMessages,
                            int entityIndexFilter);

protected: //member
    OpTree m_templatePartialTree;
    QueryProcessor *m_queryProcessor;
    int m_contextId;
    std::atomic_bool m_blockNewAllocations;
    size_t m_numOfAvailableEntities;

    boost::shared_mutex m_access;
    boost::mutex m_accessQueue;
    boost::condition_variable m_allEntitiesAvailable;

    //first element is the entitiy, second is true, if the entity is available
    //otherwise false
    typedef std::pair<ExecutionContextEntity *, bool> EntityAvailability;
    typedef std::map<ExecutionContextEntity *, bool> EntityPool;
    EntityPool m_entityPool;
    const ExecutionContextSetting &m_settings;
};

} // namespace parthread
#endif
