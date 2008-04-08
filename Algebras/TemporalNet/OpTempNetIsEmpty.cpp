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

1.1 Implementation of Operator isempty

This operator returns true if the moving gpoint is empty or undefined.

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

#include "OpTempNetIsEmpty.h"

/*
Typemap function of the operator

*/
ListExpr OpTempNetIsEmpty::TypeMap(ListExpr in_xArgs)
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

  return nl->SymbolAtom("bool");
}

/*
Value mapping function of operator ~isempty~

*/
int OpTempNetIsEmpty::ValueMapping(Word* args,
                                   Word& result,
                                   int message,
                                   Word& local,
                                   Supplier in_xSupplier)
{
  CcBool* pEmpty = (CcBool*)qp->ResultStorage(in_xSupplier).addr;
  result = SetWord( pEmpty );
  MGPoint* pMGP = (MGPoint*)args[0].addr;
  pEmpty->Set(false, false);
  if(pMGP == NULL ||!pMGP->IsDefined() || pMGP->GetNoComponents() == 0) {
    pEmpty->Set(true,true);
  } else {
    pEmpty->Set(true, false);
  }
  return 0;
}

/*
Specification of the operator

*/
const string OpTempNetIsEmpty::Spec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> bool" "</text--->"
  "<text> isemtpy (_) </text--->"
  "<text>Returns true if the moving gpoint is empty, false elsewhere.</text--->"
  "<text>isempty (MGPOINT) </text--->"
  ") )";
