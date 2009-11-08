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

November 2009, Sven Jungnickel. Added implementation for automatic
detection of lexicographical sort order (asc/desc) in method
~analyseSpec~ of class ~TupleQueueCompare~

2 Includes

*/

#include "TupleQueue.h"

/*
3 Implementation of class ~TupleQueueEntry~

*/

namespace extrel2
{

size_t TupleQueueEntry::createCounter = 0;
size_t TupleQueueEntry::copyCounter = 0;
size_t TupleQueueEntry::assignCounter = 0;
/*
Initialization of static class counters.

*/

/*
4 Implementation of class ~TupleQueueCompare~

*/

size_t TupleQueueCompare::comparisonCounter = 0;

/*
Initialization of comparison counter.

*/

bool TupleQueueCompare::traceMode = false;
/*
Initialization of tracing flag.

*/

int TupleQueueCompare::analyseSpec(const SortOrderSpecification& spec)
{
  assert(spec.size() > 0);

  if ( attributes != (int)spec.size() )
  {
    if ( traceMode )
    {
      cmsg.info() << "spec (length not equal)" << endl
                  << "attributes: " << attributes << endl
                  << "spec.size(): " << spec.size() << endl;
      cmsg.send();
    }

    return 0;
  }
  else
  {
    int flags = 0;

    for (size_t i = 0; i < spec.size(); i++)
    {
      // test attribute indices
      if ( (int)i != spec[i].first-1 )
      {
        if ( traceMode )
        {
          cmsg.info() << "spec (indices mixed)" << endl;
          cmsg.send();
        }

        return 0;
      }

      flags += (spec[i].second << i);
    }

    // test if all order specifiers are false
    if ( flags == 0 && spec[0].second == false )
    {
      if ( traceMode )
      {
        cmsg.info() << "lex desc" << endl;
        cmsg.send();
      }

      return -1;
    }

    // test if all order specifiers are true
    if ( flags == ( ( 1 << spec.size() ) -1 ) && spec[0].second == true )
    {
      if ( traceMode )
      {
        cmsg.info() << "lex asc" << endl;
        cmsg.send();
      }

      return 1;
    }

    if ( traceMode )
    {
      cmsg.info() << "spec (sort order mixed)" << endl
                  << "flags: " << flags << endl
                  << "calc: " << ( ( 1 << spec.size() ) -1 ) << endl;
      cmsg.send();
    }

    return 0;
  }
}

} // end of namespace extrel2
