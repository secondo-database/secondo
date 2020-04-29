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
#ifdef USE_MULTIHREADED_QUERY_PROCESSING
#include "ParallelQueryOptimizer.h"

#include "Operator.h"
#include "QueryProcessor.h"
#include "Profiles.h"
#include "Algebras/ParThread/ExecutionContext/ExecutionContext.h"
#include "tbb/task_scheduler_init.h"
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"

#include <string>
#include <algorithm>
#include <thread>
#include <iostream>
#include <sstream>
#include <stack>

namespace fs = boost::filesystem;

namespace parthread
{

const char *ParallelQueryOptimizer::ParOperatorName = "par";

struct ContextInformation
{
  ContextInformation(OpNode *parNode)
      : ParNode(parNode), ParentContext(NULL), MaxDegreeOfDataParallelism(1),
        NumberOfOperators(0), IsValid(true), IsHashPartitioned(false),
        PartitonAttributeIndex(-1){};

  OpNode *ParNode;
  ContextInformation *ParentContext;
  int MaxDegreeOfDataParallelism;
  int NumberOfOperators;
  bool IsValid;
  bool IsHashPartitioned;
  int PartitonAttributeIndex;
  std::vector<OpNode *> MemoryOperators;

  ContextInformation *GetNextValidParent()
  {
    if (ParentContext == NULL)
    {
      return NULL;
    }
    else if (ParentContext->IsValid)
    {
      return ParentContext;
    }
    else
    {
      return ParentContext->GetNextValidParent();
    }
  }
};

class ParallelQueryOptimizer::ParallelQueryOptimizerImpl
{
public: //methods
  ParallelQueryOptimizerImpl()
      : m_initialized(false),
        m_taskScheduler(tbb::task_scheduler_init::deferred)
  {
  }

  void InitializeIfNecessary()
  {
    if (m_initialized)
    {
      return;
    }

    //read settings from secondo-config
    long maxNumberOfConcurrentThreads =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "MaxNumberOfConcurrentThreads",
                                 1,
                                 std::string(getenv("SECONDO_CONFIG")));

    long maxBufferSizeInByte =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "MaxBufferSizeInByte",
                                 0,
                                 std::string(getenv("SECONDO_CONFIG")));

