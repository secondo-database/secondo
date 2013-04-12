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

1 Includes and defines

*/

#include "JLine.h"
#include "Symbols.h"
#include "StandardTypes.h"
#include "JRITree.h"
#include "ManageJNet.h"
#include "JNetUtil.h"

using namespace jnetwork;
/*
1 Implementation of class JLine

1.1 Constructors and Deconstructor

*/
JLine::JLine() : Attribute()
{}

JLine::JLine(const bool def) :
    Attribute(def), routeintervals(0), sorted(false),
    activBulkload(false)
{
  strcpy(nid, "");
}

JLine::JLine(const string netId, const DbArray<JRouteInterval>& rintList,
             const bool check /*=true*/,  const bool issorted /*=false*/) :
    Attribute(true), routeintervals(0), sorted(issorted), activBulkload(false)
{
  strcpy(nid, netId.c_str());
  if (check)
  {
    CheckAndFillIntervallList(&rintList, 0);
  }
  else
  {
    routeintervals.copyFrom(rintList);
  }
  if(!issorted) Sort();
  routeintervals.TrimToSize();
}

JLine::JLine(const JNetwork* jnet, const JListRInt* rintList,
             const bool check/*=true*/) :
    Attribute(true),routeintervals(0), sorted(false), activBulkload(false)
{
  if (!rintList->IsDefined() || !jnet->IsDefined())
  {
    strcpy(nid, "");
    SetDefined(false);
  }
  else
  {
    strcpy(nid, *jnet->GetId());
    DbArray<JRouteInterval> tmp = rintList->GetList();
    if (check)
    {
      CheckAndFillIntervallList(&tmp, jnet);
    }
    else
    {
      routeintervals.copyFrom(tmp);
    }
    routeintervals.TrimToSize();
  }
}

JLine::JLine(const JLine& other) :
  Attribute(other.IsDefined()), routeintervals(0), sorted(false),
  activBulkload(false)
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    routeintervals.copyFrom(other.routeintervals);
    sorted = other.IsSorted();
    activBulkload = false;
  }
  else
    strcpy(nid, "");
}

JLine::~JLine()
{}

/*
1.1 Getter and Setter for private attributes

*/

const STRING_T* JLine::GetNetworkId() const
{
  return &nid;
}

const DbArray<JRouteInterval>& JLine::GetRouteIntervals() const
{
  return routeintervals;
}

void JLine::SetNetworkId(const STRING_T& id)
{
  strcpy(nid, id);
}

void JLine::SetRouteIntervals(const DbArray<JRouteInterval>& setri,
                              const bool check /*=true*/,
                              const bool issorted /*=false*/,
                              const JNetwork* jnet /*=0*/)
{
  if (check)
  {
    CheckAndFillIntervallList (&setri, jnet);
  }
  else
  {
    routeintervals.clean();
    routeintervals.copyFrom(setri);
  }
  if (!issorted)
  {
    sorted = false;
    Sort();
  }
  routeintervals.TrimToSize();
}

/*
1.1 Override Methods from Attribute

*/

void JLine::CopyFrom(const Attribute* right)
{
  *this = *((JLine*)right);
}

size_t JLine::HashValue() const
{
  size_t res = 0;
  if (IsDefined())
  {
    res += strlen(nid);
    JRouteInterval ri(false);
    for (int i = 0; i < routeintervals.Size(); i++)
    {
      Get(i,ri);
      res += ri.HashValue();
    }
  }
  return res;
}

JLine* JLine::Clone() const
{
  return new JLine(*this);
}

bool JLine::Adjacent(const Attribute* attrib) const
{
  return false;
}

int JLine::Compare(const void* ls, const void* rs)
{
  JLine lhs(*((JLine*) ls));
  JLine rhs(*((JLine*) rs));
  return lhs.Compare(rhs);
}

int JLine::Compare(const Attribute* rhs) const
{
  JLine in(*((JLine*) rhs));
  return Compare(in);
}

