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

1.1 Implementation of the Unit-Class GPoint

Mai-Oktober 2007 Martin Scheppokat

March 2008 Simone Jandt TemporalFunction, At added

*/
#include "TemporalAlgebra.h"
#include "TupleIdentifier.h"
#include "GPoint.h"
#include "UGPoint.h"

/*
Temporal Function

*/
void UGPoint::TemporalFunction( const Instant& t,
                                GPoint& result,
                                bool ignoreLimits ) const
{
  if( !IsDefined() ||
      !t.IsDefined() ||
      (!timeInterval.Contains( t ) && !ignoreLimits) ){
      result.SetDefined(false);
  } else {
    if( t == timeInterval.start ){
      result = p0;
      result.SetDefined(true);
    } else {
      if( t == timeInterval.end ) {
        result = p1;
        result.SetDefined(true);
      }  else {
        double tStart = timeInterval.start.ToDouble();
        double tEnd = timeInterval.end.ToDouble();
        double tInst = t.ToDouble();
        double posStart = p0.GetPosition();
        double posEnd = p1.GetPosition();
        double posInst = fabs(posEnd-posStart) * (tInst-tStart) /
                        (tEnd - tStart) + posStart;
        result = GPoint(true, p0.GetNetworkId(), p0.GetRouteId(), posInst,
                        p0.GetSide());
        result.SetDefined(true);
      }
    }
  }
  return;
}


/*
Checks wether a unit passes a fixed point in the network

*/
bool UGPoint::Passes( const GPoint& p ) const
{
  assert( IsDefined() );
  assert( p.IsDefined() );
  // Check if this unit is on the same route as the GPoint
  if(p0.GetRouteId()!= p.GetRouteId())
  {
    return false;
  }

  // p is between p0 and p1
  if((p0.GetPosition() < p.GetPosition() &&
     p.GetPosition() < p1.GetPosition()) ||
     (p1.GetPosition() < p.GetPosition() &&
     p.GetPosition() < p0.GetPosition()))
  {
    return true;
  }

  // If the edge of the interval is included we need to check the exakt
  // Position too.
  if((timeInterval.lc &&
     AlmostEqual(p0.GetPosition(), p.GetPosition())) ||
     (timeInterval.rc &&
     AlmostEqual(p1.GetPosition(),p.GetPosition())))
  {
    return true;
  }
  return false;
}


