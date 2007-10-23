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

1.1 Implementation of Network

Mai-September 2007 Martin Scheppokat

Parts of the source taken from Victor Almeida

Defines, includes, and constants

*/
#include <sstream>

#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "GPoint.h"
#include "SpatialAlgebra.h"

#include "Network.h"

#include "StandardTypes.h"
#include "Algebra.h"

extern NestedList* nl;
extern QueryProcessor* qp;

string Network::routesTypeInfo =
      "(rel (tuple ((id int) (length real) (curve line) "
      "(dual bool) (startsSmaller bool))))";
string Network::routesBTreeTypeInfo =
      "(btree (tuple ((id int) (length real) (curve line) "
      "(dual bool) (startsSmaller bool))) int)";
string Network::junctionsTypeInfo =
      "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int))))";
string Network::junctionsInternalTypeInfo =
      "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int) (pos point) (r1rc tid) (r2rc tid) "
      "(sauprc tid) (sadownrc tid)(sbuprc tid) (sbdownrc tid))))";
string Network::junctionsBTreeTypeInfo =
      "(btree (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int) (pos point) (r1rc tid) (r2rc tid) "
      "(sauprc tid) (sadownrc tid)(sbuprc tid) (sbdownrc tid))) int)";
string Network::sectionsInternalTypeInfo =
      "(rel (tuple ((rid int) (meas1 real) (meas2 real) "
      "(dual bool) (curve line)(curveStartsSmaller bool) (rrc int))))";


/*
Constructor Network

*/                                     
Network::Network():
m_iId(0),
m_bDefined(false),
m_pRoutes(0),
m_pJunctions(0),
m_pSections(0),
m_pBTreeRoutes(0),
m_pBTreeJunctionsByRoute1(0),
m_pBTreeJunctionsByRoute2(0),
m_xAdjacencyList(0),
m_xSubAdjacencyList(0)
{
}

/*
Constructor reading a network from a record

*/                                     
Network::Network(SmiRecord& in_xValueRecord, 
                 size_t& inout_iOffset, 
                 const ListExpr in_xTypeInfo):
