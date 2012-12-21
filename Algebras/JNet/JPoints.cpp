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

1 Includes and defines

*/

#include "JPoints.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "JRLTree.h"
#include "ManageJNet.h"

using namespace jnetwork;

/*
1 Implementation of class JPoints

1.1 Constructors and Deconstructor

*/

JPoints::JPoints() : Attribute()
{}

JPoints::JPoints(const bool def) :
    Attribute(def), routelocations(0), sorted(false),
    activBulkload(false)
{}

JPoints::JPoints(const string netId, const DbArray<RouteLocation>& rlocList,
                 const bool check /*=true*/, const bool issorted /*=false*/) :
    Attribute(true), routelocations(0)
{
  if (check)
  {
    JNetwork* jnet = ManageJNet::GetNetwork(netId);
    strcpy(nid, *jnet->GetId());
    FillLocationList(rlocList, jnet);
    ManageJNet::CloseNetwork(jnet);
  }
  else
  {
    strcpy(nid, netId.c_str());
    routelocations.copyFrom(rlocList);
    if (!issorted)
      Sort();
  }
}

JPoints::JPoints(const JPoints& other) :
  Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    routelocations.copyFrom(other.routelocations);
    sorted = other.IsSorted();
    activBulkload = false;
  }
}

JPoints::~JPoints()
{}

/*
1.1 Getter and Setter for private attributes

*/

const STRING_T* JPoints::GetNetworkId() const
{
  return &nid;
}

const DbArray<RouteLocation>& JPoints::GetRouteLocations() const
{
  return routelocations;
}

void JPoints::SetNetworkId(const STRING_T& id)
{
  strcpy(nid, id);
}

void JPoints::SetRouteLocations(const DbArray<RouteLocation>& setri,
                                const bool check /*= true*/,
                                const bool issorted /*=false*/,
                                const JNetwork* jnet /*=0*/)
{
  routelocations.clean();
  sorted = issorted;
  if (check)
  {
    if(jnet != 0)
      FillLocationList(setri, jnet);
    else
    {
      JNetwork* j = ManageJNet::GetNetwork(nid);
      FillLocationList(setri, j);
      ManageJNet::CloseNetwork(j);
    }
  }
  else
  {
    routelocations.copyFrom(setri);
    if (!issorted)
      Sort();
  }
}

void JPoints::SetRouteLocations(const JListRLoc& rlocs,
                         const bool check /*=true*/,
                         const bool issorted /*=false*/,
                         const JNetwork* jnet /*=0*/)
{
  routelocations.clean();
  sorted = issorted;
  if (check)
  {
    if (jnet != 0)
      FillLocationList(rlocs.GetList(), jnet);
    else
    {
      JNetwork* j = ManageJNet::GetNetwork(nid);
      FillLocationList(rlocs.GetList(), j);
      ManageJNet::CloseNetwork(j);
    }
  }
  else
  {
    routelocations.copyFrom(rlocs.GetList());
    if (!issorted)
      Sort();
  }
}


/*
1.1 Override Methods from Attribute

*/

void JPoints::CopyFrom(const Attribute* right)
{
  *this = *((JPoints*) right);
}

size_t JPoints::HashValue() const
{
  size_t res = 0;
  if (IsDefined())
  {
    res += strlen(nid);
    RouteLocation ri(false);
    for (int i = 0; i < routelocations.Size(); i++)
    {
      Get(i,ri);
      res += ri.HashValue();
    }
  }
  return res;
}

JPoints* JPoints::Clone() const
{
  return new JPoints(*this);
}

bool JPoints::Adjacent(const Attribute* attrib) const
{
  return false;
}

int JPoints::Compare(const void* ls, const void* rs)
{
  JPoints lhs(*((JPoints*) ls));
  JPoints rhs(*((JPoints*) rs));
  return lhs.Compare(rhs);
}

int JPoints::Compare(const Attribute* rhs) const
{
  JPoints in(*((JPoints*) rhs));
  return Compare(in);
}

