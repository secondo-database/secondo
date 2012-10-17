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
1 Implementation of class ~MJPoint~

1.1 Constructors and Deconstructors

*/

MJPoint::MJPoint(): Attribute()
{}

MJPoint::MJPoint(const bool def) :
    Attribute(def), units(0), activBulkload(false), lenth(0.0)
{}

MJPoint::MJPoint(const MJPoint& other) :
    Attribute(other.IsDefined()), units(0), activBulkload(false), lenth(0.0)
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    units.copyFrom(other.GetUnits());
    lenth = other.Length();
  }
}


MJPoint::MJPoint(const string netId, const DbArray<JUnit>& upoints) :
  Attribute(true), units(0), activBulkload(false), lenth(0.0)
{
  JNetwork* jnet = ManageJNet::GetNetwork(netId);
  if (jnet == 0)
  {
    SetDefined(false);
  }
  else
  {
    ManageJNet::CloseNetwork(jnet);
    strcpy(nid, netId.c_str());
    bool first = true;
    JUnit u, lastUP;
    for (int i = 0; i < upoints.Size(); i++)
    {
      upoints.Get(i,u);
      if (first)
      {
        lastUP = u;
        units.Append(u);
        lenth += u.GetLength();
        first = false;
      }
      else
      {
        if(checkNextUnit(u, lastUP))
        {
          lastUP = u;
          units.Append(u);
          lenth += u.GetLength();
        }
      }
    }
  }

}

