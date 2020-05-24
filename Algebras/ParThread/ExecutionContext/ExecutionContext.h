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

1 Header File: ExecutionContext

September 2019, Fischer Thomas

1.1 Overview

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

public:
  ExecutionContext(int contextId, QueryProcessor *qp, OpTree partialTree,
                   ExecutionContextSetting settings, 
                   int numOfParallelEntities,
                   int idxPartitionAttribute);

  virtual ~ExecutionContext();

  /*
  Init 
 
  Initializes the execution context if not already done. All entities of this 
  context are opened after the first entity of the parent context reaches
  this method. Then the state of the context is set to initialized. 
  */
  void Init();

  /*

 
  */
  void Open(ParNodeInfo *nodeInfo);

  /*
  2 Close 

  */
  void Close(ParNodeInfo *entity);

  /*
  Fetches the next tuple from the ConcurrentTupleBuffer and returns 
  YIELD if a tuple is available (and a reference to the tuple as out parameter).
  If no tuple is available, but the tuple stream is not yet dried up, the 
  method creates a new task. In this case the thread continues with the next 
  child-context. Otherwise, if the stream is at the end the request returns 
  CANCEL and NULL-pointer.

  */
  int Request(ParNodeInfo *nodeInfo, Tuple *&tuple);

  /*
  The method just hands on the command to all connected child contexts
  and finish them in a serial order

  */
  void Finish();
  /*
 
  */


  void ScheduleContextTask(tbb::task_group &taskGroup,
                           ExecutionContextEntity *entity = NULL);


  int ContextId() const
  {
    return m_id;
  }

  void AddChildContexts(ExecutionContext *ctx);

  const std::vector<ExecutionContext *> &ChildContexts() const
  {
    return m_childContexts;
  }

  void ParentContext(ExecutionContext *&parent)
  {
    m_parentContext = parent;
  }

  ExecutionContext *ParentContext()
  {
    return m_parentContext;
  }

  ExecutionContextEntityManager *ContextEntities()
  {
    return m_entityManager.get();
  }

  ExecutionContextStates Status()
  {
    return m_contextState;
  }

  OpTree &PreviousParNode()
  {
    return m_parNode;
  }

  OpTree &ContextRootNode()
  {
    return m_rootNode;
  }

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
