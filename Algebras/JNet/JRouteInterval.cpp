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
    startpos = other.GetStartPosition();
    endpos = other.GetEndPosition();
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
                               const RouteLocation& to) :
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
  } else {
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

/*
1.1 Overwrite Methods from Attribute

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
  if ( rid == in->GetRouteId() &&
       (AlmostEqual(startpos, in->GetEndPosition()) ||
        AlmostEqual(endpos, in->GetStartPosition())) &&
       side.SameSide(in->GetSide(),false))
    return true;
  else
    return false;
}

int JRouteInterval::Compare(const Attribute* rhs) const
{
  JRouteInterval in ( *(JRouteInterval*) rhs);
  return Compare(in);
}

int JRouteInterval::Compare(const void* ls, const void* rs) const
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

bool JRouteInterval::Overlaps(const JRouteInterval& other) const
{
  if (rid == other.GetRouteId() && SameSide(other,true))
  {
    if (startpos <= other.GetEndPosition() ||
        endpos <= other.GetStartPosition())
      return true;
    else
      return false;
  }
  else
    return false;
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JRouteInterval& jir)
{
  jir.Print(os);
  return os;
}