int JPoints::Compare(const JPoints& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  int aux = strcmp(nid, *rhs.GetNetworkId());
  if (aux != 0) return aux;
  if (routelocations.Size() < rhs.GetNoComponents()) return -1;
  if (routelocations.Size() > rhs.GetNoComponents()) return 1;
  RouteLocation lri(false);
  RouteLocation rri(false);
  for (int i = 0; i < routelocations.Size(); i++)
  {
    routelocations.Get(i, lri);
    rhs.Get(i, rri);
    aux = lri.Compare(&rri);
    if (aux != 0) return aux;
  }
  return 0;
}

size_t JPoints::Sizeof() const
{
  return sizeof(JPoints);
}

int JPoints::NumOfFLOBs() const
{
  return 1;
}

Flob* JPoints::GetFLOB(const int i)
{
  if (i == 0) return &routelocations;
  return 0;
}

ostream& JPoints::Print(ostream& os) const
{
  os << "JPoints: ";
  if(IsDefined())
  {
    os << " NetworkId: " << nid << endl;
    RouteLocation ri(false);
    for (int i = 0; i < routelocations.Size(); i++)
    {
      routelocations.Get(i,ri);
      ri.Print(os);
    }
  }
  else
    os << Symbol::UNDEFINED() << endl;
  return os;
}

void JPoints::Destroy()
{
  routelocations.Destroy();
}

Attribute::StorageType JPoints::GetStorageType() const
{
  return Default;
}

const string JPoints::BasicType()
{
  return "jpoints";
}

const bool JPoints::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

JPoints& JPoints::operator=(const JPoints& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    routelocations.copyFrom(other.GetRouteLocations());
    sorted = other.IsSorted();
  }
  return *this;
}

bool JPoints::operator==(const JPoints& other) const
{
  return Compare(&other) == 0;
}

bool JPoints::operator!=(const JPoints& other) const
{
  return Compare(&other) != 0;
}

bool JPoints::operator<(const JPoints& other) const
{
  return Compare(&other) < 0;
}

bool JPoints::operator<=(const JPoints& other) const
{
  return Compare(&other) < 1;
}

bool JPoints::operator>(const JPoints& other) const
{
  return Compare(&other) > 0;
}

bool JPoints::operator>=(const JPoints& other) const
{
  return Compare(&other) >= 0;
}

/*
1.1 Operators for Secondo Integration

*/

ListExpr JPoints::Out(ListExpr typeInfo, Word value)
{
  JPoints* out =  (JPoints*) value.addr;

  if (!out->IsDefined())
    return nl->SymbolAtom(Symbol::UNDEFINED());
  else
  {

    NList nList((string)*out->GetNetworkId(), true, false);


    NList rlocList(nl->TheEmptyList());
    RouteLocation rl(false);
    bool first = true;
    for (int i = 0; i < out->GetNoComponents(); i++)
    {
      out->Get(i,rl);
      NList actRLocList(RouteLocation::Out(nl->TheEmptyList(),
                                           SetWord((void*) &rl)));
      if (first)
      {
        first = false;
        rlocList = actRLocList.enclose();
      }
      else
        rlocList.append(actRLocList);
    }
    ListExpr test = nl->TwoElemList(nList.listExpr(), rlocList.listExpr());
    return test;
  }
}

Word JPoints::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->ListLength(instance) == 1 && nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct = true;
    return SetWord(new JPoints(false));
  }
  else
  {
    if (nl->ListLength(instance) == 2)
    {
      ListExpr netId = nl->First(instance);
      STRING_T nids;
      strcpy(nids, nl->StringValue(netId).c_str());
      JPoints* res = new JPoints(true);
      res->SetNetworkId(nids);
      res->StartBulkload();
      ListExpr rlocList = nl->Second(instance);
      ListExpr actRLoc = nl->TheEmptyList();
      correct = true;
      while( !nl->IsEmpty( rlocList ) && correct)
      {
        actRLoc = nl->First( rlocList );
         Word w = RouteLocation::In(nl->TheEmptyList(), actRLoc, errorPos,
                                    errorInfo, correct);
        if (correct)
        {
          RouteLocation* actInt = (RouteLocation*) w.addr;
          res->Add(*actInt);
          actInt->DeleteIfAllowed();
          actInt = 0;
        }
        else
        {
          cmsg.inFunError("Error in list of " + RouteLocation::BasicType() +
            " at " + nl->ToString(actRLoc));
        }
        rlocList = nl->Rest( rlocList );
      }
      res->EndBulkload();
      if (correct)
      {
        return SetWord(res);
      }
      else
      {
        return SetWord(Address(0));
      }
    }
  }
  correct = false;
  cmsg.inFunError("Expected List of length 1 or 2.");
  return SetWord(Address(0));
}

