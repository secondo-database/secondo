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

[1] Implementation of Operator units for moving points

Mai-Oktober 2007 Martin Scheppokat
 
1 Overview


This file contains the implementation of ~gline~


2 Defines, includes, and constants

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

#include "OpUnits.h"

/*
4.1.2 Typemap function of the operator

*/
ListExpr OpUnits::TypeMap(ListExpr in_xArgs)
{
  if( nl->ListLength(in_xArgs) != 1 )
    return (nl->SymbolAtom( "typeerror" ));

  ListExpr xMPointDesc = nl->First(in_xArgs);

  if( (!nl->IsAtom( xMPointDesc )) ||
      nl->AtomType( xMPointDesc ) != SymbolType ||
      nl->SymbolValue( xMPointDesc ) != "mgpoint" )
  {
    return (nl->SymbolAtom( "typeerror" ));
  }
  
  return nl->TwoElemList(nl->SymbolAtom("stream"),
                         nl->SymbolAtom("ugpoint"));
}



/*
4.1.3 Specification of the operator

*/
const string OpUnits::Spec  = 
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "( <text>mgpoint -> (stream ugpoint)" "</text--->"
  "<text>units(mgpoint)</text--->"
  "<text>get the stream of units of the moving value.</text--->"
  "<text>units(mgpoint)</text--->"
  ") )";
