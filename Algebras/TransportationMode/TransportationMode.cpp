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
///////////////////////////////////////////////////////////////////////////
/////////////           Bus Network            ///////////////////////////
///////////////////////////////////////////////////////////////////////////

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

const string OpBusNodeSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>rel -> a stream of tuple((t1)(t2)...(tn))" "</text--->"
  "<text>busnode(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to a bus stop."
  "</text--->"
  "<text>query busnode(busroutes) count</text--->"
  "))";

const string OpBusEdgeSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>rel -> a stream of tuple((t1)(t2)...(tn))" "</text--->"
  "<text>busedge(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to"
  "the trajectory of a bus's movement.</text--->"
  "<text>query busedge(busroutes) count</text--->"
  "))";
const string OpBusMoveSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>rel -> a stream of tuple((t1)(t2)...(tn))" "</text--->"
  "<text>busmove(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to"
  "the movement of bus.</text--->"
  "<text>query busmove(busroutes) count</text--->"
  "))";

const string OpBusFindPath_T_1Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>rel -> a stream of tuple((t1)(t2)...(tn))" "</text--->"
  "<text>find_path_t_1(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to"
  "the sequence movement of a trip.</text--->"
  "<text>query find_path_t_1(mint1) count</text--->"
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
/*structure for displaying information about bus network*/
struct DisplBus{
  BusNetwork* busnet;
  TupleType* resulttype;
  DisplBus(BusNetwork* p):busnet(p){
    resulttype = NULL;
  }
  ~DisplBus(){
    if(resulttype != NULL)
      delete resulttype;
  }
};
/*structure for displaying bus stop*/
struct DisplBusStop:public DisplBus{
  int no_stop;
  int bus_stop_count;
  Relation* busstop;
  DisplBusStop(BusNetwork* p):DisplBus(p){
    busstop = busnet->GetRelBus_Node();
    no_stop = busstop->GetNoTuples();
    bus_stop_count = 1;
  }

};
/*structure for displaying bus edge*/
struct DisplBusEdge:public DisplBus{
  int no_edge;
  int bus_edge_count;
  Relation* bus_edge;
  DisplBusEdge(BusNetwork* p):DisplBus(p){
    bus_edge = busnet->GetRelBus_Edge();
    assert(bus_edge != NULL);
    no_edge = bus_edge->GetNoTuples();
    bus_edge_count = 1;
  }
};

/*structure for displaying bus route*/
struct DisplBusRoute:public DisplBus{
  int no_route;
  int bus_route_count;
  Relation* bus_route;
  DisplBusRoute(BusNetwork* p):DisplBus(p){
    bus_route = busnet->GetRelBus_Route();
    assert(bus_route != NULL);
    no_route = bus_route->GetNoTuples();
    bus_route_count = 1;
  }
};

/*structure for opeator reachability*/
struct DisplReach:public DisplBus{
  MPoint* route;
  DisplReach(BusNetwork* p):DisplBus(p){
  }
};

/*
Return all bus stops.

*/
int OpBusNodeValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  DisplBusStop* localInfo;
  switch(message){
    case OPEN:{
        localInfo = new DisplBusStop((BusNetwork*)args[0].addr);
        localInfo->resulttype =
              new TupleType(nl->Second(GetTupleResultType(s)));
        local = SetWord(localInfo);
        return 0;
    }
    case REQUEST:{
        localInfo = (DisplBusStop*)local.addr;
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
        localInfo = (DisplBusStop*)local.addr;
        delete localInfo;
        return 0;
    }
  }
  return 0;
}
/*
Display the trajectory (edge-weight) of bus movement.

*/
int OpBusEdgeValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  DisplBusEdge* localInfo;
  switch(message){
    case OPEN:{
        localInfo = new DisplBusEdge((BusNetwork*)args[0].addr);
        localInfo->resulttype =
              new TupleType(nl->Second(GetTupleResultType(s)));
        local = SetWord(localInfo);
        return 0;
    }
    case REQUEST:{
        localInfo = (DisplBusEdge*)local.addr;
        if(localInfo->bus_edge_count > localInfo->no_edge)
          return CANCEL;
        Tuple* tuple = new Tuple(localInfo->resulttype);
        Tuple* temp_tuple =
                localInfo->bus_edge->GetTuple(localInfo->bus_edge_count);
        CcInt* id = (CcInt*)temp_tuple->GetAttribute(BusNetwork::EID);
        CcInt* nid1 = (CcInt*)temp_tuple->GetAttribute(BusNetwork::V1);
        CcInt* nid2 = (CcInt*)temp_tuple->GetAttribute(BusNetwork::V2);
        Line* l = (Line*)temp_tuple->GetAttribute(BusNetwork::LINE);
        CcReal* fee = (CcReal*)temp_tuple->GetAttribute(BusNetwork::FEE);

        Periods* peri = (Periods*)temp_tuple->GetAttribute(BusNetwork::DEF_T);
        const Interval<Instant>* interval;
        peri->Get(0,interval);
        string start_t = interval->start.ToString();
        string end_t = interval->end.ToString();
//        cout<<start_t<<" "<<end_t<<endl;

        tuple->PutAttribute(0,new CcInt(*id));
        tuple->PutAttribute(1,new CcInt(*nid1));
        tuple->PutAttribute(2,new CcInt(*nid2));
        tuple->PutAttribute(3,new Line(*l));
        tuple->PutAttribute(4,new CcReal(*fee));
        tuple->PutAttribute(5,new CcString(true,start_t));
        tuple->PutAttribute(6,new CcString(true,end_t));
        result.setAddr(tuple);
        temp_tuple->DeleteIfAllowed();
        localInfo->bus_edge_count++;
        return YIELD;
    }
    case CLOSE:{
        localInfo = (DisplBusEdge*)local.addr;
        delete localInfo;
        return 0;
    }
  }
  return 0;
}

