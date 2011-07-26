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

1 Includes

*/

#include "JRouteInterval.h"
#include "../../include/ListUtils.h"
#include "../../include/NestedList.h"
#include "../../include/NList.h"
#include "../../include/Symbols.h"

/*
1 ~class JRouteInterval~

1.1 Constructors and Deconstructor

The default constructor should never been used, except in the Cast-Function.

*/

JRouteInterval::JRouteInterval(): Attribute()
{}

JRouteInterval::JRouteInterval(const JRouteInterval& other):
  Attribute(other.IsDefined()),
  rid(other.GetRouteId()),
  startpos(other.GetStartPosition()),
  endpos(other.GetEndPosition()),
  side(other.GetSide())
{}

JRouteInterval::JRouteInterval(const int routeid, const double from,
                               const double to, const Direction sideofroad)
  : Attribute(true),
    rid(routeid),
    startpos(min(from,to)),
    endpos(max(from,to)),
    side(sideofroad)
{}

JRouteInterval::JRouteInterval(const bool defined) :Attribute(defined)
{}

JRouteInterval::JRouteInterval(const RouteLocation& from,
                               const RouteLocation& to): Attribute(true)
{
  if (!from.IsDefined() || !to.IsDefined() ||
       from.GetRouteId() != to.GetRouteId() ||
      !from.SameSide(to))
      SetDefined(false);
  else
  {
    rid = from.GetRouteId();
    startpos = min(from.GetPosition(),to.GetPosition());
    endpos = max(from.GetPosition(),to.GetPosition());
    side = min(from.GetSide(), to.GetSide());
  }
}

JRouteInterval::~JRouteInterval()
{}

/*
1.2 Getter and Setter for private Attributes

*/

  int JRouteInterval::GetRouteId() const
{
  return rid;
}

  double JRouteInterval::GetStartPosition()const
{
  return startpos;
}

  double JRouteInterval::GetEndPosition()const
{
  return endpos;
}

  Direction JRouteInterval::GetSide() const
{
  return side;
}

  void JRouteInterval::SetRouteId(const int routeid)
{
  rid = routeid;
}

  void JRouteInterval::SetStartPosition(const int position)
{
  startpos = position;
}

  void JRouteInterval::SetEndPosition(const int position)
{
  endpos = position;
}

  void JRouteInterval::SetSide(const Direction sideofroad)
{
  side = sideofroad;
}

/*
1.3 Override Methods from Attribute

*/

void JRouteInterval::CopyFrom(const Attribute* right)
{
  JRouteInterval* source = (JRouteInterval*) right;
  SetDefined(source->IsDefined());
  if (source->IsDefined())
  {
    rid = source->GetRouteId();
    startpos = source->GetStartPosition();
    endpos = source->GetEndPosition();
    side = source->GetSide();
  }
}

Attribute::StorageType JRouteInterval::GetStorageType() const
{
  return Default;
}

size_t JRouteInterval::HashValue() const
{
  return (size_t) rid + (size_t) startpos + (size_t) endpos + side.HashValue();
}

Attribute* JRouteInterval::Clone() const
{
  return new JRouteInterval(*this);
}

bool JRouteInterval::Adjacent(const Attribute* attrib) const
{
  JRouteInterval* in = (JRouteInterval*) attrib;
  if (rid == in->GetRouteId() && side.SameSide(in->GetSide()) &&
      (startpos == in->GetEndPosition() || endpos == in->GetStartPosition()))
    return true;
  else
    return false;
}

int JRouteInterval::Compare(const Attribute* rhs) const
{
  JRouteInterval* in = (JRouteInterval*) rhs;
  return Compare(*in);
}

int JRouteInterval::Compare(const JRouteInterval& in) const
{
  if (!IsDefined() && !in.IsDefined()) return 0;
  if (in.IsDefined() && !IsDefined()) return -1;
  if (!in.IsDefined() && IsDefined()) return 1;
  if (rid < in.GetRouteId()) return -1;
  if (rid > in.GetRouteId()) return 1;
  if (startpos < in.GetStartPosition()) return -1;
  if (startpos > in.GetStartPosition()) return 1;
  if (endpos < in.GetEndPosition()) return -1;
  if (endpos > in.GetEndPosition()) return 1;
  return side.Compare(in.GetSide());
}

size_t JRouteInterval::Sizeof() const
{
  return sizeof(JRouteInterval);
}

ostream& JRouteInterval::Print(ostream& os) const
{
  os << "RouteInterval: ";
  if (IsDefined())
  {
    os << "RouteId: " << rid
       << ", from: " << startpos
       << ", to: " << endpos
       << ", ";
       side.Print(os);
  }
  else
    os << Symbol::UNDEFINED() << endl;
  return os;
}

const string JRouteInterval::BasicType()
{
  return "jrouteinterval";
}

const bool JRouteInterval::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Operators

*/

JRouteInterval& JRouteInterval::operator=(const JRouteInterval& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    rid = other.GetRouteId();
    startpos = other.GetStartPosition();
    endpos = other.GetEndPosition();
    side = other.GetSide();
  }
  return *this;
}

bool JRouteInterval::operator==(const JRouteInterval& other) const
{
  return (Compare(&other) == 0);
}

/*
1.5 Operators for Secondo Integration

*/

ListExpr JRouteInterval::Out(ListExpr typeInfo, Word value)
{
  JRouteInterval* actValue = (JRouteInterval*) value.addr;
  if (!actValue->IsDefined())
    return nl->SymbolAtom(Symbol::UNDEFINED());
  else
  {
    Direction actDir(actValue->GetSide());
    return nl->FourElemList(nl->IntAtom(actValue->GetRouteId()),
                            nl->RealAtom(actValue->GetStartPosition()),
                            nl->RealAtom(actValue->GetEndPosition()),
                            Direction::Out(nl->TheEmptyList(),
                                           SetWord((void*) &actDir)));
  }
}