m_xAdjacencyList(0),
m_xSubAdjacencyList(0)
{
  // Read network id
  in_xValueRecord.Read( &m_iId, sizeof( int ), inout_iOffset );
  inout_iOffset += sizeof( int );

  // Open routes
  ListExpr xType;
  nl->ReadFromString(routesTypeInfo, xType);
  ListExpr xNumericType = SecondoSystem::GetCatalog()->NumericType(xType);
  m_pRoutes = Relation::Open(in_xValueRecord, 
                             inout_iOffset, 
                             xNumericType);
  if(!m_pRoutes)
  { 
    return;
  }

  // Open junctions
  nl->ReadFromString(junctionsInternalTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pJunctions = Relation::Open(in_xValueRecord, 
                                inout_iOffset, 
                                xNumericType);
  if(!m_pJunctions) 
  {  
    m_pRoutes->Delete(); 
    return;
  }

  // Open sections  
  nl->ReadFromString(sectionsInternalTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pSections = Relation::Open(in_xValueRecord, 
                               inout_iOffset, 
                               xNumericType);
  if(!m_pSections) 
  {
    m_pRoutes->Delete(); 
    m_pJunctions->Delete(); 
    return;
  }

  // Open btree for routes
  nl->ReadFromString(routesBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pBTreeRoutes = BTree::Open(in_xValueRecord, 
                               inout_iOffset, 
                               xNumericType);
         
  if(!m_pBTreeRoutes) 
  {
    m_pRoutes->Delete(); 
    m_pJunctions->Delete(); 
    m_pSections->Delete();
    return;
  }

  // Open first btree for junctions  
  nl->ReadFromString(junctionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pBTreeJunctionsByRoute1 = BTree::Open(in_xValueRecord, 
                                          inout_iOffset, 
                                          xNumericType);
  if(!m_pBTreeJunctionsByRoute1) 
  {
    m_pRoutes->Delete(); 
    m_pJunctions->Delete(); 
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    return;
  }

  // Open second btree for junctions
  nl->ReadFromString(junctionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  m_pBTreeJunctionsByRoute2 = BTree::Open(in_xValueRecord, 
                                          inout_iOffset, 
                                          xNumericType); 
  if(!m_pBTreeJunctionsByRoute2) 
  {
    m_pRoutes->Delete(); 
    m_pJunctions->Delete(); 
    m_pSections->Delete();
    delete m_pBTreeRoutes;
    delete m_pBTreeJunctionsByRoute1;
    return;
  }

  OpenAdjacencyList(in_xValueRecord,
                    inout_iOffset);

  OpenSubAdjacencyList(in_xValueRecord,
                       inout_iOffset);
  
  m_bDefined = true;
}


/*
Construktor reading a network from a list

*/                                     
Network::Network(ListExpr in_xValue,
                 int in_iErrorPos,
                 ListExpr& inout_xErrorInfo, 
                 bool& inout_bCorrect):
m_iId(0),
m_bDefined(false),
m_pRoutes(0),
m_pJunctions(0),
m_pSections(0),
m_pBTreeRoutes(0),
m_pBTreeJunctionsByRoute1(0),
m_pBTreeJunctionsByRoute2(0),
m_xAdjacencyList(0),
m_xSubAdjacencyList(0)
{
  // Check the list
  if(!(nl->ListLength(in_xValue) == 3))
  {
    string strErrorMessage = "Network(): List length must be 3.";
    inout_xErrorInfo = 
    nl->Append(inout_xErrorInfo, nl->StringAtom(strErrorMessage));
    inout_bCorrect = false;
    return;
  }   
  
  // Get type-info for temporary table
  ListExpr xType; 
  nl->ReadFromString(routesTypeInfo, xType);
  ListExpr xRoutesNumType = SecondoSystem::GetCatalog()->NumericType(xType);
  nl->ReadFromString(junctionsTypeInfo, xType);
  ListExpr xJunctionsNumType = SecondoSystem::GetCatalog()->NumericType(xType);

  // Split into the three parts
  ListExpr xIdList = nl->First(in_xValue);
  ListExpr xRouteList = nl->Second(in_xValue);
  ListExpr xJunctionList = nl->Third(in_xValue);
  // Sections will be calculated in the load-method
 
  // Read Id 
  if(!nl->IsAtom(xIdList) ||
     nl->AtomType(xIdList) != IntType)
  {
    string strErrorMessage = "Network(): Id is missing.";
    inout_xErrorInfo = nl->Append(inout_xErrorInfo, 
                                  nl->StringAtom(strErrorMessage));
    inout_bCorrect = false;
    return;
  }
  m_iId = nl->IntValue(xIdList);
 
  // Create new temporary relations. 
  Relation* pRoutes = new Relation(xRoutesNumType, true);
  Relation* pJunctions = new Relation(xJunctionsNumType, true);

  // Iterate over all routes
  while(!nl->IsEmpty(xRouteList))
  {
    ListExpr xCurrentRoute = nl->First(xRouteList);
    xRouteList = nl->Rest(xRouteList);

    // Create tuple for internal table
    Tuple* pNewRoute = new Tuple(nl->Second(xRoutesNumType));

    // Check this part of the list
    if(nl->ListLength(xCurrentRoute) != 5 ||
      (!nl->IsAtom(nl->First(xCurrentRoute))) ||
      nl->AtomType(nl->First(xCurrentRoute)) != IntType || 
      (!nl->IsAtom(nl->Second(xCurrentRoute))) ||
      nl->AtomType(nl->Second(xCurrentRoute)) != RealType ||
      (nl->IsAtom(nl->Third(xCurrentRoute))) ||
      (!nl->IsAtom(nl->Fourth(xCurrentRoute))) ||
      nl->AtomType(nl->Fourth(xCurrentRoute)) != BoolType ||
      (!nl->IsAtom(nl->Fifth(xCurrentRoute))) ||
      nl->AtomType(nl->Fifth(xCurrentRoute)) != BoolType)
    {
      delete pRoutes;
      delete pRoutes;

      string strErrorMessage = "Network(): Error while reading out routes.";
      inout_xErrorInfo = nl->Append(inout_xErrorInfo, 
                                    nl->StringAtom(strErrorMessage));
      inout_bCorrect = false;
      return;
    }
    
    // Read attributes from list
    // Read values from table
    int iRouteId = nl->IntValue(nl->First(xCurrentRoute));
    double dLength  = nl->RealValue(nl->Second(xCurrentRoute));
    Word xLineWord = InLine(nl->TheEmptyList(),
                            nl->Third(xCurrentRoute),
                            in_iErrorPos,
                            inout_xErrorInfo,
                            inout_bCorrect);
    Line* pLine = (Line*)(xLineWord.addr);
    bool bDual= nl->BoolValue(nl->Fourth(xCurrentRoute));
    bool bStartsSmaller  = nl->BoolValue(nl->Fifth(xCurrentRoute));
    
    // Set all necessary attributes
    pNewRoute->PutAttribute(ROUTE_ID, new CcInt(true, iRouteId));
    pNewRoute->PutAttribute(ROUTE_LENGTH, new CcReal(true, dLength));
    pNewRoute->PutAttribute(ROUTE_CURVE, pLine);
    pNewRoute->PutAttribute(ROUTE_DUAL, new CcBool(true, bDual));
    pNewRoute->PutAttribute(ROUTE_STARTSSMALLER, new CcBool(true, 
                                                              bStartsSmaller));
      
    // Append new junction
    pRoutes->AppendTuple(pNewRoute);    
  }
 
   // Iterate over all junctions
  while(!nl->IsEmpty(xJunctionList))
  {
    ListExpr xCurrentJunction = nl->First(xJunctionList);
    xJunctionList = nl->Rest(xJunctionList);

    // Create tuple for internal table
    Tuple* pNewJunction = new Tuple(nl->Second(xJunctionsNumType));
  
    // Check this part of the list
    if(nl->ListLength(xCurrentJunction) != 6 ||
      (!nl->IsAtom(nl->First(xCurrentJunction))) ||
      nl->AtomType(nl->First(xCurrentJunction)) != IntType || 
      (!nl->IsAtom(nl->Second(xCurrentJunction))) ||
      nl->AtomType(nl->Second(xCurrentJunction)) != RealType ||
      (!nl->IsAtom(nl->Third(xCurrentJunction))) ||
      nl->AtomType(nl->Third(xCurrentJunction)) != IntType ||
      (!nl->IsAtom(nl->Fourth(xCurrentJunction))) ||
      nl->AtomType(nl->Fourth(xCurrentJunction)) != RealType ||
      (!nl->IsAtom(nl->Fifth(xCurrentJunction))) ||
      nl->AtomType(nl->Fifth(xCurrentJunction)) != IntType)
    {
      delete pRoutes;
      delete pJunctions;

      string strErrorMessage = "Network(): Error while reading out junctions.";
      inout_xErrorInfo = nl->Append(inout_xErrorInfo, 
                         nl->StringAtom(strErrorMessage));
      inout_bCorrect = false;
      return;
    }
    
    // Read attributes from list
    int iRoute1Id = nl->IntValue(nl->First(xCurrentJunction));
    double dMeas1 = nl->RealValue(nl->Second(xCurrentJunction));
    int iRoute2Id = nl->IntValue(nl->Third(xCurrentJunction));
    double dMeas2 = nl->RealValue(nl->Fourth(xCurrentJunction));
    int iConnectivityCode= nl->IntValue(nl->Fifth(xCurrentJunction));
    // The location of the junction "Point" is calculated in the load-method
  
    // Set all necessary attributes
    pNewJunction->PutAttribute(JUNCTION_ROUTE1_ID, 
                                new CcInt(true, iRoute1Id));
    pNewJunction->PutAttribute(JUNCTION_ROUTE1_MEAS, 
                                new CcReal(true, dMeas1));
    pNewJunction->PutAttribute(JUNCTION_ROUTE2_ID, 
                                new CcInt(true, iRoute2Id));
    pNewJunction->PutAttribute(JUNCTION_ROUTE2_MEAS, 
                                new CcReal(true, dMeas2));
    pNewJunction->PutAttribute(JUNCTION_CC, 
                                new CcInt(true, iConnectivityCode));
      
    // Append new junction
    pJunctions->AppendTuple(pNewJunction);    
  }
 
  Load(m_iId,
       pRoutes, 
       pJunctions);
       
  delete pRoutes;
  delete pJunctions;  


  m_bDefined = true;  
}

/*
Destructor

*/                                     
Network::~Network()
{
  delete m_pRoutes;
  delete m_pJunctions;
  delete m_pSections;
  delete m_pBTreeRoutes;
  delete m_pBTreeJunctionsByRoute1;
  delete m_pBTreeJunctionsByRoute2;
}

/*
Method Destroy -- Removing a network from the database

*/                                     
void Network::Destroy()
{
  assert(m_pRoutes != 0);
  m_pRoutes->Delete(); m_pRoutes = 0;
  assert(m_pJunctions != 0);
  m_pJunctions->Delete(); m_pJunctions = 0;
  assert(m_pSections != 0);
  m_pSections->Delete(); m_pSections = 0;
  assert(m_pBTreeRoutes != 0);
  m_pBTreeRoutes->DeleteFile(); 
  delete m_pBTreeRoutes; m_pBTreeRoutes = 0;
  m_pBTreeJunctionsByRoute1->DeleteFile(); 
  delete m_pBTreeJunctionsByRoute1; m_pBTreeJunctionsByRoute1 = 0;
  m_pBTreeJunctionsByRoute2->DeleteFile(); 
  delete m_pBTreeJunctionsByRoute2; m_pBTreeJunctionsByRoute2 = 0;
  m_xAdjacencyList.Destroy();
  m_xSubAdjacencyList.Destroy();  
}

/*
Method Load -- Create a network from two external relations

*/                                     
void Network::Load(int in_iId,
                   const Relation* in_pRoutes, 
                   const Relation* in_pJunctions)
{
  m_iId = in_iId;
  FillRoutes(in_pRoutes);
  FillJunctions(in_pJunctions);
  FillSections();
  FillAdjacencyLists();
  m_bDefined = true;
}

/*
Method FillRoutes

*/                                     
void Network::FillRoutes(const Relation *routes)
{
  ostringstream xRoutesPtrStream;
  xRoutesPtrStream << (long)routes;

  string strQuery = "(consume (sort (feed (" + routesTypeInfo + 
                       " (ptr " + xRoutesPtrStream.str() + ")))))";

  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  m_pRoutes = (Relation *)xResult.addr;

  // Create B-Tree for the routes
  ostringstream xThisRoutesPtrStream;
  xThisRoutesPtrStream << (long)m_pRoutes;
  strQuery = "(createbtree (" + routesTypeInfo + 
                " (ptr " + xThisRoutesPtrStream.str() + "))" + " id)";

  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted); // no query with side effects, please!
  m_pBTreeRoutes = (BTree*)xResult.addr;
}

/*
Method FillJunctions

*/                                     
void Network::FillJunctions(const Relation *in_pJunctions)
{
  /////////////////////////////////////////////////////////////////////
  //
  // Create new table for the junctions
  //
  ListExpr xTypeInfo;
  nl->ReadFromString(junctionsInternalTypeInfo, xTypeInfo);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xTypeInfo);
  Relation *pIntJunctions = new Relation(xNumType, true);

  /////////////////////////////////////////////////////////////////////
  //
  // Iterator for the input-table with junctions
  //
  GenericRelationIterator* pJunctionsIter = in_pJunctions->MakeScan();
  Tuple* pCurrentJunction;
  while((pCurrentJunction = pJunctionsIter->GetNextTuple()) != 0)
  {
    /////////////////////////////////////////////////////////////////////
    //
    // Create tuple for internal table and copy all attributes from input
    //
    Tuple* pNewJunction = new Tuple(nl->Second(xNumType));
    for(int i = 0; i < pCurrentJunction->GetNoAttributes(); i++)
    {
      pNewJunction->CopyAttribute(i, pCurrentJunction, i);
    }
    

    /////////////////////////////////////////////////////////////////////
    //
    // Fill other fields of the table
    //
    
    // Store Pointer to the first route in the new relation.
    CcInt* pR1Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE1_ID);
    BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch(pR1Id);
    int NextIter = pRoutesIter->Next();
    assert(NextIter);
    TupleIdentifier *pR1RC = new TupleIdentifier(true, pRoutesIter->GetId());
    pNewJunction->PutAttribute(JUNCTION_ROUTE1_RC, pR1RC);
    
    // Calculate and store the exakt location of the junction.
    Tuple* pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
    assert(pRoute != 0);
    Line* pLine = (Line*)pRoute->GetAttribute(ROUTE_CURVE);
    assert(pLine != 0);
    CcReal* pMeas = (CcReal*)pNewJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    Point* pPoint = new Point(false);
    pLine->AtPosition(pMeas->GetRealval(), true, *pPoint);
    pNewJunction->PutAttribute(JUNCTION_POS, pPoint);

    pRoute->DeleteIfAllowed();
    delete pRoutesIter;

    // Store Pointer to the second route in the new relation.
    CcInt* pR2Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE2_ID);
    pRoutesIter = m_pBTreeRoutes->ExactMatch(pR2Id);
    // TODO: Fehlerbehandlung verbessern
    NextIter = pRoutesIter->Next();
    assert(NextIter); // no query with side effects, please!
    TupleIdentifier *pR2RC = new TupleIdentifier(true, pRoutesIter->GetId());
    pNewJunction->PutAttribute(JUNCTION_ROUTE2_RC, pR2RC);
    delete pRoutesIter;

    // Pointers to sections are filled in FillSections 
    pNewJunction->PutAttribute(JUNCTION_SECTION_AUP_RC,
                                   new TupleIdentifier(false));
    pNewJunction->PutAttribute(JUNCTION_SECTION_ADOWN_RC,
                                   new TupleIdentifier(false));
    pNewJunction->PutAttribute(JUNCTION_SECTION_BUP_RC,
                                   new TupleIdentifier(false));
    pNewJunction->PutAttribute(JUNCTION_SECTION_BDOWN_RC,
                                   new TupleIdentifier(false));

    /////////////////////////////////////////////////////////////////////
    //
    // Append new junction
    //
    pIntJunctions->AppendTuple(pNewJunction);

    pCurrentJunction->DeleteIfAllowed();
    pNewJunction->DeleteIfAllowed();
  }
  delete pJunctionsIter;

  /////////////////////////////////////////////////////////////////////
  //
  // Sort the table which is now containing all junctions 
  //
  ostringstream xJunctionsStream;
  xJunctionsStream << (long)pIntJunctions;
  string strQuery = "(consume (sortby (feed (" + junctionsInternalTypeInfo +
                    " (ptr " + xJunctionsStream.str() + 
                    "))) ((r1id asc)(meas1 asc))))";


  Word xResult;
  int QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted);
  m_pJunctions = (Relation *)xResult.addr;
  
  // Delete internal table
  pIntJunctions->Delete();
  
  /////////////////////////////////////////////////////////////////////
  //
  // Create two b-trees for the junctions sorted by first and second id
  //
  ostringstream xThisJunctionsPtrStream;
  xThisJunctionsPtrStream << (long)m_pJunctions;
  strQuery = "(createbtree (" + junctionsInternalTypeInfo + 
                " (ptr " + xThisJunctionsPtrStream.str() + "))" + " r1id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted); // no query with side effects, please!
  m_pBTreeJunctionsByRoute1 = (BTree*)xResult.addr;
  
  ostringstream xThisJunctionsPtrStream2;
  xThisJunctionsPtrStream2 << (long)m_pJunctions;
  strQuery = "(createbtree (" + junctionsInternalTypeInfo + 
                " (ptr " + xThisJunctionsPtrStream2.str() + "))" + " r2id)";
  QueryExecuted = QueryProcessor::ExecuteQuery(strQuery, xResult);
  assert(QueryExecuted); // no query with side effects, please!
  m_pBTreeJunctionsByRoute2 = (BTree*)xResult.addr;
}


