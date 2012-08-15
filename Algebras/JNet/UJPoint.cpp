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

#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "Direction.h"
#include "UJPoint.h"
#include "JNetwork.h"


/*
1 Implementation of class ~UJPoint~

1.1 Constructors and Deconstructors

*/

UJPoint::UJPoint(): Attribute()
{}

UJPoint::UJPoint(const bool def) :
    Attribute(def), unit(def)
{}

UJPoint::UJPoint(const UJPoint& other) :
    Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    unit = other.GetUnit();
  }
}

UJPoint::UJPoint(const string id, const JUnit& u) :
  Attribute(true), unit(u)
{
  SecondoCatalog* sc = SecondoSystem::GetCatalog();
  Word value;
  bool valDefined = false;
  if (sc->IsObjectName(id) &&
      sc->GetObject(id, value, valDefined) &&
            valDefined)
  {
    JNetwork* jnet = (JNetwork*) value.addr;
    JRouteInterval rint(u.GetRouteInterval());
    if (jnet->Contains(&rint))
      strcpy(nid, id.c_str());
    else
      SetDefined(false);
    sc->CloseObject(nl->SymbolAtom(JNetwork::BasicType()), value);
  }
}

UJPoint::UJPoint(const string id, const Interval<Instant>& inst,
                 const JRouteInterval& r) :
  Attribute(true), unit(true)
{
  if (inst.IsDefined() && r.IsDefined())
  {
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
        unit.SetTimeInterval(inst);
        unit.SetRouteInterval(r);
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
                 const Interval<Instant>* timeInter) :
  Attribute(true), unit(true)
{
  if (jnet->IsDefined() && jrint->IsDefined() && jnet->Contains(jrint) &&
      timeInter->IsDefined())
  {
    strcpy(nid, *jnet->GetId());
    unit.SetTimeInterval(*timeInter);
    unit.SetRouteInterval(*jrint);
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

const STRING_T* UJPoint::GetNetworkId() const
{
  return &nid;
}


JUnit UJPoint::GetUnit() const
{
  return unit;
}

void UJPoint::SetNetworkId(const STRING_T& id)
{
  strcpy(nid, id);
}


void UJPoint::SetUnit(const JUnit& j)
{
  unit = j;
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
    unit = in.GetUnit();
  }
}

Attribute::StorageType UJPoint::GetStorageType() const
{
  return Default;
}

size_t UJPoint::HashValue() const
{
  return strlen(nid) + unit.HashValue();
}

Attribute* UJPoint::Clone() const
{
  return new UJPoint(*this);
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
  return unit.Compare(rhs.GetUnit());
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
    os << "Move in " << nid << ":";
    unit.Print(os);
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
    unit = other.GetUnit();
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
    JUnit junit(in->GetUnit());
    return nl->TwoElemList(
      netList.listExpr(),
      JUnit::Out(nl->TheEmptyList(), SetWord((void*) &junit)));
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
    if (nl->ListLength(instance) != 2)
    {
      correct = false;
      cmsg.inFunError("list length should be 1 or 2");;
      return SetWord(Address(0));
    }

    ListExpr netList = nl->First(instance);
    ListExpr unitList = nl->Second(instance);

    STRING_T netId;
    strcpy (netId, nl->StringValue(netList).c_str());

    JUnit* junit = (JUnit*) JUnit::In(nl->TheEmptyList(), unitList,
                                      errorPos, errorInfo, correct ).addr;
    if( correct == false )
    {
      cmsg.inFunError("Error in unit list.");
      return SetWord(Address(0));
    }

    UJPoint* out = new UJPoint(netId, *junit);
    junit->DeleteIfAllowed();
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
      nl->TextAtom("(string ("+ JUnit::BasicType() + ")), describes the " +
      " positions of mjpoint within the time interval."),
      nl->TextAtom(Example())));
}


/*
1.1 Other Operations

*/

string UJPoint::Example()
{
  return "(netname (" + JUnit::Example() + "))";
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const UJPoint& up)
{
  up.Print(os);
  return os;
}