MJPoint::MJPoint(const UJPoint* u) :
  Attribute(true), units(0), activBulkload(false), lenth(0.0)
{
  if (u->IsDefined())
  {
    strcpy(nid, *u->GetNetworkId());
    units.Append(u->GetUnit());
    lenth = u->GetUnit().GetLength();
  }
  else
  {
    SetDefined(false);
  }
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

double MJPoint::Length() const
{
  return lenth;
}


void MJPoint::SetUnits(const DbArray<JUnit>& upoints)
{
  assert(upoints != 0);
  units.copyFrom(upoints);
  if (!CheckSorted())
  {
    SetDefined(false);
    units.Destroy();
  }
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
  result->StartBulkload();
  result->EndBulkload();
  if (IsDefined())
  {
    result->SetDefined(true);
    result->SetNetworkId(nid);
    if (!IsEmpty())
    {
      JRITree* tree = new JRITree(0);
      JUnit actUnit;
      JRouteInterval actRInt;
      for (int i = 0; i < GetNoComponents() ; i++)
      {
        Get(i,actUnit);
        actRInt = actUnit.GetRouteInterval();
        tree->Insert(actRInt);
      }
      DbArray<JRouteInterval>* res = new DbArray<JRouteInterval>(0);
      tree->TreeToDbArray(res);
      result->SetRouteIntervals(*res);
      tree->Destroy();
      delete tree;
      res->Destroy();
      delete res;
    }
  }
    else
      result->SetDefined(false);
}

/*
1.1.1 BoundingBox

*/

Rectangle< 3 > MJPoint::BoundingBox() const
{
  if (IsDefined() && GetNoComponents() > 0)
  {
    JUnit actUnit;
    Get(0,actUnit);
    JNetwork* jnet = ManageJNet::GetNetwork(*GetNetworkId());
    Rectangle<3> result = actUnit.BoundingBox(jnet);
    for (int i = 1; i < GetNoComponents(); i++)
    {
      Get(i,actUnit);
      Rectangle<3> actRect = actUnit.BoundingBox(jnet);
      if (actRect.IsDefined())
        result.Union(actRect);
    }
        ManageJNet::CloseNetwork(jnet);
        return result;
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
  return 1;
}

Flob* MJPoint::GetFLOB(const int i)
{
  if (i == 0) return &units;
  return 0;
}

void MJPoint::Destroy()
{
  units.Destroy();
}

void MJPoint::Clear()
{
  units.clean();
  units.TrimToSize();
  SetDefined(true);
  activBulkload = false;
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
      ManageJNet::CloseNetwork(jnet);
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
        res->Add(*actU);
        actU->DeleteIfAllowed();
        actU = 0;
      }
      else
      {
        cmsg.inFunError("Error in list of " + JUnit::BasicType() +
        " at " + nl->ToString(actUP));
      }
      uList = nl->Rest( uList );
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

void MJPoint::EndBulkload()
{
  activBulkload = false;
  if (!Simplify())
  {
    SetDefined(false);
    Clear();
  }
  else
  {
    units.TrimToSize();
  }
}

MJPoint& MJPoint::Add(const JUnit& up)
{
  if (IsDefined() && up.IsDefined())
  {
    if(activBulkload)
      units.Append(up);
    else
    {
      int pos = 0;
      units.Find(&up, JUnit::Compare, pos);
      JUnit actUP, nextUP;
      units.Get(pos,actUP);
      if (actUP.Compare(up) != 0)
      {
        nextUP = actUP;
        units.Put(pos, up);
        lenth += up.GetLength();
        pos++;
        while(pos < units.Size())
        {
          units.Get(pos, actUP);
          units.Put(pos, nextUP);
          nextUP = actUP;
          pos++;
        }
        units.Append(nextUP);
      }
    }
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
  up.SetNetworkId(nid);
  up.SetUnit(ju);
}

/*
1.1.1 FromSpatial

*/
void MJPoint::FromSpatial(JNetwork* jnet, const MPoint* in)
{
  Clear();
  SetDefined(in->IsDefined());
  if (jnet != 0 && jnet->IsDefined() &&
      in != 0 && in->IsDefined())
  {
    strcpy(nid, *jnet->GetId());
    if (!in->IsEmpty())
    {
      UPoint actSource;
      int i = 0;
      RouteLocation* startPos = 0;
      RouteLocation* endPos = 0;
      Instant starttime(0.0);
      Instant endtime(0.0);
      bool lc = false;
      bool rc = false;
      StartBulkload();
      while (i < in->GetNoComponents())
      {
        //find valid startposition in network
        while((startPos == 0 || !startPos->IsDefined()) &&
              i < in->GetNoComponents())
        {
          if (startPos != 0)
            startPos->DeleteIfAllowed();
          in->Get(i,actSource);
          startPos = jnet->GetNetworkValueOf(&actSource.p0);
          starttime = actSource.getTimeInterval().start;
          lc = actSource.getTimeInterval().lc;
          if (startPos == 0 || !startPos->IsDefined())
            i++;
        }
        //find valid endposition in network
        while((endPos == 0 || !endPos->IsDefined()) &&
              i < in->GetNoComponents())
        {
          if (endPos != 0)
            endPos->DeleteIfAllowed();
          in->Get(i, actSource);
          endPos = jnet->GetNetworkValueOf(&actSource.p1);
          endtime = actSource.getTimeInterval().end;
          rc = actSource.getTimeInterval().rc;
          if (endPos == 0 || !endPos->IsDefined())
            i++;
        }
        if (startPos != 0 &&  startPos->IsDefined() &&
            endPos != 0 && endPos->IsDefined())
        { // got valid RouteLocations and
          MJPoint* partRes = jnet->SimulateTrip(*startPos, *endPos,
                                                &actSource.p1,
                                                starttime, endtime, lc, rc);
          if (partRes != 0)
          {
            Append(partRes);
            partRes->Destroy();
            partRes->DeleteIfAllowed();
          }
        }
        startPos->DeleteIfAllowed();
        startPos = endPos;
        endPos = 0;
        starttime = endtime;
        lc = !rc;
        i++;
      }
      if (startPos != 0) startPos->DeleteIfAllowed();
      if (endPos != 0) endPos->DeleteIfAllowed();
      EndBulkload();
    }
  }
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
    result.SetDefined(true);
    result.StartBulkLoad();
    JNetwork* jnet = ManageJNet::GetNetwork(nid);
    JUnit ju;
    bool endTimeCorrected = false;
    Instant lastEnd;
    for (int i = 0; i < GetNoComponents(); i++)
    {
      Get(i,ju);
      MPoint* partRes = ju.Split(jnet, endTimeCorrected, lastEnd);
      for (int j = 0; j < partRes->GetNoComponents(); j++)
      {
        UPoint up;
        partRes->Get(j, up);
        result.Add(up);
      }
      partRes->DeleteIfAllowed();
    }
    result.EndBulkLoad();
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
            result->Append(this);
            result->Append(other);
            result->units.Sort(JUnit::Compare);
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
    int pos = GetUnitPosForTime(time, 0, GetNoComponents()-1);
    if (pos > -1)
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
      per != NULL && per->IsDefined() && !per->IsEmpty())
  {
    Interval<Instant> actTimeInterval;
    JUnit actUnit;
    int timeIndex = 0;
    while (timeIndex  < per->GetNoComponents())
    {
      per->Get(timeIndex, actTimeInterval);
      Get(0, actUnit);
      if (!actTimeInterval.Before(actUnit.GetTimeInterval()))
      {
        Get(GetNoComponents()-1,actUnit);
        if (!actUnit.GetTimeInterval().Before(actTimeInterval))
        {
          int unitIndex = GetUnitPosForTime(actTimeInterval);
          if (unitIndex > -1)
            return true;
        }
      }
      timeIndex++;
    }
  }
  return false;
}

bool MJPoint::Present(const Instant* inst) const
{
  if (IsDefined() && !IsEmpty() && inst != 0 && inst->IsDefined())
  {
    int pos = GetUnitPosForTime(inst,0,GetNoComponents()-1);
    return (pos > -1);
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
    Interval<Instant> actTimeInterval;
    JUnit actUnit;
    bool nextlc = true;
    int timeIndex = 0;
    times->Get(timeIndex, actTimeInterval);
    int unitIndex = 0;
    Get(unitIndex, actUnit);
    //determine first timeinterval of periods not ending before mjpoint starts
    while (actTimeInterval.Before(actUnit.GetTimeInterval()))
    {
      if (++timeIndex >= times->GetNoComponents())
      {
        //all time intervals end before mjpoint starts. stop computation.
        return;
      }
      else
        times->Get(timeIndex, actTimeInterval);
    }
    //Find first unit of mjpoint intersecting the current time interval
    if (actUnit.GetTimeInterval().Before(actTimeInterval))
    {
      unitIndex = GetUnitPosForTime(actTimeInterval);
      if (unitIndex < 0)
      {
        //mjpoint ends before time interval starts. stop computation.
        return;
      }
      Get(unitIndex, actUnit);
    }
    result.StartBulkload();
    while (timeIndex < times->GetNoComponents() &&
           unitIndex < GetNoComponents())
    {
      if (actUnit.GetTimeInterval().end < actTimeInterval.start)
      {
        if (!Next(unitIndex, actUnit))
          break;
      }
      else
      {
        if (actUnit.GetTimeInterval().start > actTimeInterval.end)
        {
          if (++timeIndex < times->GetNoComponents())
            times->Get(timeIndex,actTimeInterval);
          else
            break;
        }
        else
        {
          if (actTimeInterval.Contains(actUnit.GetTimeInterval()))
          {
            result.Add(actUnit);
            if (!Next(unitIndex, actUnit))
              break;
          }
          else
          {
            Instant starttime = max(actUnit.GetTimeInterval().start,
                                    actTimeInterval.start);
            Instant endtime = min(actUnit.GetTimeInterval().end,
                                  actTimeInterval.end);
            bool lc, rc;
            if (starttime == endtime)
            {
              lc = true;
              rc = true;
              nextlc = false;
            }
            else
            {
              lc = actUnit.GetTimeInterval().lc && actTimeInterval.lc &&
                   nextlc;
              rc = actUnit.GetTimeInterval().rc && actTimeInterval.rc;
              nextlc = true;
            }

            double startpos = actUnit.PosAtTime(&starttime);
            double endpos = actUnit.PosAtTime(&endtime);
            result.Add(JUnit(Interval<Instant>(starttime, endtime, lc, rc),
                             JRouteInterval(
                                actUnit.GetRouteInterval().GetRouteId(),
                                startpos, endpos,
                                actUnit.GetRouteInterval().GetSide())));
            if (actTimeInterval.end < actUnit.GetTimeInterval().end)
            {
              if (++timeIndex < times->GetNoComponents())
                times->Get(timeIndex, actTimeInterval);
              else
                break;
            }
            else
            {
              if (actTimeInterval.end > actUnit.GetTimeInterval().end)
              {
                if (!Next(unitIndex, actUnit))
                  break;
              }
              else
              {
                if (++timeIndex < times->GetNoComponents() &&
                    Next(unitIndex, actUnit))
                  times->Get(timeIndex, actTimeInterval);
                else
                  break;
              }
            }
          }
        }
      }
    }
    result.EndBulkload();
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
1.1.1 ~Passes~

*/

bool MJPoint::Passes(const JPoint* jp) const
{
  if (IsDefined() && !IsEmpty() && jp != 0 && jp->IsDefined())
  {
    JUnit ju;
    for (int i = 0; i < GetNoComponents(); i++)
    {
      Get(i,ju);
      if (ju.GetRouteInterval().Contains(jp->GetLocation()))
        return true;
    }
  }
  return false;
}

bool MJPoint::Passes(const JLine* jl) const
{
  if (IsDefined() && !IsEmpty() && jl != 0 && jl->IsDefined() && !jl->IsEmpty())
  {
    JLine* traj = new JLine(0);
    Trajectory(traj);
    return traj->Intersects(jl);
  }
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
          result.Add(*entry);
          entry->DeleteIfAllowed();
          entry = 0;
        }
      }
    }
    result.EndBulkload();
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
    JUnit ju;
    JRouteInterval* rint = 0;
    for (int i = 0; i < GetNoComponents(); i++)
    {
      Get(i, ju);
      rint = jl->Intersection(ju.GetRouteInterval());
      if (rint != 0)
      {
        JUnit* t = ju.AtRint(rint);
        if (t != 0)
        {
          if (t->IsDefined())
          {
            if (t->GetTimeInterval().start == t->GetTimeInterval().end)
              t->SetTimeInterval(Interval<Instant>(t->GetTimeInterval().start,
                                                   t->GetTimeInterval().end,
                                                   true, true));
            result.Add(*t);
          }
          t->DeleteIfAllowed();
          t = 0;
        }
        rint->DeleteIfAllowed();
        rint = 0;
      }
    }
    result.EndBulkload();
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
  lenth = 0.0;
  if (IsDefined() && units.Size() > 1)
  {
    DbArray<JUnit>* simpleUnits = new DbArray<JUnit>(0);
    JUnit actNewUnit, actOldUnit;
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
        actOldUnit = actNewUnit;
      }
      i++;
    }
    simpleUnits->Append(actOldUnit);
    lenth += actOldUnit.GetLength();
    simpleUnits->TrimToSize();
    units.clean();
    units.copyFrom(*simpleUnits);
    simpleUnits->Destroy();
    delete simpleUnits;
    return sorted;
  }
  else
    return true;
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
      Add(curUnit);
    }
  }
}

