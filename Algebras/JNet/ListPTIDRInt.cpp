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

#include "ListPTIDRInt.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "AlgebraTypes.h"
#include "Symbols.h"


/*

1 Implementation of class ~ListPTIDRInt~

1.1. Constructors and deconstructors

*/


ListPTIDRInt::ListPTIDRInt():Attribute()
{}

ListPTIDRInt::ListPTIDRInt(bool defined): Attribute(defined), elemlist(0)
{}

ListPTIDRInt::ListPTIDRInt(const ListPTIDRInt& other) :
  Attribute(other.IsDefined()), elemlist(other.GetList().Size())
{
  if (other.IsDefined()) elemlist.copyFrom(other.GetList());
}

ListPTIDRInt::ListPTIDRInt(const PairTIDRInterval& inId) :
  Attribute(true), elemlist(0)
{
  elemlist.Append(inId);
}

ListPTIDRInt::~ListPTIDRInt()
{}

/*
1.2 Getter and Setter for private Attributes

*/

DbArray<PairTIDRInterval> ListPTIDRInt::GetList() const
{
  return elemlist;
}

void ListPTIDRInt::SetList(const DbArray<PairTIDRInterval> inList)
{
  elemlist.copyFrom(inList);
}

/*
1.3 Override Methods from Attribute

*/

void ListPTIDRInt::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    ListPTIDRInt* source = (ListPTIDRInt*) right;
    elemlist.copyFrom(source->GetList());
  }
}

Attribute::StorageType ListPTIDRInt::GetStorageType() const
{
  return Default;
}

size_t ListPTIDRInt::HashValue() const
{
  size_t result = 0;
  if (IsDefined())
  {
    PairTIDRInterval p;
    for (int i = 0; i < elemlist.Size(); i++)
    {
      elemlist.Get(i,p);
      result += p.HashValue();
    }
  }
  return result;
}

Attribute* ListPTIDRInt::Clone() const
{
  return new ListPTIDRInt(*this);
}


bool ListPTIDRInt::Adjacent(const Attribute* attrib) const
{
  return false;
}


int ListPTIDRInt::Compare(const Attribute* rhs) const
{
  ListPTIDRInt* in = (ListPTIDRInt*) rhs;
  return Compare(*in);
}

int ListPTIDRInt::Compare(const ListPTIDRInt& in) const
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
            PairTIDRInterval p1, p2;
            for (int i = 0; i < elemlist.Size(); i++)
            {
              elemlist.Get(i,p1);
              in.elemlist.Get(i,p2);
              int res = p1.Compare (p2);
              if (res != 0) return res;
            }
            return 0;
          }
        }
      }
    }
  }
}

int ListPTIDRInt::NumOfFLOBs() const
{
  return 1;
}

Flob* ListPTIDRInt::GetFLOB(const int n)
{
  if (n == 0) return &elemlist;
  else return 0;
}

void ListPTIDRInt::Destroy()
{
  elemlist.Destroy();
}


size_t ListPTIDRInt::Sizeof() const
{
  return sizeof(ListPTIDRInt);
}

ostream& ListPTIDRInt::Print(ostream& os) const
{
  os << "List: " ;
  if (IsDefined())
  {
    PairTIDRInterval p;
    for (int i = 0; i < elemlist.Size(); i++)
    {
      elemlist.Get(i,p);
      os << i+1 << ". ";
      p.Print(os);
    }
    os << "end of list." << endl;
  }
  else
  {
    os << Symbol::UNDEFINED() << endl;
  }
  return os;
}

const string ListPTIDRInt::BasicType()
{
  return "listptidrint";
}

const bool ListPTIDRInt::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Methods

*/


ListPTIDRInt& ListPTIDRInt::operator=(const ListPTIDRInt& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    elemlist.copyFrom(other.GetList());
  }
  return *this;
}

bool ListPTIDRInt::operator==(const ListPTIDRInt& other) const
{
  if (Compare(other) == 0) return true;
  else return false;
}

/*
1.5 Operators for Secondo Integration

*/

ListExpr ListPTIDRInt::Out(ListExpr typeInfo, Word value)
{
  ListPTIDRInt* source = (ListPTIDRInt*) value.addr;
  if (source->IsDefined())
  {
    if(source->elemlist.Size() == 0) return nl->TheEmptyList();
    else
    {
      NList result(nl->TheEmptyList());
      PairTIDRInterval e;
      bool first = true;
      for (int i = 0; i < source->elemlist.Size(); i++)
      {
        source->elemlist.Get(i,e);
        Word w = SetWord(&e);
        NList elem(PairTIDRInterval::Out(nl->TheEmptyList(), w));
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


Word ListPTIDRInt::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct=true;
    return SetWord(Address(new ListPTIDRInt(false)));
  }

  ListExpr rest = instance;
  ListExpr first = nl->TheEmptyList();
  correct = true;
  ListPTIDRInt* in = new ListPTIDRInt(true);
  while( !nl->IsEmpty( rest ) )
  {
    first = nl->First( rest );
    rest = nl->Rest( rest );
    Word w =
      PairTIDRInterval::In(nl->TheEmptyList(), first, errorPos,
                           errorInfo, correct);
    if (correct)
    {
      PairTIDRInterval* e = (PairTIDRInterval*) w.addr;
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

Word ListPTIDRInt::Create(const ListExpr typeInfo)
{
  return SetWord(new ListPTIDRInt(false));
}


void ListPTIDRInt::Delete( const ListExpr typeInfo, Word& w )
{
  ((ListPTIDRInt*) w.addr)->Destroy();
  delete ((ListPTIDRInt*) w.addr);
  w.addr = 0;
}


void ListPTIDRInt::Close( const ListExpr typeInfo, Word& w )
{
  delete ((ListPTIDRInt*) w.addr);
  w.addr = 0;
}


Word ListPTIDRInt::Clone( const ListExpr typeInfo, const Word& w )
{
  return new ListPTIDRInt(*((ListPTIDRInt*) w.addr));
}


void* ListPTIDRInt::Cast( void* addr )
{
  return (new (addr) ListPTIDRInt);
}


bool ListPTIDRInt::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}



int ListPTIDRInt::SizeOf()
{
  return sizeof(ListPTIDRInt);
}


bool ListPTIDRInt::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  ListPTIDRInt* obj = (ListPTIDRInt*) value.addr;
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


bool ListPTIDRInt::Open(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  ListPTIDRInt* obj = new ListPTIDRInt (true);
  valueRecord.Read( obj, obj->SizeOf(), offset );
  obj->del.refs = 1;
  obj->del.SetDelete();
  offset += obj->SizeOf();
  value = SetWord( obj );
  return true;
}

ListExpr ListPTIDRInt::Property()
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
      nl->TextAtom("("+ PairTIDRInterval::BasicType() + " ... " +
            PairTIDRInterval::BasicType() + "), list of parts of a route."),
      nl->StringAtom("((34 (1 17.5 35.0 Up))(57 (5 0.0 24.5 Down)))")));
}

/*
1.6 Helpful Operators

*/

void ListPTIDRInt::Append (const PairTIDRInterval& e)
{
  elemlist.Append(e);
}

int ListPTIDRInt::GetNoOfComponents() const
{
  return elemlist.Size();
}

void ListPTIDRInt::Get(const int i, PairTIDRInterval& p) const
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Get(i,p);
}

void ListPTIDRInt::Put(const int i, const PairTIDRInterval& p)
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Put(i,p);
}
