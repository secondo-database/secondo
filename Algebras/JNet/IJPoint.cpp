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

#include "IJPoint.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"


/*
1 Implementation of class ~IJPoint~

1.1 Constructors and Deconstructors

*/

IJPoint::IJPoint(): Attribute()
{}

IJPoint::IJPoint(const bool def) :
    Attribute(def), time(0.0), point(def)
{}

IJPoint::IJPoint(const IJPoint& other) :
    Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    time = other.GetInstant();
    point = other.GetPoint();
  }
  else
  {
    time.SetDefined(false);
    point.SetDefined(false);
  }
}

IJPoint::IJPoint(const Instant& inst, const JPoint& jp) :
  Attribute(true)
{
  if (inst.IsDefined() && jp.IsDefined()){
    time = inst;
    point = jp;
  } else {
    SetDefined(false);
  }
}

IJPoint::~IJPoint()
{}

/*
1.1 Getter and Setter for private Attributes

*/

Instant IJPoint::GetInstant() const
{
  return time;
}

JPoint IJPoint::GetPoint() const
{
  return point;
}

void IJPoint::SetInstant(const Instant& t)
{
  time = t;
}

void IJPoint::SetPoint(const JPoint& jp)
{
  point = jp;
}

/*
1.1 Override Methods from Attribute

*/

void IJPoint::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    IJPoint in(*(IJPoint*) right);
    time = in.GetInstant();
    point = in.GetPoint();
  }
}

Attribute::StorageType IJPoint::GetStorageType() const
{
  return Default;
}

size_t IJPoint::HashValue() const
{
  return time.HashValue() + point.HashValue();
}

Attribute* IJPoint::Clone() const
{
  return new IJPoint(*this);
}

bool IJPoint::Adjacent(const Attribute* attrib) const
{
  return false;
}

int IJPoint::Compare(const void* l, const void* r){
  IJPoint lp(*(IJPoint*) l);
  IJPoint rp(*(IJPoint*) r);
  return lp.Compare(rp);
}

int IJPoint::Compare(const Attribute* rhs) const
{
  IJPoint in(*(IJPoint*) rhs);
  return Compare(in);
}

int IJPoint::Compare(const IJPoint& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  int test = -1*rhs.GetInstant().Compare(&time);
  if (test != 0) return test;
  return point.Compare(rhs.GetPoint());
}

size_t IJPoint::Sizeof() const
{
  return sizeof(IJPoint);
}

ostream& IJPoint::Print(ostream& os) const
{
  os << "IJPoint: ";
  time.Print(os);
  os << ", ";
  point.Print(os);
  return os;
}

const string IJPoint::BasicType()
{
  return "ijpoint";
}

const bool IJPoint::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

IJPoint& IJPoint::operator=(const IJPoint& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    time = other.GetInstant();
    point = other.GetPoint();
  }
  return *this;
}

bool IJPoint::operator==(const IJPoint& other) const
{
  return (Compare(other) == 0);
}

bool IJPoint::operator!=(const IJPoint& other) const
{
  return (Compare(other) != 0);
}

bool IJPoint::operator<(const IJPoint& other) const
{
  return (Compare(other) < 0);
}

bool IJPoint::operator<=(const IJPoint& other) const
{
  return (Compare(other) < 1);
}

bool IJPoint::operator>(const IJPoint& other) const
{
  return (Compare(other) > 0);
}

bool IJPoint::operator>=(const IJPoint& other) const
{
  return (Compare(other) > -1);
}

/*
1.1 Operators for Secondo Integration

*/

ListExpr IJPoint::Out(ListExpr typeInfo, Word value)
{
  IJPoint* in = (IJPoint*) value.addr;
  if (!in->IsDefined())
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  else
  {
    Instant t(in->GetInstant());
    JPoint pt(in->GetPoint());
    return nl->TwoElemList(OutDateTime(nl->TheEmptyList(), SetWord((void*)&t)),
                           JPoint::Out(nl->TheEmptyList(),
                                       SetWord((void*) &pt)));
  }
}

Word IJPoint::In(const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if ( listutils::isSymbolUndefined( instance ) )
  {
    correct = true;
    return (new IJPoint(false));
  }
  else
  {
    if (nl->ListLength(instance) == 2)
    {
      Instant *t = (Instant *)InInstant( nl->TheEmptyList(),
                                               nl->First( instance ),
                                               errorPos,
                                               errorInfo,
                                               correct ).addr;
      if(correct == false)
      {
        cmsg.inFunError("Invalid time instant");
        delete t;
        return SetWord( Address(0) );
      }

      JPoint* j = (JPoint*) JPoint::In( nl->TheEmptyList(),
                                        nl->Second( instance ),
                                        errorPos, errorInfo, correct ).addr;
      if( correct  )
      {
        IJPoint* out = new IJPoint(*t,*j);
        delete t;
        delete j;
        return SetWord(out);
      }
      else
      {
        cmsg.inFunError("Invalid jpoint.");
        delete t;
        delete j;
      }
    }
  }
  correct = false;
  cmsg.inFunError("list length should be 1 or 2");;
  return SetWord(Address(0));
}

Word IJPoint::Create(const ListExpr typeInfo)
{
  return SetWord(new IJPoint(true));
}

void IJPoint::Delete( const ListExpr typeInfo, Word& w )
{
  ((IJPoint*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void IJPoint::Close( const ListExpr typeInfo, Word& w )
{
  ((IJPoint*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word IJPoint::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(new IJPoint(*(IJPoint*) w.addr));
}

void* IJPoint::Cast( void* addr )
{
  return (new (addr) IJPoint);
}

bool IJPoint::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int IJPoint::SizeOf()
{
  return sizeof(IJPoint);
}

ListExpr IJPoint::Property()
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
      nl->TextAtom("(" + Instant::BasicType() + " " + JPoint::BasicType() +
                   "), describes the position of mjpoint at the time instant."),
      nl->StringAtom(Example())));
}


/*
1.1 Other Operations

*/

string IJPoint::Example()
{
  return "((instant 0.5) "+ JPoint::Example() + ")";
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const IJPoint& jp)
{
  jp.Print(os);
  return os;
}

