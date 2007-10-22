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

1.1 Implementation of MGPoint

A class representing a moving point in a network.

Mai-Oktober 2007 Martin Scheppokat


Defines, includes, and constants

*/
#include "TemporalAlgebra.h"
#include "TupleIdentifier.h"
#include "GPoint.h"
#include "UGPoint.h"
#include "MGPoint.h"

/*
The simple constructor. This constructor should not be used.

*/
MGPoint::MGPoint()
{
}
    
/*
The constructor. Initializes space for ~n~ elements.

*/
MGPoint::MGPoint( const int n ) : Mapping< UGPoint, GPoint >( n )
{
}



ListExpr MGPoint::Property()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> MAPPING"),
                             nl->StringAtom("(mgpoint) "),
                             nl->StringAtom("( u1 ... un ) "),
                             nl->StringAtom("(((i1 i2 TRUE FALSE) " 
                                            "(1 1 0 0.1 2.4)) ...)"))));
}

/*
Kind Checking Function

*/
bool MGPoint::Check(ListExpr type, 
                    ListExpr& errorInfo)
{
  return (nl->IsEqual( type, "mgpoint" ));
}
