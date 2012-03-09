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

July, 2011 Jianqiu Xu

[TOC]

1 Overview

This source file essentially contains the necessary implementations for
computing shortest path in road network 

*/
#include "BusNetwork.h"
#include "RoadNetwork.h"
#include "PaveGraph.h"
#include "ListUtils.h"


/////////////////////////////////////////////////////////////////////////////
//////////////// road network graph ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
string RoadGraph::RGNodeTypeInfo = "(rel (tuple ((Jun_id int) (Jun_gp gpoint)\
(Jun_p point) (Rid int))))";

string RoadGraph::RGBTreeNodeTypeInfo = "(btree (tuple ((jun_id int)\
(jun_gp gpoint)(jun_p point) (rid int))) int)";

string RoadGraph::RGEdgeTypeInfo1 = "(rel (tuple ((Jun_id1 int)\
(Jun_id2 int))))";

string RoadGraph::RGEdgeTypeInfo2 = "(rel (tuple ((Jun_id1 int) (Jun_id2 int)\
(Path1 gline) (Path2 sline))))";


ListExpr RoadGraph::RoadGraphProp()
{
    ListExpr examplelist = nl->TextAtom();
    nl->AppendText(examplelist,
               "createroadgraph(<id>,<node-relation>,<edge-relation>)");
    return nl->TwoElemList(
             nl->TwoElemList(nl->StringAtom("Creation"),
                              nl->StringAtom("Example Creation")),
             nl->TwoElemList(examplelist,
                   nl->StringAtom("let rg=createroadgraph(id,n-rel,e-rel)")));
}


bool RoadGraph::CheckRoadGraph(ListExpr type, ListExpr& errorInfo)
{
//  cout<<"CheckRoadGraph()"<<endl;
  return nl->IsEqual(type, "roadgraph");
}

int RoadGraph::SizeOfRoadGraph()
{
//  cout<<"SizeOfRoadGraph()"<<endl;
  return 0;
}

void* RoadGraph::CastRoadGraph(void* addr)
{
//  cout<<"CastRoadGraph()"<<endl;
  return 0;
}

Word RoadGraph::CloneRoadGraph(const ListExpr typeInfo, const Word& w)
{
//  cout<<"CloneMetroGraph()"<<endl;
  return SetWord(Address(0));
}

void RoadGraph::CloseRoadGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"CloseRoadGraph()"<<endl;
  delete static_cast<RoadGraph*> (w.addr);
  w.addr = NULL;
}

Word RoadGraph::CreateRoadGraph(const ListExpr typeInfo)
{
//  cout<<"CreateRoadGraph()"<<endl;
  return SetWord(new RoadGraph());
}

void RoadGraph::DeleteRoadGraph(const ListExpr typeInfo, Word& w)
{
//  cout<<"DeleteRoadGraph()"<<endl;
  RoadGraph* rg = (RoadGraph*)w.addr;
  delete rg;
  w.addr = NULL;
}

bool RoadGraph::SaveRoadGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"SaveRoadGraph()"<<endl;
  RoadGraph* rg = (RoadGraph*)value.addr;
  bool result = rg->Save(valueRecord, offset, typeInfo);

  return result;
}

bool RoadGraph::OpenRoadGraph(SmiRecord& valueRecord, size_t& offset,
                           const ListExpr typeInfo, Word& value)
{
//  cout<<"OpenRoadGraph()"<<endl;
  value.addr = RoadGraph::Open(valueRecord, offset, typeInfo);
  bool result = (value.addr != NULL);

  return result;
}

/*
input road network graph 

*/
Word RoadGraph::InRoadGraph(ListExpr in_xTypeInfo,
                            ListExpr in_xValue,
                            int in_iErrorPos, ListExpr& inout_xErrorInfo,
                            bool& inout_bCorrect)
{
//  cout<<"InRoadGraph()"<<endl;
  RoadGraph* rg = new RoadGraph(in_xValue, in_iErrorPos, inout_xErrorInfo,
                                inout_bCorrect);
  if(inout_bCorrect) return SetWord(rg);
  else{
    delete rg;
    return SetWord(Address(0));
  }
}


ListExpr RoadGraph::OutRoadGraph(ListExpr typeInfo, Word value)
{
//  cout<<"OutRoadGraph()"<<endl;
  RoadGraph* rg = (RoadGraph*)value.addr;
  return rg->Out(typeInfo);
}


ListExpr RoadGraph::Out(ListExpr typeInfo)
{
//  cout<<"Out()"<<endl;
  ListExpr xNode = nl->TheEmptyList();
  ListExpr xLast = nl->TheEmptyList();
  ListExpr xNext = nl->TheEmptyList();

  bool bFirst = true;
  for(int i = 1;i <= node_rel->GetNoTuples();i++){
      Tuple* node_tuple = node_rel->GetTuple(i, false);

      int id = ((CcInt*)node_tuple->GetAttribute(RG_JUN_ID))->GetIntval();
      GPoint* gp = (GPoint*)node_tuple->GetAttribute(RG_JUN_GP);
      Point* loc = (Point*)node_tuple->GetAttribute(RG_JUN_P);

      xNext = nl->ThreeElemList(
           nl->IntAtom(id), 
           GPoint::OutGPoint( nl->TheEmptyList(), SetWord(gp)),
           OutPoint( nl->TheEmptyList(), SetWord(loc)));

      if(bFirst){
        xNode = nl->OneElemList(xNext);
        xLast = xNode;
        bFirst = false;
      }else
          xLast = nl->Append(xLast,xNext);
      node_tuple->DeleteIfAllowed();
  }
  
  return nl->TwoElemList(nl->IntAtom(rg_id),xNode);

//  return nl->OneElemList(nl->IntAtom(rg_id));

}


RoadGraph* RoadGraph::Open(SmiRecord& valueRecord,size_t& offset,
                          const ListExpr typeInfo)
{
  return new RoadGraph(valueRecord,offset,typeInfo);
}



RoadGraph::RoadGraph():rg_id(0), node_rel(NULL), btree_node(NULL),
edge_rel1(NULL), adj_list1(0), entry_adj_list1(0),
 edge_rel2(NULL), adj_list2(0), entry_adj_list2(0)
{
//  cout<<"RoadGraph::RoadGraph()"<<endl;
}

RoadGraph::RoadGraph(ListExpr in_xValue,int in_iErrorPos,
                     ListExpr& inout_xErrorInfo,
                     bool& inout_bCorrect):rg_id(0), node_rel(NULL),
                     btree_node(NULL), 
                     edge_rel1(NULL), adj_list1(0), entry_adj_list1(0),
edge_rel2(NULL), adj_list2(0), entry_adj_list2(0)
{
//  cout<<"RoadGraph::RoadGraph(ListExpr)"<<endl;
}


