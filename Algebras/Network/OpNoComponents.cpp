/*
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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]

1.1 Declaration of Operator nocomponents

April 2008 Simone Jandt

Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "SpatialAlgebra.h"
#include "StandardTypes.h"
#include "GLine.h"
#include "GPoint.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpNoComponents.h"
#include "Messages.h"

/*
TypeMap Function of the operator nocomponents

*/

ListExpr OpNoComponents::TypeMap(ListExpr args){
  if (nl->ListLength(args) != 1) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr gline = nl->First(args);
  if (!nl->IsAtom(gline) || nl->AtomType(gline) != SymbolType ||
       nl->SymbolValue(gline) != "gline"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("int");
}

/*
ValueMapping function of the operator nocomponents

*/

int OpNoComponents::ValueMapping (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* pGline = (GLine*) args[0].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcInt* pResult = (CcInt*) result.addr;
  if (!(pGline->IsDefined())) {
    pResult->Set(false, 0);
    return 0;
  };
  pResult-> Set(true, pGline->NoOfComponents());
  return 1;
}
/*
Specification of operator nocomponents:

*/

const string OpNoComponents::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> int" "</text--->"
  "<text>no_components(_)</text--->"
  "<text>Returns the number of route intervals.</text--->"
  "<text>query no_components(gline)</text--->"
  ") )";

