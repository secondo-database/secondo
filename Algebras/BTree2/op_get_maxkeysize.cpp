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


#include "op_get_maxkeysize.h"

#include "WinUnix.h"

#include "ListUtils.h"
#include "QueryProcessor.h"
#include "RelationAlgebra.h"
#include "TupleIdentifier.h"

#include "BTree2.h"

#include <limits>

extern NestedList* nl;
extern QueryProcessor *qp;

using namespace std;

namespace BTree2Algebra {
namespace Operators {

/*
2 Operator ~get\_maxkeysize~

Signature is

----
    get\_maxkeysize:    --> (int)
    get\_maxkeysize: (btree2) --> (int)
----

2.1 TypeMapping

*/

ListExpr get_maxkeysize::TypeMapping(ListExpr args)
{
  if ((nl->ListLength(args) > 1)) {
    return listutils::typeError("too many arguments for get_maxkeysize");
  }
  if (nl->ListLength(args) == 1) {
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
int get_maxkeysize::Select( ListExpr args )
{
  // Distinguish by number of arguments
  if (nl->ListLength(args) == 0) {
    return 0;
  } else {
    return 1;
  }
}

/*
2.3 Valuemappings

*/
int
get_maxkeysize::ValueMapping_Default(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, BTree2::GetDefaultMaxKeysize());
  return 0;
}

int
get_maxkeysize::ValueMapping_BTree(Word* args, Word& result, int message,
                            Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BTree2* btree = (BTree2*)args[0].addr;
  CcInt *res = (CcInt*) result.addr;
  res->Set( true, btree->GetMaxKeysize() );
  return 0;
}

/*
2.4 Operator specification

*/
string get_maxkeysize::Specification() {
  string header = "\"Signature\" \"Syntax\" \"Meaning\" \"Example\"";
  string sig = " -> int\n"
               "(btree2 Tk Td u) -> int";
  string spec = "get_maxkeysize(_)";
  string meaning = "If called without parameter, it returns the "
                   "default keysize. Otherwise it returns the "
                   "keysize of the given btree2. See set_maxkeysize.";

  string example = "query get_maxkeysize()";

  return "( ( "+header + ") ( " +
         "<text>"+sig+"</text--->" +
         "<text>"+spec+"</text--->" +
         "<text>"+meaning+"</text--->" +
         "<text>"+example+"</text--->" +
         " ) )";
}

int get_maxkeysize::numberOfValueMappings = 2;
ValueMapping get_maxkeysize::valueMappings[] = {
                  get_maxkeysize::ValueMapping_Default,
                  get_maxkeysize::ValueMapping_BTree,
                };

Operator get_maxkeysize::def (
          "get_maxkeysize",                     // name
          get_maxkeysize::Specification(),      // specification
          get_maxkeysize::numberOfValueMappings,// number of functions
          get_maxkeysize::valueMappings,        // value mapping
          get_maxkeysize::Select,               // trivial selection function
          get_maxkeysize::TypeMapping           // type mapping
);

} // end namespace operators
} // end namespace btree2algebra

