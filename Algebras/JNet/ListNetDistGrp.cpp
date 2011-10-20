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

2011, May Simone Jandt

*/

#include "ListNetDistGrp.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "Symbols.h"

/*

1 Implementation of class ~ListNetDistGrp~

1.1. Constructors and deconstructors

*/


ListNetDistGrp::ListNetDistGrp():Attribute()
{}

ListNetDistGrp::ListNetDistGrp(bool defined): Attribute(defined), elemlist(0)
{}

ListNetDistGrp::ListNetDistGrp(const ListNetDistGrp& other) :
  Attribute(other.IsDefined()), elemlist(other.GetList())
{}

ListNetDistGrp::ListNetDistGrp(const NetDistanceGroup& inId) :
  Attribute(true), elemlist(0)
{
  elemlist.Append(inId);
}

ListNetDistGrp::~ListNetDistGrp()
{}

/*
1.2 Getter and Setter for private Attributes

*/

DbArray<NetDistanceGroup> ListNetDistGrp::GetList() const
{
  return elemlist;
}

void ListNetDistGrp::SetList(const DbArray<NetDistanceGroup> inList)
{
  elemlist.copyFrom(inList);
}

/*
1.3 Override Methods from Attribute

*/

void ListNetDistGrp::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    ListNetDistGrp* source = (ListNetDistGrp*) right;
    elemlist.copyFrom(source->GetList());
  }
}

Attribute::StorageType ListNetDistGrp::GetStorageType() const
{
  return Default;
}

size_t ListNetDistGrp::HashValue() const
{
  size_t result = 0;
  if (IsDefined())
  {
    NetDistanceGroup e;
    for (int i = 0;  i < elemlist.Size(); i++)
    {
      elemlist.Get(i,e);
      result += e.HashValue();
    }
  }
  return result;
}

Attribute* ListNetDistGrp::Clone() const
{
  return new ListNetDistGrp(*this);
}


bool ListNetDistGrp::Adjacent(const Attribute* attrib) const
{
  return false;
}


int ListNetDistGrp::Compare(const Attribute* rhs) const
{
  ListNetDistGrp* in = (ListNetDistGrp*) rhs;
  return Compare(*in);
}

int ListNetDistGrp::Compare(const ListNetDistGrp& in) const
{
  if (!IsDefined() && !in.IsDefined()) return 0;
  else
  {
    if (IsDefined() && !in.IsDefined()) return 1;
    else
    {
      if (!IsDefined() && in.IsDefined()) return -1;
      else
      {
        if (elemlist.Size() < in.elemlist.Size()) return -1;
        else
        {
          if (elemlist.Size() > in.elemlist.Size()) return 1;
          else
          {
            NetDistanceGroup n1,n2;
            for (int i = 0; i < elemlist.Size(); i++)
            {
              elemlist.Get(i,n1);
              in.elemlist.Get(i,n2);
              int res = n1.Compare (n2);
              if (res != 0) return res;
            }
            return 0;
          }
        }
      }
    }
  }
}

int ListNetDistGrp::NumOfFLOBs() const
{
  return 1;
}

Flob* ListNetDistGrp::GetFLOB(const int n)
{
  if (n == 0) return &elemlist;
  else return 0;
}

void ListNetDistGrp::Destroy()
{
  elemlist.Destroy();
}

size_t ListNetDistGrp::Sizeof() const
{
  return sizeof(ListNetDistGrp);
}

ostream& ListNetDistGrp::Print(ostream& os) const
{
  os << "List: ";
  if (IsDefined())
  {
    NetDistanceGroup n;
    for(int i = 0; i < elemlist.Size(); i++)
    {
      os << i+1 << ". ";
      elemlist.Get(i,n);
      n.Print(os);
    }
    os << "end of list." << endl;
  }
  else
  {
    os << Symbol::UNDEFINED() << endl;
  }
  return os;
}

const string ListNetDistGrp::BasicType()
{
  return "listnetdistgrp";
}

const bool ListNetDistGrp::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}
/*
1.4 Standard Methods

*/


ListNetDistGrp& ListNetDistGrp::operator=(const ListNetDistGrp& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    elemlist.copyFrom(other.GetList());
  }
  return *this;
}

bool ListNetDistGrp::operator==(const ListNetDistGrp& other) const
{
  if (Compare(other) == 0) return true;
  else return false;
}

/*
1.5 Operators for Secondo Integration

*/

