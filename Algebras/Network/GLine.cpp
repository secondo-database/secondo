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


#include "TupleIdentifier.h"
#include "GLine.h"

/*
3 Functions

3.1 

*/
  GLine::GLine() 
  {
  }
/*
The simple constructor. Should not be used.

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
      return;
    }
    
    // Read attributes from list
    // Read values from table
    int iRouteId = nl->IntValue( nl->First(xCurrentRouteInterval) );
    float fStart = nl->RealValue( nl->Second(xCurrentRouteInterval) );
    float fEnd  = nl->RealValue( nl->Third(xCurrentRouteInterval) );
    
    m_xRouteIntervals.push_back(RouteInterval(iRouteId,
                                              fStart,
                                              fEnd));
  }
  inout_bCorrect = true;
}

void GLine::SetNetworkId(int in_iNetworkId)
{
  m_iNetworkId = in_iNetworkId;
}
  
void GLine::AddRouteInterval(int in_iRouteId,
                             float fStart,
                             float fEnd)
{
  m_xRouteIntervals.push_back(RouteInterval(in_iRouteId,
                                            fStart,
                                            fEnd));
}   


/*
4 Static Functions supporting the type-constructor

4.1 ~In~-function

*/
Word GLine::In(const ListExpr in_xTypeInfo, 
               ListExpr in_xValue,
               int in_iErrorPos, 
               ListExpr& inout_xErrorInfo, 
               bool& inout_bCorrect)
{
  GLine* pLine = new GLine(in_xValue,
                           in_iErrorPos,
                           inout_xErrorInfo,
                           inout_bCorrect);

  if( inout_bCorrect )
  {
    return SetWord( pLine );
  }
  else
  {
    delete pLine;
    return SetWord( 0 );
  }
}

/*
4.2 ~Out~-function

*/
ListExpr GLine::Out(ListExpr in_xTypeInfo, 
                    Word in_xValue)
{
  GLine *pGline = (GLine*)in_xValue.addr;

  ListExpr xLast, xNext;
  bool bFirst = true;
  ListExpr xNetworkId = nl->IntAtom(pGline->m_iNetworkId);
  ListExpr xRouteIntervals;
  
  // Iterate over all RouteIntervalls
  for( size_t i = 0; i < pGline->m_xRouteIntervals.size(); i++ )
  {
    // Get next junction
    RouteInterval xCurrentInterval = pGline->m_xRouteIntervals[i];

    // Read values from table
    int iRouteId = xCurrentInterval.m_iRouteId;
    float fStart = xCurrentInterval.m_dStart;
    float fEnd = xCurrentInterval.m_dEnd;
  
    // Build list
    xNext = nl->ThreeElemList(nl->IntAtom(iRouteId),
                              nl->RealAtom(fStart),
                              nl->RealAtom(fEnd));
     
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
  return nl->TwoElemList(xNetworkId,
                         xRouteIntervals);
}

/*
4.3 ~Create~-function

*/
Word GLine::Create(const ListExpr typeInfo)
{
  return SetWord( new GLine() );
}

/*
4.4 ~Delete~-function

*/
void GLine::Delete(const ListExpr typeInfo, 
                   Word& w )
{
  delete (GLine*)w.addr;
  w.addr = 0;
}

/*
4.5 ~Close~-function

*/
void GLine::Close(const ListExpr typeInfo, 
                  Word& w )
{
  delete (GLine*)w.addr;
  w.addr = 0;
}

/*
4.6 ~Clone~-function

*/
Word GLine::Clone(const ListExpr typeInfo, 
                  const Word& w )
{
  return SetWord( 0 );
   
}

/*
4.7 ~Cast~-function

*/
void* GLine::Cast(void* addr)
{
  return new (addr) GLine;
}

/*
4.8 ~SizeOf~-function

*/
int GLine::SizeOf()
{
  return sizeof(GLine);
}


/*
4.9 Function describing the signature of the type constructor

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
4.10 Kind Checking Function

This function checks whether the type constructor is applied correctly. Since
type constructor ~gpoint~ does not have arguments, this is trivial.

*/
bool GLine::Check( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "gline" ));
}
