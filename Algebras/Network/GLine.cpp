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

1.1 Implementation of GLine

March 2004 Victor Almeida

Mai-Oktober 2007 Martin Scheppokat

December 2007 Simone Jandt (GLine::In repaired, GetLength())

March 2008 Simone Jandt added distance operator.

Defines, includes, and constants

*/


#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "StandardTypes.h"
#include "TupleIdentifier.h"
#include "GPoint.h"
#include "GLine.h"
#include "RelationAlgebra.h"
#include "BTreeAlgebra.h"
#include "DBArray.h"
#include "SpatialAlgebra.h"
#include "Network.h"
#include "NetworkManager.h"
#include "OpShortestPath.h"


/*
Functions

*/
/*
The simple constructor. Should not be used.

*/
  GLine::GLine()
  {
  }

/*
Empty constructor

*/
  GLine::GLine(int in_iSize):
    m_xRouteIntervals(in_iSize)
  {
  }

/*
Construktor used internally

*/
  GLine::GLine(GLine* in_xOther):
  m_xRouteIntervals(0)
  {
    m_bDefined = in_xOther->m_bDefined;
    m_iNetworkId = in_xOther->m_iNetworkId;
    // Iterate over all RouteIntervalls
    for (int i = 0; i < in_xOther->m_xRouteIntervals.Size(); ++i)
    {
      // Get next Interval
      const RouteInterval* pCurrentInterval;
      in_xOther->m_xRouteIntervals.Get(i, pCurrentInterval);

      int iRouteId = pCurrentInterval->m_iRouteId;
      double dStart = pCurrentInterval->m_dStart;
      double dEnd = pCurrentInterval->m_dEnd;
      AddRouteInterval(iRouteId,
                       dStart,
                       dEnd);
    }
  }



/*
Construktor from list

*/
  GLine::GLine( ListExpr in_xValue,
                int in_iErrorPos,
                ListExpr& inout_xErrorInfo,
                bool& inout_bCorrect)
{
  // Check the list
  if(!(nl->ListLength( in_xValue ) == 2))
  {
    string strErrorMessage = "GLine(): List length must be 2.";
    inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                                  nl->StringAtom(strErrorMessage));
    inout_bCorrect = false;
    m_bDefined = false;
    return;
  }

  // Split into the two parts
  ListExpr xNetworkIdList = nl->First(in_xValue);
  ListExpr xRouteIntervalList = nl->Second(in_xValue);

  // Check the parts
  if(!nl->IsAtom(xNetworkIdList) ||
      nl->AtomType(xNetworkIdList) != IntType)
  {
    string strErrorMessage = "GLine(): Error while reading network-id.";
        inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                                      nl->StringAtom(strErrorMessage));
    m_bDefined = false;
    inout_bCorrect = false;
    return;
  }

  m_iNetworkId = nl->IntValue(xNetworkIdList);

  // Iterate over all routes
  while( !nl->IsEmpty( xRouteIntervalList) )
  {
    ListExpr xCurrentRouteInterval = nl->First( xRouteIntervalList );
    xRouteIntervalList = nl->Rest( xRouteIntervalList );

    if( nl->ListLength( xCurrentRouteInterval ) != 3 ||
      (!nl->IsAtom( nl->First(xCurrentRouteInterval))) ||
      nl->AtomType( nl->First(xCurrentRouteInterval)) != IntType ||
      (!nl->IsAtom( nl->Second(xCurrentRouteInterval))) ||
      nl->AtomType( nl->Second(xCurrentRouteInterval)) != RealType ||
      (!nl->IsAtom( nl->Third(xCurrentRouteInterval))) ||
      nl->AtomType( nl->Third(xCurrentRouteInterval)) != RealType)
    {
      string strErrorMessage = "GLine(): Error while reading route-interval.";
          inout_xErrorInfo = nl->Append(inout_xErrorInfo,
                                        nl->StringAtom(strErrorMessage));
      inout_bCorrect = false;
      m_bDefined = false;
      return;
    }

    // Read attributes from list
    // Read values from table
    int iRouteId = nl->IntValue( nl->First(xCurrentRouteInterval) );
    double dStart = nl->RealValue( nl->Second(xCurrentRouteInterval) );
    double dEnd  = nl->RealValue( nl->Third(xCurrentRouteInterval) );

    AddRouteInterval(iRouteId,
                     dStart,
                     dEnd);

  }
  inout_bCorrect = true;
  m_bDefined = true;
}

