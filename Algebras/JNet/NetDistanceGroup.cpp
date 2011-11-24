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

2011, May Simone Jandt

*/

#include "NetDistanceGroup.h"
#include "ListUtils.h"
#include "Symbols.h"

/*
1. Implementation of ~NetDistanceGroup~

1.1 Constructors and Deconstructor

The default Constructor should not been used outside the cast-Function.

*/

NetDistanceGroup::NetDistanceGroup():Attribute()
{}

NetDistanceGroup::NetDistanceGroup(const bool def) : Attribute(def)
{}

NetDistanceGroup::NetDistanceGroup ( const NetDistanceGroup& other ) :
  Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    targetTID = other.GetTargetTID();
    nextSectionTID = other.GetNextSectionTID();
    nextJunctionTID = other.GetNextJunctionTID();
    netdistance = other.GetNetdistance();
  }
}

NetDistanceGroup::NetDistanceGroup(const TupleIdentifier target,
                                   const TupleIdentifier nextSect,
                                   const TupleIdentifier nextJunc,
                                   const double netdist) :
  Attribute(true), targetTID(target), nextSectionTID(nextSect),
  nextJunctionTID(nextJunc), netdistance(netdist)
{}

NetDistanceGroup::~NetDistanceGroup()
{}

/*
1.2 Getter and Setter for private Attributes

*/

TupleIdentifier NetDistanceGroup::GetTargetTID() const
{
  return targetTID;
}

double NetDistanceGroup::GetNetdistance() const
{
  return netdistance;
}

TupleIdentifier NetDistanceGroup::GetNextSectionTID() const
{
  return nextSectionTID;
}

TupleIdentifier NetDistanceGroup::GetNextJunctionTID() const
{
  return nextJunctionTID;
}

void NetDistanceGroup::SetTargetTID(const TupleIdentifier t)
{
  targetTID = t;
}

void NetDistanceGroup::SetNetdistance(const double dist)
{
  netdistance = dist;
}

void NetDistanceGroup::SetNextSectionTID(const TupleIdentifier t)
{
  nextSectionTID = t;
}

void NetDistanceGroup::SetNextJunctionTID(const TupleIdentifier t)
{
  nextJunctionTID = t;
}

/*
1.3 Overriden Methods of Attribute

*/

void NetDistanceGroup::CopyFrom ( const Attribute* right )
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    NetDistanceGroup* in = (NetDistanceGroup*) right;
    targetTID = in->GetTargetTID();
    nextSectionTID = in->GetNextSectionTID();
    nextJunctionTID = in->GetNextJunctionTID();
    netdistance = in->GetNetdistance();
  }
}

Attribute::StorageType NetDistanceGroup::GetStorageType() const
{
  return Default;
}

size_t NetDistanceGroup::HashValue() const
{
  return targetTID.HashValue() + nextSectionTID.HashValue() +
         nextJunctionTID.HashValue() + (size_t) netdistance;
}

Attribute* NetDistanceGroup::Clone() const
{
  return new NetDistanceGroup(*this);
}

bool NetDistanceGroup::Adjacent ( const Attribute* attrib ) const
{
  return false;
}

int NetDistanceGroup::Compare ( const Attribute* rhs ) const
{
  NetDistanceGroup* in = (NetDistanceGroup*) rhs;
  return Compare(*in);
}

int NetDistanceGroup::Compare (const NetDistanceGroup& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  int res = targetTID.Compare(rhs.GetTargetTID());
  if (res != 0) return res;
  if (netdistance < rhs.GetNetdistance()) return -1;
  if (netdistance > rhs.GetNetdistance()) return 1;
  res = nextSectionTID.Compare(rhs.GetNextSectionTID());
  if (res != 0) return res;
  return nextJunctionTID.Compare(rhs.GetNextJunctionTID());
}

size_t NetDistanceGroup::Sizeof() const
{
  return sizeof(NetDistanceGroup);
}

