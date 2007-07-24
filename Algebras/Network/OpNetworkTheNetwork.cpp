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

[1] Implementation of GLine in Module Network Algebra

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

[TOC]

1 Overview


This file contains the implementation of ~gline~


2 Defines, includes, and constants

*/
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "TupleIdentifier.h"

#include "StandardTypes.h"
#include "Network.h"


#include "OpNetworkTheNetwork.h"

ListExpr OpNetworkTheNetwork::TypeMap(ListExpr args)
{
  if( nl->ListLength(args) != 2 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr rel1Desc = nl->First(args),
           rel2Desc = nl->Second(args);

  if( !IsRelDescription( rel1Desc ) ||
      !IsRelDescription( rel2Desc ) )
    return (nl->SymbolAtom( "typeerror" ));

  if( !CompareSchemas( rel1Desc, Network::GetRoutesTypeInfo() ) )
    return (nl->SymbolAtom( "typeerror" ));
  
  if( !CompareSchemas( rel2Desc, Network::GetJunctionsTypeInfo() ) )
    return (nl->SymbolAtom( "typeerror" ));
  
  return nl->SymbolAtom( "network" );
}

/*
4.1.2 Value mapping function of operator ~thenetwork~

*/
int OpNetworkTheNetwork::ValueMapping( Word* args, Word& result, 
                               int message, Word& local, Supplier s )
{
  Network *network = (Network*)qp->ResultStorage(s).addr;

  Relation *routes = (Relation*)args[0].addr,
           *junctions = (Relation*)args[1].addr;
 
  network->Load( routes, junctions );

  result = SetWord( network ); 
  return 0;
}

/*
4.1.3 Specification of operator ~thenetwork~

*/
const string OpNetworkTheNetwork::Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>rel x rel -> network" "</text--->"
  "<text>thenetwork(_, _)</text--->"
  "<text>Creates a network.</text--->"
  "<text>let n = thenetwork(r, j)</text--->"
  ") )";