/*
Set new network id

*/
void GLine::SetNetworkId(int in_iNetworkId)
{
  m_iNetworkId = in_iNetworkId;
  m_bDefined = true;
}

/*
Add a route interval

*/
void GLine::AddRouteInterval(int in_iRouteId,
                             double in_dStart,
                             double in_dEnd)
{
  m_xRouteIntervals.Append(RouteInterval(in_iRouteId,
                                         in_dStart,
                                         in_dEnd));
}


/*
Checks whether the GLine is defined

*/
bool GLine::IsDefined() const
{
  return m_bDefined;
}

/*
Checks if the GLine is defined

*/
void GLine::SetDefined( bool in_bDefined )
{
  m_bDefined = in_bDefined;
}

/*
~In~-function

*/

Word GLine::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct){
  GLine* pGline = new GLine(0);
  if (nl->ListLength(instance) != 2) {
    correct = false;
    cmsg.inFunError("Expecting (networkid (list of routeintervals))");
    return SetWord(Address(0));
  }
  ListExpr FirstElem = nl->First(instance);
  ListExpr SecondElem = nl->Second(instance);
  if (!nl->IsAtom(FirstElem) || !nl->AtomType(FirstElem)== IntType) {
    correct = false;
    cmsg.inFunError("Networkadress is not evaluable");
    return SetWord(Address(0));
  }
  if (nl->IsEmpty(SecondElem)) {
    correct = false;
    cmsg.inFunError("List of routeintervals is empty.");
    return SetWord(Address(0));
  }
  pGline->SetNetworkId(nl->IntValue(FirstElem));
  while (!nl->IsEmpty(SecondElem)) {
    ListExpr start = nl->First(SecondElem);
    SecondElem = nl->Rest(SecondElem);
    if (nl->ListLength(start) != 3) {
      correct = false;
      cmsg.inFunError("Routeinterval incorrect.Expected list of 3 Elements.");
      return SetWord(Address(0));
    }
    ListExpr lrid = nl->First(start);
    ListExpr lpos1 = nl->Second(start);
    ListExpr lpos2 = nl->Third(start);
    if (!nl->IsAtom(lrid) || !nl->AtomType(lrid) == IntType ||
         !nl->IsAtom(lpos1) || !nl->AtomType(lpos1) == RealType ||
         !nl->IsAtom(lpos2) || !nl->AtomType(lpos2) == RealType) {
      correct = false;
      cmsg.inFunError("Routeinterval should be list int, real, real.");
      return SetWord(Address(0));
    }
    pGline->AddRouteInterval(nl->IntValue(lrid),
                             nl->RealValue(lpos1),
                             nl->RealValue(lpos2));
  }
  correct = true;
  return SetWord(pGline);
}
/*
~Out~-function

*/
ListExpr GLine::Out(ListExpr in_xTypeInfo,
                    Word in_xValue)
{
  GLine *pGline = (GLine*)in_xValue.addr;

  if(!pGline->IsDefined())
  {
    return nl->SymbolAtom( "undef" );
  }

  ListExpr xLast, xNext;
  bool bFirst = true;
  ListExpr xNetworkId = nl->IntAtom(pGline->m_iNetworkId);
  ListExpr xRouteIntervals;

  // Iterate over all RouteIntervalls
  for (int i = 0; i < pGline->m_xRouteIntervals.Size(); ++i)
  {
    // Get next Interval
    const RouteInterval* pCurrentInterval;
    pGline->m_xRouteIntervals.Get(i, pCurrentInterval);

    int iRouteId = pCurrentInterval->m_iRouteId;
    double dStart = pCurrentInterval->m_dStart;
    double dEnd = pCurrentInterval->m_dEnd;
    // Build list
    xNext = nl->ThreeElemList(nl->IntAtom(iRouteId),
                              nl->RealAtom(dStart),
                              nl->RealAtom(dEnd));

    // Create new list or append element to existing list
    if(bFirst)
    {
      xRouteIntervals = nl->OneElemList(xNext);
      xLast = xRouteIntervals;
      bFirst = false;
    }
    else
    {
      xLast = nl->Append(xLast, xNext);
    }
  }
  if(pGline->m_xRouteIntervals.Size() == 0)
  {
    xRouteIntervals = nl->TheEmptyList();
  }
  return nl->TwoElemList(xNetworkId,
                         xRouteIntervals);
}

