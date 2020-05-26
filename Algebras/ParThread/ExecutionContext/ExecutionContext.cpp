/*
----
This file is part of SECONDO.

Copyright (C) since 2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

*/

#include "ExecutionContext.h"
#include "../ConcurrentTupleBuffer/ConcurrentTupleBufferReader.h"
#include "../ConcurrentTupleBuffer/ConcurrentTupleBufferWriter.h"

#include "tbb/tbb.h"
#include <iostream>

using namespace std;
using namespace std::chrono;

namespace parthread
{

  ExecutionContext::ExecutionContext(int contextId, QueryProcessor *qp,
                                     OpTree parNodeRef,
                                     ExecutionContextSetting setting,
                                     int numOfParallelEntities,
                                     int idxPartitionAttribute)
      : m_id(contextId), m_queryProcessor(qp),
        m_numOfParallelEntities(numOfParallelEntities),
        m_idxPartitionAttribute(idxPartitionAttribute),
        m_contextState(ExecutionContextStates::Created),
        m_parentContext(NULL), m_settings(setting)
  {
    //the par operator has only one son, this is the rootnode of
    //the execution context
    m_parNode = parNodeRef;
    m_rootNode = (OpTree)qp->GetSon(parNodeRef, 0);
  };

  ExecutionContext::~ExecutionContext()
  {
    for (ExecutionContext *child : m_childContexts)
    {
      delete child;
      child = NULL;
    }
  }

  void ExecutionContext::Init()
  {
    //do nothing if context is already initialized
    if (Status() == ExecutionContextStates::Created)
    {
      //use double-checked locking to avoid synchronisation effort
      std::unique_lock<std::mutex> lock(m_stateMtx);

      if (Status() == ExecutionContextStates::Created)
      {
        //Initialize the buffer
        ConcurrentTupleBufferSettings bufferSettings;

        size_t parentNumberOfEntities = 1;
        if (ParentContext() != NULL)
        {
          parentNumberOfEntities =
              ParentContext()->m_entityManager->NumberOfEntities();
        }
        if (m_idxPartitionAttribute >= 0)
        {
          bufferSettings.DataPartitioner.reset(
              new HashDataPartitioner(parentNumberOfEntities,
                                      m_idxPartitionAttribute));
        }
        else
        {
          bufferSettings.DataPartitioner.reset(
              new RoundRobinDataPartitioner(parentNumberOfEntities));
        }

        bufferSettings.InitialQueueCapacity = m_settings.QueueCapacityThreshold;
        bufferSettings.MemoryDistributionFactor =
            m_numOfParallelEntities *
            m_settings.QueueCapacityThreshold;
        bufferSettings.InitialNumberOfTuplesPerBlock =
            m_settings.MaxNumberOfTuplesPerBlock;

        //the memory assigned to the operator can be used completly for the
        //tuple buffer
        bufferSettings.TotalBufferSizeInBytes =
            m_queryProcessor->GetMemorySize(m_parNode) * 1024 * 1024;
        bufferSettings.MinMemoryPerTupleVector = 1024;
        bufferSettings.TupleBlocksToRecycle =
            m_settings.QueueCapacityThreshold * 2;
        m_buffer.reset(new ConcurrentTupleBuffer(bufferSettings));

        //Initialize the guard. The first parent context has no guard, therefore
        //the number of entities is 1
        size_t numParentEntities = 1;
        if (m_parentContext != NULL)
        {
          numParentEntities =
              m_parentContext->m_entityManager->NumberOfEntities();
        }
        m_contextGuard.reset(new ExecutionContextGuard(numParentEntities));

        //Initialize the entity pool
        m_entityManager.reset(new ExecutionContextEntityManager(
            m_numOfParallelEntities, m_rootNode, m_queryProcessor,
            m_id, m_settings));

        m_contextState = ExecutionContextStates::Initialized;
        if (m_settings.Logger->DebugMode())
        {
          std::stringstream message;
          message << "Initialized execution context " << ContextId();
          m_settings.Logger->WriteDebugOutput(message.str());
        }
      }
    }

    //iterative init all contexts of the contexttree
    for (ExecutionContext *child : m_childContexts)
    {
      child->Init();
    }
  }