ostream& NetDistanceGroup::Print ( ostream& os ) const
{
  os << "NetDistanceGroup: " << endl;
  if (IsDefined())
  {
    os << "targetTID: ";
    targetTID.Print(os);
    os << ", next section on path tid: ";
    nextSectionTID.Print(os);
    os << ", next junction on path tid: ";
    nextJunctionTID.Print(os);
    os << ", netdistance: " << netdistance;
  }
  else
    os << Symbol::UNDEFINED();
  os << endl;
  return os;
}

const string NetDistanceGroup::BasicType()
{
  return "netdistgrp";
}

const bool NetDistanceGroup::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Operators

*/

NetDistanceGroup& NetDistanceGroup::operator= ( const NetDistanceGroup& other )
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    targetTID = other.GetTargetTID();
    nextSectionTID = other.GetNextSectionTID();
    nextJunctionTID = other.GetNextJunctionTID();
    netdistance = other.GetNetdistance();
  }
  return *this;
}

bool NetDistanceGroup::operator== ( const NetDistanceGroup& other ) const
{
  return (Compare(other) == 0);
}

/*
1.5 SecondoIntegration

*/

ListExpr NetDistanceGroup::Out(ListExpr typeInfo, Word value)
{
  NetDistanceGroup* obj = (NetDistanceGroup*) value.addr;
  if (obj->IsDefined())
  {
    TupleIdentifier t = obj->GetTargetTID();
    Word wt = SetWord(&t);
    ListExpr target = TupleIdentifier::Out(nl->TheEmptyList(), wt);
    t = obj->GetNextSectionTID();
    wt = SetWord(&t);
    ListExpr nextSect = TupleIdentifier::Out(nl->TheEmptyList(), wt);
    t = obj->GetNextJunctionTID();
    wt = SetWord(&t);
    ListExpr nextJunction = TupleIdentifier::Out(nl->TheEmptyList(), wt);
    double distance = obj->GetNetdistance();
    ListExpr dist = nl->RealAtom(distance);
    return nl->FourElemList(target, nextSect, nextJunction, dist );
  }
  else
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
}

Word NetDistanceGroup::In(const ListExpr typeInfo, const ListExpr instance,
                          const int errorPos, ListExpr& errorInfo,
                          bool& correct)
{
  if (nl->ListLength(instance) == 1 && nl->IsAtom(instance) &&
      nl->SymbolValue(instance) == Symbol::UNDEFINED())
  {
    correct = true;
    return SetWord(new NetDistanceGroup(false));
  }
  else
  {
    if(nl->ListLength(instance) == 4)
    {
      correct = true;
      ListExpr target = nl->First(instance);
      Word wTarget = TupleIdentifier::In(nl->TheEmptyList(),
                                         target, errorPos, errorInfo, correct);
      if (correct)
      {
        TupleIdentifier* targ = (TupleIdentifier*) wTarget.addr;
        ListExpr nextSect = nl->Second(instance);
        Word wNextSect = TupleIdentifier::In(nl->TheEmptyList(),
                                             nextSect, errorPos, errorInfo,
                                             correct);
        if (correct)
        {
          TupleIdentifier* tNextSect = (TupleIdentifier*) wNextSect.addr;
          ListExpr nextJunction = nl->Third(instance);
          Word wNextJunction =
                      TupleIdentifier::In(nl->TheEmptyList(),
                                          nextJunction, errorPos,
                                          errorInfo, correct);
          if (correct)
          {
            TupleIdentifier* tNextJunc =
                            (TupleIdentifier*) wNextJunction.addr;
            ListExpr distance = nl->Fourth(instance);
            if (nl->IsAtom(distance) && nl->AtomType(distance) == RealType)
            {
              double ndis = nl->RealValue(distance);
              NetDistanceGroup* res = new NetDistanceGroup(*targ, *tNextSect,
                                                       *tNextJunc, ndis);
              targ->DeleteIfAllowed();
              tNextSect->DeleteIfAllowed();
              tNextJunc->DeleteIfAllowed();
              return SetWord(res);
            }
            else
            {
              targ->DeleteIfAllowed();
              tNextSect->DeleteIfAllowed();
              tNextJunc->DeleteIfAllowed();
              cmsg.inFunError("Fourth element must be " + CcReal::BasicType());
              return SetWord(Address(0));
            }
          }
          else
          {
            targ->DeleteIfAllowed();
            tNextSect->DeleteIfAllowed();
            cmsg.inFunError("Third element must be " +
              TupleIdentifier::BasicType());
            return SetWord(Address(0));
          }
        }
        else
        {
          targ->DeleteIfAllowed();
          cmsg.inFunError("Second element must be "  +
                        TupleIdentifier::BasicType());
          return SetWord(Address(0));
        }
      }
      else
      {
        cmsg.inFunError("First element must be "  +
          TupleIdentifier::BasicType());
        return SetWord(Address(0));
      }
    }
    else
    {
      correct = false;
      cmsg.inFunError("Expected list (" + TupleIdentifier::BasicType()  +
            TupleIdentifier::BasicType()  + TupleIdentifier::BasicType() +
            CcReal::BasicType()+")");
      return SetWord(Address(0));
    }
  }
}

