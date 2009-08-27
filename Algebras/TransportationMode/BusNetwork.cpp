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

[1] Source File of the Transportation Mode-Bus Network Algebra

August, 2009 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
queries moving objects with transportation modes.

2 Defines and includes

*/

#include "BusNetwork.h"

string BusNetwork::busrouteTypeInfo = "(rel(tuple((Id int)(Trip mpoint))))";
string BusNetwork::btreebusrouteTypeInfo =
                  "(btree(tuple((Id int)(Trip mpoint))) int)";
string BusNetwork::busstopTypeInfo = "(rel(tuple((Id int)(BusStop point))))";
string BusNetwork::btreebusstopTypeInfo =
                  "(btree(tuple((Id int)(BusStop point))) int)";
string BusNetwork::rtreebusstopTypeInfo =
                  "(rtree(tuple((Id int)(BusStop point))) BusStop FALSE)";
string BusNetwork::busweightTypeInfo =
                  "(rel(tuple((fid int)(def_t periods)(l line)(fee real))))";
string BusNetwork::btreebusweightTypeInfo =
              "(btree(tuple((fid int)(def_t periods)(l line)(fee real))) int)";
string BusNetwork::busedgeTypeInfo =
              "(rel(tuple((eid int)(v1 int)(v2 int)(wid int)(rid int))))";
string BusNetwork::btreebusedgeTypeInfo =
            "(btree(tuple((eid int)(v1 int)(v2 int)(wid int)(rid int))) int)";


ListExpr BusNetwork::BusNetworkProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,"thebusnetwork(<id>,<busroute-relation>)");

  return  (nl->TwoElemList(
          nl->TwoElemList(nl->StringAtom("Creation"),
                          nl->StringAtom("Example Creation")),
          nl->TwoElemList(examplelist,
          nl->StringAtom("(let citybus = thebusnetwork(id,busroutes))"))));
}

ListExpr BusNetwork::OutBusNetwork(ListExpr typeInfo,Word value)
{

  BusNetwork* p = (BusNetwork*)value.addr;
  return p->Out(typeInfo);
}

Word BusNetwork::CreateBusNetwork(const ListExpr typeInfo)
{
  return SetWord(new BusNetwork());
}


void BusNetwork::DeleteBusNetwork(const ListExpr typeInfo,Word& w)
{
//  cout<<"DeleteBusNetwork"<<endl;
  BusNetwork* p = (BusNetwork*)w.addr;
  p->Destory();
  delete p;
}

ListExpr BusNetwork::Out(ListExpr typeInfo)
{
  cout<<"out"<<endl;
  //relation for node
  ListExpr xBusStop = nl->TheEmptyList();
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();

  bool bFirst = true;
  for(int i = 1;i <= bus_node->GetNoTuples();i++){
    Tuple* pCurrentBus_Stop = bus_node->GetTuple(i);
//    cout<<*pCurrentBus_Stop<<endl;

    CcInt* id = (CcInt*)pCurrentBus_Stop->GetAttribute(SID);
    int bus_stop_id = id->GetIntval();
//    cout<<"bus stop id "<<bus_stop_id<<endl;
    Point* p = (Point*)pCurrentBus_Stop->GetAttribute(LOC);
    ListExpr xPoint = OutPoint(nl->TheEmptyList(),SetWord(p));
    //build the list
    xNext = nl->TwoElemList(nl->IntAtom(bus_stop_id),xPoint);
    if(bFirst){
      xBusStop = nl->OneElemList(xNext);
      xLast = xBusStop;
      bFirst = false;
    }else
      xLast = nl->Append(xLast,xNext);
    pCurrentBus_Stop->DeleteIfAllowed();
  }

  //relation for weight

  //relatino for edge

  return nl->TwoElemList(nl->IntAtom(busnet_id),xBusStop);
}

bool BusNetwork::OpenBusNetwork(SmiRecord& valueRecord,size_t& offset,
const ListExpr typeInfo,Word& value)
{
//  cout<<"openbusnetwork"<<endl;
  value.addr = BusNetwork::Open(valueRecord,offset,typeInfo);
  return value.addr != NULL;
}

BusNetwork* BusNetwork::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{
//  cout<<"open"<<endl;
  return new BusNetwork(valueRecord,offset,typeInfo);

}

