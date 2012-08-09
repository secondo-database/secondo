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

/*
1 Helpful Operations

1.1. ~checkNextUnit~

Returns true if u is valid Defined and after lastUP

*/

bool checkNextUnit(JUnit u, JUnit lastUP)
{
  return(u.IsDefined() && lastUP.IsDefined() &&
         u.Compare(lastUP) > 0 &&
         u.GetTimeInterval().After(lastUP.GetTimeInterval()));
}

/*
1 Implementation of class ~MJPoint~

1.1 Constructors and Deconstructors

*/

MJPoint::MJPoint(): Attribute()
{}

MJPoint::MJPoint(const bool def) :
    Attribute(def), units(0), activBulkload(false)
{}

MJPoint::MJPoint(const MJPoint& other) :
    Attribute(other.IsDefined())
{
  if (other.IsDefined())
  {
    strcpy(nid, *other.GetNetworkId());
    units.copyFrom(other.GetUnits());
    activBulkload = false;
  }
}


MJPoint::MJPoint(const string netId, const DbArray<JUnit>& upoints) :
  Attribute(true), units(0), activBulkload(false)
{
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
      first = false;
    }
    else
    {
      if(checkNextUnit(u, lastUP))
      {
        lastUP = u;
        units.Append(u);
      }
    }
  }
}

MJPoint::MJPoint(const UJPoint* u) :
  Attribute(true), units(0), activBulkload(false)
{
  if (u->IsDefined())
  {
    strcpy(nid, *u->GetNetworkId());
    units.Append(u->GetUnit());
  }
  else
  {
    SetDefined(false);
  }
}

MJPoint::MJPoint(JNetwork* jnet, const MPoint* in) :
  Attribute(in->IsDefined()), units(0), activBulkload(false)
{
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
        if (startPos != 0 && endPos != 0 &&
            startPos->IsDefined() && endPos->IsDefined())
        { // got valid RouteLocations and TimeStamps
          MJPoint* partRes = jnet->SimulateTrip(*startPos, *endPos,
                                                &actSource.p1,
                                                starttime, endtime, lc, rc);
          Append(partRes);
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
1.1 Override Methods from Attribute

*/

void MJPoint::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    MJPoint in(*(MJPoint*) right);
    strcpy(nid, *in.GetNetworkId());
    units.copyFrom(in.GetUnits());
    activBulkload = false;
  }
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

Attribute* MJPoint::Clone() const
{
  return new MJPoint(this);
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
  MJPoint in(*(MJPoint*) rhs);
  return Compare(in);
}

int MJPoint::Compare(const MJPoint& rhs) const
{
  if (!IsDefined() && !rhs.IsDefined()) return 0;
  if (IsDefined() && !rhs.IsDefined()) return 1;
  if (!IsDefined() && rhs.IsDefined()) return -1;
  int test = strcmp(nid, *rhs.GetNetworkId());
  if (test != 0) return test;
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

ostream& MJPoint::Print(ostream& os) const
{
  os << "MJPoint";
  if (IsDefined())
  {
    os << " in " << nid << " : " << endl;
    JUnit u;

    for (int i = 0; i < units.Size(); i++)
    {
      Get(i,u);
      u.Print(os);
    }
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
    MJPoint* p = new MJPoint(false);
    correct = true;
    return SetWord( p );
  }
  else
  {
    if (nl->ListLength(instance) != 2)
    {
      correct = false;
      cmsg.inFunError("list length should be 1 or 2");;
      return SetWord(Address(0));
    }

    ListExpr netList = nl->First(instance);
    ListExpr uList = nl->Second(instance);

    STRING_T netId;
    strcpy(netId, nl->StringValue(netList).c_str());
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
  ((MJPoint*) w.addr)->DeleteIfAllowed();
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
  return (new (addr) MJPoint);
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
1.1 Other Operations

*/

string MJPoint::Example()
{
  return "(netname ("+ JUnit::Example()+"))";
}

int MJPoint::GetNoComponents() const
{
  return units.Size();
}

bool MJPoint::IsEmpty() const
{
  return units.Size() == 0;
}

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

/*
1.1. Manage Bulkload

*/

void MJPoint::StartBulkload()
{
  SetDefined(true);
  activBulkload = true;
  units.clean();
  units.TrimToSize();
}

void MJPoint::EndBulkload()
{
  activBulkload = false;
  if (!CheckSorted())
  {
    SetDefined(false);
    units.Destroy();
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
1 Private Methods

1.1 CheckSorted

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
1.1 Append

*/

void MJPoint::Append(const MJPoint* in)
{
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

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const MJPoint& m)
{
  m.Print(os);
  return os;
}

