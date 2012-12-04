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

2012, May Simone Jandt

1 Includes

*/

#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"
#include "LogMsg.h"
#include "StandardTypes.h"
#include "RouteLocation.h"

using namespace jnetwork;

/*

1 Implementation of ~class RouteLocation ~

1.1 Constructors and Deconstructors

*/

RouteLocation::RouteLocation(): Attribute()
{}

RouteLocation::RouteLocation(const RouteLocation& other) :
  Attribute(other.IsDefined())
{
  if (other.IsDefined()){
    rid = other.GetRouteId();
    pos = other.GetPosition();
    side = other.GetSide();
  } else {
    rid = 0;
    pos = 0.0;
    side = (Direction) Both;
  }
}

RouteLocation::RouteLocation(const bool defined) :
  Attribute(defined), rid(0), pos(0.0), side(defined)
{}

RouteLocation::RouteLocation(const int routeId, const double position,
                             const Direction sideofroad) :
  Attribute(true), rid(routeId),  pos(position),  side(sideofroad)
{
  if(rid < 0 || pos < 0.0) SetDefined(false);
}

RouteLocation::RouteLocation(const int routeId, const double position,
                             const JSide sideofroad) :
  Attribute(true), rid(routeId), pos(position), side(Direction(sideofroad))
{
  if(!(rid >= 0 && pos >= 0.0)) SetDefined(false);
}

RouteLocation::~RouteLocation()
{}

/*
1.1 Getter and Setter for private Attributes

*/

int RouteLocation::GetRouteId() const
{
  return rid;
}

double RouteLocation::GetPosition() const
{
  return pos;
}

 Direction RouteLocation::GetSide() const
{
  return side;
}

void RouteLocation::SetRouteId(const int routeid)
{
  if(routeid >= 0) {
    rid = routeid;
  }

}

void RouteLocation::SetPosition(const double position)
{
  if (position >= 0.0)
    pos = position;
}

void RouteLocation::SetSide(const Direction sideofroad)
{
  side = sideofroad;
}

/*
1.1 Override Methods from Attribute

*/

void RouteLocation::CopyFrom(const Attribute* right)
{
  *this = *((RouteLocation*)right);
}

Attribute::StorageType RouteLocation::GetStorageType() const
{
  return Default;
}

size_t RouteLocation::HashValue() const
{
  return (size_t) rid + (size_t) pos + side.HashValue();
}

RouteLocation* RouteLocation::Clone() const
{
  return new RouteLocation(*this);
}

bool RouteLocation::Adjacent(const RouteLocation& attrib) const
{
  if (IsDefined() && attrib.IsDefined() && rid  == attrib.GetRouteId() &&
      pos == attrib.GetPosition() && side.SameSide(attrib.GetSide(),false))
    return true;
  else
    return false;
}

bool RouteLocation::Adjacent(const Attribute* attrib) const
{
  return Adjacent(*((RouteLocation*) attrib));
}

int RouteLocation::Compare(const void* rs, const void* ls)
{
  RouteLocation rhs(*(RouteLocation*)(rs));
  RouteLocation lhs(*(RouteLocation*)(ls));
  return rhs.Compare(lhs);
}

int RouteLocation::Compare(const Attribute* rhs) const
{
  return Compare(*((RouteLocation*)rhs));
}

int RouteLocation::Compare(const RouteLocation& in) const
{
  if (!IsDefined() && !in.IsDefined()) return 0;
  if (!IsDefined() && in.IsDefined()) return -1;
  if (IsDefined() && !in.IsDefined()) return 1;
  if (rid < in.GetRouteId()) return -1;
  if (rid > in.GetRouteId()) return 1;
  int test = side.Compare(in.GetSide());
  if (test != 0) return test;
  if (pos < in.GetPosition()) return -1;
  if (pos > in.GetPosition()) return 1;
  return test;
}

size_t RouteLocation::Sizeof() const
{
  return sizeof(RouteLocation);
}

ostream& RouteLocation::Print(ostream& os) const
{
  os << "RouteLocation: ";
  if (IsDefined())
  {
    os << "RouteId: " << rid;
    os << ", Position: " << pos << ", ";
    side.Print(os);
  }
  else
    os << Symbol::UNDEFINED << endl;

  return os;
}

const string RouteLocation::BasicType()
{
  return "rloc";
}

const bool RouteLocation::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

RouteLocation& RouteLocation::operator=(const RouteLocation& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    rid = other.GetRouteId();
    pos = other.GetPosition();
    side = other.GetSide();
  }
  return *this;
}

bool RouteLocation::operator==(const RouteLocation& other) const
{
  return (Compare(&other) == 0);
}

bool RouteLocation::operator!=(const RouteLocation& other) const
{
  return (Compare(&other) != 0);
}

bool RouteLocation::operator<(const RouteLocation& other) const
{
  return (Compare(&other) < 0);
}

bool RouteLocation::operator<=(const RouteLocation& other) const
{
  return (Compare(&other) < 1);
}

bool RouteLocation::operator>(const RouteLocation& other) const
{
  return (Compare(&other) > 0);
}

bool RouteLocation::operator>=(const RouteLocation& other) const
{
  return (Compare(&other) > -1);
}