void MJPoint::Append(const JUnit ju)
{
  if (ju.IsDefined())
    units.Append(ju);
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
    return new Instant(last.GetTimeInterval().start);
  }
    else
      return 0;
}

/*
1.1.1 ~GetUnitPosForTime~

*/

int MJPoint::GetUnitPosForTime(const Instant* time, const int spos,
                               const int epos) const
{
  if (!IsDefined() || time == 0 || !time->IsDefined() || IsEmpty() ||
      spos < 0 || epos > GetNoComponents()-1 || epos < spos)
    return -1;
  int mid = (epos + spos) / 2;
  JUnit ju;
  Get(mid, ju);
  if (ju.GetTimeInterval().end < *time)
    return GetUnitPosForTime(time, mid+1, epos);
  else
    if (ju.GetTimeInterval().start > *time)
      return GetUnitPosForTime(time, spos, mid-1);
    else
      return mid;
}

int MJPoint::GetUnitPosForTime(const Interval<Instant>& time) const
{
  int first = 0;
  int last = GetNoComponents()-1;
  int unitIndex = 0;
  JUnit actUnit;
  while (first <= last)
  {
    unitIndex = (first + last) /2;
    if (unitIndex < 0 || unitIndex >= GetNoComponents()) break;
    Get( unitIndex, actUnit);
    if (actUnit.GetTimeInterval().Before(time))
      first = unitIndex + 1;
    else {
      if (time.Before(actUnit.GetTimeInterval()))
        last = unitIndex - 1;
      else
        return unitIndex;
    }
  }
  return -1;
}

/*
1.1.1 Next

*/

bool MJPoint::Next(int& index, JUnit& ju) const
{
  if (index >= 0 && index < GetNoComponents()-1)
  {
    Get(++index, ju);
    return true;
  }
  else
    return false;
}
/*
1.1.1 ~Refinement~

Converts the two given ~mjpoint~ into two ~mjpoint~ with identic time intervals.
The result might be empty if the two in ~mjpoint~ does not have any common
deftime.

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

