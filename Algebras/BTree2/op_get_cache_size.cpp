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


[1] Implementation of the get\_cache\_size Operator

[TOC]

0 Overview


1 Defines and Includes

*/

#include "op_get_cache_size.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "BTree2.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor *qp;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operator ~get\_cache\_size~

Signature is

----
    get\_cache\_size: (btree2) --> (int)
----

2.1 TypeMapping

*/

ListExpr get_cache_size::TypeMapping( ListExpr args){
    if(nl->ListLength(args) != 1){
       return listutils::typeError("Operator expects exactly one argument");
    }
    if(!listutils::isBTree2Description(nl->First(args))){
      return listutils::typeError("Operator expects a btree2 "
                                  "object as argument.");
    }
  return (nl->SymbolAtom(CcInt::BasicType()));
}

/*
2.2 Value Mapping

*/
int
get_cache_size::ValueMapping(Word* args, Word& result, int message,
        Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree = (BTree2*)args[0].addr;
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, btree->GetCacheSize() );
  return 0;

}

/*
2.3 Specification of Operator

*/
struct getCacheSizeInfo : OperatorInfo {

  getCacheSizeInfo() : OperatorInfo()
  {
    name =      "get_cache_size";
    signature = "(btree2 Tk Td u) -> int";
    syntax =    "get_cache_size ( _ )";
    meaning =   "Returns the size of the btree2's cache size.";
    example =   "query get_cache_size (staedte_btree2)";
  }
};

Operator get_cache_size::def( getCacheSizeInfo(), get_cache_size::ValueMapping,
                                           get_cache_size::TypeMapping);
}
}