/*
Method FillSections 
 
*/
void Network::FillSections()
{
  // The method will iterate over routes
  GenericRelationIterator* pRoutesIt = m_pRoutes->MakeScan();

  // Create relation for sections
  ListExpr xType; 
  nl->ReadFromString(sectionsInternalTypeInfo, xType);
  ListExpr xNumType = SecondoSystem::GetCatalog()->NumericType(xType);
  m_pSections = new Relation(xNumType);
 
  /////////////////////////////////////////////////////////////////////
  //
  // Iterate over all Routes
  //
  Tuple* pRoute;
  int iSectionTid;
  while((pRoute = pRoutesIt->GetNextTuple()) != 0)
  {
    // Current position on route - starting at the beginning of the route
    double dCurrentPosOnRoute = 0;
    Line* pRouteCurve = (Line*)pRoute->GetAttribute(ROUTE_CURVE);
    int iTupleId = pRoute->GetTupleId();
    CcInt* xRouteId = (CcInt*)pRoute->GetAttribute(ROUTE_ID); 
    int iRouteId = xRouteId->GetIntval();
    bool bDual = ((CcBool*)pRoute->GetAttribute(ROUTE_DUAL))->GetBoolval();
    
    /////////////////////////////////////////////////////////////////////
    //
    // We need to find all junctions belonging to this route
    //
    vector<JunctionSortEntry> xJunctions;
    GetJunctionsOnRoute(xRouteId,
                        xJunctions);

    /////////////////////////////////////////////////////////////////////
    //
    // Now that we found all relevant junctions we can iterate over them.
    //
    JunctionSortEntry xCurrentEntry;
    xCurrentEntry.m_pJunction = 0;
    for(size_t i = 0; i < xJunctions.size(); i++)
    {
      // Get next junction
      xCurrentEntry = xJunctions[i];
    
      // Find values for the new section      
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = xCurrentEntry.getRouteMeas(); 

      // If the first junction is at the very start of the route, no 
      // section will be added
      if(xCurrentEntry.getRouteMeas() == 0)
      {
        continue;
      }

      /////////////////////////////////////////////////////////////////////
      //
      // Create a new section
      //
      // Section will only be created if the length is > 0. Otherwise the
      // one before remains valid.
      if(dEndPos - dStartPos > 0.01)
      {
        // A line for the section
        Line* pLine = new Line(0);
        
        // Take start from the route
        bool bStartSmaller = ((CcBool*)pRoute->GetAttribute(
                                  ROUTE_STARTSSMALLER))->GetBoolval();

        pRouteCurve->SubLine(dStartPos,
                             dEndPos,
                             bStartSmaller,
                             *pLine);

        // Find out, if the orientation of the subline differs from the position
        // of the line. If so, the direction has to be changed.
        bool bLineStartsSmaller;
        Point* pStartPoint = new Point(false);
        pRouteCurve->AtPosition(dStartPos, bStartSmaller, *pStartPoint);
        Point* pEndPoint = new Point(false);
        pRouteCurve->AtPosition(dEndPos, bStartSmaller, *pEndPoint);
        if(pStartPoint->GetX() < pEndPoint->GetX() ||
           (
             pStartPoint->GetX() == pEndPoint->GetX() &&
             pStartPoint->GetY() < pEndPoint->GetY()
           )
          )
        {
          // Normal orientation
          bLineStartsSmaller = true;
        }
        else
        {
          // Opposite orientation
          bLineStartsSmaller = false;
        }


      
        // The new section
        Tuple* pNewSection = new Tuple(nl->Second(xNumType));
        pNewSection->PutAttribute(SECTION_RID, new CcInt(true, iRouteId));
        pNewSection->PutAttribute(SECTION_DUAL, new CcBool(true, bDual));
        pNewSection->PutAttribute(SECTION_MEAS1, new CcReal(true, dStartPos));
        pNewSection->PutAttribute(SECTION_MEAS2, new CcReal(true, dEndPos));
        pNewSection->PutAttribute(SECTION_RRC, new CcInt(true, iTupleId));
        pNewSection->PutAttribute(SECTION_CURVE, pLine);
        pNewSection->PutAttribute(SECTION_CURVE_STARTS_SMALLER, 
                                  new CcBool(true, bLineStartsSmaller));
        m_pSections->AppendTuple(pNewSection);
        iSectionTid = pNewSection->GetTupleId();
        pNewSection->DeleteIfAllowed();

        // Update position for next loop
        dCurrentPosOnRoute = dEndPos;
      }
      
      /////////////////////////////////////////////////////////////////////
      //
      // Store ID of new section in junction behind that section.
      //
      vector<int> xIndices;
      vector<Attribute*> xAttrs;
      if(xCurrentEntry.m_bFirstRoute)
      {
        xIndices.push_back(JUNCTION_SECTION_ADOWN_RC);
        xAttrs.push_back(new TupleIdentifier(true, iSectionTid));
      }
      else
      {
        xIndices.push_back(JUNCTION_SECTION_BDOWN_RC);
        xAttrs.push_back(new TupleIdentifier(true, iSectionTid));
      } 
      m_pJunctions->UpdateTuple(xCurrentEntry.m_pJunction,
                                xIndices, 
                                xAttrs);

    } // End junctions-loop

    /////////////////////////////////////////////////////////////////////
    //
    // The last section of the route is still missing, if the last 
    // junction is not at the end of the route.
    //
    if(dCurrentPosOnRoute < pRouteCurve->Length())
    {
      // Find values for the new section      
      int iRouteId = ((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval();
      bool bDual = ((CcBool*)pRoute->GetAttribute(ROUTE_DUAL))->GetBoolval();
      double dStartPos = dCurrentPosOnRoute;
      double dEndPos = pRouteCurve->Length();
      int iTupleId = pRoute->GetTupleId();

      // Calculate line
      Line* pLine = new Line(0);
      bool bStartSmaller = ((CcBool*)pRoute->GetAttribute(
                                ROUTE_STARTSSMALLER))->GetBoolval();
      pRouteCurve->SubLine(dStartPos,
                           dEndPos,
                           bStartSmaller,
                           *pLine);

      // Find out, if the orientation of the subline differs from the position
      // of the line. If so, the direction has to be changed.
      bool bLineStartsSmaller;
      Point* pStartPoint = new Point(false);
      pRouteCurve->AtPosition(dStartPos, bStartSmaller, *pStartPoint);
      Point* pEndPoint = new Point(false);
      pRouteCurve->AtPosition(dEndPos, bStartSmaller, *pEndPoint);
      if(pStartPoint->GetX() < pEndPoint->GetX() ||
         (
           pStartPoint->GetX() == pEndPoint->GetX() &&
           pStartPoint->GetY() < pEndPoint->GetY()
         )
        )
      {
        // Normal orientation
        bLineStartsSmaller = true;
      }
      else
      {
        // Opposite orientation
        bLineStartsSmaller = false;
      }
      
      // Create a new Section
      Tuple* pNewSection = new Tuple(nl->Second(xNumType));
      pNewSection->PutAttribute(SECTION_RID, new CcInt(true, iRouteId));
      pNewSection->PutAttribute(SECTION_DUAL, new CcBool(true, bDual));
      pNewSection->PutAttribute(SECTION_MEAS1, new CcReal(true, dStartPos));
      pNewSection->PutAttribute(SECTION_MEAS2, new CcReal(true, dEndPos));
      pNewSection->PutAttribute(SECTION_RRC, new CcInt(true, iTupleId));
      pNewSection->PutAttribute(SECTION_CURVE, pLine);
      pNewSection->PutAttribute(SECTION_CURVE_STARTS_SMALLER, 
                                new CcBool(true, bLineStartsSmaller));
      m_pSections->AppendTuple(pNewSection);

      // Store ID of new section in Junction
      if(xCurrentEntry.m_pJunction != 0)
      {
        vector<int> xIndicesLast;
        vector<Attribute*> xAttrsLast;
        if(xCurrentEntry.m_bFirstRoute)
        {
          xIndicesLast.push_back(JUNCTION_SECTION_AUP_RC);
          xAttrsLast.push_back(new TupleIdentifier(true, 
                                                   pNewSection->GetTupleId()));
        }
        else
        {
          xIndicesLast.push_back(JUNCTION_SECTION_BUP_RC);
          xAttrsLast.push_back(new TupleIdentifier(true, 
                                                   pNewSection->GetTupleId()));
        } 
        m_pJunctions->UpdateTuple(xCurrentEntry.m_pJunction, 
                                  xIndicesLast, 
                                  xAttrsLast);
      }       

      pNewSection->DeleteIfAllowed();
    } // end if

    ////////////////////////////////////////////////////////////////////
    //
    // Fill Up-Pointers of all sections but the last 
    //
    for(int i = xJunctions.size()-2; i >= 0; i--)
    {
      // Get next junction
      JunctionSortEntry xEntry = xJunctions[i];
      JunctionSortEntry xEntryBehind = xJunctions[i + 1];    

      vector<int> xIndices;
      if(xEntry.m_bFirstRoute)
      {
        xIndices.push_back(JUNCTION_SECTION_AUP_RC);
      }
      else
      {
        xIndices.push_back(JUNCTION_SECTION_BUP_RC);
      } 
      vector<Attribute*> xAttrs;
      if(xEntryBehind.getRouteMeas() - xEntry.getRouteMeas() < 0.01 )
      {
        // Two junctions at the same place. In this case they do have
        // the same up-pointers
        if(xEntryBehind.m_bFirstRoute)
        {
          Tuple* pJunction = xEntryBehind.m_pJunction;
          CcInt* xTid;
          xTid = (CcInt*)pJunction->GetAttribute(JUNCTION_SECTION_AUP_RC);
          int iTid = xTid->GetIntval();
          xAttrs.push_back(new TupleIdentifier(true, iTid));
        }
        else
        {
          Tuple* pJunction = xEntryBehind.m_pJunction;
          CcInt* xTid;
          xTid = (CcInt*)pJunction->GetAttribute(JUNCTION_SECTION_BUP_RC);
          int iTid = xTid->GetIntval();
          xAttrs.push_back(new TupleIdentifier(true, iTid));
        }
      }
      else
      {
        // Junctions not on the same place. The down-pointer of the second is
        // the up-pointer of the first.
        if(xEntryBehind.m_bFirstRoute)
        {
          Tuple* pJunction = xEntryBehind.m_pJunction;
          CcInt* xTid;
          xTid = (CcInt*)pJunction->GetAttribute(JUNCTION_SECTION_ADOWN_RC);
          int iTid = xTid->GetIntval();
          xAttrs.push_back(new TupleIdentifier(true, iTid));
        }
        else
        {
          Tuple* pJunction = xEntryBehind.m_pJunction;
          CcInt* xTid;
          xTid = (CcInt*)pJunction->GetAttribute(JUNCTION_SECTION_BDOWN_RC);
          int iTid = xTid->GetIntval();
          xAttrs.push_back(new TupleIdentifier(true, iTid));
        }
      } 
      m_pJunctions->UpdateTuple(xEntry.m_pJunction, 
                                xIndices, 
                                xAttrs);

    }
    
    pRoute->DeleteIfAllowed();
  } // End while Routes
  delete pRoutesIt;
}


/*
Method FillAdjacencyLists

*/
void Network::FillAdjacencyLists()
{
  // Adjust the adjacenzy list to the correct size. From each
  // section four directions are possible - including u-turns
  m_xAdjacencyList.Resize(m_pSections->GetNoTuples() * 2);
  for(int i = 0; i < m_pSections->GetNoTuples() * 2; i++)
  {
    m_xAdjacencyList.Put(i, AdjacencyListEntry(-1, -1));
  }
  
  GenericRelationIterator* pJunctionsIt = m_pJunctions->MakeScan();
  Tuple* pCurrentJunction;

  /////////////////////////////////////////////////////////////////////////
  //
  // In a first step all pairs of adjacent sections will be collected
  //
  vector<DirectedSectionPair> xList;
  while((pCurrentJunction = pJunctionsIt->GetNextTuple()) != 0)
  {
    //////////////////////////////////
    //
    // Retrieve the connectivity code
    //
    CcInt* pCc = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_CC); 
    int iCc = pCc->GetIntval();
    ConnectivityCode xCc(iCc);

    //////////////////////////////////
    //
    // Retrieve the four sections - if they exist
    //
    // (This should also be possible without loading the Section itself)
    //
    TupleIdentifier* pTid;
    Tuple* pAUp = 0;
    Tuple* pADown = 0;
    Tuple* pBUp = 0;
    Tuple* pBDown = 0;
    
    // First section     
    Attribute* pAttr = pCurrentJunction->GetAttribute(JUNCTION_SECTION_AUP_RC);
    pTid = (TupleIdentifier*)pAttr; 
    if(pTid->GetTid() > 0)
    {
      pAUp = m_pSections->GetTuple(pTid->GetTid());
    }  

    // Second section     
    pAttr = pCurrentJunction->GetAttribute(JUNCTION_SECTION_ADOWN_RC);  
    pTid = (TupleIdentifier*)pAttr; 
    if(pTid->GetTid() > 0)
    {
      pADown = m_pSections->GetTuple(pTid->GetTid());
    }  
    
    // Third section     
    pAttr = pCurrentJunction->GetAttribute(JUNCTION_SECTION_BUP_RC);  
    pTid = (TupleIdentifier*)pAttr; 
    if(pTid->GetTid() > 0)
    {
      pBUp = m_pSections->GetTuple(pTid->GetTid());
    }  
    
    // Fourth section     
    pAttr = pCurrentJunction->GetAttribute(JUNCTION_SECTION_BDOWN_RC);  
    pTid = (TupleIdentifier*)pAttr; 
    if(pTid->GetTid() > 0)
    {
      pBDown = m_pSections->GetTuple(pTid->GetTid());
    }  
        
    //////////////////////////////////
    //
    // If a section is existing and the transition is possible
    // it will be added to the list.
    //

    // First section
    FillAdjacencyPair(pAUp, false, pAUp, true, xCc, AUP_AUP, xList);
    FillAdjacencyPair(pAUp, false, pADown, false, xCc, AUP_ADOWN, xList);
    FillAdjacencyPair(pAUp, false, pBUp, true, xCc, AUP_BUP, xList);
    FillAdjacencyPair(pAUp, false, pBDown, false, xCc, AUP_BDOWN, xList);

    // Second section
    FillAdjacencyPair(pADown, true, pAUp, true, xCc, ADOWN_AUP, xList);
    FillAdjacencyPair(pADown, true, pADown, false, xCc, ADOWN_ADOWN, xList);
    FillAdjacencyPair(pADown, true, pBUp, true, xCc, ADOWN_BUP, xList);
    FillAdjacencyPair(pADown, true, pBDown, false, xCc, ADOWN_BDOWN, xList);

    // Third section
    FillAdjacencyPair(pBUp, false, pAUp, true, xCc, BUP_AUP, xList);
    FillAdjacencyPair(pBUp, false, pADown, false, xCc, BUP_ADOWN, xList);
    FillAdjacencyPair(pBUp, false, pBUp, true, xCc, BUP_BUP, xList);
    FillAdjacencyPair(pBUp, false, pBDown, false, xCc, BUP_BDOWN, xList);

    // Fourth section
    FillAdjacencyPair(pBDown, true, pAUp, true, xCc, BDOWN_AUP, xList);
    FillAdjacencyPair(pBDown, true, pADown, false, xCc, BDOWN_ADOWN, xList);
    FillAdjacencyPair(pBDown, true, pBUp, true, xCc, BDOWN_BUP, xList);
    FillAdjacencyPair(pBDown, true, pBDown, false, xCc, BDOWN_BDOWN, xList);
        
    pCurrentJunction->DeleteIfAllowed();
  }
  delete pJunctionsIt;


  /////////////////////////////////////////////////////////////////////////
  //
  // Now - as the second step the adjacency lists are filled.
  //
  // Sort the list by the first directed section
  sort(xList.begin(),
       xList.end());

  DirectedSectionPair xLastPair;
  int iLow = 0; 
  for(size_t i = 0; i < xList.size(); i++)
  {
    // Get next
    DirectedSectionPair xPair = xList[i];

    // Entry in adjacency list if all sections adjacent to one section have 
    // been found. This is the case every time the first section changes. Never
    // at the first entry and always at the last. 
    if(i == xList.size() -1 ||
       (
        i != 0 &&
        (
         xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
         xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown
        )
       )
      )
    {
      int iHigh = m_xSubAdjacencyList.Size()-1;
      int iIndex = 2 * (xLastPair.m_iFirstSectionTid - 1);
      iIndex += xLastPair.m_bFirstUpDown ? 1 : 0;
      m_xAdjacencyList.Put(iIndex, AdjacencyListEntry(iLow, iHigh));
      
      iLow = iHigh + 1;
    }                                                   

    // Check if entry allready exists in list. As the list is sorted it
    // has to be the entry before.
    if(i == 0 ||
       xLastPair.m_iFirstSectionTid != xPair.m_iFirstSectionTid ||
       xLastPair.m_bFirstUpDown != xPair.m_bFirstUpDown ||
       xLastPair.m_iSecondSectionTid != xPair.m_iSecondSectionTid ||
       xLastPair.m_bSecondUpDown != xPair.m_bSecondUpDown)
    {
      // Append new entry to sub-list
      m_xSubAdjacencyList.Append(DirectedSection(xPair.m_iSecondSectionTid, 
                                                 xPair.m_bSecondUpDown));
    }                                            
    xLastPair = xPair;
  }
}

/*
Method FillAdjacenyPair

*/                                     
void Network::FillAdjacencyPair(Tuple* in_pFirstSection,
                                    bool in_bFirstUp,
                                    Tuple* in_pSecondSection,
                                    bool in_bSecondUp,
                                    ConnectivityCode in_xCc,
                                    Transition in_xTransition,
                                    vector<DirectedSectionPair> &inout_xPairs)
{
  if(in_pFirstSection != 0 && 
     in_pSecondSection != 0 && 
     in_xCc.IsPossible(in_xTransition))
    {
      inout_xPairs.push_back(DirectedSectionPair(in_pFirstSection->GetTupleId(),
                                                in_bFirstUp,
                                                in_pSecondSection->GetTupleId(),
                                                in_bSecondUp));
    }
}

/*
Returns the id of this network

*/
int Network::GetId()
{
  return m_iId;
}


Relation *Network::GetRoutes()
{
  return m_pRoutes->Clone();
}


Relation *Network::GetJunctions()
{
  ostringstream strJunctionsPtr;
  strJunctionsPtr << (long)m_pJunctions;

  string querystring = "(consume (feed (" + junctionsInternalTypeInfo +
                       " (ptr " + strJunctionsPtr.str() + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery(querystring, resultWord);
  assert(QueryExecuted); // no ASSERT with side effects, please
  return (Relation *)resultWord.addr;
}


void Network::GetJunctionsOnRoute(CcInt* in_pRouteId,
                                  vector<JunctionSortEntry> &inout_xJunctions)
{
  BTreeIterator* pJunctionsIt;
  pJunctionsIt = m_pBTreeJunctionsByRoute1->ExactMatch(in_pRouteId);
  while(pJunctionsIt->Next())
  {
    // Get next junction
    Tuple* pCurrentJunction = m_pJunctions->GetTuple(pJunctionsIt->GetId());
    inout_xJunctions.push_back(JunctionSortEntry(true, pCurrentJunction));
  }
  delete pJunctionsIt;
    
  // Now we look up the second b-tree
  pJunctionsIt = m_pBTreeJunctionsByRoute2->ExactMatch(in_pRouteId);
  while(pJunctionsIt->Next())
  {
    Tuple* pCurrentJunction = m_pJunctions->GetTuple(pJunctionsIt->GetId());
    inout_xJunctions.push_back(JunctionSortEntry(false, pCurrentJunction));
  }
  delete pJunctionsIt;

  // The junctions will be sorted by their mesure on the relevant route.
  sort(inout_xJunctions.begin(), 
       inout_xJunctions.end());
} 

Tuple* Network::GetSectionOnRoute(GPoint* in_xGPoint)
{
  vector<JunctionSortEntry> xJunctions;
  CcInt xRouteId(true, in_xGPoint->GetRouteId());
  GetJunctionsOnRoute(&xRouteId,
                      xJunctions);

  // Now that we found all relevant junctions we can iterate over them.
  int iSectionId = 0;
  for(size_t i = 0; i < xJunctions.size(); i++)
  {
    // Get next junction
    JunctionSortEntry xCurrentEntry = xJunctions[i];
    iSectionId = xCurrentEntry.getDownSectionId();
    if(xCurrentEntry.getRouteMeas() > in_xGPoint->GetPosition())
    {
      break;
    }
    iSectionId = xCurrentEntry.getUpSectionId();
  }
  for(size_t i = 0; i < xJunctions.size(); i++)
  {
    // Get next junction
    JunctionSortEntry xCurrentEntry = xJunctions[i];
    xCurrentEntry.m_pJunction->DeleteIfAllowed();
  }
  
  if(iSectionId == 0)
  {
    return 0;
  }
  Tuple* pSection = m_pSections->GetTuple(iSectionId);
  
  
  // Return the section
  return pSection;
}

Point* Network::GetPointOnRoute(GPoint* in_pGPoint)
{
  CcInt* pRouteId = new CcInt(true, in_pGPoint->GetRouteId());

  BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch(pRouteId);
  delete pRouteId;

  int NextSuccess = pRoutesIter->Next();
  assert(NextSuccess); // No ASSERT with side effect, please!
  Tuple* pRoute = m_pRoutes->GetTuple(pRoutesIter->GetId());
  assert(pRoute != 0);
  Line* pLine = (Line*)pRoute->GetAttribute(ROUTE_CURVE);
  assert(pLine != 0);
  Point* pPoint = new Point(false);
  pLine->AtPosition(in_pGPoint->GetPosition(),true, *pPoint);

  pRoute->DeleteIfAllowed();
  delete pRoutesIter;

  return pPoint;
}

Relation* Network::GetSectionsInternal()
{
  return m_pSections;
}

Relation *Network::GetSections()
{
  ostringstream strSectionsPtr;
  strSectionsPtr << (long)m_pSections;

  string querystring = "(consume (feed (" + sectionsInternalTypeInfo +
                       " (ptr " + strSectionsPtr.str() + "))))";

  Word resultWord;
  int QueryExecuted = QueryProcessor::ExecuteQuery(querystring, resultWord);
  assert(QueryExecuted); // No ASSERT with side effect, please!
  return (Relation *)resultWord.addr;
}

void Network::GetAdjacentSections(int in_iSectionId,
                                  bool in_bUpDown,
                                  vector<DirectedSection> &inout_xSections)
{
  int iIndex = (2 * (in_iSectionId - 1)) + (in_bUpDown ? 1 : 0);
  const AdjacencyListEntry* xEntry;
  m_xAdjacencyList.Get(iIndex, xEntry);
  if(xEntry->m_iHigh != -1)
  {
    int iLow = xEntry->m_iLow;
    int iHigh = xEntry->m_iHigh;
    
    for (int i = iLow; i <= iHigh; i++) 
    {
      const DirectedSection* xSection;
      m_xSubAdjacencyList.Get(i, xSection);

      bool bUpDownFlag = ((DirectedSection*)xSection)->getUpDownFlag();
      int iSectionTid = ((DirectedSection*)xSection)->getSectionTid();
      inout_xSections.push_back(DirectedSection(iSectionTid,
      bUpDownFlag));

    }
  }
}


/*
~Out~-function of type constructor ~network~

*/
ListExpr Network::Out(ListExpr typeInfo)
{

  ///////////////////////
  // Output of all routes
  GenericRelationIterator *pRoutesIter = m_pRoutes->MakeScan();
  Tuple *pCurrentRoute;
  ListExpr xLast, xNext, xRoutes;
  bool bFirst = true;

  while((pCurrentRoute = pRoutesIter->GetNextTuple()) != 0)
  {
    // Read values from table
    CcInt* pRouteId = (CcInt*)pCurrentRoute->GetAttribute(ROUTE_ID); 
    int iRouteId = pRouteId->GetIntval();
    CcReal* pLength = (CcReal*)pCurrentRoute->GetAttribute(ROUTE_LENGTH); 
    double dLength  = pLength->GetRealval();
    Line *pCurve = (Line*)pCurrentRoute->GetAttribute(ROUTE_CURVE);
    // The list for the curve contains all segments of the curve.
    ListExpr xCurve = OutLine(nl->TheEmptyList(), SetWord(pCurve));
    CcBool* pDual = (CcBool*)pCurrentRoute->GetAttribute(ROUTE_DUAL); 
    bool bDual= pDual->GetBoolval();
    CcBool* pStartsSmaller; 
    pStartsSmaller = (CcBool*)pCurrentRoute->GetAttribute(ROUTE_STARTSSMALLER); 
    bool bStartsSmaller = pStartsSmaller->GetBoolval();
    
    // Build list
    xNext = nl->FiveElemList(nl->IntAtom(iRouteId),
                             nl->RealAtom(dLength),
                             xCurve,
                             nl->BoolAtom(bDual),
                             nl->BoolAtom(bStartsSmaller));
      
    // Create new list or append element to existing list
    if(bFirst)
    {
      xRoutes = nl->OneElemList(xNext);
      xLast = xRoutes;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append(xLast, xNext);
    }
    pCurrentRoute->DeleteIfAllowed(); 
  }
  delete pRoutesIter; 
  
  ///////////////////////
  // Output of all junctions
  GenericRelationIterator *pJunctionsIter = m_pJunctions->MakeScan();
  Tuple *pCurrentJunction;
  ListExpr xJunctions;
  bFirst = true;

  while((pCurrentJunction = pJunctionsIter->GetNextTuple()) != 0)
  {
    // Read values from table
    CcInt* pRoute1Id;
    pRoute1Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE1_ID); 
    int iRoute1Id = pRoute1Id->GetIntval();
    CcReal* pMeas1;
    pMeas1 = (CcReal*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE1_MEAS);
    double dMeas1 = pMeas1->GetRealval();
    CcInt* pRoute2Id;
    pRoute2Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE2_ID);
    int iRoute2Id = pRoute2Id->GetIntval();
    CcReal* pMeas2;
    pMeas2 = (CcReal*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE2_MEAS); 
    double dMeas2 = pMeas2->GetRealval();
    CcInt* pConnectivityCode;
    pConnectivityCode = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_CC);
    int iConnectivityCode= pConnectivityCode->GetIntval();
    Point* pPoint = (Point*)pCurrentJunction->GetAttribute(JUNCTION_POS);
    ListExpr xPoint = OutPoint(nl->TheEmptyList(), SetWord(pPoint));
    
    // Build list
    xNext = nl->SixElemList(nl->IntAtom(iRoute1Id),
                            nl->RealAtom(dMeas1),
                            nl->IntAtom(iRoute2Id),
                            nl->RealAtom(dMeas2),
                            nl->IntAtom(iConnectivityCode),
                            xPoint);
      
    // Create new list or append element to existing list
    if(bFirst)
    {
      xJunctions= nl->OneElemList(xNext);
      xLast = xJunctions;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append(xLast, xNext);
    }
    pCurrentJunction->DeleteIfAllowed(); 
  }
  
  delete pJunctionsIter;  
    
  return nl->ThreeElemList(nl->IntAtom(m_iId),
                           xRoutes,
                           xJunctions);
}



