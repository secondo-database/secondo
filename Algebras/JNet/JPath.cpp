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

2013, May Simone Jandt

1 Includes and defines

*/

#include "JPath.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "ManageJNet.h"

using namespace jnetwork;
/*
1 Implementation of class JPath

1.1 Constructors and Deconstructor

*/
JPath::JPath() : Attribute()
{}

JPath::JPath(const bool def) :
    Attribute(def), path(0), activBulkload(false)
{
  strcpy(nid, "");
}

JPath::JPath(const string netId, const DbArray<JRouteInterval>& inList,
             const bool check /* = true*/) :
    Attribute(true), path(0), activBulkload(false)
{
  strcpy(nid, netId.c_str());
  if (check)
  {
    CheckAndFillIntervallList(&inList, 0);
  }
  else
  {
    path.copyFrom(inList);
  }
  path.TrimToSize();
}

JPath::JPath(const JPath& other) :
  Attribute(other.IsDefined()), path(other.GetPath()), activBulkload(false)
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
  }
  else
    strcpy(nid, "");
}

JPath::~JPath()
{}

/*
1.1 Getter and Setter for private attributes

*/

const STRING_T* JPath::GetNetworkId() const
{
  return &nid;
}

const DbArray<JRouteInterval>& JPath::GetPath() const
{
  return path;
}

void JPath::SetNetworkId(const STRING_T& id)
{
  strcpy(nid, id);
}

void JPath::SetPath(const DbArray<JRouteInterval>& in,
                    const bool check /*=true*/,
                    const JNetwork* jnet /*=0*/)
{
  if (check)
  {
    CheckAndFillIntervallList (&in, jnet);
  }
  else
  {
    path.clean();
    path.copyFrom(in);
  }
  path.TrimToSize();
}

/*
1.1 Override Methods from Attribute

*/

void JPath::CopyFrom(const Attribute* right)
{
  *this = *((JPath*)right);
}

size_t JPath::HashValue() const
{
  size_t res = 0;
  if (IsDefined())
  {
    res += strlen(nid);
    JRouteInterval pe(false);
    for (int i = 0; i < path.Size(); i++)
    {
      Get(i,pe);
      res += pe.HashValue();
    }
  }
  return res;
}

JPath* JPath::Clone() const
{
  return new JPath(*this);
}

bool JPath::Adjacent(const Attribute* attrib) const
{
  return false;
}

int JPath::Compare(const void* ls, const void* rs)
{
  JPath lhs(*((JPath*) ls));
  JPath rhs(*((JPath*) rs));
  return lhs.Compare(rhs);
}

int JPath::Compare(const Attribute* rhs) const
{
  JPath in(*((JPath*) rhs));
  return Compare(in);
}

int JPath::Compare(const JPath& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  int aux = strcmp(nid,*rhs.GetNetworkId());
  if (aux != 0) return aux;
  if (path.Size() < rhs.GetNoComponents()) return -1;
  if (path.Size() > rhs.GetNoComponents()) return 1;
  JRouteInterval lri(false);
  JRouteInterval rri(false);
  for (int i = 0; i < path.Size(); i++)
  {
    path.Get(i, lri);
    rhs.Get(i, rri);
    aux = lri.Compare(&rri);
    if (aux != 0) return aux;
  }
  return 0;
}

size_t JPath::Sizeof() const
{
  return sizeof(JPath);
}

int JPath::NumOfFLOBs() const
{
  return 1;
}

Flob* JPath::GetFLOB(const int i)
{
  if (i == 0) return &path;
  return 0;
}

ostream& JPath::Print(ostream& os) const
{
  os << "JPath: ";
  if(IsDefined())
  {
    os << " NetworkId: " << nid << endl;
    JRouteInterval pe(false);
    for (int i = 0; i < path.Size(); i++)
    {
      path.Get(i,pe);
      os << i+1 << ". ";
      pe.Print(os);
    }
  }
  else
    os << Symbol::UNDEFINED() << endl;
  os << endl;
  return os;
}

void JPath::Destroy()
{
  path.Destroy();
}

void JPath::Clear()
{
  path.clean();
  SetDefined(true);
  path.TrimToSize();
}

Attribute::StorageType JPath::GetStorageType() const
{
  return Default;
}

const string JPath::BasicType()
{
  return "jpath";
}

const bool JPath::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

JPath& JPath::operator=(const JPath& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    path.copyFrom(other.GetPath());
  }
  return *this;
}

bool JPath::operator==(const JPath& other) const
{
  return Compare(&other) == 0;
}

bool JPath::operator!=(const JPath& other) const
{
  return Compare(&other) != 0;
}

bool JPath::operator<(const JPath& other) const
{
  return Compare(&other) < 0;
}

bool JPath::operator<=(const JPath& other) const
{
  return Compare(&other) < 1;
}

bool JPath::operator>(const JPath& other) const
{
  return Compare(&other) > 0;
}

bool JPath::operator>=(const JPath& other) const
{
  return Compare(&other) >= 0;
}

/*
1.1 Operators for Secondo Integration

*/

