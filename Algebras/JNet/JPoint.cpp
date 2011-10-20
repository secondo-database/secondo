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

2011, August Simone Jandt

*/

#include "JPoint.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"


/*
1 Implementation of class ~JPoint~

1.1 Constructors and Deconstructors

The default constructor should only be used in the Cast-Function.

*/

JPoint::JPoint():Attribute()
{}

JPoint::JPoint(const bool def):Attribute(def),nid(""),npos(false)
{}

JPoint::JPoint(const JPoint& other):Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    nid = other.GetNetId();
    npos = other.GetPosition();
  }
}

JPoint::JPoint(const string& netId, const RouteLocation& rloc) :
  Attribute(true), nid(netId), npos(rloc)
{}

JPoint::~JPoint()
{}

/*
1.1 Getter and Setter for private Attributes

*/

string JPoint::GetNetId() const
{
  return nid;
}

RouteLocation JPoint::GetPosition() const
{
  return npos;
}

void JPoint::SetNetId(const string& netId)
{
  nid = netId;
}

void JPoint::SetPosition(const RouteLocation& rloc)
{
  npos = rloc;
}

/*
1.1 Override Methods from Attribute

*/

void JPoint::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    JPoint* in = (JPoint*) right;
    nid = in->GetNetId();
    npos = in->GetPosition();
  }
}

Attribute::StorageType JPoint::GetStorageType() const
{
  return Default;
}

size_t JPoint::HashValue() const
{
  return (size_t) nid.length() + npos.HashValue();
}

Attribute* JPoint::Clone() const
{
  return new JPoint(this);
}

bool JPoint::Adjacent(const Attribute* attrib) const
{
  if (attrib->IsDefined())
  {
    JPoint* in = (JPoint*) attrib;
    if (IsDefined() && (nid == in->GetNetId()))
    {
      return npos.Adjacent(in->GetPosition());
    }
  }
  return false;
}

int JPoint::Compare(const Attribute* rhs) const
{
  JPoint* in = (JPoint*) rhs;
  return Compare(*in);
}

int JPoint::Compare(const JPoint& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  int test = nid.compare(rhs.GetNetId());
  if (test != 0) return test;
  return npos.Compare(rhs.GetPosition());
}

size_t JPoint::Sizeof() const
{
  return sizeof(JPoint);
}

ostream& JPoint::Print(ostream& os) const
{
  os << "JPoint in Network: " << nid
     << " at: ";
  npos.Print(os);
  os << endl;
  return os;
}

const string JPoint::BasicType()
{
  return "jpoint";
}

const bool JPoint::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

JPoint& JPoint::operator=(const JPoint& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    nid = other.GetNetId();
    npos = other.GetPosition();
  }
  return *this;
}

bool JPoint::operator==(const JPoint& other) const
{
  return Compare(other) == 0;
}

/*
1.1 Operators for Secondo Integration

*/

ListExpr JPoint::Out(ListExpr typeInfo, Word value)
{
  JPoint* in = (JPoint*) value.addr;
  if (!in->IsDefined())
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  else
  {
    ListExpr netId = nl->StringAtom(in->GetNetId());
    RouteLocation netPos(in->GetPosition());
    return nl->TwoElemList(netId,
                           RouteLocation::Out(nl->TheEmptyList(),
                                              SetWord((void*) &netPos)));
  }
}

Word JPoint::In(const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo, bool& correct)
{
  NList inlist(instance);

  if (inlist.length() == 1 && inlist.isAtom() &&
      inlist.isEqual(Symbol::UNDEFINED()))
  {
    correct = true;
    return (new JPoint (false) );
  }
  else
  {
    if (inlist.length() == 2)
    {
      NList idList(inlist.first());
      NList posList(inlist.second());

      if (!(idList.isAtom() && idList.isString()))
      {
        correct = false;
        cmsg.inFunError("First element should be string atom.");
        return SetWord(Address(0));
      }
      string netId = idList.str();

      Word w = RouteLocation::In(nl->TheEmptyList(),
                                 posList.listExpr(),
                                 errorPos,
                                 errorInfo,
                                 correct);
      if (!correct)
      {
        cmsg.inFunError("Second Element must be RouteLocation");
        return SetWord(Address(0));
      }
      RouteLocation* rloc =(RouteLocation*) w.addr;
      JPoint* res = new JPoint(netId, *rloc);
      rloc->DeleteIfAllowed();
      return SetWord(res);
    }
  }
  correct = false;
  cmsg.inFunError("list length should be 1 or 2");;
  return SetWord(Address(0));
}

Word JPoint::Create(const ListExpr typeInfo)
{
  return SetWord(new JPoint(false));
}

void JPoint::Delete( const ListExpr typeInfo, Word& w )
{
  ((JPoint*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void JPoint::Close( const ListExpr typeInfo, Word& w )
{
  ((JPoint*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word JPoint::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(new JPoint(*(JPoint*) w.addr));
}

void* JPoint::Cast( void* addr )
{
  return (new (addr) JPoint);
}

bool JPoint::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int JPoint::SizeOf()
{
  return sizeof(JPoint);
}

bool JPoint::Save(SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value )
{
  JPoint* toSave = (JPoint*) value.addr;
  if (toSave->IsDefined())
  {
    Word w;
    w = SetWord(new CcString(true, toSave->GetNetId()));
    ListExpr idLE;
    nl->ReadFromString(CcString::BasicType(), idLE);
    ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
    bool ok = SaveAttribute<CcString>(valueRecord, offset, numId, w);

    RouteLocation* rloc = new RouteLocation(toSave->GetPosition());
    w = SetWord(rloc);
    ok = ok && RouteLocation::Save(valueRecord, offset, nl->TheEmptyList(), w);
    rloc->DeleteIfAllowed();
    return ok;
  }
  else
  {
    Word w;
    w.setAddr(new CcString(true,Symbol::UNDEFINED()));
    ListExpr idLE;
    nl->ReadFromString(CcString::BasicType(), idLE);
    ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
    if (!SaveAttribute<CcString>(valueRecord, offset, numId, w))
      return false;
    else
      return true;
  }
}

bool JPoint::Open(SmiRecord& valueRecord, size_t& offset,
                  const ListExpr typeInfo, Word& value )
{
  Word w;
  ListExpr idLE;
  nl->ReadFromString(CcString::BasicType(), idLE);
  ListExpr numId = SecondoSystem::GetCatalog()->NumericType(idLE);
  if (!OpenAttribute<CcString>(valueRecord, offset, numId, w))
    return false;
  string netId = ((CcString*)w.addr)->GetValue();
  if (netId == Symbol::UNDEFINED())
  {
    value = SetWord(new JPoint(false));
    return true;
  }

  if (RouteLocation::Open(valueRecord, offset, nl->TheEmptyList(), w))
  {
    RouteLocation rloc  = (*(RouteLocation*) w.addr);
    value = SetWord(new JPoint(netId, rloc));
    return true;
  }
  return false;
}

ListExpr JPoint::Property()
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
      nl->TextAtom("(" + CcString::BasicType() + " " +
                    RouteLocation::BasicType() + "), describes a position in " +
                    "the given network."),
      nl->StringAtom("(netname (1 5.8 Both))")));
}


/*
1.1 Other Operations

*/