bool BusNetwork::SaveBusNetwork(SmiRecord& valueRecord,size_t& offset,
const ListExpr typeInfo,Word& value)
{
//  cout<<"savebusnetwork"<<endl;
  BusNetwork* p = (BusNetwork*)value.addr;
  return p->Save(valueRecord,offset,typeInfo);
}

bool BusNetwork::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
//  cout<<"save"<<endl;
  /********************Save id the busnetwork****************************/
  int iId = busnet_id;
  in_xValueRecord.Write(&iId,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);
  /****************************save bus routes*****************************/
  ListExpr xType;
  nl->ReadFromString(busrouteTypeInfo,xType);
  ListExpr xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!bus_route->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;
  /*************************save b-tree on bus route********************/
  nl->ReadFromString(btreebusrouteTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bus_route->Save(in_xValueRecord,inout_iOffset,xNumericType))
    return false;

  /****************************save nodes*********************************/
  nl->ReadFromString(busstopTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!bus_node->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;
  /********************Save b-tree for bus stop***************************/
  nl->ReadFromString(btreebusstopTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bus_node->Save(in_xValueRecord,inout_iOffset,xNumericType))
    return false;

  /*******************Save r-tree for bus stop**************************/
  if(!rtree_bus_node->Save(in_xValueRecord,inout_iOffset))
    return false;

  /********************Save weight function****************************/
  nl->ReadFromString(busweightTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!bus_weight->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /*******************Save b-tree on weight function********************/
  nl->ReadFromString(btreebusweightTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bus_weight->Save(in_xValueRecord,inout_iOffset,xNumericType))
    return false;

  /********************Save edges*************************************/
  nl->ReadFromString(busedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!bus_edge->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /******************Save b-tree on edge (eid)*****************************/
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bus_edge->Save(in_xValueRecord,inout_iOffset,xNumericType))
    return false;

  /******************Save b-tree on edge start node id**********************/
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bus_edge_v1->Save(in_xValueRecord,inout_iOffset,xNumericType))
    return false;

  /******************Save b-tree on edge end node id**********************/
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!btree_bus_edge_v2->Save(in_xValueRecord,inout_iOffset,xNumericType))
    return false;

  return true;
}

void BusNetwork::CloseBusNetwork(const ListExpr typeInfo,Word& w)
{
  //bus route
 if((BusNetwork*)w.addr != NULL)
      ((BusNetwork*)w.addr)->bus_route->Close();
  //b-tree on bus route
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->btree_bus_route;
  //bus stop
  if((BusNetwork*)w.addr != NULL)
      ((BusNetwork*)w.addr)->bus_node->Close();
  //b-tree on bus stop
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->btree_bus_node;
  //r-tree on bus stop
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->rtree_bus_node;
  //bus weight function
  if((BusNetwork*)w.addr != NULL)
      ((BusNetwork*)w.addr)->bus_weight->Close();
  //b-tree on weight function
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->btree_bus_weight;
  //bus edge relation
  if((BusNetwork*)w.addr != NULL)
     ((BusNetwork*)w.addr)->bus_edge->Close();
  //b-tree on edge id
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->btree_bus_edge;
  //b-tree on edge start node id
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->btree_bus_edge_v1;
  //b-tree on edge end node id
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->btree_bus_edge_v2;
  delete static_cast<BusNetwork*>(w.addr);
  w.addr = NULL;
}

Word BusNetwork::CloneBusNetwork(const ListExpr,const Word&)
{
  return SetWord(Address(0));
}

void* BusNetwork::CastBusNetwork(void* addr)
{
  return NULL;
}
int BusNetwork::SizeOfBusNetwork()
{
  return 0;
}
bool BusNetwork::CheckBusNetwork(ListExpr type,ListExpr& errorInfo)
{
  return nl->IsEqual(type,"busnetwork");
}