int JLine::Compare(const JLine& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  int aux = strcmp(nid,*rhs.GetNetworkId());
  if (aux != 0) return aux;
  if (routeintervals.Size() < rhs.GetNoComponents()) return -1;
  if (routeintervals.Size() > rhs.GetNoComponents()) return 1;
  JRouteInterval lri(false);
  JRouteInterval rri(false);
  for (int i = 0; i < routeintervals.Size(); i++)
  {
    routeintervals.Get(i, lri);
    rhs.Get(i, rri);
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
    os << " NetworkId: " << nid << endl;
    JRouteInterval ri(false);
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

void JLine::Clear()
{
  routeintervals.clean();
  SetDefined(true);
  sorted = true;
  activBulkload  = false;
  routeintervals.TrimToSize();
}

Attribute::StorageType JLine::GetStorageType() const
{
  return Default;
}

const string JLine::BasicType()
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
    strcpy(nid, *other.GetNetworkId());
    routeintervals.copyFrom(other.GetRouteIntervals());
    sorted = other.IsSorted();
    activBulkload = false;
  }
  return *this;
}

bool JLine::operator==(const JLine& other) const
{
  return Compare(&other) == 0;
}

bool JLine::operator!=(const JLine& other) const
{
  return Compare(&other) != 0;
}

bool JLine::operator<(const JLine& other) const
{
  return Compare(&other) < 0;
}

bool JLine::operator<=(const JLine& other) const
{
  return Compare(&other) < 1;
}

bool JLine::operator>(const JLine& other) const
{
  return Compare(&other) > 0;
}

bool JLine::operator>=(const JLine& other) const
{
  return Compare(&other) >= 0;
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
    NList nList(*out->GetNetworkId(),true, false);

    NList rintList(nl->TheEmptyList());
    JRouteInterval ri(false);
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
    ListExpr test = nl->TwoElemList(nList.listExpr(), rintList.listExpr());
    return test;
  }
}

Word JLine::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->ListLength(instance) == 1 && nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct = true;
    return SetWord(new JLine(false));
  }
  else
  {
    if (nl->ListLength(instance) == 2)
    {
      ListExpr netId = nl->First(instance);
      ListExpr rintList = nl->Second(instance);
      ListExpr actRint = nl->TheEmptyList();
      correct = true;
      DbArray<JRouteInterval>* tmp = new DbArray<JRouteInterval>(0);
      while( !nl->IsEmpty( rintList ) && correct)
      {
        actRint = nl->First( rintList );
        Word w = JRouteInterval::In(nl->TheEmptyList(), actRint, errorPos,
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
            " at " + nl->ToString(actRint));
        }
        rintList = nl->Rest( rintList );
      }
      JLine* res = 0;
      if (correct)
      {
        res = new JLine(nl->StringValue(netId), *tmp);
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
      "))), describes a part of the jnetwork, named by string, by a sorted"+
      "list of "+ JRouteInterval::BasicType() +"s."),
      nl->TextAtom(Example())));
}

/*
1.1 Manage Bulkload of Routeintervals

*/

void JLine::StartBulkload()
{
  SetDefined(true);
  activBulkload = true;
  sorted = true;
  routeintervals.clean();
  routeintervals.TrimToSize();
}

void JLine::EndBulkload(const bool sort /*=true*/)
{
  activBulkload = false;
  if (sort) Sort();
  routeintervals.TrimToSize();
}

JLine& JLine::Add(const JRouteInterval& rint)
{
  if (IsDefined() && rint.IsDefined())
  {
    if(activBulkload)
      routeintervals.Append(rint);
    else
    {
      int pos = 0;
      routeintervals.Find(&rint, JRouteInterval::Compare, pos);
      JRouteInterval actElem, nextElem;
      routeintervals.Get(pos,actElem);
      if (actElem.Compare(rint) != 0)
      {
        nextElem = actElem;
        routeintervals.Put(pos, rint);
        pos++;
        while(pos < routeintervals.Size())
        {
          routeintervals.Get(pos, actElem);
          routeintervals.Put(pos, nextElem);
          nextElem = actElem;
          pos++;
        }
        routeintervals.Append(nextElem);
      }
    }
  }
  return *this;
}

/*
1.1 Other helpful operators

1.1.1 Example

*/

string JLine::Example()
{
  return "(netname ("+ JRouteInterval::Example() + "))";
}

/*
1.1.1 GetNoComponents

*/

int JLine::GetNoComponents() const
{
  return routeintervals.Size();
}

/*
1.1.1 IsEmpty

*/

bool JLine::IsEmpty() const {
  return (IsDefined() && GetNoComponents() == 0);
}

/*
1.1.1 Get

*/
void JLine::Get(const int i, JRouteInterval& ri) const
{
  assert (IsDefined() && 0 <= i && i < routeintervals.Size());
  routeintervals.Get(i, ri);
}

