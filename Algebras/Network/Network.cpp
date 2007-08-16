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

//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of GLine in Module Network Algebra

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

[TOC]

1 Overview


This file contains the implementation of ~gline~


2 Defines, includes, and constants

*/
#include <sstream>

#include "TupleIdentifier.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "GPoint.h"

#include "Network.h"

#include "SpatialAlgebra.h"
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
      "(meas2 int) (cc int))))";
string Network::junctionsInternalTypeInfo =
      "(rel (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int) (pos point) (r1rc tid) (r2rc tid) "
      "(sauprc tid) (sadownrc tid)(sbuprc tid) (sbdownrc tid))))";
string Network::junctionsBTreeTypeInfo =
      "(btree (tuple ((r1id int) (meas1 real) (r2id int) "
      "(meas2 real) (cc int) (pos point) (r1rc tid) (r2rc tid) "
      "(sauprc tid) (sadownrc tid)(sbuprc tid) (sbdownrc tid))) int)";
string Network::sectionsTypeInfo =
      "(rel (tuple ((rid int) (meas1 real) (meas2 real) "
      "(dual bool) (curve line))))";
string Network::sectionsInternalTypeInfo =
      "(rel (tuple ((rid int) (meas1 real) (meas2 real) "
      "(dual bool) (curve line) (rrc int))))";


                                     

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

Network::Network(int in_iId,
                 Relation* routes, 
                 Relation* junctions, 
                 Relation* sections, 
                 BTree* in_pBTreeRoutes,
                 BTree* in_pBTreeJunctionsByRoute1,
                 BTree* in_pBTreeJunctionsByRoute2):
m_iId(in_iId),                 
m_bDefined(true),
m_pRoutes(routes),
m_pJunctions(junctions),
m_pSections(sections),
m_pBTreeRoutes(in_pBTreeRoutes),
m_pBTreeJunctionsByRoute1(in_pBTreeJunctionsByRoute1),
m_pBTreeJunctionsByRoute2(in_pBTreeJunctionsByRoute2),
m_xAdjacencyList(0),
m_xSubAdjacencyList(0)
{
  FillAdjacencyLists();
}


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
  ListExpr xRoutesTypeInfo = 
      SecondoSystem::GetCatalog()->NumericType(GetRoutesTypeInfo());
  ListExpr xJunctionsTypeInfo = 
      SecondoSystem::GetCatalog()->NumericType(GetJunctionsTypeInfo());

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
  Relation* pRoutes = new Relation(xRoutesTypeInfo, true);
  Relation* pJunctions = new Relation(xJunctionsTypeInfo, true);

  // Iterate over all routes
  while(!nl->IsEmpty(xRouteList))
  {
    ListExpr xCurrentRoute = nl->First(xRouteList);
    xRouteList = nl->Rest(xRouteList);

    // Create tuple for internal table
    Tuple* pNewRoute = new Tuple(nl->Second(xRoutesTypeInfo));

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
      inout_xErrorInfo = 
          nl->Append(inout_xErrorInfo, nl->StringAtom(strErrorMessage));
      inout_bCorrect = false;
      return;
    }
    
    // Read attributes from list
    // Read values from table
    int iRouteId = nl->IntValue(nl->First(xCurrentRoute));
    float fLength  = nl->RealValue(nl->Second(xCurrentRoute));
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
    pNewRoute->PutAttribute(ROUTE_LENGTH, new CcReal(true, fLength));
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
    Tuple* pNewJunction = new Tuple(nl->Second(xJunctionsTypeInfo));
  
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
      inout_xErrorInfo = 
          nl->Append(inout_xErrorInfo, nl->StringAtom(strErrorMessage));
      inout_bCorrect = false;
      return;
    }
    
    // Read attributes from list
    int iRoute1Id = nl->IntValue(nl->First(xCurrentJunction));
    float fMeas1 = nl->RealValue(nl->Second(xCurrentJunction));
    int iRoute2Id = nl->IntValue(nl->Third(xCurrentJunction));
    float fMeas2 = nl->RealValue(nl->Fourth(xCurrentJunction));
    int iConnectivityCode= nl->IntValue(nl->Fifth(xCurrentJunction));
    // The location of the junction "Point" is calculated in the load-method
  
    // Set all necessary attributes
    pNewJunction->PutAttribute(JUNCTION_ROUTE1_ID, 
                                new CcInt(true, iRoute1Id));
    pNewJunction->PutAttribute(JUNCTION_ROUTE1_MEAS, 
                                new CcReal(true, fMeas1));
    pNewJunction->PutAttribute(JUNCTION_ROUTE2_ID, 
                                new CcInt(true, iRoute2Id));
    pNewJunction->PutAttribute(JUNCTION_ROUTE2_MEAS, 
                                new CcReal(true, fMeas2));
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

