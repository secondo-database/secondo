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
  "(<text>int x relation -> busnetwork" "</text--->"
  "<text>thebusnetwork(_, _)</text--->"
  "<text>Creates busnetwork with id and a relation.</text--->"
  "<text>let busnet = thebusnetwork(1, busroutes)</text--->"
  "))";

const string OpBusNodeSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>busnetwork -> stream(tuple([nid:int,loc:point]))" "</text--->"
  "<text>busnode(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to a bus stop."
  "</text--->"
  "<text>query busnode(busroutes) count</text--->"
  "))";

const string OpBusNodeNewSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>busnetwork -> stream(tuple([nid:int,loc:point,path:int,post:int]))"
   "</text--->"
  "<text>busnodenew(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to a bus stop."
  "</text--->"
  "<text>query busnodenew(busroutes) count</text--->"
  "))";

const string OpBusEdgeSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>busnetwork -> a relation</text--->"
  "<text>busedge(_)</text--->"
  "<text>returns the relation for edges.</text--->"
  "<text>query busedge(busroutes) count</text--->"
  "))";

const string OpBusEdgeNewSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>busnetwork -> a relation</text--->"
  "<text>busedgenew(_)</text--->"
  "<text>returns the relation for edges.</text--->"
  "<text>query busedgenew(busroutes) count</text--->"
  "))";


const string OpBusMoveSpec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
 "(<text>busnetwork -> stream(tuple(([pid:int,rid:int,trip:mpoint])))</text--->"
  "<text>busmove(_)</text--->"
  "<text>returns a stream of tuple where each corresponds to "
  "the movement of a bus.</text--->"
  "<text>query busmove(busroutes) count</text--->"
  "))";

const string OpBusFindPath_T_1Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>busnetwork x rel x attribute x instant-> mpoint</text--->"
  "<text>find_path_t_1(_,_,_,_)</text--->"
  "<text>returns a sequence of movement corresponding to a trip.</text--->"
  "<text>query deftime(find_path_t_1(berlintrains,q1,id,querytime));</text--->"
  "))";

const string OpBusFindPath_T_2Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>busnetwork x rel x attribute x instant-> mpoint</text--->"
  "<text>find_path_t_2(_,_,_,_)</text--->"
  "<text>returns a sequence of movement corresponding to a trip.</text--->"
  "<text>query deftime(find_path_t_2(berlintrains,q1,id,querytime));</text--->"
  "))";

const string OpBusFindPath_T_3Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>relation x btree1 x busnetwork x rel x attribute1 x attribute2 "
  "x instant-> mpoint</text--->"
  "<text>_ _ find_path_t_3[_,_,_,_,_]</text--->"
  "<text>returns a sequence of movement corresponding to a trip</text--->"
  "<text>query deftime(edge_rel btree_edge_v1 find_path_t_3[berlintrains,tq1,"
  "id,dur,querytime]); </text--->"
  "))";

const string OpBusFindPath_T_4Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>busnetwork x rel x attribute1 "
   " x attribute2 x instant-> mpoint</text--->"
  "<text>_ find_path_t_4[_,_,_,_]</text--->"
  "<text>returns a sequence of movement corresponding to a trip</text--->"
  "<text>query deftime(berlintrains find_path_t_4"
  "[tq1,id,dur,querytime])</text--->"
  "))";

const string OpBusFindPath_T_5Spec =
 "((\"Signature\" \"Syntax\" \"Meaning\" "
  "\"Example\") "
  "(<text>busnetwork x rel x attribute1 "
  "x attribute2 x instant-> mpoint</text--->"
  "<text>_ find_path_t_5[_,_,_,_]</text--->"
  "<text>returns a sequence of movement corresponding to a trip</text--->"
  "<text>query deftime(berlintrains find_path_t_5"
  "[tq1,id,dur,querytime]);</text--->"
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