/*
Display the bus movement.

*/
int OpBusMoveValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  DisplBusRoute* localInfo;
  switch(message){
    case OPEN:{
        localInfo = new DisplBusRoute((BusNetwork*)args[0].addr);
        localInfo->resulttype =
              new TupleType(nl->Second(GetTupleResultType(s)));
        local = SetWord(localInfo);
        return 0;
    }
    case REQUEST:{
        localInfo = (DisplBusRoute*)local.addr;
        if(localInfo->bus_route_count > localInfo->no_route)
          return CANCEL;
        Tuple* tuple = new Tuple(localInfo->resulttype);
        Tuple* temp_tuple =
                localInfo->bus_route->GetTuple(localInfo->bus_route_count);
        CcInt* id = (CcInt*)temp_tuple->GetAttribute(BusNetwork::RID);
        MPoint* mp = (MPoint*)temp_tuple->GetAttribute(BusNetwork::TRIP);
        tuple->PutAttribute(0,new CcInt(*id));
        tuple->PutAttribute(1,new MPoint(*mp));
        result.setAddr(tuple);
        temp_tuple->DeleteIfAllowed();
        localInfo->bus_route_count++;
        return YIELD;
    }
    case CLOSE:{
        localInfo = (DisplBusRoute*)local.addr;
        delete localInfo;
        return 0;
    }
  }
  return 0;
}

/*
query reachability for a start node and an end node.

*/
int OpBusFindPath_T_1ValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BusNetwork* busnet = (BusNetwork*)args[0].addr;
  MInt* querycond = (MInt*)args[1].addr;
  busnet->FindPath_T_1((MPoint*)result.addr,querycond);
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
Operator ~busnode~

*/
ListExpr OpBusNodeTypeMap(ListExpr in_xArgs)
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
Operator ~busedge~

*/
ListExpr OpBusEdgeTypeMap(ListExpr in_xArgs)
{
  if(nl->ListLength(in_xArgs) != 1)
    return (nl->SymbolAtom("typeerror"));

  ListExpr arg = nl->First(in_xArgs);
  if(nl->IsAtom(arg) && nl->AtomType(arg) == SymbolType &&
     nl->SymbolValue(arg) == "busnetwork"){
      return nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),
              nl->Cons(
                nl->TwoElemList(nl->SymbolAtom("id"),nl->SymbolAtom("int")),
                nl->SixElemList(
                nl->TwoElemList(nl->SymbolAtom("nid1"),nl->SymbolAtom("int")),
                nl->TwoElemList(nl->SymbolAtom("nid2"),nl->SymbolAtom("int")),
                nl->TwoElemList(nl->SymbolAtom("l"),nl->SymbolAtom("line")),
                nl->TwoElemList(nl->SymbolAtom("fee"),nl->SymbolAtom("real")),
                nl->TwoElemList(nl->SymbolAtom("t1"),nl->SymbolAtom("string")),
                nl->TwoElemList(nl->SymbolAtom("t2"),nl->SymbolAtom("string"))
              ))
            )
          );
  }
  return nl->SymbolAtom("typeerror");

}

/*
Operator ~busmove~

*/
ListExpr OpBusMoveTypeMap(ListExpr in_xArgs)
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
                nl->TwoElemList(nl->SymbolAtom("trip"),nl->SymbolAtom("mpoint"))
              )
            )
          );
  }
  return nl->SymbolAtom("typeerror");

}

/*
Operator ~reachability~

*/
ListExpr OpBusFindPath_T_1TypeMap(ListExpr in_xArgs)
{
  string err = "busnetwork x mint expected";
  if(nl->ListLength(in_xArgs) != 2){
      ErrorReporter::ReportError(err);
      return nl->TypeError();
  }

  ListExpr arg1 = nl->First(in_xArgs);
  ListExpr arg2 = nl->Second(in_xArgs);
  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "busnetwork" && nl->IsEqual(arg2,"mint"))
    return nl->SymbolAtom("mpoint");

  ErrorReporter::ReportError(err);
  return nl->TypeError();

}
/*****************Operators for Bus Network*************************/
Operator thebusnetwork(
  "thebusnetwork", //name
  OpTheBusNetworkSpec,
  OpTheBusNetworkValueMapping,
  Operator::SimpleSelect,
  OpTheBusNetworkTypeMap
);

Operator busnode(
  "busnode", //name
  OpBusNodeSpec,
  OpBusNodeValueMapping,
  Operator::SimpleSelect,
  OpBusNodeTypeMap
);

Operator busedge(
  "busedge", //name
  OpBusEdgeSpec,
  OpBusEdgeValueMapping,
  Operator::SimpleSelect,
  OpBusEdgeTypeMap
);

Operator busmove(
  "busmove", //name
  OpBusMoveSpec,
  OpBusMoveValueMapping,
  Operator::SimpleSelect,
  OpBusMoveTypeMap
);

Operator find_path_t_1(
  "find_path_t_1", //name
  OpBusFindPath_T_1Spec,
  OpBusFindPath_T_1ValueMapping,
  Operator::SimpleSelect,
  OpBusFindPath_T_1TypeMap
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
   AddOperator(&busnode);//display bus stop
   AddOperator(&busedge); //display the trajectory of a bus
   AddOperator(&busmove);//display bus movement
   //middle stop with no temporal property, no user defined time instant
   AddOperator(&find_path_t_1);//minimum total time cost
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