ListExpr JPath::Out(ListExpr typeInfo, Word value)
{
  JPath* out =  (JPath*) value.addr;

  if (!out->IsDefined())
    return nl->SymbolAtom(Symbol::UNDEFINED());
  else
  {
    NList nList(*out->GetNetworkId(),true, false);

    NList entryList(nl->TheEmptyList());
    JRouteInterval pe(false);
    bool first = true;
    for (int i = 0; i < out->GetNoComponents(); i++)
    {
      out->Get(i,pe);
      NList actRIntList(JRouteInterval::Out(nl->TheEmptyList(),
                                            SetWord((void*) &pe)));
      if (first)
      {
        first = false;
        entryList = actRIntList.enclose();
      }
      else
        entryList.append(actRIntList);
    }
    ListExpr test = nl->TwoElemList(nList.listExpr(), entryList.listExpr());
    return test;
  }
}

Word JPath::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->ListLength(instance) == 1 && nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct = true;
    return SetWord(new JPath(false));
  }
  else
  {
    if (nl->ListLength(instance) == 2)
    {
      ListExpr netId = nl->First(instance);
      ListExpr entryList = nl->Second(instance);
      ListExpr actEntry = nl->TheEmptyList();
      correct = true;
      DbArray<JRouteInterval>* tmp = new DbArray<JRouteInterval>(0);
      while( !nl->IsEmpty( entryList ) && correct)
      {
        actEntry = nl->First( entryList );
        Word w = JRouteInterval::In(nl->TheEmptyList(), actEntry, errorPos,
                                    errorInfo, correct);
        if (correct)
        {
          JRouteInterval* actInt = (JRouteInterval*) w.addr;
          tmp->Append(*actInt);
          actInt->DeleteIfAllowed();
          actInt = 0;
        }
        else
        {
          cmsg.inFunError("Error in list of " + JRouteInterval::BasicType() +
            " at " + nl->ToString(actEntry));
        }
        entryList = nl->Rest( entryList );
      }
      JPath* res = 0;
      if (correct)
      {
        res = new JPath(nl->StringValue(netId), *tmp);
      }
      tmp->Destroy();
      delete tmp;
      return SetWord(res);
    }
  }
  correct = false;
  cmsg.inFunError("Expected List of length 1 or 2.");
  return SetWord(Address(0));
}

Word JPath::Create(const ListExpr typeInfo)
{
  return SetWord(new JPath(false));
}

void JPath::Delete( const ListExpr typeInfo, Word& w )
{
  ((JPath*) w.addr)->Destroy();
  delete ((JPath*)w.addr);
  w.addr = 0;
}

void JPath::Close( const ListExpr typeInfo, Word& w )
{
  delete ((JPath*)w.addr);
  w.addr = 0;
}

Word JPath::Clone( const ListExpr typeInfo, const Word& w )
{
  return (new JPath(*((JPath*) w.addr)));
}

void* JPath::Cast( void* addr )
{
  return (new (addr) JPath);
}

bool JPath::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int JPath::SizeOf()
{
  return sizeof(JPath);
}

ListExpr JPath::Property()
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
      "))), describes a path in the jnetwork, named by string, by a "+
      "set of "+ JRouteInterval::BasicType() + "."),
      nl->TextAtom(Example())));
}

/*
1.1 Manage Bulkload of Routeintervals

*/

void JPath::StartBulkload()
{
  SetDefined(true);
  path.clean();
  path.TrimToSize();
  activBulkload = true;
}

void JPath::EndBulkload()
{
  activBulkload = false;
  path.TrimToSize();
}

JPath& JPath::Add(const JRouteInterval& pe)
{
  assert(activBulkload);
  if (IsDefined() && pe.IsDefined() && activBulkload)
  {
    path.Append(pe);
  }
  return *this;
}

/*
1.1 Other helpful operators

1.1.1 Example

*/

string JPath::Example()
{
  return "(netname ("+ JRouteInterval::Example() + "))";
}

/*
1.1.1 GetNoComponents

*/

int JPath::GetNoComponents() const
{
  return path.Size();
}

/*
1.1.1 IsEmpty

*/

bool JPath::IsEmpty() const {
  return (IsDefined() && GetNoComponents() == 0);
}

/*
1.1.1 Get

*/

void JPath::Get(const int i, JRouteInterval& ri) const
{
  assert (IsDefined() && 0 <= i && i < path.Size());
  path.Get(i, ri);
}

/*
1.1 private Methods

1.1.1 FillIntervalList

*/

void JPath::FillIntervalList(const DbArray<JRouteInterval>* inList,
                             const JNetwork* jnet)
{
  JRouteInterval actEntry;
  StartBulkload();
  for (int i = 0; i < inList->Size(); i++)
  {
    inList->Get(i,actEntry);
    if (jnet->Contains(actEntry))
    {
      Add(actEntry);
    }
  }
  EndBulkload();
}

void JPath::CheckAndFillIntervallList(const DbArray<JRouteInterval>* in,
                                     const JNetwork* jnet /*= 0*/)
{
  if (jnet != 0)
    FillIntervalList(in, jnet);
  else
  {
    JNetwork* net = ManageJNet::GetNetwork(nid);
    if (net != 0)
    {
      FillIntervalList(in, net);
      ManageJNet::CloseNetwork(net);
    }
  }
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JPath& jp)
{
  jp.Print(os);
  return os;
}