RoadGraph::RoadGraph(SmiRecord& in_xValueRecord, size_t& inout_iOffset,
const ListExpr in_xTypeInfo):rg_id(0), node_rel(NULL), btree_node(NULL),
edge_rel1(NULL), adj_list1(0), entry_adj_list1(0),
edge_rel2(NULL), adj_list2(0), entry_adj_list2(0)
{
  in_xValueRecord.Read(&rg_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);


  ListExpr xType;
  ListExpr xNumericType;
  ///////////////////Open relation for node////////////////////////////
  nl->ReadFromString(RGNodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  node_rel = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!node_rel) {
    return;
  }
  
  ///////////////////open btree built on nodes//////////////////////////
   nl->ReadFromString(RGBTreeNodeTypeInfo, xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
   btree_node = BTree::Open(in_xValueRecord, inout_iOffset, xNumericType);
   if(!btree_node) {
     node_rel->Delete();
     return;
   }

  ///////////////////////Open relation for edge1///////////////////////////
  nl->ReadFromString(RGEdgeTypeInfo1, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel1 = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel1) {
    node_rel->Delete();
    delete btree_node;
    return;
  }

  /////////////////open adjacency list1////////////////////////////////
   size_t bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   SmiSize offset = 0;
   char* buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list1.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list1.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf);

  ///////////////////////Open relation for edge2//////////////////////////
  nl->ReadFromString(RGEdgeTypeInfo2, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  edge_rel2 = Relation::Open(in_xValueRecord, inout_iOffset, xNumericType);
  if(!edge_rel2) {
    node_rel->Delete();
    delete btree_node;
    edge_rel1->Delete(); 
    adj_list1.clean();
    entry_adj_list1.clean(); 
    return;
  }

  /////////////////open adjacency list2////////////////////////////////
   bufsize = sizeof(FlobId) + sizeof(SmiSize) + 2*sizeof(int);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   assert(buf != NULL);
   adj_list2.restoreHeader(buf,offset);
   free(buf);
   offset = 0;
   buf = (char*) malloc(bufsize);
   in_xValueRecord.Read(buf, bufsize, inout_iOffset);
   assert(buf != NULL);
   entry_adj_list2.restoreHeader(buf,offset);
   inout_iOffset += bufsize;
   free(buf); 

}

RoadGraph::~RoadGraph()
{
    if(node_rel != NULL) node_rel->Close();
    if(edge_rel1 != NULL) edge_rel1->Close(); 
    if(edge_rel2 != NULL) edge_rel2->Close(); 
    if(btree_node != NULL) delete btree_node;

}


/*
save functions for the graph 

*/
bool RoadGraph::Save(SmiRecord& in_xValueRecord,size_t& inout_iOffset,
              const ListExpr in_xTypeInfo)
{

  //  cout<<"save "<<endl; 
  in_xValueRecord.Write(&rg_id,sizeof(int),inout_iOffset);
  inout_iOffset += sizeof(int);


  ListExpr xType;
  ListExpr xNumericType;
  ///////////////////////////save node/////////////////////////////
  nl->ReadFromString(RGNodeTypeInfo,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!node_rel->Save(in_xValueRecord, inout_iOffset, xNumericType))
      return false;

  ////////////////save btree on nodes///////////////////////////
   nl->ReadFromString(RGBTreeNodeTypeInfo, xType);
   xNumericType = SecondoSystem::GetCatalog()->NumericType(xType); 
   if(!btree_node->Save(in_xValueRecord, inout_iOffset, xNumericType))
     return false; 
   

  /////////////////////save edge1/////////////////////////////
  nl->ReadFromString(RGEdgeTypeInfo1, xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel1->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /////////////////adjacency list 1//////////////////////////////
   SecondoCatalog *ctlg = SecondoSystem::GetCatalog();
   SmiRecordFile *rf = ctlg->GetFlobFile();

   adj_list1.saveToFile(rf, adj_list1);
   SmiSize offset = 0;
   size_t bufsize = adj_list1.headerSize()+ 2*sizeof(int);
   char* buf = (char*) malloc(bufsize);
   adj_list1.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list1.saveToFile(rf, entry_adj_list1);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list1.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize;


  ///////////////////////save edge2/////////////////////////////////////
  nl->ReadFromString(RGEdgeTypeInfo2,xType);
  xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  if(!edge_rel2->Save(in_xValueRecord,inout_iOffset,xNumericType))
      return false;

  /////////////////adjacency list 2//////////////////////////////////

   adj_list2.saveToFile(rf, adj_list2);
   offset = 0;
   bufsize = adj_list2.headerSize()+ 2*sizeof(int);
   buf = (char*) malloc(bufsize);
   adj_list2.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf, bufsize, inout_iOffset);
   inout_iOffset += bufsize;
   free(buf);

   entry_adj_list2.saveToFile(rf, entry_adj_list2);
   offset = 0;
   buf = (char*) malloc(bufsize);
   entry_adj_list2.serializeHeader(buf,offset);
   assert(offset==bufsize);
   in_xValueRecord.Write(buf,bufsize, inout_iOffset);
   free(buf);
   inout_iOffset += bufsize; 

   return true; 


}

/*
load road graph from input relations

*/
void RoadGraph::Load(int id, Relation* r1, Relation* edge_rel1, 
                     Relation* edge_rel2)
{
  if(id < 0){
    cout<<"invalid id "<<id<<endl;
    return;
  }
  rg_id = id;


  //////////////////node relation////////////////////

  ListExpr ptrList1 = listutils::getPtrList(r1);
  string strQuery = "(consume(feed(" + RGNodeTypeInfo +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  node_rel = (Relation*)xResult.addr;

  ///////////////////rtree on junction points//////////////////////
  ListExpr ptrList2 = listutils::getPtrList(r1);
  strQuery = "(createbtree (" + RGNodeTypeInfo +
             "(ptr " + nl->ToString(ptrList2) + "))" + "Rid)";//capital 

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  btree_node = (BTree*)xResult.addr;


//  cout<<node_rel->GetNoTuples()<<endl;

  LoadEdge1(edge_rel1);
  LoadEdge2(edge_rel2);


}

/*
load road graph edges 

*/

void RoadGraph::LoadEdge1(Relation* edge1)
{

  ListExpr ptrList1 = listutils::getPtrList(edge1);

  string strQuery = "(consume(feed(" + RGEdgeTypeInfo1 +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel1 = (Relation*)xResult.addr;

  //////////////////create adjacency list////////////////////////////////////
  ListExpr ptrList2 = listutils::getPtrList(edge1);
  
  strQuery = "(createbtree (" + RGEdgeTypeInfo1 +
             "(ptr " + nl->ToString(ptrList2) + "))" + "Jun_id1)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree = (BTree*)xResult.addr;

  /////////////////////////////////////////////////////////////////////////
  /////////the adjacent list here is different from dual graph and 
  ///////// visibility graph. it is the same as indoor graph ////////////
  //////////in dual graph and visibility graph, we store the node id/////
  /////////now we store the edge id because the weight, path is stored
  ////////in the edge relation ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* ms_tuple = node_rel->GetTuple(i, false);
    int jun_id = ((CcInt*)ms_tuple->GetAttribute(RG_JUN_ID))->GetIntval();
    
    CcInt* nodeid = new CcInt(true, jun_id);
    BTreeIterator* btree_iter = btree->ExactMatch(nodeid);
    int start = adj_list1.Size();
    while(btree_iter->Next()){
      Tuple* edge_tuple = edge_rel1->GetTuple(btree_iter->GetId(), false);

      adj_list1.Append(edge_tuple->GetTupleId());//get the edge tuple id 

      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter;

    int end = adj_list1.Size();
    entry_adj_list1.Append(ListEntry(start, end));

//   cout<<jun_id<<" start "<<start<<" end "<<end<<endl;

    delete nodeid;

    ms_tuple->DeleteIfAllowed();

  }

  delete btree; 

}


void RoadGraph::LoadEdge2(Relation* edge2)
{

  ListExpr ptrList1 = listutils::getPtrList(edge2);
  
  string strQuery = "(consume(feed(" + RGEdgeTypeInfo2 +
                "(ptr " + nl->ToString(ptrList1) + "))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  edge_rel2 = (Relation*)xResult.addr;

  //////////////////create adjacency list////////////////////////////////////
  ListExpr ptrList2 = listutils::getPtrList(edge2);

  strQuery = "(createbtree (" + RGEdgeTypeInfo2 +
             "(ptr " + nl->ToString(ptrList2) + "))" + "Jun_id1)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery,xResult);
  assert(QueryExecuted);
  BTree* btree = (BTree*)xResult.addr;

  /////////////////////////////////////////////////////////////////////////
  /////////the adjacent list here is different from dual graph and 
  ///////// visibility graph. it is the same as indoor graph ////////////
  //////////in dual graph and visibility graph, we store the node id/////
  /////////now we store the edge id because the weight, path is stored
  ////////in the edge relation ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  for(int i = 1;i <= node_rel->GetNoTuples();i++){
    Tuple* ms_tuple = node_rel->GetTuple(i, false);
    int jun_id = ((CcInt*)ms_tuple->GetAttribute(RG_JUN_ID))->GetIntval();

    CcInt* nodeid = new CcInt(true, jun_id);
    BTreeIterator* btree_iter = btree->ExactMatch(nodeid);
    int start = adj_list2.Size();
    while(btree_iter->Next()){
      Tuple* edge_tuple = edge_rel2->GetTuple(btree_iter->GetId(), false);

      adj_list2.Append(edge_tuple->GetTupleId());//get the edge tuple id 

      edge_tuple->DeleteIfAllowed();
    }
    delete btree_iter;

    int end = adj_list2.Size();
    entry_adj_list2.Append(ListEntry(start, end));

//   cout<<jun_id<<" start "<<start<<" end "<<end<<endl;

    delete nodeid;

    ms_tuple->DeleteIfAllowed();

  }

  delete btree; 


}


/*
get all junctions nodes of a route 

*/
void RoadGraph::GetJunctionsNode(int rid, vector<GP_Point>& res_list)
{
    CcInt* search_id = new CcInt(true, rid);
    BTreeIterator* btree_iter = btree_node->ExactMatch(search_id);
    while(btree_iter->Next()){
        Tuple* tuple = node_rel->GetTuple(btree_iter->GetId(), false);
        int node_id = ((CcInt*)tuple->GetAttribute(RG_JUN_ID))->GetIntval();
        GPoint* gp = (GPoint*)tuple->GetAttribute(RG_JUN_GP);
        Point* loc = (Point*)tuple->GetAttribute(RG_JUN_P);
        GP_Point gp_p(gp->GetRouteId(), gp->GetPosition(), 0.0, *loc, *loc);
        gp_p.oid = node_id;
        res_list.push_back(gp_p);
        tuple->DeleteIfAllowed();
    }
    delete btree_iter;
    delete search_id;
}


/*
metro stops have the same spatial location in space

*/
void RoadGraph::FindAdj1(int node_id, vector<int>& list)
{
  ListEntry list_entry;
  entry_adj_list1.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list1.Get(j, oid);
      j++;
      list.push_back(oid);
  }
  
}

/*
metro stops are connected by moving metros 

*/
void RoadGraph::FindAdj2(int node_id, vector<int>& list)
{

  ListEntry list_entry;
  entry_adj_list2.Get(node_id - 1, list_entry);
  int low = list_entry.low;
  int high = list_entry.high;
  int j = low;
  while(j < high){
      int oid;
      adj_list2.Get(j, oid);
      j++;
      list.push_back(oid);
  }

}

/////////////////////////////////////////////////////////////////////////
/////////////// query processing on the road graph//////////////////////
///////////////////////////////////////////////////////////////////////

void RoadNav::GenerateRoadLoc(Network* rn, int no, 
                                 vector<GPoint>& gp_list, 
                                 vector<Point>& gp_loc_list)
{
  Relation* routes_rel = rn->GetRoutes();
  for (int i = 1; i <= no;i++){
     int m = GetRandom() % routes_rel->GetNoTuples() + 1;
     Tuple* road_tuple = routes_rel->GetTuple(m, false);
     int rid = ((CcInt*)road_tuple->GetAttribute(ROUTE_ID))->GetIntval();
     SimpleLine* sl = (SimpleLine*)road_tuple->GetAttribute(ROUTE_CURVE);

     double len = sl->Length();
     int pos = GetRandom() % (int)len;
     GPoint gp(true, rn->GetId(), rid, pos, None);

     Point* gp_loc = new Point();

     assert(sl->GetStartSmaller());
     assert(sl->AtPosition(pos, true, *gp_loc));//look at the value (dual) for r

     gp_list.push_back(gp);
     gp_loc_list.push_back(*gp_loc);
     delete gp_loc;

     road_tuple->DeleteIfAllowed();
   }
}

