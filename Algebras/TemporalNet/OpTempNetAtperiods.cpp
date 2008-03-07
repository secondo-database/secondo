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

1.1 Implementation of Operator atperiods

This operator restricts a moving gpoint to the given periods.

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

#include "OpTempNetAtperiods.h"

/*
Typemap function of the operator

*/
ListExpr OpTempNetAtperiods::TypeMap(ListExpr in_xArgs)
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

  return nl->SymbolAtom("mgpoint");
}

/*
Value mapping function of operator ~atperiods~

*/
int OpTempNetAtperiods::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  MGPoint* pMGPres = (MGPoint*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pMGPres );
  pMGPres->Clear();
  pMGPres->SetDefined(true);

  MGPoint* pMGP = (MGPoint*)args[0].addr;
  if(pMGP == NULL ||!pMGP->IsDefined()) {
    cerr << "MGPoint does not exist." << endl;
    pMGPres->SetDefined(false);
    return 0;
  }

  Periods* per = (Periods*)args[1].addr;
  if(per == NULL || !per->IsDefined()) {
    cerr << "Periods are not defined." << endl;
    pMGPres->SetDefined(false);
    return 0;
  }
  if (pMGP->GetNoComponents() < 1 || per->IsEmpty()) {
    pMGPres->SetDefined(true);
    return 0;
  }
  Instant perMinInst;
  per->Minimum(perMinInst);
  Instant perMaxInst;
  per->Maximum(perMaxInst);
  Instant utstart, utend;
  GPoint uGPstart, uGPend;
  bool ulc, urc;
  double permind = perMinInst.ToDouble();
  double permaxd = perMaxInst.ToDouble();
  const UGPoint *pFirstUnit, *pLastUnit, *pCurrentUnit;
  pMGP->Get(0, pFirstUnit);
  pMGP->Get(pMGP->GetNoComponents()-1, pLastUnit);
  double mind = (pFirstUnit->timeInterval.start).ToDouble();
  double maxd = (pLastUnit->timeInterval.end).ToDouble();
  double factor;
  if( (mind > permaxd && !AlmostEqual(mind,permaxd)) ||
      (maxd < permind && !AlmostEqual(maxd,permind))) {
    pMGPres->SetDefined(true);
    return 0;
  }
  pMGPres->StartBulkLoad();
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
        if (pCurrentUnit->timeInterval.start == interval->start) {
          utstart = interval->start;
          uGPstart = pCurrentUnit->p0;
          ulc = pCurrentUnit->timeInterval.lc && interval->lc;
        } else {
          if (pCurrentUnit->timeInterval.start > interval->start) {
            utstart = pCurrentUnit->timeInterval.start;
            uGPstart = pCurrentUnit->p0;
            ulc = pCurrentUnit->timeInterval.lc;
          } else {
            if (pCurrentUnit->timeInterval.start < interval->start) {
              utstart = interval->start;
              ulc = interval->lc;
              factor = (interval->start.ToDouble() -
                      pCurrentUnit->timeInterval.start.ToDouble())/
                        (pCurrentUnit->timeInterval.end.ToDouble() -
                          pCurrentUnit->timeInterval.start.ToDouble());
              uGPstart = GPoint(true, pCurrentUnit->p0.GetNetworkId(),
                             pCurrentUnit->p0.GetRouteId(),
                             fabs(pCurrentUnit->p1.GetPosition() -
                                 pCurrentUnit->p0.GetPosition()) *
                                 factor + pCurrentUnit->p0.GetPosition(),
                             pCurrentUnit->p0.GetSide());
            }
          }
        }
        if (pCurrentUnit->timeInterval.end == interval->end) {
          utend = interval->end;
          uGPend = pCurrentUnit->p1;
          urc = pCurrentUnit->timeInterval.rc && interval->rc;
        } else {
          if (pCurrentUnit->timeInterval.end < interval->end) {
            utend = pCurrentUnit->timeInterval.end;
            urc = pCurrentUnit->timeInterval.rc;
            uGPend = pCurrentUnit->p1;
          } else {
            if (pCurrentUnit->timeInterval.end > interval->end) {
              utend = interval->end;
              factor = (interval->end.ToDouble() -
                        pCurrentUnit->timeInterval.start.ToDouble())/
                        (pCurrentUnit->timeInterval.end.ToDouble() -
                        pCurrentUnit->timeInterval.start.ToDouble());
              uGPend = GPoint(true, pCurrentUnit->p1.GetNetworkId(),
                             pCurrentUnit->p1.GetRouteId(),
                             fabs(pCurrentUnit->p1.GetPosition() -
                                 pCurrentUnit->p0.GetPosition()) *
                                 factor + pCurrentUnit->p0.GetPosition(),
                                pCurrentUnit->p0.GetSide());
              urc = interval->rc;
            }
          }
        }
        pMGPres->Add(UGPoint(Interval<Instant>(utstart, utend, ulc, urc),
                             uGPstart.GetNetworkId(),
                             uGPstart.GetRouteId(),
                             uGPstart.GetSide(),
                             uGPstart.GetPosition(),
                             uGPend.GetPosition()));
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
  pMGPres->EndBulkLoad();
  if (pMGPres->GetNoComponents() == 0) pMGPres->SetDefined(false);
  else pMGPres->SetDefined(true);
  return 0;
}

/*
Specification of the operator

*/
const string OpTempNetAtperiods::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint x periods -> mgpoint" "</text--->"
  "<text> _ atperiods _</text--->"
  "<text>Restricts the moving gpoint to the given periods.</text--->"
  "<text>X_MGPOINT atperiods PERIODS </text--->"
  ") )";