/*
1.1.1 FromSpatial

*/

void JLine::FromSpatial(const JNetwork* jnet, const Line* in)
{
  routeintervals.clean();
  if (jnet != NULL && jnet->IsDefined() &&
      in != NULL && in->IsDefined())
    SetDefined(jnet->GetNetworkValueOf(in, this));
  else
    SetDefined(false);
}

/*
1.1.1 ToSpatial

*/

void JLine::ToSpatial(Line& result) const
{
  if (IsDefined() && !IsEmpty())
  {
    JNetwork* jnet = ManageJNet::GetNetwork(nid);
    jnet->GetSpatialValueOf(this, result);
    ManageJNet::CloseNetwork(jnet);
  }
  else
    result.SetDefined(false);
}

/*
1.1.1 Intersects

*/

bool JLine::Intersects(const JLine* other) const
{
  if (IsDefined() && !IsEmpty() &&
      other != 0 && other->IsDefined() && !other->IsEmpty())
  {
    if (IsSorted() && other->IsSorted())
    {
      int i = 0;
      int j = 0;
      JRouteInterval ri1, ri2;
      while (i < GetNoComponents() && j < other->GetNoComponents())
      {
        Get(i, ri1);
        Get(j, ri2);
        if (ri1.Overlaps(ri2, false))
          return true;
        else
        {
          switch (ri1.Compare(ri2))
          {
            case -1:
            {
              i++;
              break;
            }
            case 1:
            {
              j++;
              break;
            }
            default: // should never been reached
            {
              assert(false);
              break;
            }
          }
        }
      }
    }
    else
    {
      if (IsSorted() && !other->IsSorted())
      {
        JRouteInterval ri;
        for (int i = 0; i < other->GetNoComponents(); i++)
        {
          other->Get(i,ri);
          if (JNetUtil::GetIndexOfJRouteIntervalForJRInt(routeintervals, ri,
                                                         0,
                                                         GetNoComponents()-1)
              > -1)
            return true;
        }
      }
      else
      {
        if (!IsSorted() && other->IsSorted())
        {
          JRouteInterval ri;
          for (int j = 0; j < GetNoComponents(); j++)
          {
            Get(j,ri);
            if (JNetUtil::GetIndexOfJRouteIntervalForJRInt(
                                          other->GetRouteIntervals(), ri,
                                          0, other->GetNoComponents()-1)  > -1)
              return true;
          }
        }
        else
        {
          JRouteInterval ri1, ri2;
          for (int i = 0; i < GetNoComponents(); i++)
          {
            Get(i,ri1);
            for (int j = 0; j < other->GetNoComponents(); j++)
            {
              other->Get(j,ri2);
              if (ri1.Overlaps(ri2, false))
                return true;
            }
          }
        }
      }
    }
  }
  return false;
}

/*
1.1.1 Intersection

*/

JRouteInterval* JLine::Intersection(const JRouteInterval& rint) const
{
  if (IsDefined() && !IsEmpty() && rint.IsDefined())
  {
     if (IsSorted())
    {
      int j= JNetUtil::GetIndexOfJRouteIntervalForJRInt(routeintervals,
                                                        rint,
                                                        0,
                                                        GetNoComponents()-1);
      if (-1 < j)
      {
        JRouteInterval actInt;
        Get(j,actInt);
        return actInt.Intersection(rint);
      }
    }
    else
    {
      int j =  0;
      JRouteInterval actInt;
      while (j < GetNoComponents())
      {
        Get(j,actInt);
        if (actInt.Overlaps(rint, false))
          return actInt.Intersection(rint);
        j++;
      }
    }
  }
  return  0;
}

/*
1.1.1 Contains

*/

bool JLine::Contains(const JPoint* jp) const
{
  if (IsDefined() && !IsEmpty() && jp != 0 && jp->IsDefined())
    return (-1 < JNetUtil::GetIndexOfJRouteIntervalForRLoc(routeintervals,
                                                           jp->GetLocation(),
                                                           0,
                                                         GetNoComponents()-1));
  else
    return false;
}

/*
1.1.1 Union

*/

