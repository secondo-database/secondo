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
#include "../../include/ListUtils.h"
#include "../../include/NestedList.h"
#include "../../include/NList.h"
#include "../../include/Symbols.h"
#include "../../include/LogMsg.h"

extern NestedList* nl;

static bool DEBUG = false;

/*
1 Constructors and Deconstructors

The default constructor should only be used in the Cast-Function.

*/

Direction::Direction():Attribute()
{
  if (DEBUG) cout << "Direction::Direction()" << endl;
}

Direction::Direction(const bool defined): Attribute(defined)
{
  if (DEBUG) cout << "Direction::Direction(bool)" << endl;
}

Direction::Direction(const Direction& other):Attribute(other.IsDefined())
{
  if (DEBUG) cout << "Direction::Direction(other)" << endl;
  if (other.IsDefined()) side = other.GetDirection();
}

Direction::Direction(const JSide inside):Attribute(true)
{
  if (DEBUG) cout << "Direction::Direction(jside)" << endl;
  side = inside;
}

Direction::~Direction()
{if (DEBUG) cout << "Direction::~Direction()" << endl;}

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
  if (DEBUG) cout << "Direction::CopeFrom()" << endl;
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
  if (DEBUG) cout << "Direction::HashValue()" << endl;
  return (size_t) side;
}

Attribute* Direction::Clone() const
{
  if (DEBUG) cout << "Direction::Clone()" << endl;
  return new Direction(*this);
}

bool Direction::Adjacent(const Attribute* attrib) const
{
  if (DEBUG) cout << "Direction::Adjacent()" << endl;
  return false;
}

int Direction::Compare(const Attribute* rhs) const
{
  if (DEBUG) cout << "Direction::Compare(attr)" << endl;
  Direction* in = (Direction*) rhs;
  return Compare(*in);
}

int Direction::Compare(const Direction& indir) const
{
  if (DEBUG) cout << "Direction::Compare(Direction)" << endl;
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
  if (DEBUG) cout << "Direction::Sizeof" << endl;
  return sizeof(Direction);
}

ostream& Direction::Print(ostream& os) const
{
  if (DEBUG) cout << "Direction::Print" << endl;
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
  if (DEBUG) cout << "Direction::operator=" << endl;
  SetDefined(other.IsDefined());
  if (IsDefined()) side = other.GetDirection();
  return *this;
}

bool Direction::operator==(const Direction& other) const
{
  if (DEBUG) cout << "Direction::operator==(Direction)" << endl;
  if (Compare(&other) == 0) return true;
  else return false;
}

bool Direction::operator==(const JSide& other) const
{
  if (DEBUG) cout << "Direction::operator==(JSide)" << endl;
  if (IsDefined() && Compare(side, other) == 0) return true;
  else return false;
}

bool Direction::operator<(const Direction& other) const
{
  if (DEBUG) cout << "Direction::operator<" << endl;
  if (Compare(&other) < 0) return true;
  else return false;
}

bool Direction::operator<(const JSide& other) const
{
  if (DEBUG) cout << "Direction::operator<" << endl;
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
  if (DEBUG)
  {
    cout << "Direction::Out";
    source->Print(cout);
  }
  if (source->IsDefined())
  {
    switch(source->GetDirection())
    {
      case Up:
      {
        if (DEBUG) cout << "Case Up" << endl;
        NList res("Up");
        return res.enclose().listExpr();
        break;
      }

      case Down:
      {
        if (DEBUG) cout << "Case Down" << endl;
        NList res("Down");
        return res.enclose().listExpr();
        break;
      }

      case Both:
      {
        if (DEBUG) cout << "Case Both" << endl;
        NList res("Both");
        return res.enclose().listExpr();
        break;
      }

    }
  }
  if (DEBUG) cout << "Case undef" << endl;
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
  if (DEBUG)
  {
    cout << "Direction::In";
    in_inst.writeAsStringTo(cout);
  }
  if (in_inst.length() == 1)
  {
    NList first = in_inst.first();
    if (first.hasStringValue())
    {
      string s = first.str();
      if (s == "Up")
      {
        if (DEBUG) cout << "Case Up" << endl;
        correct = true;
        return SetWord(new Direction((JSide)Up));
      }
      if (s == "Down"){
        if (DEBUG) cout << "Case Down" << endl;
        correct = true;
        return SetWord(new Direction((JSide)Down));
      }
      if (s == "Both")
      {
        if (DEBUG) cout << "Case Both" << endl;
        correct = true;
        return SetWord(new Direction((JSide)Both));
      }
      if ( s == Symbol::UNDEFINED())
      {
        if (DEBUG) cout << "Case undefined" << endl;
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
  if (DEBUG) cout << "Direction::Create" << endl;
  return SetWord(new Direction(false));
}

void Direction::Delete( const ListExpr typeInfo, Word& w )
{
  if (DEBUG) cout << "Direction::Delete" << endl;
  ((Direction*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void Direction::Close( const ListExpr typeInfo, Word& w )
{
  if (DEBUG) cout << "Direction::Close" << endl;
  ((Direction*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word Direction::Clone( const ListExpr typeInfo, const Word& w )
{
  if (DEBUG) cout << "Direction::Clone" << endl;
   return SetWord(new Direction(*(Direction*) w.addr));
}

void* Direction::Cast( void* addr )
{
  return (new (addr) Direction);
}

bool Direction::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  if(DEBUG) cout << "Direction::kindCheck" << endl;
  return checkType(type);
}

int Direction::SizeOf()
{
  if (DEBUG) cout << "Direction::SizeOf" << endl;
  return sizeof(Direction);
}

bool Direction::Save(SmiRecord& valueRecord, size_t& offset,
                     const ListExpr typeInfo, Word& value)
{
  int i = -1;
  Direction* toSave = (Direction*) value.addr;
  if (DEBUG)
  {
    cout << "Direction: Save:";
    toSave->Print(cout);
  }
  if (toSave->IsDefined())
  {
    switch(toSave->GetDirection())
    {
      case Up:
      {
        if (DEBUG) cout << "case Up" << endl;
        i = 0;
        break;
      }

      case Down:
      {
        if (DEBUG) cout << "case Down" << endl;
        i = 1;
        break;
      }

      case Both:
      {
        if (DEBUG) cout << "case Both" << endl;
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
  if (DEBUG) cout << "Direction: Open i = " << i << endl;
  switch (i)
  {
    case 0:
    {
      if (DEBUG) cout << "case Up" << endl;
      value = SetWord(new Direction((JSide)Up));
      return true;
      break;
    }

    case 1:
    {
      if (DEBUG) cout << "case Down" << endl;
      value = SetWord(new Direction((JSide)Down));
      return true;
      break;
    }

    case 2:
    {
      if (DEBUG) cout << "case Both" << endl;
      value = SetWord(new Direction((JSide)Both));
      return true;
      break;
    }

    case -1:
    {
      if (DEBUG) cout << "case undef" << endl;
      value = SetWord(new Direction(false));
      return true;
      break;
    }

    default: // should never happen
    {
      if (DEBUG) cout << "default" << endl;
      value = SetWord(Address(0));
      return false;
      break;
    }
  }
}

/*
1 Helpful Operations

1.1 SameSide

Returns true if the both direction values are equal or one of them is ~Both~.

*/

bool Direction::SameSide(const Direction& dir) const
{
  if (DEBUG) cout << "Direction::SameSide" << endl;
  if (IsDefined() && dir.IsDefined())
  {
    if (side == dir.GetDirection() ||
        side == Both || dir.GetDirection() == Both)
      return true;
    else
      return false;
  }
  else
    return false;
}
