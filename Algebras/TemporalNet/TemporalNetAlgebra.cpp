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

1.1 Implementation of Algebra TemporalNet

May 2007 Martin Scheppokat

March 2008 Simone Jandt Added type constructor igpoint and operators: length,
initial, atinstant and final.

Defines, includes, and constants

*/
#include "NestedList.h"
#include "TupleIdentifier.h"
#include "DBArray.h"

#include "GPoint.h"
#include "NetworkAlgebra.h"
#include "TemporalAlgebra.h"
#include "TemporalNetAlgebra.h"
#include "OpMPoint2MGPoint.h"
#include "OpUnits.h"
#include "OpSimplify.h"
#include "OpPasses.h"
#include "OpTrajectory.h"
#include "OpTempNetLength.h"
#include "OpTempNetAtinstant.h"
#include "OpTempNetAtperiods.h"
#include "OpTempNetInitial.h"
#include "OpTempNetFinal.h"
#include "OpTempNetAt.h"
#include "OpTempNetVal.h"
#include "OpTempNetInst.h"
#include "OpTempNetPresent.h"
#include "OpTempNetIsEmpty.h"
#include "OpTempNetNoComp.h"
#include "OpTempNetInside.h"
#include "OpTempNetIntersection.h"
#include "OpTempNetDeftime.h"

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
Intime GPoint Property

*/
ListExpr
IntimeGPointProperty()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> TEMPORAL"),
                             nl->StringAtom("(igpoint) "),
                             nl->StringAtom("(instant gpoint-value)"),
                             nl->StringAtom("((instant) (1 1 1.0 2))"))));
}

/*
Kind checking Function for ~igpoint~

*/
bool
CheckIntimeGPoint( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "igpoint" ));
}

/*
Creation of the type constructor  ~igpoint~

*/
TypeConstructor intimegpoint(
        "igpoint",                    //name
        IntimeGPointProperty,  //property function describing signature
        OutIntime<GPoint, GPoint::OutGPoint>,
        InIntime<GPoint, GPoint::InGPoint>,         //Out and In functions
        0,
        0,       //SaveToList and RestoreFromList functions
        CreateIntime<GPoint>,
        DeleteIntime<GPoint>,              //object creation and deletion
        0,
        0,                           // object open and save
        CloseIntime<GPoint>,
        CloneIntime<GPoint>,               //object close and clone
        CastIntime<GPoint>,                //cast function
        SizeOfIntime<GPoint>,              //sizeof function
        CheckIntimeGPoint );               //kind checking function

/*
Creation of the type constructor ~upoint~

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
Creation of the type constructor ~mpoint~

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
Operator mpoint2mgpoint

*/
Operator mpoint2mgpoint (
          "mpoint2mgpoint",                // name
          OpMPoint2MGPoint::Spec,          // specification
          OpMPoint2MGPoint::ValueMapping,  // value mapping
          Operator::SimpleSelect,          // trivial selection function
          OpMPoint2MGPoint::TypeMap        // type mapping
);

/*
Operator units

*/
Operator units("units",
               OpUnits::Spec,
               MappingUnits<MGPoint, UGPoint>,
               Operator::SimpleSelect,
               OpUnits::TypeMap );

/*
Operator simplify

*/
Operator simplify("simplify",
                  OpSimplify::Spec,
                  OpSimplify::ValueMapping,
                  Operator::SimpleSelect,
                  OpSimplify::TypeMap );

/*
Operator passes

*/

Operator tempnetpasses (
  "passes",
  OpPasses::Spec,
  2,
  OpPasses::passesmap,
  OpPasses::SelectPasses,
  OpPasses::PassesMap );

/*
Operator at

*/
Operator tempnetat("at",
                OpTempNetAt::Spec,
                2,
                OpTempNetAt::atmap,
                OpTempNetAt::SelectAt,
                OpTempNetAt::AtMap );
/*
Operator trajectory

*/
Operator trajectory("trajectory",
                    OpTrajectory::Spec,
                    OpTrajectory::ValueMapping,
                    Operator::SimpleSelect,
                    OpTrajectory::TypeMap );

/*
Operator length

*/
Operator tempnetlength("length",
                    OpTempNetLength::Spec,
                    OpTempNetLength::ValueMapping,
                    Operator::SimpleSelect,
                    OpTempNetLength::TypeMap );

