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

string BusNetwork::busrouteTypeInfo =
                    "(rel(tuple((Id int)(LineNo int)(Trip mpoint))))";
string BusNetwork::btreebusrouteTypeInfo =
                    "(btree(tuple((Id int)(LineNo int)(Trip mpoint))) int)";
string BusNetwork::busstopTypeInfo = "(rel(tuple((Id int)(BusStop point))))";
string BusNetwork::btreebusstopTypeInfo =
                  "(btree(tuple((Id int)(BusStop point))) int)";
string BusNetwork::rtreebusstopTypeInfo =
                  "(rtree(tuple((Id int)(BusStop point))) BusStop FALSE)";
string s1 = "(rel(tuple((eid int)(v1 int)(v2 int)";
string s2 = "(def_t periods)(l line)(fee real)(rid int)(trip mpoint)";
string s3 = "(pid int)(p1 point)(p2 point))))";
string BusNetwork::busedgeTypeInfo = s1+s2+s3;

string bs1 = "(btree(tuple((eid int)(v1 int)(v2 int)";
string bs2 = "(def_t periods)(l line)(fee real)(rid int)(trip mpoint)";
string bs3 = "(pid int)(p1 point)(p2 point))) int)";
string BusNetwork::btreebusedgeTypeInfo = bs1+bs2+bs3;


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
  cout<<"DeleteBusNetwork"<<endl;
  BusNetwork* p = (BusNetwork*)w.addr;
  p->Destory();
  delete p;
}

