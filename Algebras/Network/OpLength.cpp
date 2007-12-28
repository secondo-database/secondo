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

1.1 Declaration of Operator Length

Dezember 2007 Simone Jandt

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

#include "OpLength.h"
#include "Messages.h"

/*
TypeMap Function of the operator length

*/

ListExpr OpLength::TypeMap(ListExpr args){
  if (nl->ListLength(args) != 1) {
    return (nl->SymbolAtom("typeerror"));
  }
  ListExpr gline = nl->First(args);
  if (!nl->IsAtom(gline) || nl->AtomType(gline) != SymbolType ||
       nl->SymbolValue(gline) != "gline"){
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("real");
}

/*
ValueMapping function of the operator length

*/

int OpLength::ValueMapping (Word* args, Word& result, int message,
      Word& local, Supplier in_pSupplier){
  GLine* pGline = (GLine*) args[0].addr;
  result = qp->ResultStorage(in_pSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  if (!(pGline->IsDefined())) {
    cmsg.inFunError("gline is not defined!");
    return 0;
  };
  pResult-> Set(true, pGline->GetLength());
  return 1;
}
/*
Specification of operator length:

*/

const string OpLength::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>gline -> real" "</text--->"
  "<text>length(_)</text--->"
  "<text>Calculates the length of the gline.</text--->"
  "<text>let n = length(line)</text--->"
  ") )";

