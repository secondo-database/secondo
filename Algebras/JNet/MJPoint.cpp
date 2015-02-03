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

1 Includes

*/

#include "MJPoint.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "NList.h"
#include "Symbols.h"
#include "Direction.h"
#include "StandardTypes.h"
#include "JRITree.h"
#include "ManageJNet.h"
#include "IJPoint.h"
#include "JNetUtil.h"

using namespace jnetwork;

/*
1 Helpful Operations

1.1. ~checkNextUnit~

Returns true if u is valid Defined and after lastUP

*/

bool checkNextUnit(JUnit u, JUnit lastUP)
{
  return(u.IsDefined() &&
         lastUP.IsDefined() &&
         (u.Compare(lastUP) > 0 || lastUP.Contains(u)));
}

/*
1.1. ~skipTimeIntervalsBefore~

Searches in per for the first timeInterval not before the time interval of
actUnit.

*/

void skipTimeIntervalsBefore(const JUnit& actUnit,
                             const Periods* per,
                             int& timeIndex,
                             Interval<Instant>& actTimeInterval)
{
  while (actTimeInterval.Before(actUnit.GetTimeInterval()) &&
         timeIndex < per->GetNoComponents())
  {
    if (++timeIndex < per->GetNoComponents()){
      per->Get(timeIndex, actTimeInterval);
    }
  }
}

/*
1 Implementation of class ~MJPoint~

1.1 Constructors and Deconstructors

*/

MJPoint::MJPoint(): Attribute()
{}

MJPoint::MJPoint(const bool def) :
    Attribute(def), units(0), trajectory(0), activBulkload(false),
    lenth(0.0)
{
  strcpy(nid,"");
}


MJPoint::MJPoint(const MJPoint& other) :
    Attribute(other.IsDefined()), units(0), trajectory(0),
    activBulkload(false), lenth(0.0)
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    units.copyFrom(other.GetUnits());
    trajectory.copyFrom(other.GetTrajectory());
    lenth = other.Length();
  }
  else
  {
    strcpy(nid, "");
  }
}

MJPoint::MJPoint(const UJPoint* u) :
  Attribute(u != 0 && u->IsDefined()), units(0), trajectory(0),
  activBulkload(false), lenth(0.0)
{
  if (u != 0 && u->IsDefined())
  {
    strcpy(nid, *u->GetNetworkId());
    units.Append(u->GetUnit());
    lenth = u->GetUnit().GetLength();
    trajectory.Append(u->GetUnit().GetRouteInterval());
  }
  else
    strcpy(nid, "");
}


MJPoint::~MJPoint()
{}

/*
1.1 Getter and Setter for private Attributes

*/

const STRING_T* MJPoint::GetNetworkId() const
{
  return &nid;
}

const DbArray<JUnit>& MJPoint::GetUnits() const
{
  return units;
}

const DbArray<JRouteInterval>& MJPoint::GetTrajectory() const
{
  return trajectory;
}

double MJPoint::Length() const
{
  return lenth;
}

void MJPoint::SetNetworkId(const STRING_T& id)
{
  strcpy(nid, id);
}

/*
1.1.1 Trajectory

*/

void MJPoint::Trajectory(JLine* result) const
{
  result->Clear();
  result->SetDefined(IsDefined());
  if (IsDefined())
  {
    result->SetNetworkId(nid);
    result->SetRouteIntervals(trajectory, false, true);
  }
}

/*
1.1.1 BoundingBox

*/

