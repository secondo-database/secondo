/*
This file is part of SECONDO.

Copyright (C) 2011, University in Hagen, Department of Computer Science,
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

2012, July Simone Jandt

1 Includes

*/

#include "UJPoint.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"
#include "Direction.h"
#include "StandardTypes.h"



/*
1 Implementation of class ~UJPoint~

1.1 Constructors and Deconstructors

*/

UJPoint::UJPoint(): Attribute()
{}

UJPoint::UJPoint(const bool def) :
    Attribute(def), time(), rint(def)
{}

UJPoint::UJPoint(const UJPoint& other) :
    Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    time = other.GetTimeInterval();
    rint = other.GetRouteInterval();
  }
}


UJPoint::UJPoint(const string id, const Interval<Instant>& inst,
                 const JRouteInterval& r) :
  Attribute(true)
{
  if (inst.IsDefined() && r.IsDefined())
  {
    time = inst;
    SecondoCatalog* sc = SecondoSystem::GetCatalog();
    Word value;
    bool valDefined = false;
    if (sc->IsObjectName(id) &&
        sc->GetObject(id, value, valDefined) &&
        valDefined)
    {
      JNetwork* jnet = (JNetwork*) value.addr;
      if (jnet->Contains(&r))
      {
        strcpy(nid, id.c_str());
        rint = r;
      }
      else
      {
        SetDefined(false);
      }
      sc->CloseObject(nl->SymbolAtom(JNetwork::BasicType()), value);
    }
    else
    {
      SetDefined(false);
    }
  }
  else
  {
    SetDefined(false);
  }
}

UJPoint::UJPoint(const JNetwork* jnet, const JRouteInterval* jrint,
                 const Instant* starttime, const Instant* endtime,
                 const bool lc, const bool rc) :
  Attribute(true)
{
  if (jnet->IsDefined() && jrint->IsDefined() && jnet->Contains(jrint) &&
      starttime->IsDefined() && endtime->IsDefined())
  {
    strcpy(nid, *jnet->GetId());
    rint = *jrint;
    Interval<Instant>* i = new Interval<Instant> (*starttime, *endtime, lc, rc);
    time = *i;
    delete i;
  }
  else
  {
    SetDefined(false);
  }
}

UJPoint::~UJPoint()
{}

/*
1.1 Getter and Setter for private Attributes

*/

Interval<Instant> UJPoint::GetTimeInterval() const
{
  return time;
}

const STRING_T* UJPoint::GetNetworkId() const
{
  return &nid;
}


JRouteInterval UJPoint::GetRouteInterval() const
{
  return rint;
}

JPoint* UJPoint::GetStartPoint() const
{
  return new JPoint (nid, RouteLocation(rint.GetRouteId(),
                                        rint.GetStartPosition(),
                                        rint.GetSide()));
}

JPoint* UJPoint::GetEndPoint() const
{
  return new JPoint (nid, RouteLocation(rint.GetRouteId(),
                                        rint.GetEndPosition(),
                                        rint.GetSide()));
}


void UJPoint::SetTimeInterval(const Interval<Instant>& t)
{
  time = t;
}

void UJPoint::SetNetworkId(const STRING_T& id)
{
  strcpy(nid, id);
}


void UJPoint::SetRouteInterval(const JRouteInterval& r)
{
  rint = r;
}

/*
1.1 Override Methods from Attribute

*/

void UJPoint::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    UJPoint in(*(UJPoint*) right);
    strcpy(nid, *in.GetNetworkId());
    time = in.GetTimeInterval();
    rint = in.GetRouteInterval();
  }
}

Attribute::StorageType UJPoint::GetStorageType() const
{
  return Default;
}

size_t UJPoint::HashValue() const
{
  return strlen(nid) + time.start.HashValue() + time.end.HashValue() +
         rint.HashValue();
}

Attribute* UJPoint::Clone() const
{
  return new UJPoint(this);
}

bool UJPoint::Adjacent(const Attribute* attrib) const
{
  return false;
}

int UJPoint::Compare(const void* l, const void* r){
  UJPoint lp(*(UJPoint*) l);
  UJPoint rp(*(UJPoint*) r);
  return lp.Compare(rp);
}

int UJPoint::Compare(const Attribute* rhs) const
{
  UJPoint in(*(UJPoint*) rhs);
  return Compare(in);
}

int UJPoint::Compare(const UJPoint& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  int test = strcmp(nid, *rhs.GetNetworkId());
  if (test != 0) return test;
  test = time.CompareTo(rhs.GetTimeInterval());
  if (test != 0) return test;
  return rint.Compare(rhs.GetRouteInterval());
}

size_t UJPoint::Sizeof() const
{
  return sizeof(UJPoint);
}

ostream& UJPoint::Print(ostream& os) const
{
  os << "UJPoint ";
  if (IsDefined())
  {
    os << "in " << nid << " at ";
    time.Print(os);
    os << ", at ";
    rint.Print(os);
  }
  else
  {
    os << " : " << Symbol::UNDEFINED() << endl;
  }

  return os;
}

const string UJPoint::BasicType()
{
  return "ujpoint";
}

const bool UJPoint::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

UJPoint& UJPoint::operator=(const UJPoint& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    time = other.GetTimeInterval();
    rint = other.GetRouteInterval();
  }
  return *this;
}

bool UJPoint::operator==(const UJPoint& other) const
{
  return (Compare(other) == 0);
}

bool UJPoint::operator!=(const UJPoint& other) const
{
  return (Compare(other) != 0);
}

