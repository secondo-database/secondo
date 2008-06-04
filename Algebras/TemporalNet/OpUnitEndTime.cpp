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

1.1 Implementation of Operator ~unitstarttime~

Returns the double value representing the start time instant of the given
~ugpoint~.

June 2008 Simone Jandt

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

#include "OpUnitEndTime.h"

/*
Typemap function of the operator

*/
ListExpr OpUnitEndTime::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMGPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMGPointDesc )) ||
      nl->AtomType( xMGPointDesc ) != SymbolType ||
      nl->SymbolValue( xMGPointDesc ) != "ugpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  return nl->SymbolAtom("real");
}

/*
Value mapping function of operator

*/
int OpUnitEndTime::ValueMapping( Word* args, Word& result, int message,
                             Word& local, Supplier s ){
  CcReal* pNumber = (CcReal*)qp->ResultStorage(s).addr;
  result = SetWord( pNumber );
  UGPoint* pUGP = (UGPoint*)args[0].addr;
  pNumber->Set(false, 0.0);
  if(pUGP == NULL ||!pUGP->IsDefined()) {
    pNumber->Set(false,0.0);
    cerr << "UGPoint is not defined." << endl;
  } else {
    pNumber->Set(true, (double) pUGP->GetUnitEndTime());
  }
  return 0;
}

/*
Specification of the operator

*/
const string OpUnitEndTime::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>ugpoint -> real" "</text--->"
  "<text> unitendtime (_)</text--->"
  "<text>Returns the double value representing the end time instant of the"
  " given ugpoint.</text--->"
  "<text>unitendtime(UGPOINT)</text--->"
  ") )";