    long maxDegreeOfDataParallelism =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "MaxDegreeOfDataParallelism",
                                 1,
                                 std::string(getenv("SECONDO_CONFIG")));

    long maxNumberOfTuples =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "MaxNumberOfTuplesPerBlock",
                                 1000,
                                 std::string(getenv("SECONDO_CONFIG")));

    long queueCapacityThreshold =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "QueueCapacityThreshold",
                                 100,
                                 std::string(getenv("SECONDO_CONFIG")));

    long usePipelineParallelism =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "UsePipelineParallelism",
                                 1,
                                 std::string(getenv("SECONDO_CONFIG")));

    long loggerMode =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "LoggerMode", 0,
                                 std::string(getenv("SECONDO_CONFIG")));

    std::string debugOutputPath =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "DebugOutputPath", "",
                                 std::string(getenv("SECONDO_CONFIG")));

    long useOptimization =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "UseStaticParallelOptimization", 0,
                                 std::string(getenv("SECONDO_CONFIG")));

    std::string operatorsSupportingDataParallelismStr =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "OperatorsSupportingDataParallelism",
                                 "", std::string(getenv("SECONDO_CONFIG")));

    std::string operatorsRetainingOrderStr =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "OperatorsRetainingOrder", "",
                                 std::string(getenv("SECONDO_CONFIG")));

    std::string operatorsRequiringHashPartitioningStr =
        SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                 "OperatorsRequiringHashPartitioning", "",
                                 std::string(getenv("SECONDO_CONFIG")));

    if (maxNumberOfConcurrentThreads < 1 &&
        tbb::task_scheduler_init::default_num_threads() > 1)
    {
      //if the number of threads is not explicite configured
      //let tbb decide how many cores are used for parallel execution.
      m_settings.MaxNumberOfConcurrentThreads =
          tbb::task_scheduler_init::default_num_threads();
    }
    else if (maxNumberOfConcurrentThreads > 1)
    {
      m_settings.MaxNumberOfConcurrentThreads = maxNumberOfConcurrentThreads;
    }
    else
    {
      //at least two threads are needed for parallelized query processing.
      //Fall back to sequential query processing
      m_settings.MaxNumberOfConcurrentThreads = 1;
    }

    m_settings.QueueCapacityThreshold = std::max(queueCapacityThreshold, 1L);
    m_settings.MaxDegreeOfDataParallelism =
        std::max((int)maxDegreeOfDataParallelism, 1);
    m_settings.TotalBufferSizeInBytes = std::max(maxBufferSizeInByte, 0L);
    m_settings.MaxNumberOfTuplesPerBlock = std::max(maxNumberOfTuples, 1L);
    m_settings.UsePipelineParallelism = usePipelineParallelism == 1;
    m_settings.UseOptimization = useOptimization == 1;

    if (!debugOutputPath.empty() &&
        !fs::is_directory(debugOutputPath))
    {
      debugOutputPath = std::string();
    }
    ExecutionContextLogger::LoggerModes mode =
        (ExecutionContextLogger::LoggerModes)
            std::min((int)loggerMode,
                     (int)ExecutionContextLogger::LoggerModes::CompleteOutput);
    m_settings.Logger.reset(new ExecutionContextLogger(mode, debugOutputPath));

    m_settings.OperatorsSupportingDataParallelism =
        ParseSettingString(operatorsSupportingDataParallelismStr);
    m_settings.OperatorsRetainingOrder =
        ParseSettingString(operatorsRetainingOrderStr);
    m_settings.OperatorsRequiringHashPartitioning =
        ParseSettingString(operatorsRequiringHashPartitioningStr);

    //initialize a new scheduler to manage the number of threads
    //given as argument.
    if (m_settings.MaxNumberOfConcurrentThreads > 1)
    {
      m_taskScheduler.initialize(m_settings.MaxNumberOfConcurrentThreads);
      assert(m_taskScheduler.is_active());
    }

    m_initialized = true;
  };

  void ParallelizeQueryPlan(QueryProcessor *queryProcessor, void *queryPlan)
  {
    InitializeIfNecessary();

    m_queryProcessor = queryProcessor;
    m_settings.Logger->WriteDebugOutput("Start parallelizing query plan");

    //in case of incorrect settings or disabled concurrency fall back to
    //regular serial execution
    if (m_settings.MaxNumberOfConcurrentThreads < 2)
    {
      return;
    }

    OpTree rootNode = static_cast<OpTree>(queryPlan);
    std::vector<ContextInformation *> analyzedContexts;
    if (m_settings.UseOptimization)
    {
      //Inserts new par operators in the query plan, returns a list of
      //analyzed contexts
      OptimizeQueryPlan(rootNode, analyzedContexts);
    }
    else
    {
      //Checks the existing par operators inserted by the user for validity
      //and returns a list of analyzed contexts

      //create an initial context from root node to first par- operator before
      //analyzing the operator tree recursive depth first
      ContextInformation *beforeFirstParOpContext =
          new ContextInformation(NULL);
      analyzedContexts.push_back(beforeFirstParOpContext);
      AnalyzeQueryPlanRecursive(queryProcessor, rootNode, analyzedContexts,
                                beforeFirstParOpContext);
    }

    std::map<OpTree, ExecutionContext *> createdContexts;
    for (ContextInformation *contextInfo : analyzedContexts)
    {
      //leave ivalid par operators in the tree. Without an execution context the
      //operator does not use parallel execution.
      if (contextInfo->IsValid)
      {
        //adjust the first parameter of the par node
        //(degree of data parallelism)
        queryProcessor->ChangeParOperatorsNumberOfInstances(
            contextInfo->ParNode, contextInfo->MaxDegreeOfDataParallelism);

        //set local2 with new context object
        ExecutionContext *context = new ExecutionContext(queryProcessor,
                                                         contextInfo->ParNode,
                                                         m_settings);
        createdContexts[contextInfo->ParNode] = context;

        ParNodeInfo *local2 = new ParNodeInfo(context);
        ContextInformation *parent = contextInfo->GetNextValidParent();
        if (parent == NULL)
        {
          //the first par node close by the root has no managed entities,
          //create a single dummy entity to store a tuple reader
          ExecutionContextEntity *entity = new ExecutionContextEntity(
              0, (OpTree)queryPlan);
          local2->CurrentEntity(entity);
        }
        else
        {
          ExecutionContext *parentContext = createdContexts[parent->ParNode];
          parentContext->AddChildContexts(context);
          context->ParentContext(parentContext);
        }

        //Remove the connection to the sons of the par operator,
        //to avoid forwarding following commands to subtrees.
        assert(queryProcessor->GetNoSons(contextInfo->ParNode) > 0);
        queryProcessor->RemoveSon(contextInfo->ParNode, 0);

        queryProcessor->GetLocal2(contextInfo->ParNode).setAddr(local2);
      }
    }

    RedistributeMemory(analyzedContexts);

    for (ContextInformation *contextInfo : analyzedContexts)
    {
      delete contextInfo;
    }
    analyzedContexts.clear();

    m_settings.Logger->WriteDebugOutput("Finished parallelizing query plan");
    if (m_settings.Logger->DebugMode())
    {
      fs::path outputPath(m_settings.Logger->LoggerDebugPath());
      outputPath /= "partialTree_ContextEntity_0_0.gv";

      m_queryProcessor->OutputToDotFile(rootNode, outputPath.c_str());
    }
  };

  std::set<std::string> ParseSettingString(const std::string &setting,
                                           const char *separator = " ")
  {
    std::string trimmedSetting(setting);
    std::set<std::string> splittedSettings;
    boost::trim(trimmedSetting);
    boost::split(splittedSettings, trimmedSetting, boost::is_any_of(separator),
                 boost::token_compress_on);
    return splittedSettings;
  }

  ContextInformation *AnalyzeQueryPlanRecursive(
      QueryProcessor *queryProcessor,
      OpTree rootNode, std::vector<ContextInformation *> &analyzedContexts,
      ContextInformation *infoOfCurrentContext)
  {
    Operator *op = queryProcessor->GetOperator(rootNode);
    if (op == NULL)
    {
      //this happens when the node is no operator or a function object
      // in this case skip investigation of the subtree
      return infoOfCurrentContext;
    }

    std::string opName = op->GetName();
    bool isParOperator = opName == ParOperatorName;
    if (!isParOperator && op->UsesMemory())
    {
      infoOfCurrentContext->MemoryOperators.push_back(rootNode);
    }
    if (isParOperator)
    {
      ContextInformation *context = new ContextInformation(rootNode);
      context->MaxDegreeOfDataParallelism =
          queryProcessor->GetParOperatorsNumberOfInstances(rootNode);
      context->PartitonAttributeIndex =
          queryProcessor->GetParOperatorsPartitoningAttrIdx(rootNode);

      analyzedContexts.push_back(context);
      context->ParentContext = infoOfCurrentContext;
      infoOfCurrentContext = context;
    }
    else //regular operator
    {
      //analyze the operators of the context to setup the context with
      // correct parameters
      if (infoOfCurrentContext->ParNode == NULL ||
          (m_settings.OperatorsRetainingOrder.find(opName) !=
           m_settings.OperatorsRetainingOrder.end()))
      {
        //if operators are found which rely on an order of the tuple stream,
        //then invalidate all par-operators to the root  node
        ContextInformation *lastAnalyzedContext = infoOfCurrentContext;
        while (lastAnalyzedContext != NULL)
        {
          lastAnalyzedContext->IsValid = false;
          lastAnalyzedContext = lastAnalyzedContext->ParentContext;
        }
      }

      if (m_settings.OperatorsSupportingDataParallelism.find(opName) ==
          m_settings.OperatorsSupportingDataParallelism.end())
      {
        infoOfCurrentContext->MaxDegreeOfDataParallelism = 1;
      }

      if (m_settings.OperatorsRequiringHashPartitioning.find(opName) !=
              m_settings.OperatorsRequiringHashPartitioning.end() &&
          infoOfCurrentContext->MaxDegreeOfDataParallelism > 1)
      {
        infoOfCurrentContext->IsHashPartitioned = true;
      }
    }

    //for each son call analysis recursive
    int numSons = qp->GetNoSons(rootNode);
    for (int i = 0; i < numSons; i++)
    {
      OpNode *son = static_cast<OpNode *>(qp->GetSon(rootNode, i));

      ContextInformation *sonContext = AnalyzeQueryPlanRecursive(
          queryProcessor, son, analyzedContexts, infoOfCurrentContext);

      //check if the son context has a partition attribute in case of
      //partitioned contexts
      if (infoOfCurrentContext->IsHashPartitioned &&
          sonContext->ParNode != infoOfCurrentContext->ParNode &&
          sonContext->PartitonAttributeIndex < 0)
      {
        infoOfCurrentContext->MaxDegreeOfDataParallelism = 1;
      }
    }

    //return the contextInfo of the last reached par-operator
    return infoOfCurrentContext;
  }

  void OptimizeQueryPlan(OpTree rootNode,
                         std::vector<ContextInformation *> &analyzedContexts)
  {
    /* TODO: implement adding par-operator in operator tree */
  }

  void RedistributeMemory(std::vector<ContextInformation *> &analyzedContexts)
  {
    //TODO: reduce memory of the parallelized operators depending on the number
    //of instances no public query processor method available.
  }

private: //member
  bool m_initialized;
  QueryProcessor *m_queryProcessor;
  tbb::task_scheduler_init m_taskScheduler;
  ExecutionContextSetting m_settings;

}; // ParallelQueryOptimizerImpl

ParallelQueryOptimizer::ParallelQueryOptimizer()
    : m_pImpl(new ParallelQueryOptimizerImpl())
{
}

ParallelQueryOptimizer::~ParallelQueryOptimizer() = default;

void ParallelQueryOptimizer::ParallelizeQueryPlan(
    QueryProcessor *queryProcessor, void *queryPlan)
{
  return m_pImpl->ParallelizeQueryPlan(queryProcessor, queryPlan);
}

} // namespace parthread

#endif // USE_MULTIHREADED_QUERY_PROCESSING