Word BusNetwork::InBusNetwork(ListExpr in_xTypeInfo,ListExpr in_xValue,
int in_iErrorPos,ListExpr& inout_xErrorInfo,bool& inout_bCorrect)
{
  cout<<"inbusnetwork"<<endl;
  BusNetwork* p = new BusNetwork(in_xValue,in_iErrorPos, inout_xErrorInfo,
                                inout_bCorrect);
  if(inout_bCorrect)
    return SetWord(p);
  else{
    delete p;
    return SetWord(Address(0));
  }
}
/*
Create the relation for storing bus stops

*/
void BusNetwork::FillBusNode(const Relation* in_busRoute)
{
  cout<<"create bus stop....."<<endl;
  Points* ps = new Points(0);
  ps->StartBulkLoad();
  for(int i = 1; i <=  in_busRoute->GetNoTuples();i++){
    Tuple* tuple = in_busRoute->GetTuple(i);
    MPoint* trip = (MPoint*)tuple->GetAttribute(TRIP);
    const UPoint* up;
    for(int j = 0;j < trip->GetNoComponents();j++){
      trip->Get(j,up);
      Point p0 = up->p0;
      Point p1 = up->p1;
      if(j == 0 ){ //add the start location as a stop
        *ps += p0;
         continue;
      }
      if(j == trip->GetNoComponents() - 1){ //the end location
        *ps += p1;
        continue;
      }
      if(AlmostEqual(p0,p1)){
        *ps += p0;
      }
    }
    tuple->DeleteIfAllowed();
  }
  ps->EndBulkLoad(true,true);

  ListExpr xTypeInfo;
  nl->ReadFromString(busstopTypeInfo,xTypeInfo);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
  Relation* temp_bus_node = new Relation(xNumType,true);
  for(int i = 0;i < ps->Size();i++){
    const Point* temp_p;
    ps->Get(i,temp_p);
    Tuple* tuple = new Tuple(nl->Second(xNumType));
    tuple->PutAttribute(SID,new CcInt(true,i+1));
    tuple->PutAttribute(LOC,new Point(*temp_p));
    temp_bus_node->AppendTuple(tuple);
    tuple->DeleteIfAllowed();
  }
  delete ps;
  /***************************bus route relation***********************/
  ostringstream xBusRoutePtrStream;
  xBusRoutePtrStream << (long)in_busRoute;
  string strQuery = "(consume(sort(feed(" + busrouteTypeInfo +
                "(ptr " + xBusRoutePtrStream.str() + ")))))";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  bus_route = (Relation*)xResult.addr;
  /*********************b-tree on bus route**************************/
  cout<<"create b-tree on bus route id..."<<endl;
  ostringstream xThisBusRoutePtrStream;
  xThisBusRoutePtrStream<<(long)bus_route;
  strQuery = "(createbtree (" + busrouteTypeInfo +
             "(ptr " + xThisBusRoutePtrStream.str() + "))" + "Id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bus_route = (BTree*)xResult.addr;
  cout<<"b-tree on bus route is finished..."<<endl;
  /***********************bus stop relation***************************/
  ostringstream xBusStopPtrStream;
  xBusStopPtrStream << (long)temp_bus_node;
  strQuery = "(consume(sort(feed(" + busstopTypeInfo +
                "(ptr " + xBusStopPtrStream.str() + ")))))";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  bus_node = (Relation*)xResult.addr;
  temp_bus_node->Delete();
  cout<<"bus stop is finished....."<<endl;
  /************b-tree on bus stop****************************/
  cout<<"create b-tree on bus stop id...."<<endl;
  ostringstream xThisBusStopPtrStream;
  xThisBusStopPtrStream<<(long)bus_node;
  strQuery = "(createbtree (" + busstopTypeInfo +
             "(ptr " + xThisBusStopPtrStream.str() + "))" + "Id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bus_node = (BTree*)xResult.addr;
  cout<<"b-tree on bus stop id is finished...."<<endl;
  /***********r-tree on bus stop*****************************/
  cout<<"create r-tree on bus stop..."<<endl;
  ostringstream xBusStop;
  xBusStop <<(long)bus_node;
  strQuery = "(bulkloadrtree(sortby(addid(feed(" + busstopTypeInfo +
            "(ptr " + xBusStop.str() + "))))((BusStop asc))) BusStop)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  rtree_bus_node = (R_Tree<2,TupleId>*)xResult.addr;
  cout<<"r-tree on bus stop is finished...."<<endl;
}
/*
Get the node id of a bus stop

*/
int BusNetwork::FindPointTid(Point& p)
{
  BBox<2> searchbox = p.BoundingBox();
  R_TreeLeafEntry<2,TupleId> e;
  if(rtree_bus_node->First(searchbox,e)){
    Tuple* tuple = bus_node->GetTuple(e.info);
    CcInt* nid = (CcInt*)tuple->GetAttribute(SID);
    tuple->DeleteIfAllowed();
    return nid->GetIntval();
  }
  return 0;
}
/*
Create the relation for storing edges as well as its weight function

*/
void BusNetwork::FillBusWeightAndEdge(const Relation* in_busRoute)
{
  cout<<"create edge and weight function relation..."<<endl;
  vector<Point> endpoints;//store start and end pointss
  vector<Point> connectpoints; //store points between two stops
  Interval<Instant> timeInterval;

  //relation description for edge weight function
  ListExpr xTypeInfoWeight;
  nl->ReadFromString(busweightTypeInfo,xTypeInfoWeight);
  ListExpr xNumType1 =
                SecondoSystem::GetCatalog()->NumericType(xTypeInfoWeight);
  Relation* temp_bus_weight = new Relation(xNumType1,true);

  ListExpr xTypeInfoEdge;
  nl->ReadFromString(busedgeTypeInfo,xTypeInfoEdge);
  ListExpr xNumType2 =
                SecondoSystem::GetCatalog()->NumericType(xTypeInfoEdge);
  Relation* temp_bus_edge = new Relation(xNumType2,true);

  int fid = 1;
  int eid = 1;
  for(int i = 1; i <=  in_busRoute->GetNoTuples();i++){
    Tuple* tuple = in_busRoute->GetTuple(i);
    MPoint* trip = (MPoint*)tuple->GetAttribute(TRIP);
    const UPoint* up;
    CcInt* rid = (CcInt*)tuple->GetAttribute(RID);
    int routeid = rid->GetIntval();

    endpoints.clear();
    connectpoints.clear();
    /////generate cost fee /////
    srand(time(0));
    float costfee = (10+rand() % 20)/100.0;
//    cout<<costfee<<endl;
    srand(i);
    //////////
    for(int j = 0;j < trip->GetNoComponents();j++){
      trip->Get(j,up);
      Point p0 = up->p0;
      Point p1 = up->p1;
      if(j == 0 ){ //add the start location as a stop
        endpoints.push_back(p0);
        connectpoints.push_back(p0);
        if(!AlmostEqual(p0,p1))
          connectpoints.push_back(p1);
        timeInterval.start = up->timeInterval.start;
        continue;
      }

      if(AlmostEqual(p0,p1)){

        endpoints.push_back(p0);

        if(endpoints.size() == 2){ //extract weight and edge
          timeInterval.end = up->timeInterval.start;
 //       cout<<i<<" Interval "<<timeInterval<<" "<<connectpoints.size()<<endl;
          //create line
          Line* line = new Line(0);
          line->StartBulkLoad();
          int edgeno = 0;
          HalfSegment hs;
          for(unsigned int k = 0;k < connectpoints.size() - 1;k++){
            Point start = connectpoints[k];
            Point end = connectpoints[k+1];
            hs.Set(true,start,end);
            hs.attr.edgeno = ++edgeno;
            *line += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *line += hs;
          }
          line->EndBulkLoad();
 ////////////////////    edge         ///////////////////////////////////////
          int id1 = FindPointTid(endpoints[0]);
          int id2 = FindPointTid(endpoints[1]);
//          cout<<id1<<" "<<id2<<endl;
          assert(id1 != id2 && id1 != 0 && id2 != 0);
//          cout<<endpoints[0]<<" "<<endpoints[1]<<endl;
          Tuple* tuple_p1 = bus_node->GetTuple(id1);
          Tuple* tuple_p2 = bus_node->GetTuple(id2);
//          cout<<*tuple_p1<<" "<<*tuple_p2<<endl;
          Point* p1 = (Point*)tuple_p1->GetAttribute(LOC);
          Point* p2 = (Point*)tuple_p2->GetAttribute(LOC);
          assert(AlmostEqual(*p1,endpoints[0])&&AlmostEqual(*p2,endpoints[1]));
          tuple_p1->DeleteIfAllowed();
          tuple_p2->DeleteIfAllowed();

          Tuple* edgetuple = new Tuple(nl->Second(xNumType2));
          edgetuple->PutAttribute(EID,new CcInt(true,eid));
          edgetuple->PutAttribute(V1, new CcInt(true,id1));
          edgetuple->PutAttribute(V2, new CcInt(true,id2));
          edgetuple->PutAttribute(WFID, new CcInt(true,fid));
          edgetuple->PutAttribute(PID,new CcInt(true,routeid));
          temp_bus_edge->AppendTuple(edgetuple);
          edgetuple->DeleteIfAllowed();
          eid++;
//////////////////// weight function //////////////////////////////////////////
          Tuple* tuple = new Tuple(nl->Second(xNumType1));
          tuple->PutAttribute(FID,new CcInt(true,fid));
          Periods* peri = new Periods(1);
          peri->StartBulkLoad();
          peri->Add(timeInterval);
          peri->EndBulkLoad(true);
          tuple->PutAttribute(DEF_T,peri);
          tuple->PutAttribute(LINE, new Line(*line));
          tuple->PutAttribute(FEE,new CcReal(true,costfee));
          temp_bus_weight->AppendTuple(tuple);
          tuple->DeleteIfAllowed();
          delete line;
          fid++;
//////////////////////////////////////////////////////////////////////////
          endpoints.clear();
          connectpoints.clear();

          endpoints.push_back(p0);//next line
          connectpoints.push_back(p0);
          timeInterval.start = up->timeInterval.end;
          timeInterval.end = up->timeInterval.end;
        }else{
          //the prorgram should never come here
          assert(false);
        }
      }else{ //not AlmostEqual
        //add points
         connectpoints.push_back(p1);

         if(j == trip->GetNoComponents() - 1){ //add a trip without middle stop
          endpoints.push_back(p1);

          timeInterval.end = up->timeInterval.end;
          Line* line = new Line(0);
          line->StartBulkLoad();
          int edgeno = 0;
          HalfSegment hs;
          for(unsigned int k = 0;k < connectpoints.size() - 1;k++){
            Point start = connectpoints[k];
            Point end = connectpoints[k+1];
            hs.Set(true,start,end);
            hs.attr.edgeno = ++edgeno;
            *line += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            *line += hs;
          }
          line->EndBulkLoad();

////////////////////    edge         ///////////////////////////////////////
          int id1 = FindPointTid(endpoints[0]);
          int id2 = FindPointTid(endpoints[1]);
//          cout<<id1<<" "<<id2<<endl;
          assert(id1 != id2);
//          cout<<endpoints[0]<<" "<<endpoints[1]<<endl;
          Tuple* tuple_p1 = bus_node->GetTuple(id1);
          Tuple* tuple_p2 = bus_node->GetTuple(id2);
//          cout<<*tuple_p1<<" "<<*tuple_p2<<endl;
          Point* p1 = (Point*)tuple_p1->GetAttribute(LOC);
          Point* p2 = (Point*)tuple_p2->GetAttribute(LOC);
          assert(AlmostEqual(*p1,endpoints[0])&&AlmostEqual(*p2,endpoints[1]));
          tuple_p1->DeleteIfAllowed();
          tuple_p2->DeleteIfAllowed();

          Tuple* edgetuple = new Tuple(nl->Second(xNumType2));
          edgetuple->PutAttribute(EID,new CcInt(true,eid));
          edgetuple->PutAttribute(V1, new CcInt(true,id1));
          edgetuple->PutAttribute(V2, new CcInt(true,id2));
          edgetuple->PutAttribute(WFID, new CcInt(true,fid));
          edgetuple->PutAttribute(PID,new CcInt(true,routeid));
          temp_bus_edge->AppendTuple(edgetuple);
          edgetuple->DeleteIfAllowed();
          eid++;
//////////////////// weight function //////////////////////////////////////////
          Tuple* tuple = new Tuple(nl->Second(xNumType1));
          tuple->PutAttribute(FID,new CcInt(true,fid));
          Periods* peri = new Periods(1);
          peri->StartBulkLoad();
          peri->Add(timeInterval);
          peri->EndBulkLoad(true);
          tuple->PutAttribute(DEF_T,peri);
          tuple->PutAttribute(LINE, new Line(*line));
          tuple->PutAttribute(FEE,new CcReal(true,costfee));
          temp_bus_weight->AppendTuple(tuple);
          tuple->DeleteIfAllowed();
          delete line;
          fid++;
//////////////////////////////////////////////////////////////////////////
          endpoints.clear();
          connectpoints.clear();
        }
      }
    }
    tuple->DeleteIfAllowed();
  }
///////////////////relation for weight function///////////////////////////
  ostringstream xBusWeightPtrStream;
  xBusWeightPtrStream << (long)temp_bus_weight;
  string strQuery = "(consume(sort(feed(" + busweightTypeInfo +
                "(ptr " + xBusWeightPtrStream.str() + ")))))";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  bus_weight = (Relation*)xResult.addr;
  delete temp_bus_weight;
  cout<<"weight function relation is finished..."<<endl;
  /************b-tree on weight function****************************/
  ostringstream xThisBusWeightPtrStream;
  xThisBusWeightPtrStream<<(long)bus_weight;
  strQuery = "(createbtree (" + busweightTypeInfo +
             "(ptr " + xThisBusWeightPtrStream.str() + "))" + "fid)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bus_weight = (BTree*)xResult.addr;
  cout<<"b-tree on weight function is built..."<<endl;
///////////////////relation for edge///////////////////////////////////
//  cout<<temp_bus_edge->GetNoTuples()<<endl;
  ostringstream xBusEdgePtrStream;
  xBusEdgePtrStream << (long)temp_bus_edge;
  strQuery = "(consume(sort(feed(" + busedgeTypeInfo +
                "(ptr " + xBusEdgePtrStream.str() + ")))))";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  bus_edge = (Relation*)xResult.addr;
  delete temp_bus_edge;
  cout<<"edge relation is finished..."<<endl;
 /////////////////b-tree on edge///////////////////////////////////
  ostringstream xThisBusEdgePtrStream;
  xThisBusEdgePtrStream<<(long)bus_edge;
  strQuery = "(createbtree (" + busedgeTypeInfo +
             "(ptr " + xThisBusEdgePtrStream.str() + "))" + "eid)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bus_edge = (BTree*)xResult.addr;
  cout<<"b-tree on edge id is built..."<<endl;
  /*********************b-tree on edge start node id******************/
  ostringstream xThisBusEdgeV1PtrStream;
  xThisBusEdgeV1PtrStream<<(long)bus_edge;
  strQuery = "(createbtree (" + busedgeTypeInfo +
             "(ptr " + xThisBusEdgeV1PtrStream.str() + "))" + "v1)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bus_edge_v1 = (BTree*)xResult.addr;
  cout<<"b-tree on edge start node id is built..."<<endl;
  /*********************b-tree on edge end node id******************/
  ostringstream xThisBusEdgeV2PtrStream;
  xThisBusEdgeV2PtrStream<<(long)bus_edge;
  strQuery = "(createbtree (" + busedgeTypeInfo +
             "(ptr " + xThisBusEdgeV2PtrStream.str() + "))" + "v2)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bus_edge_v2 = (BTree*)xResult.addr;
  cout<<"b-tree on edge end node id is built..."<<endl;
}
void BusNetwork::Load(int in_iId,const Relation* in_busRoute)
{
  busnet_id = in_iId;
  bus_def = true;
  assert(in_busRoute != NULL);
  FillBusNode(in_busRoute);//for node
  FillBusWeightAndEdge(in_busRoute);//for weight and edge
}

