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

/*
1 Implementation of class JLine

1.1 Constructors and Deconstructor

*/

JLine::JLine() : Attribute()
{}

JLine::JLine(const bool def) :
    Attribute(def), routeintervals(0), sorted(false),
    activBulkload(false)
{}

JLine::JLine(const string netId, const DbArray<JRouteInterval>& rintList) :
    Attribute(true), routeintervals(0), sorted(false), activBulkload(false)
{
  JNetwork* jnet = ManageJNet::GetNetwork(netId);
  strcpy(nid, netId.c_str());
  FillIntervalList(&rintList, jnet);
  ManageJNet::CloseNetwork(jnet);
}

JLine::JLine(const JNetwork* jnet, const JListRInt* rintList) :
    Attribute(true),routeintervals(0), sorted(false), activBulkload(false)
{
  if (!rintList->IsDefined() || !jnet->IsDefined())
  {
    SetDefined(false);
  }
  else
  {
    strcpy(nid, *jnet->GetId());
    DbArray<JRouteInterval> rlist = rintList->GetList();
    FillIntervalList(&rlist, jnet);
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

void JLine::SetRouteIntervals(DbArray<JRouteInterval>& setri)
{
  assert(setri != 0);
  routeintervals.copyFrom(setri);
  sorted = false;
  Sort();
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
      STRING_T nid;
      strcpy(nid, nl->StringValue(netId).c_str());
      JLine* res = new JLine(true);
      res->SetNetworkId(nid);
      res->StartBulkload();
      ListExpr rintList = nl->Second(instance);
       ListExpr actRint = nl->TheEmptyList();
      correct = true;
      while( !nl->IsEmpty( rintList ) && correct)
      {
        actRint = nl->First( rintList );
         Word w = JRouteInterval::In(nl->TheEmptyList(), actRint, errorPos,
                                    errorInfo, correct);
        if (correct)
        {
          JRouteInterval* actInt = (JRouteInterval*) w.addr;
          res->Add(*actInt);
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
  sorted = false;
  routeintervals.clean();
  routeintervals.TrimToSize();
}

void JLine::EndBulkload()
{
  activBulkload = false;
  Sort();
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
  {
    strcpy(nid,*jnet->GetId());
    HalfSegment hs;
    JRITree* tree = new JRITree(0);
    for (int i = 0; i < in->Size(); i++)
    {
      in->Get(i,hs);
      JRouteInterval* actInt = jnet->GetNetworkValueOf(hs);
      if (actInt != NULL && actInt->IsDefined())
      {
        actInt->SetSide((Direction) Both);
        tree->Insert(*actInt);
        actInt->DeleteIfAllowed();
        actInt = 0;
      }
    }
    tree->TreeToDbArray(&routeintervals);
    tree->Destroy();
    delete tree;
    sorted = true;
  }
  else
    SetDefined(false);
  SetDefined(in->IsDefined());
}
/*
1.1.1 ToSpatial

*/

void JLine::ToSpatial(Line& result) const
{
  if (IsDefined() && !IsEmpty())
  {
    result.Clear();
    JNetwork* jnet = ManageJNet::GetNetwork(nid);
    JRouteInterval rint;
    for (int i = 0; i < GetNoComponents(); i++)
    {
      Get(i,rint);
      SimpleLine* tmp = rint.GetSpatialValue(jnet);
      Line* tmp1 = new Line(0);
      result.Union(*tmp, *tmp1);
      result = *tmp1;
      tmp1->DeleteIfAllowed();
      tmp->DeleteIfAllowed();
      tmp = 0;
    }
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
        int j = -1;
        JRouteInterval ri;
        for (int i = 0; i < other->GetNoComponents(); i++)
        {
          other->Get(i,ri);
          j = GetOverlappingPos(ri, 0, GetNoComponents()-1);
          if (j > -1)
            return true;
        }
      }
      else
      {
        if (!IsSorted() && other->IsSorted())
        {
          int i = -1;
          JRouteInterval ri;
          for (int j = 0; j < GetNoComponents(); j++)
          {
            Get(j,ri);
            i = other->GetOverlappingPos(ri, 0, other->GetNoComponents()-1);
            if (i > -1)
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
              Get(j,ri2);
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
      int j = GetOverlappingPos(rint, 0, GetNoComponents()-1);
      if (j > -1)
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
        {
          return actInt.Intersection(rint);
        }
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
    return (GetOverlappingPos(jp->GetLocation(), 0, GetNoComponents()-1) > -1);
  else
    return false;
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
  for (int i = 0; i < rintList->Size(); i++){
    rintList->Get(i,actInt);
    if (jnet->Contains(&actInt))
    {
      Add(actInt);
    }
  }
  EndBulkload();
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
      JRITree* sorted = new JRITree(&routeintervals);
      routeintervals.clean();
      sorted->TreeToDbArray(&routeintervals);
      sorted->Destroy();
      delete sorted;
    }
    sorted = true;
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
1.1.1 GetOverlappingPos

*/

int JLine::GetOverlappingPos(const JRouteInterval& rint, int spos, int epos)
const
{
  if (IsDefined() && !IsEmpty() && IsSorted() && rint.IsDefined() &&
    spos > -1 && epos < GetNoComponents() && spos <= epos)
  {
    JRouteInterval ri;
    int mid = (epos + spos)/ 2;
    Get(mid, ri);
    if (ri.Overlaps(rint, false))
    {
      return mid;
    }
    else
    {
      switch(ri.Compare(rint))
      {
        case -1:
        {
          return GetOverlappingPos(rint, mid+1, epos);
          break;
        }

        case 1:
        {
          return GetOverlappingPos(rint, spos, mid-1);
          break;
        }

        default: //should never been reached
        {
          assert(false);
          return -1;
          break;
        }
      }
    }
  }
  return -1;
}

int JLine::GetOverlappingPos(const RouteLocation& rloc, int spos, int epos)
const
{
  if (IsDefined() && !IsEmpty() && IsSorted() && rloc.IsDefined() &&
    spos > -1 && epos < GetNoComponents() && spos <= epos)
  {
    JRouteInterval ri;
    int mid = (epos + spos)/ 2;
    Get(mid, ri);
    if (ri.Contains(rloc))
    {
      return mid;
    }
    else
    {
      switch(ri.Compare(rloc))
      {
        case -1:
        {
          return GetOverlappingPos(rloc, mid+1, epos);
          break;
        }

        case 1:
        {
          return GetOverlappingPos(rloc, spos, mid-1);
          break;
        }

        default: //should never been reached
        {
          assert(false);
          return -1;
          break;
        }
      }
    }
  }
    return -1;
}


/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JLine& line)
{
  line.Print(os);
  return os;
}