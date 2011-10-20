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

2011 May Simone Jandt

*/

#include "PairTIDRLoc.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "Symbols.h"

/*
1. Implementation of PairTIDRLoc

1.1 Constructors and deconstructor

The default constructor should only be used in the cast-Function.

*/

PairTIDRLoc::PairTIDRLoc():Attribute()
{}

PairTIDRLoc::PairTIDRLoc(const PairTIDRLoc& other) :
  Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    tid = other.GetTID();
    rloc = other.GetRouteLocation();
  }
}

PairTIDRLoc::PairTIDRLoc(const bool def) : Attribute(def), tid(def), rloc(def)
{}

PairTIDRLoc::PairTIDRLoc(const TupleIdentifier& t, const RouteLocation& rl) :
  Attribute (true), tid (t), rloc(rl)
{}

PairTIDRLoc::~PairTIDRLoc()
{}

/*
1.2 Getter and Setter for private Attributes

*/

TupleIdentifier PairTIDRLoc::GetTID() const
{
  return tid;
}

TupleId PairTIDRLoc::GetTid() const
{
  return tid.GetTid();
}

RouteLocation PairTIDRLoc::GetRouteLocation() const
{
  return rloc;
}

void PairTIDRLoc::SetTID(const TupleId& t)
{
  tid.SetTid(t);
}

void PairTIDRLoc::SetTID(const TupleIdentifier& t)
{
  tid = t;
}

void PairTIDRLoc::SetRouteLocation(const RouteLocation& rl)
{
  rloc = rl;
}

/*
1.3 Override Methods from Attribute

*/

void PairTIDRLoc::CopyFrom(const Attribute* right)
{
  PairTIDRLoc* in = (PairTIDRLoc*) right;
  SetDefined(in->IsDefined());
  if (in->IsDefined())
  {
    tid = in->GetTID();
    rloc = in->GetRouteLocation();
  }
}

Attribute::StorageType PairTIDRLoc::GetStorageType() const
{
  return Default;
}

size_t PairTIDRLoc::HashValue() const
{
  return  tid.HashValue() + rloc.HashValue();
}

Attribute* PairTIDRLoc::Clone() const
{
  return new PairTIDRLoc(*this);
}

bool PairTIDRLoc::Adjacent(const Attribute* attrib) const
{
  return false;
}

int PairTIDRLoc::Compare(const Attribute* rhs) const
{
  PairTIDRLoc* in = (PairTIDRLoc*) rhs;
  return Compare(*in);
}

int PairTIDRLoc::Compare(const PairTIDRLoc& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  int res = tid.Compare(rhs.GetTID());
  if (res != 0) return res;
  else return rloc.Compare(rhs.GetRouteLocation());
}

size_t PairTIDRLoc::Sizeof() const
{
  return sizeof(PairTIDRLoc);
}

ostream& PairTIDRLoc::Print(ostream& os) const
{
  os << "Pair ";
  if (IsDefined())
  {
    os << "of TupleId: ";
    tid.Print(os);
    os << " and RouteLocation: ";
    rloc.Print(os);
  }
  else
  {
    os << Symbol::UNDEFINED() << endl;
  }
  return os;
}

const string PairTIDRLoc::BasicType()
{
  return "pairtidrloc";
}

const bool PairTIDRLoc::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Methods

*/

PairTIDRLoc& PairTIDRLoc::operator=(const PairTIDRLoc& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    tid = other.GetTID();
    rloc = other.GetRouteLocation();
  }
  return *this;
}

bool PairTIDRLoc::operator==(const PairTIDRLoc& other) const
{
  return (Compare(other)==0);
}

/*
1.5 Operators for Secondo Integration

*/

ListExpr PairTIDRLoc::Out(ListExpr typeInfo, Word value)
{
  PairTIDRLoc* obj = (PairTIDRLoc*) value.addr;
  if (!obj->IsDefined()) return nl->SymbolAtom(Symbol::UNDEFINED());
  else
  {
    TupleIdentifier t = obj->GetTID();
    Word wt = SetWord(&t);
    ListExpr tl = TupleIdentifier::Out(nl->TheEmptyList(), wt);
    RouteLocation r = obj->GetRouteLocation();
    Word wr = SetWord(&r);
    ListExpr rl = RouteLocation::Out(nl->TheEmptyList(), wr);
    return nl->TwoElemList(tl, rl);
  }
}

