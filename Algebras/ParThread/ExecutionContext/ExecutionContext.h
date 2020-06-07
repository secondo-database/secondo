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

1 Header File: ExecutionContext

September 2019, Fischer Thomas

1.1 Overview

The ~ExecutionContext~ describes a part of the operator tree that can be
executed concurrently and independend from other execution contexts. 

This class supports an interface for all stream messages called by the ~par~
operator.  

It is also the container for all entitities managed by the ~EntityManager~ a 
member of this class and contains a tuple buffer to save tuples before they are 
consumed by adjacent ~par~-operators.

1.2 Imports

*/

#ifndef SECONDO_PARTHREAD_EXECUTIONCONTEXT_H
#define SECONDO_PARTHREAD_EXECUTIONCONTEXT_H

#include "ExecutionContextTypes.h"
#include "../ParNodeInfo.h"
#include "ExecutionContextEntityManager.h"
#include "../ConcurrentTupleBuffer/ConcurrentTupleBuffer.h"

#include <string>
#include <memory>
#include <vector>
#include <future>
#include <tuple>

#include "tbb/task_group.h"

namespace parthread
{
  class ExecutionContext
  {

/*
1.3 Initialization and destruction

*/

  public:
    ExecutionContext(int contextId, QueryProcessor *qp, OpTree partialTree,
                     ExecutionContextSetting settings,
                     int numOfParallelEntities,
                     int idxPartitionAttribute);

    virtual ~ExecutionContext();

/*
Execution context objects are created by the ~ParallelQueryOptimizer~. The 
necessary parameters are as follows:

  * ~contextId~: an unique identifier used for debug purposes.

  * ~qp~: reference to the query processor.
   
  * ~partialTree~: the separated subtree of the operator tree enclosed by this context.
  
  * ~settings~: information and configuration settings loaded from secondo-config.ini
  (see the TestConfigs subdirectory for more information).

  * ~numOfParallelEntities~ and ~idxPartitionAttribute~ are the parameters of the 
  ~par~-operator passed to the execution context.

The constructor sets the context in state created. The only 

1.4 Stream message handler

*/

    void Init();
/*
Setup the execution context for execution if not already done. All entities 
of this context are initialized after the first entity of the parent context 
reaches this method. The buffer is created here too. This handler sets the 
state of the context is set to ~Initialized~. 

*/

    void Open(ParNodeInfo *nodeInfo);
/*
This method pins all entities of this context and calls the open messagen on 
each of them. The precondition is that all parent entities reached this handler
before. The method is responsible to create a tuple reader connected to the 
execution contexts tuple buffer and assign it to the calling ~par~ node. 

Before the method finish the execution context state is set to ~Opened~

*/

    void Close(ParNodeInfo *entity);
/*
The close handler works in a similar way like the open-handler. It forwards the
close message to all entities of this context. Finally it deletes the reader of
the calling ~par~-node and sets the state to ~Closed~

*/

    int Request(ParNodeInfo *nodeInfo, Tuple *&tuple);
/*
Fetches the next tuple from the ConcurrentTupleBuffer and returns 
~YIELD~ if a tuple is available as well a reference to the tuple as out 
parameter.
If no tuple is available, but the tuple stream is not yet dried up, the 
method creates a new task. In this case the thread continues with the next 
child-context. 
Otherwise, if the stream is at the end the request returns ~CANCEL~ and 
NULL-pointer instead of a tuple-reference.

*/

    void Finish();
/*
The method just hands on the command to all connected child contexts
and finish them in a serial order. The state of the context is set to ~Finished~
before the method ends.

1.5 Properties

*/

    int ContextId() const
    {
      return m_id;
    }
/*
The purpose of the ContextId is just to unique identify the context and needed
for debug output.

*/

    void AddChildContexts(ExecutionContext *ctx);

    const std::vector<ExecutionContext *> &ChildContexts() const
    {
      return m_childContexts;
    }
/*
An execution context can have zero to many child context. This are adjacent 
execution context which are connected by the ~par~-nodes of this contexts entities.

The ~ParallelQueryOptimizer~ use the setter to build up this tree structure of
contexts.

*/

    void ParentContext(ExecutionContext *&parent)
    {
      m_parentContext = parent;
    }

    ExecutionContext *ParentContext()
    {
      return m_parentContext;
    }

/*
Every execution context has zero or one parent context. The first context following
the root of the operator tree is the only context that has no parent and can be
identified by this property.

*/

    ExecutionContextStates Status()
    {
      return m_contextState;
    }

/*
Additional readonly getter to the status of the context. See the 
~ExecutionContextStates~ class for detailed information about the context states.

*/

    OpTree &PreviousParNode()
    {
      return m_parNode;
    }

    OpTree &ContextRootNode()
    {
      return m_rootNode;
    }

/*
The ~PreviousParNode~ is the reference to the node in the original operator-tree.
~ContextRootNode~ gets a reference to the first node in the execution context, based
on the original tree too.  

*/

  private: //methods
    bool AllParentEntitiesAchievedState(ExecutionContextStates stateToBeReached,
                                        ExecutionContextEntity *entity);

    void ScheduleContextTaskRecursive(
        int message, std::vector<ExecutionContextEntity *> entities);

    void OnBufferSizeChanged();

    void ScheduleContextTask(int message, ExecutionContextEntity *entity);

    void ScheduleRequestTasks(
        int entityIndex = ExecutionContextEntityManager::AllEntityIndices);

    void TriggerPufferChange();

    size_t ExecuteRequestTask(ExecutionContextEntity *entity);

  protected: //member
    int m_id;
    QueryProcessor *m_queryProcessor;
    OpTree m_rootNode;
    OpTree m_parNode;
    int m_numOfParallelEntities;
    int m_idxPartitionAttribute;

    //Status changes are made in a sequential order.
    ExecutionContextStates m_contextState;
    std::mutex m_stateMtx;

    std::mutex m_bufferSizeHandlerMtx;

    ExecutionContext *m_parentContext;
    std::vector<ExecutionContext *> m_childContexts;

    ExecutionContextSetting m_settings;
    ConcurrentTupleBufferPtr m_buffer;

    std::unique_ptr<ExecutionContextEntityManager> m_entityManager;
    std::unique_ptr<ExecutionContextGuard> m_contextGuard;
  };

} // namespace parthread
#endif
