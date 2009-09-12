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

1 Implementation File TupleQueue.cpp

May 2009, Sven Jungnickel. Initial version.

2 Includes

*/

#include "TupleQueue.h"

/*
3 Implementation of class ~TupleAndRelPos~

*/

namespace extrel2
{

size_t TupleAndRelPos::createCounter = 0;
size_t TupleAndRelPos::copyCounter = 0;
size_t TupleAndRelPos::assignCounter = 0;
/*
Initialization of static class counters.

*/

/*
4 Implementation of class ~TupleCompare~

*/

size_t TupleCompare::comparisonCounter = 0;
/*
Initialization of comparison counter.

*/

/*
5 Implementation of class ~TupleAndRelPosQueue~

*/

TupleAndRelPosQueue::TupleAndRelPosQueue(TupleCompareBy* cmp)
: Queue<TupleAndRelPosCompareDesc, TupleAndRelPos>(cmp)
, totalByteSize(0)
{
}

void TupleAndRelPosQueue::Push(TupleAndRelPos& t)
{
  t.tuple()->IncReference();
  pQueue->push(t);
  totalByteSize += t.tuple()->GetSize();
}

void TupleAndRelPosQueue::Pop()
{
  Tuple* t = pQueue->top().tuple();
  totalByteSize -= t->GetSize();
  t->DeleteIfAllowed();
  pQueue->pop();
}

} // end of namespace extrel2
