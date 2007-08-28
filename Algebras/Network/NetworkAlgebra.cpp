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

[1] Implementation of Module Network Algebra

March 2004 Victor Almeida

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

[TOC]

1 Overview


This file contains the implementation of the type constructors ~network~,
~gpoint~, and ~gline~ and the temporal corresponding ~moving~(~gpoint~)
and ~moving~(~gline~).

2 Defines, includes, and constants

*/

/*
3.2 Type property of type constructor ~network~

*/

#include <sstream>

#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"

#include "GPoint.h"
#include "GLine.h"
#include "SpatialAlgebra.h"
#include "Network.h"

#include "NetworkAlgebra.h"

#include "Algebra.h"
#include "StandardTypes.h"

#include "OpNetworkJunctions.h"
#include "OpNetworkTheNetwork.h"
#include "OpNetworkRoutes.h"
#include "OpNetworkSections.h"
#include "OpShortestPath.h"

extern NestedList* nl;
extern QueryProcessor* qp;



/*
3.14 Type Constructor object for type constructor ~network~

*/
TypeConstructor network( "network",          Network::NetworkProp,
                         Network::OutNetwork,           Network::InNetwork,
                         0,                    0,
                         Network::CreateNetwork,        Network::DeleteNetwork,
                         Network::OpenNetwork,          Network::SaveNetwork,
                         Network::CloseNetwork,         Network::CloneNetwork,
                         Network::CastNetwork,          Network::SizeOfNetwork,
                         Network::CheckNetwork );

/*
4 Type Constructor ~gpoint~

4.1 List Representation

The list representation of a graph point is

----    (nid rid pos side)
----

3.3 ~Out~-function

*/

/*
3.12 Creation of the type constructor instance

*/
TypeConstructor gpoint(
        "gpoint",                                   //name
        GPoint::GPointProperty,                     //property function
        GPoint::OutGPoint, GPoint::InGPoint,        //Out and In functions
        0,                   0,                     //SaveToList and
                                                    //RestoreFromList functions
        GPoint::CreateGPoint, GPoint::DeleteGPoint, //object creation/deletion
        0,                   0,                     //open and save functions
        GPoint::CloseGPoint, GPoint::CloneGPoint,   //object close, and clone
        GPoint::CastGPoint,                         //cast function
        GPoint::SizeOfGPoint,                       //sizeof function
        GPoint::CheckGPoint );                      //kind checking function

/*
3.12 Creation of the type constructor instance

*/
TypeConstructor gline(
        "gline",                       //name
        GLine::Property,               //property function
        GLine::Out, GLine::In,         //Out and In functions
        0, 0,                          //SaveToList and
                                       //RestoreFromList functions
        GLine::Create, GLine::Delete,  //object creation and deletion
        0, 0,                          //open and save functions
        GLine::Close, GLine::Clone,    //object close, and clone
        GLine::Cast,                   //cast function
        GLine::SizeOf,                 //sizeof function
        GLine::Check);                 //kind checking function


/*
4 Operators

Definition of operators is similar to definition of type constructors. An
operator is defined by creating an instance of class ~Operator~. Again we
have to define some functions before we are able to create an ~Operator~
instance.

4.1 Operator ~thenetwork~

4.1.1 Type Mapping of operator ~thenetwork~

*/

/*
4.1.4 Definition of operator ~thenetwork~

*/
Operator networkthenetwork (
          "thenetwork",                // name
          OpNetworkTheNetwork::Spec,              // specification
          OpNetworkTheNetwork::ValueMapping,      // value mapping
          Operator::SimpleSelect,               // trivial selection function
          OpNetworkTheNetwork::TypeMap            // type mapping
);


/*
4.2.4 Definition of operator ~routes~

*/
Operator networkroutes (
          "routes",                // name
          OpNetworkRoutes::Spec,              // specification
          OpNetworkRoutes::ValueMapping,      // value mapping
          Operator::SimpleSelect,               // trivial selection function
          OpNetworkRoutes::TypeMap            // type mapping
);


/*
4.3.4 Definition of operator ~junctions~

*/
Operator networkjunctions (
          "junctions",                // name
          OpNetworkJunctions::Spec,          // specification
          OpNetworkJunctions::ValueMapping,  // value mapping
          Operator::SimpleSelect,            // trivial selection function
          OpNetworkJunctions::TypeMap        // type mapping
);


/*
4.4.4 Definition of operator ~sections~

*/
Operator networksections (
          "sections",                       // name
          OpNetworkSections::Spec,          // specification
          OpNetworkSections::ValueMapping,  // value mapping
          Operator::SimpleSelect,           // trivial selection function
          OpNetworkSections::TypeMap        // type mapping
);

/*
4.4.4 Definition 

*/
Operator shortest_path (
          "shortest_path",               // name
          OpShortestPath::Spec,          // specification
          OpShortestPath::ValueMapping,  // value mapping
          Operator::SimpleSelect,        // trivial selection function
          OpShortestPath::TypeMap        // type mapping
);


/*
5 Creating the Algebra

*/

class NetworkAlgebra : public Algebra
{
 public:
  NetworkAlgebra() : Algebra()
  {
    AddTypeConstructor( &network );
    AddTypeConstructor( &gpoint );
    AddTypeConstructor( &gline );

    AddOperator(&networkthenetwork);
    AddOperator(&networkroutes);
    AddOperator(&networkjunctions);
    AddOperator(&networksections);
    AddOperator(&shortest_path);
  }
  ~NetworkAlgebra() {};
};

NetworkAlgebra networkAlgebra;

/*
6 Initialization

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

extern "C"
Algebra*
InitializeNetworkAlgebra( NestedList* nlRef, 
                          QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;

  return (&networkAlgebra);
}


