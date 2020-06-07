/*
---- 
This file is part of SECONDO.

Copyright (C) 2019, University in Hagen, Department of Computer Science, 
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

The ~ExecutionContextEntityManager~ is a collector for the entities of the 
execution context. It creates entities and destroys them after execution.
The manager is responsible that each entity is just used by one thread at a 
time.

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
1.3 Initialization and destruction

*/
    ExecutionContextEntityManager(size_t numEntities, OpTree partialTree,
                                  QueryProcessor *queryProcessor,
                                  int executionContextId,
                                  const ExecutionContextSetting &settings);

    virtual ~ExecutionContextEntityManager();
/*
The number of entities is fixed and must be known at initialization of the class.
~partialTree~ is the original, but already separated part of the operator tree. 
The manager uses methods from ~QueryProcessor~ to copy the tree to multiple instances.

During destruction all entities are pinned and ~destroy~ is called on each partial
tree. 

1.4 Methods

All following methods are threadsave and can be called in a concurrent environment.

*/
    bool PinSingleEntity(ExecutionContextEntity *&entity,
                         MessageFilter allowedLastMessages = AllMessages,
                         int entityIndexFilter = AllEntityIndices);
/*
Pins a single entity. The entity is returned as out parameter and the return 
values indicates if the call was successfull. Optional the entities can be 
restricted to the last called message or a certain index.

*/

    bool PinAllEntities(std::vector<ExecutionContextEntity *> &entitys,
                        MessageFilter allowedLastMessages = AllMessages);
/*
Similar to the previous method, but reserves all entities and return them as 
vector. The method will block as long as all entitys are available and can be 
allocated. It is possible to constrain this to entities with a certain messages

*/

    void UnpinSingleEntity(ExecutionContextEntity *entity);
/*
Returns a single entity back to the managed pool of entities. After an entity was
unpinned through this method it can be used by other threads.  

*/

    bool AllEntitiesAchievedLastMessage(MessageFilter allowedLastMessages);
/*
Returns true if all entities managed by this class achieved a given state.

*/


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

/*
Returns true if at least one entity is available or the number of available entities.
This is no guarantee that a following call of ~UnpinSingleEntity~ will succeed 
if more than one thread access the entity manager and should only be used for 
debugging purposes.

*/

    inline size_t NumberOfEntities()
    {
        boost::shared_lock<boost::shared_mutex> lock(m_access);
        return m_entityPool.size();
    };
/*
Gets the constant size of entities managed by this class.

*/


private: //methods

    ExecutionContextEntity *CreateAndInitEntity(bool copyTemplate);

    void FindParAndSetLocal2(OpTree node, ExecutionContextEntity *entity,
                             bool copyNodeInfo);

/*
Create a new entity call initialize to the partial tree. If ~copyTemplate~ is
set the partial tree will be copied otherwise the original tree structure is used.

The ~FindParAndSetLocal2~ iterates through the tree and search for par nodes to
set the current entity in local2 space. CopyNodeInfo indicates that the object 
must be copied for the par- node in the current entity

*/

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
