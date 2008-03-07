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
ListExpr OpTempNetPresent::TypeMap(ListExpr in_xArgs)
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

  if (!nl->IsEqual( xPeriods, "periods" ))
  {
    return (nl->SymbolAtom( "typeerror" ));
  }

  return nl->SymbolAtom("bool");
}

/*
Value mapping function of operator ~present~

*/
int OpTempNetPresent::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  CcBool* pPresent = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pPresent );
  pPresent->Set(false, false);
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
  Instant perMinInst;
  per->Minimum(perMinInst);
  Instant perMaxInst;
  per->Maximum(perMaxInst);
  double permind = perMinInst.ToDouble();
  double permaxd = perMaxInst.ToDouble();
  const UGPoint *pFirstUnit, *pLastUnit, *pCurrentUnit;
  pMGP->Get(0, pFirstUnit);
  pMGP->Get(pMGP->GetNoComponents()-1, pLastUnit);
  double mind = (pFirstUnit->timeInterval.start).ToDouble();
  double maxd = (pLastUnit->timeInterval.end).ToDouble();
  if( (mind > permaxd && !AlmostEqual(mind,permaxd)) ||
      (maxd < permind && !AlmostEqual(maxd,permind))) {
    pPresent->Set(true, false);
    return 0;
  }
  const Interval<Instant> *interval;
  int i = 0, j = 0;
  pMGP->Get( i, pCurrentUnit );
  per->Get( j, interval );
  while( 1 ) {
    if( pCurrentUnit->timeInterval.Before( *interval)){
      if( ++i == pMGP->GetNoComponents()) break;
      pMGP->Get( i, pCurrentUnit );
    } else {
      if( interval->Before( pCurrentUnit->timeInterval )) {
        if( ++j == per->GetNoComponents()) break;
        per->Get( j, interval );
      } else { // we have overlapping intervals, now
        if (!((interval->start == interval->end &&
           (!interval->lc || !interval->rc)) ||
           (pCurrentUnit->timeInterval.start == pCurrentUnit->timeInterval.end
           &&(!pCurrentUnit->timeInterval.lc || !pCurrentUnit->timeInterval.rc))
           || (interval->end == pCurrentUnit->timeInterval.start &&
           (!interval->rc || !pCurrentUnit->timeInterval.lc)) ||
           (interval->start == pCurrentUnit->timeInterval.end &&
           (!interval->lc || !pCurrentUnit->timeInterval.rc)))) {
          pPresent->Set(true, true);
          return 0;
        } else {
          if( interval->end == pCurrentUnit->timeInterval.end ){
            // same ending instant
            if( interval->rc == pCurrentUnit->timeInterval.rc ) {
              // same ending instant and rightclosedness: Advance both
              if( ++i == pMGP->GetNoComponents()) break;
              pMGP->Get( i, pCurrentUnit );
              if( ++j == per->GetNoComponents()) break;
              per->Get( j, interval );
            } else {
              if( interval->rc == true ) { // Advanve in mapping
                if( ++i == pMGP->GetNoComponents() ) break;
                pMGP->Get( i, pCurrentUnit );
              } else { // Advance in periods
                assert( pCurrentUnit->timeInterval.rc == true );
                if( ++j == per->GetNoComponents() ) break;
                per->Get( j, interval );
              }
            }
          } else {
            if( interval->end > pCurrentUnit->timeInterval.end ) {
              // Advance in mpoint
              if( ++i == pMGP->GetNoComponents() ) break;
              pMGP->Get( i, pCurrentUnit );
            } else { // Advance in periods
              assert( interval->end < pCurrentUnit->timeInterval.end );
              if( ++j == per->GetNoComponents() ) break;
              per->Get( j, interval );
            }
          }
        }
      }
    }
  }
  pPresent->Set(true,false);
  return 0;
}

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
