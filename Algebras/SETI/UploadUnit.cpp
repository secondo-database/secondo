/******************************************************************************
----
This file is part of SECONDO.

Copyright (C) 2004-2008, University in Hagen, Faculty of Mathematics and
Computer Science, Database Systems for New Applications.

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
----

//paragraph [1] Title: [{\Large \bf] [}]
//paragraph [10] Footnote: [{\footnote{] [}}]
//[TOC] [\tableofcontents]

[1] Implementation of class UploadUnit

May 2010, Daniel Brockmann

1 Overview

The UploadUnit is a spatio-temporal point with the following attributes:

- Moving object id (id)
- Time instant (t)
- 2D position information (pos)

It represents the current position and time of a moving object.

2 Includes and globals

******************************************************************************/

#include "DateTime.h"
#include "Algebras/Temporal/TemporalAlgebra.h"
#include "UploadUnit.h"

using namespace std;
using namespace datetime;
/******************************************************************************

3 Implementation of class UploadUnit

3.1 Basic and copy constructors

******************************************************************************/

UploadUnit::UploadUnit( int ID, Instant T, UnitPos POS )
{
  id   = ID;
  t    = T;
  pos  = POS;
  this->SetDefined(T.IsDefined());
}

UploadUnit::UploadUnit( const UploadUnit& UNIT )
{
  id   = UNIT.id;
  t    = UNIT.t;
  pos  = UNIT.pos;
  this->SetDefined(UNIT.IsDefined());
}

/******************************************************************************

3.2 The mandatory set of algebra support functions

3.2.1 In-method

******************************************************************************/

Word UploadUnit::In( const ListExpr typeInfo, const ListExpr instance,
                    const int errorPos, ListExpr& errorInfo, bool& correct )
{
  Word result = SetWord(Address(0));
  correct = false;
  // the SETI cannot yet handle UNDEFINED UploadUnits, hence we do not create
  // them:
//   if(listutils::isSymbolUndefined(instance)){ // undefined UploadUnit
//     Instant inst(isnatttype);
//     inst.SetDefined(false);
//     UploadUnit* r = new UploadUnit(-1,inst);
//     result = SetWord(r);
//     correct = true;
//     return result;
//   }
  if ( nl->ListLength(instance) != 3 )
  {
    cmsg.inFunError("Three arguments expected!");
    return result;
  }

  // Check moving object id
  ListExpr unitID = nl->First(instance);
  if (!(nl->IsAtom(unitID) && nl->AtomType(unitID == IntType)))
  {
    cmsg.inFunError("The id is invalid!");
    return result;
  }

  // Check timestamp
  ListExpr updatetime  = nl->Second(instance);
  Instant* ts;
  if (  nl->IsAtom( updatetime ) &&
        nl->AtomType( updatetime ) == StringType )
  {
    bool ok;
    ts = static_cast<Instant*>(InInstant( nl->TheEmptyList(), updatetime,
                                          errorPos, errorInfo, ok ).addr);
    if ( !ok )
    {
      cmsg.inFunError("The time stamp must be an instant-format string!");
      return result;
    }
  }
  else
  {
    cmsg.inFunError("The time stamp must be of string type!");
  }

  // Check position information
  ListExpr pos = nl->Third(instance);
  if (!( nl->ListLength(pos) == 2 &&
         nl->IsAtom(nl->First(pos))  &&
         nl->IsAtom(nl->Second(pos)) &&
         nl->AtomType(nl->First(pos)) == RealType &&
         nl->AtomType(nl->Second(pos)) == RealType ))
  {
    cmsg.inFunError("Two double type values for position expected!");
    return result;
  }

  // Create UploadUnit
  int   id = nl->IntValue(unitID);
  UnitPos p(nl->RealValue(nl->First(pos)),nl->RealValue(nl->Second(pos)));
  result.addr = new UploadUnit(id, *ts , p);
  correct = true;
  return result;
}

/******************************************************************************

3.2.2 Out-method

******************************************************************************/

ListExpr  UploadUnit::Out( ListExpr typeInfo, Word value )
{
  UploadUnit* unitPtr = static_cast<UploadUnit*>(value.addr);

  ListExpr id  = nl->IntAtom(unitPtr->id);

  ListExpr t   = OutDateTime(nl->TheEmptyList(), SetWord((void*) &unitPtr->t));

  ListExpr pos = nl->TwoElemList(
    nl->RealAtom(unitPtr->pos.x),
    nl->RealAtom(unitPtr->pos.y));

  if (unitPtr->IsDefined()) return nl->ThreeElemList( id, t, pos );
  else return nl->SymbolAtom(Symbol::UNDEFINED());
}

/******************************************************************************

3.2.3 Create-method

******************************************************************************/

Word UploadUnit::Create( const ListExpr typeInfo )
{
 return SetWord(new UploadUnit());
}

/******************************************************************************

3.2.4 Delete-method

******************************************************************************/

void UploadUnit::Delete( const ListExpr typeInfo, Word& w )
{
  delete static_cast<UploadUnit*>(w.addr);
  w.addr = 0;
}

