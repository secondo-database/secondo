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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Source File of the Spatiotemporal Pattern Algebra

August, 2009 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
queries moving objects with transportation modes.

2 Defines and includes

*/

#include "TransportationMode.h"
#include "BusNetwork.h"

extern NestedList* nl;
extern QueryProcessor *qp;

namespace TransportationMode{


/*data type for bus network*/
TypeConstructor busnetwork( "busnetwork", BusNetwork::BusNetworkProp,
  BusNetwork::OutBusNetwork, BusNetwork::InBusNetwork,
  0,0,
  BusNetwork::CreateBusNetwork, BusNetwork::DeleteBusNetwork,
  BusNetwork::OpenBusNetwork, BusNetwork::SaveBusNetwork,
  BusNetwork::CloseBusNetwork, BusNetwork::CloneBusNetwork,
  BusNetwork::CastBusNetwork, BusNetwork::SizeOfBusNetwork,
  BusNetwork::CheckBusNetwork);



const string OpTheBusNetworkSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>int x rel -> busnetwork" "</text--->"
  "<text>thebusnetwork(_, _)</text--->"
  "<text>Creates busnetwork with id and a relation.</text--->"
  "<text>let busnet = thebusnetwork(int, busroutes)</text--->"
  "))";

int OpTheBusNetworkValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  BusNetwork* bus = (BusNetwork*)qp->ResultStorage(s).addr;
  CcInt* pId = (CcInt*)args[0].addr;
  int iId = pId->GetIntval();
  Relation* BusRoutes = (Relation*)args[1].addr;
  bus->Load(iId,BusRoutes);
  result = SetWord(bus);
  return 0;
}
/*
6.11 Operator ~thebusnetwork~

Creates a bus network with the given id, from the given bus routes relations.

*/
ListExpr OpTheBusNetworkTypeMap(ListExpr in_xArgs)
{
  if(nl->ListLength(in_xArgs) != 2)
    return (nl->SymbolAtom("typeerror"));

  ListExpr xIdDesc = nl->First(in_xArgs);
  ListExpr xRoutesRelDesc = nl->Second(in_xArgs);

  if(!nl->IsEqual(xIdDesc, "int"))
  {
    return (nl->SymbolAtom("typeerror"));
  }

  if(!IsRelDescription(xRoutesRelDesc))
  {
    return (nl->SymbolAtom("typeerror"));
  }

  ListExpr xType;
  nl->ReadFromString(BusNetwork::busrouteTypeInfo, xType);
  if(!CompareSchemas(xRoutesRelDesc, xType))
  {
    return (nl->SymbolAtom("typeerror"));
  }
  return nl->SymbolAtom("busnetwork");
}

Operator thebusnetwork(
  "thebusnetwork", //name
  OpTheBusNetworkSpec,
  OpTheBusNetworkValueMapping,
  Operator::SimpleSelect,
  OpTheBusNetworkTypeMap
);

/*
Main Class for Transportation Mode

*/
class TransportationModeAlgebra : public Algebra
{
 public:
  TransportationModeAlgebra() : Algebra()
  {
   AddTypeConstructor(&busnetwork);
  //can't be stored as an attribute in a relation
   busnetwork.AssociateKind("BUSNETWORK");


  AddOperator(&thebusnetwork);//construct bus network
  //AddOperator(&busstop);
  }
  ~TransportationModeAlgebra() {};
 private:
  BusNetwork* busnet;
};

};


extern "C"
Algebra* InitializeTransportationModeAlgebra( NestedList* nlRef,
    QueryProcessor* qpRef )
    {
    nl = nlRef;
    qp = qpRef;
  // The C++ scope-operator :: must be used to qualify the full name
  return new TransportationMode::TransportationModeAlgebra();
    }