/*
2 Type Constructors and Deconstructor

*/
BusNetwork::BusNetwork():
busnet_id(0),bus_def(false),bus_route(NULL),btree_bus_route(NULL),
bus_node(NULL),btree_bus_node(NULL),
rtree_bus_node(NULL),bus_weight(NULL),btree_bus_weight(NULL),bus_edge(NULL),
btree_bus_edge(NULL),btree_bus_edge_v1(NULL),btree_bus_edge_v2(NULL)
{

}

void BusNetwork::Destory()
{
  //bus route
  if(bus_route != NULL){
    bus_route->Delete();
    bus_route = NULL;
  }
  //b-tree on bus route
  if(btree_bus_route != NULL){
    delete btree_bus_route;
    btree_bus_route = NULL;
  }
 //bus stop
  if(bus_node != NULL){
    bus_node->Delete();
    bus_node = NULL;
  }
  //b-tree on bus stop
  if(btree_bus_node != NULL){
    delete btree_bus_node;
    btree_bus_node = NULL;
  }
  //r-tree on bus stop
  if(rtree_bus_node != NULL){
    delete rtree_bus_node;
    rtree_bus_node = NULL;
  }
  //bus weight function
  if(bus_weight != NULL){
    bus_weight->Delete();
    bus_weight = NULL;
  }
  //b-tree on weight function
  if(btree_bus_weight != NULL){
    delete btree_bus_weight;
    btree_bus_weight = NULL;
  }
  //bus edge relation
  if(bus_edge != NULL){
    bus_edge->Delete();
    bus_edge = NULL;
  }
  //b-tree on bus edge id
  if(btree_bus_edge != NULL){
    delete btree_bus_edge;
    btree_bus_edge = NULL;
  }
  //b-tree on bus edge start node id
  if(btree_bus_edge_v1 != NULL){
    delete btree_bus_edge_v1;
    btree_bus_edge_v1 = NULL;
  }
  //b-tree on bus edge end node id
  if(btree_bus_edge_v2 != NULL){
    delete btree_bus_edge_v2;
    btree_bus_edge_v2 = NULL;
  }
}