Network::~Network()
{
  delete m_pRoutes;
  delete m_pJunctions;
  delete m_pSections;
  delete m_pBTreeRoutes;
  delete m_pBTreeJunctionsByRoute1;
  delete m_pBTreeJunctionsByRoute2;
}

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

void Network::FillRoutes(const Relation *routes)
{
  ostringstream xRoutesPtrStream;
  xRoutesPtrStream << (long)routes;

  string strQuery = "(consume (sort (feed (" + routesTypeInfo + 
                       " (ptr " + xRoutesPtrStream.str() + ")))))";

  Word xResult;
  assert(QueryProcessor::ExecuteQuery(strQuery, xResult));
  m_pRoutes = (Relation *)xResult.addr;

  // Create B-Tree for the routes
  ostringstream xThisRoutesPtrStream;
  xThisRoutesPtrStream << (long)m_pRoutes;
  strQuery = "(createbtree (" + routesTypeInfo + 
                " (ptr " + xThisRoutesPtrStream.str() + "))" + " id)";

  assert(QueryProcessor::ExecuteQuery(strQuery, xResult));
  m_pBTreeRoutes = (BTree*)xResult.addr;
}

Relation *Network::GetRoutes()
{
  return m_pRoutes->Clone();
}

Relation *Network::GetRoutesInternal()
{
  return m_pRoutes;
}

void Network::FillJunctions(const Relation *in_pJunctions)
{
  // Get type-info for internal table
  ListExpr xIntTypeInfo = 
      SecondoSystem::GetCatalog()->NumericType(GetJunctionsIntTypeInfo());

  // Create new relation. Each junction will be added twice to
  // this relation. Afterwards the relation will be sorted.
  Relation *pIntJunctions = new Relation(xIntTypeInfo, true);

  // Iterator for the input-table with junctions
  GenericRelationIterator* pJunctionsIter = in_pJunctions->MakeScan();
  Tuple* pCurrentJunction;

  while((pCurrentJunction = pJunctionsIter->GetNextTuple()) != 0)
  {
    // Create tuple for internal table
    Tuple* pNewJunction = new Tuple(nl->Second(xIntTypeInfo));
    for(int i = 0; i < pCurrentJunction->GetNoAttributes(); i++)
    {
      // Copy all attributes from the input-relation
      pNewJunction->CopyAttribute(i, pCurrentJunction, i);
    }
    

    // Store Pointer to the first route in the new relation.
    CcInt* pR1Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE1_ID);
    BTreeIterator* pRoutesIter = m_pBTreeRoutes->ExactMatch(pR1Id);
    // TODO: Fehlerbehandlung verbessern
    assert(pRoutesIter->Next());
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
    assert(pRoutesIter->Next());
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

    // Append new junction
    pIntJunctions->AppendTuple(pNewJunction);

    pCurrentJunction->DeleteIfAllowed();
    pNewJunction->DeleteIfAllowed();
  }
  delete pJunctionsIter;

  // Sort the table which is now containing all junctions twice  
  ostringstream xJunctionsStream;
  xJunctionsStream << (long)pIntJunctions;
  string strQuery = "(consume (sortby (feed (" + junctionsInternalTypeInfo +
                    " (ptr " + xJunctionsStream.str() + 
                    "))) ((r1id asc)(meas1 asc))))";


  Word xResult;
  assert(QueryProcessor::ExecuteQuery(strQuery, xResult));
  m_pJunctions = (Relation *)xResult.addr;
  
  // Delete internal table
  pIntJunctions->Delete();
  
  // Create two b-trees for the junctions sorted by first and second id
  ostringstream xThisJunctionsPtrStream;
  xThisJunctionsPtrStream << (long)m_pJunctions;
  strQuery = "(createbtree (" + junctionsInternalTypeInfo + 
                " (ptr " + xThisJunctionsPtrStream.str() + "))" + " r1id)";
  assert(QueryProcessor::ExecuteQuery(strQuery, xResult));
  m_pBTreeJunctionsByRoute1 = (BTree*)xResult.addr;
  
  ostringstream xThisJunctionsPtrStream2;
  xThisJunctionsPtrStream2 << (long)m_pJunctions;
  strQuery = "(createbtree (" + junctionsInternalTypeInfo + 
                " (ptr " + xThisJunctionsPtrStream2.str() + "))" + " r2id)";
  assert(QueryProcessor::ExecuteQuery(strQuery, xResult));
  m_pBTreeJunctionsByRoute2 = (BTree*)xResult.addr;
}