ListExpr ListNetDistGrp::Out(ListExpr typeInfo, Word value)
{
  ListNetDistGrp* source = (ListNetDistGrp*) value.addr;
  if (source->IsDefined())
  {
    if(source->elemlist.Size() == 0) return nl->TheEmptyList();
    else
    {
      NList result(nl->TheEmptyList());
      NetDistanceGroup e;
      bool first = true;
      for (int i = 0; i < source->elemlist.Size(); i++)
      {
        source->elemlist.Get(i,e);
        Word w = SetWord(&e);
        NList elem(NetDistanceGroup::Out(nl->TheEmptyList(), w));
        if (first)
        {
          result = elem.enclose();
          first = false;
        }
        else
        {
          result.append(elem);
        }
      }
      return result.listExpr();
    }
  }
  else
  {
    return nl->SymbolAtom(Symbol::UNDEFINED());
  }
}


Word ListNetDistGrp::In(const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct=true;
    return SetWord(Address(new ListNetDistGrp(false)));
  }

  ListExpr rest = instance;
  ListExpr first = nl->TheEmptyList();
  correct = true;
  ListNetDistGrp* in = new ListNetDistGrp(true);
  while( !nl->IsEmpty( rest ) )
  {
    first = nl->First( rest );
    rest = nl->Rest( rest );
    Word w =
      NetDistanceGroup::In(nl->TheEmptyList(), first, errorPos,
                      errorInfo, correct);
    if (correct)
    {
      NetDistanceGroup* e = (NetDistanceGroup*) w.addr;
      in->Append(*e);
      e->DeleteIfAllowed();
      e = 0;
    }
    else
    {
      in->DeleteIfAllowed();
      in = 0;
      return SetWord(Address(0));
    }
  }
  return SetWord(in);
}

Word ListNetDistGrp::Create(const ListExpr typeInfo)
{
  return SetWord(new ListNetDistGrp(false));
}


void ListNetDistGrp::Delete( const ListExpr typeInfo, Word& w )
{
  ListNetDistGrp* obj = (ListNetDistGrp*) w.addr;
  obj->DeleteIfAllowed();
  w.addr = 0;
}


void ListNetDistGrp::Close( const ListExpr typeInfo, Word& w )
{
  ((ListNetDistGrp*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}


Word ListNetDistGrp::Clone( const ListExpr typeInfo, const Word& w )
{
  return new ListNetDistGrp(*((ListNetDistGrp*) w.addr));
}


void* ListNetDistGrp::Cast( void* addr )
{
  return (new (addr) ListNetDistGrp);
}


bool ListNetDistGrp::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return (listutils::isSymbol( type, "listnetdistgrp" ));
}



int ListNetDistGrp::SizeOf()
{
  return sizeof(ListNetDistGrp);
}


bool ListNetDistGrp::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  ListNetDistGrp* obj = (ListNetDistGrp*) value.addr;
  for( int i = 0; i < obj->NumOfFLOBs(); i++ )
  {
    Flob *tmpFlob = obj->GetFLOB(i);
    SecondoCatalog* ctlg = SecondoSystem::GetCatalog();
    SmiRecordFile* rf = ctlg->GetFlobFile();
    //cerr << "FlobFileId = " << fileId << endl;
    tmpFlob->saveToFile(rf, *tmpFlob);
  }
  valueRecord.Write( obj, obj->SizeOf(), offset );
  offset += obj->SizeOf();
  return true;
}


bool ListNetDistGrp::Open(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  ListNetDistGrp* obj = new ListNetDistGrp (true);
  valueRecord.Read( obj, obj->SizeOf(), offset );
  obj->del.refs = 1;
  obj->del.SetDelete();
  offset += obj->SizeOf();
  value = SetWord( obj );
  return true;
}

ListExpr ListNetDistGrp::Property()
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
      nl->TextAtom("("+ NetDistanceGroup::BasicType() + " ... " +
                        NetDistanceGroup::BasicType() + "), junctions and " +
                   "network distances, with some additional information see" +
                   NetDistanceGroup::BasicType() +"."),
      nl->StringAtom("((34 1 25 17.5)(57 5 46 24.5))")));
}


/*
1.6 Helpful Operators

*/

void ListNetDistGrp::Append (const NetDistanceGroup& e)
{
  elemlist.Append(e);
}

int ListNetDistGrp::GetNoOfComponents() const
{
  return elemlist.Size();
}

void ListNetDistGrp::Get(const int i, NetDistanceGroup& e) const
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Get(i,e);
}

void ListNetDistGrp::Put(const int i, NetDistanceGroup& e)
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Put(i,e);
}
