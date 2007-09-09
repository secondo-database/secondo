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

1 Implementation of operator thenetwork


Mai-Oktober 2007 Martin Scheppokat

1.1 Overview


This file contains the implementation of ~gline~


2.2 Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "StandardTypes.h"
#include "GPoint.h"
#include "SpatialAlgebra.h"
#include "Network.h"


#include "OpNetworkTheNetwork.h"

ListExpr OpNetworkTheNetwork::TypeMap(ListExpr in_xArgs)
{
  if(nl->ListLength(in_xArgs) != 3)
    return (nl->SymbolAtom("typeerror"));

  ListExpr xIdDesc = nl->First(in_xArgs);
  ListExpr xRoutesRelDesc = nl->Second(in_xArgs);
  ListExpr xJunctionsRelDesc = nl->Third(in_xArgs);

  if(!nl->IsEqual(xIdDesc, "int"))
  {
    return (nl->SymbolAtom("typeerror"));
  }

  if(!IsRelDescription(xRoutesRelDesc) ||
     !IsRelDescription(xJunctionsRelDesc))
  {
    return (nl->SymbolAtom("typeerror"));
  }

  ListExpr xType;
  nl->ReadFromString(Network::routesTypeInfo, xType);
  if(!CompareSchemas(xRoutesRelDesc, xType))
  {
    return (nl->SymbolAtom("typeerror"));
  }
  
  nl->ReadFromString(Network::junctionsTypeInfo, xType);
  if(!CompareSchemas(xJunctionsRelDesc, xType))
  {
    return (nl->SymbolAtom("typeerror"));
  }
  
  return nl->SymbolAtom("network");
}

/*
4.1.2 Value mapping function of operator ~thenetwork~

*/
int OpNetworkTheNetwork::ValueMapping(Word* args, Word& result, 
                               int message, Word& local, Supplier s)
{
  Network* pNetwork = (Network*)qp->ResultStorage(s).addr;

  CcInt* pId = (CcInt*)args[0].addr;
  int iId = pId->GetIntval();

  Relation* pRoutes = (Relation*)args[1].addr;
  Relation* pJunctions = (Relation*)args[2].addr;
 
  pNetwork->Load(iId,
                 pRoutes, 
                 pJunctions);

  result = SetWord(pNetwork); 
  return 0;
}

/*
4.1.3 Specification of operator ~thenetwork~

*/
const string OpNetworkTheNetwork::Spec  = 
  "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>int x rel x rel -> network" "</text--->"
  "<text>thenetwork(_, _, _)</text--->"
  "<text>Creates a network.</text--->"
  "<text>let n = thenetwork(1, r, j)</text--->"
  "))";