/*structure for displaying bus stop*/
struct DisplBusStopNew:public DisplBus{
  int no_stop;
  int bus_stop_count;
  Relation* busstopnew;
  DisplBusStopNew(BusNetwork* p):DisplBus(p){
    busstopnew = busnet->GetRelBus_NodeNew();
    no_stop = busstopnew->GetNoTuples();
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
Return all bus stops new.

*/
int OpBusNodeNewValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  DisplBusStopNew* localInfo;
  switch(message){
    case OPEN:{
        localInfo = new DisplBusStopNew((BusNetwork*)args[0].addr);
        localInfo->resulttype =
              new TupleType(nl->Second(GetTupleResultType(s)));
        local = SetWord(localInfo);
        return 0;
    }
    case REQUEST:{
        localInfo = (DisplBusStopNew*)local.addr;
        if(localInfo->bus_stop_count > localInfo->no_stop)
          return CANCEL;
        Tuple* tuple = new Tuple(localInfo->resulttype);
        Tuple* temp_tuple =
                localInfo->busstopnew->GetTuple(localInfo->bus_stop_count);
        CcInt* id = (CcInt*)temp_tuple->GetAttribute(BusNetwork::NEWSID);
        Point* location = (Point*)temp_tuple->GetAttribute(BusNetwork::NEWLOC);
        CcInt* path = (CcInt*)temp_tuple->GetAttribute(BusNetwork::BUSPATH);
        CcInt* pos = (CcInt*)temp_tuple->GetAttribute(BusNetwork::POS);

        tuple->PutAttribute(BusNetwork::NEWSID,new CcInt(*id));
        tuple->PutAttribute(BusNetwork::NEWLOC,new Point(*location));
        tuple->PutAttribute(BusNetwork::BUSPATH,new CcInt(*path));
        tuple->PutAttribute(BusNetwork::POS,new CcInt(*pos));

        result.setAddr(tuple);
        temp_tuple->DeleteIfAllowed();
        localInfo->bus_stop_count++;
        return YIELD;
    }
    case CLOSE:{
        localInfo = (DisplBusStopNew*)local.addr;
        delete localInfo;
        return 0;
    }
  }
  return 0;
}

/*
Display the edge

*/

int OpBusEdgeValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  BusNetwork* busnet = (BusNetwork*)args[0].addr;
  Relation* busedge = busnet->GetRelBus_Edge();
  result = SetWord(busedge->Clone());
  Relation* resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s,result);
  return 0;
}

int OpBusEdgeNewValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  BusNetwork* busnet = (BusNetwork*)args[0].addr;
  Relation* busedge = busnet->GetRelBus_EdgeNew();
  result = SetWord(busedge->Clone());
  Relation* resultSt = (Relation*)qp->ResultStorage(s).addr;
  resultSt->Close();
  qp->ChangeResultStorage(s,result);
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
minimum total time cost
simple algorithm

*/

int OpBusFindPath_T_1ValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BusNetwork* busnet = (BusNetwork*)args[0].addr;
  Relation* querycond = (Relation*)args[1].addr;
  Instant* instant = static_cast<Instant*>(args[3].addr);
  int attrpos = ((CcInt*)args[4].addr)->GetIntval() - 1;
//  cout<<"attrpos "<<attrpos<<endl;
//  cout<<*instant<<endl;
  busnet->FindPath_T_1((MPoint*)result.addr,querycond,attrpos,instant);
  return 0;
}
/*
optimize -1

*/
int OpBusFindPath_T_2ValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BusNetwork* busnet = (BusNetwork*)args[0].addr;
  Relation* querycond = (Relation*)args[1].addr;
  Instant* instant = static_cast<Instant*>(args[3].addr);
  int attrpos = ((CcInt*)args[4].addr)->GetIntval() - 1;
  busnet->FindPath_T_2((MPoint*)result.addr,querycond,attrpos,*instant);
  return 0;
}

/*
optimize-1
middle stop with temporal property

*/
int OpBusFindPath_T_3ValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
    result = qp->ResultStorage(s);
    BusNetwork* busnet = (BusNetwork*)args[2].addr;
    Relation* busedge = (Relation*)args[0].addr;
    BTree* btree1 = (BTree*)args[1].addr;
    Relation* querycond = (Relation*)args[3].addr;
    Instant* instant = static_cast<Instant*>(args[6].addr);
    int attrpos1 = ((CcInt*)args[7].addr)->GetIntval() - 1;
    int attrpos2 = ((CcInt*)args[8].addr)->GetIntval() - 1;
//    cout<<attrpos1<<" "<<attrpos2<<endl;
//    cout<<*instant<<endl;

    busnet->FindPath_T_3((MPoint*)result.addr,querycond,busedge,btree1,
        attrpos1,attrpos2,*instant);
    return 0;
}

/*
optimize-1, 3
middle stop with temporal property
input edge relation and b-tree

*/
int OpBusFindPath_T_4ValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BusNetwork* busnet = (BusNetwork*)args[0].addr;

  Relation* querycond = (Relation*)args[1].addr;
  Instant* instant = static_cast<Instant*>(args[4].addr);

  int attrpos1 = ((CcInt*)args[5].addr)->GetIntval() - 1;
  int attrpos2 = ((CcInt*)args[6].addr)->GetIntval() - 1;

  busnet->FindPath_T_4((MPoint*)result.addr,
                  querycond,attrpos1,attrpos2,*instant);
  return 0;
}

