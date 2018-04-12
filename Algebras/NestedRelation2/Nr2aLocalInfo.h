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

#ifndef ALGEBRAS_NESTEDRELATION2_NR2ALOCALINFO_H_
#define ALGEBRAS_NESTEDRELATION2_NR2ALOCALINFO_H_

#include "Progress.h"

#include "Nr2aHelper.h"

namespace nr2a {

template <class T>
class Nr2aLocalInfo
{
  public:
    Nr2aLocalInfo();
    virtual ~Nr2aLocalInfo();
    virtual void setCostEstimator(T* costEstimator);
    virtual T* getCostEstimator() const;

    virtual void UnitReceived();
    virtual void UnitProcessed();

    ProgressInfo base;

    uint64_t m_unitsProcessed;
    double m_totalTimeProcessed;
    clock_t m_firstClock;
    clock_t m_lastClock;

  protected:
    virtual void SetUnitsProcessed(const uint64_t unitsProcessed);

  private:
    T* m_costEstimator;
};

template <class T>
Nr2aLocalInfo<T>::Nr2aLocalInfo()
  :m_unitsProcessed(0), m_totalTimeProcessed(0),
   m_firstClock(0), m_lastClock(0), m_costEstimator(NULL)
{

}

template <class T>
Nr2aLocalInfo<T>::~Nr2aLocalInfo()
{
    //intentionally left blank
}

template <class T>
/*virtual*/ void
Nr2aLocalInfo<T>::setCostEstimator
  (T* costEstimator)
{
  m_costEstimator = costEstimator;
}

template <class T>
/*virtual*/ T*
Nr2aLocalInfo<T>::getCostEstimator() const
{
  return m_costEstimator;
}

template <class T>
void Nr2aLocalInfo<T>::UnitReceived()
{
  m_lastClock = clock();
  if (m_firstClock == 0)
  {
    m_firstClock = m_lastClock;
  }
}

template <class T>
void Nr2aLocalInfo<T>::UnitProcessed()
{
//  m_totalTimeProcessed +=
//      Nr2aHelper::MillisecondsElapsedSince(m_lastClock);
  m_totalTimeProcessed = Nr2aHelper::MillisecondsElapsedSince(m_firstClock);
  m_unitsProcessed++;
  m_lastClock = 0;
}

template <class T>
void Nr2aLocalInfo<T>::SetUnitsProcessed(const uint64_t unitsProcessed)
{
  m_totalTimeProcessed = Nr2aHelper::MillisecondsElapsedSince(m_firstClock);
  assert(unitsProcessed >= m_unitsProcessed);
  m_unitsProcessed = unitsProcessed;
  m_lastClock = 0;
}

} /* namespace nr2a */

#endif /* ALGEBRAS_NESTEDRELATION2_NR2ALOCALINFO_H_ */
