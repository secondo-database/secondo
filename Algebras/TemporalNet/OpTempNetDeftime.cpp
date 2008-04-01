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
#include "DateTime.h"

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

#include "OpTempNetDeftime.h"



/*
Typemap function of the operator ~deftime~

*/
ListExpr OpTempNetDeftime::TypeMap(ListExpr in_xArgs)
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

  return nl->SymbolAtom( "periods" );
}

/*
Value mapping function of operator ~periods~

*/
int OpTempNetDeftime::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  Periods* pResult = (Periods*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pResult );
  MGPoint* pMGP = (MGPoint*) args[0].addr;
  pResult->Clear();
  if (!pMGP->IsDefined() || pMGP->GetNoComponents() < 1) {
    return 0;
  }
  const UGPoint *unit;
  pMGP->Get(0, unit );
  Instant start, end;
  bool bstart, bend;
  start = unit->timeInterval.start;
  end = unit->timeInterval.end;
  bstart = unit->timeInterval.lc;
  bend = unit->timeInterval.rc;
  pResult->StartBulkLoad();
  for( int i = 1; i < pMGP->GetNoComponents(); i++ ) {
    pMGP->Get( i, unit );
    if (unit->timeInterval.start == end && (bend || unit->timeInterval.lc)){
      end = unit->timeInterval.end;
      bend = unit->timeInterval.rc;
    } else {
      pResult->Add(Interval<Instant> (start, end, bstart, bend));
      start = unit->timeInterval.start;
      end = unit->timeInterval.end;
      bstart = unit->timeInterval.lc;
      bend = unit->timeInterval.rc;
    }
  }
  pResult->Add(Interval<Instant> (start, end, bstart, bend));
  pResult->EndBulkLoad();
  return 0;
}


/*
Specification of operator ~deftime~

*/
const string OpTempNetDeftime::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> periods" "</text--->"
  "<text>deftime(mgpoint)</text--->"
  "<text>Returns the periods in which the moving gpoint is defined.</text--->"
  "<text>deftime(mgpoint)</text--->"
  ") )";

