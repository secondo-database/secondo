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

#include "PairTIDRInterval.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "Symbols.h"

/*
1. Implementation of PairTIDRInteval

1.1 Constructors and deconstructor

The default constructor should only be used in the cast-Function.

*/

PairTIDRInterval::PairTIDRInterval():Attribute()
{}

PairTIDRInterval::PairTIDRInterval(const PairTIDRInterval& other) :
  Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    tid = other.GetTID();
    rint = other.GetRouteInterval();
  }
}

PairTIDRInterval::PairTIDRInterval(const bool def) :
  Attribute(def), tid(def), rint(def)
{}

PairTIDRInterval::PairTIDRInterval(const TupleIdentifier& t,
                                   const JRouteInterval& ri) :
  Attribute (true), tid (t), rint(ri)
{}

PairTIDRInterval::~PairTIDRInterval()
{}

/*
1.2 Getter and Setter for private Attributes

*/

TupleIdentifier PairTIDRInterval::GetTID() const
{
  return tid;
}

TupleId PairTIDRInterval::GetTid() const
{
  return tid.GetTid();
}


JRouteInterval PairTIDRInterval::GetRouteInterval() const
{
  return rint;
}

void PairTIDRInterval::SetTID(const TupleIdentifier& t)
{
  tid = t;
}

void PairTIDRInterval::SetTID(const TupleId& t)
{
  tid.SetTid(t);
}


void PairTIDRInterval::SetRouteInterval(const JRouteInterval& ri)
{
  rint = ri;
}

/*
1.3 Override Methods from Attribute

*/

void PairTIDRInterval::CopyFrom(const Attribute* right)
{
  PairTIDRInterval* in = (PairTIDRInterval*) right;
  SetDefined(in->IsDefined());
  if (in->IsDefined())
  {
    tid = in->GetTID();
    rint = in->GetRouteInterval();
  }
}

Attribute::StorageType PairTIDRInterval::GetStorageType() const
{
  return Default;
}

size_t PairTIDRInterval::HashValue() const
{
  return (tid.HashValue() + rint.HashValue());
}

Attribute* PairTIDRInterval::Clone() const
{
  return new PairTIDRInterval(*this);
}

bool PairTIDRInterval::Adjacent(const Attribute* attrib) const
{
  return false;
}

int PairTIDRInterval::Compare(const Attribute* rhs) const
{
  PairTIDRInterval* in = (PairTIDRInterval*) rhs;
  return Compare(*in);
}

int PairTIDRInterval::Compare(const PairTIDRInterval& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  int res = tid.Compare (rhs.GetTID());
  if (res != 0) return res;
  else return rint.Compare(rhs.GetRouteInterval());
}

size_t PairTIDRInterval::Sizeof() const
{
  return sizeof(PairTIDRInterval);
}

ostream& PairTIDRInterval::Print(ostream& os) const
{
  os << "Pair ";
  if (IsDefined())
  {
    os << "of TupleId: ";
    tid.Print(os);
    os << " and RouteInterval: ";
    rint.Print(os);
  }
  else
  {
    os << Symbol::UNDEFINED() << endl;
  }
  return os;
}

const string PairTIDRInterval::BasicType()
{
  return "pairtidrint";
}

const bool PairTIDRInterval::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}
/*
1.4 Standard Methods

*/

PairTIDRInterval& PairTIDRInterval::operator=(const PairTIDRInterval& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    tid = other.GetTID();
    rint = other.GetRouteInterval();
  }
  return *this;
}

bool PairTIDRInterval::operator==(const PairTIDRInterval& other) const
{
  return (Compare(other)==0);
}

/*
1.5 Operators for Secondo Integration

*/

ListExpr PairTIDRInterval::Out(ListExpr typeInfo, Word value)
{
  PairTIDRInterval* obj = (PairTIDRInterval*) value.addr;
  if (!obj->IsDefined()) return nl->SymbolAtom(Symbol::UNDEFINED());
  else
  {
    TupleIdentifier t = obj->GetTID();
    Word wt = SetWord(&t);
    ListExpr tl = TupleIdentifier::Out(nl->TheEmptyList(), wt);
    JRouteInterval r = obj->GetRouteInterval();
    Word wr = SetWord(&r);
    ListExpr rl = JRouteInterval::Out(nl->TheEmptyList(), wr);
    return nl->TwoElemList(tl,rl);
  }
}

