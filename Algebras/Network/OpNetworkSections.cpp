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
#include "GPoint.h"
#include "Network.h"

#include "OpNetworkSections.h"

/*
4.4 Operator ~sections~

4.4.1 Type Mapping of operator ~sections~

*/
ListExpr OpNetworkSections::TypeMap(ListExpr args)
{
  ListExpr arg1;
  if( nl->ListLength( args ) == 1 )
  {
    arg1 = nl->First( args );
    if( nl->IsAtom( arg1 ) &&
        nl->AtomType( arg1 ) == SymbolType &&
        nl->SymbolValue( arg1 ) == "network" )
      return Network::GetSectionsInternalTypeInfo();
  }
  return (nl->SymbolAtom( "typeerror" ));
}

/*
4.4.2 Value mapping function of operator ~sections~

*/
int OpNetworkSections::ValueMapping( Word* args, Word& result, int message, 
                             Word& local, Supplier s )
{
  Network *network = (Network*)args[0].addr;
  result = SetWord( network->GetSections() );

  Relation *resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s, result);

  return 0;
}

/*
4.4.3 Specification of operator ~sections~

*/
const string OpNetworkSections::Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\" ) "
  "( <text>network -> rel" "</text--->"
  "<text>sections(_)</text--->"
  "<text>Return the sections of a network.</text--->"
  "<text>let j = sections(n)</text--->"
  ") )";

