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

[1] Source File of the Transportation Mode Algebra

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


/************string description about bus network operators*************/
const string OpTheBusNetworkSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>int x rel -> busnetwork" "</text--->"
  "<text>thebusnetwork(_, _)</text--->"
  "<text>Creates busnetwork with id and a relation.</text--->"
  "<text>let busnet = thebusnetwork(1, busroutes)</text--->"
  "))";

const string OpBusStopSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>rel -> a stream of tuple((t1)(t2)...(tn))" "</text--->"
  "<text>busstop(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to a bus stop."
  "</text--->"
  "<text>query busstop(busroutes) count</text--->"
  "))";

const string OpBusLineSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>rel -> a stream of tuple((t1)(t2)...(tn))" "</text--->"
  "<text>busline(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to"
  "the trajectoryof a bus's movement.</text--->"
  "<text>query busline(busroutes) count</text--->"
  "))";

/***********Value Map Function for Bus Network****************************/
/*
Creates a bus network with the given id, from the given bus routes relations.

*/
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


struct DisplBus{
  BusNetwork* busnet;
  Relation* busstop;
  Relation* busweight;
  int no_weight;
  int bus_weight_count;
  int no_stop;
  int bus_stop_count;
  TupleType* resulttype;
  DisplBus(BusNetwork* p):busnet(p){
    busstop = busnet->GetRelBus_Node();
    busweight = busnet->GetRelBus_Weight();
    no_stop = busstop->GetNoTuples();
    no_weight = busweight->GetNoTuples();
    bus_stop_count = 1;
    bus_weight_count = 1;
    resulttype = NULL;
  }
  ~DisplBus(){
    if(resulttype != NULL)
      delete resulttype;
  }
};
/*
Return all bus stops.

*/
int OpBusStopValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  DisplBus* localInfo;
  switch(message){
    case OPEN:{
        localInfo = new DisplBus((BusNetwork*)args[0].addr);
        localInfo->resulttype =
              new TupleType(nl->Second(GetTupleResultType(s)));
        local = SetWord(localInfo);
        return 0;
    }
    case REQUEST:{
        localInfo = (DisplBus*)local.addr;
        if(localInfo->bus_stop_count > localInfo->no_stop)
          return CANCEL;
        Tuple* tuple = new Tuple(localInfo->resulttype);
        Tuple* temp_tuple =
                localInfo->busstop->GetTuple(localInfo->bus_stop_count);
        CcInt* id = (CcInt*)temp_tuple->GetAttribute(BusNetwork::SID);
        Point* location = (Point*)temp_tuple->GetAttribute(BusNetwork::LOC);
        tuple->PutAttribute(BusNetwork::SID,new CcInt(*id));
        tuple->PutAttribute(BusNetwork::LOC,new Point(*location));
        result.setAddr(tuple);
        temp_tuple->DeleteIfAllowed();
        localInfo->bus_stop_count++;
        return YIELD;
    }
    case CLOSE:{
        localInfo = (DisplBus*)local.addr;
        delete localInfo;
        return 0;
    }
  }
  return 0;
}
/*
Display the trajectory of bus movement.

*/
int OpBusLineValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  DisplBus* localInfo;
  switch(message){
    case OPEN:{
        localInfo = new DisplBus((BusNetwork*)args[0].addr);
        localInfo->resulttype =
              new TupleType(nl->Second(GetTupleResultType(s)));
        local = SetWord(localInfo);
        return 0;
    }
    case REQUEST:{
        localInfo = (DisplBus*)local.addr;
        if(localInfo->bus_weight_count > localInfo->no_weight)
          return CANCEL;
        Tuple* tuple = new Tuple(localInfo->resulttype);
        Tuple* temp_tuple =
                localInfo->busweight->GetTuple(localInfo->bus_weight_count);
        CcInt* id = (CcInt*)temp_tuple->GetAttribute(BusNetwork::FID);
        Line* l = (Line*)temp_tuple->GetAttribute(BusNetwork::LINE);
        CcReal* fee = (CcReal*)temp_tuple->GetAttribute(BusNetwork::FEE);
        tuple->PutAttribute(0,new CcInt(*id));
        tuple->PutAttribute(1,new Line(*l));
        tuple->PutAttribute(2,new CcReal(*fee));
        result.setAddr(tuple);
        temp_tuple->DeleteIfAllowed();
        localInfo->bus_weight_count++;
        return YIELD;
    }
    case CLOSE:{
        localInfo = (DisplBus*)local.addr;
        delete localInfo;
        return 0;
    }
  }
  return 0;
}

/*
Operator ~thebusnetwork~

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

/*
Operator ~busstop~

*/
ListExpr OpBusStopTypeMap(ListExpr in_xArgs)
{
  if(nl->ListLength(in_xArgs) != 1)
    return (nl->SymbolAtom("typeerror"));

  ListExpr arg = nl->First(in_xArgs);
  if(nl->IsAtom(arg) && nl->AtomType(arg) == SymbolType &&
     nl->SymbolValue(arg) == "busnetwork"){
    return nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),
              nl->TwoElemList(
                nl->TwoElemList(nl->SymbolAtom("id"),nl->SymbolAtom("int")),
                nl->TwoElemList(nl->SymbolAtom("loc"),nl->SymbolAtom("point"))
              )
            )
          );
  }
  return nl->SymbolAtom("typeerror");

}
/*
Operator ~busline~

*/
ListExpr OpBusLineTypeMap(ListExpr in_xArgs)
{
  if(nl->ListLength(in_xArgs) != 1)
    return (nl->SymbolAtom("typeerror"));

  ListExpr arg = nl->First(in_xArgs);
  if(nl->IsAtom(arg) && nl->AtomType(arg) == SymbolType &&
     nl->SymbolValue(arg) == "busnetwork"){
    return nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),
              nl->ThreeElemList(
                nl->TwoElemList(nl->SymbolAtom("id"),nl->SymbolAtom("int")),
                nl->TwoElemList(nl->SymbolAtom("l"),nl->SymbolAtom("line")),
                nl->TwoElemList(nl->SymbolAtom("fee"),nl->SymbolAtom("real"))
              )
            )
          );
  }
  return nl->SymbolAtom("typeerror");

}

/*****************Operators for Bus Network*************************/
Operator thebusnetwork(
  "thebusnetwork", //name
  OpTheBusNetworkSpec,
  OpTheBusNetworkValueMapping,
  Operator::SimpleSelect,
  OpTheBusNetworkTypeMap
);

Operator busstop(
  "busstop", //name
  OpBusStopSpec,
  OpBusStopValueMapping,
  Operator::SimpleSelect,
  OpBusStopTypeMap
);

Operator busline(
  "busline", //name
  OpBusLineSpec,
  OpBusLineValueMapping,
  Operator::SimpleSelect,
  OpBusLineTypeMap
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
   AddOperator(&busstop);//display bus stop
   AddOperator(&busline); //display the trajectory of a bus
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