/*
use road graph to calculate shortest path
1000 pair of shortest path: 
old method: 99.64 seconds
road graph: 22.40 seconds 

old method has some bugs:
NetworkId: 1 RouteId: 1011  Position: 613 Side: 2
NetworkId: 1 RouteId: 2655  Position: 622 Side: 2

road graph:11848.8  old:11975.2

*/
void RoadNav::ShortestPath(GPoint* gp1, GPoint* gp2, 
                           RoadGraph* rg, Network* rn, GLine* res)
{
  ///////////////////////////////////////////////////////////////////////////
  /////// testing the performance of different shortest path computation/////
  //////////////////////////////////////////////////////////////////////////

//   int no = 1000;
/*   int no = 100;
   vector<GPoint> gp_loc_list1;
   vector<Point> p_loc_list1;
   GenerateRoadLoc(rn, no, gp_loc_list1, p_loc_list1);

   vector<GPoint> gp_loc_list2;
   vector<Point> p_loc_list2;
   GenerateRoadLoc(rn, no, gp_loc_list2, p_loc_list2);

   const double delta_d = 0.001;
   for(unsigned int i = 0;i < gp_loc_list1.size();i++){
      GPoint* loc1 = &gp_loc_list1[i];
      GPoint* loc2 = &gp_loc_list2[i];
      GLine* gl1 = new GLine(0);
      GLine* gl2 = new GLine(0);

      ShortestPathSub(loc1, loc2, rg, rn, gl1);
//      loc1->ShortestPath(loc2, gl2, rn);

//      if(fabs(gl1->GetLength() - gl2->GetLength()) > delta_d){
//            cout<<"error:different shortest path "<<endl;
//            cout<<*loc1<<" "<<*loc2<<" "
//            <<gl1->GetLength()<<" "<<gl2->GetLength()<<endl;
//      }
      cout<<i<<" "<<gl1->GetLength()<<endl;

      delete gl1;
      delete gl2;
   } */
   //////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////

   ShortestPathSub(gp1, gp2, rg, rn, res);

}

/*
calculate shortest path 

*/

void RoadNav::ShortestPathSub(GPoint* gp1, GPoint* gp2, RoadGraph* rg, 
                           Network* rn, GLine* res_path)
{
   if(gp1->IsDefined() == false || gp2->IsDefined() == false){
      return;
   }



   //////////////////////////////////////////////////////////////////////
   /////////find the junction node for gp1 and gp2//////////////////////
   /////////////////////////////////////////////////////////////////////
   const double delta_d = 0.001;
   ////////// collect all junction points of gp1.rid, gp2.rid//////////
   //////// find the node id of these junction points in road graph////
   ///////////////////////////////////////////////////////////////////
   
   vector<GP_Point> gp_p_list1;
   rg->GetJunctionsNode(gp1->GetRouteId(), gp_p_list1);
   assert(gp_p_list1.size() > 0);
//   LOOP_PRINT2(gp_p_list1);


   vector<GP_Point> gp_p_list2;
   rg->GetJunctionsNode(gp2->GetRouteId(), gp_p_list2);
   assert(gp_p_list1.size() > 0);
//   LOOP_PRINT2(gp_p_list2);


   Tuple* road_tuple1 = rn->GetRoute(gp1->GetRouteId());
   SimpleLine* sl1 = (SimpleLine*)road_tuple1->GetAttribute(ROUTE_CURVE);
   Point loc_start;
   assert(sl1->GetStartSmaller());
   assert(sl1->AtPosition(gp1->GetPosition(), true, loc_start));
   road_tuple1->DeleteIfAllowed();

   Tuple* road_tuple2 = rn->GetRoute(gp2->GetRouteId());
   SimpleLine* sl2 = (SimpleLine*)road_tuple2->GetAttribute(ROUTE_CURVE);
   Point loc_end;
   assert(sl2->GetStartSmaller());
   assert(sl2->AtPosition(gp2->GetPosition(), true, loc_end));
   road_tuple2->DeleteIfAllowed();

   if(loc_start.Distance(loc_end) < delta_d){
      res_path->SetDefined(false);
      return;
   }

   ////////////check whether the start location equals to junction node///////
   bool exist_jun1 = false;
   int start_index = -1;
   for(unsigned int i = 0;i < gp_p_list1.size();i++){
      if(gp1->GetRouteId() == gp_p_list1[i].rid &&
         fabs(gp1->GetPosition() - gp_p_list1[i].pos1) < delta_d && 
         fabs(loc_start.Distance(gp_p_list1[i].loc1) < delta_d)){

          start_index = i;
          exist_jun1 = true;
          break;
      }
   }

   ////////////check whether the end location is equal to junction node///////
   bool exist_jun2 = false;
   int end_index = -1;
   for(unsigned int i = 0;i < gp_p_list2.size();i++){
      if(gp2->GetRouteId() == gp_p_list2[i].rid &&
         fabs(gp2->GetPosition() - gp_p_list2[i].pos1) < delta_d && 
         fabs(loc_end.Distance(gp_p_list2[i].loc1) < delta_d)){
          end_index = i;
          exist_jun2 = true;
          break;
      }
   }



   priority_queue<RNPath_elem> path_queue;
   vector<RNPath_elem> expand_queue;

   //////////////////////////////////////////////////////////////////////////
   //////////////////initialize the start node /////////////////////////////
   /////////////////////////////////////////////////////////////////////////
   vector<GP_Point>  start_jun_list;
   
   if(exist_jun1 == false){///start location is not equal to junction point 
     if(gp_p_list1.size() == 1){

          RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[0].pos1);
          double w = fabs(gp1->GetPosition() - gp_p_list1[0].pos1);
          double hw = gp_p_list1[0].loc1.Distance(loc_end);

          int cur_size = expand_queue.size();
          RNPath_elem rn_elem(-1, cur_size, gp_p_list1[0].oid, w + hw, w,
                              ri, true, gp_p_list1[0].loc1);

          path_queue.push(rn_elem);
          expand_queue.push_back(rn_elem);
//          ri.Print(cout);
          start_jun_list.push_back(gp_p_list1[0]);

     }else{
        for(unsigned int i = 0;i < gp_p_list1.size();i++){
//          if(i == 0 && gp1->GetPosition() < gp_p_list1[i].pos1){//first jun
          if(i == 0){
            if(gp1->GetPosition() < gp_p_list1[i].pos1){
              RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i].pos1);
              double w = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
              double hw = gp_p_list1[i].loc1.Distance(loc_end);

              int cur_size = expand_queue.size();
              RNPath_elem rn_elem(-1, cur_size, gp_p_list1[i].oid, w + hw, w, 
                              ri, true, gp_p_list1[i].loc1);

              path_queue.push(rn_elem);
              expand_queue.push_back(rn_elem);

              start_jun_list.push_back(gp_p_list1[i]);
              break;
            }
//           }else if(i == gp_p_list1.size() - 1 && 
//                    gp1->GetPosition() > gp_p_list1[i].pos1){
          }else if(i == gp_p_list1.size() - 1){

            if(gp1->GetPosition() > gp_p_list1[i].pos1){
                RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i].pos1);
                double w = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
                double hw = gp_p_list1[i].loc1.Distance(loc_end);

                int cur_size = expand_queue.size();
                RNPath_elem rn_elem(-1, cur_size, gp_p_list1[i].oid, w + hw, w,
                              ri, true, gp_p_list1[i].loc1);

                path_queue.push(rn_elem);
                expand_queue.push_back(rn_elem);

                start_jun_list.push_back(gp_p_list1[i]);

                break;
            }
          }else if(gp1->GetPosition() > gp_p_list1[i - 1].pos1&& 
                   gp1->GetPosition() < gp_p_list1[i].pos1){ //two junctions

            ///////////////first element//////////////////////
            RouteInterval ri1(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i - 1].pos1);
            double w1 = fabs(gp1->GetPosition() - gp_p_list1[i - 1].pos1);
            double hw1 = gp_p_list1[i - 1].loc1.Distance(loc_end);
            int cur_size = expand_queue.size();
            RNPath_elem rn_elem1(-1, cur_size, gp_p_list1[i - 1].oid, w1 + hw1, 
                                 w1, ri1, true, gp_p_list1[i - 1].loc1);
            path_queue.push(rn_elem1);
            expand_queue.push_back(rn_elem1);

            ///////////////second element//////////////////////////////
            RouteInterval ri2(gp1->GetRouteId(), gp1->GetPosition(),
                           gp_p_list1[i].pos1);
            double w2 = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
            double hw2 = gp_p_list1[i].loc1.Distance(loc_end);

            cur_size = expand_queue.size();
            RNPath_elem rn_elem2(-1, cur_size, gp_p_list1[i].oid, w2 + hw2, w2,
                              ri2, true, gp_p_list1[i].loc1);

            path_queue.push(rn_elem2);
            expand_queue.push_back(rn_elem2);

            start_jun_list.push_back(gp_p_list1[i - 1]);
            start_jun_list.push_back(gp_p_list1[i]);

            break;
          }

        }
     }
   }else{////////start location is equal to jun point 

     assert(0 <= start_index && start_index < (int)gp_p_list1.size());

     RouteInterval ri(0, 0, 0);
     double w = 0;
     double hw = loc_start.Distance(loc_end);

     int cur_size = expand_queue.size();
     RNPath_elem rn_elem(-1, cur_size, gp_p_list1[start_index].oid, w + hw, w,
                              ri, false, loc_start);

     path_queue.push(rn_elem);
     expand_queue.push_back(rn_elem);
     
     /////////////////////////////////////////////////////////////////
     /////put the neighbor node (same location) into queue////////////
     /////////////////////////////////////////////////////////////////
     vector<int> adj_list;
     rg->FindAdj1(gp_p_list1[start_index].oid, adj_list);

     for(unsigned int i = 0;i < adj_list.size();i++){
      Tuple* edge_tuple = rg->GetEdge_Rel1()->GetTuple(adj_list[i], false);

      int neighbor_id = 
          ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN2))->GetIntval();

      edge_tuple->DeleteIfAllowed();

      Tuple* node_tuple = rg->GetNode_Rel()->GetTuple(neighbor_id, false);
      int oid = 
         ((CcInt*)node_tuple->GetAttribute(RoadGraph::RG_JUN_ID))->GetIntval();
      assert(neighbor_id == oid);
      Point* jun_p = 
          (Point*)node_tuple->GetAttribute(RoadGraph::RG_JUN_P);
      assert(jun_p->Distance(loc_start) < delta_d);
      double w = 0.0;
      double hw = jun_p->Distance(loc_end);
      RouteInterval ri(0, 0, 0);


      int cur_size = expand_queue.size();

      RNPath_elem elem(0, cur_size, neighbor_id, w + hw, w,
                      ri, false, *jun_p);
      path_queue.push(elem);
      expand_queue.push_back(elem); 

      node_tuple->DeleteIfAllowed();

    }////////end for 

   }
   
   vector<GP_Point>  end_jun_list;
   if(exist_jun2 == false){//find the junction point to end location
      if(gp_p_list2.size() == 1){
          end_jun_list.push_back(gp_p_list2[0]);
      }else{
         for(unsigned int i = 0;i < gp_p_list2.size();i++){
            if(i == 0 && gp2->GetPosition() < gp_p_list2[i].pos1){//first jun
              end_jun_list.push_back(gp_p_list2[i]);
              break;
          }else if(i == gp_p_list2.size() - 1 && 
                   gp2->GetPosition() > gp_p_list2[i].pos1){
            end_jun_list.push_back(gp_p_list2[i]);
            break;
          }else if(gp2->GetPosition() > gp_p_list2[i - 1].pos1 && 
                   gp2->GetPosition() < gp_p_list2[i].pos1){

            end_jun_list.push_back(gp_p_list2[i - 1]);
            end_jun_list.push_back(gp_p_list2[i]);
            break;
          }
         }
      }

      ////////start and end locations are on the same road sections
      if(start_jun_list.size() == end_jun_list.size()){
          unsigned int i;
          for(i = 0;i < start_jun_list.size();i++){
            if(start_jun_list[i].oid != end_jun_list[i].oid)break;
          }
          if(i == start_jun_list.size()){
              res_path->SetNetworkId(gp1->GetNetworkId());
              res_path->AddRouteInterval(gp1->GetRouteId(), 
                                        gp1->GetPosition(), gp2->GetPosition());
              res_path->SetDefined(true);
              res_path->SetSorted(false);
              res_path->TrimToSize();
              return;
          }
      }
   }


   //////////////////////////////start searching//////////////////////////

   vector<bool> visit_flag; //junction node visit 
   for(int i = 1;i <= rg->GetNode_Rel()->GetNoTuples();i++){
      visit_flag.push_back(false);
   }

   bool found = false;
   RNPath_elem dest;
   while(path_queue.empty() == false){
      RNPath_elem top = path_queue.top();
      path_queue.pop();

//      top.Print();

     if(top.tri_index == 0 || top.to_loc.Distance(loc_end) < delta_d){
        dest = top;
        found = true;
        break;
     }

     if(visit_flag[top.tri_index - 1]) continue;

     //////////////////////////////////////////////////////////////////////
     if(exist_jun2 == false){//end location is not equal to junction point 
          for(unsigned int i = 0;i < end_jun_list.size();i++){
              if(end_jun_list[i].oid == top.tri_index){
                  int pos_expand_path = top.cur_index;;
                  int cur_size = expand_queue.size();

                  assert(end_jun_list[i].rid == gp2->GetRouteId());
                  double w = top.real_w + 
                             fabs(end_jun_list[i].pos1 - gp2->GetPosition());
                  double hw = 0.0; 
                  RouteInterval ri(gp2->GetRouteId(), end_jun_list[i].pos1,
                                   gp2->GetPosition());

                  RNPath_elem elem(pos_expand_path, cur_size, 0, w + hw, w,
                                   ri, true, loc_end);//end point not oid 
                  path_queue.push(elem);
                  expand_queue.push_back(elem); 
              }
          }
     }


    int pos_expand_path;
    int cur_size;
    if(top.len){

     ///////////////////////neighbor list 1////////////////////////////////
      vector<int> adj_list1;
      rg->FindAdj1(top.tri_index, adj_list1);

      for(unsigned int i = 0;i < adj_list1.size();i++){
        Tuple* edge_tuple = rg->GetEdge_Rel1()->GetTuple(adj_list1[i], false);

        int neighbor_id = 
          ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN2))->GetIntval();

        if(visit_flag[neighbor_id - 1]){
          edge_tuple->DeleteIfAllowed();
          continue; 
        }
        edge_tuple->DeleteIfAllowed();



        double w = top.real_w;
        double hw = top.to_loc.Distance(loc_end);
        RouteInterval ri(0, 0, 0);

        pos_expand_path = top.cur_index;
        cur_size = expand_queue.size();

        RNPath_elem elem(pos_expand_path, cur_size, neighbor_id, w + hw, w,
                      ri, false, top.to_loc);

        path_queue.push(elem);
        expand_queue.push_back(elem); 

      }
    }

    ///////////////////////neighbor list 2////////////////////////////////
     vector<int> adj_list2;
     rg->FindAdj2(top.tri_index, adj_list2);