Relation *Network::GetJunctionsInternal()
{
  return m_pJunctions;
}

Relation *Network::GetJunctions()
{
  ostringstream strJunctionsPtr;
  strJunctionsPtr << (long)m_pJunctions;

  string querystring = "(consume (feed (" + junctionsInternalTypeInfo +
                       " (ptr " + strJunctionsPtr.str() + "))))";

  Word resultWord;
  assert(QueryProcessor::ExecuteQuery(querystring, resultWord));
  return (Relation *)resultWord.addr;
}

void Network::FillSections()
{
  // The method will iterate over routes
  GenericRelationIterator* pRoutesIt = m_pRoutes->MakeScan();

  // Create relation for sections
  ListExpr xSectionsNumInt = 
    SecondoSystem::GetCatalog()->NumericType(GetSectionsInternalTypeInfo());
  m_pSections = new Relation(xSectionsNumInt);

  // Iterate over all Routes
  Tuple* pRoute;
  while((pRoute = pRoutesIt->GetNextTuple()) != 0)
  {
    // Current position on route - starting at the beginning of the route (0)
    float fCurrentPosOnRoute = 0;
    Line* pRouteCurve = (Line*)pRoute->GetAttribute(ROUTE_CURVE);
    int iTupleId = pRoute->GetTupleId();
    CcInt* xRouteId = (CcInt*)pRoute->GetAttribute(ROUTE_ID); 
    int iRouteId = xRouteId->GetIntval();
    bool bDual = ((CcBool*)pRoute->GetAttribute(ROUTE_DUAL))->GetBoolval();
    
    // Now we need to find all junctions belonging to this route
    //
    // The junctions can have this route as their first as well as
    // their second parameter. Thus we have to look up via both 
    // b-trees.
    //
    vector<JunctionSortEntry> xJunctions;
    GetJunctionsOnRoute(xRouteId,
                        xJunctions);

    Tuple* pLastJunction = 0;
    JunctionSortEntry xLastEntry;

    // Now that we found all relevant junctions we can iterate over them.
    for(size_t i = 0; i < xJunctions.size(); i++)
    {
      // Get next junction
      JunctionSortEntry xCurrentEntry = xJunctions[i];
      Tuple* pJunction = xCurrentEntry.m_pJunction;
    
      // Find values for the new section      
      float fStartPos = fCurrentPosOnRoute;
      float fEndPos = xCurrentEntry.getRouteMeas(); 

      // If the first junction is at the very start of the route, no 
      // section will be added
      if(xCurrentEntry.getRouteMeas() > fCurrentPosOnRoute)
      {

        // Calculate line
        Line* pLine = new Line(0);
        bool bStartSmaller = ((CcBool*)pRoute->GetAttribute(
                                  ROUTE_STARTSSMALLER))->GetBoolval();
        pRouteCurve->SubLine(fStartPos,
                              fEndPos,
                              bStartSmaller,
                              *pLine);
      
        // Create a new Section
        Tuple* pNewSection = new Tuple(nl->Second(xSectionsNumInt));
        pNewSection->PutAttribute(SECTION_RID, new CcInt(true, iRouteId));
        pNewSection->PutAttribute(SECTION_DUAL, new CcBool(true, bDual));
        pNewSection->PutAttribute(SECTION_MEAS1, new CcReal(true, fStartPos));
        pNewSection->PutAttribute(SECTION_MEAS2, new CcReal(true, fEndPos));
        pNewSection->PutAttribute(SECTION_RRC, new CcInt(true, iTupleId));
        pNewSection->PutAttribute(SECTION_CURVE, pLine);
        m_pSections->AppendTuple(pNewSection);
        
        // Store ID of new section in Junction
        vector<int> xIndices;
        if(xCurrentEntry.m_bFirstRoute)
        {
          xIndices.push_back(JUNCTION_SECTION_ADOWN_RC);
        }
        else
        {
          xIndices.push_back(JUNCTION_SECTION_BDOWN_RC);
        } 
        vector<Attribute*> xAttrs;
        xAttrs.push_back(new TupleIdentifier(true, pNewSection->GetTupleId()));
        m_pJunctions->UpdateTuple(pJunction, xIndices, xAttrs);
        if(pLastJunction != 0)
        {
          vector<int> xIndicesLast;
          if(xLastEntry.m_bFirstRoute)
          {
            xIndicesLast.push_back(JUNCTION_SECTION_AUP_RC);
          }
          else
          {
            xIndicesLast.push_back(JUNCTION_SECTION_BUP_RC);
          } 
          vector<Attribute*> xAttrsLast;
          xAttrsLast.push_back(new TupleIdentifier(true, 
                                                   pNewSection->GetTupleId()));
          m_pJunctions->UpdateTuple(pLastJunction, xIndicesLast, xAttrsLast);
        }  
        pNewSection->DeleteIfAllowed();
      }
      // Update for next loop
      fCurrentPosOnRoute = fEndPos;
      pLastJunction = pJunction;
      xLastEntry = xCurrentEntry;
    } // End While-Junctions

    // One part is missing, 
    //if the last junction was not at the end of the route
    if(fCurrentPosOnRoute < pRouteCurve->Length())
    {
      // Find values for the new section      
      int iRouteId = ((CcInt*)pRoute->GetAttribute(ROUTE_ID))->GetIntval();
      bool bDual = ((CcBool*)pRoute->GetAttribute(ROUTE_DUAL))->GetBoolval();
      float fStartPos = fCurrentPosOnRoute;
      float fEndPos = pRouteCurve->Length();
      int iTupleId = pRoute->GetTupleId();

      // Calculate line
      Line* pLine = new Line(0);
      bool bStartSmaller = ((CcBool*)pRoute->GetAttribute(
                                ROUTE_STARTSSMALLER))->GetBoolval();
      pRouteCurve->SubLine(fStartPos,
                            fEndPos,
                            bStartSmaller,
                            *pLine);
      
      // Create a new Section
      Tuple* pNewSection = new Tuple(nl->Second(xSectionsNumInt));
      pNewSection->PutAttribute(SECTION_RID, new CcInt(true, iRouteId));
      pNewSection->PutAttribute(SECTION_DUAL, new CcBool(true, bDual));
      pNewSection->PutAttribute(SECTION_MEAS1, new CcReal(true, fStartPos));
      pNewSection->PutAttribute(SECTION_MEAS2, new CcReal(true, fEndPos));
      pNewSection->PutAttribute(SECTION_RRC, new CcInt(true, iTupleId));
      pNewSection->PutAttribute(SECTION_CURVE, pLine);
      m_pSections->AppendTuple(pNewSection);

      // Store ID of new section in Junction
      if(pLastJunction != 0)
      {
        vector<int> xIndicesLast;
        if(xLastEntry.m_bFirstRoute)
        {
          xIndicesLast.push_back(JUNCTION_SECTION_AUP_RC);
        }
        else
        {
          xIndicesLast.push_back(JUNCTION_SECTION_BUP_RC);
        } 
        vector<Attribute*> xAttrsLast;
        xAttrsLast.push_back(new TupleIdentifier(true, 
                                                  pNewSection->GetTupleId()));
        m_pJunctions->UpdateTuple(pLastJunction, xIndicesLast, xAttrsLast);
      }       

      pNewSection->DeleteIfAllowed();
    } // end if

    pRoute->DeleteIfAllowed();
  } // End while Routes
  delete pRoutesIt;


//  GenericRelationIterator *pSectionsIter = m_pSections->MakeScan();
//  Tuple *pCurrentSection;
//
//  while((pCurrentSection = pSectionsIter->GetNextTuple()) != 0)
//  {
//    CcInt* xRouteId = (CcInt*)pCurrentSection->GetAttribute(SECTION_RID);
//    int iRouteId = xRouteId->GetIntval();
//  CcReal* xStartPos = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS1);
//    float fStartPos = xStartPos->GetRealval();
//    CcReal* xEndPos = (CcReal*)pCurrentSection->GetAttribute(SECTION_MEAS2);
//    float fEndPos = xEndPos->GetRealval();
//
//    cout << "Route:" << iRouteId << " "
//         << "Section:" << pCurrentSection->GetTupleId() << " "
//         << "Start:" << fStartPos << " "
//         << "End:" << fEndPos 
//         << endl;
//  }
//  delete pSectionsIter;
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

Tuple* Network::GetSectionIdOnRoute(GPoint* in_xGPoint)
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
  
  Tuple* pSection = m_pSections->GetTuple(iSectionId);
  
  
  // Return the section
  return pSection;
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
  assert(QueryProcessor::ExecuteQuery(querystring, resultWord));
  return (Relation *)resultWord.addr;
}

