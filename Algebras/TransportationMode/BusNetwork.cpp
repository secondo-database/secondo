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

#include "BusNetwork.h"

string BusNetwork::busrouteTypeInfo = "(rel(tuple((Id int)(Trip mpoint))))";
string BusNetwork::busstopTypeInfo = "(rel(tuple((Id int)(BusStop point))))";
string BusNetwork::btreebusstopTypeInfo =
                  "(btree(tuple((Id int)(BusStop point))) int)";
string BusNetwork::rtreebusstopTypeInfo =
                  "(rtree(tuple((Id int)(BusStop point))) BusStop FALSE)";

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

  return nl->TwoElemList(nl->IntAtom(bus_id),xBusStop);
}

bool BusNetwork::OpenBusNetwork(SmiRecord& valueRecord,size_t& offset,
const ListExpr typeInfo,Word& value)
{
  cout<<"openbusnetwork"<<endl;
  value.addr = BusNetwork::Open(valueRecord,offset,typeInfo);
  return value.addr != NULL;
}

BusNetwork* BusNetwork::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{
  cout<<"open"<<endl;
  return new BusNetwork(valueRecord,offset,typeInfo);

}

bool BusNetwork::SaveBusNetwork(SmiRecord& valueRecord,size_t& offset,
const ListExpr typeInfo,Word& value)
{
  cout<<"savebusnetwork"<<endl;
  BusNetwork* p = (BusNetwork*)value.addr;
  return p->Save(valueRecord,offset,typeInfo);
}

bool BusNetwork::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
  cout<<"save"<<endl;
  /********************Save id the busnetwork****************************/
  int iId = bus_id;
  in_xValueRecord.Write(&iId,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);
  /****************************save nodes*********************************/
  ListExpr xType;
  nl->ReadFromString(busstopTypeInfo,xType);
  ListExpr xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
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

  //save edges

  return true;

}

void BusNetwork::CloseBusNetwork(const ListExpr typeInfo,Word& w)
{
  //bus stop
  if((BusNetwork*)w.addr != NULL)
      ((BusNetwork*)w.addr)->bus_node->Close();
  //b-tree on bus stop
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->btree_bus_node;
  //r-tree on bus stop
  if((BusNetwork*)w.addr != NULL)
      delete ((BusNetwork*)w.addr)->rtree_bus_node;

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

  ostringstream xBusStopPtrStream;
  xBusStopPtrStream << (long)temp_bus_node;
  string strQuery = "(consume(sort(feed(" + busstopTypeInfo +
                "(ptr " + xBusStopPtrStream.str() + ")))))";
  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  bus_node = (Relation*)xResult.addr;
  temp_bus_node->Delete();

  /************b-tree on bus stop****************************/
  ostringstream xThisBusStopPtrStream;
  xThisBusStopPtrStream<<(long)bus_node;
  strQuery = "(createbtree (" + busstopTypeInfo +
             "(ptr " + xThisBusStopPtrStream.str() + "))" + "Id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  btree_bus_node = (BTree*)xResult.addr;

  /***********r-tree on bus stop*****************************/
  ostringstream xBusStop;
  xBusStop <<(long)bus_node;
  strQuery = "(bulkloadrtree(sortby(addid(feed(" + busstopTypeInfo +
            "(ptr " + xBusStop.str() + "))))((BusStop asc))) BusStop)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  rtree_bus_node = (R_Tree<2,TupleId>*)xResult.addr;

}
/*
Create the relation for storing edges as well as its weight function

*/
void BusNetwork::FillBusWeightAndEdge(const Relation* in_busRoute)
{


}
void BusNetwork::Load(int in_iId,const Relation* in_busRoute)
{
  cout<<"Load() "<<in_iId<<endl;
  bus_id = in_iId;
  bus_def = true;
  assert(in_busRoute != NULL);
  FillBusNode(in_busRoute);
}

/*
2 Type Constructors and Deconstructor

*/
BusNetwork::BusNetwork():
bus_id(0),bus_def(false),bus_node(NULL),btree_bus_node(NULL),
rtree_bus_node(NULL),bus_weight(NULL),bus_edge(NULL)
{

}

void BusNetwork::Destory()
{
 //bus stop
  if(bus_node != NULL){
//  p->bus_node->Close();
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

}

BusNetwork::BusNetwork(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
const ListExpr in_xTypeInfo)
{
  cout<<"BusNetwork() 2"<<endl;
  /***********************Read busnetwork id********************************/
  in_xValueRecord.Read(&bus_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);

  /***************************Open relation for nodes*********************/
  ListExpr xType;
  nl->ReadFromString(busstopTypeInfo,xType);
  ListExpr xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  bus_node = Relation::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!bus_node) return;

  /***********************Open btree for bus stop************************/
  nl->ReadFromString(btreebusstopTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  btree_bus_node = BTree::Open(in_xValueRecord,inout_iOffset,xNumericType);
  if(!btree_bus_node){
    bus_node->Delete();
    return;
  }
///////// Test B-tree //////////////////////
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
//////////////////////////////////////////

/***************************Open R-tree for bus stop************************/
  Word xValue;
  if(!(rtree_bus_node->Open(in_xValueRecord,inout_iOffset,
                          rtreebusstopTypeInfo,xValue))){
    bus_node->Delete();
    delete btree_bus_node;
    return;
  }
  rtree_bus_node = (R_Tree<2,TupleId>*)xValue.addr;
////////////////////Test R-tree ////////////////////////////////
//  cout<<rtree_bus_node->NodeCount()<<endl;
//  rtree_bus_node->BoundingBox().Print(cout);


/////////////////////////////////////////////////////////////////

  /************************* weight**********************************/

  /****************************edges*********************************/

  bus_def = true;
}


BusNetwork::BusNetwork(ListExpr in_xValue,int in_iErrorPos,
ListExpr& inout_xErrorInfo,bool& inout_bCorrect):
bus_id(0),bus_def(false),bus_node(NULL),btree_bus_node(NULL),
rtree_bus_node(NULL),bus_weight(NULL),bus_edge(NULL)
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
  bus_id = nl->IntValue(xIdList);
  cout<<"bus id "<<bus_id<<endl;
  bus_def = true;
}