/*
~Save~-function of type constructor ~network~

*/
ListExpr Network::Save(SmiRecord& in_xValueRecord, 
                       size_t& inout_iOffset,
                       const ListExpr in_xTypeInfo)
{
  // Save id of the network
  int iId = m_iId;
  in_xValueRecord.Write(&iId, 
                        sizeof(int), 
                        inout_iOffset);
  inout_iOffset += sizeof(int);
  
  // Save routes
  ListExpr xType;
  nl->ReadFromString(routesTypeInfo, xType);
  ListExpr xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pRoutes->Save(in_xValueRecord, 
                      inout_iOffset, 
                      xNumericType))
  {                      
    return false;
  }

  // Save junctions
  nl->ReadFromString(junctionsInternalTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pJunctions->Save(in_xValueRecord, 
                         inout_iOffset, 
                         xNumericType))
  {
    return false;
  }

  // Save sections
  nl->ReadFromString(sectionsInternalTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pSections->Save(in_xValueRecord, 
                        inout_iOffset, 
                        xNumericType))
  {
    return false;
  }

  // Save btree for routes
  nl->ReadFromString(routesBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeRoutes->Save(in_xValueRecord, 
                           inout_iOffset,
                           xNumericType))
  {
    return false;
  }
  
  // Save first btree for junctions
  nl->ReadFromString(junctionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeJunctionsByRoute1->Save(in_xValueRecord, 
                                      inout_iOffset,
                                      xNumericType))
  {
    return false;
  }
      
  // Save second btree for junctions
  nl->ReadFromString(junctionsBTreeTypeInfo, xType);
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeJunctionsByRoute2->Save(in_xValueRecord, 
                                      inout_iOffset,
                                      xNumericType))
  {                                                  
    return false;
  }

  SaveAdjacencyList(in_xValueRecord,
                    inout_iOffset);    

  SaveSubAdjacencyList(in_xValueRecord,
                       inout_iOffset);    

  return true; 
}


