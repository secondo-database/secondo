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

#include "JLine.h"
#include "Symbols.h"
#include "StandardTypes.h"

/*
1. Class JLine

1.1 Constructors and Deconstructor

*/

JLine::JLine():Attribute()
{}

JLine::JLine(bool def):Attribute(def),routeintervals(0)
{}

JLine::JLine(const JLine& other):Attribute(other.IsDefined()),
                                 routeintervals(other.GetNoComponents())
{
  if (other.IsDefined())
  {
    strcpy(netId, other.GetNetworkId().c_str());
    routeintervals.copyFrom(other.routeintervals);
  }
}

JLine::~JLine()
{}

/*
1.1 Getter and Setter for private attributes

*/

string JLine::GetNetworkId() const
{
  return (string) netId;
}

DbArray<JRouteInterval>* JLine::GetRouteIntervals() const
{
  if ( IsDefined() )
  {
    DbArray<JRouteInterval>* res = new DbArray<JRouteInterval>(0);
    res->copyFrom(routeintervals);
    return res;
  }
  else return 0;
}

void JLine::SetNetworkId(string& nid)
{
  strcpy(netId, nid.c_str());
}

void JLine::SetRouteIntervals(DbArray<JRouteInterval>* setri)
{
  routeintervals.copyFrom(*setri);
}

/*
1.1 Override Methods from Attribute

*/

void JLine::CopyFrom(const Attribute* right)
{
  if (right->IsDefined())
  {
    JLine* in = (JLine*) right;
    string nid = in->GetNetworkId();
    in->SetNetworkId(nid);
    in->SetRouteIntervals(in->GetRouteIntervals());
  }
  else
    SetDefined(false);
}

size_t JLine::HashValue() const
{
  size_t res = 0;
  if (IsDefined())
  {
    res += (size_t) ((string)netId).length();
    JRouteInterval ri;
    for (int i = 0; i < routeintervals.Size(); i++)
    {
      Get(i,ri);
      res += ri.HashValue();
    }
  }
  return res;
}

Attribute* JLine::Clone() const
{
  return new JLine(*this);
}

bool JLine::Adjacent(const Attribute* attrib) const
{
  return false;
}

int JLine::Compare(const Attribute* rhs) const
{
  JLine* in = (JLine*) rhs;
  return Compare(in);
}

int JLine::Compare(const JLine* rhs) const
{
  if (!IsDefined() && !rhs->IsDefined()) return 0;
  if (!IsDefined() && rhs->IsDefined()) return -1;
  if (IsDefined() && !rhs->IsDefined()) return 1;
  int aux = ((string) netId).compare(rhs->GetNetworkId());
  if (aux != 0) return aux;
  if (routeintervals.Size() < rhs->GetNoComponents()) return -1;
  if (routeintervals.Size() > rhs->GetNoComponents()) return 1;
  JRouteInterval lri, rri;
  for (int i = 0; i < routeintervals.Size(); i++)
  {
    routeintervals.Get(i, lri);
    rhs->Get(i, rri);
    aux = lri.Compare(&rri);
    if (aux != 0) return aux;
  }
  return 0;
}

size_t JLine::Sizeof() const
{
  return sizeof(JLine);
}

int JLine::NumOfFLOBs() const
{
  return 1;
}

Flob* JLine::GetFLOB(const int i)
{
  if (i == 0) return &routeintervals;
  return 0;
}

ostream& JLine::Print(ostream& os) const
{
  os << "JLine: ";
  if(IsDefined())
  {
    os << " NetworkId: " << (string) netId << endl;
    JRouteInterval ri;
    for (int i = 0; i < routeintervals.Size(); i++)
    {
      routeintervals.Get(i,ri);
      ri.Print(os);
    }
  }
  else
    os << Symbol::UNDEFINED() << endl;
  os << endl;
  return os;
}

void JLine::Destroy()
{
  routeintervals.Destroy();
}

Attribute::StorageType JLine::GetStorageType() const
{
  return Default;
}

const std::string JLine::BasicType()
{
  return "jline";
}

const bool JLine::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

JLine& JLine::operator=(const JLine& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    strcpy(netId, other.GetNetworkId().c_str());
    routeintervals.copyFrom(*(other.GetRouteIntervals()));
  }
  return *this;
}