ListExpr BusNetwork::Out(ListExpr typeInfo)
{

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

  in_xValueRecord.Write(&maxspeed,sizeof(double),inout_iOffset);
  inout_iOffset += sizeof(double);

  ListExpr xType;
  ListExpr xNumericType;
  /************************save bus routes****************************/
  nl->ReadFromString(busrouteTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!bus_route->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;
  /************************save b-tree on bus routes***********************/
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

  ///////////////////////adjacency list /////////////////////////////////
  SmiFileId fileId = 0;
  adjacencylist_index.SaveToRecord(in_xValueRecord, inout_iOffset, fileId);
  adjacencylist.SaveToRecord(in_xValueRecord, inout_iOffset, fileId);

  return true;
}

void BusNetwork::CloseBusNetwork(const ListExpr typeInfo,Word& w)
{
//  cout<<"CloseBusNetwork"<<endl;
  delete static_cast<BusNetwork*>(w.addr);

  w.addr = NULL;
}
BusNetwork:: ~BusNetwork()
{
//  cout<<"~BusNetwork"<<endl;
  //bus route
  if(bus_route != NULL)
    bus_route->Close();

  //b-tree on bus route
  if(btree_bus_route != NULL)
    delete btree_bus_route;

  //bus stop
  if(bus_node != NULL)
      bus_node->Close();
  //b-tree on bus stop
  if(btree_bus_node != NULL)
      delete btree_bus_node;
  //r-tree on bus stop
  if(rtree_bus_node != NULL)
      delete rtree_bus_node;

  //bus edge relation
  if(bus_edge != NULL)
     bus_edge->Close();
  //b-tree on edge id
  if(btree_bus_edge != NULL)
      delete btree_bus_edge;
  //b-tree on edge start node id
  if(btree_bus_edge_v1 != NULL)
      delete btree_bus_edge_v1;
  //b-tree on edge end node id
  if(btree_bus_edge_v2 != NULL)
     delete btree_bus_edge_v2;

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

  ListExpr xTypeInfoEdge;
  nl->ReadFromString(busrouteTypeInfo,xTypeInfoEdge);
  ListExpr xNumType2 =
                SecondoSystem::GetCatalog()->NumericType(xTypeInfoEdge);
  Relation* temp_bus_route = new Relation(xNumType2,true);


  for(int i = 1; i <=  in_busRoute->GetNoTuples();i++){
    Tuple* tuple = in_busRoute->GetTuple(i);

    temp_bus_route->AppendTuple(tuple);

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
  /***************** bus route **************************/
  ostringstream xBusRoutePtrStream;
  xBusRoutePtrStream << (long)temp_bus_route;
  string strQuery = "(consume(sort(feed(" + busrouteTypeInfo +
                "(ptr " + xBusRoutePtrStream.str() + ")))))";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  bus_route = (Relation*)xResult.addr;
  temp_bus_route->Delete();
  cout<<"bus route is finished....."<<endl;

  /****************b-tree on bus route*****************/
  ostringstream xThisRoutePtrStream;
  xThisRoutePtrStream << (long)bus_route;
  strQuery = "(createbtree (" + busrouteTypeInfo +
             "(ptr " + xThisRoutePtrStream.str() + "))" + "Id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bus_route = (BTree*)xResult.addr;
  cout<<"b-tree on bus route is finished....."<<endl;


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
TupleId BusNetwork::FindPointTid(Point& p)
{
  BBox<2> searchbox = p.BoundingBox();
  R_TreeLeafEntry<2,TupleId> e;
  if(rtree_bus_node->First(searchbox,e))
    return e.info;
  return 0;
}
/*
Create the relation for storing edges

*/
void BusNetwork::FillBusEdge(const Relation* in_busRoute)
{
  cout<<"create edge relation..."<<endl;
  vector<Point> endpoints;//store start and end pointss
  vector<Point> connectpoints; //store points between two stops
  Interval<Instant> timeInterval;

  //relation description for edge

  ListExpr xTypeInfoEdge;
  nl->ReadFromString(busedgeTypeInfo,xTypeInfoEdge);
  ListExpr xNumType2 =
                SecondoSystem::GetCatalog()->NumericType(xTypeInfoEdge);
  Relation* temp_bus_edge = new Relation(xNumType2,true);

  int eid = 1;
  for(int i = 1; i <=  in_busRoute->GetNoTuples();i++){
    Tuple* tuple = in_busRoute->GetTuple(i);
    MPoint* trip = (MPoint*)tuple->GetAttribute(TRIP);
    const UPoint* up;
    CcInt* rid = (CcInt*)tuple->GetAttribute(LINENO);//bus line no
    int routeid = rid->GetIntval();
    CcInt* rpid = (CcInt*)tuple->GetAttribute(RID);//path id

    endpoints.clear();
    connectpoints.clear();
    /////generate cost fee /////
    srand(time(0));
    float costfee = (10+rand() % 20)/100.0;
//    cout<<costfee<<endl;
    srand(i);
    //////////
    int start_index=0;
    int end_index=0;
//    cout<<*trip<<endl;
    for(int j = 0;j < trip->GetNoComponents();j++){
      trip->Get(j,up);
      Point p0 = up->p0;
      Point p1 = up->p1;

      if(j == 0 ){ //add the start location as a stop
        endpoints.push_back(p0);
        connectpoints.push_back(p0);
        start_index = j;
        if(AlmostEqual(p0,p1))
          start_index++;
        if(!AlmostEqual(p0,p1))
          connectpoints.push_back(p1);

        continue;
      }

      if(AlmostEqual(p0,p1)){
        endpoints.push_back(p0);

        if(endpoints.size() == 2){ //extract edge
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
          TupleId tid1 = FindPointTid(endpoints[0]);
          TupleId tid2 = FindPointTid(endpoints[1]);
//          cout<<id1<<" "<<id2<<endl;
          assert(tid1 != tid2 && tid1 != 0 && tid2 != 0);
//          cout<<endpoints[0]<<" "<<endpoints[1]<<endl;
          Tuple* tuple_p1 = bus_node->GetTuple(tid1);
          Tuple* tuple_p2 = bus_node->GetTuple(tid2);
//          cout<<*tuple_p1<<" "<<*tuple_p2<<endl;
          Point* p1 = (Point*)tuple_p1->GetAttribute(LOC);
          Point* p2 = (Point*)tuple_p2->GetAttribute(LOC);
          int nid1 = ((CcInt*)tuple_p1->GetAttribute(SID))->GetIntval();
          int nid2 = ((CcInt*)tuple_p2->GetAttribute(SID))->GetIntval();
          assert(AlmostEqual(*p1,endpoints[0])&&AlmostEqual(*p2,endpoints[1]));


          Tuple* edgetuple = new Tuple(nl->Second(xNumType2));

///////////////////////////////////////////////////////////////////////////
          end_index = j;

          trip->Get(start_index,up);
          timeInterval.start = up->timeInterval.start;
          trip->Get(end_index,up);
          timeInterval.end = up->timeInterval.start;
          Periods* peri = new Periods(1);
          peri->StartBulkLoad();
          peri->Add(timeInterval);
          peri->EndBulkLoad(true);

          MPoint* temp_mp = new MPoint(0);
          temp_mp->StartBulkLoad();
          for(;start_index < end_index;start_index++){//no static behavior
            trip->Get(start_index,up);
            temp_mp->Add(*up);
          }
          temp_mp->EndBulkLoad(true);

//          cout<<timeInterval<<endl;
//          cout<<*temp_mp<<endl;
/////////////////////////////////////////////////////////////////////////////
          edgetuple->PutAttribute(EID,new CcInt(true,eid));
          edgetuple->PutAttribute(V1, new CcInt(true,nid1));
          edgetuple->PutAttribute(V2, new CcInt(true,nid2));

          edgetuple->PutAttribute(DEF_T, peri);
          edgetuple->PutAttribute(LINE, new Line(*line));
          edgetuple->PutAttribute(FEE,new CcReal(true,costfee));
          edgetuple->PutAttribute(PID,new CcInt(true,routeid));
          edgetuple->PutAttribute(MOVE,temp_mp);
          edgetuple->PutAttribute(RPID,new CcInt(*rpid));
          edgetuple->PutAttribute(P1,new Point(*p1));
          edgetuple->PutAttribute(P2,new Point(*p2));
          temp_bus_edge->AppendTuple(edgetuple);
          tuple_p1->DeleteIfAllowed();
          tuple_p2->DeleteIfAllowed();
          edgetuple->DeleteIfAllowed();
          eid++;
          delete line;
//          temp_mp->Clear();
          start_index = j+1; //no static behavior
//////////////////////////////////////////////////////////////////////////
          endpoints.clear();
          connectpoints.clear();

          endpoints.push_back(p0);//next line
          connectpoints.push_back(p0);
        }else{
          //the prorgram should never come here
          assert(false);
        }
      }else{ //not AlmostEqual
        //add points
         connectpoints.push_back(p1);

         if(j == trip->GetNoComponents() - 1){ //add a trip without middle stop
          endpoints.push_back(p1);

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
          TupleId tid1 = FindPointTid(endpoints[0]);
          TupleId tid2 = FindPointTid(endpoints[1]);
//          cout<<id1<<" "<<id2<<endl;
          assert(tid1 != tid2);
//          cout<<endpoints[0]<<" "<<endpoints[1]<<endl;
          Tuple* tuple_p1 = bus_node->GetTuple(tid1);
          Tuple* tuple_p2 = bus_node->GetTuple(tid2);
//          cout<<*tuple_p1<<" "<<*tuple_p2<<endl;
          Point* p1 = (Point*)tuple_p1->GetAttribute(LOC);
          Point* p2 = (Point*)tuple_p2->GetAttribute(LOC);
          int nid1 = ((CcInt*)tuple_p1->GetAttribute(SID))->GetIntval();
          int nid2 = ((CcInt*)tuple_p2->GetAttribute(SID))->GetIntval();

          assert(AlmostEqual(*p1,endpoints[0])&&AlmostEqual(*p2,endpoints[1]));

    ///////////////////////////////////////////////////////////////////////////
          end_index = j;
//          cout<<start_index<<" "<<end_index<<endl;
          trip->Get(start_index,up);
          timeInterval.start = up->timeInterval.start;
          trip->Get(end_index,up);
          timeInterval.end = up->timeInterval.end;

          Periods* peri = new Periods(1);
          peri->StartBulkLoad();
          peri->Add(timeInterval);
          peri->EndBulkLoad(true);

          MPoint* temp_mp = new MPoint(0);
          temp_mp->StartBulkLoad();
          for(;start_index <= end_index;start_index++){//no static behavior
            trip->Get(start_index,up);
            temp_mp->Add(*up);
          }
          temp_mp->EndBulkLoad(true);
//          cout<<timeInterval<<endl;
//          cout<<*temp_mp<<endl;
////////////////////////////////////////////////////////////////////////////
          Tuple* edgetuple = new Tuple(nl->Second(xNumType2));
          edgetuple->PutAttribute(EID,new CcInt(true,eid));
          edgetuple->PutAttribute(V1, new CcInt(true,nid1));
          edgetuple->PutAttribute(V2, new CcInt(true,nid2));
          edgetuple->PutAttribute(DEF_T,peri);
          edgetuple->PutAttribute(LINE, new Line(*line));
          edgetuple->PutAttribute(FEE,new CcReal(true,costfee));
          edgetuple->PutAttribute(PID,new CcInt(true,routeid));
          edgetuple->PutAttribute(MOVE,temp_mp);
          edgetuple->PutAttribute(RPID,new CcInt(*rpid));
          edgetuple->PutAttribute(P1,new Point(*p1));
          edgetuple->PutAttribute(P2,new Point(*p2));

          temp_bus_edge->AppendTuple(edgetuple);
          tuple_p1->DeleteIfAllowed();
          tuple_p2->DeleteIfAllowed();
          edgetuple->DeleteIfAllowed();
          eid++;
          delete line;
//////////////////////////////////////////////////////////////////////////
          endpoints.clear();
          connectpoints.clear();
        }
      }
    }
    tuple->DeleteIfAllowed();
  }



///////////////////relation for edge///////////////////////////////////
//  cout<<temp_bus_edge->GetNoTuples()<<endl;
  ostringstream xBusEdgePtrStream;
  xBusEdgePtrStream << (long)temp_bus_edge;
//  string strQuery = "(consume(sort(feed(" + busedgeTypeInfo +
//                "(ptr " + xBusEdgePtrStream.str() + ")))))";

//  string strQuery = "(consume(sortby(feed(" + busedgeTypeInfo +
//         "(ptr " + xBusEdgePtrStream.str() + ")))((v1 asc))))";

  string sq1 = ")))((start_t(fun(tuple1 TUPLE)(minimum(attr tuple1 def_t))))))";
  string sq2 = "((v1 asc)(start_t desc)))(start_t)))";
  string strQuery = "(consume(remove(sortby(extend(feed(" + busedgeTypeInfo +
  "(ptr " + xBusEdgePtrStream.str() + sq1 + sq2;

/*(consume
        (remove
            (sortby
                (extend
                    (feed
                        (busedge berlintrains))
                    (
                        (start_t
                            (fun
                                (tuple1 TUPLE)
                                (minimum
                                    (attr tuple1 def_t))))))
                (
                    (v1 asc)
                    (start_t desc)))
            (start_t))) */

/*"(bulkloadrtree
                (sortby
                      (addid
                            (feed(" + busstopTypeInfo +
            "(ptr " + xBusStop.str() + ")
                                 )
                            )
                      )
                ((BusStop asc))
                )
               BusStop)";*/


//  cout<<strQuery<<endl;

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  bus_edge = (Relation*)xResult.addr;
  temp_bus_edge->Delete();
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
/*
store the adjacency list, for each edge it records which edge comes after it

*/
void BusNetwork::FillAdjacency()
{
  for(int i = 1;i <= bus_edge->GetNoTuples();i++){
    Tuple* t = bus_edge->GetTuple(i);
//    cout<<"i "<<i<<" tuple id "<<t->GetTupleId()<<endl;
    CcInt* end_node = (CcInt*)t->GetAttribute(V2);
    Periods* def_t = (Periods*)t->GetAttribute(DEF_T);
    const Interval<Instant>* interval;
    def_t->Get(0,interval);


    BTreeIterator* btree_iter = btree_bus_edge_v1->ExactMatch(end_node);
    int start = adjacencylist.Size();
    int end = 0;
    while(btree_iter->Next()){
      Tuple* edge_tuple = bus_edge->GetTuple(btree_iter->GetId());

      Periods* def_t_next = (Periods*)edge_tuple->GetAttribute(DEF_T);
      const Interval<Instant>* interval_next;
      def_t_next->Get(0,interval_next);
      if(interval->end < interval_next->start)
        adjacencylist.Append(btree_iter->GetId());
      edge_tuple->DeleteIfAllowed();
    }
    end = adjacencylist.Size();
//    cout<<"start "<<start<<" end "<<end<<endl;
    //[start,end)
    adjacencylist_index.Append(ListEntry(start,end)); //start from 0 = tid - 1

    delete btree_iter;
    t->DeleteIfAllowed();
  }
}

void BusNetwork::Load(int in_iId,const Relation* in_busRoute)
{
  busnet_id = in_iId;
  bus_def = true;
  assert(in_busRoute != NULL);
  FillBusNode(in_busRoute);//for node
  FillBusEdge(in_busRoute);//for edge
  CalculateMaxSpeed();//calculate the maxspeed
  FillAdjacency();
}

/*
2 Type Constructors and Deconstructor

*/
BusNetwork::BusNetwork():
busnet_id(0),bus_def(false),bus_route(NULL),btree_bus_route(NULL),
bus_node(NULL),btree_bus_node(NULL),rtree_bus_node(NULL),bus_edge(NULL),
btree_bus_edge(NULL),btree_bus_edge_v1(NULL),btree_bus_edge_v2(NULL),
maxspeed(0),adjacencylist_index(0),adjacencylist(0)
{

}

void BusNetwork::Destory()
{
  cout<<"destory"<<endl;
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
  in_xValueRecord.Read(&maxspeed,sizeof(double),inout_iOffset);
  inout_iOffset += sizeof(double);

  ListExpr xType;
  ListExpr xNumericType;
  /***********************Open relation for bus routes*********************/
//  cout<<"open bus_route"<<endl;
  nl->ReadFromString(busrouteTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bus_route = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!bus_route) {
    return;
  }

  /*******************b-tree on bus routes********************************/
//  cout<<"open b-tree on bus route"<<endl;
  nl->ReadFromString(btreebusrouteTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_route = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_route) {
    bus_route->Delete();
    return;
  }

  /***************************Open relation for nodes*********************/
//  cout<<"open bus_node"<<endl;
  nl->ReadFromString(busstopTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bus_node = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!bus_node) {
    bus_route->Delete();
    delete btree_bus_route;
    return;
  }

  /***********************Open btree for bus stop************************/
//  cout<<"open btree bus_node"<<endl;
  nl->ReadFromString(btreebusstopTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_node = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_node){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    return ;
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
//  cout<<"open rtree bus_node"<<endl;
  Word xValue;
  if(!(rtree_bus_node->Open(in_xValueRecord,inout_iOffset,
                          rtreebusstopTypeInfo,xValue))){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    return ;
  }
  rtree_bus_node = (R_Tree<2,TupleId>*)xValue.addr;
////////////////////Test R-tree ////////////////////////////////
//  cout<<rtree_bus_node->NodeCount()<<endl;
//  rtree_bus_node->BoundingBox().Print(cout);


  /****************************edges*********************************/
//  cout<<"open bus_edge"<<endl;
  nl->ReadFromString(busedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bus_edge = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!bus_edge){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    return ;
  }

  /*************************b-tree on edges (eid)****************************/
//  cout<<"open b-tree bus_edge id"<<endl;
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_edge = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_edge){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    bus_edge->Delete();
    return ;
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
//  cout<<"open b-tree bus_edge nid1"<<endl;
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_edge_v1 = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_edge_v1){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    bus_edge->Delete();
    delete btree_bus_edge;
    return ;
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
//  cout<<"open b-tree bus_edge nid2"<<endl;
  nl->ReadFromString(btreebusedgeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_edge_v2 = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_edge_v2){
    bus_route->Delete();
    delete btree_bus_route;
    bus_node->Delete();
    delete btree_bus_node;
    delete rtree_bus_node;
    bus_edge->Delete();
    delete btree_bus_edge;
    delete btree_bus_edge_v1;
    return ;
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
  ///////////////adjacency list ////////////////////////////////////
  adjacencylist_index.OpenFromRecord(in_xValueRecord, inout_iOffset);
  adjacencylist.OpenFromRecord(in_xValueRecord, inout_iOffset);

  bus_def = true;
//  cout<<"maxspeed "<<maxspeed<<endl;
}
void BusNetwork::CalculateMaxSpeed()
{
  maxspeed = numeric_limits<float>::min();
  for(int i = 1;i <= bus_route->GetNoTuples();i++){
    Tuple* t = bus_route->GetTuple(i);
    MPoint* trip = (MPoint*)t->GetAttribute(TRIP);
    for(int j = 0;j < trip->GetNoComponents();j++){
      const UPoint* up;
      trip->Get(j,up);
      double t = (up->timeInterval.end - up->timeInterval.start).ToDouble();
      double dist = up->p0.Distance(up->p1);
      if(dist/t > maxspeed)
        maxspeed = dist/t;
    }
    t->DeleteIfAllowed();
  }
//  cout<<"max speed "<<maxspeed<<endl;
}

BusNetwork::BusNetwork(ListExpr in_xValue,int in_iErrorPos,
ListExpr& inout_xErrorInfo,bool& inout_bCorrect):
busnet_id(0),bus_def(false),bus_route(NULL),btree_bus_route(NULL),
bus_node(NULL),btree_bus_node(NULL),rtree_bus_node(NULL),bus_edge(NULL),
btree_bus_edge(NULL),btree_bus_edge_v1(NULL),btree_bus_edge_v2(NULL),
maxspeed(0),adjacencylist_index(0),adjacencylist(0)
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
//  ListExpr xRouteList = nl->Second(in_xValue);

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
/****************************Application Function***************************/

/*
Check whether it is reachable from a start node to an end node

*/
struct Elem{
  int edge_tuple_id;
  int pre_eid;
  double dist; //network distance to start node + Euclidan distance to end node
  Interval<Instant> interval;
  int s_node_id;
  int e_node_id;
  double delta_t;
  int rid;
  int pre_edge_tid;
  int rpid;
  Elem(int id,Interval<Instant> interv,int snid):edge_tuple_id(id),
  interval(interv),s_node_id(snid),e_node_id(snid)
  {
    pre_eid=-1;
    dist=0.0;
    delta_t=0.0;
    rid=0;
    pre_edge_tid = 0;
    rpid = 0;
  }
  Elem(const Elem& e):
  edge_tuple_id(e.edge_tuple_id),pre_eid(e.pre_eid),
  dist(e.dist),interval(e.interval),
  s_node_id(e.s_node_id),e_node_id(e.e_node_id),
  delta_t(e.delta_t),rid(e.rid),pre_edge_tid(e.pre_edge_tid),rpid(e.rpid){}
  Elem& operator=(const Elem& e)
  {
    edge_tuple_id = e.edge_tuple_id;
    pre_eid = e.pre_eid;
    dist = e.dist;
    interval = e.interval;
    s_node_id = e.s_node_id;
    e_node_id = e.e_node_id;
    delta_t = e.delta_t;
    rid = e.rid;
    pre_edge_tid = e.pre_edge_tid;
    rpid = e.rpid;
    return *this;
  }
  bool operator<(const Elem& e)const
  {
      if(delta_t < e.delta_t)
        return false;
      return true;

//    if(AlmostEqual(dist,e.dist)){
//      if(interval.start < e.interval.start)
//        return false;
//      return true;
//    }
//    if(dist < e.dist) return false;
//    return true;
  }
  void Print1()
  {
    cout<<"etid "<<edge_tuple_id<<" pre_eid "<<pre_eid<<
    " time "<<interval<<" delta_t "<<delta_t<<endl;
  }
  void Print2()
  {
    cout<<"etid "<<edge_tuple_id<<
    " time "<<interval<<"rid "<<rid<<" pid "<<rpid<<endl;
  }
};
/* use the time instant*/
class TimeCompare1{
public:
  bool operator()(const Elem& e1, const Elem& e2) const{
    if(e1.interval.start > e2.interval.start) return false;
    if(AlmostEqual(e1.interval.start.ToDouble(), e2.interval.start.ToDouble()))
      if(e1.interval.end > e2.interval.end)
        return false;
    return true;
  }
};

void BusNetwork::FindPath1(int id1,int id2,vector<int>& path,Instant* instant)
{
  ofstream outfile("temp_result"); //record info for debug

  if(id1 < 1 || id1 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return;
  }

  if(id2 < 1 || id2 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return;
  }
//  cout<<"start "<<id1<<" end "<<id2<<endl;
  //find all edges start from node id1, if time interval is given, filter
  //all edges start time earlier than it, no heuristic value

//  priority_queue<Elem,vector<Elem>,TimeCompare> q_list;
  priority_queue<Elem> q_list;
  vector<Elem> expansionlist;
  int expansion_count = 0;
  //Initialize list
  CcInt* start_id = new CcInt(true,id1);
  BTreeIterator* bt_iter_edge_v1 = btree_bus_edge_v1->ExactMatch(start_id);

  while(bt_iter_edge_v1->Next()){
    Tuple* t = bus_edge->GetTuple(bt_iter_edge_v1->GetId());
    CcInt* start_node = (CcInt*)t->GetAttribute(V1);
    CcInt* end_node = (CcInt*)t->GetAttribute(V2);
    Periods* peri = (Periods*)t->GetAttribute(DEF_T);
    const Interval<Instant>* interval;
    peri->Get(0,interval);
    if(path.size() != 0){ //middle stop
        TupleId edge_tid = path[path.size()-1];
        Tuple* edge_tuple = bus_edge->GetTuple(edge_tid);
        Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
        const Interval<Instant>* interval_cur;
        cur_def_t->Get(0,interval_cur);
        //if user defined time instant, one more compare condition should be
        //considered
        //if ui1->lc == true

        if(interval->start > interval_cur->end){
            Elem e(bt_iter_edge_v1->GetId(),*interval,
                   start_node->GetIntval());
            e.delta_t = (interval->end-interval_cur->end).ToDouble();
            e.e_node_id = end_node->GetIntval();
            q_list.push(e);
        }
        edge_tuple->DeleteIfAllowed();
    }else{
//      if(interval->start > ui1->timeInterval.start){
      if(interval->start > *instant){
        Elem e(bt_iter_edge_v1->GetId(),*interval,
                  start_node->GetIntval());
//        e.delta_t = (interval->end-ui1->timeInterval.start).ToDouble();
        e.delta_t = (interval->end-*instant).ToDouble();
        e.e_node_id = end_node->GetIntval();
        q_list.push(e);
      }
    }
    t->DeleteIfAllowed();
  }

  delete bt_iter_edge_v1;
  delete start_id;
  cout<<"initialize size "<<q_list.size()<<endl;

//////////////////////////////////////////////////////////////////////////

  while(q_list.empty() == false){
    Elem top = q_list.top();
//    Tuple* edge_tuple = bus_edge->GetTuple(top.edge_tuple_id);
//    top.Print();

//    CcInt* start_node = (CcInt*)edge_tuple->GetAttribute(V1);
//    CcInt* end_node = (CcInt*)edge_tuple->GetAttribute(V2);
      CcInt* start_node = new CcInt(true,top.s_node_id);
      CcInt* end_node = new CcInt(true,top.e_node_id);

    if(end_node->GetIntval() == id2){//find the end
//      cout<<"find "<<endl;
      delete start_node;
      delete end_node;
      break;
    }

    expansionlist.push_back(top);
    expansion_count++;
    q_list.pop();

//    Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
//    const Interval<Instant>* interval_cur;
//    cur_def_t->Get(0,interval_cur);

    Interval<Instant>* interval_cur = &top.interval;

//    cout<<"edge "<<edge_id->GetIntval()<<" v1 "<<start_node->GetIntval()
//        <<" v2 "<<end_node->GetIntval()<<" time "<<*interval_cur<<endl;

    /////////////get all edges from the same start node////////////////
    bt_iter_edge_v1 = btree_bus_edge_v1->ExactMatch(end_node);
    while(bt_iter_edge_v1->Next()){
     Tuple* t = bus_edge->GetTuple(bt_iter_edge_v1->GetId());

     CcInt* start_node_next = (CcInt*)t->GetAttribute(V1);//start node
     CcInt* end_node_next = (CcInt*)t->GetAttribute(V2);//end node
     Periods* next_def_t = (Periods*)t->GetAttribute(DEF_T);

     const Interval<Instant>* interval_next;
     next_def_t->Get(0,interval_next);
   //time instant of next edge should be late than cur edge
   //end node id of next edge should not be equal to start node id of cur edge
//     cout<<"next "<<*interval_next<<endl;
//     cout<<*trip<<endl;

      if(interval_next->start > interval_cur->end &&
           end_node_next->GetIntval() != start_node->GetIntval()){
            //store all edges from the same start node
            Elem e(bt_iter_edge_v1->GetId(),*interval_next,
                  start_node_next->GetIntval());
            e.pre_eid = expansion_count - 1;
            e.delta_t = top.delta_t +
                          (interval_next->end-interval_cur->end).ToDouble();
            e.e_node_id = end_node_next->GetIntval();
            q_list.push(e);
        }
        t->DeleteIfAllowed();
      }
      delete bt_iter_edge_v1;
    //////////////////////////////////////////////////////////////////////////
    delete start_node;
    delete end_node;
  }

//  cout<<expansionlist.size()<<endl;
  if(q_list.empty() == false){
    stack<Elem> temp_path;
    Elem top = q_list.top();
    temp_path.push(top);
//    cout<<top.edge_tuple_id<<endl;
    while(top.pre_eid != -1){
      int id = top.pre_eid;
//      cout<<"pre id "<<id<<endl;
      top = expansionlist[id];
//      cout<<top.edge_tuple_id<<endl;
      temp_path.push(top);
    }
//    cout<<temp_path.size()<<endl;
    while(temp_path.empty() == false){
      Elem top = temp_path.top();
      temp_path.pop();
      path.push_back(top.edge_tuple_id);
    }
  }

}

/* expand the graph by Dijkstra with minimum total time cost so far*/
void BusNetwork::FindPath_T_1(MPoint* mp,Relation* query,int attrpos,
Instant* instant)
{
//  cout<<"BusNetwork::Reachability"<<endl;
  if(query->GetNoTuples() < 2){
    cout<<"there is only start location, please give destination"<<endl;
    return;
  }

//  MPoint* mp = new MPoint(0);
  mp->Clear();
  mp->StartBulkLoad();

  vector<int> path; //record edge id
  for(int i = 1;i <= query->GetNoTuples() - 1;i++){
    Tuple* t1 = query->GetTuple(i);
    Tuple* t2 = query->GetTuple(i+1);
    CcInt* id1 = (CcInt*)t1->GetAttribute(attrpos);
    CcInt* id2 = (CcInt*)t2->GetAttribute(attrpos);
//    cout<<id1->GetIntval()<<" "<<id2->GetIntval()<<endl;

    FindPath1(id1->GetIntval(),id2->GetIntval(),path,instant);
    t1->DeleteIfAllowed();
    t2->DeleteIfAllowed();

  }
  /****************Construct the Reulst (MPoint)*************************/
  const UPoint* lastup = NULL;
  for(unsigned int i = 0;i < path.size();i++){
//    cout<<path[i]<<" ";
    Tuple* edge_tuple = bus_edge->GetTuple(path[i]);
    MPoint* temp_mp = (MPoint*)edge_tuple->GetAttribute(MOVE);
//    cout<<*temp_mp<<endl;
    for(int j = 0;j < temp_mp->GetNoComponents();j++){
      const UPoint* up;
      temp_mp->Get(j,up);
      //not the first trip
      if(lastup != NULL && i != 0 && j == 0){
        UPoint* insert_up = new UPoint(true);
//        cout<<"last "<<*lastup<<endl;
//        cout<<"cur "<<*up<<endl;
        insert_up->p0 = lastup->p1;
        insert_up->timeInterval.start = lastup->timeInterval.end;
        insert_up->p1 = up->p0;
        insert_up->timeInterval.end = up->timeInterval.start;
//        cout<<"insert "<<*insert_up<<endl;
        insert_up->timeInterval.lc = true;
        insert_up->timeInterval.rc = false;
        mp->Add(*insert_up);
        delete insert_up;
        delete lastup;
        lastup = NULL;
      }
      mp->Add(*up);
      if(j == temp_mp->GetNoComponents() - 1 && i != path.size() - 1)
        lastup = new UPoint(*up);
    }
    edge_tuple->DeleteIfAllowed();
  }
  mp->EndBulkLoad();
}

/*
due to periodic property, use the edge with earliest start time in their route

*/

void BusNetwork::Optimize1(priority_queue<Elem>& q_list,
priority_queue<Elem>& temp_list)
{
  vector<Elem> elem_pop;
  while(temp_list.empty() == false){
    Elem top = temp_list.top();
    temp_list.pop();
    if(elem_pop.size() == 0){
      q_list.push(top);
      elem_pop.push_back(top);
    }else{
      unsigned int i = 0;
      for(;i < elem_pop.size();i++)
       if(elem_pop[i].rid == top.rid && elem_pop[i].e_node_id == top.e_node_id)
          break;
//      cout<<"i "<<i<<" elem_pop size "<<elem_pop.size()<<endl;
      if(i == elem_pop.size()){
        q_list.push(top);
        elem_pop.push_back(top);
      }
    }
  }

}
/*use some optimization technique*/
void BusNetwork::FindPath2(int id1,int id2,vector<int>& path,Instant& instant)
{
  ofstream outfile("temp_result"); //record info for debug

  if(id1 < 1 || id1 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return;
  }

  if(id2 < 1 || id2 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return;
  }
//  cout<<"start "<<id1<<" end "<<id2<<endl;
  //find all edges start from node id1, if time interval is given, filter
  //all edges start time earlier than it, no heuristic value

  //to control that one edge is not expanded more than once
  vector<bool> edge_flag;
  for(int i = 0; i < bus_edge->GetNoTuples() + 1;i++)
    edge_flag.push_back(true);

 //priority_queue<Elem,vector<Elem>,TimeCompare> q_list;
  priority_queue<Elem> q_list;
  vector<Elem> expansionlist;

  priority_queue<Elem> tmp_list; //Optimize--1
  int expansion_count = 0;
  //Initialize list
  CcInt* start_id = new CcInt(true,id1);
  BTreeIterator* bt_iter_edge_v1 = btree_bus_edge_v1->ExactMatch(start_id);

  while(bt_iter_edge_v1->Next()){
    Tuple* t = bus_edge->GetTuple(bt_iter_edge_v1->GetId());
    CcInt* start_node = (CcInt*)t->GetAttribute(V1);
    CcInt* end_node = (CcInt*)t->GetAttribute(V2);
    Periods* peri = (Periods*)t->GetAttribute(DEF_T);
    CcInt* rid = (CcInt*)t->GetAttribute(PID);
    const Interval<Instant>* interval;
    peri->Get(0,interval);

    if(path.size() != 0){ //middle stop
        TupleId edge_tid = path[path.size()-1];
        Tuple* edge_tuple = bus_edge->GetTuple(edge_tid);
        Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
        const Interval<Instant>* interval_cur;
        cur_def_t->Get(0,interval_cur);

        if(interval->start > interval_cur->end){
            Elem e(bt_iter_edge_v1->GetId(),*interval,
                   start_node->GetIntval());
            e.delta_t = (interval->end-interval_cur->end).ToDouble();
            e.e_node_id = end_node->GetIntval();//end node id
            e.pre_edge_tid = 0;
            e.rid = rid->GetIntval();
            if(edge_flag[bt_iter_edge_v1->GetId()]){
              tmp_list.push(e);
              edge_flag[bt_iter_edge_v1->GetId()] = false;
            }

        }
        edge_tuple->DeleteIfAllowed();
    }else{
//      if(interval->start > ui1->timeInterval.start){
      if(interval->start > instant){
        Elem e(bt_iter_edge_v1->GetId(),*interval,
                  start_node->GetIntval());
//        e.delta_t = (interval->end-ui1->timeInterval.start).ToDouble();
        e.delta_t = (interval->end-instant).ToDouble();
        e.e_node_id = end_node->GetIntval();//end node id
        e. pre_edge_tid = 0;
        e.rid = rid->GetIntval();
        if(edge_flag[bt_iter_edge_v1->GetId()]){
          tmp_list.push(e);
          edge_flag[bt_iter_edge_v1->GetId()] = false;
        }
      }
    }
    t->DeleteIfAllowed();
  }

  delete bt_iter_edge_v1;
  delete start_id;
  Optimize1(q_list,tmp_list);

  cout<<"initialize size "<<q_list.size()<<endl;
//////////////////////////////////////////////////////////////////////////

  while(q_list.empty() == false){
    Elem top = q_list.top();
//    Tuple* edge_tuple = bus_edge->GetTuple(top.edge_tuple_id);
//    CcInt* start_node = (CcInt*)edge_tuple->GetAttribute(V1);
//    CcInt* end_node = (CcInt*)edge_tuple->GetAttribute(V2);
    CcInt* start_node = new CcInt(true,top.s_node_id);
    CcInt* end_node = new CcInt(true,top.e_node_id);

    if(end_node->GetIntval() == id2){//find the end
      delete start_node;
      delete end_node;
      break;
    }

    expansionlist.push_back(top);
    expansion_count++;
    q_list.pop();

//    Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
//    const Interval<Instant>* interval_cur;
//    cur_def_t->Get(0,interval_cur);

    Interval<Instant>* interval_cur = &top.interval;

//    outfile<<"edge "<<edge_id->GetIntval()<<" pre-edge tid "<<top.pre_edge_tid
//        <<" v1 "<<start_node->GetIntval()
//        <<" v2 "<<end_node->GetIntval()<<" time "<<*interval_cur
//        <<" delta_t "<<top.delta_t<<" rid "<<top.rid<<endl;

    /////////////get all edges from the same start node////////////////
    bt_iter_edge_v1 = btree_bus_edge_v1->ExactMatch(end_node);

    priority_queue<Elem> temp_list; //Optimize--1

    while(bt_iter_edge_v1->Next()){
     Tuple* t = bus_edge->GetTuple(bt_iter_edge_v1->GetId());

     CcInt* start_node_next = (CcInt*)t->GetAttribute(V1);//start node
     CcInt* end_node_next = (CcInt*)t->GetAttribute(V2);//end node
     Periods* next_def_t = (Periods*)t->GetAttribute(DEF_T);

     CcInt* rid = (CcInt*)t->GetAttribute(PID);
     const Interval<Instant>* interval_next;
     next_def_t->Get(0,interval_next);
   //time instant of next edge should be late than cur edge
   //end node id of next edge should not be equal to start node id of cur edge
//     cout<<"next "<<*interval_next<<endl;
//     cout<<*trip<<endl;

      if(interval_next->start > interval_cur->end &&
           end_node_next->GetIntval() != start_node->GetIntval()){
            //store all edges from the same start node
            Elem e(bt_iter_edge_v1->GetId(),*interval_next,
                  start_node_next->GetIntval());
            e.pre_eid = expansion_count - 1;
            e.delta_t = top.delta_t +
                          (interval_next->end-interval_cur->end).ToDouble();
            e.rid = rid->GetIntval();
            e.e_node_id = end_node_next->GetIntval();
            e.pre_edge_tid = top.edge_tuple_id;
//            cout<<e.delta_t<<endl;
//////////////////    Optimize - 1    ////////////////////////////////////////
            if(edge_flag[bt_iter_edge_v1->GetId()]){
              temp_list.push(e);
              edge_flag[bt_iter_edge_v1->GetId()] = false;
            }
            //q_list.push(e);
//////////////////////////////////////////////////////////////////////////////

        }
        t->DeleteIfAllowed();
      }
      delete bt_iter_edge_v1;

      Optimize1(q_list,temp_list);

    //////////////////////////////////////////////////////////////////////////
      delete start_node;
      delete end_node;
  }


  if(q_list.empty() == false){
    stack<Elem> temp_path;
    Elem top = q_list.top();
    temp_path.push(top);
//    cout<<top.edge_tuple_id<<endl;
    while(top.pre_eid != -1){
      int id = top.pre_eid;
//      cout<<"pre id "<<id<<endl;
      top = expansionlist[id];
//      cout<<top.edge_tuple_id<<endl;
      temp_path.push(top);
    }
//    cout<<temp_path.size()<<endl;
    while(temp_path.empty() == false){
      Elem top = temp_path.top();
      temp_path.pop();
      path.push_back(top.edge_tuple_id);
    }
  }
}

/*
expand the graph by Dijkstra with minimum total time cost so far
with some optimization techniques

*/
void BusNetwork::FindPath_T_2(MPoint* mp,Relation* query,int attrpos,
Instant& instant)
{
//  cout<<"BusNetwork::Reachability"<<endl;
  if(query->GetNoTuples() < 2){
    cout<<"there is only start location, please give destination"<<endl;
    return;
  }

//  MPoint* mp = new MPoint(0);
  mp->Clear();
  mp->StartBulkLoad();

  vector<int> path; //record edge id
  for(int i = 1;i <= query->GetNoTuples() - 1;i++){
    Tuple* t1 = query->GetTuple(i);
    Tuple* t2 = query->GetTuple(i+1);
    CcInt* id1 = (CcInt*)t1->GetAttribute(attrpos);
    CcInt* id2 = (CcInt*)t2->GetAttribute(attrpos);

    FindPath2(id1->GetIntval(),id2->GetIntval(),path,instant);
    t1->DeleteIfAllowed();
    t2->DeleteIfAllowed();
  }
  /****************Construct the Result (MPoint)*************************/
  const UPoint* lastup = NULL;
  for(unsigned int i = 0;i < path.size();i++){
//    cout<<path[i]<<" ";
    Tuple* edge_tuple = bus_edge->GetTuple(path[i]);
    MPoint* temp_mp = (MPoint*)edge_tuple->GetAttribute(MOVE);
//    cout<<*temp_mp<<endl;
    for(int j = 0;j < temp_mp->GetNoComponents();j++){
      const UPoint* up;
      temp_mp->Get(j,up);
      //not the first trip
      if(lastup != NULL && i != 0 && j == 0){
        UPoint* insert_up = new UPoint(true);
//        cout<<"last "<<*lastup<<endl;
//        cout<<"cur "<<*up<<endl;
        insert_up->p0 = lastup->p1;
        insert_up->timeInterval.start = lastup->timeInterval.end;
        insert_up->p1 = up->p0;
        insert_up->timeInterval.end = up->timeInterval.start;
//        cout<<"insert "<<*insert_up<<endl;
        insert_up->timeInterval.lc = true;
        insert_up->timeInterval.rc = false;
        mp->Add(*insert_up);
        delete insert_up;
        delete lastup;
        lastup = NULL;
      }
      mp->Add(*up);
      if(j == temp_mp->GetNoComponents() - 1 && i != path.size() - 1)
        lastup = new UPoint(*up);
    }
    edge_tuple->DeleteIfAllowed();
  }
  mp->EndBulkLoad();
}

/*
use some optimization technique, temporal property with middle stop
optimize-1
input relation and b-tree

*/
bool BusNetwork::FindPath3(int id1,int id2,vector<int>& path,
Relation* busedge,BTree* btree1,Instant& queryinstant,double wait_time)
{

//  struct timeb t1;
//  struct timeb t2;

  ofstream outfile("temp_result"); //record info for debug

  if(id1 < 1 || id1 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return false;
  }

  if(id2 < 1 || id2 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return false;
  }
  if(id1 == id2){
    cout<<"two locations are the same"<<endl;
    return false;
  }
  cout<<"start "<<id1<<" end "<<id2<<endl;
  //find all edges start from node id1, if time interval is given, filter
  //all edges start time earlier than it, no heuristic value

  //to control that one edge is not expanded more than once
  vector<bool> edge_flag;
  for(int i = 0; i < busedge->GetNoTuples() + 1;i++)
    edge_flag.push_back(true);

 //priority_queue<Elem,vector<Elem>,TimeCompare> q_list;
  priority_queue<Elem> q_list;
  vector<Elem> expansionlist;

  priority_queue<Elem> tmp_list; //Optimize--1
  int expansion_count = 0;
  //Initialize list
  CcInt* start_id = new CcInt(true,id1);
  BTreeIterator* bt_iter_edge_v1 = btree1->ExactMatch(start_id);

  while(bt_iter_edge_v1->Next()){
    Tuple* t = busedge->GetTuple(bt_iter_edge_v1->GetId());
    CcInt* start_node = (CcInt*)t->GetAttribute(V1);
    CcInt* end_node = (CcInt*)t->GetAttribute(V2);
    Periods* peri = (Periods*)t->GetAttribute(DEF_T);
    CcInt* rid = (CcInt*)t->GetAttribute(PID);
    const Interval<Instant>* interval;
    peri->Get(0,interval);
    if(path.size() != 0){ //middle stop
        TupleId edge_tid = path[path.size()-1];
        Tuple* edge_tuple = busedge->GetTuple(edge_tid);
        Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
        const Interval<Instant>* interval_cur;
        cur_def_t->Get(0,interval_cur);

        //if user defined time instant, one more compare condition should be
        //considered
        //if ui1->lc == true
        Instant depart_time(instanttype);
        depart_time.ReadFrom(wait_time+interval_cur->end.ToDouble());

        //optimize-3, filter edge by start time
        if(interval->start < depart_time){
           edge_tuple->DeleteIfAllowed();
           t->DeleteIfAllowed();
           break;
        }

        if(interval->start > depart_time){
            Elem e(bt_iter_edge_v1->GetId(),*interval,
                   start_node->GetIntval());
            e.delta_t = (interval->end-interval_cur->end).ToDouble();
            e.e_node_id = end_node->GetIntval();//end node id
            e.pre_edge_tid = 0;
            e.rid = rid->GetIntval();
            if(edge_flag[bt_iter_edge_v1->GetId()]){
              tmp_list.push(e);
              edge_flag[bt_iter_edge_v1->GetId()] = false;
            }
        }
        edge_tuple->DeleteIfAllowed();
    }else{
        if(interval->start < queryinstant){
           t->DeleteIfAllowed();
           break;
        }

      if(interval->start > queryinstant){
        Elem e(bt_iter_edge_v1->GetId(),*interval,
                  start_node->GetIntval());

        e.delta_t = (interval->end-queryinstant).ToDouble();
        e.e_node_id = end_node->GetIntval();//end node id
        e. pre_edge_tid = 0;
        e.rid = rid->GetIntval();
        if(edge_flag[bt_iter_edge_v1->GetId()]){
          tmp_list.push(e);
          edge_flag[bt_iter_edge_v1->GetId()] = false;
        }
      }
    }
    t->DeleteIfAllowed();
  }

  delete bt_iter_edge_v1;
  delete start_id;

  Optimize1(q_list,tmp_list);

//  cout<<"initialize size "<<q_list.size()<<endl;
  if(q_list.empty() == true)
    return false;

//////////////////////////////////////////////////////////////////////////

  while(q_list.empty() == false){
    Elem top = q_list.top();

//    Tuple* edge_tuple = busedge->GetTuple(top.edge_tuple_id);
//    CcInt* start_node = (CcInt*)edge_tuple->GetAttribute(V1);
//    CcInt* end_node = (CcInt*)edge_tuple->GetAttribute(V2);

    CcInt* start_node = new CcInt(true,top.s_node_id);
    CcInt* end_node = new CcInt(true,top.e_node_id);

    if(end_node->GetIntval() == id2){//find the end
      delete start_node;
      delete end_node;
      break;
    }

    expansionlist.push_back(top);
    expansion_count++;
    q_list.pop();

//    Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
//    const Interval<Instant>* interval_cur;
//    cur_def_t->Get(0,interval_cur);

     Interval<Instant>* interval_cur = &top.interval;

//    outfile<<"edge "<<edge_id->GetIntval()<<" pre-edge tid "<<top.pre_edge_tid
//        <<" v1 "<<start_node->GetIntval()
//        <<" v2 "<<end_node->GetIntval()<<" time "<<*interval_cur
//        <<" delta_t "<<top.delta_t<<" rid "<<top.rid<<endl;

    /////////////get all edges from the same start node////////////////
//    clock_t start_cpu = clock();
//    ftime(&t1);
    bt_iter_edge_v1 = btree1->ExactMatch(end_node);
    priority_queue<Elem> temp_list; //Optimize--1

    while(bt_iter_edge_v1->Next()){
     Tuple* t = busedge->GetTuple(bt_iter_edge_v1->GetId());

     CcInt* start_node_next = (CcInt*)t->GetAttribute(V1);//start node
     CcInt* end_node_next = (CcInt*)t->GetAttribute(V2);//end node
     Periods* next_def_t = (Periods*)t->GetAttribute(DEF_T);

     CcInt* rid = (CcInt*)t->GetAttribute(PID);
     const Interval<Instant>* interval_next;
     next_def_t->Get(0,interval_next);
   //time instant of next edge should be late than cur edge
   //end node id of next edge should not be equal to start node id of cur edge
//     cout<<"next "<<*interval_next<<endl;
//     cout<<*trip<<endl;
    //optimize-3
    if(interval_next->start < interval_cur->end){
        t->DeleteIfAllowed();
        break;
    }

      if(interval_next->start > interval_cur->end &&
           end_node_next->GetIntval() != start_node->GetIntval()){
            //store all edges from the same start node
            Elem e(bt_iter_edge_v1->GetId(),*interval_next,
                  start_node_next->GetIntval());
            e.pre_eid = expansion_count - 1;
            e.delta_t = top.delta_t +
                          (interval_next->end-interval_cur->end).ToDouble();
            e.rid = rid->GetIntval();
            e.e_node_id = end_node_next->GetIntval();
            e.pre_edge_tid = top.edge_tuple_id;
//            cout<<e.delta_t<<endl;
//////////////////    Optimize - 1    ////////////////////////////////////////
            if(edge_flag[bt_iter_edge_v1->GetId()]){
              temp_list.push(e);
              edge_flag[bt_iter_edge_v1->GetId()] = false;
            }
            //q_list.push(e);
//////////////////////////////////////////////////////////////////////////////

        }
        t->DeleteIfAllowed();
      }
      delete bt_iter_edge_v1;

//      ftime(&t2);
//      clock_t stop_cpu = clock();
//      cout<<"big searching 2 total "<<difftimeb(&t2,&t1)<<" ";
//      cout<<"big searching 2 CPU "<<
//                        ((double)(stop_cpu-start_cpu))/CLOCKS_PER_SEC<<endl;
      Optimize1(q_list,temp_list);

    //////////////////////////////////////////////////////////////////////////
    delete start_node;
    delete end_node;
  }

  if(q_list.empty() == false){
    stack<Elem> temp_path;
    Elem top = q_list.top();
    temp_path.push(top);
//    cout<<top.edge_tuple_id<<endl;
    while(top.pre_eid != -1){
      int id = top.pre_eid;
//      cout<<"pre id "<<id<<endl;
      top = expansionlist[id];
//      cout<<top.edge_tuple_id<<endl;
      temp_path.push(top);
    }
//    cout<<temp_path.size()<<endl;
    while(temp_path.empty() == false){
      Elem top = temp_path.top();
      temp_path.pop();
      path.push_back(top.edge_tuple_id);
    }
  }

  return true;
}

/*
expand the graph by Dijkstra with minimum total time cost so far
with some optimization techniques, supporting time duration for middle stop
optimize-1
input relation and b-tree

*/
void BusNetwork::FindPath_T_3(MPoint* mp,Relation* query,Relation* busedge,
BTree* btree1,int attrpos1,int attrpos2,Instant& instant)
{
  if(query->GetNoTuples() < 2){
    cout<<"there is only start location, please give destination"<<endl;
    return;
  }

  mp->Clear();
  mp->StartBulkLoad();

  vector<int> path; //record edge id

  //searching process

  for(int i = 1;i <= query->GetNoTuples() - 1;i++){
    Tuple* t1 = query->GetTuple(i);
    Tuple* t2 = query->GetTuple(i+1);
    CcInt* id1 = (CcInt*)t1->GetAttribute(attrpos1);
    CcInt* id2 = (CcInt*)t2->GetAttribute(attrpos1);
    DateTime* timestay = (DateTime*)t1->GetAttribute(attrpos2);
//    cout<<id1->GetIntval()<<" "<<id2->GetIntval()<<endl;
//    cout<<*timestay<<endl;
    double waittime = timestay->ToDouble();

   if(FindPath3(id1->GetIntval(),id2->GetIntval(),
              path,busedge,btree1,instant,waittime)==false){
        cout<<"such a route is not valid"<<endl;
        path.clear();
        break;
    }
    t1->DeleteIfAllowed();
    t2->DeleteIfAllowed();
  }
  /****************Construct the Reulst (MPoint)*************************/
  const UPoint* lastup = NULL;
  for(unsigned int i = 0;i < path.size();i++){
//    cout<<path[i]<<" ";
    Tuple* edge_tuple = busedge->GetTuple(path[i]);
    MPoint* temp_mp = (MPoint*)edge_tuple->GetAttribute(MOVE);
//    cout<<*temp_mp<<endl;
    for(int j = 0;j < temp_mp->GetNoComponents();j++){
      const UPoint* up;
      temp_mp->Get(j,up);
      //not the first trip
      if(lastup != NULL && i != 0 && j == 0){
        UPoint* insert_up = new UPoint(true);
//        cout<<"last "<<*lastup<<endl;
//        cout<<"cur "<<*up<<endl;
        insert_up->p0 = lastup->p1;
        insert_up->timeInterval.start = lastup->timeInterval.end;
        insert_up->p1 = up->p0;
        insert_up->timeInterval.end = up->timeInterval.start;
//        cout<<"insert "<<*insert_up<<endl;
        insert_up->timeInterval.lc = true;
        insert_up->timeInterval.rc = false;
        mp->Add(*insert_up);
        delete insert_up;
        delete lastup;
        lastup = NULL;
      }
      mp->Add(*up);
      if(j == temp_mp->GetNoComponents() - 1 && i != path.size() - 1)
        lastup = new UPoint(*up);
    }
    edge_tuple->DeleteIfAllowed();
  }
  mp->EndBulkLoad();
}

/*
use some optimization technique, temporal property with middle stop
optimize-1

optimize-3 filter edge by their start time
edge relation and a b-tree

*/
bool BusNetwork::FindPath4(int id1,int id2,vector<Elem>& path,
Instant& queryinstant,double& wait_time)
{
//  struct timeb t1;
//  struct timeb t2;

  ofstream outfile("temp_result"); //record info for debug

  if(id1 < 1 || id1 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return false;
  }

  if(id2 < 1 || id2 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return false;
  }
  if(id1 == id2){
    cout<<"two locations are the same"<<endl;
    return false;
  }
  cout<<"start "<<id1<<" end "<<id2<<endl;
  //find all edges start from node id1, if time interval is given, filter
  //all edges start time earlier than it, no heuristic value

  //to control that one edge is not expanded more than once
  vector<bool> edge_flag;
//  for(int i = 0; i < busedge->GetNoTuples() + 1;i++)
  for(int i = 0; i < bus_edge->GetNoTuples() + 1;i++)
    edge_flag.push_back(true);

 //priority_queue<Elem,vector<Elem>,TimeCompare> q_list;
  priority_queue<Elem> q_list;
  vector<Elem> expansionlist;

  priority_queue<Elem> tmp_list; //Optimize--1
  int expansion_count = 0;
  //Initialize list
  CcInt* start_id = new CcInt(true,id1);
//  BTreeIterator* bt_iter_edge_v1 = btree1->ExactMatch(start_id);

  BTreeIterator* bt_iter_edge_v1 = btree_bus_edge_v1->ExactMatch(start_id);

  while(bt_iter_edge_v1->Next()){
//    Tuple* t = busedge->GetTuple(bt_iter_edge_v1->GetId());
    Tuple* t = bus_edge->GetTuple(bt_iter_edge_v1->GetId());
    CcInt* start_node = (CcInt*)t->GetAttribute(V1);
    CcInt* end_node = (CcInt*)t->GetAttribute(V2);
    Periods* peri = (Periods*)t->GetAttribute(DEF_T);
    CcInt* rpid = (CcInt*)t->GetAttribute(RPID);
    CcInt* rid = (CcInt*)t->GetAttribute(PID);
    const Interval<Instant>* interval;
    peri->Get(0,interval);
    if(path.size() != 0){ //middle stop
//        TupleId edge_tid = path[path.size()-1];
        TupleId edge_tid = path[path.size()-1].edge_tuple_id;
//        Tuple* edge_tuple = busedge->GetTuple(edge_tid);
        Tuple* edge_tuple = bus_edge->GetTuple(edge_tid);
        Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
        const Interval<Instant>* interval_cur;
        cur_def_t->Get(0,interval_cur);

        //if user defined time instant, one more compare condition should be
        //considered
        //if ui1->lc == true
        Instant depart_time(instanttype);
        depart_time.ReadFrom(wait_time+interval_cur->end.ToDouble());

        //optimize-3, filter edge by start time
        if(interval->start < depart_time){
           edge_tuple->DeleteIfAllowed();
           t->DeleteIfAllowed();
           break;
        }

        if(interval->start > depart_time){
            Elem e(bt_iter_edge_v1->GetId(),*interval,
                   start_node->GetIntval());
            e.delta_t = (interval->end-interval_cur->end).ToDouble();
            e.e_node_id = end_node->GetIntval();//end node id
            e.pre_edge_tid = 0;
            e.rpid = rpid->GetIntval();
            e.rid = rid->GetIntval();
            if(edge_flag[bt_iter_edge_v1->GetId()]){
              tmp_list.push(e);
              edge_flag[bt_iter_edge_v1->GetId()] = false;
//              e.Print2();
            }

        }
        edge_tuple->DeleteIfAllowed();
    }else{
        //optimize-3, filter edge by start time

        if(interval->start < queryinstant){
           t->DeleteIfAllowed();
           break;
        }


      if(interval->start > queryinstant){
        Elem e(bt_iter_edge_v1->GetId(),*interval,
                  start_node->GetIntval());

        e.delta_t = (interval->end-queryinstant).ToDouble();
        e.e_node_id = end_node->GetIntval();//end node id
        e.pre_edge_tid = 0;
        e.rpid = rpid->GetIntval();
        e.rid = rid->GetIntval();
        if(edge_flag[bt_iter_edge_v1->GetId()]){
          tmp_list.push(e);
          edge_flag[bt_iter_edge_v1->GetId()] = false;

        }
      }
    }
    t->DeleteIfAllowed();
  }

  delete bt_iter_edge_v1;
  delete start_id;

  Optimize1(q_list,tmp_list);


  if(q_list.empty() == true)
    return false;

//////////////////////////////////////////////////////////////////////////

  while(q_list.empty() == false){

    Elem top = q_list.top();
//    cout<<"top delta_t "<<top.delta_t<<endl;

    CcInt* start_node = new CcInt(true,top.s_node_id);
    CcInt* end_node = new CcInt(true,top.e_node_id);

    if(end_node->GetIntval() == id2){//find the end
      delete start_node;
      delete end_node;
      break;
    }

    expansionlist.push_back(top);
    expansion_count++;
    q_list.pop();

//    Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
//    const Interval<Instant>* interval_cur;
//    cur_def_t->Get(0,interval_cur);
     Interval<Instant>* interval_cur = &top.interval;

//    outfile<<"edge "<<edge_id->GetIntval()<<" pre-edge tid "<<top.pre_edge_tid
//        <<" v1 "<<start_node->GetIntval()
//        <<" v2 "<<end_node->GetIntval()<<" time "<<*interval_cur
//        <<" delta_t "<<top.delta_t<<" rid "<<top.rid<<endl;

    /////////////get all edges from the same start node////////////////
//    clock_t start_cpu = clock();
//    ftime(&t1);
//    bt_iter_edge_v1 = btree1->ExactMatch(end_node);

    const ListEntry* listentry;
    adjacencylist_index.Get(top.edge_tuple_id-1,listentry);
    int start = listentry->low;
    int high = listentry->high;
    priority_queue<Elem> temp_list; //Optimize--1

    for(int i = start; i < high;i++){

//    bt_iter_edge_v1 = btree_bus_edge_v1->ExactMatch(end_node);


//    while(bt_iter_edge_v1->Next()){
      const int* tuple_id;
      adjacencylist.Get(i,tuple_id);

//      Tuple* t = bus_edge->GetTuple(bt_iter_edge_v1->GetId());
      Tuple* t = bus_edge->GetTuple(*tuple_id);

      CcInt* start_node_next = (CcInt*)t->GetAttribute(V1);//start node
      CcInt* end_node_next = (CcInt*)t->GetAttribute(V2);//end node
      Periods* next_def_t = (Periods*)t->GetAttribute(DEF_T);
      CcInt* rpid = (CcInt*)t->GetAttribute(RPID);
      CcInt* rid = (CcInt*)t->GetAttribute(PID);
      const Interval<Instant>* interval_next;
      next_def_t->Get(0,interval_next);
     //optimize-3  filter edge by their start time
      if(interval_next->start < interval_cur->end){
        t->DeleteIfAllowed();
        break;
      }

      if(interval_next->start > interval_cur->end &&
           end_node_next->GetIntval() != start_node->GetIntval()){
            //store all edges from the same start node
//            Elem e(bt_iter_edge_v1->GetId(),*interval_next,
//                  start_node_next->GetIntval());
            Elem e(*tuple_id,*interval_next,start_node_next->GetIntval());

            e.pre_eid = expansion_count - 1;
            e.delta_t = top.delta_t +
                          (interval_next->end-interval_cur->end).ToDouble();
            e.rid = rid->GetIntval();
            e.e_node_id = end_node_next->GetIntval();
            e.pre_edge_tid = top.edge_tuple_id;
            e.rpid = rpid->GetIntval();

//            if(edge_flag[bt_iter_edge_v1->GetId()]){
//              temp_list.push(e);
//              edge_flag[bt_iter_edge_v1->GetId()] = false;
//            }
            if(edge_flag[*tuple_id]){
              temp_list.push(e);
              edge_flag[*tuple_id] = false;
            }

        }
        t->DeleteIfAllowed();
      }

//      delete bt_iter_edge_v1;

//      ftime(&t2);
//      clock_t stop_cpu = clock();
//      cout<<"big searching 2 total "<<difftimeb(&t2,&t1)<<" ";
//      cout<<"big searching 2 CPU "<<
//                        ((double)(stop_cpu-start_cpu))/CLOCKS_PER_SEC<<endl;
      Optimize1(q_list,temp_list);

    //////////////////////////////////////////////////////////////////////////
    delete start_node;
    delete end_node;
//    cout<<"find_path_t_4 q_list size "<<q_list.size()<<endl;
  }

  if(q_list.empty() == false){
    stack<Elem> temp_path;
    Elem top = q_list.top();
    temp_path.push(top);
    while(top.pre_eid != -1){
      int id = top.pre_eid;
      top = expansionlist[id];
      temp_path.push(top);
    }
    while(temp_path.empty() == false){
      Elem top = temp_path.top();
      temp_path.pop();
//      path.push_back(top.edge_tuple_id);
      path.push_back(top);
    }
  }

  return true;
}

/*
expand the graph by Dijkstra with minimum total time cost so far
with some optimization techniques, supporting time duration for middle stop
optimize-1 filter the edge from the same route but comes later

optimize-3 filter edge by their start time
input edge relation and b-tree

*/
void BusNetwork::TestFunction(Relation* busedge, BTree* btree1)
{
  CcInt* id = new CcInt(true,1);
  BTreeIterator* bt_iter_edge_v1 = btree1->ExactMatch(id);
  while(bt_iter_edge_v1->Next()){
     Tuple* t = busedge->GetTuple(bt_iter_edge_v1->GetId());
     CcInt* eid = (CcInt*)t->GetAttribute(EID);//start node
     cout<<"eid "<<eid->GetIntval()<<endl;
     t->DeleteIfAllowed();
  }
  delete id;
  delete bt_iter_edge_v1;
}

void BusNetwork::FindPath_T_4(MPoint* mp,Relation* query,int attrpos1,
int attrpos2,Instant& queryinstant)
{
  if(query->GetNoTuples() < 2){
    cout<<"there is only start location, please give destination"<<endl;
    return;
  }

  mp->Clear();
  mp->StartBulkLoad();

  vector<Elem> path; //record edge id

  //searching process

//  TestFunction(busedge,btree1);
  for(int i = 1;i <= query->GetNoTuples() - 1;i++){
    Tuple* t1 = query->GetTuple(i);
    Tuple* t2 = query->GetTuple(i+1);
    CcInt* id1 = (CcInt*)t1->GetAttribute(attrpos1);
    CcInt* id2 = (CcInt*)t2->GetAttribute(attrpos1);

    DateTime* timestay = (DateTime*)t1->GetAttribute(attrpos2);
//    cout<<id1->GetIntval()<<" "<<id2->GetIntval()<<endl;
//    cout<<*timestay<<endl;
    double waittime = timestay->ToDouble();

    if(FindPath4(id1->GetIntval(),id2->GetIntval(),path,
           queryinstant,waittime)==false){
        cout<<"such a route is not valid"<<endl;
        path.clear();
        break;
    }
    t1->DeleteIfAllowed();
    t2->DeleteIfAllowed();
  }
  /****************Construct the Reulst (MPoint)*************************/
  const UPoint* lastup = NULL;
  for(unsigned int i = 0;i < path.size();i++){
//    cout<<path[i]<<" ";
//    Tuple* edge_tuple = bus_edge->GetTuple(path[i]);
    Tuple* edge_tuple = bus_edge->GetTuple(path[i].edge_tuple_id);
    MPoint* temp_mp = (MPoint*)edge_tuple->GetAttribute(MOVE);
//    cout<<*temp_mp<<endl;
    for(int j = 0;j < temp_mp->GetNoComponents();j++){
      const UPoint* up;
      temp_mp->Get(j,up);
      //not the first trip
      if(lastup != NULL && i != 0 && j == 0){
        UPoint* insert_up = new UPoint(true);
//        cout<<"last "<<*lastup<<endl;
//        cout<<"cur "<<*up<<endl;
        insert_up->p0 = lastup->p1;
        insert_up->timeInterval.start = lastup->timeInterval.end;
        insert_up->p1 = up->p0;
        insert_up->timeInterval.end = up->timeInterval.start;
//        cout<<"insert "<<*insert_up<<endl;
        insert_up->timeInterval.lc = true;
        insert_up->timeInterval.rc = false;
        mp->Add(*insert_up);
        delete insert_up;
        delete lastup;
        lastup = NULL;
      }
      mp->Add(*up);
      if(j == temp_mp->GetNoComponents() - 1 && i != path.size() - 1)
        lastup = new UPoint(*up);
    }
    edge_tuple->DeleteIfAllowed();
  }
  mp->EndBulkLoad();
}

/*
due to periodic property, use the edge with earliest start time in their route
and use the pre-defined path
optimize-2

*/

void BusNetwork::Optimize2(priority_queue<Elem>& q_list,
priority_queue<Elem>& temp_list,list<Elem>& end_node_edge,double& prune_time)
{
//  cout<<"prune_time "<<prune_time<<endl;
  vector<Elem> elem_pop;
  while(temp_list.empty() == false){
    Elem top = temp_list.top();
    temp_list.pop();

    if(top.delta_t > prune_time)
      break;

//    cout<<"top rpid "<<top.rpid<<endl;
//    top.Print2();
    if(elem_pop.size() == 0){
      if(top.delta_t <= prune_time){
//        cout<<"here 1"<<endl;
        q_list.push(top);
        elem_pop.push_back(top);

        //modify end_node_edge by time, reduce the size of queue
/*        while(top.interval.start > end_node_edge.front().interval.start)
          end_node_edge.pop_front();
        list<Elem>::iterator start = end_node_edge.begin();
        for(;start != end_node_edge.end();start++){
            if(start->rpid == top.rpid){
              double t = top.delta_t +
                (start->interval.end - top.interval.end).ToDouble();
              if(t < prune_time)
                prune_time = t;
              break;
            }
        }*/

      }
    }else{
      unsigned int i = 0;
      for(;i < elem_pop.size();i++)
       if(elem_pop[i].rid == top.rid && elem_pop[i].e_node_id == top.e_node_id)
          break;
//      cout<<"i "<<i<<" elem_pop size "<<elem_pop.size()<<endl;
      if(i == elem_pop.size()){
        if(top.delta_t <= prune_time){
//          cout<<"here 2"<<endl;
          q_list.push(top);
          elem_pop.push_back(top);

          //modify end_node_edge by time, reduce the size of queue
/*        while(top.interval.start > end_node_edge.front().interval.start)
            end_node_edge.pop_front();
            list<Elem>::iterator start = end_node_edge.begin();
            for(;start != end_node_edge.end();start++){
              if(start->rpid == top.rpid){
                double t = top.delta_t +
                  (start->interval.end - top.interval.end).ToDouble();
                if(t < prune_time)
                  prune_time = t;
                break;
              }
            }*/

          }
      }
    }
  }
//  cout<<"temp_list size 2 "<<temp_list.size()<<endl;
}

/*
use some optimization technique, temporal property with middle stop
optimize-1
optimize-2
optimize-3 filter edge by their start time
optimize-4 use A star algorithm, distance / maxspeed
edge relation and a b-tree

*/
bool BusNetwork::FindPath5(int id1,int id2,vector<Elem>& path,
Relation* busedge, BTree* btree1,BTree* btree2,Instant& queryinstant,
double& wait_time)
{
//  struct timeb t1;
//  struct timeb t2;

  ofstream outfile("temp_result"); //record info for debug

  if(id1 < 1 || id1 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return false;
  }

  if(id2 < 1 || id2 > bus_node->GetNoTuples()){
    cout<<"bus id is not valid"<<endl;
    return false;
  }
  if(id1 == id2){
    cout<<"two locations are the same"<<endl;
    return false;
  }
  cout<<"start "<<id1<<" end "<<id2<<endl;


//get the end point
  Point end_point;
  CcInt* end_node_id = new CcInt(true,id2);
  BTreeIterator* bt_iter_edge = btree2->ExactMatch(end_node_id);
  while(bt_iter_edge->Next()){
    Tuple* edge_tuple = busedge->GetTuple(bt_iter_edge->GetId());
    Point* end_p = (Point*)edge_tuple->GetAttribute(P2);
    end_point = *end_p;
    edge_tuple->DeleteIfAllowed();
    break;
  }
  cout<<"end point "<<end_point<<endl;
  delete end_node_id;
  delete bt_iter_edge;

  //find all edges start from node id1, if time interval is given, filter
  //all edges start time earlier than it

  //to control that one edge is not expanded more than once
  vector<bool> edge_flag;
  for(int i = 0; i < busedge->GetNoTuples() + 1;i++)
    edge_flag.push_back(true);

 //priority_queue<Elem,vector<Elem>,TimeCompare> q_list;
  priority_queue<Elem> q_list;
  vector<Elem> expansionlist;

  priority_queue<Elem> tmp_list; //Optimize--1
  int expansion_count = 0;
  vector<Elem> elemlist; //Optimize-1 faster
  //Initialize list
  CcInt* start_id = new CcInt(true,id1);
  BTreeIterator* bt_iter_edge_v1 = btree1->ExactMatch(start_id);

  while(bt_iter_edge_v1->Next()){
    Tuple* t = busedge->GetTuple(bt_iter_edge_v1->GetId());
    CcInt* start_node = (CcInt*)t->GetAttribute(V1);
    CcInt* end_node = (CcInt*)t->GetAttribute(V2);
    Periods* peri = (Periods*)t->GetAttribute(DEF_T);
    CcInt* rpid = (CcInt*)t->GetAttribute(RPID);
    CcInt* rid = (CcInt*)t->GetAttribute(PID);
    Point* endp = (Point*)t->GetAttribute(P2);
    const Interval<Instant>* interval;
    peri->Get(0,interval);
//    cout<<"etid "<<t->GetTupleId()<<" v1 "<<start_node->GetIntval()
//        <<" v2 "<<end_node->GetIntval()<<" rid "<<rid->GetIntval()<<endl;

    if(path.size() != 0){ //middle stop
        const Interval<Instant>* interval_cur = &path[path.size()-1].interval;

        //if user defined time instant, one more compare condition should be
        //considered
        //if ui1->lc == true
        Instant depart_time(instanttype);
        depart_time.ReadFrom(wait_time+interval_cur->end.ToDouble());

        //optimize-3, filter edge by start time
        if(interval->start < depart_time){

           t->DeleteIfAllowed();
           break;
        }

        if(interval->start > depart_time){
            Elem e(bt_iter_edge_v1->GetId(),*interval,
                   start_node->GetIntval());
            e.delta_t = (interval->end-interval_cur->end).ToDouble();
            e.delta_t += endp->Distance(end_point)/maxspeed;

            e.e_node_id = end_node->GetIntval();//end node id
            e.pre_edge_tid = 0;
            e.rpid = rpid->GetIntval();
            e.rid = rid->GetIntval();
            if(edge_flag[bt_iter_edge_v1->GetId()]){
//              tmp_list.push(e);
              edge_flag[bt_iter_edge_v1->GetId()] = false;

        //////////////////////////////////////////////////
              if(elemlist.empty())
                elemlist.push_back(e);
              else{
                unsigned int i = 0;
                for(;i < elemlist.size();i++){
                  if(elemlist[i].rid == e.rid &&
                        elemlist[i].e_node_id == e.e_node_id &&
                        elemlist[i].interval.start > e.interval.start){
                    elemlist[i] = e;
                    break;
                  }
                }
                if(i == elemlist.size())
                  elemlist.push_back(e);
              }
        /////////////////////////////////////////////////

            }
        }

    }else{
        //optimize-3, filter edge by start time
//        if(interval->start < ui1->timeInterval.start){
        if(interval->start < queryinstant){
           t->DeleteIfAllowed();
           break;
        }

//      if(interval->start > ui1->timeInterval.start){
        if(interval->start > queryinstant){
        Elem e(bt_iter_edge_v1->GetId(),*interval,
                  start_node->GetIntval());
//        e.delta_t = (interval->end-ui1->timeInterval.start).ToDouble();
        e.delta_t = (interval->end-queryinstant).ToDouble();
        e.delta_t += endp->Distance(end_point)/maxspeed;

        e.e_node_id = end_node->GetIntval();//end node id
        e.pre_edge_tid = 0;
        e.rpid = rpid->GetIntval();
        e.rid = rid->GetIntval();
        if(edge_flag[bt_iter_edge_v1->GetId()]){
 //         tmp_list.push(e);
          edge_flag[bt_iter_edge_v1->GetId()] = false;

              if(elemlist.empty())
                elemlist.push_back(e);
              else{
                unsigned int i = 0;
                for(;i < elemlist.size();i++){
                  if(elemlist[i].rid == e.rid &&
                        elemlist[i].e_node_id == e.e_node_id &&
                        elemlist[i].interval.start > e.interval.start){//update
                    elemlist[i] = e;
                    break;
                  }
                }
                if(i == elemlist.size())
                  elemlist.push_back(e);
              }
        }
      }
    }
    t->DeleteIfAllowed();
  }

  delete bt_iter_edge_v1;
  delete start_id;

//  cout<<"elemlist size "<<elemlist.size()<<endl;
  for(unsigned int i = 0;i < elemlist.size();i++)
    q_list.push(elemlist[i]);


  //////optimization techinique - 2  find all edges(path) end at end_node
//  ftime(&t1);
//  clock_t start_cpu = clock();
/*  double prune_time = numeric_limits<double>::max();

  list<Elem> end_node_edge;
  CcInt* end_node_id = new CcInt(true,id2);
  BTreeIterator* bt_iter_edge = btree2->ExactMatch(end_node_id);
  while(bt_iter_edge->Next()){
    Tuple* edge_tuple = busedge->GetTuple(bt_iter_edge->GetId());

    CcInt* start_node_id = (CcInt*)edge_tuple->GetAttribute(V1);
    CcInt* end_node_id = (CcInt*)edge_tuple->GetAttribute(V2);
    Periods* def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
    CcInt* rpid = (CcInt*)edge_tuple->GetAttribute(RPID);
    const Interval<Instant>* interval;
    def_t->Get(0,interval);
    //optimize-3, filter edge by start time
    if(interval->start < ui1->timeInterval.start){
        edge_tuple->DeleteIfAllowed();
        break;
    }
    if(interval->start >= ui1->timeInterval.start){
      Elem elem(bt_iter_edge->GetId(),*interval,start_node_id->GetIntval());
      elem.e_node_id = end_node_id->GetIntval();
      elem.rpid = rpid->GetIntval();
      end_node_edge.push_front(elem);
    }
    edge_tuple->DeleteIfAllowed();
  }
  delete end_node_id;
  delete bt_iter_edge;
  end_node_edge.sort(TimeCompare1());*/

  ///////////  Print ///////////////////////////
//  cout<<"end_node_edge size "<<end_node_edge.size()<<endl;
//  list<Elem>::iterator start = end_node_edge.begin();
//  for(;start != end_node_edge.end();start++)
//      start->Print2();

//   Optimize2(q_list,tmp_list,end_node_edge,prune_time);

//   ftime(&t2);
//   clock_t stop_cpu = clock();
//   cout<<"time 1 "<<difftimeb(&t2,&t1)<<" ";
//   cout<<"time 1 CPU "<<
//                        ((double)(stop_cpu-start_cpu))/CLOCKS_PER_SEC<<endl;
//  cout<<"initialize size "<<q_list.size()<<endl;
  if(q_list.empty() == true)
    return false;

//////////////////////////////////////////////////////////////////////////

  while(q_list.empty() == false){
//    cout<<"q_list size "<<q_list.size()<<endl;
    Elem top = q_list.top();

//    cout<<"top delta_t "<<top.delta_t<<endl;
//    cout<<"prune_time "<<prune_time<<endl;

//    Tuple* edge_tuple = busedge->GetTuple(top.edge_tuple_id);
//    CcInt* start_node = (CcInt*)edge_tuple->GetAttribute(V1);
//    CcInt* end_node = (CcInt*)edge_tuple->GetAttribute(V2);
    CcInt* start_node = new CcInt(true,top.s_node_id);
    CcInt* end_node = new CcInt(true,top.e_node_id);

    if(end_node->GetIntval() == id2){//find the end
      delete start_node;
      delete end_node;
      break;
    }

    expansionlist.push_back(top);
    expansion_count++;
    q_list.pop();

//    Periods* cur_def_t = (Periods*)edge_tuple->GetAttribute(DEF_T);
//    const Interval<Instant>* interval_cur;
//    cur_def_t->Get(0,interval_cur);
     Interval<Instant>* interval_cur = &top.interval;

//    outfile<<"edge "<<edge_id->GetIntval()<<" pre-edge tid "<<top.pre_edge_tid
//        <<" v1 "<<start_node->GetIntval()
//        <<" v2 "<<end_node->GetIntval()<<" time "<<*interval_cur
//        <<" delta_t "<<top.delta_t<<" rid "<<top.rid<<endl;

    /////////////get all edges from the same start node////////////////
//    clock_t start_cpu = clock();
//    ftime(&t1);
    bt_iter_edge_v1 = btree1->ExactMatch(end_node);
    priority_queue<Elem> temp_list; //Optimize--1
    elemlist.clear();
    while(bt_iter_edge_v1->Next()){
     Tuple* t = busedge->GetTuple(bt_iter_edge_v1->GetId());

     CcInt* start_node_next = end_node;
     CcInt* end_node_next = (CcInt*)t->GetAttribute(V2);//end node
     Periods* next_def_t = (Periods*)t->GetAttribute(DEF_T);
     CcInt* rpid = (CcInt*)t->GetAttribute(RPID);
     CcInt* rid = (CcInt*)t->GetAttribute(PID);
     const Interval<Instant>* interval_next;
     next_def_t->Get(0,interval_next);
     Point* endp = (Point*)t->GetAttribute(P2);
     //optimize-3  filter edge by their start time
    if(interval_next->start < interval_cur->end){
        t->DeleteIfAllowed();
        break;
    }

   //time instant of next edge should be late than cur edge
   //end node id of next edge should not be equal to start node id of cur edge
//     cout<<"next "<<*interval_next<<endl;
//     cout<<*trip<<endl;

      if(interval_next->start > interval_cur->end &&
           end_node_next->GetIntval() != start_node->GetIntval()){
            //store all edges from the same start node
            Elem e(bt_iter_edge_v1->GetId(),*interval_next,
                  start_node_next->GetIntval());
            e.pre_eid = expansion_count - 1;
            e.delta_t = top.delta_t +
                          (interval_next->end-interval_cur->end).ToDouble();

            e.delta_t += endp->Distance(end_point)/maxspeed;
            e.rid = rid->GetIntval();
            e.e_node_id = end_node_next->GetIntval();
            e.pre_edge_tid = top.edge_tuple_id;
            e.rpid = rpid->GetIntval();
//            cout<<e.delta_t<<endl;
//////////////////    Optimize - 1    ////////////////////////////////////////
            if(edge_flag[bt_iter_edge_v1->GetId()]){
//              temp_list.push(e);
                edge_flag[bt_iter_edge_v1->GetId()] = false;

                if(elemlist.empty())
                  elemlist.push_back(e);
                else{
                  unsigned int i = 0;
                  for(;i < elemlist.size();i++){
                    if(elemlist[i].rid == e.rid &&
                        elemlist[i].e_node_id == e.e_node_id &&
                        elemlist[i].interval.start > e.interval.start){//update
                        elemlist[i] = e;
                        break;
                    }
                  }
                  if(i == elemlist.size())
                    elemlist.push_back(e);
                }
            }
//////////////////////////////////////////////////////////////////////////////

        }
        t->DeleteIfAllowed();
      }
      delete bt_iter_edge_v1;

//      ftime(&t2);
//      clock_t stop_cpu = clock();
//      cout<<"time 2 total "<<difftimeb(&t2,&t1)<<" ";
//      cout<<"time 2 CPU "<<
//                        ((double)(stop_cpu-start_cpu))/CLOCKS_PER_SEC<<endl;
//      cout<<"temp_list size 1 "<<temp_list.size()<<endl;
//      Optimize2(q_list,temp_list,end_node_edge,prune_time);

  for(unsigned int i = 0;i < elemlist.size();i++)
    q_list.push(elemlist[i]);

    //////////////////////////////////////////////////////////////////////////
    delete start_node;
    delete end_node;
//    cout<<"find_path_t_5 q_list size "<<q_list.size()<<endl;
  }

  if(q_list.empty() == false){
    stack<Elem> temp_path;
    Elem top = q_list.top();
    temp_path.push(top);
//    cout<<top.edge_tuple_id<<endl;
    while(top.pre_eid != -1){
      int id = top.pre_eid;
//      cout<<"pre id "<<id<<endl;
      top = expansionlist[id];
//      cout<<top.edge_tuple_id<<endl;
      temp_path.push(top);
    }
//    cout<<temp_path.size()<<endl;
    while(temp_path.empty() == false){
      Elem top = temp_path.top();
      temp_path.pop();
      path.push_back(top);
    }
  }

  return true;
}

/*
expand the graph by Dijkstra with minimum total time cost so far
with some optimization techniques, supporting time duration for middle stop
optimize-1 filter the edge from the same route but comes later
optimize-2 use pre-defined path to find end point
optimize-3 filter edge by their start time
optimize-4 use A star algorithm, heuristic-value = distance/maxspeed
input edge relation and b-tree

*/
void BusNetwork::FindPath_T_5(MPoint* mp,Relation* query,Relation* busedge,
BTree* btree1,BTree* btree2,int attrpos1,int attrpos2,Instant& queryinstant)
{
  if(query->GetNoTuples() < 2){
    cout<<"there is only start location, please give destination"<<endl;
    return;
  }

  mp->Clear();
  mp->StartBulkLoad();

//  vector<int> path; //record edge id
  vector<Elem> path;
  //searching process

//  TestFunction(busedge,btree1);
  for(int i = 1;i <= query->GetNoTuples() - 1;i++){
    Tuple* t1 = query->GetTuple(i);
    Tuple* t2 = query->GetTuple(i+1);
    CcInt* id1 = (CcInt*)t1->GetAttribute(attrpos1);
    CcInt* id2 = (CcInt*)t2->GetAttribute(attrpos1);

    DateTime* timestay = (DateTime*)t1->GetAttribute(attrpos2);
//    cout<<id1->GetIntval()<<" "<<id2->GetIntval()<<endl;
//    cout<<*timestay<<endl;
    double waittime = timestay->ToDouble();
//    cout<<waittime<<endl;

    if(FindPath5(id1->GetIntval(),id2->GetIntval(),
              path,busedge,btree1,btree2,queryinstant,waittime)==false){
        cout<<"such a route is not valid"<<endl;
        path.clear();
        break;
    }

  }
  /****************Construct the Reulst (MPoint)*************************/
  const UPoint* lastup = NULL;
  for(unsigned int i = 0;i < path.size();i++){
//    cout<<path[i]<<" ";
    Tuple* edge_tuple = busedge->GetTuple(path[i].edge_tuple_id);
    MPoint* temp_mp = (MPoint*)edge_tuple->GetAttribute(MOVE);
//    cout<<*temp_mp<<endl;
    for(int j = 0;j < temp_mp->GetNoComponents();j++){
      const UPoint* up;
      temp_mp->Get(j,up);
      //not the first trip
      if(lastup != NULL && i != 0 && j == 0){
        UPoint* insert_up = new UPoint(true);
//        cout<<"last "<<*lastup<<endl;
//        cout<<"cur "<<*up<<endl;
        insert_up->p0 = lastup->p1;
        insert_up->timeInterval.start = lastup->timeInterval.end;
        insert_up->p1 = up->p0;
        insert_up->timeInterval.end = up->timeInterval.start;
//        cout<<"insert "<<*insert_up<<endl;
        insert_up->timeInterval.lc = true;
        insert_up->timeInterval.rc = false;
        mp->Add(*insert_up);
        delete insert_up;
        delete lastup;
        lastup = NULL;
      }
      mp->Add(*up);
      if(j == temp_mp->GetNoComponents() - 1 && i != path.size() - 1)
        lastup = new UPoint(*up);
    }
    edge_tuple->DeleteIfAllowed();
  }
  mp->EndBulkLoad();
}
