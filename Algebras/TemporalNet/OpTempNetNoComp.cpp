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

1.1 Implementation of Operator no components

This operator returns the number of components of the moving gpoint

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

#include "OpTempNetNoComp.h"

/*
Typemap function of the operator

*/
ListExpr OpTempNetNoComp::TypeMap(ListExpr in_xArgs)
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

  return nl->SymbolAtom("int");
}

/*
Value mapping function of operator ~nocomponents~

*/
int OpTempNetNoComp::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  CcInt* pNumber = (CcInt*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pNumber );
  MGPoint* pMGP = (MGPoint*)args[0].addr;
  pNumber->Set(false, 0);
  if(pMGP == NULL ||!pMGP->IsDefined()) {
    pNumber->Set(false,0);
    cerr << "MGPoint is not defined." << endl;
  } else {
    pNumber->Set(true, pMGP->GetNoComponents());
  }
  return 0;
}

/*
Specification of the operator

*/
const string OpTempNetNoComp::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> int" "</text--->"
  "<text> no_components _ </text--->"
  "<text>Returns the number of components of the moving gpoint.</text--->"
  "<text>no_components(MGPOINT) </text--->"
  ") )";