/*
1.1 Operators for Secondo Integration

*/

ListExpr RouteLocation::Out(ListExpr typeInfo, Word value)
{
  RouteLocation* elem = (RouteLocation*) value.addr;
  if (!elem->IsDefined())
    return nl->SymbolAtom(Symbol::UNDEFINED());
  else
  {
    Direction elemdir(elem->GetSide());
    return nl->ThreeElemList(nl->IntAtom(elem->GetRouteId()),
                             nl->RealAtom(elem->GetPosition()),
                             Direction::Out(nl->TheEmptyList(),
                                            SetWord((void*) &elemdir)));
  }
}

Word RouteLocation::In(const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo, bool& correct)
{
  NList in_list(instance);
  if (in_list.length() == 1)
  {
    NList first = in_list.first();
    if (first.hasStringValue())
    {
      if (first.str() == Symbol::UNDEFINED())
      {
        correct = true;
        return (new RouteLocation (false) );
      }
    }
  }
  else
  {
    if (in_list.length() == 3)
    {
      NList routeIdList(in_list.first());
      NList posList(in_list.second());
      NList sideList(in_list.third());
      int routeId = 0;
      double position = 0.0;
      if (routeIdList.isInt() && routeIdList.intval() >= 0)
        routeId = routeIdList.intval();
      else
      {
        correct = false;
        cmsg.inFunError("1.Element must be " + CcInt::BasicType() + " >= 0.");
        return SetWord(Address(0));
      }
      if (posList.isReal() && posList.realval() >= 0.0)
        position = posList.realval();
      else
      {
        correct = false;
        cmsg.inFunError("2.Element must be " + CcReal::BasicType() + " >= 0.0");
        return SetWord(Address(0));
      }
      correct = true;
      Word sideaddr;
      if (sideList.isList())
        sideaddr = Direction::In(nl->TheEmptyList(),
                                    sideList.listExpr(),
                                    errorPos,
                                    errorInfo,
                                    correct);
      else
        sideaddr = Direction::In(nl->TheEmptyList(),
                                 nl->OneElemList(sideList.listExpr()),
                                 errorPos,
                                 errorInfo,
                                 correct);
      if (!correct)
      {
        cmsg.inFunError("3.Element must be " + Direction::BasicType());
        return SetWord(Address(0));
      }
      Direction* sideofroad =(Direction*)sideaddr.addr;
      RouteLocation* res = new RouteLocation(routeId, position, *sideofroad);
      sideofroad->DeleteIfAllowed();
      return SetWord(res);
    }
  }
  correct = false;
  cmsg.inFunError("List length should be one or three");;
  return SetWord(Address(0));
}

Word RouteLocation::Create(const ListExpr typeInfo)
{
  return SetWord(new RouteLocation(true));
}

void RouteLocation::Delete( const ListExpr typeInfo, Word& w )
{
  ((RouteLocation*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void RouteLocation::Close( const ListExpr typeInfo, Word& w )
{
  ((RouteLocation*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word RouteLocation::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(new RouteLocation(*(RouteLocation*) w.addr));
}

void* RouteLocation::Cast( void* addr )
{
  return (new (addr) RouteLocation);
}

bool RouteLocation::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int RouteLocation::SizeOf()
{
  return sizeof(RouteLocation);
}

ListExpr RouteLocation::Property()
{
  return nl->TwoElemList(
    nl->FourElemList(
      nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List")),
    nl->FourElemList(
      nl->StringAtom("-> "+ Kind::DATA()),
      nl->StringAtom(BasicType()),
      nl->TextAtom("(" + CcInt::BasicType() + " "+ CcReal::BasicType() + " " +
                     Direction::BasicType() + "), which means route id, " +
                     "distance from start of route, reachable from side " +
                     "of route."),
      nl->StringAtom(Example())));
}

/*
1.1 Helpful operations

1.1.1 ~Example~

Returns true if the side values are identic or at least one of them is ~Both~.

*/

string RouteLocation::Example()
{
  return "(1 2.0 " + Direction::Example() +")";
}

/*
1.1.1 ~SameSide~

Returns true if the both RouteLocations are on the same Side or, if
strict is set to false, if side of one of the both routelocations is Both.

*/

bool RouteLocation::SameSide(const RouteLocation& rloc,
                             const bool strict /*true*/) const
{
  if (IsDefined() && rloc.IsDefined())
    return side.SameSide(rloc.GetSide(), strict);
  else
    if (!IsDefined() && !rloc.IsDefined())
      return true;
    else
      return false;
}

/*
1.1.1.1 ~IsOnSameRoute~

Returns true if the rid is the same. Otherwise false.

*/

bool RouteLocation::IsOnSameRoute(const RouteLocation& rloc) const
{
  return rid == rloc.GetRouteId();
}

/*
1.1.1.1 NetBox

*/

Rectangle< 2  > RouteLocation::NetBox() const
{
  if (IsDefined())
    return Rectangle<2>(true, (double) rid, (double) rid, pos, pos);
  else
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
}

/*
1 Implementation of overloaded output operator

*/

ostream& operator<<(ostream& os, const RouteLocation& dir)
{
  dir.Print(os);
  return os;
}