//     cout<<"adj2 size "<<adj_list2.size()<<endl;

     for(unsigned int i = 0;i < adj_list2.size();i++){
      Tuple* edge_tuple = rg->GetEdge_Rel2()->GetTuple(adj_list2[i], false);

      int neighbor_id = 
        ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN_2))->GetIntval();

      if(visit_flag[neighbor_id - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }
      GLine* gl = (GLine*)edge_tuple->GetAttribute(RoadGraph::RG_PATHA1);
      assert(gl->NoOfComponents() == 1);
      RouteInterval ri;
      gl->Get(0, ri);
      edge_tuple->DeleteIfAllowed();


      Tuple* node_tuple = rg->GetNode_Rel()->GetTuple(neighbor_id, false);
      int oid = 
         ((CcInt*)node_tuple->GetAttribute(RoadGraph::RG_JUN_ID))->GetIntval();
      assert(neighbor_id == oid);
      Point* jun_p = 
          (Point*)node_tuple->GetAttribute(RoadGraph::RG_JUN_P);

      double w = top.real_w + fabs(ri.GetStartPos() - ri.GetEndPos());
      double hw = jun_p->Distance(loc_end);


      pos_expand_path = top.cur_index;
      cur_size = expand_queue.size();

      RNPath_elem elem(pos_expand_path, cur_size, neighbor_id, w + hw, w,
                      ri, true, *jun_p);
      path_queue.push(elem);
      expand_queue.push_back(elem); 


      node_tuple->DeleteIfAllowed();
    }

    visit_flag[top.tri_index - 1] = true;

   }

  if(found){

    res_path->SetNetworkId(gp1->GetNetworkId());
    vector<RouteInterval> ri_list;

    while(dest.prev_index != -1){
      if(dest.len){
//        res_path->AddRouteInterval(dest.ri);
        ri_list.push_back(dest.ri);
      }
      dest = expand_queue[dest.prev_index];

    }
    if(dest.len){
//      res_path->AddRouteInterval(dest.ri);
        ri_list.push_back(dest.ri);
    }

//     for(int i = ri_list.size() - 1;i >= 0; i--){
//       res_path->AddRouteInterval(ri_list[i]);
//     }


     ////////////////merge intervals on the same route///////////////////
    vector<RouteInterval> new_ri_list;
    for(int i = ri_list.size() - 1; i >= 0;i--){
      RouteInterval ri = ri_list[i];
      int j = i - 1;
      while(j >= 0 && ri_list[j].GetRouteId() == ri.GetRouteId()){

        if(fabs(ri.GetEndPos() - ri_list[j].GetStartPos()) < delta_d){
          ri.SetEndPos(ri_list[j].GetEndPos());
          j--;
        }else
          assert(false);
      }
      new_ri_list.push_back(ri);
      i = j + 1;
    }

    for(unsigned int i = 0;i < new_ri_list.size(); i++){
        res_path->AddRouteInterval(new_ri_list[i]);
    } 


    res_path->SetDefined(true);
    res_path->SetSorted(false);
    res_path->TrimToSize();
  }else{
    res_path->SetDefined(false);
  }


}

/*
find all routes that intersect the route 

*/

void RoadNav::DFTraverse(Network* rn, R_Tree<2,TupleId>* rtree, 
                         SmiRecordId adr, 
                          SimpleLine* line, vector<GPoint_Dist>& res_list)
{
  Relation* routes = rn->GetRoutes();

  R_TreeNode<2,TupleId>* node = rtree->GetMyNode(adr,false,
                  rtree->MinEntries(0), rtree->MaxEntries(0));
  for(int j = 0;j < node->EntryCount();j++){
      if(node->IsLeaf()){
              R_TreeLeafEntry<2,TupleId> e =
                 (R_TreeLeafEntry<2,TupleId>&)(*node)[j];
              Tuple* route_tuple = routes->GetTuple(e.info, false);
              SimpleLine* road =
                     (SimpleLine*)route_tuple->GetAttribute(ROUTE_CURVE);
              int rid = 
                  ((CcInt*)route_tuple->GetAttribute(ROUTE_ID))->GetIntval();

              Points* ps = new Points(0);
              line->Crossings(*road, *ps);
              if(ps->Size() > 0){
//                  cout<<"ps size "<<ps->Size()<<endl;
                  for(int i = 0;i < ps->Size();i++){
                    Point p;
                    ps->Get(i, p);
                    double pos;
                    if(road->AtPoint(p, true, pos)){
                      GPoint gp(true, rn->GetId(), rid, pos, None);
                      GPoint_Dist gpd(gp, p, 0.0);
                      res_list.push_back(gpd);
                    }
                  }
              }
              delete ps;
              route_tuple->DeleteIfAllowed();
      }else{
            R_TreeInternalEntry<2> e =
                (R_TreeInternalEntry<2>&)(*node)[j];
            if(e.box.Intersects(line->BoundingBox())){
                DFTraverse(rn, rtree, e.pointer, line, res_list);
            }
      }
  }
  delete node;

}