/*
~Open~-function of type constructor ~network~

*/
Network *Network::Open(SmiRecord& in_xValueRecord, 
                       size_t& inout_iOffset, 
                       const ListExpr in_xTypeInfo)
{
  // Create network
  return new Network(in_xValueRecord,
                     inout_iOffset,
                     in_xTypeInfo);
}



ListExpr Network::NetworkProp()
{
  ListExpr examplelist = nl->TextAtom();
  nl->AppendText(examplelist,
                 "thenetwork(<routes-relation>, <junctions-relation>)");

  return (nl->TwoElemList(
            nl->TwoElemList(nl->StringAtom("Creation"),
                            nl->StringAtom("Example Creation")),
            nl->TwoElemList(examplelist,
                            nl->StringAtom("(let n = thenetwork(r, j))"))));
}

ListExpr Network::OutNetwork(ListExpr typeInfo, Word value)
{
  Network *n = (Network*)value.addr;
  return n->Out(typeInfo);
}

/*
~In~-function of type constructor ~network~

*/
Word Network::InNetwork(ListExpr in_xTypeInfo, 
                        ListExpr in_xValue,
                        int in_iErrorPos, 
                        ListExpr& inout_xErrorInfo, 
                        bool& inout_bCorrect)
{
  Network* pNetwork = new Network(in_xValue,
                                  in_iErrorPos,
                                  inout_xErrorInfo,
                                  inout_bCorrect);

  if(inout_bCorrect)
  {
    return SetWord(pNetwork);
  }
  else
  {
    delete pNetwork;
    return SetWord(0);
  }
}

