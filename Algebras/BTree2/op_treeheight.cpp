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

[1] Implementation of the treeheight Operator

[TOC]

0 Overview

*/

#include "op_treeheight.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "BTree2.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor *qp;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operator ~treeheight~

Signature is

----
    treeheight: (btree2) --> (int)
----

2.1 TypeMapping

*/
ListExpr treeheight::TypeMapping( ListExpr args){
    if(nl->ListLength(args) != 1){
      return listutils::typeError( "Operator expects exactly one argument");
     }
     if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a btree2 "
                                  "object as argument.");
     }
  return (nl->SymbolAtom(CcInt::BasicType()));
}

/*
2.2 Valuemapping

*/
int
treeheight::ValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree = (BTree2*)args[0].addr;
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, btree->GetTreeHeight() );
  return 0;

}

/*
2.3 Operator specification

*/
struct treeheightInfo : OperatorInfo {

  treeheightInfo() : OperatorInfo()
  {
    name =      "treeheight";
    signature = "(btree2 Tk Td u) -> int";
    syntax =    "treeheight ( _ )";
    meaning =   "Returns the height of the btree2.";
    example =   "query treeheight (staedte_btree2)";
  }
};

Operator treeheight::def( treeheightInfo(), treeheight::ValueMapping,
                                           treeheight::TypeMapping);
}
}