Word PairTIDRInterval::In(const ListExpr typeInfo, const ListExpr instance,
                     const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if (nl->IsAtom(instance) && nl->AtomType(instance) == SymbolType &&
    nl->IsEqual(instance, Symbol::UNDEFINED()))
  {
    correct = true;
    return (new PairTIDRInterval(false));
  }
  else
  {
    if (nl->ListLength(instance) == 2)
    {
      ListExpr tl = nl->First(instance);
      Word wt = TupleIdentifier::In(nl->TheEmptyList(), tl,
                                    errorPos, errorInfo, correct);
      if (correct)
      {
        TupleIdentifier* t = (TupleIdentifier*) wt.addr;
        ListExpr rl = nl->Second(instance);
        Word wr = JRouteInterval::In(nl->TheEmptyList(), rl, errorPos,
                                     errorInfo, correct);
        if (correct)
        {
          JRouteInterval* r = (JRouteInterval*) wr.addr;
          PairTIDRInterval* res = new PairTIDRInterval(*t,*r);
          r->DeleteIfAllowed();
          t->DeleteIfAllowed();
          return SetWord(res);
        }
        else
        {
          correct = false;
          cmsg.inFunError("Second element must be " +
            JRouteInterval::BasicType());
          t->DeleteIfAllowed();
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
      cmsg.inFunError("List should be (" + TupleIdentifier::BasicType() + " " +
        JRouteInterval::BasicType() + ").");
      return (SetWord(Address(0)));
    }
  }
}

Word PairTIDRInterval::Create(const ListExpr typeInfo)
{
  return SetWord(new PairTIDRInterval(false));
}

void PairTIDRInterval::Delete( const ListExpr typeInfo, Word& w )
{
  ((PairTIDRInterval*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void PairTIDRInterval::Close( const ListExpr typeInfo, Word& w )
{
  ((PairTIDRInterval*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word PairTIDRInterval::Clone( const ListExpr typeInfo, const Word& w )
{
  return new PairTIDRInterval(*((PairTIDRInterval*) w.addr));
}
void* PairTIDRInterval::Cast( void* addr )
{
  return (new (addr) PairTIDRInterval);
}

bool PairTIDRInterval::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int PairTIDRInterval::SizeOf()
{
  return sizeof(PairTIDRInterval);
}

bool PairTIDRInterval::Save(SmiRecord& valueRecord, size_t& offset,
                       const ListExpr typeInfo, Word& value )
{
  PairTIDRInterval* obj = (PairTIDRInterval*) value.addr;
  TupleIdentifier t = obj->GetTID();
  Word wt = SetWord(&t);
  bool ok =
      TupleIdentifier::Save(valueRecord, offset,
                            nl->TheEmptyList(), wt);
  JRouteInterval r = obj->GetRouteInterval();
  Word wr = SetWord(&r);
  ok = ok && JRouteInterval::Save(valueRecord, offset,
                                  nl->TheEmptyList(), wr);
  return ok;
}

bool PairTIDRInterval::Open(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value )
{
  Word wt;
  if (TupleIdentifier::Open(valueRecord, offset,
                           nl->TheEmptyList(), wt))
  {
    TupleIdentifier* t = (TupleIdentifier*) wt.addr;
    Word wr;
    if (JRouteInterval::Open(valueRecord, offset,
                             nl->TheEmptyList(), wr))
    {
      JRouteInterval* r = (JRouteInterval*) wr.addr;
      value = SetWord(new PairTIDRInterval(*t,*r));
      return true;
    }
  }
  value = SetWord(Address(0));
  return false;
}

ListExpr PairTIDRInterval::Property()
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
                    JRouteInterval::BasicType()+ "), pair connecting the tuple"+
                   " of the sections relation with an part of a route."),
      nl->StringAtom("(34 (1 5.6 15.5 Down))")));
}