/*
~Create~-function of type constructor ~network~

*/
Word Network::CreateNetwork(const ListExpr typeInfo)
{
  return SetWord(new Network());
}

/*
~Close~-function of type constructor ~network~

*/
void Network::CloseNetwork(const ListExpr typeInfo, Word& w)
{
  delete (Network*)w.addr;
}

/*
~Clone~-function of type constructor ~network~

Not implemented yet.

*/
Word Network::CloneNetwork(const ListExpr typeInfo, const Word& w)
{
  return SetWord(Address(0));
}

/*
~Delete~-function of type constructor ~network~

*/
void Network::DeleteNetwork(const ListExpr typeInfo, Word& w)
{
  Network* n = (Network*)w.addr;
  n->Destroy();
  delete n;
}

/*
~Check~-function of type constructor ~network~

*/
bool Network::CheckNetwork(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual(type, "network"));
}

/*
~Cast~-function of type constructor ~network~

*/
void* Network::CastNetwork(void* addr)
{
  return (0);
}

bool Network::SaveNetwork(SmiRecord& valueRecord,
                          size_t& offset,
                          const ListExpr typeInfo,
                          Word& value)
{
  Network *n = (Network*)value.addr;
  return n->Save(valueRecord, offset, typeInfo);
}

bool Network::OpenNetwork(SmiRecord& valueRecord, 
                           size_t& offset, 
                           const ListExpr typeInfo, 
                           Word& value)
{
  value.addr = Network::Open(valueRecord, offset, typeInfo);
  return value.addr != 0;
}