/*
4 Static Functions supporting the type-constructor

4.1 ~In~-function

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

  while((pCurrentJunction = pJunctionsIt->GetNextTuple()) != 0)
  {
    // Retrieve the connectivity code
    int iCc = 
       ((CcInt*)pCurrentJunction->GetAttribute(JUNCTION_CC))->GetIntval();
    ConnectivityCode xCc(iCc);
       
    // Retrieve the four sections, possibly less than four
    TupleIdentifier* pTid;
    Tuple* pSectionAUp = 0;
    Tuple* pSectionADown = 0;
    Tuple* pSectionBUp = 0;
    Tuple* pSectionBDown = 0;
     
    pTid = 
    (TupleIdentifier*)pCurrentJunction->GetAttribute(JUNCTION_SECTION_AUP_RC); 
    if(pTid->GetTid() > 0)
    {
      pSectionAUp = m_pSections->GetTuple(pTid->GetTid());
    }  
    pTid = 
   (TupleIdentifier*)pCurrentJunction->GetAttribute(JUNCTION_SECTION_ADOWN_RC);
    if(pTid->GetTid() > 0)
    {
      pSectionADown = m_pSections->GetTuple(pTid->GetTid());
    }  
    pTid =
    (TupleIdentifier*)pCurrentJunction->GetAttribute(JUNCTION_SECTION_BUP_RC); 
    if(pTid->GetTid() > 0)
    {
      pSectionBUp = m_pSections->GetTuple(pTid->GetTid());
    }  
    pTid = 
  (TupleIdentifier*)pCurrentJunction->GetAttribute(JUNCTION_SECTION_BDOWN_RC); 
    if(pTid->GetTid() > 0)
    {
      pSectionBDown = m_pSections->GetTuple(pTid->GetTid());
    }  
        
    // If a section is existing and the transition is possible
    // it will be added to the list.
    //
    // First section
    // 
    int iLow = m_xSubAdjacencyList.Size();
    if(pSectionAUp != 0 && pSectionAUp != 0 && xCc.IsPossible(xCc.AUP_AUP))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionAUp->GetTupleId(), 
                                                   true));  
    }
    if(pSectionAUp != 0 && pSectionADown != 0 && xCc.IsPossible(xCc.AUP_ADOWN))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionADown->GetTupleId(), 
                                                   false));  
    }
    if(pSectionAUp != 0 && pSectionBUp != 0 && xCc.IsPossible(xCc.AUP_BUP))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionBUp->GetTupleId(), 
                                                   true));  
    }
    if(pSectionAUp != 0 && pSectionBDown != 0 && xCc.IsPossible(xCc.AUP_BDOWN))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionBDown->GetTupleId(), 
                                                   false));  
    }
    // Mark the part of the sub-adjacency-list in the adjacency-list 
    int iHigh = m_xSubAdjacencyList.Size()-1;
    if(iHigh >= iLow)
    {
      m_xAdjacencyList.Put((2 * (pSectionAUp->GetTupleId() - 1) + 1),
                            AdjacencyListEntry(iLow, iHigh));
    }

    //
    // Second section
    // 
    iLow = m_xSubAdjacencyList.Size();
    if(pSectionADown != 0 && pSectionAUp != 0 && xCc.IsPossible(xCc.ADOWN_AUP))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionAUp->GetTupleId(), 
                                                   true));  
    }
    if(pSectionADown != 0 && 
    pSectionADown != 0 && xCc.IsPossible(xCc.ADOWN_ADOWN))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionADown->GetTupleId(), 
                                                   false));  
    }
    if(pSectionADown != 0 && pSectionBUp != 0 && xCc.IsPossible(xCc.ADOWN_BUP))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionBUp->GetTupleId(), 
                                                   true));  
    }
    if(pSectionADown != 0 
    && pSectionBDown != 0 && xCc.IsPossible(xCc.ADOWN_BDOWN))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionBDown->GetTupleId(), 
                                                   false));  
    }
    // Mark the part of the sub-adjacency-list in the adjacency-list 
    iHigh = m_xSubAdjacencyList.Size()-1;
    if(iHigh >= iLow)
    {
      m_xAdjacencyList.Put((2 * (pSectionADown->GetTupleId() - 1)),
                            AdjacencyListEntry(iLow, iHigh));
    }

    //
    // Third section
    // 
    iLow = m_xSubAdjacencyList.Size();
    if(pSectionBUp != 0 && pSectionAUp != 0 && xCc.IsPossible(xCc.BUP_AUP))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionAUp->GetTupleId(), 
                                                   true));  
    }
    if(pSectionBUp != 0 && pSectionADown != 0 && xCc.IsPossible(xCc.BUP_ADOWN))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionADown->GetTupleId(), 
                                                   false));  
    }
    if(pSectionBUp != 0 && pSectionBUp != 0 && xCc.IsPossible(xCc.BUP_BUP))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionBUp->GetTupleId(), 
                                                   true));  
    }
    if(pSectionBUp != 0 && pSectionBDown != 0 && xCc.IsPossible(xCc.BUP_BDOWN))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionBDown->GetTupleId(), 
                                                   false));  
    }
    // Mark the part of the sub-adjacency-list in the adjacency-list 
    iHigh = m_xSubAdjacencyList.Size()-1;
    if(iHigh >= iLow)
    {
      m_xAdjacencyList.Put((2 * (pSectionBUp->GetTupleId() - 1) + 1),
                            AdjacencyListEntry(iLow, iHigh));
    }

    //
    // Fourth section
    // 
    iLow = m_xSubAdjacencyList.Size();
    if(pSectionBDown != 0 && pSectionAUp != 0 && xCc.IsPossible(xCc.BDOWN_AUP))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionAUp->GetTupleId(), 
                                                   true));  
    }
    if(pSectionBDown != 0 
    && pSectionADown != 0 && xCc.IsPossible(xCc.BDOWN_ADOWN))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionADown->GetTupleId(), 
                                                   false));  
    }
    if(pSectionBDown != 0 && pSectionBUp != 0 && xCc.IsPossible(xCc.BDOWN_BUP))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionBUp->GetTupleId(), 
                                                   true));  
    }
    if(pSectionBDown != 0 
    && pSectionBDown != 0 && xCc.IsPossible(xCc.BDOWN_BDOWN))
    {
      m_xSubAdjacencyList.Append(DirectedSection(pSectionBDown->GetTupleId(), 
                                                   false));  
    }
    // Mark the part of the sub-adjacency-list in the adjacency-list 
    iHigh = m_xSubAdjacencyList.Size()-1;
    if(iHigh >= iLow)
    {
      m_xAdjacencyList.Put((2 * (pSectionBDown->GetTupleId() - 1)),
                            AdjacencyListEntry(iLow, iHigh));
    }
        
    pCurrentJunction->DeleteIfAllowed();
  }
  delete pJunctionsIt;

//  cout << "AdjacencyList" << endl;
//  cout << "-------------" << endl;
//  for (int i = 0; i < m_xAdjacencyList.Size(); ++i) 
//  {
//    const AdjacencyListEntry* xEntry;
//    m_xAdjacencyList.Get(i, xEntry);
//    cout << i << ": " 
//         << "High:" << xEntry->m_iHigh << " " 
//         << "Low:" << xEntry->m_iLow 
//         << endl;
//    
//  }
//  cout << "-------------" << endl;
//
//  cout << "SubAdjacencyList" << endl;
//  cout << "----------------" << endl;
//  for (int i = 0; i < m_xSubAdjacencyList.Size(); ++i) 
//  {
//    const DirectedSection* xEntry;
//    m_xSubAdjacencyList.Get(i, xEntry);
//
//    bool bUpDownFlag = ((DirectedSection*)xEntry)->getUpDownFlag();
//    int iSectionTid = ((DirectedSection*)xEntry)->getSectionTid();
//    cout << i << ": " 
//         << "Tid:" << iSectionTid << " "
//         << "UpDownFlag:" << bUpDownFlag 
//         << endl;
//         
//  }
//  cout << "----------------" << endl;
}

void Network::GetAdjacentSections(int in_iSectionId,
                                  bool in_bUpDown,
                                  vector<DirectedSection> &inout_xSections)
{
  int iIndex = (2 * (in_iSectionId - 1)) + (in_bUpDown ? 0 : 1);
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
4 Static Functions supporting the type-constructor

4.1 ~In~-function

*/
ListExpr Network::GetRoutesTypeInfo()
{
  ListExpr result;

  if(nl->ReadFromString(routesTypeInfo, result))
    return result;

  return 0; 
}

