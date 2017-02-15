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

#ifndef ALGEBRAS_NESTEDRELATION2_BLOCKINGPROGRESSESTIMATOR_H_
#define ALGEBRAS_NESTEDRELATION2_BLOCKINGPROGRESSESTIMATOR_H_

#include "AlgebraTypes.h"
#include "Algebra.h"

#include "ProgressEstimator.h"
#include "Nr2aLocalInfo.h"

namespace nr2a {

/*
The "BlockingProgressEstimator"[2] can be used for Operators which do not work
in a pipeline, but output their results when enarly finished (blocking
operators).

*/
class BlockingProgressEstimator :
    public ProgressEstimator<Nr2aLocalInfo<BlockingProgressEstimator> >
{
  public:
    BlockingProgressEstimator(const bool waitForFirstOnly = false);
    BlockingProgressEstimator(Word predecessor,
        const bool waitForFirstOnly = false);
    virtual ~BlockingProgressEstimator();

    virtual int requestProgress(Word* args, ProgressInfo* result,
        void* localInfo, const bool argsAvailable);

    static CostEstimation * Build();
    static CostEstimation * BuildWaitForFirstOnly();

  private:
    void * m_predecessor;
    bool m_waitForFirstOnly;
};

} /* namespace nr2a */

#endif /* ALGEBRAS_NESTEDRELATION2_BLOCKINGPROGRESSESTIMATOR_H_ */
