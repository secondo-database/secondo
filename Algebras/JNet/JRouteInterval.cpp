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

#include "JRouteInterval.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "JPoint.h"

/*
1 Implementation of ~class JRouteInterval~

1.1 Constructors and Deconstructor

The default constructor should never been used, except in the Cast-Function.

*/

JRouteInterval::JRouteInterval() :
    Attribute()
{}

JRouteInterval::JRouteInterval(const JRouteInterval& other) :
    Attribute(other.IsDefined())
{
  if (other.IsDefined()){
    rid = other.GetRouteId();
    startpos = other.GetFirstPosition();
    endpos = other.GetLastPosition();
    side = other.GetSide();
  } else {
    rid = 0;
    startpos = 0.0;
    endpos = 0.0;
    side = (Direction) Both;
  }
}


JRouteInterval::JRouteInterval(const int routeid, const double from,
                               const double to, const Direction sideofroad) :
    Attribute(true), rid(routeid), startpos(min(from,to)), endpos(max(from,to)),
    side(sideofroad)
{
  if(!(rid >= 0 && startpos >= 0.0 && startpos <= endpos && endpos >= 0.0))
    SetDefined(false);
}

JRouteInterval::JRouteInterval(const int routeid, const double from,
                               const double to, const JSide sideofroad) :
    Attribute(true), rid(routeid), startpos(min(from,to)), endpos(max(from,to)),
    side(sideofroad)
{
  if(!(rid >= 0 && startpos >= 0.0 && startpos <= endpos && endpos >= 0.0))
    SetDefined(false);
}

JRouteInterval::JRouteInterval(const bool defined) :
    Attribute(defined), rid(0), startpos(0.0), endpos(0.0), side(defined)
{}

JRouteInterval::JRouteInterval(const RouteLocation& from,
                               const RouteLocation& to,
                               const bool allowResetSide /*false*/) :
  Attribute(true)
{
  if (from.IsDefined() && to.IsDefined() &&
      from.GetRouteId() == to.GetRouteId() &&
      from.SameSide(to, false))
  {
    SetDefined(true);
    rid = from.GetRouteId();
    startpos = min(from.GetPosition(),to.GetPosition());
    endpos = max(from.GetPosition(),to.GetPosition());
    side = min(from.GetSide(), to.GetSide());
    if (allowResetSide)
    {
      if (from.GetPosition() <= to.GetPosition())
        side = (Direction) Up;
      else
        side = (Direction) Down;
    }
  }
  else
  {
    SetDefined(false);
    rid = 0;
    startpos = 0.0;
    endpos = 0.0;
    side = (Direction) Both;
  }
}

JRouteInterval::~JRouteInterval()
{}

/*
1.1 Getter and Setter for private Attributes

*/

  int JRouteInterval::GetRouteId() const
{
  return rid;
}

  double JRouteInterval::GetFirstPosition()const
{
  return startpos;
}

  double JRouteInterval::GetLastPosition()const
{
  return endpos;
}

double JRouteInterval::GetStartPosition() const
{
  Direction compD(Down);
  if (side.Compare(compD) == 0)
    return endpos;
  else
    return startpos;
}

RouteLocation JRouteInterval::GetStartLocation() const
{
  Direction compD(Down);
  if (side.Compare(compD) == 0)
    return RouteLocation(rid, endpos, side);
  else
    return RouteLocation(rid, startpos, side);
}

double JRouteInterval::GetEndPosition() const
{
  Direction compD(Down);
  if (side.Compare(compD) == 0)
    return startpos;
  else
    return endpos;
}

RouteLocation JRouteInterval::GetEndLocation() const
{
  Direction compD(Down);
  if (side.Compare(compD) == 0)
    return RouteLocation(rid, startpos, side);
  else
    return RouteLocation(rid, endpos, side);
}

  Direction JRouteInterval::GetSide() const
{
  return side;
}

double JRouteInterval::GetLength() const
{
  return endpos - startpos;
}

  void JRouteInterval::SetRouteId(const int routeid)
{
  if (routeid >= 0)
    rid = routeid;
}

  void JRouteInterval::SetStartPosition(const double position)
{
  if (position >= 0.0){
    if (position > endpos)
    {
      startpos = endpos;
      endpos = position;
    }
    else
      startpos = position;
  }

}

  void JRouteInterval::SetEndPosition(const double position)
{
  if(position >= 0.0) {
    if (position > startpos)
      endpos = position;
    else
    {
      endpos = startpos;
      startpos = position;
    }
  }
}

  void JRouteInterval::SetSide(const Direction sideofroad)
{
  side = sideofroad;
}

void JRouteInterval::SetInterval(const double f, const double t)
{
  startpos =  min(f, t);
  endpos = max (f,t);
}

/*
1.1 Overwrite Methods from Attribute

*/

void JRouteInterval::CopyFrom(const Attribute* right)
{
  *this = *((JRouteInterval*)right);
}

Attribute::StorageType JRouteInterval::GetStorageType() const
{
  return Default;
}