void JLine::Union(const JLine* other, JLine* result) const
{
  result->Clear();
  if (IsDefined())
  {
    if (other != 0 && other->IsDefined())
    {
      if (strcmp(nid, *other->GetNetworkId()) == 0)
      {
        result->SetNetworkId(nid);
        if(!IsEmpty())
        {
          if (!other->IsEmpty())
          {
            result->StartBulkload();
            int i = 0;
            int j = 0;
            JRouteInterval ri1(false);
            JRouteInterval ri2(false);
            while (i < GetNoComponents() && j < other->GetNoComponents())
            {
              Get(i,ri1);
              other->Get(j,ri2);
              if (ri1.GetRouteId() < ri2.GetRouteId())
              {
                result->Add(ri1);
                i++;
              }
              else if (ri1.GetRouteId() > ri2.GetRouteId())
              {
                result->Add(ri2);
                j++;
              }
              else if (ri1.Overlaps(ri2))
              {
                JRouteInterval nri(ri1.GetRouteId(),
                                   min(ri1.GetFirstPosition(),
                                       ri2.GetFirstPosition()),
                                   max(ri1.GetLastPosition(),
                                       ri2.GetLastPosition()),
                                   ri1.GetSide());
                i++;
                j++;
                bool ready = false;
                while (!ready &&
                       i < GetNoComponents() &&
                       j < other->GetNoComponents())
                {
                  Get(i,ri1);
                  other->Get(j,ri2);
                  if (ri1.Overlaps(nri))
                  {
                    nri.SetInterval(min(nri.GetFirstPosition(),
                                        ri1.GetFirstPosition()),
                                    max(nri.GetLastPosition(),
                                        ri1.GetLastPosition()));
                    i++;
                  }
                  else if (ri2.Overlaps(nri))
                  {
                    nri.SetInterval(min(nri.GetFirstPosition(),
                                        ri2.GetFirstPosition()),
                                    max(nri.GetLastPosition(),
                                        ri2.GetLastPosition()));
                    j++;
                  }
                  else
                    ready = true;
                }
                result->Add(nri);
              }
              else if (ri1.GetFirstPosition() < ri2.GetFirstPosition())
              {
                result->Add(ri1);
                i++;
              }
              else
              {
                result->Add(ri2);
                j++;
              }
            }
            while (i < GetNoComponents())
            {
              Get(i,ri1);
              result->Add(ri1);
              i++;
            }
            while (j < other->GetNoComponents())
            {
              other->Get(j,ri2);
              result->Add(ri2);
              j++;
            }
            result->EndBulkload(false);
          }
          else
            *result = *this;
        }
        else
          *result = *other;
      }
      else
        result->SetDefined(false);
    }
    else
        *result = *this;
  }
  else
  {
    if (other != 0)
      *result = *other;
    else
      result->SetDefined(false);
  }
}

/*
1.1 private Methods

1.1.1 FillIntervalList

*/

void JLine::FillIntervalList(const DbArray<JRouteInterval>* rintList,
                             const JNetwork* jnet)
{
  JRouteInterval actInt;
  StartBulkload();
  for (int i = 0; i < rintList->Size(); i++)
  {
    rintList->Get(i,actInt);
    if (jnet->Contains(actInt))
    {
      Add(actInt);
    }
  }
  EndBulkload();
}

void JLine::CheckAndFillIntervallList(const DbArray<JRouteInterval>* setri,
                                     const JNetwork* jnet /*= 0*/)
{
  if (jnet != 0)
    FillIntervalList(setri, jnet);
  else
  {
    JNetwork* net = ManageJNet::GetNetwork(nid);
    if (net != 0)
    {
      FillIntervalList(setri, net);
      ManageJNet::CloseNetwork(net);
    }
  }
}

/*
1.1.1 Sort

*/

void JLine::Sort()
{
  if (IsDefined() && !IsSorted())
  {
    if (!IsEmpty())
    {
      JRITree* sortTree = new JRITree(&routeintervals);
      routeintervals.clean();
      sortTree->TreeToDbArray(&routeintervals);
      sortTree->Destroy();
      delete sortTree;
    }
    sorted = true;
  }
}

/*
1.1.1 Append

*/

void JLine::Append(const JLine* other)
{
  assert(activBulkload);
  JRouteInterval curInt;
  if (other != 0 && other->IsDefined() && !other->IsEmpty())
  {
    for (int j = 0; j < other->GetNoComponents(); j++)
    {
      other->Get(j,curInt);
      Add(curInt);
    }
  }
}

/*
1.1.1 IsSorted

*/
bool JLine::IsSorted() const
{
  if (IsDefined())
    return sorted;
  else
    return false;
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JLine& line)
{
  line.Print(os);
  return os;
}