/******************************************************************************

3.2.5 Open-method

******************************************************************************/

bool UploadUnit::Open( SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value )
{
  int      id;
  Instant  t;
  UnitPos pos;

  bool ok = true;
  ok = ok && valueRecord.Read(&id,sizeof(int),offset);
  offset += sizeof(int);
  ok = ok && valueRecord.Read(&t,sizeof(Instant),offset);
  offset += sizeof(Instant);
  ok = ok && valueRecord.Read(&pos,sizeof(UnitPos),offset);
  if(!ok){
    return false;
  }
  value.addr = new UploadUnit(id,t,pos);
  return ok;
}

/******************************************************************************

3.2.6 Save-method

******************************************************************************/

bool UploadUnit::Save( SmiRecord& valueRecord, size_t& offset,
                      const ListExpr typeInfo, Word& value )
{
  UploadUnit* unit = static_cast<UploadUnit*>(value.addr);

  bool ok = true;
  ok = ok && valueRecord.Write(&unit->id,sizeof(int),offset);
  offset += sizeof(int);
  ok = ok && valueRecord.Write(&unit->t,sizeof(Instant),offset);
  offset += sizeof(Instant);
  ok = ok && valueRecord.Write(&unit->pos,sizeof(UnitPos),offset);
  return ok;
}

/******************************************************************************

3.2.7 Close-method

******************************************************************************/

void UploadUnit::Close( const ListExpr typeInfo, Word& w )
{
  delete static_cast<UploadUnit*>(w.addr);
  w.addr = 0;
}

/******************************************************************************

3.2.8 Clone-method

******************************************************************************/

Word UploadUnit::Clone( const ListExpr typeInfo, const Word& w )
{
  return SetWord(new UploadUnit(*static_cast<UploadUnit*>(w.addr)));
}

/******************************************************************************

3.2.9 SizeOfObj-method

******************************************************************************/

int UploadUnit::SizeOfObj()
{
  return sizeof(UploadUnit);
}

/******************************************************************************

3.2.10 KindCheck-method

******************************************************************************/

bool UploadUnit::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (nl->IsEqual( type, "uploadunit" ));
}

/******************************************************************************

3.2.11 Cast-method

******************************************************************************/

void* UploadUnit::Cast(void* addr)
{
  return (new (addr) UploadUnit);
}

/******************************************************************************

3.2.12 Type description

******************************************************************************/

ListExpr UploadUnit::Property()
{
  return (nl->TwoElemList(
        nl->FiveElemList(nl->StringAtom("Signature"),
                         nl->StringAtom("Example Type List"),
                         nl->StringAtom("List Rep"),
                         nl->StringAtom("Example List"),
                         nl->StringAtom("Remarks")),
        nl->FiveElemList(nl->StringAtom("-> DATA"),
                         nl->StringAtom("uploadunit"),
                         nl->StringAtom("(<id> <dateTime> (<x> <y>))"),
                         nl->StringAtom("(9 \"2010-10-12\" (8.2 1.6)"),
                         nl->StringAtom("dateTime must be of type string."))));
}

/******************************************************************************

3.3 Methods for the abstract Attribute class

3.3.1 NumOfFLOBs-method

******************************************************************************/

int UploadUnit::NumOfFLOBs() const
{
  return 0;
}

/******************************************************************************

3.3.2 GetFLOB-method

******************************************************************************/

Flob *UploadUnit::GetFLOB(const int i)
{
  assert( i >= 0 && i < NumOfFLOBs() );
  return 0;
}

/******************************************************************************

3.3.3 Compare-method

******************************************************************************/

int UploadUnit::Compare(const Attribute*) const
{
  return 0;
}

/******************************************************************************

3.3.4 Adjacent-method

******************************************************************************/

bool UploadUnit::Adjacent(const Attribute*) const
{
  return 0;
}

/******************************************************************************

3.3.5 Sizeof-method

******************************************************************************/

size_t UploadUnit::Sizeof() const
{
  return sizeof( *this );
}

/******************************************************************************

3.3.6 Clone-method

******************************************************************************/

UploadUnit* UploadUnit::Clone() const
{
  return (new UploadUnit( *this));
}

/******************************************************************************

3.3.7 HashValue-method

******************************************************************************/

size_t UploadUnit::HashValue() const{
  return  1;
}

/******************************************************************************

3.3.8 CopyFrom-method

******************************************************************************/

void UploadUnit::CopyFrom(const Attribute* right){
  *this = *( (UploadUnit*) right);
}

/******************************************************************************

3.3.9 Print-method

******************************************************************************/

ostream& UploadUnit::Print( ostream& os ) const
{
  return (os << *this);
}

/******************************************************************************

3.4 GetID-method

Returns the moving object id

******************************************************************************/

int UploadUnit::GetID() const  { return id; }

/******************************************************************************

3.5 GetTime-method

Returns the instant of time

******************************************************************************/

Instant UploadUnit::GetTime() const  { return t; }

/******************************************************************************

3.6 GetPos-method

Returns the position information

******************************************************************************/

UnitPos UploadUnit::GetPos() const  { return pos; }