Word PairTIDRLoc::In(const ListExpr typeInfo, const ListExpr instance,
                     const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if (nl->IsAtom(instance) && nl->AtomType(instance) == SymbolType &&
      nl->IsEqual(instance, Symbol::UNDEFINED()))
  {
    correct = true;
    return (new PairTIDRLoc(false));
  }
  else
  {
    if (nl->ListLength(instance) == 2)
    {
      ListExpr tl = nl->First(instance);
      Word wt = TupleIdentifier::In(nl->TheEmptyList(), tl, errorPos, errorInfo,
                                    correct);
      if (correct)
      {
        ListExpr rl = nl->Second(instance);
        Word wr = RouteLocation::In(nl->TheEmptyList(), rl, errorPos, errorInfo,
                                    correct);
        if (correct)
        {
          TupleIdentifier* t= (TupleIdentifier*) wt.addr;
          RouteLocation* r = (RouteLocation*) wr.addr;
          PairTIDRLoc* res = new PairTIDRLoc(*t,*r);
          t->DeleteIfAllowed();
          r->DeleteIfAllowed();
          return SetWord(res);
        }
        else
        {
          correct = false;
          cmsg.inFunError("Second element must be " +
            RouteLocation::BasicType());
          return (SetWord(Address(0)));
        }
      }
      else
      {
        correct = false;
        cmsg.inFunError("First element must be " +
          TupleIdentifier::BasicType());
        return (SetWord(Address(0)));
      }
    }
    else
    {
      correct = false;
      cmsg.inFunError("ListLength must be one or two.");
      return (SetWord(Address(0)));
    }
  }
}

Word PairTIDRLoc::Create(const ListExpr typeInfo)
{
  return SetWord(new PairTIDRLoc(false));
}

void PairTIDRLoc::Delete( const ListExpr typeInfo, Word& w )
{
  ((PairTIDRLoc*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void PairTIDRLoc::Close( const ListExpr typeInfo, Word& w )
{
  ((PairTIDRLoc*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word PairTIDRLoc::Clone( const ListExpr typeInfo, const Word& w )
{
  return new PairTIDRLoc(*((PairTIDRLoc*) w.addr));
}

void* PairTIDRLoc::Cast( void* addr )
{
  return (new (addr) PairTIDRLoc);
}

bool PairTIDRLoc::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int PairTIDRLoc::SizeOf()
{
  return sizeof(PairTIDRLoc);
}

bool PairTIDRLoc::Save(SmiRecord& valueRecord, size_t& offset,
                       const ListExpr typeInfo, Word& value )
{
  PairTIDRLoc* obj = (PairTIDRLoc*) value.addr;
  TupleIdentifier t = obj->GetTID();
  Word wt = SetWord(&t);
  bool ok =
    TupleIdentifier::Save(valueRecord, offset, nl->TheEmptyList(), wt);
  RouteLocation r = obj->GetRouteLocation();
  Word wr = SetWord(&r);
  ok = ok && RouteLocation::Save(valueRecord, offset, nl->TheEmptyList(), wr);
  return ok;
}

bool PairTIDRLoc::Open(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value )
{
  Word wt;
  if (TupleIdentifier::Open(valueRecord, offset, nl->TheEmptyList(), wt))
  {
    TupleIdentifier* t = (TupleIdentifier*) wt.addr;
    Word wr;
    if (RouteLocation::Open(valueRecord, offset, nl->TheEmptyList(), wr))
    {
      RouteLocation* r = (RouteLocation*) wr.addr;
      value = SetWord(new PairTIDRLoc(*t,*r));
      return true;
    }
  }
  value = SetWord(Address(0));
  return false;
}

ListExpr PairTIDRLoc::Property()
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
      nl->TextAtom("("+ TupleIdentifier::BasicType() + " " +
                     RouteLocation::BasicType()+ "), pair connecting the tuple"+
                     " of the junctions relation with an position on a route."),
      nl->StringAtom("(34 (1 5.6 Up))")));
}
