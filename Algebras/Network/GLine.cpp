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

Defines, includes, and constants

*/


#include "TupleIdentifier.h"
#include "GLine.h"

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
    if (!nl->ListLength(start) == 3) {
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
  GLine *l = (GLine *)w.addr;
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
    cout << "called Get for routeinterval: " << i << " of gline" << endl;
    m_xRouteIntervals.Get(i, ri);
    cout << "got route: " << ri->m_iRouteId << endl;
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