Word JRouteInterval::In(const ListExpr typeInfo, const ListExpr instance,
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
        return (new JRouteInterval (false) );
      }
    }
  }
  else
  {
    if (in_list.length() == 4)
    {
      NList routeIdList(in_list.first());
      NList startPosList(in_list.second());
      NList endPosList(in_list.third());
      NList sideList(in_list.fourth());
      int routeId = 0;
      double spos = 0.0;
      double epos = 0.0;
      if (routeIdList.isInt())
          routeId = routeIdList.intval();
      else
      {
        correct = false;
        cerr << "First should be int" << endl;
        return SetWord(Address(0));
      }
      if (startPosList.isReal())
        spos = startPosList.realval();
      else
      {
        correct = false;
        cerr << "Second should be real" << endl;
        return SetWord(Address(0));
      }
      if (endPosList.isReal())
        epos = endPosList.realval();
      else
      {
        correct = false;
        cerr << "Third should be real" << endl;
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
        cerr << "third should be jdirection" << endl;
        return SetWord(Address(0));
      }
      Direction* sideofroad = (Direction*) sideaddr.addr;
      JRouteInterval* res = new JRouteInterval(routeId, spos, epos,*sideofroad);
      sideofroad->DeleteIfAllowed();
      return SetWord(res);
    }
  }
  correct = false;
  cerr << "length should be one or four" << endl;
  return SetWord(Address(0));
}

Word JRouteInterval::Create(const ListExpr typeInfo)
{
  return SetWord(new JRouteInterval(false));
}

void JRouteInterval::Delete( const ListExpr typeInfo, Word& w )
{
  ((JRouteInterval*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void JRouteInterval::Close( const ListExpr typeInfo, Word& w )
{
  ((JRouteInterval*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word JRouteInterval::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(new JRouteInterval(*(JRouteInterval*) w.addr));
}

void* JRouteInterval::Cast( void* addr )
{
  return (new (addr) JRouteInterval);
}

bool JRouteInterval::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int JRouteInterval::SizeOf()
{
  return sizeof(JRouteInterval);
}

bool JRouteInterval::Save(SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value )
{
  JRouteInterval* toSave = (JRouteInterval*) value.addr;
  if (toSave->IsDefined())
  {
    int routeid = toSave->GetRouteId();
    valueRecord.Write(&routeid, sizeof(int), offset);
    offset += sizeof(int);
    double spos = toSave->GetStartPosition();
    valueRecord.Write(&spos, sizeof(double), offset);
    offset += sizeof(double);
    double epos = toSave->GetEndPosition();
    valueRecord.Write(&epos, sizeof(double), offset);
    offset += sizeof(double);
    Word wside;
    Direction* tdir = new Direction(toSave->GetSide());
    wside = SetWord(tdir);
    bool ok = Direction::Save(valueRecord, offset,
                              nl->TheEmptyList(),
                              wside);
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

bool JRouteInterval::Open(SmiRecord& valueRecord, size_t& offset,
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
  double spos;
  valueRecord.Read(&spos, sizeof(double), offset);
  offset += sizeof(double);
  double epos;
  valueRecord.Read(&epos, sizeof(double), offset);
  offset += sizeof(double);
  Word wside;
  if (Direction::Open(valueRecord, offset, nl->TheEmptyList(),
                      wside))
  {
    Direction direct = (*(Direction*) wside.addr);
    value = SetWord(new JRouteInterval(routeid, spos, epos, direct));
    return true;
  }
  return false;
}

/*
1.6 Helpful Operators

1.1 ~IsOneSided~

Returns true if the routeinterval covers only one side of the route.

*/

bool JRouteInterval::IsOneSided() const
{
  if (!IsDefined() || side == Both) return false;
  else return true;
}

/*
1.1 ~SameSide~

Returns true if the ~route intervals~ have identic ~side~ values or at least one
of them is both.

*/

bool JRouteInterval::SameSide(const JRouteInterval& other) const
{
  if (!IsDefined() || !other.IsDefined()) return false;
  else return side.SameSide(other.GetSide());
}

/*
1.1 Intersects

Returns true if the ~jrouteintervals~ belong to the same route and intersect.

*/

bool JRouteInterval::Intersects(const JRouteInterval& other) const
{
  if (!IsDefined() || !other.IsDefined()) return false;
  else
  {
    if (rid == other.GetRouteId() && SameSide(other) &&
       (endpos < other.GetStartPosition() ||
        other.GetEndPosition() < startpos ))
      return true;
    else
      return false;
  }
}


/*
1.1 Contains

Returns true if the ~routelocation~ is covered by the ~jrouteinterval~.

*/

bool JRouteInterval::Contains(const RouteLocation& rloc) const
{
  if (!IsDefined() || !rloc.IsDefined()) return false;
  else
  {
    if (rid == rloc.GetRouteId() && side.SameSide(rloc.GetSide()) &&
        startpos <= rloc.GetPosition() && endpos >= rloc.GetPosition())
      return true;
    else
      return false;
  }
}

/*
Returns true if the ~jrouteinterval~ covers the ~other jrouteinterval~.

*/

bool JRouteInterval::Contains(const JRouteInterval& other) const
{
  if (!IsDefined() || !other.IsDefined()) return false;
  else
  {
    if (Intersects(other) &&
        startpos <= other.GetStartPosition() &&
        endpos >= other.GetEndPosition())
      return true;
    else
      return false;
  }
}