BusNetwork::BusNetwork(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
//  cout<<"BusNetwork() 2"<<endl;
  /***********************Read busnetwork id********************************/
  in_xValueRecord.Read(&busnet_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);
  /************************Open relation for bus routes*********************/
  ListExpr xType;
  nl->ReadFromString(busrouteTypeInfo,xType);
  ListExpr xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bus_route = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!bus_route) return;

  /***********************Open b-tree on bus route**************************/
  nl->ReadFromString(btreebusrouteTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_route = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_node){
    bus_route->Delete();
    return;
  }

///////// Test B-tree on bus route id //////////////////////
/*  for(int i = 1;i <= bus_route->GetNoTuples();i++){
    CcInt* id = new CcInt(true,i);
    BTreeIterator* btreeiter = btree_bus_node->ExactMatch(id);
    while(btreeiter->Next()){
      Tuple* tuple = bus_route->GetTuple(btreeiter->GetId());
      cout<<*tuple<<endl;
      tuple->DeleteIfAllowed();
    }
    delete id;
    delete btreeiter;
  }*/


  /***************************Open relation for nodes*********************/
  nl->ReadFromString(busstopTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bus_node = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!bus_node) {
    bus_route->Delete();
    delete btree_bus_route;
    return;
  }

  /***********************Open btree for bus stop************************/
  nl->ReadFromString(btreebusstopTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_node = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_node){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    return;
  }
