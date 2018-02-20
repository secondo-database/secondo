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


[1] Implementation of the set\_cache\_size Operator

[TOC]

0 Overview


1 Defines and Includes

*/
#include "op_set_cache_size.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "BTree2.h"

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;


/*
2 Operator ~set\_cache\_size~

Signature is

----
    set[_]cache[_]size: (btree2) x int--> (bool)
----

2.1 TypeMapping

*/

namespace BTree2Algebra {
namespace Operators {

ListExpr set_cache_size::TypeMapping( ListExpr args){
    if(nl->ListLength(args) != 2){
      return listutils::typeError("Operator expects 2 arguments");
    }
    if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a "
                                  "btree2 object as argument.");
    }
    ListExpr second = nl->Second(args);
    if(!nl->IsEqual(second, CcInt::BasicType())){
      return listutils::typeError( "Second argument must be of type int.");
    }
  return (nl->SymbolAtom(CcBool::BasicType()));
}


/*
2.2 Valuemapping

*/
int
set_cache_size::ValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree = (BTree2*)args[0].addr;
  unsigned max = ((CcInt*)args[1].addr)->GetIntval();
  CcBool *res = (CcBool*) result.addr;
  const set<BTree2::PinnedNodeInfo>* pinnedNodes = btree->GetPinnedNodes();
  if (btree->IsFixedElementsLimitType())
  {
    if (max < pinnedNodes->size())
      res->Set(true, false);
    else{
      btree->SetCacheSize(max);
      res->Set( true, max == btree->GetCacheSize() );
    }
  }
  else{
    set<BTree2::PinnedNodeInfo>::iterator it;
    size_t pinnedNodesMemory = 0;
    for (it = pinnedNodes->begin(); it != pinnedNodes->end(); it++){
      pinnedNodesMemory += it->memoryUsage;
    }

    if (pinnedNodesMemory > max){
      res->Set(true, false);
    }
    else{
      btree->SetCacheSize(max);
      res->Set( true, max == btree->GetCacheSize() );
    }
  }
  return 0;
}

/*
2.3 Operator specification

*/
struct setCacheSizeInfo : OperatorInfo {

  setCacheSizeInfo() : OperatorInfo()
  {
    name =      "set_cache_size";
    signature = "(btree2 Tk Td u) x int -> bool";
    syntax =    "set_cache_size ( _, _ )";
    meaning =   "Sets the size of the btree2's cache size.";
    example =   "query set_cache_size (staedte_btree2, 10000000)";
  }
};

Operator set_cache_size::def( setCacheSizeInfo(), set_cache_size::ValueMapping,
                                           set_cache_size::TypeMapping);
}
}