/*
~SizeOf~-function of type constructor ~network~

*/
int Network::SizeOfNetwork()
{
  return 0;
}


void Network::SaveAdjacencyList(SmiRecord& in_xValueRecord, 
                                size_t& inout_iOffset)
{
  int iSize = m_xAdjacencyList.Size();
  in_xValueRecord.Write(&iSize, 
                        sizeof(int), 
                        inout_iOffset);
  inout_iOffset += sizeof(int);

  for (int i = 0; i < m_xAdjacencyList.Size(); ++i) 
  {
    // Read current entry
    const AdjacencyListEntry* xEntry;
    m_xAdjacencyList.Get(i, xEntry);

    // Write high
    in_xValueRecord.Write(&xEntry->m_iHigh, 
                          sizeof(int), 
                          inout_iOffset);
    inout_iOffset += sizeof(int);

    // Write low
    in_xValueRecord.Write(&xEntry->m_iLow, 
                          sizeof(int), 
                          inout_iOffset);
    inout_iOffset += sizeof(int);    
  }
}

void Network::SaveSubAdjacencyList(SmiRecord& in_xValueRecord, 
                                   size_t& inout_iOffset)
{
  int iSize = m_xSubAdjacencyList.Size();
  in_xValueRecord.Write(&iSize, 
                        sizeof(int), 
                        inout_iOffset);
  inout_iOffset += sizeof(int);

  for (int i = 0; i < m_xSubAdjacencyList.Size(); ++i) 
  {
    // Read current entry
    const DirectedSection* xEntry;
    m_xSubAdjacencyList.Get(i, xEntry);

    // Write high
    int iSectionTid = ((DirectedSection*)xEntry)->getSectionTid();
    in_xValueRecord.Write(&iSectionTid, 
                          sizeof(int), 
                          inout_iOffset);
    inout_iOffset += sizeof(int);

    // Write low
    bool bUpDownFlag = ((DirectedSection*)xEntry)->getUpDownFlag();
    in_xValueRecord.Write(&bUpDownFlag, 
                          sizeof(bool), 
                          inout_iOffset);
    inout_iOffset += sizeof(bool);    
  }
}

