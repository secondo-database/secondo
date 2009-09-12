/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Faculty of Mathematics and Computer Science,
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

1 Implementation File SortedRun.cpp

May 2009, Sven Jungnickel. Initial version.

2 Includes

*/

#include "SortedRun.h"

using namespace extrel2;

ostream& extrel2::operator<<(ostream& os, SortedRun& run)
{
  Tuple* t;

  os << "-------------------- Run "
     << run.GetRunNumber()
     << " -----------------" << endl;

  while ( ( t = run.GetNextTuple() ) != 0 )
  {
    os << *t << endl;
  }

  return os;
}

/*
3 Implementation of class ~SortedRun~

*/
extrel2::SortedRun::SortedRun(int runNumber, TupleCompareBy* cmp)
: runNumber(runNumber)
, cmp(cmp)
, runLength(0)
, tupleCount(0)
, minTupleSize(UINT_MAX)
, maxTupleSize(0)
, buffer(0)
, iter(0)
, traceMode(RTFlag::isActive("ERA:TraceSort"))
{
  heap = new TupleQueue(cmp);

  if( traceMode )
  {
    info = new SortedRunInfo(runNumber);
  }
}

extrel2::SortedRun::~SortedRun()
{
  if ( iter != 0 )
  {
    delete iter;
    iter = 0;
  }

  if ( traceMode )
  {
    // delete info;
    // Delete is handled in SortInfo Destructor
    info = 0;
  }

  while ( !heap->Empty() )
  {
    heap->Pop();
  }
  delete heap;
  heap = 0;

  lastTuple.setTuple(0);

  cmp = 0;
}