Word JPoints::Create(const ListExpr typeInfo)
{
  return SetWord(new JPoints(false));
}

void JPoints::Delete( const ListExpr typeInfo, Word& w )
{
  ((JPoints*) w.addr)->Destroy();
  delete ((JPoints*)w.addr);
  w.addr = 0;
}

void JPoints::Close( const ListExpr typeInfo, Word& w )
{
  delete ((JPoints*)w.addr);
  w.addr = 0;
}

Word JPoints::Clone( const ListExpr typeInfo, const Word& w )
{
  return (new JPoints(*((JPoints*) w.addr)));
}

void* JPoints::Cast( void* addr )
{
  return (new (addr) JPoints);
}

bool JPoints::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int JPoints::SizeOf()
{
  return sizeof(JPoints);
}

ListExpr JPoints::Property()
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
      RouteLocation::BasicType() + ") ...("+ RouteLocation::BasicType() +
      "))), describes a set of single network positons in an jnetwork, named" +
      " by string, by a sorted list of "+ RouteLocation::BasicType() +"s."),
      nl->TextAtom(Example())));
}


/*
1.1 Other helpful operators

*/

string JPoints::Example()
{
  return "(netname ("+ RouteLocation::Example() + "))";
}


int JPoints::GetNoComponents() const
{
  return routelocations.Size();
}

bool JPoints::IsEmpty() const {
  return (IsDefined() && GetNoComponents() == 0);
}

void JPoints::Get(const int i, RouteLocation& ri) const
{
  assert (IsDefined() && 0 <= i && i < routelocations.Size());
  routelocations.Get(i, ri);
}

/*
1.1 Managing bulkload of routelocations

*/

void JPoints::StartBulkload()
{
  SetDefined(true);
  activBulkload = true;
  sorted = false;
  routelocations.clean();
  routelocations.TrimToSize();
}

void JPoints::EndBulkload()
{
  activBulkload = false;
  Sort();
}

JPoints& JPoints::Add(const RouteLocation& rloc)
{
  if (IsDefined() && rloc.IsDefined())
  {
    if(activBulkload)
      routelocations.Append(rloc);
    else
    {
      int pos = 0;
      routelocations.Find(&rloc, RouteLocation::Compare, pos);
      RouteLocation actElem, nextElem;
      routelocations.Get(pos,actElem);
      if (actElem.Compare(rloc) != 0)
      {
        nextElem = actElem;
        routelocations.Put(pos, rloc);
        pos++;
        while(pos < routelocations.Size())
        {
          routelocations.Get(pos, actElem);
          routelocations.Put(pos, nextElem);
          nextElem = actElem;
          pos++;
        }
        routelocations.Append(nextElem);
      }
    }
  }
  return *this;
}

/*
1.1 Management of RouteIntervals

*/

void JPoints::Sort()
{
  if (IsDefined() && !IsSorted())
  {
    if (!IsEmpty())
    {
      JRLTree* sorted = new JRLTree(&routelocations);
      routelocations.clean();
      sorted->TreeToDbArray(&routelocations);
      sorted->Destroy();
      delete sorted;
    }
    sorted = true;
  }
}

bool JPoints::IsSorted() const
{
  return (IsDefined() && sorted);
}

void JPoints::FillLocationList(const DbArray<RouteLocation>& locList,
                               const JNetwork* jnet)
{
  StartBulkload();
  if (locList.Size() > 0)
  {
    RouteLocation actInt;
    for (int i = 0; i < locList.Size(); i++)
    {
      locList.Get(i,actInt);
      if (jnet->Contains(actInt))
        Add(actInt);
    }
  }
  EndBulkload();
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JPoints& line)
{
  line.Print(os);
  return os;
}
