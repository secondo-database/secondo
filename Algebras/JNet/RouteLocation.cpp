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

2011, April Simone Jandt

*/

#include "RouteLocation.h"
#include "../../include/ListUtils.h"
#include "../../include/NestedList.h"
#include "../../include/NList.h"
#include "../../include/Symbols.h"
#include "../../include/LogMsg.h"


/*

1. ~class RouteLocation ~
1.1 Constructors and Deconstructors

*/

RouteLocation::RouteLocation():Attribute()
{}

RouteLocation::RouteLocation(const RouteLocation& other) :
  Attribute(other.IsDefined()),
  rid(other.GetRouteId()),
  pos(other.GetPosition()),
  side(other.GetSide())
{}

RouteLocation::RouteLocation(const bool defined):Attribute(defined)
{}

RouteLocation::RouteLocation(const int routeId, const double position,
                             const Direction sideofroad) :
    Attribute(true),
    rid(routeId),
    pos(position),
    side(sideofroad)
{}

RouteLocation::~RouteLocation()
{}

/*
 1 .2 Getter and Setter for private Attributes

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
  rid = routeid;
}

void RouteLocation::SetPosition(const double position)
{
  pos = position;
}

void RouteLocation::SetSide(const Direction sideofroad)
{
  side = sideofroad;
}

/*
1.3 Override Methods from Attribute

*/

void RouteLocation::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    RouteLocation* source = (RouteLocation*) right;
    rid = source->GetRouteId();
    pos = source->GetPosition();
    side = source->GetSide();
  }
}

Attribute::StorageType RouteLocation::GetStorageType() const
{
  return Default;
}

size_t RouteLocation::HashValue() const
{
  return (size_t) rid + (size_t) pos + side.HashValue();
}

Attribute* RouteLocation::Clone() const
{
  return new RouteLocation(*this);
}

bool RouteLocation::Adjacent(const RouteLocation attrib) const
{
  if (rid  == attrib.GetRouteId() &&
      AlmostEqual(pos, attrib.GetPosition()) &&
      side.SameSide(attrib.GetSide()))
    return true;
  else
    return false;
}

bool RouteLocation::Adjacent(const Attribute* attrib) const
{
  return Adjacent(*((RouteLocation*) attrib));
}

int RouteLocation::Compare(const Attribute* rhs) const
{
  RouteLocation* in = (RouteLocation*) rhs;
  return Compare(*in);
}

int RouteLocation::Compare(const RouteLocation& in) const
{
  if (in.IsDefined() && !IsDefined()) return -1;
  if (!in.IsDefined() && IsDefined()) return 1;
  if (rid < in.GetRouteId()) return -1;
  if (rid > in.GetRouteId()) return 1;
  if (pos < in.GetPosition()) return -1;
  if (pos > in.GetPosition()) return 1;
  return side.Compare(in.GetSide());
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
    os << Symbol::UNDEFINED;
  os << endl;
  return os;
}

const string RouteLocation::BasicType()
{
  return "routelocation";
}

const bool RouteLocation::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Operators

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

/*
1.5 Operators for Secondo Integration

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
      if (routeIdList.isInt())
        routeId = routeIdList.intval();
      else
      {
        correct = false;
        cmsg.inFunError("RouteLocation: First Element must be int");
        return SetWord(Address(0));
      }
      if (posList.isReal())
        position = posList.realval();
      else
      {
        correct = false;
        cmsg.inFunError("RouteLocation:Second Element must be real");
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
        cmsg.inFunError("RouteLocation: Third element must be jdirection");
        return SetWord(Address(0));
      }
      Direction* sideofroad =(Direction*)sideaddr.addr;
      RouteLocation* res = new RouteLocation(routeId, position, *sideofroad);
      sideofroad->DeleteIfAllowed();
      return SetWord(res);
    }
  }
  correct = false;
  cmsg.inFunError("list length should be 1 or 3");;
  return SetWord(Address(0));
}

Word RouteLocation::Create(const ListExpr typeInfo)
{
  return SetWord(new RouteLocation(false));
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

bool RouteLocation::Save(SmiRecord& valueRecord, size_t& offset,
                                const ListExpr typeInfo, Word& value )
{
  RouteLocation* toSave = (RouteLocation*) value.addr;
  if (toSave->IsDefined())
  {
    int routeid = toSave->GetRouteId();
    valueRecord.Write(&routeid, sizeof(int), offset);
    offset += sizeof(int);
    double position = toSave->GetPosition();
    valueRecord.Write(&position, sizeof(double),offset);
    offset += sizeof(double);
    Word wside;
    Direction* tdir = new Direction(toSave->GetSide());
    wside = SetWord(tdir);
    bool ok = Direction::Save(valueRecord, offset, nl->TheEmptyList(), wside);
    tdir->DeleteIfAllowed();
    return ok;
  }
  else
  {
    int i = -1;
    valueRecord.Write(&i, sizeof(int),offset);
    offset += sizeof(int);
    return true;
  }
}

bool RouteLocation::Open(SmiRecord& valueRecord, size_t& offset,
                                const ListExpr typeInfo, Word& value )
{
  int routeid = -1;
  valueRecord.Read(&routeid, sizeof(int), offset);
  offset += sizeof(int);
  if (routeid == -1)
  {
    value = SetWord(new RouteLocation(false));
    return true;
  }
  double position;
  valueRecord.Read(&position, sizeof(double), offset);
  offset += sizeof(double);
  Word wside;
  if (Direction::Open(valueRecord, offset, nl->TheEmptyList(),wside))
  {
    Direction direct = (*(Direction*) wside.addr);
    value = SetWord(new RouteLocation(routeid, position, direct));
    return true;
  }
  return false;
}

/*
1.6 Helpful operations

Returns true if the side values are identic or at least one of them is ~Both~.

*/

bool RouteLocation::SameSide(const RouteLocation& rloc) const
{
  if (!IsDefined() || !rloc.IsDefined()) return false;
  else return side.SameSide(rloc.GetSide());
}