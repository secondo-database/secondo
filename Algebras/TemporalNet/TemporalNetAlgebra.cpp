/*
----
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
----

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of Module

May 2007 Martin Scheppokat

[TOC]

1 Overview

2 Defines, includes, and constants

*/
#include "NestedList.h"
#include "TupleIdentifier.h"
#include "DBArray.h"

#include "GPoint.h"
#include "NetworkAlgebra.h"
#include "TemporalAlgebra.h"
#include "TemporalNetAlgebra.h"
#include "OpMPoint2MGPoint.h"

#include <iostream>
#include <sstream>
#include <string>
#include "QueryProcessor.h"
#include "Algebra.h"

#include "DateTime.h"
#include "UGPoint.h"
#include "MGPoint.h"

extern NestedList* nl;
extern QueryProcessor* qp;


/*
4.9.12 Creation of the type constructor ~upoint~

*/
TypeConstructor unitgpoint(
        "ugpoint",                              // Name
        UGPoint::Property,                      // Property function
        UGPoint::Out, UGPoint::In,              // Out and In functions
        0,             0,                       // Save to and restore
                                                // from list functions
        UGPoint::Create,                        // Object creation 
        UGPoint::Delete,                        // and deletion
        0,             0,                       // Object open and save
        UGPoint::Close, UGPoint::Clone,         // Object close and clone
        UGPoint::Cast,                          // Cast function
        UGPoint::SizeOf,                        // Sizeof function
        UGPoint::Check);                        // Kind checking function


/*
4.12 Type Constructor ~mgpoint~


4.12.2 function Describing the Signature of the Type Constructor

*/

/*
4.12.4 Creation of the type constructor ~mpoint~

*/
TypeConstructor movinggpoint(
        "mgpoint",                                  // Name
        MGPoint::Property,                          // Property function 
        OutMapping<MGPoint, UGPoint, UGPoint::Out>,
        InMapping<MGPoint, UGPoint, UGPoint::In>,   // Out and In functions
        0,                                          // SaveToList and 
        0,                                          // RestoreFromList
        CreateMapping<MGPoint>,                     // Object creation and 
        DeleteMapping<MGPoint>,                     // deletion
        0,                                          // Object open and save
        0,      
        CloseMapping<MGPoint>,                      // Object close and clone
        CloneMapping<MGPoint>, 
        CastMapping<MGPoint>,                       // Cast function
        SizeOfMapping<MGPoint>,                     // Sizeof function
        MGPoint::Check);                            // Kind checking function


/*
4.4.4 Definition 

*/
Operator mpoint2mgpoint (
          "mpoint2mgpoint",                // name
          OpMPoint2MGPoint::Spec,          // specification
          OpMPoint2MGPoint::ValueMapping,  // value mapping
          Operator::SimpleSelect,          // trivial selection function
          OpMPoint2MGPoint::TypeMap        // type mapping
);




/*
6 Creating the Algebra

*/

class TemporalNetAlgebra : public Algebra
{
  public:
  
  TemporalNetAlgebra() : Algebra()
  {
    AddTypeConstructor( &unitgpoint );
    AddTypeConstructor( &movinggpoint );

    movinggpoint.AssociateKind( "TEMPORAL" );
    movinggpoint.AssociateKind( "DATA" );

    AddOperator(&mpoint2mgpoint);

  }


  ~TemporalNetAlgebra() {};
};

TemporalNetAlgebra temporalNetAlgebra;

/*
7 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C" Algebra* InitializeTemporalNetAlgebra( NestedList* in_pNL, 
                                                  QueryProcessor* in_pQP )
{
  nl = in_pNL;
  qp = in_pQP;
  return (&temporalNetAlgebra);
}