Word NetDistanceGroup::Create(const ListExpr typeInfo)
{
  return SetWord(new NetDistanceGroup(false));
}

void NetDistanceGroup::Delete( const ListExpr typeInfo, Word& w )
{
  ((NetDistanceGroup*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

void NetDistanceGroup::Close( const ListExpr typeInfo, Word& w )
{
  ((NetDistanceGroup*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word NetDistanceGroup::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(new NetDistanceGroup(*((NetDistanceGroup*) w.addr)));
}

void* NetDistanceGroup::Cast( void* addr )
{
  return (new (addr) NetDistanceGroup);
}
bool NetDistanceGroup::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int NetDistanceGroup::SizeOf()
{
  return sizeof(NetDistanceGroup);
}

bool NetDistanceGroup::Save(SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value )
{
  NetDistanceGroup* toSave = (NetDistanceGroup*) value.addr;
  TupleIdentifier t = toSave->GetTargetTID();;
  Word wt = SetWord(&t);
  if (!TupleIdentifier::Save(valueRecord, offset,
                             nl->TheEmptyList(), wt))
    return false;
  t = toSave->GetNextSectionTID();
  wt = SetWord (&t);
  if (!TupleIdentifier::Save(valueRecord, offset,
                             nl->TheEmptyList(), wt))
    return false;
  t = toSave->GetNextJunctionTID();
  wt = SetWord (&t);
  if (!TupleIdentifier::Save(valueRecord, offset,
                             nl->TheEmptyList(), wt))
    return false;
  double dist = toSave->GetNetdistance();
  valueRecord.Write(&dist, sizeof(double), offset);
  offset += sizeof(double);
  return true;
}

bool NetDistanceGroup::Open(SmiRecord& valueRecord, size_t& offset,
                            const ListExpr typeInfo, Word& value )
{
  Word wt;
  if (TupleIdentifier::Open(valueRecord, offset,
                            nl->TheEmptyList(), wt))
  {
    TupleIdentifier* target = (TupleIdentifier*) wt.addr;
    if (TupleIdentifier::Open(valueRecord, offset,
                              nl->TheEmptyList(), wt))
    {
      TupleIdentifier* nextSect = (TupleIdentifier*) wt.addr;
      if (TupleIdentifier::Open(valueRecord, offset,
                                nl->TheEmptyList(), wt))
      {
        TupleIdentifier* nextJunct = (TupleIdentifier*) wt.addr;
        double dist = -1.0;
        valueRecord.Read(&dist, sizeof(double), offset);
        offset += sizeof(double);
        value = SetWord(new NetDistanceGroup(*target, *nextSect, *nextJunct,
                                             dist));
        return true;
      }
    }
  }
  value = SetWord(Address(0));
  return false;
}

ListExpr NetDistanceGroup::Property()
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
            TupleIdentifier::BasicType() + " " + TupleIdentifier::BasicType() +
            " " + CcReal::BasicType()+ "), identifying target junction, " +
            "next junction and next section on the way to target and network " +
            "distance to target node."),
      nl->TextAtom(Example())));
}

/*
1.6 Helpful Operators

*/

string NetDistanceGroup::Example()
{
  return "(34 25 64 18.5)";
}
