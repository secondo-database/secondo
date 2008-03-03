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
//[TOC] [\tableofcontents]

1.1 Implementation of Operator Length

The operator computes the length of the path passed by a moving(gpoint).

March 2008 Simone Jandt

Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "StandardTypes.h"

#include "TemporalAlgebra.h"
#include "NetworkAlgebra.h"
#include "GPoint.h"
#include "GLine.h"
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpTempNetLength.h"



/*
Typemap function of the operator ~length~

*/
ListExpr OpTempNetLength::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom( "real" );
}

/*
Value mapping function of operator ~length~

*/
int OpTempNetLength::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  result = qp->ResultStorage(in_xSupplier);
  CcReal* pResult = (CcReal*) result.addr;
  // Get input value
  MGPoint* pMGPoint = (MGPoint*)args[0].addr;
  if(pMGPoint == NULL || pMGPoint->GetNoComponents() < 1 ||
      !pMGPoint->IsDefined()) {
    cmsg.inFunError("MGPoint does not exist.");
    pResult->Set(false, 0.0);
    return 0;
  }
  double dLength = 0.0;
  const UGPoint *pCurrentUnit;
  double curStartPos, curEndPos;
  for (int i = 0; i < pMGPoint->GetNoComponents(); i++)
  {
    // Get start and end of current unit
    pMGPoint->Get(i, pCurrentUnit);
    curStartPos = pCurrentUnit->p0.GetPosition();
    curEndPos = pCurrentUnit->p1.GetPosition();
    dLength = dLength + fabs(curEndPos - curStartPos);
  }
  pResult-> Set(true, dLength);
  return 1;
}


/*
Specification of operator ~length~

*/
const string OpTempNetLength::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> real" "</text--->"
  "<text>length(mgpoint)</text--->"
  "<text>Calculates the length of the pass passed by a moving gpoint.</text--->"
  "<text>length(mgpoint)</text--->"
  ") )";