size_t JRouteInterval::HashValue() const
{
  return (size_t) rid + (size_t) startpos + (size_t) endpos + side.HashValue();
}

JRouteInterval* JRouteInterval::Clone() const
{
  return new JRouteInterval(*this);
}

bool JRouteInterval::Adjacent(const JRouteInterval& other) const
{
  if (IsDefined() && other.IsDefined())
    return (rid == other.GetRouteId() &&
            (AlmostEqual(startpos, other.GetLastPosition()) ||
             AlmostEqual(endpos, other.GetFirstPosition())) &&
             side.SameSide(other.GetSide(),true));
  else
    return false;
}

bool JRouteInterval::Adjacent(const Attribute* attrib) const
{
  return Adjacent(*((JRouteInterval*) attrib));
}

int JRouteInterval::Compare(const Attribute* rhs) const
{
  return Compare(*((JRouteInterval*)rhs));
}

int JRouteInterval::Compare(const void* ls, const void* rs)
{
  JRouteInterval lhs( *(JRouteInterval*) ls);
  JRouteInterval rhs( *(JRouteInterval*) rs);
  return lhs.Compare(rhs);
}


int JRouteInterval::Compare(const JRouteInterval& in) const
{
  if (!IsDefined() && !in.IsDefined()) return 0;
  if (!IsDefined() && in.IsDefined()) return -1;
  if (IsDefined() && !in.IsDefined()) return 1;
  if (rid < in.GetRouteId()) return -1;
  if (rid > in.GetRouteId()) return 1;
  int test = side.Compare(in.GetSide());
  if (test == 0)
  {
    if (endpos < in.GetFirstPosition()) return -1;
    if (startpos > in.GetLastPosition()) return 1;
    if (startpos < in.GetFirstPosition()) return -1;
    if (startpos > in.GetFirstPosition()) return 1;
    if (endpos < in.GetLastPosition()) return -1;
    if (endpos > in.GetLastPosition()) return 1;
    return 0;
  }
  return test;
}

int JRouteInterval::Compare(const RouteLocation& rloc) const
{
  if (!IsDefined() && !rloc.IsDefined()) return 0;
  if (!IsDefined() && rloc.IsDefined()) return -1;
  if (IsDefined() && !rloc.IsDefined()) return 1;
  if (rid < rloc.GetRouteId()) return -1;
  if (rid > rloc.GetRouteId()) return 1;
  if (Contains(rloc)) return 0;
  Direction boDir(Both);
  int test = side.Compare(rloc.GetSide());
  if (side != boDir)
  {
    if (test == 0)
    {
      if (endpos < rloc.GetPosition()) return -1;
      if (startpos > rloc.GetPosition()) return 1;
    }
    return test;
  }
  else
  {
    if (startpos > rloc.GetPosition()) return 1;
    return -2; //might be special case for search in sorted list of jrint
  }
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
  return "jrint";
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
    startpos = other.GetFirstPosition();
    endpos = other.GetLastPosition();
    side = other.GetSide();
  }
  return *this;
}

bool JRouteInterval::operator==(const JRouteInterval& other) const
{
  return (Compare(&other) == 0);
}

bool JRouteInterval::operator!=(const JRouteInterval& other) const
{
  return (Compare(&other) != 0);
}

bool JRouteInterval::operator<(const JRouteInterval& other) const
{
  return (Compare(&other) < 0);
}

bool JRouteInterval::operator<=(const JRouteInterval& other) const
{
  return (Compare(&other) < 1);
}

bool JRouteInterval::operator>(const JRouteInterval& other) const
{
  return (Compare(&other) > 0);
}

bool JRouteInterval::operator>=(const JRouteInterval& other) const
{
  return (Compare(&other) > -1);
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
                            nl->RealAtom(actValue->GetFirstPosition()),
                            nl->RealAtom(actValue->GetLastPosition()),
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
      if (routeIdList.isInt() && routeIdList.intval() >= 0)
          routeId = routeIdList.intval();
      else
      {
        correct = false;
        cmsg.inFunError("1.Element should be " + CcInt::BasicType() + " >= 0.");
        return SetWord(Address(0));
      }
      if (startPosList.isReal() && startPosList.realval() >= 0)
      {
        spos = startPosList.realval();
      }
      else
      {
        correct = false;
        cmsg.inFunError("2.Element should be " + CcReal::BasicType() +" >= 0.");
        return SetWord(Address(0));
      }
      if (endPosList.isReal() && endPosList.realval() >= 0.0)
        epos = endPosList.realval();
      else
      {
        correct = false;
        cmsg.inFunError("3.Element should be " + CcReal::BasicType() +" >= 0.");
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
        cmsg.inFunError("Third should be jdirection");
        return SetWord(Address(0));
      }
      Direction* psideofroad = (Direction*) sideaddr.addr;
      JRouteInterval* res = new JRouteInterval(routeId, spos, epos,
                                               psideofroad->GetDirection());
      psideofroad->DeleteIfAllowed();
      return SetWord(res);
    }
  }
  correct = false;
  cmsg.inFunError("List length should be one or four");
  return SetWord(Address(0));
}

