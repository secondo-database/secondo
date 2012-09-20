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

#include "ListUtils.h"
#include "Symbols.h"
#include <limits>
#include "NetDistanceGroup.h"

/*
1 Implementation of ~NetDistanceGroup~

1.1 Constructors and Deconstructor

The default Constructor should not been used outside the cast-Function.

*/

NetDistanceGroup::NetDistanceGroup():Attribute()
{}

NetDistanceGroup::NetDistanceGroup(const bool def) :
  Attribute(def), source(0), target(0), nextJunction(0), nextSection(0),
  netdistance(numeric_limits<double>::max())
{}

NetDistanceGroup::NetDistanceGroup ( const NetDistanceGroup& other ) :
  Attribute(other.IsDefined()), source(0), target(0), nextJunction(0),
  nextSection(0), netdistance(numeric_limits<double>::max())
{
  if (other.IsDefined())
  {
    source = other.GetSource();
    target = other.GetTarget();
    nextJunction = other.GetNextJunction();
    nextSection = other.GetNextSection();
    netdistance = other.GetNetdistance();
  }
  assert(source >= 0 && target >= 0 && nextJunction >= 0 && nextSection >= 0 &&
         netdistance >= 0.0);
}

NetDistanceGroup::NetDistanceGroup(const int sourc,
                                   const int targe,
                                   const int nextJunct,
                                   const int nextSect,
                                   const double netdist) :
  Attribute(true), source(sourc), target(targe), nextJunction(nextJunct),
  nextSection(nextSect), netdistance(netdist)
{
  assert(source >= 0 && target >= 0 && nextJunction >= 0 && nextSection >= 0 &&
         netdistance >= 0.0);
}

NetDistanceGroup::~NetDistanceGroup()
{}

/*
1.1.1 Getter and Setter for private Attributes

*/

int NetDistanceGroup::GetSource() const
{
  return source;
}

int NetDistanceGroup::GetTarget() const
{
  return target;
}

double NetDistanceGroup::GetNetdistance() const
{
  return netdistance;
}

int NetDistanceGroup::GetNextSection() const
{
  return nextSection;
}

int NetDistanceGroup::GetNextJunction() const
{
  return nextJunction;
}

void NetDistanceGroup::SetSource(const int t)
{
  assert(t >= 0);
  source = t;
}


void NetDistanceGroup::SetTarget(const int t)
{
  assert(t >= 0);
  target = t;
}

void NetDistanceGroup::SetNetdistance(const double dist)
{
  assert(dist >= 0.0);
  netdistance = dist;
}

void NetDistanceGroup::SetNextSection(const int t)
{
  assert(t >= 0);
  nextSection = t;
}

void NetDistanceGroup::SetNextJunction(const int t)
{
  assert(t >= 0);
  nextJunction = t;
}

/*
1.1.1 Overwrite Methods of Attribute

*/

void NetDistanceGroup::CopyFrom ( const Attribute* right )
{
  *this = *((NetDistanceGroup*) right);
}

Attribute::StorageType NetDistanceGroup::GetStorageType() const
{
  return Default;
}

size_t NetDistanceGroup::HashValue() const
{
  return (size_t) (source + target + nextJunction + nextSection) +
         (size_t) netdistance;
}

NetDistanceGroup* NetDistanceGroup::Clone() const
{
  return new NetDistanceGroup(*this);
}

bool NetDistanceGroup::Adjacent ( const Attribute* attrib ) const
{
  return false;
}

int NetDistanceGroup::Compare(const void* ls, const void* rs)
{
  NetDistanceGroup lhs(*((NetDistanceGroup*) ls));
  NetDistanceGroup rhs(*((NetDistanceGroup*) rs));
  return lhs.Compare(rhs);
}

int NetDistanceGroup::Compare ( const Attribute* rhs ) const
{
  NetDistanceGroup in(*(NetDistanceGroup*) rhs);
  return Compare(*((NetDistanceGroup*) rhs));
}

int NetDistanceGroup::Compare (const NetDistanceGroup& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  if (source < rhs.GetSource()) return -1;
  if (source > rhs.GetSource()) return 1;
  if (target < rhs.GetTarget()) return -1;
  if (target > rhs.GetTarget()) return 1;
  if (netdistance < rhs.GetNetdistance()) return -1;
  if (netdistance > rhs.GetNetdistance()) return 1;
  if (nextJunction < rhs.GetNextJunction()) return -1;
  if (nextJunction > rhs.GetNextJunction()) return 1;
  if (nextSection < rhs.GetNextSection()) return -1;
  if (nextSection > rhs.GetNextSection()) return 1;
  return 0;
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
    os << "source:" << source;
    os << "target: " << target;
    os << ", netdistance: " << netdistance;
    os << ", path over section: " << nextSection;
    os << ", to junction: " << nextJunction;
  }
  else
    os << Symbol::UNDEFINED();
  os << endl;
  return os;
}

const string NetDistanceGroup::BasicType()
{
  return "ndg";
}

const bool NetDistanceGroup::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1.1 Standard Operators

