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

2011, April 14 Simone Jandt

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
1 Constructors and Deconstructors

The default constructor should only be used in the Cast-Function.

*/

Direction::Direction():Attribute()
{}

Direction::Direction(const bool defined): Attribute(defined)
{}

Direction::Direction(const Direction& other):Attribute(other.IsDefined())
{
  if (other.IsDefined()) side = other.GetDirection();
}

Direction::Direction(const JSide inside):Attribute(true)
{
  side = inside;
}

Direction::~Direction()
{}

/*
1 Getter and Setter

*/

JSide Direction::GetDirection() const
{
  return side;
}

void Direction::SetDirection(const JSide inside)
{
  side = inside;
}

/*
1 Override Methods from Attribute

*/

void Direction::CopyFrom(const Attribute* right)
{
  Direction* in = (Direction*) right;
  SetDefined(in->IsDefined());
  side = in->GetDirection();
}

Attribute::StorageType Direction::GetStorageType() const
{
  return Default;
}


size_t Direction::HashValue() const
{
  return (size_t) side;
}

Attribute* Direction::Clone() const
{
  return new Direction(*this);
}

bool Direction::Adjacent(const Attribute* attrib) const
{
  return false;
}

int Direction::Compare(const Attribute* rhs) const
{
  Direction* in = (Direction*) rhs;
  return Compare(*in);
}

int Direction::Compare(const Direction& indir) const
{
  if (indir.IsDefined() && !IsDefined()) return -1;
  if (!indir.IsDefined() && IsDefined()) return 1;
  return Compare(side, indir.GetDirection());
}

int Direction::Compare(const JSide& a, const JSide& b) const
{
  if (a == b) return 0;
  if (a == Up) return -1;
  if (b == Up) return 1;
  if (a == Down ) return -1;
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
1 Standard Operators

*/

Direction& Direction::operator=(const Direction& other)
{
  SetDefined(other.IsDefined());
  if (IsDefined()) side = other.GetDirection();
  return *this;
}

bool Direction::operator==(const Direction& other) const
{
  if (Compare(&other) == 0) return true;
  else return false;
}

bool Direction::operator==(const JSide& other) const
{
  if (IsDefined() && Compare(side, other) == 0) return true;
  else return false;
}

bool Direction::operator<(const Direction& other) const
{
  if (Compare(&other) < 0) return true;
  else return false;
}

bool Direction::operator<(const JSide& other) const
{
  if (IsDefined() && Compare(side, other) < 0) return true;
  else return false;
}

/*
1 Operators for Secondo Integration

1.1 Converts the direction into a nested list.

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
1.2 Constructs a Direction object from a nested list.

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
  return SetWord(new Direction(false));
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

bool Direction::Save(SmiRecord& valueRecord, size_t& offset,
                     const ListExpr typeInfo, Word& value)
{
  int i = -1;
  Direction* toSave = (Direction*) value.addr;
  if (toSave->IsDefined())
  {
    switch(toSave->GetDirection())
    {
      case Up:
      {
        i = 0;
        break;
      }

      case Down:
      {
        i = 1;
        break;
      }

      case Both:
      {
        i = 2;
        break;
      }
    }
  }
  valueRecord.Write(&i, sizeof(int), offset);
  offset += sizeof(int);
  return true;
}

bool Direction::Open(SmiRecord& valueRecord, size_t& offset,
                     const ListExpr typeInfo, Word& value )
{
  int i = -1;
  valueRecord.Read(&i, sizeof(int), offset);
  offset += sizeof(int);
  switch (i)
  {
    case 0:
    {
      value = SetWord(new Direction((JSide)Up));
      return true;
      break;
    }

    case 1:
    {
      value = SetWord(new Direction((JSide)Down));
      return true;
      break;
    }

    case 2:
    {
      value = SetWord(new Direction((JSide)Both));
      return true;
      break;
    }

    case -1:
    {
      value = SetWord(new Direction(false));
      return true;
      break;
    }

    default: // should never happen
    {
      value = SetWord(Address(0));
      return false;
      break;
    }
  }
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
  if (!IsDefined() && !dir.IsDefined())
    return true;
  else
  {
    if (!IsDefined() || !dir.IsDefined())
      return false;
    else
    {
      if (strict)
        return side == dir.GetDirection();
      else
      {
        if (side == dir.GetDirection() ||
            side == Both || dir.GetDirection() == Both)
          return true;
        else
          return false;
      }
    }
  }
}