bool UGPoint::At( const GPoint& p, TemporalUnit<GPoint>& result ) const
{
  if (!IsDefined() || !p.IsDefined()) {
    cerr << "mgpoint and gpoint must be defined." << endl;
    return false;
  }
  assert (IsDefined());
  assert (p.IsDefined());
  UGPoint *pResult = (UGPoint*) &result;
  if (p0.GetNetworkId() != p.GetNetworkId()) {
    return false;
  } else {
    if (p0.GetRouteId() != p.GetRouteId()){
      return false;
    } else {
      if (p.GetSide() != p0.GetSide() &&
          !(p.GetSide() == 2 || p0.GetSide() == 2)) {
            return false;
      } else {
        double start = p0.GetPosition();
        double end = p1.GetPosition();
        double pos = p.GetPosition();
        if (AlmostEqual(start,pos) && timeInterval.lc) {
          Interval<Instant> interval(timeInterval.start,
                                     timeInterval.start, true, true);
          UGPoint aktunit(interval, p,p);
          *pResult = aktunit;
          return true;
        } else {
          if (AlmostEqual(end,pos) && timeInterval.rc) {
            Interval<Instant> interval(timeInterval.end,
                                       timeInterval.end, true, true);
            UGPoint aktunit(interval, p,p);
            *pResult = aktunit;
            return true;
          } else {
            if ((start < pos && pos < end) || (end < pos && pos < start)) {
              double factor = fabs(pos-start) / fabs(end-start);
              Instant tpos = (timeInterval.end - timeInterval.start) * factor +
                              timeInterval.start;
              Interval<Instant> interval(tpos, tpos, true, true);
              UGPoint aktunit(interval, p, p);
              *pResult = aktunit;
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

/*
Property-Function

*/
ListExpr UGPoint::Property()
{
  return (nl->TwoElemList(
            nl->FourElemList(nl->StringAtom("Signature"),
                             nl->StringAtom("Example Type List"),
                             nl->StringAtom("List Rep"),
                             nl->StringAtom("Example List")),
            nl->FourElemList(nl->StringAtom("-> UNIT"),
                             nl->StringAtom("(ugpoint) "),
nl->TextAtom("( timeInterval (<nid> <rid> <side> <pos1> <pos2> ) ) "),
nl->StringAtom("((i1 i2 TRUE FALSE) (1 1 0 0.0 0.3))"))));
}

/*
Kind Checking Function

*/
bool UGPoint::Check(ListExpr type, ListExpr& errorInfo)
{
  return (nl->IsEqual( type, "ugpoint" ));
}

/*
~Out~-function

*/
ListExpr UGPoint::Out(ListExpr typeInfo,
                      Word value)
{
  UGPoint* ugpoint = (UGPoint*)(value.addr);

  if( !(((UGPoint*)value.addr)->IsDefined()) )
  {
    return (nl->SymbolAtom("undef"));
  }
  else
  {
    ListExpr timeintervalList =
        nl->FourElemList(OutDateTime( nl->TheEmptyList(),
                                      SetWord(&ugpoint->timeInterval.start) ),
                         OutDateTime( nl->TheEmptyList(),
                                      SetWord(&ugpoint->timeInterval.end) ),
                         nl->BoolAtom( ugpoint->timeInterval.lc ),
                         nl->BoolAtom( ugpoint->timeInterval.rc));

    ListExpr pointsList =
         nl->FiveElemList(nl->IntAtom( ugpoint->p0.GetNetworkId() ),
                          nl->IntAtom( ugpoint->p0.GetRouteId() ),
                          nl->IntAtom( ugpoint->p0.GetSide() ),
                          nl->RealAtom( ugpoint->p0.GetPosition()),
                          nl->RealAtom( ugpoint->p1.GetPosition()));
      return nl->TwoElemList( timeintervalList, pointsList );
  }
}

/*
~In~-function

*/
Word UGPoint::In(const ListExpr typeInfo,
                 const ListExpr instance,
                 const int errorPos,
                 ListExpr& errorInfo,
                 bool& correct)
{
  string errmsg;
  if ( nl->ListLength( instance ) == 2 )
  {
    ListExpr first = nl->First( instance );

    if( nl->ListLength( first ) == 4 &&
        nl->IsAtom( nl->Third( first ) ) &&
        nl->AtomType( nl->Third( first ) ) == BoolType &&
        nl->IsAtom( nl->Fourth( first ) ) &&
        nl->AtomType( nl->Fourth( first ) ) == BoolType )
    {
      correct = true;
      Instant *start = (Instant *)InInstant( nl->TheEmptyList(),
       nl->First( first ),
        errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUGPoint(): Error in first instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        return SetWord( Address(0) );
      }

      Instant *end = (Instant *)InInstant( nl->TheEmptyList(),
       nl->Second( first ),
       errorPos, errorInfo, correct ).addr;

      if( !correct )
      {
        errmsg = "InUPoint(): Error in second instant.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete start;
        delete end;
        return SetWord( Address(0) );
      }

      Interval<Instant> tinterval( *start, *end,
                                   nl->BoolValue( nl->Third( first ) ),
                                   nl->BoolValue( nl->Fourth( first ) ) );
      delete start;
      delete end;

      correct = tinterval.IsValid();
      if (!correct)
        {
          errmsg = "InUPoint(): Non valid time interval.";
          errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
          return SetWord( Address(0) );
        }

      ListExpr second = nl->Second( instance );
      if( nl->ListLength( second ) == 5 &&
          nl->IsAtom( nl->First( second ) ) &&
          nl->AtomType( nl->First( second ) ) == IntType &&
          nl->IsAtom( nl->Second( second ) ) &&
          nl->AtomType( nl->Second( second ) ) == IntType &&
          nl->IsAtom( nl->Third( second ) ) &&
          nl->AtomType( nl->Third( second ) ) == IntType &&
          nl->IsAtom( nl->Fourth( second ) ) &&
          nl->AtomType( nl->Fourth( second ) ) == RealType &&
          nl->IsAtom( nl->Fifth( second ) ) &&
          nl->AtomType( nl->Fifth( second ) ) == RealType )
      {
        UGPoint *ugpoint = new UGPoint(tinterval,
                                     nl->IntValue( nl->First( second ) ),
                                     nl->IntValue( nl->Second( second ) ),
                                     (Side)nl->IntValue( nl->Third( second ) ),
                                     nl->RealValue( nl->Fourth( second ) ),
                                     nl->RealValue( nl->Fifth( second ) ));

        correct = ugpoint->IsValid();
        if( correct )
          return SetWord( ugpoint );

        errmsg = "InUGPoint(): Error in start/end point.";
        errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
        delete ugpoint;
      }
    }
  }
  else if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType
            && nl->SymbolValue( instance ) == "undef" )
    {
      UGPoint *ugpoint = new UGPoint(true);
      ugpoint->SetDefined(false);
      ugpoint->timeInterval=
        Interval<DateTime>(DateTime(instanttype),
                           DateTime(instanttype),true,true);
      correct = ugpoint->timeInterval.IsValid();
      if ( correct )
        return (SetWord( ugpoint ));
    }
  errmsg = "InUGPoint(): Error in representation.";
  errorInfo = nl->Append(errorInfo, nl->StringAtom(errmsg));
  correct = false;
  return SetWord( Address(0) );
}

/*
~Create~-function

*/
Word UGPoint::Create(const ListExpr typeInfo)
{
  return (SetWord( new UGPoint() ));
}

/*
~Delete~-function

*/
void UGPoint::Delete(const ListExpr typeInfo,
                     Word& w)
{
  delete (UGPoint *)w.addr;
  w.addr = 0;
}

/*
~Close~-function

*/
void UGPoint::Close(const ListExpr typeInfo,
                    Word& w)
{
  delete (UGPoint *)w.addr;
  w.addr = 0;
}

/*
~Clone~-function

*/
Word UGPoint::Clone(const ListExpr typeInfo,
                    const Word& w )
{
  UGPoint *ugpoint = (UGPoint *)w.addr;
  return SetWord( new UGPoint( *ugpoint ) );
}

/*
~Sizeof~-function

*/
int UGPoint::SizeOf()
{
  return sizeof(UGPoint);
}

/*
~Cast~-function

*/
void* UGPoint::Cast(void* addr)
{
  return new (addr) UGPoint;
}

int UGPoint::GetUnitRid(){
  return p0.GetRouteId();
}

double UGPoint::GetUnitStartPos(){
  return p0.GetPosition();
}

double UGPoint::GetUnitEndPos(){
  return p1.GetPosition();
}

double UGPoint::GetUnitStartTime(){
  return timeInterval.start.ToDouble();
}

double UGPoint::GetUnitEndTime(){
  return timeInterval.end.ToDouble();
}

