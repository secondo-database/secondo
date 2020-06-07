/*
----
This file is part of SECONDO.

Copyright (C) 2019,
Faculty of Mathematics and Computer Science,
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

*/

#include "ParOperator.h"
#include "ExecutionContext/ExecutionContext.h"

#include "Algebra.h"
#include "StandardTypes.h"
#include "Stream.h"

#include "ParNodeInfo.h"

using namespace std;

namespace parThread
{

ListExpr ParOperator::ParTM(ListExpr args)
{
  NList type(args);
  // args[0] : tuple-stream
  // args[2] : integer, number of parallel instances
  // args[3] : [optional] attribute, name of the attribute used for partitioning

  Cardinal numberOfParameters = type.length();

  if (numberOfParameters < 2 ||
      numberOfParameters > 3)
  {
    ErrorReporter::ReportError("two or three arguments expected");
    return nl->TypeError();
  }

  NList inputStream = type.first();
  NList numOfParallelInstances = type.second();

  NList attrInputStream;
  if (!inputStream.checkStreamTuple(attrInputStream))
  {
    ErrorReporter::ReportError("first argument must be a stream of tuples");
    return nl->TypeError();
  }

  // check if there is a valid tuple description
  if (!IsTupleDescription(attrInputStream.listExpr()))
  {
    ErrorReporter::ReportError("first argument must contain \
                                a tuple description");
    return nl->TypeError();
  }

  if (numOfParallelInstances.str() != CcInt::BasicType())
  {
    ErrorReporter::ReportError("second argument must be of type int");
    return nl->TypeError();
  }
  
  int attrIndex = -1;
  if (numberOfParameters == 3)
  {
    //third parameter is attribute from the tuple stream used for partitioning
    NList partitionAttribute = type.third();
    ListExpr attrType;

    attrIndex = FindAttribute(attrInputStream.listExpr(), 
                              partitionAttribute.str(), attrType);

    if (attrIndex < 1)
    {
      ErrorReporter::ReportError("the given attribute for partitioning is not \
                                  contained in the streams tuple description");
      return nl->TypeError();
    }
  }

  return nl->ThreeElemList( nl->SymbolAtom(Symbol::APPEND()),
                            nl->OneElemList(nl->IntAtom(attrIndex)),
                                            nl->First(args));
}

int ParOperator::ParVM(Word *args, Word &result, int message,
                           Word &local, Supplier s)
{
  if (IsOptimizedForParallelQueryExecution(s))
  {
    return ParVMParallel(args, result, message, local, s);
  }

  return ParVMSerial(args, result, message, local, s);
}

bool ParOperator::IsOptimizedForParallelQueryExecution(Supplier s)
{
  return qp->GetLocal2(s).addr != NULL;
}

int ParOperator::ParVMParallel(Word *args, Word &result, int message,
                                   Word &local, Supplier s)
{
  parthread::ParNodeInfo *nodeInfo 
  = static_cast<parthread::ParNodeInfo *>(qp->GetLocal2(s).addr);

  if (nodeInfo == NULL)
  {
    ErrorReporter::ReportError("No valid parallel query optimizer information \
                                deposited");
    return -1;
  }
  parthread::ExecutionContext *context = nodeInfo->ConnectedContext();

  switch (message)
  {
  case INIT:
  {
    //All operators from the first par- operator to the leaves of the operator 
    //tree must be initialized through the execution contexts entity manager.
    //This step is not parallelized.
    if (context->ParentContext() == NULL)
    {
      context->Init();
    }
    return 0;
  }
  case OPEN:
  {
    context->Open(nodeInfo);

    return 0;
  }
  case REQUEST:
  {
    Tuple *tuple = NULL;
    int status = context->Request(nodeInfo, tuple);
    if (status == YIELD)
    {
      result.setAddr(tuple);

      //decrement reference increased by the tuple buffer
      if (context->ParentContext() == NULL)
      {
        tuple->DeleteIfAllowed();
      }

      assert(tuple->GetNumOfRefs() > 0);
    }
    else
    {
      tuple = NULL;
    }

    return status;
  }
  case CLOSE:
  {
    context->Close(nodeInfo);

    return 0;
  }
  case FINISH:
  {
    context->Finish();
    if (context->ParentContext() == NULL)
    {
      //trigger deletion of the context tree 
      //with the first context after the operator tree root
      delete context;
      context = NULL;

      //the first par node close to the root has no managed entity
      //so its necessary to delete it here
      delete nodeInfo->CurrentEntity();
      nodeInfo->CurrentEntity(NULL);
    }
    
    delete nodeInfo;
    qp->GetLocal2(s).addr = NULL;

    return 0;
  }
  }

  return -1;
}

int ParOperator::ParVMSerial(Word *args, Word &result, int message,
                                 Word &local, Supplier s)
{

  switch (message)
  {
  case INIT:
    return 0;
  case OPEN:
    qp->Open(args[0].addr);
    return 0;
  case REQUEST:
    qp->Request(args[0].addr, result);
    return qp->Received(args[0].addr) ? YIELD : CANCEL;
  case CLOSE:
    return 0;
  case FINISH:
    return 0;
  }

  return -1;
}

} // end namespace parThread