/*
find a path from a gpoint to another. avoid the road network center area

*/
void RoadNav::ShortestPath2(GPoint* gp1, GPoint* gp2, RoadGraph* rg, 
                           Network* rn, GLine* res_path)
{
//   ShortestPathSub2(gp1, gp2, rg, rn, res_path);
   ShortestPathSub3(gp1, gp2, rg, rn, res_path);
}

void RoadNav::ShortestPathSub2(GPoint* gp1, GPoint* gp2, RoadGraph* rg, 
                           Network* rn, GLine* res_path)
{
   if(gp1->IsDefined() == false || gp2->IsDefined() == false){
      return;
   }

   Rectangle<2> bbox = rn->GetRTree()->BoundingBox();
   double mid_x = (bbox.MinD(0) + bbox.MaxD(0))/2;
   double mid_y = (bbox.MinD(1) + bbox.MaxD(1))/2;
//   cout<<"mindx "<<mid_x<<" mindy "<<mid_y<<endl;
   const double dist1 = 5000.0;
   const double dist2 = 4000.0;
   double min[2], max[2];
   min[0] = mid_x - dist1;
   min[1] = mid_y - dist2;
   max[0] = mid_x + dist1;
   max[1] = mid_y + dist2;

  Rectangle<2> center_area(true, min, max);//shortest path avoid center area

   //////////////////////////////////////////////////////////////////////
   /////////find the junction node for gp1 and gp2//////////////////////
   /////////////////////////////////////////////////////////////////////
   const double delta_d = 0.001;
   ////////// collect all junction points of gp1.rid, gp2.rid//////////
   //////// find the node id of these junction points in road graph////
   ///////////////////////////////////////////////////////////////////
   
   vector<GP_Point> gp_p_list1;
   rg->GetJunctionsNode(gp1->GetRouteId(), gp_p_list1);
   assert(gp_p_list1.size() > 0);
//   LOOP_PRINT2(gp_p_list1);


   vector<GP_Point> gp_p_list2;
   rg->GetJunctionsNode(gp2->GetRouteId(), gp_p_list2);
   assert(gp_p_list1.size() > 0);
//   LOOP_PRINT2(gp_p_list2);


   Tuple* road_tuple1 = rn->GetRoute(gp1->GetRouteId());
   SimpleLine* sl1 = (SimpleLine*)road_tuple1->GetAttribute(ROUTE_CURVE);
   Point loc_start;
   assert(sl1->GetStartSmaller());
   assert(sl1->AtPosition(gp1->GetPosition(), true, loc_start));
   road_tuple1->DeleteIfAllowed();

   Tuple* road_tuple2 = rn->GetRoute(gp2->GetRouteId());
   SimpleLine* sl2 = (SimpleLine*)road_tuple2->GetAttribute(ROUTE_CURVE);
   Point loc_end;
   assert(sl2->GetStartSmaller());
   assert(sl2->AtPosition(gp2->GetPosition(), true, loc_end));
   road_tuple2->DeleteIfAllowed();

   if(loc_start.Distance(loc_end) < delta_d){
      res_path->SetDefined(false);
      return;
   }

   ////////////check whether the start location equals to junction node///////
   bool exist_jun1 = false;
   int start_index = -1;
   for(unsigned int i = 0;i < gp_p_list1.size();i++){
      if(gp1->GetRouteId() == gp_p_list1[i].rid &&
         fabs(gp1->GetPosition() - gp_p_list1[i].pos1) < delta_d && 
         fabs(loc_start.Distance(gp_p_list1[i].loc1) < delta_d)){

          start_index = i;
          exist_jun1 = true;
          break;
      }
   }
   
   ////////////check whether the end location is equal to junction node///////
   bool exist_jun2 = false;
   int end_index = -1;
   for(unsigned int i = 0;i < gp_p_list2.size();i++){
      if(gp2->GetRouteId() == gp_p_list2[i].rid &&
         fabs(gp2->GetPosition() - gp_p_list2[i].pos1) < delta_d && 
         fabs(loc_end.Distance(gp_p_list2[i].loc1) < delta_d)){
          end_index = i;
          exist_jun2 = true;
          break;
      }
   }



   priority_queue<RNPath_elem> path_queue;
   vector<RNPath_elem> expand_queue;

   //////////////////////////////////////////////////////////////////////////
   //////////////////initialize the start node /////////////////////////////
   /////////////////////////////////////////////////////////////////////////
   vector<GP_Point>  start_jun_list;
   
   if(exist_jun1 == false){///start location is not equal to junction point 
     if(gp_p_list1.size() == 1){

          RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[0].pos1);
          double w = fabs(gp1->GetPosition() - gp_p_list1[0].pos1);
          double hw = gp_p_list1[0].loc1.Distance(loc_end);

          int cur_size = expand_queue.size();
          RNPath_elem rn_elem(-1, cur_size, gp_p_list1[0].oid, w + hw, w,
                              ri, true, gp_p_list1[0].loc1);

          path_queue.push(rn_elem);
          expand_queue.push_back(rn_elem);
//          ri.Print(cout);
          start_jun_list.push_back(gp_p_list1[0]);

     }else{
        for(unsigned int i = 0;i < gp_p_list1.size();i++){
          if(i == 0 && gp1->GetPosition() < gp_p_list1[i].pos1){//first jun
            RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i].pos1);
            double w = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
            double hw = gp_p_list1[i].loc1.Distance(loc_end);

            int cur_size = expand_queue.size();
            RNPath_elem rn_elem(-1, cur_size, gp_p_list1[i].oid, w + hw, w, 
                              ri, true, gp_p_list1[i].loc1);

            path_queue.push(rn_elem);
            expand_queue.push_back(rn_elem);

            start_jun_list.push_back(gp_p_list1[i]);
            break;
          }else if(i == gp_p_list1.size() - 1 && 
                   gp1->GetPosition() > gp_p_list1[i].pos1){

            RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i].pos1);
            double w = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
            double hw = gp_p_list1[i].loc1.Distance(loc_end);

            int cur_size = expand_queue.size();
            RNPath_elem rn_elem(-1, cur_size, gp_p_list1[i].oid, w + hw, w, 
                              ri, true, gp_p_list1[i].loc1);

            path_queue.push(rn_elem);
            expand_queue.push_back(rn_elem);
            
            start_jun_list.push_back(gp_p_list1[i]);

            break;
          }else if(gp1->GetPosition() > gp_p_list1[i - 1].pos1&& 
                   gp1->GetPosition() < gp_p_list1[i].pos1){///two juns

            ///////////////first element//////////////////////
            RouteInterval ri1(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i - 1].pos1);
            double w1 = fabs(gp1->GetPosition() - gp_p_list1[i - 1].pos1);
            double hw1 = gp_p_list1[i - 1].loc1.Distance(loc_end);
            int cur_size = expand_queue.size();
            RNPath_elem rn_elem1(-1, cur_size, gp_p_list1[i - 1].oid, w1 + hw1, 
                                 w1, ri1, true, gp_p_list1[i - 1].loc1);
            path_queue.push(rn_elem1);
            expand_queue.push_back(rn_elem1);

            ///////////////second element//////////////////////////////
            RouteInterval ri2(gp1->GetRouteId(), gp1->GetPosition(),
                           gp_p_list1[i].pos1);
            double w2 = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
            double hw2 = gp_p_list1[i].loc1.Distance(loc_end);

            cur_size = expand_queue.size();
            RNPath_elem rn_elem2(-1, cur_size, gp_p_list1[i].oid, w2 + hw2, w2,
                              ri2, true, gp_p_list1[i].loc1);

            path_queue.push(rn_elem2);
            expand_queue.push_back(rn_elem2);

            start_jun_list.push_back(gp_p_list1[i - 1]);
            start_jun_list.push_back(gp_p_list1[i]);

            break;
          }

        }
     }
   }else{////////start location is equal to jun point 

     assert(0 <= start_index && start_index < (int)gp_p_list1.size());

     RouteInterval ri(0, 0, 0);
     double w = 0;
     double hw = loc_start.Distance(loc_end);

     int cur_size = expand_queue.size();
     RNPath_elem rn_elem(-1, cur_size, gp_p_list1[start_index].oid, w + hw, w,
                              ri, false, loc_start);

     path_queue.push(rn_elem);
     expand_queue.push_back(rn_elem);
     
     /////////////////////////////////////////////////////////////////
     /////put the neighbor node (same location) into queue////////////
     /////////////////////////////////////////////////////////////////
     vector<int> adj_list;
     rg->FindAdj1(gp_p_list1[start_index].oid, adj_list);

     for(unsigned int i = 0;i < adj_list.size();i++){
      Tuple* edge_tuple = rg->GetEdge_Rel1()->GetTuple(adj_list[i], false);

      int neighbor_id = 
          ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN2))->GetIntval();

      edge_tuple->DeleteIfAllowed();

      Tuple* node_tuple = rg->GetNode_Rel()->GetTuple(neighbor_id, false);
      int oid = 
         ((CcInt*)node_tuple->GetAttribute(RoadGraph::RG_JUN_ID))->GetIntval();
      assert(neighbor_id == oid);
      Point* jun_p = 
          (Point*)node_tuple->GetAttribute(RoadGraph::RG_JUN_P);
      assert(jun_p->Distance(loc_start) < delta_d);
      double w = 0.0;
      double hw = jun_p->Distance(loc_end);
      RouteInterval ri(0, 0, 0);


      int cur_size = expand_queue.size();

      RNPath_elem elem(0, cur_size, neighbor_id, w + hw, w,
                      ri, false, *jun_p);
      path_queue.push(elem);
      expand_queue.push_back(elem); 

      node_tuple->DeleteIfAllowed();

    }////////end for 

   }
   
   vector<GP_Point>  end_jun_list;
   if(exist_jun2 == false){//find the junction point to end location
      if(gp_p_list2.size() == 1){
          end_jun_list.push_back(gp_p_list2[0]);
      }else{
         for(unsigned int i = 0;i < gp_p_list2.size();i++){
            if(i == 0 && gp2->GetPosition() < gp_p_list2[i].pos1){//first jun
              end_jun_list.push_back(gp_p_list2[i]);
              break;
          }else if(i == gp_p_list2.size() - 1 && 
                   gp2->GetPosition() > gp_p_list2[i].pos1){
            end_jun_list.push_back(gp_p_list2[i]);
            break;
          }else if(gp2->GetPosition() > gp_p_list2[i - 1].pos1 && 
                   gp2->GetPosition() < gp_p_list2[i].pos1){

            end_jun_list.push_back(gp_p_list2[i - 1]);
            end_jun_list.push_back(gp_p_list2[i]);
            break;
          }
         }
      }

      ////////start and end locations are on the same road sections
      if(start_jun_list.size() == end_jun_list.size()){
          unsigned int i;
          for(i = 0;i < start_jun_list.size();i++){
            if(start_jun_list[i].oid != end_jun_list[i].oid)break;
          }
          if(i == start_jun_list.size()){
              res_path->SetNetworkId(gp1->GetNetworkId());
              res_path->AddRouteInterval(gp1->GetRouteId(), 
                                        gp1->GetPosition(), gp2->GetPosition());
              res_path->SetDefined(true);
              res_path->SetSorted(false);
              res_path->TrimToSize();
              return;
          }
      }
   }


   //////////////////////////////start searching//////////////////////////

   vector<bool> visit_flag; //junction node visit 
   for(int i = 1;i <= rg->GetNode_Rel()->GetNoTuples();i++){
      visit_flag.push_back(false);
   }

   bool found = false;
   RNPath_elem dest;
   
   
   
   while(path_queue.empty() == false){
      RNPath_elem top = path_queue.top();
      path_queue.pop();

//      top.Print();

     if(top.tri_index == 0 || top.to_loc.Distance(loc_end) < delta_d){
        dest = top;
        found = true;
        break;
     }

     if(visit_flag[top.tri_index - 1]) continue;

     //////////////////////////////////////////////////////////////////////
     if(exist_jun2 == false){//end location is not equal to junction point 
          for(unsigned int i = 0;i < end_jun_list.size();i++){
              if(end_jun_list[i].oid == top.tri_index){
                  int pos_expand_path = top.cur_index;;
                  int cur_size = expand_queue.size();

                  assert(end_jun_list[i].rid == gp2->GetRouteId());
                  double w = top.real_w + 
                             fabs(end_jun_list[i].pos1 - gp2->GetPosition());
                  double hw = 0.0; 
                  RouteInterval ri(gp2->GetRouteId(), end_jun_list[i].pos1,
                                   gp2->GetPosition());

                  RNPath_elem elem(pos_expand_path, cur_size, 0, w + hw, w,
                                   ri, true, loc_end);//end point not oid 
                  path_queue.push(elem);
                  expand_queue.push_back(elem); 
              }
          }
     }


    int pos_expand_path;
    int cur_size;
    if(top.len){

     ///////////////////////neighbor list 1////////////////////////////////
      vector<int> adj_list1;
      rg->FindAdj1(top.tri_index, adj_list1);

      for(unsigned int i = 0;i < adj_list1.size();i++){
        Tuple* edge_tuple = rg->GetEdge_Rel1()->GetTuple(adj_list1[i], false);

        int neighbor_id = 
          ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN2))->GetIntval();

        if(visit_flag[neighbor_id - 1]){
          edge_tuple->DeleteIfAllowed();
          continue; 
        }
        edge_tuple->DeleteIfAllowed();



        double w = top.real_w;
        double hw = top.to_loc.Distance(loc_end);
        RouteInterval ri(0, 0, 0);

        pos_expand_path = top.cur_index;
        cur_size = expand_queue.size();

        RNPath_elem elem(pos_expand_path, cur_size, neighbor_id, w + hw, w,
                      ri, false, top.to_loc);

        path_queue.push(elem);
        expand_queue.push_back(elem); 

      }
    }

    ///////////////////////neighbor list 2////////////////////////////////
     vector<int> adj_list2;
     rg->FindAdj2(top.tri_index, adj_list2);
