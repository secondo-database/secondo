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

2011, October Simone Jandt

*/

#ifndef PAIRTID_H
#define PAIRTID_H

#include <ostream>
#include "Attribute.h"
#include "../TupleIdentifier/TupleIdentifier.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "Symbols.h"
#include "RouteLocation.h"
#include "JRouteInterval.h"

/*
1. class ~PairTID~
Template class for Pairs of ~tupleid~ and some other element used to link
elements and relations.

*/

template<class Elem>
class PairTID : public Attribute
{

public:
/*
1.1 Constructors and deconstructor

The default constructor should only be used in the cast-Function.

*/

PairTID();
PairTID(const PairTID<Elem>& other) ;
PairTID(const bool def);
PairTID(const TupleIdentifier& t, const Elem& rl);

~PairTID();

/*
1.2 Getter and Setter for private Attributes

*/

TupleIdentifier GetTID() const;
TupleId GetTid() const;
Elem GetElement() const;
void SetTID(const TupleIdentifier& t);
void SetTID(const TupleId& t);
void SetElement(const Elem& rl);

/*
1.3 Override Methods from Attribute

*/

void CopyFrom(const Attribute* right);
StorageType GetStorageType() const;
size_t HashValue() const;
Attribute* Clone() const;
bool Adjacent(const Attribute* attrib) const;
int Compare(const Attribute* rhs) const;
int Compare(const PairTID<Elem>& rhs) const;
size_t Sizeof() const;
ostream& Print(ostream& os) const;
static const string BasicType();
static const bool checkType(const ListExpr type);
/*
1.4 Standard Methods

*/

PairTID<Elem>& operator=(const PairTID<Elem>& other);
bool operator==(const PairTID<Elem>& other) const;

/*
1.5 Operators for Secondo Integration

*/

static ListExpr Out(ListExpr typeInfo, Word value);
static Word In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct);
static Word Create(const ListExpr typeInfo);
static void Delete( const ListExpr typeInfo, Word& w );
static void Close( const ListExpr typeInfo, Word& w );
static Word Clone( const ListExpr typeInfo, const Word& w );
static void* Cast( void* addr );
static bool KindCheck( ListExpr type, ListExpr& errorInfo );
static int SizeOf();
static bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value );
static bool Open(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value );
static ListExpr Property();

/*
1.6 Helpful Operators

*/

private:

  TupleIdentifier tid;
  Elem elem;
};

typedef PairTID<RouteLocation> PairTIDRLoc;
typedef PairTID<JRouteInterval> PairTIDRInt;

/*
1. Implementation of Pair

1.1 Constructors and deconstructor

The default constructor should only be used in the cast-Function.

*/

template<class Elem>
PairTID<Elem>::PairTID():Attribute()
{}

template<class Elem>
PairTID<Elem>::PairTID(const PairTID<Elem>& other) :
Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    tid = other.GetTID();
    elem = other.GetElement();
  }
}

template<class Elem>
PairTID<Elem>::PairTID(const bool def) : Attribute(def), tid(def), elem(def)
{}

template<class Elem>
PairTID<Elem>::PairTID(const TupleIdentifier& t, const Elem& e) :
  Attribute (true), tid (t), elem(e)
{}

template<class Elem>
PairTID<Elem>::~PairTID()
{}

/*
1.2 Getter and Setter for private Attributes

*/

template<class Elem>
TupleIdentifier PairTID<Elem>::GetTID() const
{
  return tid;
}

template<class Elem>
TupleId PairTID<Elem>::GetTid() const
{
  return tid.GetTid();
}

template<class Elem>
Elem PairTID<Elem>::GetElement() const
{
  return elem;
}

template<class Elem>
void PairTID<Elem>::SetTID(const TupleId& t)
{
  tid.SetTid(t);
}

template<class Elem>
void PairTID<Elem>::SetTID(const TupleIdentifier& t)
{
  tid = t;
}

template<class Elem>
void PairTID<Elem>::SetElement(const Elem& e)
{
  elem = e;
}

/*
1.3 Override Methods from Attribute

*/

template<class Elem>
void PairTID<Elem>::CopyFrom(const Attribute* right)
{
  PairTID<Elem>* in = (PairTID<Elem>*) right;
  SetDefined(in->IsDefined());
  if (in->IsDefined())
  {
    tid = in->GetTID();
    elem = in->GetElement();
  }
}

template<class Elem>
Attribute::StorageType PairTID<Elem>::GetStorageType() const
{
  return Default;
}

template<class Elem>
size_t PairTID<Elem>::HashValue() const
{
  return  tid.HashValue() + elem.HashValue();
}

template<class Elem>
Attribute* PairTID<Elem>::Clone() const
{
  return new PairTID<Elem>(*this);
}

template<class Elem>
bool PairTID<Elem>::Adjacent(const Attribute* attrib) const
{
  return false;
}

template<class Elem>
int PairTID<Elem>::Compare(const Attribute* rhs) const
{
  PairTID<Elem>* in = (PairTID<Elem>*) rhs;
  return Compare(*in);
}

template<class Elem>
int PairTID<Elem>::Compare(const PairTID<Elem>& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  int res = tid.Compare(rhs.GetTID());
  if (res != 0) return res;
  else return elem.Compare(rhs.GetElement());
}