Rectangle< 3 > MJPoint::BoundingBox() const
{
  if (IsDefined() && GetNoComponents() > 0)
  {
    Instant* startTime = Starttime();
    Instant* endTime = Endtime();
    double mintime = startTime->ToDouble();
    double maxtime = endTime->ToDouble();
    startTime->DeleteIfAllowed();
    endTime->DeleteIfAllowed();
    JNetwork* jnet = ManageJNet::GetNetwork(*GetNetworkId());
    Rectangle<3> result = jnet->BoundingBox(trajectory, mintime, maxtime);
    ManageJNet::CloseNetwork(jnet);
    return Rectangle<3> (result);
  }
  else
    return Rectangle<3>(false, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

/*
1.1 Override Methods from Attribute

*/

void MJPoint::CopyFrom(const Attribute* right)
{
  *this = *((MJPoint*)right);
}

Attribute::StorageType MJPoint::GetStorageType() const
{
  return Default;
}

size_t MJPoint::HashValue() const
{
  size_t res = strlen(nid);
  JUnit u;
  for (int i = 0 ; i < units.Size(); i++)
  {
    Get(i,u);
    res += u.HashValue();
  }
  return res;
}

MJPoint* MJPoint::Clone() const
{
  return new MJPoint(*this);
}

bool MJPoint::Adjacent(const Attribute* attrib) const
{
  return false;
}

int MJPoint::Compare(const void* l, const void* r){
  MJPoint lp(*(MJPoint*) l);
  MJPoint rp(*(MJPoint*) r);
  return lp.Compare(rp);
}

int MJPoint::Compare(const Attribute* rhs) const
{
  return Compare(*((MJPoint*)rhs));
}

int MJPoint::Compare(const MJPoint& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  int test = strcmp(nid, *rhs.GetNetworkId());
  if (test != 0) return test;
  if (lenth < rhs.Length()) return -1;
  if (lenth >  rhs.Length()) return 1;
  if (units.Size() < rhs.GetNoComponents()) return -1;
  if (units.Size() > rhs.GetNoComponents()) return 1;
  JUnit u, ur;
  for (int i = 0; i < units.Size(); i++)
  {
    Get(i,u);
    rhs.Get(i,ur);
    test = u.Compare(ur);
    if (test != 0) return test;
  }
  return 0;
}

size_t MJPoint::Sizeof() const
{
  return sizeof(MJPoint);
}

int MJPoint::NumOfFLOBs() const
{
  return 2;
}

Flob* MJPoint::GetFLOB(const int i)
{
  if (i == 0) return &units;
  if (i == 1) return &trajectory;
  return 0;
}

void MJPoint::Destroy()
{
  units.Destroy();
  trajectory.Destroy();
}

void MJPoint::Clear()
{
  units.clean();
  units.TrimToSize();
  trajectory.clean();
  trajectory.TrimToSize();
  SetDefined(true);
  activBulkload = false;
  lenth = 0.0;
}

ostream& MJPoint::Print(ostream& os) const
{
  os << "MJPoint:" << endl;
  if (IsDefined())
  {
    os << " in " << nid << " : " << endl;
    JUnit u;

    for (int i = 0; i < units.Size(); i++)
    {
      Get(i,u);
      u.Print(os);
    }
    os << "Total Trip Length: " << lenth << endl;
  }
  else
  {
    os << " : " << Symbol::UNDEFINED() << endl;
  }
  return os;
}

const string MJPoint::BasicType()
{
  return "mjpoint";
}

const bool MJPoint::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.1 Standard Operators

*/

MJPoint& MJPoint::operator=(const MJPoint& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    units.copyFrom(other.GetUnits());
    trajectory.copyFrom(other.GetTrajectory());
    activBulkload = false;
    lenth = other.Length();
  }
  return *this;
}

bool MJPoint::operator==(const MJPoint& other) const
{
  return (Compare(other) == 0);
}

bool MJPoint::operator!=(const MJPoint& other) const
{
  return (Compare(other) != 0);
}

bool MJPoint::operator<(const MJPoint& other) const
{
  return (Compare(other) < 0);
}

bool MJPoint::operator<=(const MJPoint& other) const
{
  return (Compare(other) < 1);
}

bool MJPoint::operator>(const MJPoint& other) const
{
  return (Compare(other) > 0);
}

bool MJPoint::operator>=(const MJPoint& other) const
{
  return (Compare(other) > -1);
}

/*
1.1 Operators for Secondo Integration

*/

ListExpr MJPoint::Out(ListExpr typeInfo, Word value)
{
  MJPoint* in = (MJPoint*) value.addr;
  if (!in->IsDefined())
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
  else
  {
    NList netList(*in->GetNetworkId(),true,false);
    ListExpr uList = nl->TheEmptyList();
    ListExpr lastElem = nl->TheEmptyList();
    JUnit u;
    for (int i = 0; i < in->GetNoComponents(); i++)
    {
      in->Get(i,u);
      Word w;
      w.setAddr(&u);
      ListExpr curList = JUnit::Out(nl->TheEmptyList(), w);
      if (nl->TheEmptyList() == uList)
      {
        uList =  nl->Cons( curList, nl->TheEmptyList() );
        lastElem = uList;
      }
      else
      {
        lastElem = nl->Append(lastElem, curList);
      }
    }
    return nl->TwoElemList(netList.listExpr(), uList);
  }
}

Word MJPoint::In(const ListExpr typeInfo, const ListExpr instance,
                       const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if ( listutils::isSymbolUndefined( instance ) )
  {
    correct = true;
    return SetWord(  new MJPoint(false) );
  }
  else
  {
    if (nl->ListLength(instance) != 2)
    {
      correct = false;
      cmsg.inFunError("list length should be 1 or 2");
      return SetWord(Address(0));
    }

    ListExpr netList = nl->First(instance);
    ListExpr uList = nl->Second(instance);

    STRING_T netId;
    strcpy(netId, nl->StringValue(netList).c_str());
    JNetwork* jnet = ManageJNet::GetNetwork(netId);
    if (jnet == 0)
    {
      correct = false;
      cmsg.inFunError("Given jnetwork does not exist.");
      return SetWord(Address(0));
    }
    else
    {
      MJPoint* res = new MJPoint(true);
      res->SetNetworkId(netId);
      res->StartBulkload();
      ListExpr actUP = nl->TheEmptyList();
      JUnit lastUP;
      correct = true;
      while( !nl->IsEmpty( uList ) && correct)
      {
        actUP = nl->First( uList );
        Word w = JUnit::In(nl->TheEmptyList(), actUP, errorPos,
                           errorInfo, correct);
        if (correct)
        {
          JUnit* actU = (JUnit*) w.addr;
          if (actU != 0)
          {
            res->Add(*actU);
            actU->DeleteIfAllowed();
            actU = 0;
          }
        }
        else
        {
          cmsg.inFunError("Error in list of " + JUnit::BasicType() +
          " at " + nl->ToString(actUP));
        }
        uList = nl->Rest( uList );
      }
      res->EndBulkload(false);
      ManageJNet::CloseNetwork(jnet);
      if (correct)
      {
        return SetWord(res);
      }
      else
      {
        res->Destroy();
        delete res;
        return SetWord(Address(0));
      }
    }
  }
}

Word MJPoint::Create(const ListExpr typeInfo)
{
  return SetWord(new MJPoint(true));
}

void MJPoint::Delete( const ListExpr typeInfo, Word& w )
{
  MJPoint* in = (MJPoint*) w.addr;
  in->DeleteIfAllowed();
  w.addr = 0;
}

void MJPoint::Close( const ListExpr typeInfo, Word& w )
{
  ((MJPoint*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}

Word MJPoint::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(new MJPoint(*(MJPoint*) w.addr));
}

void* MJPoint::Cast( void* addr )
{
  return new (addr) MJPoint();
}

bool MJPoint::KindCheck ( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

int MJPoint::SizeOf()
{
  return sizeof(MJPoint);
}

ListExpr MJPoint::Property()
{
  return nl->TwoElemList(
    nl->FourElemList(
      nl->StringAtom("Signature"),
      nl->StringAtom("Example Type List"),
      nl->StringAtom("List Rep"),
      nl->StringAtom("Example List")),
    nl->FourElemList(
      nl->StringAtom("-> " + Kind::TEMPORAL()),
      nl->StringAtom(BasicType()),
      nl->TextAtom("(string (("+ JUnit::BasicType() + ")... ( " +
      JUnit::BasicType() + "))), describes the moving of an jpoint in the "+
      "jnetwork."),
      nl->TextAtom(Example())));
}

/*
1.1. Manage Bulkload

*/

void MJPoint::StartBulkload()
{
  Clear();
  SetDefined(true);
  activBulkload = true;
}

void MJPoint::EndBulkload(const bool simplify /*=true*/,
                          const bool buildtraj /*=true*/)
{
  activBulkload = false;
  if (simplify)
  {
    if(!Simplify())
    {
      SetDefined(false);
      Clear();
    }
    else
    {
      units.TrimToSize();
      trajectory.TrimToSize();
    }
  }
  if (buildtraj)
  {
    if (IsDefined() && units.Size() > 0)
    {
      JRITree* tree = new JRITree(0);
      JUnit unit;
      int i = 0;
      while (i < units.Size())
      {
        Get(i,unit);
        lenth += unit.GetLength();
        tree->Insert(unit.GetRouteInterval());
        i++;
      }
      tree->TreeToDbArray(&trajectory,0);
      tree->Destroy();
      delete tree;
      SetDefined(true);
      units.TrimToSize();
      trajectory.TrimToSize();
    }
    else
      SetDefined(false);
  }
}

MJPoint& MJPoint::Add(const JUnit& up)
{
  if (IsDefined() && up.IsDefined() && activBulkload)
  {
    units.Append(up);
  }
  return *this;
}

/*
1.1 Other Operations

1.1.1 Example

*/

string MJPoint::Example()
{
  return "(netname ("+ JUnit::Example()+"))";
}


/*
1.1.1 GetNoComponents

*/

int MJPoint::GetNoComponents() const
{
  return units.Size();
}


/*
1.1.1.1 IsEmpty

*/

bool MJPoint::IsEmpty() const
{
  return units.Size() == 0;
}


/*
1.1.1 Get

*/

void MJPoint::Get(const int i, JUnit& up) const
{
  assert (IsDefined() && 0 <= i && i < units.Size());
  units.Get(i,up);
}

void MJPoint::Get(const int i, JUnit* up) const
{
  assert (IsDefined() && 0 <= i && i < units.Size());
  units.Get(i,up);
}

void MJPoint::Get(const int i, UJPoint& up) const
{
  assert(IsDefined()&& 0 <= i && i < units.Size());
  JUnit ju;
  units.Get(i,ju);
  up.SetDefined(true);
  up.SetNetworkId(nid);
  up.SetUnit(ju, false);
}

/*
1.1.1 FromSpatial

*/
void MJPoint::FromSpatial(JNetwork* jnet, const MPoint* in)
{
  SetDefined(in->IsDefined());
  if (jnet != 0 && jnet->IsDefined() &&
      in != 0 && in->IsDefined())
    SetDefined(jnet->GetNetworkValueOf(in, this));
  else
    SetDefined(false);
}

/*
1.1.1 ~ToSpatial~

*/

void MJPoint::ToSpatial(MPoint& result) const
{
  result.Clear();
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
1.1.1. ~Union~

*/

void MJPoint::Union(const MJPoint* other, MJPoint* result) const
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
            JUnit a(false);
            JUnit b(false);
            int i = 0;
            int j = 0;
            while (i < GetNoComponents() && j < other->GetNoComponents())
            {
              Get(i,a);
              other->Get(j,b);
              Instant bt = b.GetTimeInterval().start;
              int test =
                a.GetTimeInterval().end.CompareTo(&bt);
              if (test < 1)
              {
                result->Add(a);
                i++;
              }
              else
              {
                bt = b.GetTimeInterval().end;
                test = a.GetTimeInterval().start.CompareTo(&bt);
                if (test > -1)
                {
                  result->Add(b);
                  j++;
                }
                else if (a.Compare(b) == 0)
                {
                  result->Add(a);
                  i++;
                  j++;
                }
                else
                {
                  result->EndBulkload(false,false);
                  result->SetDefined(false);
                  i = GetNoComponents();
                  j = other->GetNoComponents();
                }
              }
            }
            while (i < GetNoComponents())
            {
              Get(i,a);
              result->Add(a);
              i++;
            }
            while (j < other->GetNoComponents())
            {
              other->Get(j,b);
              result->Add(b);
              j++;
            }
            result->EndBulkload();
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
1.1.1 ~AtInstant~

*/

IJPoint MJPoint::AtInstant(const Instant* time) const
{
  if (IsDefined() && !IsEmpty() && time != 0 && time->IsDefined())
  {
    int pos = GetUnitPosForTime(*time, 0, GetNoComponents()-1);
    if (pos > -1 && pos < GetNoComponents())
    {
      JUnit ju;
      Get(pos, ju);
      return ju.AtInstant(time, nid);
    }
  }
  return IJPoint(false);
}

/*
1.1.1 Present

*/

bool MJPoint::Present(const Periods* per) const
{
  if (IsDefined() && !IsEmpty() &&
      per != 0 && per->IsDefined() && !per->IsEmpty())
  {
    Interval<Instant> actTimeInterval;
    JUnit actUnit;
    int timeIndex = 0;
    per->Get(timeIndex, actTimeInterval);
    int unitIndex = 0;
    Get(unitIndex, actUnit);
    //determine first timeinterval of periods not ending before mjpoint starts
    skipTimeIntervalsBefore(actUnit, per, timeIndex, actTimeInterval);
    //Find first unit of mjpoint intersecting the current time interval
    FindFirstUnit(actTimeInterval, unitIndex, actUnit);
    while (timeIndex < per->GetNoComponents() &&
           unitIndex < GetNoComponents())
    {
      if (actUnit.GetTimeInterval().Before(actTimeInterval))
      {
        if (++unitIndex < GetNoComponents())
          Get(unitIndex,actUnit);
      }
      else if (actTimeInterval.Before(actUnit.GetTimeInterval()))
      {
        if (++timeIndex < per->GetNoComponents())
          per->Get(timeIndex,actTimeInterval);
      }
      else
      {
        return true;
      }
    }
  }
  return false;
}

bool MJPoint::Present(const Instant* inst) const
{
  if (IsDefined() && !IsEmpty() && inst != 0 && inst->IsDefined())
  {
    JUnit first, last;
    Get(0,first);
    Get(GetNoComponents()-1,last);
    if (*inst < first.GetTimeInterval().start ||
         last.GetTimeInterval().end < *inst)
      return false;
    else
      return (-1 < GetUnitPosForTime(*inst, 0, GetNoComponents()-1));
  }
  return false;
}

/*
1.1.1 AtPeriods

*/

void MJPoint::AtPeriods(const Periods* times, MJPoint& result) const
{
  result.Clear();
  if (IsDefined() && !IsEmpty() &&
      times != 0 && times->IsDefined() && !times->IsEmpty())
  {
    result.SetDefined(true);
    result.SetNetworkId(nid);
    result.StartBulkload();
    Interval<Instant> actTimeInterval;
    JUnit actUnit;
    bool nextlc = true;
    int timeIndex = 0;
    times->Get(timeIndex, actTimeInterval);
    int unitIndex = 0;
    Get(unitIndex, actUnit);
    //determine first timeinterval of periods not ending before mjpoint starts
    skipTimeIntervalsBefore (actUnit, times, timeIndex, actTimeInterval);

    //Find first unit of mjpoint intersecting the current time interval
    FindFirstUnit(actTimeInterval, unitIndex, actUnit);
    while (timeIndex < times->GetNoComponents() &&
           unitIndex < GetNoComponents())
    {
      if (actUnit.GetTimeInterval().Before(actTimeInterval))
      {
        if (++unitIndex < GetNoComponents())
          Get(unitIndex,actUnit);
      }
      else
      {
        if (actTimeInterval.Before(actUnit.GetTimeInterval()))
        {
          if (++timeIndex < times->GetNoComponents())
            times->Get(timeIndex,actTimeInterval);
        }
        else
        {
          if (actTimeInterval.Contains(actUnit.GetTimeInterval()))
          {
            result.Add(actUnit);
            if (++unitIndex < GetNoComponents())
              Get(unitIndex,actUnit);
          }
          else
          {
            Instant starttime = max(actUnit.GetTimeInterval().start,
                                    actTimeInterval.start);
            Instant endtime = min(actUnit.GetTimeInterval().end,
                                  actTimeInterval.end);
            bool lc = nextlc;
            bool rc = false;
            if (starttime == endtime)
            {
              rc = true;
              nextlc = false;
            }
            else
            {
              lc = actUnit.GetTimeInterval().lc && actTimeInterval.lc &&
                   lc;
              rc = actUnit.GetTimeInterval().rc && actTimeInterval.rc;
              nextlc = true;
            }
            double startpos = actUnit.PosAtTime(&starttime);
            double endpos = actUnit.PosAtTime(&endtime);
            JUnit res(Interval<Instant>(starttime, endtime, lc, rc),
                      JRouteInterval(actUnit.GetRouteInterval().GetRouteId(),
                                     min(startpos, endpos),
                                     max(startpos, endpos),
                                     actUnit.GetRouteInterval().GetSide()));
            result.Add(res);
            if (actTimeInterval.end < actUnit.GetTimeInterval().end)
            {
              if (++timeIndex < times->GetNoComponents())
                times->Get(timeIndex, actTimeInterval);
            }
            else
            {
              if (actTimeInterval.end > actUnit.GetTimeInterval().end)
              {
                if (++unitIndex < GetNoComponents())
                  Get(unitIndex,actUnit);
              }
              else
              {
                if (++timeIndex < times->GetNoComponents())
                  times->Get(timeIndex, actTimeInterval);
                if (++unitIndex < GetNoComponents())
                  Get(unitIndex,actUnit);
              }
            }
          }
        }
      }
    }
    result.EndBulkload(false, true);
  }
  else
    result.SetDefined(false);
}

/*
1.1.1 ~Initial~

*/

IJPoint MJPoint::Initial() const
{
  if (IsDefined() && !IsEmpty())
  {
    JUnit ju;
    Get(0, ju);
    return ju.Initial(nid);
  }
  return IJPoint(false);
}

/*
1.1.1 ~Final~

*/

IJPoint MJPoint::Final() const
{
  if (IsDefined() && !IsEmpty())
  {
    JUnit ju;
    Get(GetNoComponents() - 1, ju);
    return ju.Final(nid);
  }
  return IJPoint(false);
}


/*
1.1.1 ~Passes~

*/

bool MJPoint::Passes(const JPoint* jp) const
{
  if (IsDefined() && !IsEmpty() && jp != 0 && jp->IsDefined()  &&
      strcmp(nid,*jp->GetNetworkId()) == 0)
    return (JNetUtil::GetIndexOfJRouteIntervalForRLoc(trajectory,
                                                      jp->GetLocation(),
                                                      0,
                                                      trajectory.Size()-1)
              > -1);
  else
    return false;
}

bool MJPoint::Passes(const JLine* jl) const
{
  if (IsDefined() && !IsEmpty() &&
      jl != 0 && jl->IsDefined() && !jl->IsEmpty() &&
      strcmp(nid, *jl->GetNetworkId()) == 0)
    return JNetUtil::ArrayContainIntersections(jl->GetRouteIntervals(),
                                               trajectory);
  else
    return false;
}

/*
1.1.1 ~At~

*/

void MJPoint::At(const JPoint* jp, MJPoint& result) const
{
  result.Clear();
  if (IsDefined() && !IsEmpty() && jp != 0 && jp->IsDefined() &&
      strcmp(nid, *jp->GetNetworkId()) == 0)
  {
    result.SetDefined(true);
    result.SetNetworkId(nid);
    result.StartBulkload();
    JUnit ju;
    for (int i = 0; i < GetNoComponents(); i++)
    {
      Get(i,ju);
      if (ju.GetRouteInterval().Contains(jp->GetLocation()))
      {
        JUnit* entry = ju.AtPos(jp);
        if (entry != 0)
        {
          if (entry->IsDefined())
            result.Add(*entry);
          entry->DeleteIfAllowed();
          entry = 0;
        }
      }
    }
    result.EndBulkload(false, true);
  }
  else
    result.SetDefined(false);
}

void MJPoint::At(const JLine* jl, MJPoint& result) const
{
  if (IsDefined() && !IsEmpty() &&
      jl != 0 && jl->IsDefined() && !jl->IsEmpty() &&
      strcmp(nid, *jl->GetNetworkId()) == 0)
  {
    result.SetDefined(true);
    result.SetNetworkId(nid);
    result.StartBulkload();
    bool lastrc = false;
    JUnit ju(false);
    JRouteInterval* rint = 0;
    for (int i = 0; i < GetNoComponents(); i++)
    {
      Get(i, ju);
      rint = jl->Intersection(ju.GetRouteInterval());
      if (rint != 0)
      {
        JUnit* t = ju.AtRint(rint, lastrc);
        if (t != 0)
        {
          if (t->IsDefined())
          {
            result.Add(*t);
          }
          t->DeleteIfAllowed();
          t = 0;
        }
        rint->DeleteIfAllowed();
        rint = 0;
      }
    }
    result.EndBulkload(false, true);
  }
  else
    result.SetDefined(false);
}

/*
1.1.1 ~Intersects~

*/

bool MJPoint::Intersects(const MJPoint* other) const
{
  bool result = false;
  if (IsDefined() && !IsEmpty() &&
      other  != 0 && other->IsDefined() && !other->IsEmpty() &&
      strcmp(nid, *other->GetNetworkId()) == 0)
  {
    MJPoint* refA = new MJPoint(false);
    MJPoint* refB = new MJPoint(false);
    Refinement(other, refA, refB);
    if (refA->IsDefined() && refB->IsDefined() &&
        !refA->IsEmpty() && !refB->IsEmpty() &&
        refA->GetNoComponents() == refB->GetNoComponents())
    {
      JUnit juA, juB;
      int i = 0;
      while (!result && i < refA->GetNoComponents())
      {
        refA->Get(i,juA);
        refB->Get(i,juB);
        if (juA.GetRouteInterval().Overlaps(juB.GetRouteInterval()))
          result = true;
        i++;
      }
    }
    refA->Destroy();
    refA->DeleteIfAllowed();
    refB->Destroy();
    refB->DeleteIfAllowed();
  }
  return result;
}

/*
1.1  Private Methods

1.1.1 CheckSorted

*/

bool MJPoint::CheckSorted() const
{
  if (IsDefined() && units.Size() > 1)
  {
    JUnit lastUP, actUP;
    Get(0,lastUP);
    int i = 1;
    bool sorted = true;
    while (i < units.Size() && sorted)
    {
      Get(i,actUP);
      sorted = checkNextUnit(actUP, lastUP);
      lastUP = actUP;
      i++;
    }
        return sorted;
  }
    else
    {
      return true;
    }
}

/*
1.1.1 ~Simplify~

*/

bool MJPoint::Simplify()
{
  if (IsDefined() && units.Size() > 0)
  {
    JRITree* tree = new JRITree(0);
    DbArray<JUnit>* simpleUnits = new DbArray<JUnit>(0);
    JUnit actNewUnit(false), actOldUnit(false);
    Get(0,actOldUnit);
    actNewUnit = actOldUnit;
    int i = 1;
    bool sorted = true;
    while (i < units.Size() && sorted)
    {
      Get(i,actNewUnit);
      sorted = sorted && checkNextUnit(actNewUnit, actOldUnit);
      if (!actOldUnit.ExtendBy(actNewUnit))
      {
        simpleUnits->Append(actOldUnit);
        lenth += actOldUnit.GetLength();
        tree->Insert(actOldUnit.GetRouteInterval());
        actOldUnit = actNewUnit;
      }
          i++;
    }
    simpleUnits->Append(actOldUnit);
    lenth += actOldUnit.GetLength();
    tree->Insert(actOldUnit.GetRouteInterval());
    simpleUnits->TrimToSize();
    units.clean();
    units.copyFrom(*simpleUnits);
    delete simpleUnits;
    tree->TreeToDbArray(&trajectory,0);
    tree->Destroy();
    delete tree;
    return sorted;
  }
  return false;
}

/*
1.1.1 Append

*/

void MJPoint::Append(const MJPoint* in)
{
  assert(activBulkload);
  JUnit curUnit;
  if (in != 0 && in->IsDefined() && !in->IsEmpty())
  {
    for (int j = 0; j < in->GetNoComponents(); j++)
    {
      in->Get(j,curUnit);
      if (curUnit.IsDefined())
        Add(curUnit);
    }
  }
}

void MJPoint::Append(const JUnit ju)
{
  assert(activBulkload);
  if (ju.IsDefined())
  {
    units.Append(ju);
  }
}


/*
1.1.1 ~Starttime~

*/

Instant* MJPoint::Starttime() const
{
  JUnit first;
  if (IsDefined() && !IsEmpty())
  {
    Get(0, first);
    return new Instant(first.GetTimeInterval().start);
  }
  else
    return 0;
}

/*
1.1.1 ~Endtime~

*/

Instant* MJPoint::Endtime() const
{
  JUnit last;
  if (IsDefined() && !IsEmpty())
  {
    Get(GetNoComponents()-1, last);
    return new Instant(last.GetTimeInterval().end);
  }
    else
      return 0;
}

/*
1.1.1 ~GetUnitPosForTime~

*/

int MJPoint::GetUnitPosForTime(const Interval<Instant>& time, const int spos,
                               const int epos) const
{

  if (!IsDefined() || IsEmpty() || !time.IsDefined() ||
    spos < 0 || epos > GetNoComponents()-1 || epos < spos)
    return -1;
  else
  {
    int mid = (epos + spos) / 2;
    JUnit ju;
    Get(mid, ju);
    if (ju.GetTimeInterval().Before(time))
    {
      if (mid != spos)
        return GetUnitPosForTime(time, mid, epos);
      else
        return GetUnitPosForTime(time, mid+1,epos);
    }
    else
    {
      if (time.Before(ju.GetTimeInterval()))
      {
        if (mid != epos)
          return GetUnitPosForTime(time, spos, mid);
        else
          return GetUnitPosForTime(time, spos, mid-1);
      }
      else
      {
        if (ju.GetTimeInterval().Contains(time.start))
          return mid;
        else
        {
          while (ju.GetTimeInterval().start > time.start &&
                 mid > 0)
          {
            mid--;
            Get(mid, ju);
          }
          return mid;
        }
      }
    }
  }
  //return GetUnitPosForTime(time.start, spos, epos);
}

int MJPoint::GetUnitPosForTime(const Instant& time,
                               const int spos, const int epos) const
{
  if (!IsDefined() || IsEmpty() || !time.IsDefined() ||
    spos < 0 || epos > GetNoComponents()-1 || epos < spos)
    return -1;
  else
  {
    int mid = (epos + spos) / 2;
    JUnit ju;
    Get(mid, ju);
    if (time > ju.GetTimeInterval().end)
      return GetUnitPosForTime(time,mid+1,epos);
    else if (time < ju.GetTimeInterval().start)
      return GetUnitPosForTime(time, spos, mid-1);
    else
      return mid;
  }
}

/*
1.1.1 ~findFirstUnit~

*/

void MJPoint::FindFirstUnit(const Interval<Instant>& actTimeInterval,
                            int& unitIndex,
                            JUnit& actUnit) const
{
  if (actUnit.GetTimeInterval().Before(actTimeInterval))
  {
    unitIndex = GetUnitPosForTime(actTimeInterval,
                                  0, GetNoComponents()-1);
    if (unitIndex < 0)
        unitIndex = GetNoComponents();
    if (unitIndex < GetNoComponents())
      Get(unitIndex, actUnit);
  }
}

/*
1.1.1 ~Refinement~

*/

void MJPoint::Refinement(const MJPoint* in2, MJPoint* out1, MJPoint* out2) const
{
  if (IsDefined() && in2 != 0 && in2->IsDefined() && out1 != 0 && out2 != 0)
  {
    out1->Clear();
    out2->Clear();
    JUnit juIn1, juIn2;
    Instant starttime, endtime;
    bool lc, rc;
    int i = 0;
    int j = 0;
    out1->StartBulkload();
    out2->StartBulkload();
    while (i < GetNoComponents() && j < in2->GetNoComponents())
    {
      Get(i,juIn1);
      in2->Get(j,juIn2);
      if (juIn1.GetTimeInterval().Before(juIn2.GetTimeInterval()))
      {
        i++;
      }
      else
      {
        if (juIn2.GetTimeInterval().Before(juIn1.GetTimeInterval()))
        {
          j++;
        }
        else
        {
          //Overlapping time intervals found
          //Compute common time interval
          starttime = max(juIn1.GetTimeInterval().start,
                          juIn2.GetTimeInterval().start);
          endtime = min(juIn1.GetTimeInterval().end,
                        juIn2.GetTimeInterval().end);
          if (starttime != endtime)
          {
            lc = false;
            rc = false;
          }
          else
          {
            lc = true;
            rc = true;
          }
          Interval<Instant> actTimeInterval(starttime, endtime, lc, rc);
          //compute route intervals of juIn1 and juIn2 for time interval
          JRouteInterval* rint1 = juIn1.PosAtTimeInterval(actTimeInterval);
          JRouteInterval* rint2 = juIn2.PosAtTimeInterval(actTimeInterval);
          out1->Append(JUnit(actTimeInterval, *rint1));
          out2->Append(JUnit(actTimeInterval, *rint2));
          rint1->DeleteIfAllowed();
          rint2->DeleteIfAllowed();
          //search next pair
          if (juIn1.GetTimeInterval().end == juIn2.GetTimeInterval().end)
          {
            i++;
            j++;
          }
          else
          {
            if (endtime == juIn1.GetTimeInterval().end)
            {
              i++;
            }
            else
            {
              j++;
            }
          }
        }
      }
    }
    out1->EndBulkload(false, false);
    out2->EndBulkload(false, false);
  }
}

/*
1 Overwrite output operator

*/

ostream& operator<< (ostream& os, const MJPoint& m)
{
  m.Print(os);
  return os;
}