//     cout<<"adj2 size "<<adj_list2.size()<<endl;

     for(unsigned int i = 0;i < adj_list2.size();i++){
      Tuple* edge_tuple = rg->GetEdge_Rel2()->GetTuple(adj_list2[i], false);

      int neighbor_id = 
        ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN_2))->GetIntval();

      if(visit_flag[neighbor_id - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }
      GLine* gl = (GLine*)edge_tuple->GetAttribute(RoadGraph::RG_PATHA1);
      assert(gl->NoOfComponents() == 1);
      RouteInterval ri;
      gl->Get(0, ri);
      edge_tuple->DeleteIfAllowed();

//      cout<<ri.GetRouteId()<<" "<<ri.GetStartPos()<<" "<<ri.GetEndPos()<<endl;
      ///////////////////////////////////////////////////////////////////
      Tuple* road_tuple = rn->GetRoute(ri.GetRouteId());
      SimpleLine* sl = (SimpleLine*)road_tuple->GetAttribute(ROUTE_CURVE);
      assert(sl->StartsSmaller());
      SimpleLine* sub_sl = new SimpleLine(0);
      if(ri.GetStartPos() < ri.GetEndPos())
        sl->SubLine(ri.GetStartPos(), ri.GetEndPos(), true, *sub_sl);
      else
        sl->SubLine(ri.GetEndPos(), ri.GetStartPos(), true, *sub_sl);

//      cout<<sub_sl->Length()<<endl;
      if(center_area.Contains(sub_sl->BoundingBox())){
        delete sub_sl;
        road_tuple->DeleteIfAllowed();
        continue;
      }

      delete sub_sl;
      road_tuple->DeleteIfAllowed();
      //////////////////////////////////////////////////////////////////


      Tuple* node_tuple = rg->GetNode_Rel()->GetTuple(neighbor_id, false);
      int oid = 
         ((CcInt*)node_tuple->GetAttribute(RoadGraph::RG_JUN_ID))->GetIntval();
      assert(neighbor_id == oid);
      Point* jun_p = 
          (Point*)node_tuple->GetAttribute(RoadGraph::RG_JUN_P);

      double w = top.real_w + fabs(ri.GetStartPos() - ri.GetEndPos());
      double hw = jun_p->Distance(loc_end);


      pos_expand_path = top.cur_index;
      cur_size = expand_queue.size();

      RNPath_elem elem(pos_expand_path, cur_size, neighbor_id, w + hw, w,
                      ri, true, *jun_p);
      path_queue.push(elem);
      expand_queue.push_back(elem); 


      node_tuple->DeleteIfAllowed();
    }

    visit_flag[top.tri_index - 1] = true;

   }

  if(found){

    res_path->SetNetworkId(gp1->GetNetworkId());
    vector<RouteInterval> ri_list;

    while(dest.prev_index != -1){
      if(dest.len){
//        res_path->AddRouteInterval(dest.ri);
        ri_list.push_back(dest.ri);
      }
      dest = expand_queue[dest.prev_index];

    }
    if(dest.len){
//      res_path->AddRouteInterval(dest.ri);
        ri_list.push_back(dest.ri);
    }

//     for(int i = ri_list.size() - 1;i >= 0; i--){
//       res_path->AddRouteInterval(ri_list[i]);
//     }


     ////////////////merge intervals on the same route///////////////////
    vector<RouteInterval> new_ri_list;
    for(int i = ri_list.size() - 1; i >= 0;i--){
      RouteInterval ri = ri_list[i];
      int j = i - 1;
      while(j >= 0 && ri_list[j].GetRouteId() == ri.GetRouteId()){

        if(fabs(ri.GetEndPos() - ri_list[j].GetStartPos()) < delta_d){
          ri.SetEndPos(ri_list[j].GetEndPos());
          j--;
        }else
          assert(false);
      }
      new_ri_list.push_back(ri);
      i = j + 1;
    }

    for(unsigned int i = 0;i < new_ri_list.size(); i++){
        res_path->AddRouteInterval(new_ri_list[i]);
    } 


    res_path->SetDefined(true);
    res_path->SetSorted(false);
    res_path->TrimToSize();
  }else{
    res_path->SetDefined(false);
  }


}

