/*
----
This file is part of SECONDO.

Copyright (C) since 2019, University in Hagen, Faculty of Mathematics
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

#include "ExecutionContextEntityManager.h"
#include <thread>
#include "boost/filesystem.hpp"
#include "../ParNodeInfo.h"

namespace fs = boost::filesystem;

namespace parthread
{

int ExecutionContextEntityManager::AllEntityIndices = -1;
long ExecutionContextEntityManager::AllMessages = 4095;

ExecutionContextEntityManager::ExecutionContextEntityManager(
    size_t numEntities, OpTree partialTree, QueryProcessor *queryProcessor,
    int executionContextId, const ExecutionContextSetting &settings)
    : m_templatePartialTree(partialTree), m_queryProcessor(queryProcessor),
      m_contextId(executionContextId), m_blockNewAllocations(false),
      m_numOfAvailableEntities(0), m_settings(settings)

{
  //create all entities
  for (size_t i = 0; i < numEntities - 1; i++)
  {
    ExecutionContextEntity *entity = CreateAndInitEntity(true);
    m_entityPool.insert(EntityAvailability(entity, false));
  }

  //take the template as operator tree for the last entity
  ExecutionContextEntity *entity = CreateAndInitEntity(false);
  m_entityPool.insert(EntityAvailability(entity, false));
}

ExecutionContextEntityManager::~ExecutionContextEntityManager()
{
  std::vector<ExecutionContextEntity *> entities(m_entityPool.size());

  //all managed entities must be available at destruction of the object.
  if (PinAllEntities(entities))
  {
    for (size_t i = 0; i < entities.size(); i++)
    {
      ExecutionContextEntity *entity = entities[i];
      assert(entity != NULL);

      m_queryProcessor->Destroy(entity->PartialTree, true);
      delete entity;
    }
  }
}

bool ExecutionContextEntityManager::PinSingleEntity(
    ExecutionContextEntity *&entity, MessageFilter allowedLastMessages,
    int entityIndexFilter)
{
  if (allowedLastMessages.none() ||
      m_blockNewAllocations)
  {
    return false;
  }

  boost::unique_lock<boost::shared_mutex> lock(m_access);

  return PinAvailableEntity(entity, allowedLastMessages, entityIndexFilter);
}

bool ExecutionContextEntityManager::PinAllEntities(
    std::vector<ExecutionContextEntity *> &entities,
    MessageFilter allowedLastMessages)
{
  entities.clear();
  if (m_blockNewAllocations)
  {
    return false;
  }
  m_blockNewAllocations = true;

  boost::unique_lock<boost::mutex> queuelock(m_accessQueue);
  while (m_numOfAvailableEntities < m_entityPool.size())
  {
    m_allEntitiesAvailable.wait(queuelock);
  }

  boost::unique_lock<boost::shared_mutex> accessLock(m_access);
  ExecutionContextEntity *entity = NULL;
  while (PinAvailableEntity(entity, allowedLastMessages, AllEntityIndices))
  {
    entities.push_back(entity);
  }

  m_blockNewAllocations = false;
  return entities.size() > 0;
}

void ExecutionContextEntityManager::UnpinSingleEntity(
    ExecutionContextEntity *entity)
{
  boost::unique_lock<boost::shared_mutex> lock(m_access);
  bool foundEntity = false;

  EntityPool::iterator it = m_entityPool.find(entity);
  if (it != m_entityPool.end())
  {
    entity = it->first;
    m_entityPool.at(it->first) = false;
    m_numOfAvailableEntities++;

    foundEntity = true;
  }
  assert(foundEntity);

  //free unique lock to allow shared lock
  lock.unlock();
  m_allEntitiesAvailable.notify_one();
}

bool ExecutionContextEntityManager::PinAvailableEntity(
    ExecutionContextEntity *&entity, MessageFilter allowedLastMessages,
    int entityIndexFilter)
{
  //non blocking, not threadsave, returns true if an entity is available
  entity = NULL;

  if (m_numOfAvailableEntities > 0)
  {
    //find the first available entity
    for (EntityAvailability curEntity : m_entityPool)
    {
      if (curEntity.second == false &&
          allowedLastMessages.test(curEntity.first->LastSendMessage) &&
          (entityIndexFilter == AllEntityIndices ||
           curEntity.first->EntityIdx == entityIndexFilter))
      {
        m_entityPool.at(curEntity.first) = true;
        entity = curEntity.first;
        m_numOfAvailableEntities--;
        assert(m_numOfAvailableEntities >= 0);
        break;
      }
    }
  }

  return entity != NULL;
}

ExecutionContextEntity *ExecutionContextEntityManager::CreateAndInitEntity(
    bool copyTemplate)
{
  //Copy the uninitialized entity of the operator tree. This is
  //necessary to avoid duplicating pointer to initialised objects in
  //local2 space. The last possible entity should use the tree template
  //itself to make sure to destroy all node objects of this partial tree.
  OpTree partialTree = m_templatePartialTree;
  if (copyTemplate)
  {
    partialTree = (OpTree)m_queryProcessor->CopySupplier(m_templatePartialTree);
  }

  ExecutionContextEntity *entity = new ExecutionContextEntity(
      m_entityPool.size(), partialTree);

  FindParAndSetLocal2(entity->PartialTree, entity, copyTemplate);
  if (m_settings.Logger->DebugMode())
  {
    fs::path outputPath(m_settings.Logger->LoggerDebugPath());
    std::stringstream fileNameBuilder;
    fileNameBuilder << "partialTree_ContextEntity_" << m_contextId << "_"
                    << entity->EntityIdx << ".gv";
    outputPath /= fileNameBuilder.str();
    m_queryProcessor->OutputToDotFile(entity->PartialTree, outputPath.c_str());
  }

  //Initialize the new tree
  m_queryProcessor->InitTree(entity->PartialTree);

  m_numOfAvailableEntities++;
  return entity;
}

bool ExecutionContextEntityManager::AllEntitiesAchievedLastMessage(
    MessageFilter allowedLastMessages)
{
  boost::unique_lock<boost::shared_mutex> lock(m_access);

  for (EntityAvailability curEntity : m_entityPool)
  {
    if (!allowedLastMessages.test(curEntity.first->LastSendMessage))
    {
      return false;
    }
  }

  return true;
}

void ExecutionContextEntityManager::FindParAndSetLocal2(
    OpTree node, ExecutionContextEntity *entity, bool copyNodeInfo)
{
  if (!m_queryProcessor->IsOperatorNode(node))
  {
    return;
  }

  //add the entity to local2 space if an par node was found
  Operator *op = m_queryProcessor->GetOperator(node);

  if (op != NULL &&
      op->GetName() == "par")
  {
    ParNodeInfo *nodeInfo = static_cast<parthread::ParNodeInfo *>(
        m_queryProcessor->GetLocal2(node).addr);

    if (copyNodeInfo)
    {
      //create a new parNodeInfo object as copy of the initial one
      //This is necessary to make sure that for each par node and entity has 
      //an independent pointer to a tuplereader
      nodeInfo = new ParNodeInfo(*nodeInfo);

      m_queryProcessor->GetLocal2(node).addr = nodeInfo;
    }

    nodeInfo->CurrentEntity(entity);

    return;
  }

  //iterate through the opeartor tree in depth first order
  for (int i = 0; i < m_queryProcessor->GetNoSons(node); i++)
  {
    FindParAndSetLocal2((OpTree)m_queryProcessor->GetSon(node, i), entity,
                        copyNodeInfo);
  }
}

} // namespace parthread
