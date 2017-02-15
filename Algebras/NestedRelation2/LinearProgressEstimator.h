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

#ifndef ALGEBRAS_NESTEDRELATION2_LINEARPROGRESSESTIMATOR_H_
#define ALGEBRAS_NESTEDRELATION2_LINEARPROGRESSESTIMATOR_H_

#include <time.h>

#include "ProgressEstimator.h"
#include "Nr2aLocalInfo.h"

namespace nr2a {

/*
A "LinearProgressEstimator"[2] can be used in conjunction with operators which
have a linear progress. This is also true for all operators which are
suitable for pipelining.

*/
template <class T>
class LinearProgressEstimator : public ProgressEstimator<T>
{
  public:
    LinearProgressEstimator();
    LinearProgressEstimator(void * predecessor);
    virtual ~LinearProgressEstimator();

    virtual int requestProgress(Word* args, ProgressInfo* result,
        void* localInfo, const bool argsAvailable);

    static CostEstimation * Build();

  private:
    void * m_predecessor;
    ProgressInfo m_base;
};

template <class T>
nr2a::LinearProgressEstimator<T>::LinearProgressEstimator()
: ProgressEstimator<T>(), m_predecessor(NULL)
{
  //intentionally left blank
}

template <class T>
nr2a::LinearProgressEstimator<T>::LinearProgressEstimator
(void * predecessor)
: ProgressEstimator<T>(), m_predecessor(predecessor)
{

}

template <class T>
nr2a::LinearProgressEstimator<T>::~LinearProgressEstimator()
{
  //~ProgressEstimator<T>();
}

/*
To determine the progress the operator simply measures the time spent on the
tuples processed so far and calculates the estimated time needed to processs
all tuples.

*/
template <class T>
/*virtual*/ int nr2a::LinearProgressEstimator<T>::requestProgress
(Word* args, ProgressInfo* result, void* localInfo,
    const bool argsAvailable)
{
  T* info = (T*)localInfo;
  int resultMessage = 0;
  if (info != NULL)
  {
    info->setCostEstimator(this);
  }
  Supplier son = qp->GetSupplierSon(CostEstimation::supplier, 0);
  if (qp->IsObjectNode(son))
  {
    if (info != NULL)
    {
      m_base.Copy(info->base);
    }
  }
  else
  {
    ProgressInfo progressInfo;
    if (qp->RequestProgress(son, &progressInfo) )
    {
      m_base.Copy(progressInfo);
    }
    else
    {
      resultMessage = CANCEL;
    }
  }
  // feed of RelC++ yields progress beyond 1. See comment on FixProgress
  ProgressEstimator<T>::FixProgress(m_base);
  if ((resultMessage != CANCEL) && (info != NULL) && (m_base.Card > 0)
      && (info->m_unitsProcessed>0))
  {
    result->Copy(m_base);
    result->Time +=
        ((double)info->m_totalTimeProcessed * m_base.Card
        / info->m_unitsProcessed);
    result->Progress = (double)info->m_unitsProcessed / m_base.Card;
    ProgressEstimator<T>::FixProgress(*result);
    //assert(result->checkRanges());
    resultMessage = YIELD;
  }
  else
  {
    // If no tuple is processed yet the estimation will be discretionary or
    // has to be done for each operator separately analysing their behaviour
    // first, resulting in an estimation not working well on significantly
    // different hardware.
    resultMessage = CANCEL;
  }
  return resultMessage;
}

/*
Use this function, when defining an operator.

*/
template <class T>
/*static*/ CostEstimation *
nr2a::LinearProgressEstimator<T>::Build()
{
  return new LinearProgressEstimator<T>();
}

} /* namespace nr2a */

#endif /* ALGEBRAS_NESTEDRELATION2_LINEARPROGRESSESTIMATOR_H_ */
