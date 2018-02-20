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

[1] Implementation of the get\_maxkeysize Operator

[TOC]

0 Overview

1 Defines and Includes

*/

#include "op_get_maxvaluesize.h"

#include "WinUnix.h"

#include "ListUtils.h"
#include "QueryProcessor.h"

#include "BTree2.h"

#include <limits>



using namespace std;

extern NestedList* nl;
extern QueryProcessor *qp;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operator ~get\_maxvaluesize~

Signature is

----
    get\_maxvaluesize:    --> (int)
    get\_maxvaluesize: (btree2) --> (int)
----

2.1 TypeMapping

*/


ListExpr get_maxvaluesize::TypeMapping(ListExpr args)
{
  if ((nl->ListLength(args) > 1)) {
    return listutils::typeError("too many arguments for get_maxvaluesize");
  }
  if (nl->ListLength(args) != 0) {
    ListExpr first = nl->First(args);
    if (!listutils::isBTree2Description(first)) {
      return listutils::typeError("argument must be a valid btree2");
    }
  }
  return nl->OneElemList( nl->SymbolAtom(CcInt::BasicType()));
}

/*
2.2 Select

*/
int get_maxvaluesize::Select( ListExpr args )
{
  if (nl->ListLength(args) == 0) {
     // Distinguish by number of arguments
    return 0;
  } else {
    return 1;
  }
  return -1;
}

/*
2.3 Valuemappings

*/
int
get_maxvaluesize::ValueMapping_Default(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, BTree2::GetDefaultMaxValuesize());
  return 0;
}

int
get_maxvaluesize::ValueMapping_BTree(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree = (BTree2*)args[0].addr;
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, btree->GetMaxValuesize() );
  return 0;
}

/*
2.4 Operator specification

*/
string get_maxvaluesize::Specification() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = " -> int\n"
               "(btree2 ti ta unique) -> int";
  string spec = "get_maxvaluesize(_)";
  string meaning = "If called without parameter, it givea the "
                   "default valuesize. Otherwise it gives the "
                   "valuesize of the given btree2.";

  string example = "query get_maxvaluesize()";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

int get_maxvaluesize::numberOfValueMappings = 2;
ValueMapping get_maxvaluesize::valueMappings[] = {
                  get_maxvaluesize::ValueMapping_Default,
                  get_maxvaluesize::ValueMapping_BTree,
                };

Operator get_maxvaluesize::def (
          "get_maxvaluesize",                     // name
          get_maxvaluesize::Specification(),      // specification
          get_maxvaluesize::numberOfValueMappings,// number of functions
          get_maxvaluesize::valueMappings,        // value mapping
          get_maxvaluesize::Select,               // trivial selection function
          get_maxvaluesize::TypeMapping           // type mapping
);

} // end namespace operators
} // end namespace btree2algebra