  void ExecutionContext::Open(ParNodeInfo *nodeInfo)
  {
    ExecutionContextStates stateToBeReached = ExecutionContextStates::Opened;

    m_contextGuard->SetState(nodeInfo->CurrentEntity()->EntityIdx,
                             stateToBeReached);

    if (m_contextGuard->Pass(stateToBeReached))
    {
      std::unique_lock<std::mutex> lock(m_stateMtx);
      m_contextGuard->Reset();

      //do nothing if context is already open or requesting
      if (Status() == ExecutionContextStates::Initialized ||
          Status() == ExecutionContextStates::Closed)
      {
        std::vector<ExecutionContextEntity *> entities;

        ExecutionContextEntityManager::MessageFilter filter;
        filter.set(INIT);
        filter.set(CLOSE);
        filter.set(CANCEL);

        m_entityManager->PinAllEntities(entities, filter);
        ScheduleContextTaskRecursive(OPEN, entities);

        m_contextState = stateToBeReached;
        if (m_settings.Logger->DebugMode())
        {
          std::stringstream message;
          message << "Opened execution context " << ContextId();
          m_settings.Logger->WriteDebugOutput(message.str());
        }
      }
    }

    nodeInfo->TupleReader(m_buffer->GetTupleBufferReader(
        nodeInfo->CurrentEntity()->EntityIdx));
  }

  void ExecutionContext::Close(ParNodeInfo *nodeInfo)
  {
    ExecutionContextStates stateToBeReached = ExecutionContextStates::Closed;

    m_contextGuard->SetState(nodeInfo->CurrentEntity()->EntityIdx,
                             stateToBeReached);

    if (m_contextGuard->Pass(stateToBeReached))
    {
      std::unique_lock<std::mutex> lock(m_stateMtx);
      m_contextGuard->Reset();

      //do nothing if context is already closed/open
      if (Status() == ExecutionContextStates::Opened ||
          Status() == ExecutionContextStates::Canceled ||
          Status() == ExecutionContextStates::Closed)
      {
        std::vector<ExecutionContextEntity *> entities;

        //close can be called on entities in every state
        m_entityManager->PinAllEntities(entities);
        ScheduleContextTaskRecursive(CLOSE, entities);

        m_contextState = stateToBeReached;
      }

      if (m_settings.Logger->DebugMode())
      {
        std::stringstream message;
        message << "Closed execution context " << ContextId();
        m_settings.Logger->WriteDebugOutput(message.str());
      }
    }

    //delete the reader
    ConcurrentTupleBufferReader *reader = nodeInfo->TupleReader();
    if (reader != NULL)
    {
      if (m_settings.Logger->DebugMode())
      {
        std::stringstream message;
        message << "Finished reader of entity "
                << nodeInfo->CurrentEntity()->EntityIdx
                << " related to context " << ContextId()
                << " as " << reader->ToString()
                << " with " << reader->NumReadTuples() << " read tuples";

        m_settings.Logger->WriteDebugOutput(message.str());
      }

      delete reader;
      nodeInfo->TupleReader(NULL);
    }
  }

  int ExecutionContext::Request(ParNodeInfo *nodeInfo, Tuple *&tuple)
  {
    int status = CANCEL;

    if (nodeInfo != NULL &&
        nodeInfo->TupleReader() != NULL)
    {
      ConcurrentTupleBufferReader *reader = nodeInfo->TupleReader();

      while (!reader->IsEndOfDataReached())
      {
        if (reader->TryReadTuple(tuple))
        {
          /* got a new tuple from reader,
            break and return with yield*/
          status = YIELD;
          break;
        }
        else
        {
          /* found no new tuple, so trigger a buffer change*/
          TriggerPufferChange();
        }
      }
    }

    return status;
  }

  void ExecutionContext::AddChildContexts(ExecutionContext *ctx)
  {
    m_childContexts.push_back(ctx);
  }

