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

2012, August Simone Jandt

1 Includes

*/

#include "JPoint.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"
#include "ManageJNet.h"


/*
1 Implementation of class ~JPoint~

1.1 Constructors and Deconstructors

*/

JPoint::JPoint(): Attribute()
{}

JPoint::JPoint(const bool def) :
    Attribute(def), npos(def)
{}

JPoint::JPoint(const JPoint& other) :
    Attribute(other.IsDefined()), npos(other.GetLocation())
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
  }
}

JPoint::JPoint(const string netId, const RouteLocation& rloc) :
  Attribute(rloc.IsDefined()), npos(rloc)
{
  JNetwork* jnet = ManageJNet::GetNetwork(netId);
  if (jnet != 0){
    strcpy(nid, *jnet->GetId());
    SetDefined(jnet->Contains(&rloc));
    ManageJNet::CloseNetwork(jnet);
  }
  else
    SetDefined(false);
}

JPoint::JPoint(const JNetwork* jnet, const RouteLocation* rloc) :
  Attribute(true), npos(*rloc)
{
  if (!rloc->IsDefined() || !jnet->IsDefined() || !jnet->Contains(rloc))
    SetDefined(false);
  else
  {
    strcpy(nid, *jnet->GetId());
  }
}

JPoint::~JPoint()
{}

/*
1.1 Getter and Setter for private Attributes

*/

const STRING_T* JPoint::GetNetworkId() const
{
  return &nid;
}

RouteLocation JPoint::GetLocation() const
{
  return npos;
}

void JPoint::SetNetId(const STRING_T& netId)
{
  strcpy(nid, netId);
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
  *this = *((JPoint*) right);
}

Attribute::StorageType JPoint::GetStorageType() const
{
  return Default;
}

size_t JPoint::HashValue() const
{
  return strlen(nid) + npos.HashValue();
}

JPoint* JPoint::Clone() const
{
  return new JPoint(*this);
}

bool JPoint::Adjacent(const Attribute* attrib) const
{
  if (attrib->IsDefined())
  {
    JPoint in(*(JPoint*) attrib);
    if (IsDefined() && strcmp(nid, *in.GetNetworkId()) == 0)
    {
      return npos.Adjacent(in.GetLocation());
    }
  }
  return false;
}

int JPoint::Compare(const void* l, const void* r)
{
  JPoint lp(*(JPoint*) l);
  JPoint rp(*(JPoint*) r);
  return lp.Compare(rp);

}
int JPoint::Compare(const Attribute* rhs) const
{
  JPoint in(*(JPoint*) rhs);
  return Compare(in);
}

int JPoint::Compare(const JPoint& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  int test = strcmp(nid, *rhs.GetNetworkId());
  if (test != 0) return test;
  return npos.Compare(rhs.GetLocation());
}

size_t JPoint::Sizeof() const
{
  return sizeof(JPoint);
}

ostream& JPoint::Print(ostream& os) const
{
  os << "JPoint ";
  if (IsDefined())
  {
    os << "in Network: " << nid
       << " at: ";
    npos.Print(os);
    os << endl;
  }
  else
  {
    os << Symbol::UNDEFINED() << endl;
  }
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
    strcpy(nid, *other.GetNetworkId());
    npos = other.GetLocation();
  }
  return *this;
}

bool JPoint::operator==(const JPoint& other) const
{
  return (Compare(other) == 0);
}

bool JPoint::operator!=(const JPoint& other) const
{
  return (Compare(other) != 0);
}

bool JPoint::operator<(const JPoint& other) const
{
  return (Compare(other) < 0);
}

bool JPoint::operator<=(const JPoint& other) const
{
  return (Compare(other) < 1);
}

bool JPoint::operator>(const JPoint& other) const
{
  return (Compare(other) > 0);
}

bool JPoint::operator>=(const JPoint& other) const
{
  return (Compare(other) > -1);
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
    NList netList(*in->GetNetworkId(),true,false);
    RouteLocation netPos(in->GetLocation());
    return nl->TwoElemList(netList.listExpr(),
                           RouteLocation::Out(nl->TheEmptyList(),
                                              SetWord((void*) &netPos)));
  }
}

Word JPoint::In(const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo, bool& correct)
{
  NList inlist(instance);

  if (listutils::isSymbolUndefined( instance ))
  {
    correct = true;
    return (new JPoint (false) );
  }
  else
  {
    if (inlist.length() == 2)
    {
      ListExpr netList = nl->First(instance);
      string netId = nl->StringValue(netList);
      Word w = RouteLocation::In(nl->TheEmptyList(),
                                 nl->Second(instance),
                                 errorPos,
                                 errorInfo,
                                 correct);
      if (!correct)
      {
        cmsg.inFunError("Second Element must be RouteLocation");
        return SetWord(Address(0));
      }
      RouteLocation* rloc = (RouteLocation*) w.addr;
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
  return SetWord(new JPoint(true));
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
      nl->StringAtom(Example())));
}


/*
1.1 Other Operations

*/

string JPoint::Example()
{
  return "(netname " + RouteLocation::Example() + ")";
}

/*
1.1.1 FromSpatial

*/

void JPoint::FromSpatial(const JNetwork* jnet, const Point* in)
{
  SetDefined(in->IsDefined());
  if (jnet != NULL && jnet->IsDefined() &&
      in != NULL && in->IsDefined())
  {
    strcpy(nid,*(jnet->GetId()));
    RouteLocation* rloc = jnet->GetNetworkValueOf(in);
    if (rloc != NULL)
    {
      npos = *rloc;
      rloc->DeleteIfAllowed();
    }
    else
      SetDefined(false);
  }
  else
    SetDefined(false);
}

/*
1.1.1 ToSpatial

*/

void JPoint::ToSpatial(Point& result) const
{
  if (IsDefined())
  {
    result.SetDefined(true);
    JNetwork* jnet = ManageJNet::GetNetwork(nid);
    Point* tmp = jnet->GetSpatialValueOf(*this);
    result = *tmp;
    tmp->DeleteIfAllowed();
    ManageJNet::CloseNetwork(jnet);
  }
  else
    result.SetDefined(false);
}

/*
1.1.1 NetBox

*/

Rectangle< 2 > JPoint::NetBox() const
{
  if (IsDefined())
    return npos.NetBox();
  else
    return Rectangle<2>(false, 0.0, 0.0, 0.0, 0.0);
}

/*
1.1.1 OtherNetworkPositions

*/

JListRLoc* JPoint::OtherNetworkPositions() const
{
  JNetwork* jnet = ManageJNet::GetNetwork(nid);
  JListRLoc* result = jnet->GetNetworkValuesOf(npos);
  ManageJNet::CloseNetwork(jnet);
  return result;
}


/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JPoint& jp)
{
  jp.Print(os);
  return os;
}

