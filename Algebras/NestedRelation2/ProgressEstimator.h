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

#ifndef ALGEBRAS_NESTEDRELATION2_PROGRESSESTIMATOR_H_
#define ALGEBRAS_NESTEDRELATION2_PROGRESSESTIMATOR_H_

#include "AlgebraTypes.h"

#include <time.h>
#include "Progress.h"
#include "CostEstimation.h"


namespace nr2a {

/*
1 Progress estimation

"ProgressEstimator"[2] is the base class for all progress estimation done in
this algebra. the template parameter takes the class of the operators local
info, which will then be accessible in derived progress estimators.

*/
template <class T>
class ProgressEstimator : public CostEstimation
{
  public:
    virtual ~ProgressEstimator();

    // function is forced to exist by CostEstimation, but is unused
    virtual void init(Word* args, void* localInfo);

  protected:
    ProgressEstimator();
    static void FixProgress(ProgressInfo &info);

  private:
    T* m_localInfo;
};

template <class T>
ProgressEstimator<T>::ProgressEstimator()
{
  //Intentionally left blank
}

template <class T>
ProgressEstimator<T>::~ProgressEstimator()
{
  //Intentionally left blank
}

template <class T>
/*virtual*/ void ProgressEstimator<T>::init(Word* args, void* localInfo)
{
    m_localInfo = (T*)localInfo;
}

/*
For some operators yield invalid progress information objects, those are fixed
here. If they would not be fixed, they might compromise the algorithms of the
NestedRelation2 algebra, resulting in false negatives, when checking with
"assert(progressInfo->checkRanges())"[2].

*/
template <class T>
/*static*/ void ProgressEstimator<T>::FixProgress(ProgressInfo &info)
{
    if (info.Progress > 1)
    {
      info.Progress = 1;
    }
}

} /* namespace nr2a */

#endif /* ALGEBRAS_NESTEDRELATION2_PROGRESSESTIMATOR_H_ */