  void ExecutionContext::Finish()
  {
    //do nothing if context is already destroyed
    if (Status() < ExecutionContextStates::Finished)
    {
      //iterative finish all contexts of the contexttree
      //beginning with the child nodes
      for (ExecutionContext *child : m_childContexts)
      {
        child->Finish();
      }

      //destructor of entity manager destroys all entities
      m_entityManager.reset();

      m_contextState = ExecutionContextStates::Finished;

      if (m_settings.Logger->DebugMode())
      {
        std::stringstream message;
        message << "Finished execution context " << ContextId();
        m_settings.Logger->WriteDebugOutput(message.str());
      }
    }
  }

  void ExecutionContext::ScheduleContextTaskRecursive(
      int message, std::vector<ExecutionContextEntity *> entities)
  {
    if (entities.size() == 0)
    {
      return;
    }
    if (entities.size() == 1)
    {
      ScheduleContextTask(message, entities[0]);
    }
    else
    {
      //fork the entities in two groups, this will support task stealing
      tbb::task_group taskGroup;

      std::size_t const splitIndex = entities.size() / 2;

      std::vector<ExecutionContextEntity *> firstPart(
          entities.begin(), entities.begin() + splitIndex);
      taskGroup.run([&, message] {
        ScheduleContextTaskRecursive(message, firstPart);
      });

      std::vector<ExecutionContextEntity *> secondPart(
          entities.begin() + splitIndex, entities.end());
      taskGroup.run_and_wait([&, message] {
        ScheduleContextTaskRecursive(message, secondPart);
      });
    }
  }

  void ExecutionContext::TriggerPufferChange()
  {
    if (m_buffer->Size() > 0)
    {
      //Trigger the puffer change if the context has a non empty buffer
      OnBufferSizeChanged();
      return;
    }

    //fetch all opened child contexts independent of buffer state
    std::vector<ExecutionContext *> openChildContexts;
    for (ExecutionContext *child : m_childContexts)
    {
      if (child->Status() == ExecutionContextStates::Opened)
      {
        openChildContexts.push_back(child);
      }
    }

    //create a fork of execution if the context tree has multiple branches
    if (openChildContexts.size() > 1)
    {
      tbb::task_group innerTaskGroup;

      for (ExecutionContext *child : openChildContexts)
      {
        innerTaskGroup.run([&] { child->TriggerPufferChange(); });
      }

      innerTaskGroup.wait();
    }
    else if (openChildContexts.size() == 1)
    {
      //avoid forks for degenerated tree structures
      openChildContexts[0]->TriggerPufferChange();
    }
    else
    {
      //reached a leaf of the context tree
      ScheduleRequestTasks();
    }
  }

  void ExecutionContext::ScheduleRequestTasks(int entityIndex)
  {
    ExecutionContextEntityManager::MessageFilter filter;
    filter.set(YIELD);
    filter.set(OPEN);
    ExecutionContextEntity *entity = NULL;

    std::vector<ExecutionContextEntity *> entities;
    while (m_entityManager->PinSingleEntity(entity, filter, entityIndex))
    {
      if (entity->Writer != NULL && entity->Writer->Allocate())
      {
        entities.push_back(entity);
      }
      else
      {
        //no free memory available, so unpin the entity again
        m_entityManager->UnpinSingleEntity(entity);
      }
    }
    ScheduleContextTaskRecursive(REQUEST, entities);

    //if all entities reached cancel state, set the state of the whole
    //context to cancel
    ExecutionContextEntityManager::MessageFilter cancelFilter;
    cancelFilter.set(CANCEL);
    if (m_entityManager->AllEntitiesAchievedLastMessage(cancelFilter))
    {
      std::unique_lock<std::mutex> lock(m_stateMtx);
      m_contextState = ExecutionContextStates::Canceled;
    }
  }