/*
4 Static Functions supporting the type-constructor

4.1 ~In~-function

*/
ListExpr Network::GetRoutesBTreeTypeInfo()
{
  ListExpr result;

  if(nl->ReadFromString(routesBTreeTypeInfo, result))
    return result;

  return 0;
}


/*
4 Static Functions supporting the type-constructor

4.1 ~In~-function

*/
ListExpr Network::GetJunctionsBTreeTypeInfo()
{
  ListExpr result;

  if(nl->ReadFromString(junctionsBTreeTypeInfo, result))
    return result;

  return 0;
}


/*
4 Static Functions supporting the type-constructor

4.1 ~In~-function

*/
ListExpr Network::GetJunctionsTypeInfo()
{
  ListExpr result;

  if(nl->ReadFromString(junctionsTypeInfo, result))
    return result;

  return 0;
}


/*
4 Static Functions supporting the type-constructor

4.1 ~In~-function

*/
ListExpr Network::GetJunctionsIntTypeInfo()
{
  ListExpr result;

  if(nl->ReadFromString(junctionsInternalTypeInfo, result))
    return result;

  return 0;
}


/*
4 Static Functions supporting the type-constructor

4.1 ~In~-function

*/
ListExpr Network::GetSectionsTypeInfo()
{
  ListExpr result;

  if(nl->ReadFromString(sectionsTypeInfo, result))
    return result;

  return 0;
}