bool UJPoint::operator<(const UJPoint& other) const
{
  return (Compare(other) < 0);
}

bool UJPoint::operator<=(const UJPoint& other) const
{
  return (Compare(other) < 1);
}

bool UJPoint::operator>(const UJPoint& other) const
{
  return (Compare(other) > 0);
}

bool UJPoint::operator>=(const UJPoint& other) const
{
  return (Compare(other) > -1);
}

/*
1.1 Operators for Secondo Integration

*/

ListExpr UJPoint::Out(ListExpr typeInfo, Word value)
{
  UJPoint* in = (UJPoint*) value.addr;
  if (!in->IsDefined())
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  else
  {
    NList netList(*in->GetNetworkId(),true,false);
    Interval<Instant> netTime(in->GetTimeInterval());
    Instant* start = (Instant*)&netTime.start;
    Instant* end = (Instant*)&netTime.end;
    JRouteInterval netRI(in->GetRouteInterval());
    return nl->ThreeElemList(
      netList.listExpr(),
      nl->FourElemList(
        OutDateTime(nl->TheEmptyList(), SetWord(start)),
        OutDateTime(nl->TheEmptyList(), SetWord(end)),
        nl->BoolAtom( netTime.lc ),
        nl->BoolAtom( netTime.rc)),
      JRouteInterval::Out(nl->TheEmptyList(), SetWord((void*) &netRI)));
  }
}

Word UJPoint::In(const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if ( listutils::isSymbolUndefined( instance ) )
  {
    UJPoint* p = new UJPoint(false);
    correct = true;
    return SetWord( p );
  }
  else
  {
    if (nl->ListLength(instance) != 3)
    {
      correct = false;
      cmsg.inFunError("list length should be 1 or 3");;
      return SetWord(Address(0));
    }

    ListExpr netList = nl->First(instance);
    ListExpr intervalList = nl->Second(instance);
    ListExpr rintList = nl->Third(instance);

    STRING_T netId;
    strcpy (netId, nl->StringValue(netList).c_str());

    if (nl->ListLength(intervalList) != 4 ||
        !nl->IsAtom(nl->First(intervalList)) ||
        !nl->IsAtom(nl->Second(intervalList)) ||
        !nl->IsAtom(nl->Third(intervalList)) ||
        !nl->IsAtom(nl->Fourth(intervalList)) ||
        !nl->AtomType(nl->Third(intervalList) == BoolType) ||
        !nl->AtomType(nl->Fourth(intervalList) == BoolType))
    {
      cmsg.inFunError("Intervallist must have length 4");
      correct = false;
      return SetWord(Address(0));
    }

    Instant* start = (Instant*)InInstant( nl->TheEmptyList(),
                                          nl->First(intervalList),
                                          errorPos, errorInfo,
                                          correct ).addr;
    if(correct == false)
    {
      cmsg.inFunError("Invalid start time instant");
      return SetWord( Address(0) );
    }

    Instant* end = (Instant*)InInstant( nl->TheEmptyList(),
                                        nl->Second(intervalList),
                                        errorPos, errorInfo,
                                        correct ).addr;
    if(correct == false)
    {
      cmsg.inFunError("Invalid end time instant");
      delete start;
      return SetWord( Address(0) );
    }

    bool lc = nl->BoolValue(nl->Third(intervalList));
    bool rc = nl->BoolValue(nl->Fourth(intervalList));

    Interval<Instant> interval( *start, *end, lc, rc );
    correct = interval.IsValid();
    delete start;
    delete end;

    if (correct == false)
    {
      cmsg.inFunError("Timeinterval not valid.");
      return SetWord( Address(0) );
    }

    JRouteInterval* rint =
      (JRouteInterval*) JRouteInterval::In(nl->TheEmptyList(),
                                           rintList,
                                           errorPos, errorInfo,
                                           correct ).addr;
    if( correct == false )
    {
      cmsg.inFunError("Error in routeinterval list not correct.");
      return SetWord(Address(0));
    }

    UJPoint* out = new UJPoint(netId, interval,*rint);
    delete rint;
    return SetWord(out);
  }
}

Word UJPoint::Create(const ListExpr typeInfo)
{
  return SetWord(new UJPoint(true));
}

void UJPoint::Delete( const ListExpr typeInfo, Word& w )
{
  ((UJPoint*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void UJPoint::Close( const ListExpr typeInfo, Word& w )
{
  ((UJPoint*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word UJPoint::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(new UJPoint(*(UJPoint*) w.addr));
}

void* UJPoint::Cast( void* addr )
{
  return (new (addr) UJPoint);
}

bool UJPoint::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int UJPoint::SizeOf()
{
  return sizeof(UJPoint);
}

ListExpr UJPoint::Property()
{
  return nl->TwoElemList(
    nl->FourElemList(
      nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List")),
    nl->FourElemList(
      nl->StringAtom("-> " + Kind::TEMPORAL()),
      nl->StringAtom(BasicType()),
      nl->TextAtom("(string ("+ Instant::BasicType() + " " +
      Instant::BasicType() + " "+ CcBool::BasicType() +" " +
      CcBool::BasicType() + ") " + JRouteInterval::BasicType() +
      "), describes the positions of mjpoint within the time interval."),
      nl->TextAtom(Example())));
}


/*
1.1 Other Operations

*/

string UJPoint::Example()
{
  return "(netname ((instant 0.5)(instant 0.6) TRUE FALSE)" +
          JRouteInterval::Example() + ")";
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const UJPoint& up)
{
  up.Print(os);
  return os;
}