/*
~Create~-function

*/
Word GLine::Create(const ListExpr typeInfo)
{
  return SetWord( new GLine(0) );
}

/*
~Delete~-function

*/
void GLine::Delete(const ListExpr typeInfo,
                   Word& w )
{
 // GLine *l = (GLine *)w.addr;
  w.addr = 0;
}

/*
~Close~-function

*/
void GLine::Close(const ListExpr typeInfo,
                  Word& w )
{
  delete (GLine*)w.addr;
  w.addr = 0;
}

/*
~Clone~-function

*/
Word GLine::Clone(const ListExpr typeInfo,
                  const Word& w )
{
  return SetWord( ((GLine*)w.addr)->Clone() );
}

/*
~Cast~-function

*/
void* GLine::Cast(void* addr)
{
  return new (addr) GLine;
}

/*
~SizeOf~-function

*/
int GLine::SizeOf()
{
  return sizeof(GLine);
}

/*
Another ~SizeOf~-function

*/
size_t GLine::Sizeof() const
{
  return sizeof(GLine);
}

/*
Adjazent-function

*/
bool GLine::Adjacent( const Attribute* arg ) const
{
  return false;
}

/*
Compare-function

*/
int GLine::Compare( const Attribute* arg ) const
{
  return false;
}

Attribute* GLine::Clone() const
{
  GLine* xOther = (GLine*)this;
  return new GLine(xOther);
}

size_t GLine::HashValue() const
{
  size_t xHash = m_iNetworkId;

  // Iterate over all RouteIntervalls
  for (int i = 0; i < m_xRouteIntervals.Size(); ++i)
  {
    // Get next Interval
    const RouteInterval* pCurrentInterval;
    m_xRouteIntervals.Get(i, pCurrentInterval);

    // Add something for each entry
    int iRouteId = pCurrentInterval->m_iRouteId;
    double dStart = pCurrentInterval->m_dStart;
    double dEnd = pCurrentInterval->m_dEnd;
    xHash += iRouteId + (size_t)dStart + (size_t)dEnd;
  }
  return xHash;
}

int GLine::NumOfFLOBs() const
{
  return 1;
}

FLOB* GLine::GetFLOB(const int i)
{
  return &m_xRouteIntervals;
}


void GLine::CopyFrom(const StandardAttribute* right)
{
  *this = *(const GLine *)right;
}

/*

Returns the length of the given GLine as double value

*/

double GLine::GetLength(){
  double length = 0.0;
  for (int i= 0; i<m_xRouteIntervals.Size(); ++i) {
    const RouteInterval* pCurrentInterval;
    m_xRouteIntervals.Get(i,pCurrentInterval);
    double posStart = pCurrentInterval->m_dStart;
    double posEnd = pCurrentInterval->m_dEnd;
    length = length + fabs(posEnd - posStart);
  };
  return length;
}


 int GLine::GetNetworkId() {
  return  m_iNetworkId;
 };

  void GLine::Get(const int i, const RouteInterval* &ri) const{
    m_xRouteIntervals.Get(i, ri);
  };

int GLine::NoOfComponents(){
  return m_xRouteIntervals.Size();
};