void Network::OpenAdjacencyList(SmiRecord& in_xValueRecord, 
                         size_t& inout_iOffset)
{
  // Read length from record
  int iSize;
  in_xValueRecord.Read( &iSize, 
                        sizeof( int ), 
                        inout_iOffset );
  inout_iOffset += sizeof( int );
  
  for (int i = 0; i < iSize; ++i) 
  {
  // Read high
  int iHigh;
  in_xValueRecord.Read( &iHigh, 
                        sizeof( int ), 
                        inout_iOffset );
  inout_iOffset += sizeof( int );

  // Read low
  int iLow;
  in_xValueRecord.Read( &iLow, 
                        sizeof( int ), 
                        inout_iOffset );
  inout_iOffset += sizeof( int );

    m_xAdjacencyList.Append(AdjacencyListEntry(iLow, iHigh));  
  }
}

void Network::OpenSubAdjacencyList(SmiRecord& in_xValueRecord, 
                            size_t& inout_iOffset)
{
  // Read length from record
  int iSize;
  in_xValueRecord.Read( &iSize, 
                        sizeof( int ), 
                        inout_iOffset );
  inout_iOffset += sizeof( int );
  
  for (int i = 0; i < iSize; ++i) 
  {
  // Read SectionTid
  int iSectionTid;
  in_xValueRecord.Read( &iSectionTid, 
                        sizeof( int ), 
                        inout_iOffset );
  inout_iOffset += sizeof( int );

  // Read UpDownFlag
  bool bUpDownFlag;
  in_xValueRecord.Read( &bUpDownFlag, 
                        sizeof( bool ), 
                        inout_iOffset );
  inout_iOffset += sizeof( bool );

    m_xSubAdjacencyList.Append(DirectedSection(iSectionTid, 
                                               bUpDownFlag));
  }
}


