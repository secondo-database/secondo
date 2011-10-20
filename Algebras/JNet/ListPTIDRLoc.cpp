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

#include "ListPTIDRLoc.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "Symbols.h"

/*

1 Implementation of class ~JPairTIDRLocList~

1.1. Constructors and deconstructors

*/


ListPTIDRLoc::ListPTIDRLoc():Attribute()
{}

ListPTIDRLoc::ListPTIDRLoc(bool defined): Attribute(defined), elemlist(0)
{}

ListPTIDRLoc::ListPTIDRLoc(const ListPTIDRLoc& other) :
  Attribute(other.IsDefined()), elemlist(other.GetList().Size())
{
  if (other.IsDefined()) elemlist.copyFrom(other.GetList());
}

ListPTIDRLoc::ListPTIDRLoc(const PairTIDRLoc& inId) :
  Attribute(true), elemlist(0)
{
  elemlist.Append(inId);
}

ListPTIDRLoc::~ListPTIDRLoc()
{}

/*
1.2 Getter and Setter for private Attributes

*/

DbArray<PairTIDRLoc> ListPTIDRLoc::GetList() const
{
  return elemlist;
}

void ListPTIDRLoc::SetList(const DbArray<PairTIDRLoc> inList)
{
  elemlist.copyFrom(inList);
}

/*
1.3 Override Methods from Attribute

*/

void ListPTIDRLoc::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    ListPTIDRLoc* source = (ListPTIDRLoc*) right;
    elemlist.copyFrom(source->GetList());
  }
}

Attribute::StorageType ListPTIDRLoc::GetStorageType() const
{
  return Default;
}

size_t ListPTIDRLoc::HashValue() const
{
  size_t result = 0;
  if (IsDefined())
  {
    PairTIDRLoc e;
    for(int i = 0; i < elemlist.Size(); i++)
    {
      elemlist.Get(i, e);
      result += e.HashValue();
    }
  }
  return result;
}

Attribute* ListPTIDRLoc::Clone() const
{
  return new ListPTIDRLoc(*this);
}


bool ListPTIDRLoc::Adjacent(const Attribute* attrib) const
{
  return false;
}


int ListPTIDRLoc::Compare(const Attribute* rhs) const
{
  ListPTIDRLoc* in = (ListPTIDRLoc*) rhs;
  return Compare(*in);
}

int ListPTIDRLoc::Compare(const ListPTIDRLoc& in) const
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
            PairTIDRLoc p1,p2;
            for(int i = 0; i < elemlist.Size(); i++)
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

int ListPTIDRLoc::NumOfFLOBs() const
{
  return 1;
}

Flob* ListPTIDRLoc::GetFLOB(const int n)
{
  if (n == 0) return &elemlist;
  else return 0;
}

void ListPTIDRLoc::Destroy()
{
  elemlist.Destroy();
}

size_t ListPTIDRLoc::Sizeof() const
{
  return sizeof(ListPTIDRLoc);
}

ostream& ListPTIDRLoc::Print(ostream& os) const
{
  os << "List: ";
  if (IsDefined())
  {
    PairTIDRLoc p;
    for (int i = 0; i < elemlist.Size(); i++)
    {
      os << i+1 << ". ";
      elemlist.Get(i,p);
      p.Print(os);
    }
    os << "end of List." << endl;
  }
  else
  {
    os << Symbol::UNDEFINED() << endl;
  }
  return os;
}

const std::string ListPTIDRLoc::BasicType()
{
  return "listptidrloc";
}

const bool ListPTIDRLoc::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Methods

*/


ListPTIDRLoc& ListPTIDRLoc::operator=(const ListPTIDRLoc& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    elemlist.copyFrom(other.GetList());
  }
  return *this;
}

bool ListPTIDRLoc::operator==(const ListPTIDRLoc& other) const
{
  if (Compare(other) == 0) return true;
  else return false;
}

/*
1.5 Operators for Secondo Integration

*/

ListExpr ListPTIDRLoc::Out(ListExpr typeInfo, Word value)
{
  ListPTIDRLoc* source = (ListPTIDRLoc*) value.addr;
  if (source->IsDefined())
  {
    if(source->elemlist.Size() == 0) return nl->TheEmptyList();
    else
    {
      NList result(nl->TheEmptyList());
      PairTIDRLoc e;
      bool first = true;
      for (int i = 0; i < source->elemlist.Size(); i++)
      {
        source->elemlist.Get(i,e);
        Word w = SetWord(&e);
        NList elem(PairTIDRLoc::Out(nl->TheEmptyList(), w));
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


Word ListPTIDRLoc::In(const ListExpr typeInfo, const ListExpr instance,
                      const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct=true;
    return SetWord(Address(new ListPTIDRLoc(false)));
  }

  ListExpr rest = instance;
  ListExpr first = nl->TheEmptyList();
  correct = true;
  ListPTIDRLoc* in = new ListPTIDRLoc(true);
  while( !nl->IsEmpty( rest ) )
  {
    first = nl->First( rest );
    rest = nl->Rest( rest );
    Word w =
      PairTIDRLoc::In(nl->TheEmptyList(), first, errorPos,
                      errorInfo, correct);
    if (correct)
    {
      PairTIDRLoc* e = (PairTIDRLoc*) w.addr;
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

Word ListPTIDRLoc::Create(const ListExpr typeInfo)
{
  return SetWord(new ListPTIDRLoc(false));
}


void ListPTIDRLoc::Delete( const ListExpr typeInfo, Word& w )
{
 ListPTIDRLoc* obj = (ListPTIDRLoc*) w.addr;
 obj->DeleteIfAllowed();
  w.addr = 0;
}


void ListPTIDRLoc::Close( const ListExpr typeInfo, Word& w )
{
  ((ListPTIDRLoc*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}


Word ListPTIDRLoc::Clone( const ListExpr typeInfo, const Word& w )
{
  return new ListPTIDRLoc(*((ListPTIDRLoc*) w.addr));
}


void* ListPTIDRLoc::Cast( void* addr )
{
  return (new (addr) ListPTIDRLoc);
}


bool ListPTIDRLoc::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}



int ListPTIDRLoc::SizeOf()
{
  return sizeof(ListPTIDRLoc);
}


bool ListPTIDRLoc::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  ListPTIDRLoc* obj = (ListPTIDRLoc*) value.addr;
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


bool ListPTIDRLoc::Open(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  ListPTIDRLoc* obj = new ListPTIDRLoc (true);
  valueRecord.Read( obj, obj->SizeOf(), offset );
  obj->del.refs = 1;
  obj->del.SetDelete();
  offset += obj->SizeOf();
  value = SetWord( obj );
  return true;
}

ListExpr ListPTIDRLoc::Property()
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
      nl->TextAtom("("+ PairTIDRLoc::BasicType() + " ... " +
        PairTIDRLoc::BasicType() + "), list of junctions on a route."),
      nl->StringAtom("((34 (1 17.5 Up))(57 (5 0.0 Down)))")));
}

/*
1.6 Helpful Operators

*/

void ListPTIDRLoc::Append (const PairTIDRLoc& e)
{
  elemlist.Append(e);
}

int ListPTIDRLoc::GetNoOfComponents()const
{
  return elemlist.Size();
}

void ListPTIDRLoc::Get(const int i, PairTIDRLoc& res) const
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Get(i, res);
}

void ListPTIDRLoc::Put(const int i, const PairTIDRLoc& p)
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Put(i,p);
}