/*
Function describing the signature of the type constructor

*/
ListExpr GLine::Property()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> DATA"),
                             nl->StringAtom("gline"),
                    nl->StringAtom("(<nid> ((<rid> <startpos> <endpos>)...))"),
                             nl->StringAtom("(1 ((1 1.5 2.5)(2 1.5 2.0)))"))));
}

/*
Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~gpoint~ does not have arguments, this is trivial.

*/
bool GLine::Check( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "gline" ));
}

/*
Distance function computes the distance between two glines. Using Dijkstras-
Algorithm for shortestpath computing from OpShortestPath and GetLength-function
from GLine.

*/

double GLine::distance (GLine* pgl2){
  GLine* pgl1 = (GLine*) this;
  double minDist = numeric_limits<double>::max();
  double aktDist = numeric_limits<double>::max();
  double lastDist = numeric_limits<double>::max();
  if (pgl1->GetNetworkId() != pgl2->GetNetworkId()) {
    cmsg.inFunError("Both glines belong to different networks.");
    return minDist;
  }
  Network* pNetwork=NetworkManager::GetNetwork(pgl1->GetNetworkId());
  if (pNetwork == 0) {
    cmsg.inFunError("Network does not exist on this database");
    return minDist;
  };
  vector<JunctionSortEntry> juncsRoute;
  juncsRoute.clear();
  JunctionSortEntry pCurrJunc;
  pCurrJunc.m_pJunction = 0;
  vector<GPoint> gpointlistgline1;
  gpointlistgline1.clear();
  vector<GPoint> gpointlistgline2;
  gpointlistgline2.clear();
  const RouteInterval *pCurrRInterval1, *pCurrRInterval2;
  int i, j;
  size_t k;
  i = 0;
  while (i < pgl1->NoOfComponents()) {
    pgl1->Get(i, pCurrRInterval1);
    gpointlistgline1.push_back(GPoint(true, pgl1->GetNetworkId(),
                             pCurrRInterval1->m_iRouteId,
                             pCurrRInterval1->m_dStart));
    juncsRoute.clear();
    CcInt iRouteId(true, pCurrRInterval1->m_iRouteId);
    pNetwork->GetJunctionsOnRoute(&iRouteId, juncsRoute);
    k = 0;
    while ( k < juncsRoute.size()) {
      pCurrJunc = juncsRoute[k];
      if ((pCurrRInterval1->m_dStart < pCurrJunc.getRouteMeas() &&
          pCurrRInterval1->m_dEnd > pCurrJunc.getRouteMeas()) ||
          (pCurrRInterval1->m_dStart > pCurrJunc.getRouteMeas() &&
          pCurrRInterval1->m_dEnd < pCurrJunc.getRouteMeas())) {
        gpointlistgline1.push_back(GPoint(true, pgl1->GetNetworkId(),
                                 pCurrRInterval1->m_iRouteId,
                                 pCurrJunc.getRouteMeas()));
      }
      k++;
    }
    gpointlistgline1.push_back(GPoint(true, pgl1->GetNetworkId(),
                             pCurrRInterval1->m_iRouteId,
                             pCurrRInterval1->m_dEnd));
    i++;
  }
  j= 0;
  while (j < pgl2->NoOfComponents()){
    pgl2->Get(j, pCurrRInterval2);
    gpointlistgline2.push_back(GPoint(true, pgl2->GetNetworkId(),
                             pCurrRInterval2->m_iRouteId,
                             pCurrRInterval2->m_dStart));
    juncsRoute.clear();
    CcInt iRouteId(true, pCurrRInterval1->m_iRouteId);
    pNetwork->GetJunctionsOnRoute(&iRouteId, juncsRoute);
    k = 0;
    while ( k < juncsRoute.size()) {
      pCurrJunc = juncsRoute[k];
      if ((pCurrRInterval2->m_dStart < pCurrJunc.getRouteMeas() &&
           pCurrRInterval2->m_dEnd > pCurrJunc.getRouteMeas()) ||
          (pCurrRInterval1->m_dStart > pCurrJunc.getRouteMeas() &&
           pCurrRInterval1->m_dEnd < pCurrJunc.getRouteMeas())) {
        gpointlistgline2.push_back(GPoint(true, pgl1->GetNetworkId(),
                                      pCurrRInterval2->m_iRouteId,
                                      pCurrJunc.getRouteMeas()));
      }
      k++;
    }
    gpointlistgline2.push_back(GPoint(true, pgl1->GetNetworkId(),
                                   pCurrRInterval2->m_iRouteId,
                                   pCurrRInterval2->m_dEnd));
    j++;
  }
  i = 0;
  j = 0;
  while (i < pgl1->NoOfComponents()) {
    pgl1->Get(i, pCurrRInterval1);
    while (j < pgl2->NoOfComponents()){
      pgl2->Get(j, pCurrRInterval2);
      if (pCurrRInterval1->m_iRouteId == pCurrRInterval2->m_iRouteId) {
        if ((pCurrRInterval1->m_dStart <= pCurrRInterval2->m_dStart &&
           pCurrRInterval2->m_dStart <= pCurrRInterval1->m_dEnd) ||
           (pCurrRInterval2->m_dStart <= pCurrRInterval1->m_dStart &&
           pCurrRInterval1->m_dStart <= pCurrRInterval2->m_dEnd) ||
           (pCurrRInterval1->m_dStart <= pCurrRInterval2->m_dEnd &&
           pCurrRInterval2->m_dEnd <= pCurrRInterval1->m_dEnd) ||
           (pCurrRInterval2->m_dStart <= pCurrRInterval1->m_dEnd &&
           pCurrRInterval1->m_dEnd <= pCurrRInterval2->m_dEnd)) {
          minDist = 0.0;
          return minDist;
        } else {
          aktDist = fabs(pCurrRInterval1->m_dStart - pCurrRInterval2->m_dStart);
          if (aktDist < minDist) minDist = aktDist;
          aktDist = fabs(pCurrRInterval1->m_dEnd - pCurrRInterval2->m_dStart);
          if (aktDist < minDist) minDist = aktDist;
          aktDist = fabs(pCurrRInterval1->m_dStart - pCurrRInterval2->m_dEnd);
          if (aktDist < minDist) minDist = aktDist;
          aktDist = fabs(pCurrRInterval1->m_dStart - pCurrRInterval2->m_dEnd);
          if (aktDist < minDist) minDist = aktDist;
        }
      }
      j++;
    }
    i++;
  }
  for (size_t l = 0; l < gpointlistgline1.size(); l++){
    lastDist = numeric_limits<double>::max();
    for (size_t m = 0; m < gpointlistgline2.size(); m++) {
      if (gpointlistgline1[l].GetRouteId() != gpointlistgline2[m].GetRouteId()){
        aktDist = gpointlistgline1[l].distance(&gpointlistgline2[m]);
        if (aktDist < minDist) minDist = aktDist;
        if (minDist == 0) {
          juncsRoute.clear();
          gpointlistgline1.clear();
          gpointlistgline2.clear();
          NetworkManager::CloseNetwork(pNetwork);
          return minDist;
        }
        if (aktDist > lastDist) {
          int aktRouteId = gpointlistgline2[m].GetRouteId();
          while (m < gpointlistgline2.size()-1 &&
                 gpointlistgline2[m+1].GetRouteId() == aktRouteId) {
            m++;
          }
        } else {
          lastDist = aktDist;
        }
      }
      if ((m < gpointlistgline2.size()-1) &&
           (gpointlistgline2[m].GetRouteId() !=
            gpointlistgline2[m+1].GetRouteId()))
          lastDist = numeric_limits<double>::max();
    }
  }
  juncsRoute.clear();
  gpointlistgline1.clear();
  gpointlistgline2.clear();
  NetworkManager::CloseNetwork(pNetwork);
  return minDist;
}