bool JLine::operator==(const JLine& other) const
{
  return Compare(&other) == 0;
}

/*
1.1 Operators for Secondo Integration

*/

ListExpr JLine::Out(ListExpr typeInfo, Word value)
{
  JLine* out =  (JLine*) value.addr;

  if (!out->IsDefined())
    return nl->SymbolAtom(Symbol::UNDEFINED());
  else
  {
    NList nid(out->GetNetworkId());

    NList rintList(nl->TheEmptyList());
    JRouteInterval ri;
    bool first = true;
    for (int i = 0; i < out->GetNoComponents(); i++)
    {
      out->Get(i,ri);
      NList actRIntList(JRouteInterval::Out(nl->TheEmptyList(),
                                            SetWord((void*) &ri)));
      if (first)
      {
        first = false;
        rintList = actRIntList.enclose();
      }
      else
        rintList.append(actRIntList);
    }
    ListExpr test = nl->TwoElemList(nid.listExpr(), rintList.listExpr());
    return test;
  }
}

Word JLine::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct=true;
    return SetWord(Address(new JLine(false)));
  }

  if (nl->ListLength(instance) != 2)
  {
    correct = false;
    cmsg.inFunError("List Length should be two.");
    return SetWord(Address(0));
  }

  ListExpr netList = nl->First(instance);

  if (!(nl->IsAtom(netList) && nl->AtomType(netList) == StringType))
  {
    correct = false;
    cmsg.inFunError("First element should be network name.");
    return SetWord(Address(0));
  }

  string nid = nl->StringValue(netList);
  ListExpr rintList = nl->Second(instance);

  if (nl->IsEmpty(rintList))
  {
    correct = true;
    JLine* res = new JLine(true);
    res->SetNetworkId(nid);
    res->SetRouteIntervals(new DbArray<JRouteInterval> (0));
    return SetWord(res);
  }

  ListExpr actRint = nl->TheEmptyList();
  correct = true;
  DbArray<JRouteInterval>* setri =
    new DbArray<JRouteInterval> (0);

  while( !nl->IsEmpty( rintList ) )
  {
    actRint = nl->First( rintList );
    rintList = nl->Rest( rintList );
    Word w = JRouteInterval::In(nl->TheEmptyList(), actRint, errorPos,
                                errorInfo, correct);
    if (correct)
    {
      JRouteInterval* actInt = (JRouteInterval*) w.addr;
      setri->Append(*actInt);
      actInt->DeleteIfAllowed();
      actInt = 0;
    }
    else
    {
      setri->Destroy();
      delete setri;
      setri = 0;
      cmsg.inFunError("Error in list of " + JRouteInterval::BasicType());
      return SetWord(Address(0));
    }
  }

  JLine* result = new JLine(true);
  result->SetNetworkId(nid);
  result->SetRouteIntervals(setri);
  setri->Destroy();
  delete setri;

  return SetWord(result);
}

Word JLine::Create(const ListExpr typeInfo)
{
  return SetWord(new JLine(false));
}

void JLine::Delete( const ListExpr typeInfo, Word& w )
{
  ((JLine*) w.addr)->Destroy();
  delete ((JLine*)w.addr);
  w.addr = 0;
}

void JLine::Close( const ListExpr typeInfo, Word& w )
{
  delete ((JLine*)w.addr);
  w.addr = 0;
}

Word JLine::Clone( const ListExpr typeInfo, const Word& w )
{
  return (new JLine(*((JLine*) w.addr)));
}

void* JLine::Cast( void* addr )
{
  return (new (addr) JLine);
}

bool JLine::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int JLine::SizeOf()
{
  return sizeof(JLine);
}

ListExpr JLine::Property()
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
      nl->TextAtom("("+ CcString::BasicType() + " ((" +
      JRouteInterval::BasicType() + ") ...("+ JRouteInterval::BasicType() +
      "))), part of network named by string represented by a list of route " +
      "intervals."),
      nl->TextAtom(Example())));
}


/*
1.1 Other helpful operators

*/

string JLine::Example()
{
  return "(netname ("+ JRouteInterval::Example() + "))";
}


int JLine::GetNoComponents() const
{
  return routeintervals.Size();
}

void JLine::Get(const int i, JRouteInterval& ri) const
{
  assert (0 <= i && i < routeintervals.Size());
  routeintervals.Get(i, ri);
}