/*
Operator atinstant

*/
Operator tempnetatinstant("atinstant",
                OpTempNetAtinstant::Spec,
                MappingAtInstant<MGPoint, GPoint>,
                Operator::SimpleSelect,
                OpTempNetAtinstant::TypeMap );

/*
Operator initial

*/
Operator tempnetinitial("initial",
                OpTempNetInitial::Spec,
                MappingInitial<MGPoint, UGPoint, GPoint>,
                Operator::SimpleSelect,
                OpTempNetInitial::TypeMap );

/*
Operator final

*/
Operator tempnetfinal("final",
                OpTempNetFinal::Spec,
                MappingFinal<MGPoint,UGPoint, GPoint>,
                Operator::SimpleSelect,
                OpTempNetFinal::TypeMap );

/*

Operator val

*/
Operator tempnetval("val",
                OpTempNetVal::Spec,
                IntimeVal<GPoint>,
                Operator::SimpleSelect,
                OpTempNetVal::TypeMap );

/*

Operator inst

*/
Operator tempnetinst("inst",
                OpTempNetInst::Spec,
                IntimeInst<GPoint>,
                Operator::SimpleSelect,
                OpTempNetInst::TypeMap );


/*
Operator atinstant

*/
Operator tempnetatperiods("atperiods",
                OpTempNetAtperiods::Spec,
                OpTempNetAtperiods::ValueMapping,
                Operator::SimpleSelect,
                OpTempNetAtperiods::TypeMap );

/*
Operator present

*/
Operator tempnetpresent(
                "present",
                OpTempNetPresent::Spec,
                2,
                OpTempNetPresent::presentmap,
                OpTempNetPresent::SelectPresent,
                OpTempNetPresent::PresentMap );

/*
Operator isempty

*/
Operator tempnetisempty("isempty",
                OpTempNetIsEmpty::Spec,
                OpTempNetIsEmpty::ValueMapping,
                Operator::SimpleSelect,
                OpTempNetIsEmpty::TypeMap );

/*
Operator nocomponents

*/
Operator tempnetnocomp("no_components",
                OpTempNetNoComp::Spec,
                OpTempNetNoComp::ValueMapping,
                Operator::SimpleSelect,
                OpTempNetNoComp::TypeMap );

/*
Operator inside

*/
Operator tempnetinside("inside",
                OpTempNetInside::Spec,
                OpTempNetInside::ValueMapping,
                Operator::SimpleSelect,
                OpTempNetInside::TypeMapping );

/*
Operator intersection

*/
Operator tempnetintersection("intersection",
                OpTempNetIntersection::Spec,
                OpTempNetIntersection::ValueMapping,
                Operator::SimpleSelect,
                OpTempNetIntersection::TypeMapping );

/*
Operator deftime

*/
Operator tempnetdeftime("deftime",
                OpTempNetDeftime::Spec,
                OpTempNetDeftime::ValueMapping,
                Operator::SimpleSelect,
                OpTempNetDeftime::TypeMap );

/*
Creating the Algebra

*/

class TemporalNetAlgebra : public Algebra
{
  public:

  TemporalNetAlgebra() : Algebra()
  {
    AddTypeConstructor( &unitgpoint );
    AddTypeConstructor( &movinggpoint );
    AddTypeConstructor( &intimegpoint);

    movinggpoint.AssociateKind( "TEMPORAL" );
    movinggpoint.AssociateKind( "DATA" );
    unitgpoint.AssociateKind( "DATA" );
    intimegpoint.AssociateKind("TEMPORAL");
    intimegpoint.AssociateKind("DATA");

    AddOperator(&mpoint2mgpoint);
    AddOperator(&units);
    AddOperator(&simplify);
    AddOperator(&tempnetpasses);
    AddOperator(&trajectory);
    AddOperator(&tempnetlength);
    AddOperator(&tempnetatinstant);
    AddOperator(&tempnetinitial);
    AddOperator(&tempnetfinal);
    AddOperator(&tempnetat);
    AddOperator(&tempnetval);
    AddOperator(&tempnetinst);
    AddOperator(&tempnetatperiods);
    AddOperator(&tempnetpresent);
    AddOperator(&tempnetisempty);
    AddOperator(&tempnetnocomp);
    AddOperator(&tempnetinside);
    AddOperator(&tempnetintersection);
    AddOperator(&tempnetdeftime);
  }


  ~TemporalNetAlgebra() {};
};

TemporalNetAlgebra temporalNetAlgebra;

/*
Initialization

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