///////// Test B-tree on Nodes//////////////////////
/*  for(int i = 1;i <= bus_node->GetNoTuples();i++){
    CcInt* id = new CcInt(true,i);
    BTreeIterator* btreeiter = btree_bus_node->ExactMatch(id);
    while(btreeiter->Next()){
      Tuple* tuple = bus_node->GetTuple(btreeiter->GetId());
      cout<<*tuple<<endl;
      tuple->DeleteIfAllowed();
    }
    delete id;
    delete btreeiter;
  }*/
///////////////////////////////////////////////////

/***************************Open R-tree for bus stop************************/
  Word xValue;
  if(!(rtree_bus_node->Open(in_xValueRecord,inout_iOffset,
                          rtreebusstopTypeInfo,xValue))){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    return;
  }
  rtree_bus_node = (R_Tree<2,TupleId>*)xValue.addr;
////////////////////Test R-tree ////////////////////////////////
//  cout<<rtree_bus_node->NodeCount()<<endl;
//  rtree_bus_node->BoundingBox().Print(cout);


/////////////////////////////////////////////////////////////////

 /************************* weight function**********************************/

  nl->ReadFromString(busweightTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bus_weight = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!bus_weight){
   bus_route->Delete();
   delete btree_bus_route;
   bus_node->Delete();
   delete btree_bus_node;
   delete rtree_bus_node;
   return;
  }
  /******************Open b-tree for weight function**********************/
  nl->ReadFromString(btreebusweightTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_weight = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_weight){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    bus_weight->Delete();
    return;
  }