/*
not use the distance as weight in the priority queue

*/
void RoadNav::ShortestPathSub3(GPoint* gp1, GPoint* gp2, RoadGraph* rg, 
                           Network* rn, GLine* res_path)
{
   if(gp1->IsDefined() == false || gp2->IsDefined() == false){
      return;
   }

   //////////////////////////////////////////////////////////////////////
   /////////find the junction node for gp1 and gp2//////////////////////
   /////////////////////////////////////////////////////////////////////
   const double delta_d = 0.001;
   ////////// collect all junction points of gp1.rid, gp2.rid//////////
   //////// find the node id of these junction points in road graph////
   ///////////////////////////////////////////////////////////////////
   
   vector<GP_Point> gp_p_list1;
   rg->GetJunctionsNode(gp1->GetRouteId(), gp_p_list1);
   assert(gp_p_list1.size() > 0);
//   LOOP_PRINT2(gp_p_list1);


   vector<GP_Point> gp_p_list2;
   rg->GetJunctionsNode(gp2->GetRouteId(), gp_p_list2);
   assert(gp_p_list1.size() > 0);
//   LOOP_PRINT2(gp_p_list2);


   Tuple* road_tuple1 = rn->GetRoute(gp1->GetRouteId());
   SimpleLine* sl1 = (SimpleLine*)road_tuple1->GetAttribute(ROUTE_CURVE);
   Point loc_start;
   assert(sl1->GetStartSmaller());
   assert(sl1->AtPosition(gp1->GetPosition(), true, loc_start));
   road_tuple1->DeleteIfAllowed();

   Tuple* road_tuple2 = rn->GetRoute(gp2->GetRouteId());
   SimpleLine* sl2 = (SimpleLine*)road_tuple2->GetAttribute(ROUTE_CURVE);
   Point loc_end;
   assert(sl2->GetStartSmaller());
   assert(sl2->AtPosition(gp2->GetPosition(), true, loc_end));
   road_tuple2->DeleteIfAllowed();

   if(loc_start.Distance(loc_end) < delta_d){
      res_path->SetDefined(false);
      return;
   }

   ////////////check whether the start location equals to junction node///////
   bool exist_jun1 = false;
   int start_index = -1;
   for(unsigned int i = 0;i < gp_p_list1.size();i++){
      if(gp1->GetRouteId() == gp_p_list1[i].rid &&
         fabs(gp1->GetPosition() - gp_p_list1[i].pos1) < delta_d && 
         fabs(loc_start.Distance(gp_p_list1[i].loc1) < delta_d)){

          start_index = i;
          exist_jun1 = true;
          break;
      }
   }
   
   ////////////check whether the end location is equal to junction node///////
   bool exist_jun2 = false;
   int end_index = -1;
   for(unsigned int i = 0;i < gp_p_list2.size();i++){
      if(gp2->GetRouteId() == gp_p_list2[i].rid &&
         fabs(gp2->GetPosition() - gp_p_list2[i].pos1) < delta_d && 
         fabs(loc_end.Distance(gp_p_list2[i].loc1) < delta_d)){
          end_index = i;
          exist_jun2 = true;
          break;
      }
   }



   priority_queue<RNPath_elem> path_queue;
   vector<RNPath_elem> expand_queue;

   //////////////////////////////////////////////////////////////////////////
   //////////////////initialize the start node /////////////////////////////
   /////////////////////////////////////////////////////////////////////////
   vector<GP_Point>  start_jun_list;
   
   if(exist_jun1 == false){///start location is not equal to junction point 
     if(gp_p_list1.size() == 1){

          RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[0].pos1);
          double w = fabs(gp1->GetPosition() - gp_p_list1[0].pos1);
          double hw = gp_p_list1[0].loc1.Distance(loc_end);

          int cur_size = expand_queue.size();
          RNPath_elem rn_elem(-1, cur_size, gp_p_list1[0].oid, w + hw, w,
                              ri, true, gp_p_list1[0].loc1);

          path_queue.push(rn_elem);
          expand_queue.push_back(rn_elem);
//          ri.Print(cout);
          start_jun_list.push_back(gp_p_list1[0]);

     }else{
        for(unsigned int i = 0;i < gp_p_list1.size();i++){
          if(i == 0 && gp1->GetPosition() < gp_p_list1[i].pos1){//first jun
            RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i].pos1);
            double w = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
            double hw = gp_p_list1[i].loc1.Distance(loc_end);

            int cur_size = expand_queue.size();
            RNPath_elem rn_elem(-1, cur_size, gp_p_list1[i].oid, w + hw, w, 
                              ri, true, gp_p_list1[i].loc1);

            path_queue.push(rn_elem);
            expand_queue.push_back(rn_elem);

            start_jun_list.push_back(gp_p_list1[i]);
            break;
          }else if(i == gp_p_list1.size() - 1 && 
                   gp1->GetPosition() > gp_p_list1[i].pos1){

            RouteInterval ri(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i].pos1);
            double w = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
            double hw = gp_p_list1[i].loc1.Distance(loc_end);

            int cur_size = expand_queue.size();
            RNPath_elem rn_elem(-1, cur_size, gp_p_list1[i].oid, w + hw, w, 
                              ri, true, gp_p_list1[i].loc1);

            path_queue.push(rn_elem);
            expand_queue.push_back(rn_elem);
            
            start_jun_list.push_back(gp_p_list1[i]);

            break;
          }else if(gp1->GetPosition() > gp_p_list1[i - 1].pos1&& 
                   gp1->GetPosition() < gp_p_list1[i].pos1){///two juns

            ///////////////first element//////////////////////
            RouteInterval ri1(gp1->GetRouteId(), gp1->GetPosition(), 
                           gp_p_list1[i - 1].pos1);
            double w1 = fabs(gp1->GetPosition() - gp_p_list1[i - 1].pos1);
            double hw1 = gp_p_list1[i - 1].loc1.Distance(loc_end);
            int cur_size = expand_queue.size();
            RNPath_elem rn_elem1(-1, cur_size, gp_p_list1[i - 1].oid, w1 + hw1, 
                                 w1, ri1, true, gp_p_list1[i - 1].loc1);
            path_queue.push(rn_elem1);
            expand_queue.push_back(rn_elem1);

            ///////////////second element//////////////////////////////
            RouteInterval ri2(gp1->GetRouteId(), gp1->GetPosition(),
                           gp_p_list1[i].pos1);
            double w2 = fabs(gp1->GetPosition() - gp_p_list1[i].pos1);
            double hw2 = gp_p_list1[i].loc1.Distance(loc_end);

            cur_size = expand_queue.size();
            RNPath_elem rn_elem2(-1, cur_size, gp_p_list1[i].oid, w2 + hw2, w2,
                              ri2, true, gp_p_list1[i].loc1);

            path_queue.push(rn_elem2);
            expand_queue.push_back(rn_elem2);

            start_jun_list.push_back(gp_p_list1[i - 1]);
            start_jun_list.push_back(gp_p_list1[i]);

            break;
          }

        }
     }
   }else{////////start location is equal to jun point 

     assert(0 <= start_index && start_index < (int)gp_p_list1.size());

     RouteInterval ri(0, 0, 0);
     double w = 0;
     double hw = loc_start.Distance(loc_end);

     int cur_size = expand_queue.size();
     RNPath_elem rn_elem(-1, cur_size, gp_p_list1[start_index].oid, w + hw, w,
                              ri, false, loc_start);

     path_queue.push(rn_elem);
     expand_queue.push_back(rn_elem);
     
     /////////////////////////////////////////////////////////////////
     /////put the neighbor node (same location) into queue////////////
     /////////////////////////////////////////////////////////////////
     vector<int> adj_list;
     rg->FindAdj1(gp_p_list1[start_index].oid, adj_list);

     for(unsigned int i = 0;i < adj_list.size();i++){
      Tuple* edge_tuple = rg->GetEdge_Rel1()->GetTuple(adj_list[i], false);

      int neighbor_id = 
          ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN2))->GetIntval();

      edge_tuple->DeleteIfAllowed();

      Tuple* node_tuple = rg->GetNode_Rel()->GetTuple(neighbor_id, false);
      int oid = 
         ((CcInt*)node_tuple->GetAttribute(RoadGraph::RG_JUN_ID))->GetIntval();
      assert(neighbor_id == oid);
      Point* jun_p = 
          (Point*)node_tuple->GetAttribute(RoadGraph::RG_JUN_P);
      assert(jun_p->Distance(loc_start) < delta_d);
      double w = 0.0;
      double hw = jun_p->Distance(loc_end);
      RouteInterval ri(0, 0, 0);


      int cur_size = expand_queue.size();

      RNPath_elem elem(0, cur_size, neighbor_id, w + hw, w,
                      ri, false, *jun_p);
      path_queue.push(elem);
      expand_queue.push_back(elem); 

      node_tuple->DeleteIfAllowed();

    }////////end for 

   }
   
   vector<GP_Point>  end_jun_list;
   if(exist_jun2 == false){//find the junction point to end location
      if(gp_p_list2.size() == 1){
          end_jun_list.push_back(gp_p_list2[0]);
      }else{
         for(unsigned int i = 0;i < gp_p_list2.size();i++){
            if(i == 0 && gp2->GetPosition() < gp_p_list2[i].pos1){//first jun
              end_jun_list.push_back(gp_p_list2[i]);
              break;
          }else if(i == gp_p_list2.size() - 1 && 
                   gp2->GetPosition() > gp_p_list2[i].pos1){
            end_jun_list.push_back(gp_p_list2[i]);
            break;
          }else if(gp2->GetPosition() > gp_p_list2[i - 1].pos1 && 
                   gp2->GetPosition() < gp_p_list2[i].pos1){

            end_jun_list.push_back(gp_p_list2[i - 1]);
            end_jun_list.push_back(gp_p_list2[i]);
            break;
          }
         }
      }

      ////////start and end locations are on the same road sections
      if(start_jun_list.size() == end_jun_list.size()){
          unsigned int i;
          for(i = 0;i < start_jun_list.size();i++){
            if(start_jun_list[i].oid != end_jun_list[i].oid)break;
          }
          if(i == start_jun_list.size()){
              res_path->SetNetworkId(gp1->GetNetworkId());
              res_path->AddRouteInterval(gp1->GetRouteId(), 
                                        gp1->GetPosition(), gp2->GetPosition());
              res_path->SetDefined(true);
              res_path->SetSorted(false);
              res_path->TrimToSize();
              return;
          }
      }
   }


   //////////////////////////////start searching//////////////////////////

   vector<bool> visit_flag; //junction node visit 
   for(int i = 1;i <= rg->GetNode_Rel()->GetNoTuples();i++){
      visit_flag.push_back(false);
   }

   bool found = false;
   RNPath_elem dest;
   
   
   
   while(path_queue.empty() == false){
      RNPath_elem top = path_queue.top();
      path_queue.pop();

//      top.Print();

     if(top.tri_index == 0 || top.to_loc.Distance(loc_end) < delta_d){
        dest = top;
        found = true;
        break;
     }

     if(visit_flag[top.tri_index - 1]) continue;

     //////////////////////////////////////////////////////////////////////
     if(exist_jun2 == false){//end location is not equal to junction point 
          for(unsigned int i = 0;i < end_jun_list.size();i++){
              if(end_jun_list[i].oid == top.tri_index){
                  int pos_expand_path = top.cur_index;;
                  int cur_size = expand_queue.size();

                  assert(end_jun_list[i].rid == gp2->GetRouteId());
/*                  double w = top.real_w + 
                             fabs(end_jun_list[i].pos1 - gp2->GetPosition());*/

                  double l = fabs(end_jun_list[i].pos1 - gp2->GetPosition());
                  double w = top.real_w + l;

                  double hw = 0.0; 
                  RouteInterval ri(gp2->GetRouteId(), end_jun_list[i].pos1,
                                   gp2->GetPosition());

                  RNPath_elem elem(pos_expand_path, cur_size, 0, w + hw, w,
                                   ri, true, loc_end);//end point not oid 

                  path_queue.push(elem);
                  expand_queue.push_back(elem); 
              }
          }
     }


    int pos_expand_path;
    int cur_size;
    if(top.len){

     ///////////////////////neighbor list 1////////////////////////////////
      vector<int> adj_list1;
      rg->FindAdj1(top.tri_index, adj_list1);

      for(unsigned int i = 0;i < adj_list1.size();i++){
        Tuple* edge_tuple = rg->GetEdge_Rel1()->GetTuple(adj_list1[i], false);

        int neighbor_id = 
          ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN2))->GetIntval();

        if(visit_flag[neighbor_id - 1]){
          edge_tuple->DeleteIfAllowed();
          continue; 
        }
        edge_tuple->DeleteIfAllowed();



        double w = top.real_w;
        double hw = top.to_loc.Distance(loc_end);
        RouteInterval ri(0, 0, 0);

        pos_expand_path = top.cur_index;
        cur_size = expand_queue.size();

        RNPath_elem elem(pos_expand_path, cur_size, neighbor_id, w + hw, w,
                      ri, false, top.to_loc);

        path_queue.push(elem);
        expand_queue.push_back(elem); 

      }
    }

    ///////////////////////neighbor list 2////////////////////////////////
     vector<int> adj_list2;
     rg->FindAdj2(top.tri_index, adj_list2);
