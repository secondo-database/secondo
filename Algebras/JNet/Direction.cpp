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

#include "Direction.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"
#include "LogMsg.h"

extern NestedList* nl;

/*
1 Implementation of class ~Direction~

1.1 Constructors and Deconstructors

The default constructor should only be used in the Cast-Function.

*/

Direction::Direction():Attribute()
{}

Direction::Direction(const bool defined): Attribute(defined), side(Both)
{}

Direction::Direction(const Direction& other) : Attribute(other.IsDefined())
{
  if (other.IsDefined())
    side = other.GetDirection();
}

Direction::Direction(const JSide inside) : Attribute(true)
{
  side = inside;
}

Direction::~Direction()
{}

/*
1.1 Getter and Setter

*/

JSide Direction::GetDirection() const
{
  return side;
}

void Direction::SetSide(const JSide inside)
{
  side = inside;
}

void Direction::SetDirection(const Direction& indir)
{
  side = indir.GetDirection();
}

/*
1.1 Override Methods from Attribute

*/

void Direction::CopyFrom(const Attribute* right)
{
  *this = *((Direction*)right);
}

Attribute::StorageType Direction::GetStorageType() const
{
  return Default;
}


size_t Direction::HashValue() const
{
  if (IsDefined()) return (size_t) side;
  else return (size_t) 0;
}

Direction* Direction::Clone() const
{
  return new Direction(*this);
}

bool Direction::Adjacent(const Attribute* attrib) const
{
  return false;
}

int Direction::Compare(const void* ls, const void* rs)
{
  Direction pl(*(Direction*) ls);
  Direction pr(*(Direction*) rs);
  return pl.Compare(pr);
}

int Direction::Compare(const Attribute* rhs) const
{
  Direction in(*(Direction*) rhs);
  return Compare(in);
}

int Direction::Compare(const Direction& indir) const
{
  if (!IsDefined() && !indir.IsDefined()) return 0;
  if (!IsDefined() && indir.IsDefined()) return -1;
  if (IsDefined() && !indir.IsDefined()) return 1;
  return Compare(side, indir.GetDirection());
}

int Direction::Compare(const JSide& a, const JSide& b) const
{
  if (a == b) return 0;
  if (a == Up) return -1;
  if (b == Up) return 1;
  if (b == Both) return -1;
  return 1;
}

size_t Direction::Sizeof() const
{
  return sizeof(Direction);
}

ostream& Direction::Print(ostream& os) const
{
  os << "Direction: ";
  if (IsDefined())
  {
    switch(side)
    {
      case Up:
        os << "Up";
        break;

      case Down:
        os << "Down";
        break;

      case Both:
        os << "Both";
        break;
    }
  }
  else
  {
    os << Symbol::UNDEFINED();
  }
  os << endl;
  return os;
}

const string Direction::BasicType()
{
  return "jdirection";
}

const bool Direction::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

Direction& Direction::operator=(const Direction& other)
{
  SetDefined(other.IsDefined());
  if (IsDefined()) side = other.GetDirection();
  return *this;
}

bool Direction::operator==(const Direction& other) const
{
  return (Compare(&other) == 0);
}

bool Direction::operator<(const Direction& other) const
{
  return (Compare(&other) < 0);
}

bool Direction::operator<=(const Direction& other) const
{
  return (Compare(&other) < 1);
}

bool Direction::operator>(const Direction& other) const
{
  return (Compare(&other) > 0 );
}

bool Direction::operator>=(const Direction& other) const
{
  return (Compare(&other) > -1);
}

bool Direction::operator!=(const Direction& other) const
{
  return (Compare(&other) != 0);
}


/*
1.1 Operators for Secondo Integration

1.1.1 Converts the direction into a nested list.

*/

ListExpr Direction::Out(ListExpr typeInfo, Word value)
{
  Direction* source = (Direction*) value.addr;
  if (source->IsDefined())
  {
    switch(source->GetDirection())
    {
      case Up:
      {
        NList res("Up");
        return res.enclose().listExpr();
        break;
      }

      case Down:
      {
        NList res("Down");
        return res.enclose().listExpr();
        break;
      }

      case Both:
      {
        NList res("Both");
        return res.enclose().listExpr();
        break;
      }

    }
  }
  NList res(Symbol::UNDEFINED());
  return res.enclose().listExpr();
}

/*
1.1.1 Constructs a Direction object from a nested list.

*/

Word Direction::In(const ListExpr typeInfo, const ListExpr instance,
                   const int errorPos, ListExpr& errorInfo, bool& correct)
{
  NList in_inst(instance);
  if (in_inst.length() == 1)
  {
    NList first = in_inst.first();
    if (first.hasStringValue())
    {
      string s = first.str();
      if (s == "Up")
      {
        correct = true;
        return SetWord(new Direction((JSide)Up));
      }
      if (s == "Down"){
        correct = true;
        return SetWord(new Direction((JSide)Down));
      }
      if (s == "Both")
      {
        correct = true;
        return SetWord(new Direction((JSide)Both));
      }
      if ( s == Symbol::UNDEFINED())
      {
        correct = true;
        return SetWord(new Direction(false));
      }
    }
  }
  correct = false;
  cmsg.inFunError("Direction must be Up, Down, Both or " + Symbol::UNDEFINED());
  return SetWord(Address( 0 ));
}

Word Direction::Create(const ListExpr typeInfo)
{
  return SetWord(new Direction(true));
}

void Direction::Delete( const ListExpr typeInfo, Word& w )
{
  ((Direction*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void Direction::Close( const ListExpr typeInfo, Word& w )
{
  ((Direction*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word Direction::Clone( const ListExpr typeInfo, const Word& w )
{
   return SetWord(new Direction(*(Direction*) w.addr));
}

void* Direction::Cast( void* addr )
{
  return (new (addr) Direction);
}

bool Direction::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int Direction::SizeOf()
{
  return sizeof(Direction);
}

ListExpr Direction::Property()
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
      nl->TextAtom("(<JSide>) where JSide is Up, Down or Both"),
      nl->StringAtom("("+ Example() +")")));
}

/*
1 Helpful Operations

*/

string Direction::Example()
{
  return "Up";
}


/*
1.1 SameSide

Returns true if the both direction values are equal or one of them is ~Both~.

*/

bool Direction::SameSide(const Direction& dir, const bool strict /*true*/)const
{
  if (Compare(&dir) == 0)
    return true;
  else
  {
    if (strict)
      return false;
    else
    {
      if (!IsDefined() || !dir.IsDefined())
        return false;
      else
      {
        if (side == Both || dir.GetDirection() == Both)
          return true;
        else
          return false;
      }
    }
  }
}

/*
1 Implementation of overloaded output operator

*/

ostream& operator<<(ostream& os, const Direction& dir)
{
  dir.Print(os);
  return os;
}