/*
optimize-1, 3 ,4
middle stop with temporal property
input edge relation and b-tree

*/
int OpBusFindPath_T_5ValueMapping(Word* args, Word& result,
                               int message, Word& local, Supplier s)
{
  result = qp->ResultStorage(s);
  BusNetwork* busnet = (BusNetwork*)args[0].addr;

  Relation* querycond = (Relation*)args[1].addr;
  Instant* instant = static_cast<Instant*>(args[4].addr);

  int attrpos1 = ((CcInt*)args[5].addr)->GetIntval() - 1;
  int attrpos2 = ((CcInt*)args[6].addr)->GetIntval() - 1;

  busnet->FindPath_T_5((MPoint*)result.addr,
                  querycond,attrpos1,attrpos2,*instant);

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
Operator ~busnewnode~

*/
ListExpr OpBusNodeNewTypeMap(ListExpr in_xArgs)
{
  if(nl->ListLength(in_xArgs) != 1)
    return (nl->SymbolAtom("typeerror"));

  ListExpr arg = nl->First(in_xArgs);
  if(nl->IsAtom(arg) && nl->AtomType(arg) == SymbolType &&
     nl->SymbolValue(arg) == "busnetwork"){
    return nl->TwoElemList(
          nl->SymbolAtom("stream"),
            nl->TwoElemList(nl->SymbolAtom("tuple"),
              nl->FourElemList(
               nl->TwoElemList(nl->SymbolAtom("id"),nl->SymbolAtom("int")),
               nl->TwoElemList(nl->SymbolAtom("loc"),nl->SymbolAtom("point")),
               nl->TwoElemList(nl->SymbolAtom("buspath"),nl->SymbolAtom("int")),
               nl->TwoElemList(nl->SymbolAtom("pos"),nl->SymbolAtom("int"))
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
      ListExpr xType;
      nl->ReadFromString(BusNetwork::busedgeTypeInfo,xType);
      return xType;
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
  string err = "busnetwork x rel x attribute x instant expected";
  if(nl->ListLength(in_xArgs) != 4){
      ErrorReporter::ReportError(err);
      return nl->TypeError();
  }

  ListExpr arg1 = nl->First(in_xArgs);
  ListExpr arg2 = nl->Second(in_xArgs);
  ListExpr arg3 = nl->Third(in_xArgs);
  ListExpr arg4 = nl->Fourth(in_xArgs);
  if(!IsRelDescription(arg2)){
      string msg = "second argument must be a relation";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  int j;
  ListExpr attrType;
  j = FindAttribute(nl->Second(nl->Second(arg2)),
                   nl->SymbolValue(arg3),attrType);

  CHECK_COND( j > 0 && (nl->IsEqual(attrType,"int")),
             "the third attribute should be of type int");

  if(!nl->IsEqual(arg4,"instant")){
      string msg = "fourth argument must be an instant";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "busnetwork"){
//    return nl->SymbolAtom("mpoint");
    ListExpr res = nl->SymbolAtom("mpoint");
    return nl->ThreeElemList(
            nl->SymbolAtom("APPEND"),
            nl->OneElemList(nl->IntAtom(j)),res);
  }

  ErrorReporter::ReportError(err);
  return nl->TypeError();

}

ListExpr OpBusFindPath_T_2TypeMap(ListExpr in_xArgs)
{
string err = "busnetwork x rel x attribute x instant expected";
  if(nl->ListLength(in_xArgs) != 4){
      ErrorReporter::ReportError(err);
      return nl->TypeError();
  }

  ListExpr arg1 = nl->First(in_xArgs);
  ListExpr arg2 = nl->Second(in_xArgs);
  ListExpr arg3 = nl->Third(in_xArgs);
  ListExpr arg4 = nl->Fourth(in_xArgs);
  if(!IsRelDescription(arg2)){
      string msg = "second argument must be a relation";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  int j;
  ListExpr attrType;
  j = FindAttribute(nl->Second(nl->Second(arg2)),
                   nl->SymbolValue(arg3),attrType);

  CHECK_COND( j > 0 && (nl->IsEqual(attrType,"int")),
             "the third attribute should be of type int");

  if(!nl->IsEqual(arg4,"instant")){
      string msg = "fourth argument must be an instant";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "busnetwork"){
//    return nl->SymbolAtom("mpoint");
    ListExpr res = nl->SymbolAtom("mpoint");
    return nl->ThreeElemList(
            nl->SymbolAtom("APPEND"),
            nl->OneElemList(nl->IntAtom(j)),res);
  }

  ErrorReporter::ReportError(err);
  return nl->TypeError();

}


ListExpr OpBusFindPath_T_3TypeMap(ListExpr in_xArgs)
{
  string s1 = "edgerel x b-tree x busnetwork x rel x attribute1";
  string s2 = "x attribute2 x instant expected";
  string err = s1 + s2;
  if(nl->ListLength(in_xArgs) != 7){
      ErrorReporter::ReportError(err);
      return nl->TypeError();
  }
  ListExpr arg1 = nl->First(in_xArgs);
  ListExpr arg2 = nl->Second(in_xArgs);
  ListExpr arg3 = nl->Third(in_xArgs);
  ListExpr arg4 = nl->Fourth(in_xArgs);
  ListExpr arg5 = nl->Fifth(in_xArgs);
  ListExpr arg6 = nl->Sixth(in_xArgs);
  ListExpr arg7 = nl->Nth(7,in_xArgs);
  if(!IsRelDescription(arg1)){
      string msg =  "first argument must be a relation";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  if(!IsRelDescription(arg4)){
      string msg =  "fourth argument must be a relation";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  CHECK_COND(listutils::isBTreeDescription(arg2),
      "second argument muse be an btree");

  ListExpr relAttrList = nl->Second(nl->Second(arg1));
  ListExpr btreeAttrList = nl->Second(nl->Second(arg2));
  if(!nl->Equal(relAttrList,btreeAttrList)){
    ErrorReporter::ReportError(err);
    return nl->TypeError();
  }

  if(!nl->IsEqual(arg7,"instant")){
      string msg = "seventh argument must be an instant";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  int j1,j2;
  ListExpr attrType;
  j1 = FindAttribute(nl->Second(nl->Second(arg4)),
                   nl->SymbolValue(arg5),attrType);

  CHECK_COND( j1 > 0 && (nl->IsEqual(attrType,"int")),
             "the fifth attribute should be of type int");

  j2 = FindAttribute(nl->Second(nl->Second(arg4)),
                   nl->SymbolValue(arg6),attrType);

  CHECK_COND( j1 > 0 && (nl->IsEqual(attrType,"duration")),
             "the sixth attribute should be of type duration");


  if(nl->IsAtom(arg3) && nl->AtomType(arg3) == SymbolType &&
     nl->SymbolValue(arg3) == "busnetwork"){

//    return nl->SymbolAtom("mpoint");
    ListExpr res = nl->SymbolAtom("mpoint");
    return nl->ThreeElemList(
            nl->SymbolAtom("APPEND"),
            nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),res);

  }
  ErrorReporter::ReportError(err);
  return nl->TypeError();

}

ListExpr OpBusFindPath_T_4TypeMap(ListExpr in_xArgs)
{
  string s1 = "busnetwork x rel x attribute1";
  string s2 = "x attribute2 x instant expected";
  string err = s1 + s2;

  if(nl->ListLength(in_xArgs) != 5){
      ErrorReporter::ReportError(err);
      return nl->TypeError();
  }
  ListExpr arg1 = nl->First(in_xArgs);
  ListExpr arg2 = nl->Second(in_xArgs);

  ListExpr arg3 = nl->Third(in_xArgs);
  ListExpr arg4 = nl->Fourth(in_xArgs);
  ListExpr arg5 = nl->Fifth(in_xArgs);


  if(!IsRelDescription(arg2)){
      string msg =  "second argument must be a relation";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  int j1,j2;
  ListExpr attrType;
  j1 = FindAttribute(nl->Second(nl->Second(arg2)),
                   nl->SymbolValue(arg3),attrType);

  CHECK_COND( j1 > 0 && (nl->IsEqual(attrType,"int")),
             "the sixth attribute should be of type int");

  j2 = FindAttribute(nl->Second(nl->Second(arg2)),
                   nl->SymbolValue(arg4),attrType);

  CHECK_COND( j1 > 0 && (nl->IsEqual(attrType,"duration")),
             "the seventh attribute should be of type duration");

  if(!nl->IsEqual(arg5,"instant")){
      string msg = "eighth argument must be an instant";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "busnetwork"){
//    return nl->SymbolAtom("mpoint");
    ListExpr res = nl->SymbolAtom("mpoint");
    return nl->ThreeElemList(
            nl->SymbolAtom("APPEND"),
            nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),res);

  }
  ErrorReporter::ReportError(err);
  return nl->TypeError();

}


ListExpr OpBusFindPath_T_5TypeMap(ListExpr in_xArgs)
{
  string s1 = "busnetwork x rel x attribute1";
  string s2 = "x attribute2 x instant expected";
  string err = s1 + s2;

  if(nl->ListLength(in_xArgs) != 5){
      ErrorReporter::ReportError(err);
      return nl->TypeError();
  }
  ListExpr arg1 = nl->First(in_xArgs);
  ListExpr arg2 = nl->Second(in_xArgs);

  ListExpr arg3 = nl->Third(in_xArgs);
  ListExpr arg4 = nl->Fourth(in_xArgs);
  ListExpr arg5 = nl->Fifth(in_xArgs);


  if(!IsRelDescription(arg2)){
      string msg =  "second argument must be a relation";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  int j1,j2;
  ListExpr attrType;
  j1 = FindAttribute(nl->Second(nl->Second(arg2)),
                   nl->SymbolValue(arg3),attrType);

  CHECK_COND( j1 > 0 && (nl->IsEqual(attrType,"int")),
             "the sixth attribute should be of type int");

  j2 = FindAttribute(nl->Second(nl->Second(arg2)),
                   nl->SymbolValue(arg4),attrType);

  CHECK_COND( j1 > 0 && (nl->IsEqual(attrType,"duration")),
             "the seventh attribute should be of type duration");

  if(!nl->IsEqual(arg5,"instant")){
      string msg = "eighth argument must be an instant";
      ErrorReporter::ReportError(msg);
      return nl->TypeError();
  }

  if(nl->IsAtom(arg1) && nl->AtomType(arg1) == SymbolType &&
     nl->SymbolValue(arg1) == "busnetwork"){
//    return nl->SymbolAtom("mpoint");
    ListExpr res = nl->SymbolAtom("mpoint");
    return nl->ThreeElemList(
            nl->SymbolAtom("APPEND"),
            nl->TwoElemList(nl->IntAtom(j1),nl->IntAtom(j2)),res);

  }
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

Operator busnodenew(
  "busnodenew", //name
  OpBusNodeNewSpec,
  OpBusNodeNewValueMapping,
  Operator::SimpleSelect,
  OpBusNodeNewTypeMap
);


Operator busedge(
  "busedge", //name
  OpBusEdgeSpec,
  OpBusEdgeValueMapping,
  Operator::SimpleSelect,
  OpBusEdgeTypeMap
);

Operator busedgenew(
  "busedgenew", //name
  OpBusEdgeNewSpec,
  OpBusEdgeNewValueMapping,
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

Operator find_path_t_2(
  "find_path_t_2", //name
  OpBusFindPath_T_2Spec,
  OpBusFindPath_T_2ValueMapping,
  Operator::SimpleSelect,
  OpBusFindPath_T_2TypeMap
);

Operator find_path_t_3(
  "find_path_t_3", //name
  OpBusFindPath_T_3Spec,
  OpBusFindPath_T_3ValueMapping,
  Operator::SimpleSelect,
  OpBusFindPath_T_3TypeMap
);

Operator find_path_t_4(
  "find_path_t_4", //name
  OpBusFindPath_T_4Spec,
  OpBusFindPath_T_4ValueMapping,
  Operator::SimpleSelect,
  OpBusFindPath_T_4TypeMap
);

Operator find_path_t_5(
  "find_path_t_5", //name
  OpBusFindPath_T_5Spec,
  OpBusFindPath_T_5ValueMapping,
  Operator::SimpleSelect,
  OpBusFindPath_T_5TypeMap
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
   AddOperator(&busnodenew);
   AddOperator(&busedge); //display the trajectory of a bus
   AddOperator(&busedgenew); //display the trajectory of a bus
   AddOperator(&busmove);//display bus movement
   //middle stop with no temporal property, no user defined time instant
   AddOperator(&find_path_t_1);//minimum total time cost
   AddOperator(&find_path_t_2);//minimum total time cost, faster
   //minimum total time cost, faster, time duration for middle stop
   AddOperator(&find_path_t_3);
   //input relation and b-tree
   AddOperator(&find_path_t_4);
   AddOperator(&find_path_t_5);//optimize-4
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