//     cout<<"adj2 size "<<adj_list2.size()<<endl;

     for(unsigned int i = 0;i < adj_list2.size();i++){
      Tuple* edge_tuple = rg->GetEdge_Rel2()->GetTuple(adj_list2[i], false);

      int neighbor_id = 
        ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN_2))->GetIntval();

      if(visit_flag[neighbor_id - 1]){
        edge_tuple->DeleteIfAllowed();
        continue; 
      }
      GLine* gl = (GLine*)edge_tuple->GetAttribute(RoadGraph::RG_PATHA1);
      assert(gl->NoOfComponents() == 1);
      RouteInterval ri;
      gl->Get(0, ri);
      edge_tuple->DeleteIfAllowed();

//      cout<<ri.GetRouteId()<<" "<<ri.GetStartPos()<<" "<<ri.GetEndPos()<<endl;
      ///////////////////////////////////////////////////////////////////
      Tuple* road_tuple = rn->GetRoute(ri.GetRouteId());
      SimpleLine* sl = (SimpleLine*)road_tuple->GetAttribute(ROUTE_CURVE);
      assert(sl->StartsSmaller());
      SimpleLine* sub_sl = new SimpleLine(0);
      if(ri.GetStartPos() < ri.GetEndPos())
        sl->SubLine(ri.GetStartPos(), ri.GetEndPos(), true, *sub_sl);
      else
        sl->SubLine(ri.GetEndPos(), ri.GetStartPos(), true, *sub_sl);

//      cout<<sub_sl->Length()<<endl;

      delete sub_sl;
      road_tuple->DeleteIfAllowed();
      //////////////////////////////////////////////////////////////////


      Tuple* node_tuple = rg->GetNode_Rel()->GetTuple(neighbor_id, false);
      int oid = 
         ((CcInt*)node_tuple->GetAttribute(RoadGraph::RG_JUN_ID))->GetIntval();
      assert(neighbor_id == oid);
      Point* jun_p = 
          (Point*)node_tuple->GetAttribute(RoadGraph::RG_JUN_P);

//      double w = top.real_w + fabs(ri.GetStartPos() - ri.GetEndPos());
      double l = fabs(ri.GetStartPos() - ri.GetEndPos());

      double w;
      if(l > 50.0) ////////10 ?
        w = top.real_w + GetRandom() % (int)l;
      else 
        w = top.real_w + l;

      double hw = jun_p->Distance(loc_end);


      pos_expand_path = top.cur_index;
      cur_size = expand_queue.size();

      RNPath_elem elem(pos_expand_path, cur_size, neighbor_id, w + hw, w,
                      ri, true, *jun_p);
      path_queue.push(elem);
      expand_queue.push_back(elem); 


      node_tuple->DeleteIfAllowed();
    }

    visit_flag[top.tri_index - 1] = true;

   }

  if(found){

    res_path->SetNetworkId(gp1->GetNetworkId());
    vector<RouteInterval> ri_list;

    while(dest.prev_index != -1){
      if(dest.len){
//        res_path->AddRouteInterval(dest.ri);
        ri_list.push_back(dest.ri);
      }
      dest = expand_queue[dest.prev_index];

    }
    if(dest.len){
//      res_path->AddRouteInterval(dest.ri);
        ri_list.push_back(dest.ri);
    }

//     for(int i = ri_list.size() - 1;i >= 0; i--){
//       res_path->AddRouteInterval(ri_list[i]);
//     }


     ////////////////merge intervals on the same route///////////////////
    vector<RouteInterval> new_ri_list;
    for(int i = ri_list.size() - 1; i >= 0;i--){
      RouteInterval ri = ri_list[i];
      int j = i - 1;
      while(j >= 0 && ri_list[j].GetRouteId() == ri.GetRouteId()){

        if(fabs(ri.GetEndPos() - ri_list[j].GetStartPos()) < delta_d){
          ri.SetEndPos(ri_list[j].GetEndPos());
          j--;
        }else
          assert(false);
      }
      new_ri_list.push_back(ri);
      i = j + 1;
    }

    for(unsigned int i = 0;i < new_ri_list.size(); i++){
        res_path->AddRouteInterval(new_ri_list[i]);
    } 


    res_path->SetDefined(true);
    res_path->SetSorted(false);
    res_path->TrimToSize();
  }else{
    res_path->SetDefined(false);
  }


}


/*
get all neighbor junction nodes for a given junction 

*/
void RoadNav::GetAdjNodeRG(RoadGraph* rg, int nodeid)
{
  if(rg->GetNode_Rel() == NULL){
    cout<<"no road graph node rel"<<endl;
    return; 
  }
  if(nodeid < 1 || nodeid > rg->GetNode_Rel()->GetNoTuples()){
      cout<<"invalid node id "<<endl; 
      return; 
  }

  cout<<"total "<<rg->GetNode_Rel()->GetNoTuples()<<" nodes "<<endl;
  cout<<"total "<<rg->GetEdge_Rel1()->GetNoTuples() +
                  rg->GetEdge_Rel2()->GetNoTuples()<<" edges "<<endl;

  Relation* node_rel = rg->GetNode_Rel();
  
  Tuple* jun_tuple = node_rel->GetTuple(nodeid, false);
  GPoint* jun1 = (GPoint*)jun_tuple->GetAttribute(RoadGraph::RG_JUN_GP);
//  cout<<*jun1<<endl; 

///////////////////////////////////////////////////////////////////////////
  //////the first kind of connection (no path; the same spatial location)////
  ////////////////////////////////////////////////////////////////////////////
  vector<int> tid_list1; 
  rg->FindAdj1(nodeid, tid_list1); 
  
  for(unsigned int i = 0;i < tid_list1.size();i++){
    Tuple* edge_tuple = rg->GetEdge_Rel1()->GetTuple(tid_list1[i], false);
    int neighbor_id = 
     ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN2))->GetIntval();
    edge_tuple->DeleteIfAllowed();

    Tuple* jun_neighbor1 = node_rel->GetTuple(neighbor_id, false);
    GPoint* gp2 = (GPoint*)jun_neighbor1->GetAttribute(RoadGraph::RG_JUN_GP);

    GLine* path = new GLine(0);

    jun_list1.push_back(*jun1);
    jun_list2.push_back(*gp2);
    gline_list.push_back(*path);
    delete path; 

    jun_neighbor1->DeleteIfAllowed();
    type_list.push_back(1); 
  }
  
  ////////////////////////////////////////////////////////////////////
  //////the second kind of connection (connected by moving metros)////
  ////////////////////////////////////////////////////////////////////
  
  vector<int> tid_list2; 
  rg->FindAdj2(nodeid, tid_list2); 

  for(unsigned int i = 0;i < tid_list2.size();i++){

    Tuple* edge_tuple = rg->GetEdge_Rel2()->GetTuple(tid_list2[i], false);
    int neighbor_id = 
     ((CcInt*)edge_tuple->GetAttribute(RoadGraph::RG_JUN_2))->GetIntval();
    Tuple* jun_neighbor2 = node_rel->GetTuple(neighbor_id, false);

    GPoint* gp3 = 
      (GPoint*)jun_neighbor2->GetAttribute(RoadGraph::RG_JUN_GP);


    GLine* path = 
          (GLine*)edge_tuple->GetAttribute(RoadGraph::RG_PATHA1);
    jun_list1.push_back(*jun1);
    jun_list2.push_back(*gp3);
    gline_list.push_back(*path);

    edge_tuple->DeleteIfAllowed();
    jun_neighbor2->DeleteIfAllowed();

    type_list.push_back(2); 
  }



  jun_tuple->DeleteIfAllowed();
}