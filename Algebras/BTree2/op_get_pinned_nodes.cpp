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

[1] Implementation of the get\_pinned\_nodes Operator

[TOC]

0 Overview

*/


#include "op_get_pinned_nodes.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "BTree2.h"
#include "Symbols.h"

extern NestedList* nl;
extern QueryProcessor *qp;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operator ~get\_pinned\_nodes~

Signature is

----
    get\_pinned\_nodes: (btree2) --> stream(int)
----

2.1 TypeMapping for Operator ~get\_pinned\_nodes~

*/
ListExpr get_pinned_nodes::TypeMapping( ListExpr args){
    if(nl->ListLength(args) != 1){
      return listutils::typeError("Operator expects exactly one argument");
    }
    if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a btree2 "
                                  "object as argument.");
    }
  return (nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
                          nl->SymbolAtom(CcInt::BasicType())));
}

/*
2.2 Value Mapping function

*/
struct valueMappingInfo{
  std::set<BTree2::PinnedNodeInfo>::iterator iter;
  std::set<BTree2::PinnedNodeInfo>::iterator end;
  valueMappingInfo(std::set<BTree2::PinnedNodeInfo>::iterator it,
          std::set<BTree2::PinnedNodeInfo>::iterator e) : iter(it), end(e) {}
};


int
get_pinned_nodes::ValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{

  valueMappingInfo* vmi;
  BTree2::PinnedNodeInfo pni;
  switch( message )
  {
    case OPEN: {
      BTree2* btree = (BTree2*)args[0].addr;
      std::set<BTree2::PinnedNodeInfo> const* pinnedNodes = btree->
                                                         GetPinnedNodes();
      std::set<BTree2::PinnedNodeInfo>::iterator iter = pinnedNodes->begin();
      std::set<BTree2::PinnedNodeInfo>::iterator end = pinnedNodes->end();
      vmi = new valueMappingInfo(iter, end);

      local.addr = vmi;

      return 0;
    }
    case REQUEST: {
      vmi = (valueMappingInfo*)local.addr;

      if ( vmi->iter != vmi->end )
      {
        pni = *vmi->iter;
        CcInt* elem = new CcInt(true, pni.id);
        result.addr = elem;
        vmi->iter++;
        return YIELD;
      }
      else
      {
        result.addr = 0;
        return CANCEL;
      }
    }
    case CLOSE: {

      if (local.addr != 0) {
        vmi = (valueMappingInfo*) local.addr;
        delete vmi;
        local.addr = 0;
      }

      return 0;
    }
    default: {
      return -1;
    }
  }
}

/*
2.3 Operator specification

*/
struct getPinnedNodesInfo : OperatorInfo {

  getPinnedNodesInfo() : OperatorInfo()
  {
    name =      "get_pinned_nodes";
    signature = "(btree2 Tk Td u) -> stream(int)";
    syntax =    "get_pinned_nodes ( _ )";
    meaning =   "Returns the pinned nodes in the btree2.";
    example =   "query get_pinned_nodes (staedte_btree2) countintstream";
  }
};

Operator get_pinned_nodes::def( getPinnedNodesInfo(),
                             get_pinned_nodes::ValueMapping,
                                get_pinned_nodes::TypeMapping);
}
}