ListExpr Network::GetSectionsInternalTypeInfo()
{
  ListExpr result;

  if(nl->ReadFromString(sectionsInternalTypeInfo, result))
    return result;

  return 0;
}



/*
3.3 ~Out~-function of type constructor ~network~

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
    float fLength  = pLength->GetRealval();
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
                             nl->RealAtom(fLength),
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
    float fMeas1 = pMeas1->GetRealval();
    CcInt* pRoute2Id;
    pRoute2Id = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE2_ID);
    int iRoute2Id = pRoute2Id->GetIntval();
    CcReal* pMeas2;
    pMeas2 = (CcReal*)pCurrentJunction->GetAttribute(JUNCTION_ROUTE2_MEAS); 
    float fMeas2 = pMeas2->GetRealval();
    CcInt* pConnectivityCode;
    pConnectivityCode = (CcInt*)pCurrentJunction->GetAttribute(JUNCTION_CC);
    int iConnectivityCode= pConnectivityCode->GetIntval();
    Point* pPoint = (Point*)pCurrentJunction->GetAttribute(JUNCTION_POS);
    ListExpr xPoint = OutPoint(nl->TheEmptyList(), SetWord(pPoint));
    
    // Build list
    xNext = nl->SixElemList(nl->IntAtom(iRoute1Id),
                            nl->RealAtom(fMeas1),
                            nl->IntAtom(iRoute2Id),
                            nl->RealAtom(fMeas2),
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
3.11 ~Save~-function of type constructor ~network~

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
  cout << "Written: " << iId << endl;
  
    // Read network id
  int iReadId = 0;
  in_xValueRecord.Read( &iReadId, sizeof( int ), 0 );
  cout << "Read: " << iReadId << endl;
  
  
  // Save routes
  ListExpr xType = GetRoutesTypeInfo();
  ListExpr xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pRoutes->Save(in_xValueRecord, 
                      inout_iOffset, 
                      xNumericType))
  {                      
    return false;
  }

  // Save junctions
  xType = GetJunctionsIntTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pJunctions->Save(in_xValueRecord, 
                         inout_iOffset, 
                         xNumericType))
  {
    return false;
  }
  // Save sections
  xType = GetSectionsInternalTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pSections->Save(in_xValueRecord, 
                        inout_iOffset, 
                        xNumericType))
  {
    return false;
  }

  int iReadId2 = 0;
  in_xValueRecord.Read( &iReadId2, sizeof( int ), 0 );
  cout << "Read2: " << iReadId2 << endl;

  // Save btree for routes
  xType = GetRoutesBTreeTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeRoutes->Save(in_xValueRecord, 
                           inout_iOffset,
                           xNumericType))
  {
    return false;
  }
  
  int iReadId3 = 0;
  in_xValueRecord.Read( &iReadId3, sizeof( int ), 0 );
  cout << "Read3: " << iReadId3 << endl;


  // Save first btree for junctions
  xType = GetJunctionsBTreeTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeJunctionsByRoute1->Save(in_xValueRecord, 
                                      inout_iOffset,
                                      xNumericType))
  {
    return false;
  }
  
    int iReadId4 = 0;
  in_xValueRecord.Read( &iReadId4, sizeof( int ), 0 );
  cout << "Read4: " << iReadId4 << endl;

  
    
  // Save second btree for junctions
  xType = GetJunctionsBTreeTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  if(!m_pBTreeJunctionsByRoute2->Save(in_xValueRecord, 
                                      inout_iOffset,
                                      xNumericType))
  {                                                  
    return false;
  }

//  Attribute::Save(valueRecord, 
//                   offset, 
//                   typeInfo, // AlgebraId und TypId
//                   m_xAdjacencyList);
//
//  Attribute::Save(valueRecord, 
//                   offset, 
//                   typeInfo, 
//                   m_xSubAdjacencyList);
    
    // Read network id

  return true; 
}


/*
3.12 ~Open~-function of type constructor ~network~

*/
Network *Network::Open(SmiRecord& in_xValueRecord, 
                        size_t& inout_iOffset, 
                        const ListExpr in_xTypeInfo)
{
  int iId = 0;
  Relation* pRoutes = 0;
  Relation* pJunctions = 0;  
  Relation* pSections = 0;
  BTree* pBTreeRoutes = 0;
  BTree* pBTreeJunctionsByRoute1 = 0;
  BTree* pBTreeJunctionsByRoute2 = 0;

    // Read network id
  int iReadId = 0;
  in_xValueRecord.Read( &iReadId, sizeof( int ), 0 );
  cout << "Read: " << iReadId << endl;

  // Read network id
  in_xValueRecord.Read( &iId, sizeof( int ), inout_iOffset );
  inout_iOffset += sizeof( int );


  // Open routes
  ListExpr xType = GetRoutesTypeInfo();
  ListExpr xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  pRoutes = Relation::Open(in_xValueRecord, 
                           inout_iOffset, 
                           xNumericType);
  if(!pRoutes)
  { 
    return 0;
  }

  // Open junctions
  xType = GetJunctionsIntTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  pJunctions = Relation::Open(in_xValueRecord, 
                              inout_iOffset, 
                              xNumericType);
  if(!pJunctions) 
  {  
    pRoutes->Delete(); 
    return 0;
  }

  // Open sections  
  xType = GetSectionsInternalTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  pSections = Relation::Open(in_xValueRecord, 
                             inout_iOffset, 
                             xNumericType);
  if(!pSections) 
  {
    pRoutes->Delete(); 
    pJunctions->Delete(); 
    return 0;
  }

  // Open btree for routes
  xType = GetRoutesBTreeTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  pBTreeRoutes = BTree::Open(in_xValueRecord, 
                             inout_iOffset, 
                             xNumericType);
         
  if(!pBTreeRoutes) 
  {
    pRoutes->Delete(); 
    pJunctions->Delete(); 
    pSections->Delete();
    return 0;
  }

  // Open first btree for junctions  
  xType = GetJunctionsBTreeTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  pBTreeJunctionsByRoute1 = BTree::Open(in_xValueRecord, 
                                        inout_iOffset, 
                                        xNumericType);
  if(!pBTreeJunctionsByRoute1) 
  {
    pRoutes->Delete(); 
    pJunctions->Delete(); 
    pSections->Delete();
    delete pBTreeRoutes;
    return 0;
  }

  // Open second btree for junctions
  xType = GetJunctionsBTreeTypeInfo();
  xNumericType =SecondoSystem::GetCatalog()->NumericType(xType);
  pBTreeJunctionsByRoute2 = BTree::Open(in_xValueRecord, 
                                        inout_iOffset, 
                                        xNumericType); 
  if(!pBTreeJunctionsByRoute2) 
  {
    pRoutes->Delete(); 
    pJunctions->Delete(); 
    pSections->Delete();
    delete pBTreeRoutes;
    delete pBTreeJunctionsByRoute1;
    return 0;
  }
  
  // Create network
  return new Network(iId,
                     pRoutes, 
                     pJunctions, 
                     pSections, 
                     pBTreeRoutes,
                     pBTreeJunctionsByRoute1,
                     pBTreeJunctionsByRoute2);
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
3.4 ~In~-function of type constructor ~network~

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
3.5 ~Create~-function of type constructor ~network~

*/
Word Network::CreateNetwork(const ListExpr typeInfo)
{
  return SetWord(new Network());
}

/*
3.6 ~Close~-function of type constructor ~network~

*/
void Network::CloseNetwork(const ListExpr typeInfo, Word& w)
{
  delete (Network*)w.addr;
}

/*
3.7 ~Clone~-function of type constructor ~network~

Not implemented yet.

*/
Word Network::CloneNetwork(const ListExpr typeInfo, const Word& w)
{
  return SetWord(Address(0));
}

/*
3.8 ~Delete~-function of type constructor ~network~

*/
void Network::DeleteNetwork(const ListExpr typeInfo, Word& w)
{
  Network* n = (Network*)w.addr;
  n->Destroy();
  delete n;
}

/*
3.9 ~Check~-function of type constructor ~network~

*/
bool Network::CheckNetwork(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual(type, "network"));
}

/*
3.10 ~Cast~-function of type constructor ~network~

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
3.13 ~SizeOf~-function of type constructor ~network~

*/
int Network::SizeOfNetwork()
{
  return 0;
}