*/

NetDistanceGroup& NetDistanceGroup::operator= ( const NetDistanceGroup& other )
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    source = other.GetSource();
    target = other.GetTarget();
    nextJunction = other.GetNextJunction();
    nextSection = other.GetNextSection();
    netdistance = other.GetNetdistance();
  }
  return *this;
}

bool NetDistanceGroup::operator== ( const NetDistanceGroup& other ) const
{
  return (Compare(other) == 0);
}

bool NetDistanceGroup::operator!= ( const NetDistanceGroup& other ) const
{
  return (Compare(other) != 0);
}


bool NetDistanceGroup::operator< ( const NetDistanceGroup& other ) const
{
  return (Compare(other) < 0);
}

bool NetDistanceGroup::operator<= ( const NetDistanceGroup& other ) const
{
  return (Compare(other) < 1);
}

bool NetDistanceGroup::operator>= ( const NetDistanceGroup& other ) const
{
  return (Compare(other) > -1);
}

bool NetDistanceGroup::operator> ( const NetDistanceGroup& other ) const
{
  return (Compare(other) > 0);
}



/*
1.1.1 SecondoIntegration

*/

ListExpr NetDistanceGroup::Out(ListExpr typeInfo, Word value)
{
  NetDistanceGroup* obj = (NetDistanceGroup*) value.addr;
  if (obj->IsDefined())
  {
    int so = obj->GetSource();
    ListExpr leso = nl->IntAtom(so);
    int t = obj->GetTarget();
    ListExpr let = nl->IntAtom(t);
    int j = obj->GetNextJunction();
    ListExpr lej = nl->IntAtom(j);
    int s = obj->GetNextSection();
    ListExpr les = nl->IntAtom(s);
    double d = obj->GetNetdistance();
    ListExpr led = nl->RealAtom(d);
    return nl->FiveElemList(leso, let, lej, les, led);
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

  if (nl->ListLength(instance) == 1)
  {
    correct = true;
    return SetWord(new NetDistanceGroup(false));
  }
  else
  {
    if(nl->ListLength(instance) == 5)
    {
      ListExpr leso = nl->First(instance);
      ListExpr let = nl->Second(instance);
      ListExpr lej = nl->Third(instance);
      ListExpr les = nl->Fourth(instance);
      ListExpr led = nl->Fifth(instance);
      int so, t, j, s;
      double d;

      if (nl->IsAtom(leso) && nl->AtomType(leso) == IntType &&
          nl->IntValue(leso) >= 0)
      {
        so = nl->IntValue(leso);

        if (nl->IsAtom(let) && nl->AtomType(let) == IntType &&
            nl->IntValue(let) >= 0)
        {
          t = nl->IntValue(let);
          if (nl->IsAtom(lej) && nl->AtomType(lej) == IntType &&
              nl->IntValue(lej) >= 0)
          {
            j = nl->IntValue(lej);
            if (nl->IsAtom(les) && nl->AtomType(les) == IntType &&
                nl->IntValue(les) >= 0)
            {
              s = nl->IntValue(les);
              if (nl->IsAtom(led) && nl->AtomType(led) == RealType &&
                  nl->RealValue(led) >= 0.0)
              {
                d = nl->RealValue(led);
                NetDistanceGroup* res = new NetDistanceGroup(so, t, j, s, d);
                correct = true;
                return SetWord(res);
              }
              else
              {
                correct = false;
                cmsg.inFunError("5.Element must be " + CcReal::BasicType() +
                                " >= 0.");
                return SetWord(Address(0));
              }
            }
            else
            {
              correct = false;
              cmsg.inFunError("4.Element must be " + CcInt::BasicType()+
                              " >= 0.");
              return SetWord(Address(0));
            }
          }
          else
          {
            correct = false;
            cmsg.inFunError("3.Element must be " + CcInt::BasicType()+
                            " >= 0.");
            return SetWord(Address(0));
          }
        }
        else
        {
          correct = false;
          cmsg.inFunError("2.Element must be " + CcInt::BasicType()+
                          " >= 0.");
          return SetWord(Address(0));
        }
      }
      else
      {
        correct = false;
        cmsg.inFunError("1.Element must be " + CcInt::BasicType() +
                        " >= 0.");
        return SetWord(Address(0));
      }
    }
    correct = false;
    cmsg.inFunError("List of length 5 or 1 expected.");
    return SetWord(Address(0));
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
      nl->TextAtom("("+ CcInt::BasicType() + CcInt::BasicType() + " " +
            CcInt::BasicType() + " " +  CcInt::BasicType() + " " +
            CcReal::BasicType() + "), identifying source junction, target "+
            " junction, next junction and next section on the way to target " +
            "and network distance from source to target junction."),
      nl->TextAtom(Example())));
}

/*
1.1.1 Helpful Operators

*/

string NetDistanceGroup::Example()
{
  return "(34 25 64 18.5)";
}

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const NetDistanceGroup& ndg)
{
  ndg.Print(os);
  return os;
}