template<class Elem>
size_t PairTID<Elem>::Sizeof() const
{
  return sizeof(PairTID<Elem>);
}

template<class Elem>
ostream& PairTID<Elem>::Print(ostream& os) const
{
  os << "PairTID ";
  if (IsDefined())
  {
    os << "of TupleId: ";
    tid.Print(os);
    os << " and ";
    elem.Print(os);
  }
  else
  {
    os << Symbol::UNDEFINED() << endl;
  }
  return os;
}

template<class Elem>
const string PairTID<Elem>::BasicType()
{
  return ("pairtid" + Elem::BasicType());
}

template<class Elem>
const bool PairTID<Elem>::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Methods

*/

template<class Elem>
PairTID<Elem>& PairTID<Elem>::operator=(const PairTID<Elem>& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    tid = other.GetTID();
    elem = other.GetElement();
  }
  return *this;
}

template<class Elem>
bool PairTID<Elem>::operator==(const PairTID<Elem>& other) const
{
  return (Compare(other)==0);
}

/*
1 .5 Operators for Secondo Integration

*/

template<class Elem>
ListExpr PairTID<Elem>::Out(ListExpr typeInfo, Word value)
{
  PairTID<Elem>* obj = (PairTID<Elem>*) value.addr;
  if (!obj->IsDefined()) return nl->SymbolAtom(Symbol::UNDEFINED());
  else
  {
    TupleIdentifier t = obj->GetTID();
    Word wt = SetWord(&t);
    ListExpr tl = TupleIdentifier::Out(nl->TheEmptyList(), wt);
    Elem r = obj->GetElement();
    Word wr = SetWord(&r);
    ListExpr rl = Elem::Out(nl->TheEmptyList(), wr);
    return nl->TwoElemList(tl, rl);
  }
}

template<class Elem>
Word PairTID<Elem>::In(const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if (nl->IsAtom(instance) && nl->AtomType(instance) == SymbolType &&
    nl->IsEqual(instance, Symbol::UNDEFINED()))
  {
    correct = true;
    return (new PairTID<Elem>(false));
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
        Word wr = Elem::In(nl->TheEmptyList(), rl, errorPos, errorInfo,
                           correct);
        if (correct)
        {
          TupleIdentifier* t= (TupleIdentifier*) wt.addr;
          Elem* r = (Elem*) wr.addr;
          PairTID<Elem>* res = new PairTID<Elem>(*t,*r);
          t->DeleteIfAllowed();
          r->DeleteIfAllowed();
          return SetWord(res);
        }
        else
        {
          correct = false;
          cmsg.inFunError("Second element must be " +
          Elem::BasicType());
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

template<class Elem>
Word PairTID<Elem>::Create(const ListExpr typeInfo)
{
  return SetWord(new PairTID<Elem>(false));
}

template<class Elem>
void PairTID<Elem>::Delete( const ListExpr typeInfo, Word& w )
{
  ((PairTID<Elem>*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

template<class Elem>
void PairTID<Elem>::Close( const ListExpr typeInfo, Word& w )
{
  ((PairTID<Elem>*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

template<class Elem>
Word PairTID<Elem>::Clone( const ListExpr typeInfo, const Word& w )
{
  return new PairTID<Elem>(*((PairTID<Elem>*) w.addr));
}

template<class Elem>
void* PairTID<Elem>::Cast( void* addr )
{
  return (new (addr) PairTID<Elem>);
}

template<class Elem>
bool PairTID<Elem>::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

template<class Elem>
int PairTID<Elem>::SizeOf()
{
  return sizeof(PairTID<Elem>);
}

template<class Elem>
bool PairTID<Elem>::Save(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value )
{
  PairTID<Elem>* obj = (PairTID<Elem>*) value.addr;
  TupleIdentifier t = obj->GetTID();
  Word wt = SetWord(&t);
  bool ok =
    TupleIdentifier::Save(valueRecord, offset, nl->TheEmptyList(), wt);
    Elem r = obj->GetElement();
    Word wr = SetWord(&r);
    ok = ok && Elem::Save(valueRecord, offset, nl->TheEmptyList(), wr);
    return ok;
}

template<class Elem>
bool PairTID<Elem>::Open(SmiRecord& valueRecord, size_t& offset,
                         const ListExpr typeInfo, Word& value )
{
  Word wt;
  if (TupleIdentifier::Open(valueRecord, offset, nl->TheEmptyList(), wt))
  {
    TupleIdentifier* t = (TupleIdentifier*) wt.addr;
    Word wr;
    if (Elem::Open(valueRecord, offset, nl->TheEmptyList(), wr))
    {
      Elem* r = (Elem*) wr.addr;
      value = SetWord(new PairTID<Elem>(*t,*r));
      return true;
    }
  }
  value = SetWord(Address(0));
  return false;
}

template<class Elem>
ListExpr PairTID<Elem>::Property()
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
      nl->TextAtom("("+ TupleIdentifier::BasicType() + " " + Elem::BasicType()
      + "), pair connecting the tuples of an relation with an " +
      "network position or part."),
      nl->TextAtom("(34 (1 5.6 Up)) or (25 (6 5.8 546.6 Down))")));
}

#endif // PAIRTIDRLOC_H