///////// Test B-tree on Weight Functions//////////////////////
/*  for(int i = 1;i <= bus_weight->GetNoTuples();i++){
    CcInt* id = new CcInt(true,i);
    BTreeIterator* btreeiter = btree_bus_weight->ExactMatch(id);
    while(btreeiter->Next()){
      Tuple* tuple = bus_weight->GetTuple(btreeiter->GetId());
//      cout<<*tuple<<endl;
      Periods* peri = (Periods*)tuple->GetAttribute(DEF_T);
      const Interval<Instant>* interval;
      peri->Get(0,interval);
      interval->Print(cout);
      tuple->DeleteIfAllowed();
    }
    delete id;
    delete btreeiter;
  }*/
///////////////////////////////////////////////////

  /****************************edges*********************************/
  nl->ReadFromString(busedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bus_edge = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!bus_edge){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    bus_weight->Delete();
    delete btree_bus_weight;
    return;
  }

  /*************************b-tree on edges (eid)****************************/
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_edge = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_edge){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    bus_weight->Delete();
    delete btree_bus_weight;
    bus_edge->Delete();
    return;
  }
///////// Test B-tree on Edge //////////////////////
/*  for(int i = 1;i <= bus_edge->GetNoTuples();i++){
    CcInt* id = new CcInt(true,i);
    BTreeIterator* btreeiter = btree_bus_edge->ExactMatch(id);
    while(btreeiter->Next()){
      Tuple* tuple = bus_edge->GetTuple(btreeiter->GetId());
      cout<<*tuple<<endl;
      tuple->DeleteIfAllowed();
    }
    delete id;
    delete btreeiter;
  }*/
  /***********************b-tree on edge start node id************************/
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_edge_v1 = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_edge_v1){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    bus_weight->Delete();
    delete btree_bus_weight;
    bus_edge->Delete();
    delete btree_bus_edge;
    return;
  }

