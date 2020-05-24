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
    ContextInformation(OpNode *node)
        : InsertPosition(node), SonIndex(0), ParentContext(NULL),
          MaxDegreeOfDataParallelism(1), NumberOfOperators(0), IsValid(true),
          IsHashPartitioned(false), PartitonAttributeIndex(-1){};

    OpNode *InsertPosition;
    int SonIndex;
    ContextInformation *ParentContext;
    int MaxDegreeOfDataParallelism;
    int NumberOfOperators;
    bool IsValid;
    bool IsHashPartitioned;
    int PartitonAttributeIndex;
    std::vector<OpNode *> MemoryOperators;

    ContextInformation *MergeWithParent()
    {
      if (ParentContext == NULL)
      {
        return this;
      }

      ParentContext->NumberOfOperators += NumberOfOperators;
      ParentContext->MaxDegreeOfDataParallelism =
          std::min(MaxDegreeOfDataParallelism,
                   ParentContext->MaxDegreeOfDataParallelism);
      ParentContext->IsValid = ParentContext->IsValid && IsValid;
      ParentContext->IsHashPartitioned = ParentContext->IsHashPartitioned &&
                                         IsHashPartitioned;

      return ParentContext;
    }

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

    void WriteDebugOutput(const std::string message)
    {
      if (m_settings.Logger != NULL)
      {
        m_settings.Logger->WriteDebugOutput(message);
      }
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

      long totalBufferSizeInByte =
          SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                   "TotalBufferSizeInByte",
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

      long timeoutPerTask =
          SmiProfile::GetParameter("MultithreadedQueryProcessing",
                                   "TimeoutPerTaskInMicroseconds",
                                   0,
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
      m_settings.TotalBufferSizeInBytes = std::max(totalBufferSizeInByte, 0L);
      m_settings.MaxNumberOfTuplesPerBlock = std::max(maxNumberOfTuples, 1L);
      m_settings.TimeoutPerTaskInMicroSeconds =
          (clock_t)std::max(timeoutPerTask, 0L);
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
                       (int)ExecutionContextLogger::LoggerModes::FullOutput);

      m_settings.Logger.reset(
          new ExecutionContextLogger(mode, debugOutputPath));

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

        std::stringstream debugText;
        debugText << "Initialized task scheduler with "
                  << m_settings.MaxNumberOfConcurrentThreads << " threads";

        m_settings.Logger->WriteDebugOutput(debugText.str());
      }
      else
      {
        std::stringstream debugText;
        debugText << "Use serial execution";

        m_settings.Logger->WriteDebugOutput(debugText.str());
      }

      m_initialized = true;
    };

    void ParallelizeQueryPlan(QueryProcessor *queryProcessor, void *queryPlan,
                              size_t memorySpent, int noMemoryOperators)
    {
      InitializeIfNecessary();
      m_queryProcessor = queryProcessor;
      m_settings.Logger->ResetTimer();
      m_settings.Logger->WriteDebugOutput("Start parallelizing query plan");

      //in case of incorrect settings or disabled concurrency fall back to
      //regular serial execution
      if (m_settings.MaxNumberOfConcurrentThreads < 2)
      {
        return;
      }

      //create an initial context from root node to first par- operator before
      //analyzing the operator tree recursive depth first
      OpTree rootNode = static_cast<OpTree>(queryPlan);
      std::vector<ContextInformation *> analyzedContexts;
      ContextInformation *beforeFirstParOpContext =
          new ContextInformation(NULL);
      analyzedContexts.push_back(beforeFirstParOpContext);

      bool needsAnalysis = true;
      if (m_settings.UseOptimization)
      {
        //Inserts new par operators in the query plan, returns a list of
        //analyzed contexts

        //find first insert position of par-operator
        CreateInsertionPoints(
            queryProcessor, rootNode, 0, NULL,
            analyzedContexts, analyzedContexts[0]);

        if (analyzedContexts.size() > 1)
        {
          //first context contains nodes from root to first stream operator
          //no possibility for paralllelism, so set to invalid
          analyzedContexts[0]->IsValid = false;

          // create par operators for each context
          for (ContextInformation *contextInfo : analyzedContexts)
          {
            if (contextInfo->InsertPosition != NULL &&
                contextInfo->IsValid)
            {
              contextInfo->InsertPosition =
                  static_cast<OpNode *>(
                      m_queryProcessor->InsertParOperatorInTree(
                          contextInfo->InsertPosition, contextInfo->SonIndex,
                          contextInfo->MaxDegreeOfDataParallelism,
                          contextInfo->PartitonAttributeIndex));
            }
          }

          if (m_settings.Logger->DebugMode())
          {
            m_settings.Logger->WriteDebugOutput(
                "Finished parallelizing query plan");

            fs::path outputPath(m_settings.Logger->LoggerDebugPath());
            outputPath /= "parallelOptimizedTree.gv";

            m_queryProcessor->OutputToDotFile(rootNode, outputPath.c_str());
          }

          needsAnalysis = false;
        }
      }

      if (needsAnalysis)
      {
        //Checks the existing par operators inserted by the user for validity
        //and returns a list of analyzed contexts
        AnalyzeQueryPlanRecursive(queryProcessor, rootNode, analyzedContexts,
                                  beforeFirstParOpContext);
      }

      std::map<OpTree, ExecutionContext *> createdContexts;
      int contextId = 0;
      for (ContextInformation *contextInfo : analyzedContexts)
      {
        //leave ivalid par operators in the tree. Without an execution context
        //the operator does not use parallel execution.
        if (contextInfo->IsValid)
        {
          //set local2 with new context object
          contextId++;
          ExecutionContext *context =
              new ExecutionContext(contextId,
                                   queryProcessor, contextInfo->InsertPosition,
                                   m_settings,
                                   contextInfo->MaxDegreeOfDataParallelism,
                                   contextInfo->PartitonAttributeIndex);
          createdContexts[contextInfo->InsertPosition] = context;

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
            ExecutionContext *parentContext =
                createdContexts[parent->InsertPosition];
            parentContext->AddChildContexts(context);
            context->ParentContext(parentContext);
          }

          //Remove the connection to the sons of the par operator,
          //to avoid forwarding following commands to subtrees.
          assert(queryProcessor->GetNoSons(contextInfo->InsertPosition) > 0);
          queryProcessor->RemoveSon(contextInfo->InsertPosition, 0);

          queryProcessor->GetLocal2(contextInfo->InsertPosition)
              .setAddr(local2);

          std::stringstream debugText;
          debugText << "Created Context " << context->ContextId()
                    << " with max. entities "
                    << contextInfo->MaxDegreeOfDataParallelism;
          if (contextInfo->IsHashPartitioned)
          {
            debugText << " and hash partitioned";
          }

          m_settings.Logger->WriteDebugOutput(debugText.str());
        }
      }

      RedistributeMemory(analyzedContexts, contextId, memorySpent,
                         noMemoryOperators);

      DeleteContextInformation(analyzedContexts);
      analyzedContexts.clear();

      if (m_settings.Logger->DebugMode())
      {
        m_settings.Logger->WriteDebugOutput(
            "Finished parallelizing query plan");

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
      boost::split(splittedSettings, trimmedSetting,
                   boost::is_any_of(separator),
                   boost::token_compress_on);
      return splittedSettings;
    }

    ContextInformation *AnalyzeQueryPlanRecursive(
        QueryProcessor *queryProcessor,
        OpTree rootNode, std::vector<ContextInformation *> &analyzedContexts,
        ContextInformation *infCurrentContext)
    {
      Operator *op = queryProcessor->GetOperator(rootNode);
      if (op == NULL)
      {
        //this happens when the node is no operator or a function object
        // in this case skip investigation of the subtree
        return infCurrentContext;
      }

      std::string opName = op->GetName();
      bool isParOperator = opName == ParOperatorName;
      if (!isParOperator && op->UsesMemory())
      {
        infCurrentContext->MemoryOperators.push_back(rootNode);
      }
      if (isParOperator)
      {
        ContextInformation *context = new ContextInformation(rootNode);
        context->MaxDegreeOfDataParallelism =
            queryProcessor->GetParOperatorsNumberOfInstances(rootNode);
        context->PartitonAttributeIndex =
            queryProcessor->GetParOperatorsPartitoningAttrIdx(rootNode);

        analyzedContexts.push_back(context);
        context->ParentContext = infCurrentContext;
        infCurrentContext = context;
      }
      else //regular operator
      {
        //analyze the operators of the context to setup the context with
        // correct parameters
        if (infCurrentContext->InsertPosition == NULL ||
            OperatorRetainsOrder(opName))
        {
          //if operators are found which rely on an order of the tuple stream,
          //then invalidate all par-operators to the root  node
          ContextInformation *lastAnalyzedContext = infCurrentContext;
          while (lastAnalyzedContext != NULL)
          {
            lastAnalyzedContext->IsValid = false;
            lastAnalyzedContext = lastAnalyzedContext->ParentContext;
          }
        }

        if (!OperatorSupportsDataparallelism(opName))
        {
          infCurrentContext->MaxDegreeOfDataParallelism = 1;
        }
        else
        {
          infCurrentContext->MaxDegreeOfDataParallelism =
              std::min((int)m_settings.MaxDegreeOfDataParallelism,
                       infCurrentContext->MaxDegreeOfDataParallelism);
        }

        if (OperatorRequiresHashPartitioning(opName) &&
            infCurrentContext->MaxDegreeOfDataParallelism > 1)
        {
          infCurrentContext->IsHashPartitioned = true;
        }
      }

      //for each son call analysis recursive
      int numSons = qp->GetNoSons(rootNode);
      for (int i = 0; i < numSons; i++)
      {
        OpNode *son = static_cast<OpNode *>(qp->GetSon(rootNode, i));

        ContextInformation *sonContext = AnalyzeQueryPlanRecursive(
            queryProcessor, son, analyzedContexts, infCurrentContext);

        //check if the son context has a partition attribute in case of
        //partitioned contexts
        if (infCurrentContext->IsHashPartitioned &&
            sonContext->InsertPosition != infCurrentContext->InsertPosition &&
            sonContext->PartitonAttributeIndex < 0)
        {
          infCurrentContext->MaxDegreeOfDataParallelism = 1;
        }
      }

      //return the contextInfo of the last reached par-operator
      return infCurrentContext;
    }

    ContextInformation *CreateInsertionPoints(
        QueryProcessor *queryProcessor,
        OpTree currentNode, int sonIndex, OpTree previousNode,
        std::vector<ContextInformation *> &analyzedContexts,
        ContextInformation *infoOfCurrentContext)
    {
      bool insertionPoint = false;
      Operator *op = queryProcessor->GetOperator(currentNode);
      if (op == NULL)
      {
        //this happens when the node is no operator or a function object
        // in this case skip investigation of the subtree
        return infoOfCurrentContext;
      }

      std::string opName = op->GetName();
      if (opName == ParOperatorName)
      {
        //cancel the optimization if the user inserted par-operators
        DeleteContextInformation(analyzedContexts);
        ContextInformation *beforeFirstParOpContext =
            new ContextInformation(NULL);
        analyzedContexts.push_back(beforeFirstParOpContext);
        return NULL;
      }

      if (OperatorRetainsOrder(opName))
      {
        //found an operator relying on order of the datasets
        //in this case remove all other insertion points before
        ContextInformation *lastAnalyzedContext = infoOfCurrentContext;
        lastAnalyzedContext->MaxDegreeOfDataParallelism = 1;
        lastAnalyzedContext->IsHashPartitioned = false;
        while (lastAnalyzedContext != NULL)
        {
          lastAnalyzedContext->IsValid = false;
          lastAnalyzedContext = lastAnalyzedContext->MergeWithParent();
        }
      }
      else if (analyzedContexts.size() == 1)
      {
        //find the first insertion point after the root node
        insertionPoint = queryProcessor->IsTupleStreamOperator(currentNode);
      }
      else
      {
        if (queryProcessor->IsTupleStreamOperator(currentNode))
        {
          //in case of the first operator set the context information
          //otherwise check if another insertion point is necessary

          //if the degree of dataparallelism changes, add another
          //insertion point
          bool contextSupportsDataparallelism =
              infoOfCurrentContext->MaxDegreeOfDataParallelism > 1;
          insertionPoint = contextSupportsDataparallelism ^
                           OperatorSupportsDataparallelism(opName);
        }
        else if (false)
        {
          //operator switch between Stream and Relation
        }
        else
        {
          //other type of operator, stop analysis of sub trees
          return infoOfCurrentContext;
        }
      }

      if (insertionPoint)
      {
        ContextInformation *ctx = new ContextInformation(previousNode);
        ctx->SonIndex = sonIndex;
        ctx->ParentContext = infoOfCurrentContext;
        ctx->MaxDegreeOfDataParallelism = 1;
        if (OperatorSupportsDataparallelism(opName))
        {
          ctx->MaxDegreeOfDataParallelism =
              m_settings.MaxDegreeOfDataParallelism;
        }
        analyzedContexts.push_back(ctx);
        infoOfCurrentContext = ctx;
      }

      if (op->UsesMemory())
      {
        infoOfCurrentContext->MemoryOperators.push_back(currentNode);
      }
      infoOfCurrentContext->NumberOfOperators++;

      //for each son call analysis recursive
      int numSons = qp->GetNoSons(currentNode);
      for (int i = 0; i < numSons; i++)
      {
        OpNode *son = static_cast<OpNode *>(qp->GetSon(currentNode, i));

        //ContextInformation *sonContext =
        CreateInsertionPoints(
            queryProcessor, son, i, currentNode, analyzedContexts,
            infoOfCurrentContext);
      }

      //return the contextInfo of the last reached par-operator
      return infoOfCurrentContext;
    }

    bool OperatorSupportsDataparallelism(const std::string &opName)
    {
      return m_settings.OperatorsSupportingDataParallelism.find(opName) !=
             m_settings.OperatorsSupportingDataParallelism.end();
    }

    bool OperatorRetainsOrder(const std::string &opName)
    {
      return m_settings.OperatorsRetainingOrder.find(opName) !=
             m_settings.OperatorsRetainingOrder.end();
    }

    bool OperatorRequiresHashPartitioning(const std::string &opName)
    {
      return m_settings.OperatorsRequiringHashPartitioning.find(opName) !=
             m_settings.OperatorsRequiringHashPartitioning.end();
    }

    void DeleteContextInformation(
        std::vector<ContextInformation *> &analyzedContexts)
    {
      for (ContextInformation *contextInfo : analyzedContexts)
      {
        delete contextInfo;
      }
      analyzedContexts.clear();
    }

    void OptimizeQueryPlanRecursive(
        QueryProcessor *queryProcessor,
        OpTree rootNode, std::vector<ContextInformation *> &analyzedContexts,
        ContextInformation *infoOfCurrentContext)
    {
      /* TODO: implement adding par-operator in operator tree */
    }

    void RedistributeMemory(std::vector<ContextInformation *> analyzedContexts,
                            int numberOfValidContexts,
                            size_t memorySpent, int numberMemoryOperators)
    {
      if (numberOfValidContexts == 0)
      {
        return;
      }

      size_t globalMemory = m_queryProcessor->GetGlobalMemory();

      size_t memoryDistributionFactor =
          numberMemoryOperators > 0 ? numberMemoryOperators : 1;
      size_t prevMemoryPerOperator = (globalMemory - memorySpent) /
                                     memoryDistributionFactor;

      //calculate the distributed memory considering the numberOfValidContexts
      size_t memoryPerOperator = (globalMemory - memorySpent) /
                                 (numberMemoryOperators +
                                  numberOfValidContexts);

      size_t memPerTupleBuffer = m_settings.TotalBufferSizeInBytes /
                                 numberOfValidContexts;

      if (m_settings.TotalBufferSizeInBytes > 0 &&
          memoryPerOperator > memPerTupleBuffer)
      {
        //if a total buffer size is given and the value is less than the
        //distributed memory, then take the total size ...
        memorySpent += m_settings.TotalBufferSizeInBytes;
        memoryPerOperator = (globalMemory - memorySpent) /
                            memoryDistributionFactor;
      }
      else
      {
        //... otherwise distribute the same amount of memory to the par-Operator
        // as for all other memory operators.
        memPerTupleBuffer = memoryPerOperator;
      }

      for (ContextInformation *contextInfo : analyzedContexts)
      {
        if (!contextInfo->IsValid)
        {
          continue;
        }

        //set the memory of the par-operators for the contained tuple buffers
        m_queryProcessor->SetMemorySize(contextInfo->InsertPosition,
                                        memPerTupleBuffer);

        //reduce memory of the parallelized operators depending on the number
        //of instances
        for (OpNode *memOp : contextInfo->MemoryOperators)
        {
          size_t mem = m_queryProcessor->GetMemorySize(memOp);

          size_t memoryPerOpAndInstance =
              memoryPerOperator / contextInfo->MaxDegreeOfDataParallelism;

          if (prevMemoryPerOperator != mem)
          {
            //the operators memory was probably not distributed, so just
            //divide the original memory size through the number of instances
            memoryPerOpAndInstance = mem /
                                     contextInfo->MaxDegreeOfDataParallelism;
          }

          m_queryProcessor->SetMemorySize(memOp, memoryPerOpAndInstance);
        }
      }

      std::stringstream debugText;
      debugText << "Redistributed memory for " << numberOfValidContexts
                << " contexts with " << memPerTupleBuffer << "MB each and "
                << numberMemoryOperators << " memory operators with "
                << memoryPerOperator << "MB distributed over the instances "
                << "of the operators execution context";

      m_settings.Logger->WriteDebugOutput(debugText.str());
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

  void ParallelQueryOptimizer::WriteDebugOutput(const std::string message)
  {
    m_pImpl->WriteDebugOutput(message);
  }

  void ParallelQueryOptimizer::ParallelizeQueryPlan(
      QueryProcessor *queryProcessor, void *queryPlan,
      size_t memorySpent, int noMemoryOperators)
  {
    return m_pImpl->ParallelizeQueryPlan(queryProcessor, queryPlan, memorySpent,
                                         noMemoryOperators);
  }

} // namespace parthread

#endif // USE_MULTIHREADED_QUERY_PROCESSING