Word JRouteInterval::Create(const ListExpr typeInfo)
{
  return SetWord(new JRouteInterval(true));
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

ListExpr JRouteInterval::Property()
{
  return nl->TwoElemList(
    nl->FourElemList(
      nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List")),
    nl->FourElemList(
      nl->StringAtom("-> " + Kind::DATA()),
      nl->StringAtom(BasicType()),
      nl->TextAtom("(" + CcInt::BasicType() + " "+ CcReal::BasicType() + " " +
                    CcReal::BasicType() + " " + Direction::BasicType() + "), " +
                    "which means route id, part of the route described by the" +
                    " distance from the start of the route , side of route" +
                    "part."),
      nl->StringAtom(Example())));
}


/*
1.1 Helpful Operators

*/

string JRouteInterval::Example()
{
  return "(1 0.5 3.75 "+ Direction::Example() + ")";
}

/*
1.1.1 ~SameSide~

Returns true if the ~route intervals~ have identic ~side~ values or at least one
of them is both.

*/

bool JRouteInterval::SameSide(const JRouteInterval& other,
                              const bool strict /*true*/) const
{
  if (IsDefined() && other.IsDefined())
    return (side.SameSide(other.GetSide()), strict);
  else {
    if (!IsDefined() && !other.IsDefined())
      return true;
    else
      return false;
  }
}

/*
1.1 ~Overlaps~

*/

bool JRouteInterval::Overlaps(const JRouteInterval& other,
                              bool strict /* = True*/) const
{
  return (rid == other.GetRouteId() &&
          side.SameSide(other.GetSide(), strict)&&
          (!(startpos > other.GetLastPosition() ||
              endpos < other.GetFirstPosition())));
}


/*
1.1 Contains

Returns true if the JRouteInterval contains the route location.

*/

bool JRouteInterval::Contains(const RouteLocation& rloc) const
{
  return (IsDefined() && rloc.IsDefined() &&
          rid == rloc.GetRouteId() &&
          (startpos < rloc.GetPosition() ||
                        AlmostEqual(startpos, rloc.GetPosition())) &&
          (rloc.GetPosition() < endpos ||
                        AlmostEqual(endpos, rloc.GetPosition())) &&
          side.SameSide(rloc.GetSide(),false));
}

bool JRouteInterval::Contains(const RouteLocation& rloc,
                              const double tolerance) const
{
  return (IsDefined() && rloc.IsDefined() &&
          rid == rloc.GetRouteId() &&
          (startpos < rloc.GetPosition() ||
              AlmostEqualAbsolute(startpos, rloc.GetPosition(), tolerance)) &&
          (rloc.GetPosition() < endpos ||
              AlmostEqualAbsolute(endpos, rloc.GetPosition(), tolerance)) &&
            side.SameSide(rloc.GetSide(),false));
}

bool JRouteInterval::Contains(const JRouteInterval& other) const
{
  return (IsDefined() && other.IsDefined() && rid == other.GetRouteId() &&
          startpos <= other.GetFirstPosition() &&
          other.GetLastPosition()<= endpos &&
          side.SameSide(other.GetSide(), false));
}

/*
1.1.Extend

*/

JRouteInterval& JRouteInterval::Extend(const JRouteInterval& rint)
{
  if (IsDefined() && rint.IsDefined() && Adjacent(&rint))
  {
    startpos = min(startpos, rint.GetFirstPosition());
    endpos = max(endpos, rint.GetLastPosition());
  }
  return *this;
}

/*
1.1 Between

*/

bool JRouteInterval::Between(const RouteLocation& left,
                             const RouteLocation& right) const
{
  return (rid == left.GetRouteId() && rid == right.GetRouteId() &&
          startpos >= min(left.GetPosition(),right.GetPosition()) &&
          max(left.GetPosition(),right.GetPosition()) >= endpos);
}

/*
1.1. Inside

*/

bool JRouteInterval::Inside(const JRouteInterval& other) const
{
  return (rid == other.GetRouteId() && startpos >= other.GetFirstPosition() &&
          endpos <= other.GetLastPosition());
}

/*
1.1. Intersection

*/

JRouteInterval* JRouteInterval::Intersection(const JRouteInterval& rint) const
{
  if (IsDefined() && rint.IsDefined() && Overlaps(rint, false))
     return new JRouteInterval(rid,
                              max(startpos, rint.GetFirstPosition()),
                              min(endpos, rint.GetLastPosition()),
                              min(side, rint.GetSide()));
  else
    return 0;

}

/*
1.1 NetBox

*/

Rectangle<2> JRouteInterval::NetBox() const
{
  if (IsDefined())
    return Rectangle<2>(true, (double) rid, (double) rid, startpos, endpos);
  else
    return Rectangle<2>(false,0.0,0.0,0.0,0.0);
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JRouteInterval& jir)
{
  jir.Print(os);
  return os;
}
