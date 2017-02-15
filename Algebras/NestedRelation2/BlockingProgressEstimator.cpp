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

*/

#include "BlockingProgressEstimator.h"

using namespace nr2a;

/*
Constructors of "BlockingProgressEstimator"[2]. The first can be used if there
is no preceeding operator. In this case the "baseInfo"[2] has to be set, before
the estimator can yield results.

*/
nr2a::BlockingProgressEstimator::BlockingProgressEstimator
    (const bool waitForFirstOnly /*= false*/)
    : m_predecessor(NULL), m_waitForFirstOnly(waitForFirstOnly)
{
  //intentionally left blank
}

nr2a::BlockingProgressEstimator::BlockingProgressEstimator
    (Word predecessor, const bool waitForFirstOnly /*= false*/)
    : m_predecessor(predecessor.addr), m_waitForFirstOnly(waitForFirstOnly)
{
  //intentionally left blank
}

nr2a::BlockingProgressEstimator::~BlockingProgressEstimator()
{
  //intentionally left blank
}

/*
The estimator uses the info of the preceeding operator, if possible. Otherwise
the "baseInfo"[2] will be used. In case of an object being its operators
parameter, the estimator can distinguish between operators consuming only
the first tuple. This is used with the "extract"[2] operator for example.

*/
/*virtual*/ int nr2a::BlockingProgressEstimator::requestProgress
(Word* args, ProgressInfo* result, void* localInfo,
    const bool argsAvailable)
{
  Nr2aLocalInfo<BlockingProgressEstimator>* info =
      (Nr2aLocalInfo<BlockingProgressEstimator>*)localInfo;
  int resultMessage = CANCEL;
  ProgressInfo predecessorsProgress;
  Supplier son = qp->GetSupplierSon(CostEstimation::supplier, 0);
  if (qp->IsObjectNode(son))
  {
    // If the operator's son is an object, it is assumed that this operator
    // is waiting for the object's creation and therefore totally blocked
    if (info != NULL)
    {
      result->Copy(info->base);
      if (m_waitForFirstOnly)
      {
        result->BTime = info->base.BTime;
        if (info->base.Card > 0)
        {
          result->BTime += (info->base.Time / info->base.Card);
          result->BProgress =
              (info->base.BProgress * info->base.BTime
                  + info->base.Progress * info->base.Time)
              / (info->base.BTime + info->base.Time);
          result->Time = 0;
          result->Progress = 0;
          resultMessage = YIELD;
        }
        else
        {
          // There is no way to tell, what progress to assume if the
          // cardinality is missing.
          resultMessage = CANCEL;
        }
      }
      else
      {
        result->BTime = info->base.BTime + info->base.Time;
        result->BProgress =
            (info->base.BProgress * info->base.BTime
                + info->base.Progress * info->base.Time)
            / (info->base.BTime + info->base.Time);
        result->Time = 0;
        result->Progress = 0;
        resultMessage = YIELD;
      }
    }
  }
  else
  {
    // If the operator's son is not an object, this operator itself is assumed
    // to take a negligible amount of time, so the son's estimation is adopted
    ProgressInfo progressInfo;
    if (qp->RequestProgress(son, &progressInfo) )
    {
      result->Copy(progressInfo);
      resultMessage = YIELD;
    }
  }
  return resultMessage;
}

/*
Functions, that can be used when defining operators.

*/
/*static*/ CostEstimation *
nr2a::BlockingProgressEstimator::Build()
{
  return new BlockingProgressEstimator();
}

/*static*/ CostEstimation *
nr2a::BlockingProgressEstimator::BuildWaitForFirstOnly()
{
  return new BlockingProgressEstimator(true);
}
