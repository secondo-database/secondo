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

1.1 Implementation of Operator present

This operator returns true if a moving gpoint at least once exists in the given
periods.

March 2008 Simone Jandt

*/
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "StandardTypes.h"

#include "TemporalAlgebra.h"
#include "NetworkAlgebra.h"
#include "GPoint.h"
#include "UGPoint.h"
#include "MGPoint.h"
#include "TemporalNetAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"

#include "OpTempNetPresent.h"

/*
Typemap function of the operator

*/
ListExpr OpTempNetPresent::PresentMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);
  ListExpr xPeriods = nl->Second(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  if (nl->IsEqual( xPeriods, "periods" ) ||
     nl->IsEqual( xPeriods, "instant" ))
  {
    return (nl->SymbolAtom( "bool" ));
  }

  return nl->SymbolAtom("typeerror");
}

/*
Value mapping function of operator ~present~

*/


int OpTempNetPresent::present_mgpi(Word* args, Word& result, int message,
                                Word& local,Supplier in_xSupplier){
  CcBool* pPresent = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pPresent );
  MGPoint* pMGP = (MGPoint*)args[0].addr;
  if(pMGP == NULL ||!pMGP->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pPresent->Set(false,false);
    return 0;
  }
  const Instant* ins = (Instant*)args[1].addr;
  if(ins == NULL || !ins->IsDefined()) {
    cerr << "Instant is not defined." << endl;
    pPresent->Set(false,false);
    return 0;
  }
  if (pMGP->GetNoComponents() < 1) {
    pPresent->Set(true,false);
    return 0;
  }
  const UGPoint *pCurrentUnit;
  int i = 0;
  int comp;
  while( i < pMGP->GetNoComponents()) {
    pMGP->Get( i, pCurrentUnit );
    comp = pCurrentUnit->timeInterval.end.CompareTo(ins);
    if (comp < 0) i++;
    else {
      comp = pCurrentUnit->timeInterval.start.CompareTo(ins);
      if (comp > 0) {
        pPresent->Set(true, false);
        return 0;
      } else {
        pPresent->Set(true, true);
        return 0;
      }
    }
  }
  pPresent->Set(true,false);
  return 0;
};

int OpTempNetPresent::present_mgpp(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  CcBool* pPresent = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pPresent );
  MGPoint* pMGP = (MGPoint*)args[0].addr;
  if(pMGP == NULL ||!pMGP->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pPresent->Set(false,false);
    return 0;
  }
  Periods* per = (Periods*)args[1].addr;
  if(per == NULL || !per->IsDefined()) {
    cerr << "Periods are not defined." << endl;
    pPresent->Set(false,false);
    return 0;
  }
  if (pMGP->GetNoComponents() < 1 || per->IsEmpty()) {
    pPresent->Set(true,false);
    return 0;
  }
  const Interval<Instant> *interval;
  const UGPoint *pCurrentUnit;
  int i, j;
  j = 0;
  while (j < per->GetNoComponents()) {
    per->Get( j, interval );
    i = 0;
    while (i < pMGP->GetNoComponents()) {
      pMGP->Get( i, pCurrentUnit );
      if (pCurrentUnit->timeInterval.Before(*interval)) i++;
      else {
        if (interval->Before(pCurrentUnit->timeInterval)) break;
        else {
          pPresent->Set(true, true);
          return 0;
        }
      }
    }
    j++;
  }
  pPresent->Set(true,false);
  return 0;
}

int OpTempNetPresent::SelectPresent(ListExpr args){
  ListExpr arg1 = nl->First( args );
  ListExpr arg2 = nl->Second( args );

  if ( nl->SymbolValue(arg1) == "mgpoint" &&
       nl->IsEqual( arg2, "periods" ))
    return 0;
  if ( nl->SymbolValue(arg1) == "mgpoint" &&
       nl->IsEqual( arg2, "instant"))
    return 1;
  return -1; // This point should never be reached
};

ValueMapping OpTempNetPresent::presentmap [] = {
  present_mgpp,
  present_mgpi
};


/*
Specification of the operator

*/
const string OpTempNetPresent::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x periods -> bool" "</text--->"
  "<text> _ present _</text--->"
  "<text>Returns true if the moving gpoint at least once exists in the given"
    "periods.</text--->"
  "<text>X_MGPOINT present PERIODS </text--->"
  ") )";