///////// Test B-tree on edge start node id //////////////////////
/*  for(int i = 1;i <= bus_node->GetNoTuples();i++){
    CcInt* id = new CcInt(true,i);
    BTreeIterator* btreeiter = btree_bus_edge_v1->ExactMatch(id);
    while(btreeiter->Next()){
      Tuple* tuple = bus_edge->GetTuple(btreeiter->GetId());
      cout<<*tuple<<endl;
      tuple->DeleteIfAllowed();
    }
    delete id;
    delete btreeiter;
  }*/

  /***********************b-tree on edge end node id************************/
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_edge_v2 = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_edge_v2){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    bus_weight->Delete();
    delete btree_bus_weight;
    bus_edge->Delete();
    delete btree_bus_edge;
    delete btree_bus_edge_v1;
    return;
  }
  ///////// Test B-tree on edge start node id //////////////////////
/*  for(int i = 1;i <= bus_node->GetNoTuples();i++){
    CcInt* id = new CcInt(true,i);
    BTreeIterator* btreeiter = btree_bus_edge_v2->ExactMatch(id);
    while(btreeiter->Next()){
      Tuple* tuple = bus_edge->GetTuple(btreeiter->GetId());
      cout<<*tuple<<endl;
      tuple->DeleteIfAllowed();
    }
    delete id;
    delete btreeiter;
  }*/

  bus_def = true;
}


BusNetwork::BusNetwork(ListExpr in_xValue,int in_iErrorPos,
ListExpr& inout_xErrorInfo,bool& inout_bCorrect):
busnet_id(0),bus_def(false),bus_route(NULL),btree_bus_route(NULL),
bus_node(NULL),btree_bus_node(NULL),
rtree_bus_node(NULL),bus_weight(NULL),btree_bus_weight(NULL),bus_edge(NULL),
btree_bus_edge(NULL),btree_bus_edge_v1(NULL),btree_bus_edge_v2(NULL)
{
  cout<<"BusNetwork() 3"<<endl;
  if(!(nl->ListLength(in_xValue) == 2)){
    string strErrorMessage = "BusNetwork(): List length must be 2";
    inout_xErrorInfo =
      nl->Append(inout_xErrorInfo,nl->StringAtom(strErrorMessage));
      inout_bCorrect = false;
      return;
  }
  //Get type-info fro temporary table

  //Split into the two parts
  ListExpr xIdList = nl->First(in_xValue);
  ListExpr xRouteList = nl->Second(in_xValue);

  //Read Id
  if(!nl->IsAtom(xIdList) || nl->AtomType(xIdList) != IntType) {
    string strErrorMessage = "BusNetwork(): Id is missing)";
    inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                       nl->StringAtom(strErrorMessage));
    inout_bCorrect = false;
    return;
  }
  busnet_id = nl->IntValue(xIdList);
  cout<<"bus id "<<busnet_id<<endl;
  bus_def = true;
}

