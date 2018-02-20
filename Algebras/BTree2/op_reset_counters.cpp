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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of the reset\_counters Operator

[TOC]

0 Overview

1 Defines and Includes

*/

#include "op_reset_counters.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "BTree2.h"


extern NestedList* nl;
extern QueryProcessor *qp;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operator ~reset\_counters~

Sets the nodesVisitedCounter and the cacheHitsCounter to 0.

Signature is

----
    reset\_counters: (btree2) --> (bool)
----

2.1 TypeMapping

*/
ListExpr reset_counters::TypeMapping( ListExpr args){
    if(nl->ListLength(args) != 1){
       return listutils::typeError("Operator expects exact1y 1 argument");
    }
    if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a btree2 object "
                                  "as argument.");
    }
    return (nl->SymbolAtom(CcBool::BasicType()));
}

/*
2.2 Valuemapping

*/
int
reset_counters::ValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree = (BTree2*)args[0].addr;
  CcBool *res = (CcBool*) result.addr;
  btree->resetNodesVisitedCounter();
  btree->resetCacheHitCounter();
  res->Set( true, true );
  return 0;

}

/*
2.3 Operator specification

*/
struct resetCountersInfo : OperatorInfo {

  resetCountersInfo() : OperatorInfo()
  {
    name =      "reset_counters";
    signature = "(btree2 Tk Td u) -> bool";
    syntax =    "reset_counters ( _ )";
    meaning =   "Resets the cachehit and nodesvisited counter";
    example =   "query reset_counters (staedte_btree2)";
  }
};

Operator reset_counters::def( resetCountersInfo(),
                                  reset_counters::ValueMapping,
                                  reset_counters::TypeMapping);
}
}