  void ExecutionContext::OnBufferSizeChanged()
  {
    tbb::task_group taskGroup;
    bool needProducerTasks = false;
    for (int i = 0; i < (int)m_buffer->NumberOfPartitions(); i++)
    {
      size_t numberOfTupleBlocks = m_buffer->SizeOfPartition(i);

      if (numberOfTupleBlocks > 0 &&
          ParentContext() != NULL &&
          m_settings.UsePipelineParallelism)
      {
        //create consumer tasks for this partition
        taskGroup.run([&, i] { ParentContext()->ScheduleRequestTasks(i); });
      }
      if (numberOfTupleBlocks < m_settings.QueueCapacityThreshold)
      {
        needProducerTasks = true;
      }
    }

    if (needProducerTasks)
    {
      //create producer tasks
      taskGroup.run_and_wait([&] { ScheduleRequestTasks(); });
    }
    else
    {
      taskGroup.wait();
    }
  }

  void ExecutionContext::ScheduleContextTask(int message,
                                             ExecutionContextEntity *entity)
  {
    assert(entity);
    sc::_V2::high_resolution_clock::time_point startPoint;
    startPoint = sc::high_resolution_clock::now();

    int processedTuples = 0;

    if (message == REQUEST)
    {
      processedTuples = ExecuteRequestTask(entity);
    }
    else if (message == OPEN)
    {
      assert(entity->Writer == NULL);
      entity->Writer = m_buffer->GetTupleBufferWriter();
      m_queryProcessor->Open(entity->PartialTree);

      entity->LastSendMessage = message;

      //release the entity
      ContextEntities()->UnpinSingleEntity(entity);
    }
    else if (message == CLOSE)
    {
      if (entity->Writer != NULL)
      {
        delete entity->Writer;
        entity->Writer = NULL;
      }

      m_queryProcessor->Close(entity->PartialTree);

      entity->LastSendMessage = message;

      //release the entity
      ContextEntities()->UnpinSingleEntity(entity);
    }

    if (m_settings.Logger->DebugMode())
    {
      std::string msgStr = m_queryProcessor->MsgToStr(message);
      m_settings.Logger->WriteTaskTrace(startPoint,
                                        msgStr,
                                        ContextId(),
                                        processedTuples,
                                        entity->EntityIdx);
    }
  }

  size_t ExecutionContext::ExecuteRequestTask(ExecutionContextEntity *entity)
  {
    assert(entity->Writer != NULL);
    Word currentResult;
    size_t numProcessedTuples = 0;

    ThreadSpecificStopWatch sw;
    bool continueRequest = true;
    while (continueRequest &&
           entity->Writer->HasFreeMemory())
    {
      m_queryProcessor->Request(entity->PartialTree, currentResult);

      if (m_queryProcessor->Received(entity->PartialTree))
      {
        //get result and increment the tuple reference counter to keep
        //it into memory
        Tuple *tuple = static_cast<Tuple *>(currentResult.addr);
        tuple->IncReference();

        //increment reference counter before adding tuple to buffer
        entity->Writer->WriteTuple(tuple);
        numProcessedTuples++;
        entity->LastSendMessage = YIELD;
      }
      else
      {
        //indicates that no more data will be written to the buffer
        entity->LastSendMessage = CANCEL;
        break;
      }

      if (numProcessedTuples > 0 &&
          (numProcessedTuples % m_settings.MaxNumberOfTuplesPerBlock) == 0)
      {
        //release the tuple block
        if (sw.SpendTimeInMicroseconds() <
            m_settings.TimeoutPerTaskInMicroSeconds)
        {
          entity->Writer->Flush();
          continueRequest = entity->Writer->Allocate();
        }
        else
        {
          continueRequest = false;
        }
      }
    }

    //release the tuple block
    entity->Writer->Flush();

    if (entity->LastSendMessage == CANCEL)
    {
      if (m_settings.Logger->DebugMode())
      {
        std::stringstream message;
        message << "Reached cancel: Finished writer of entity "
                << entity->EntityIdx
                << " related to context " << ContextId()
                << " with " << entity->Writer->NumWrittenTuples()
                << " written tuples";

        m_settings.Logger->WriteDebugOutput(message.str());
      }

      delete entity->Writer;
      entity->Writer = NULL;
    }

    //release the entity
    ContextEntities()->UnpinSingleEntity(entity);

    return numProcessedTuples;
  };

} // namespace parthread