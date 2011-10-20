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

1 Includes

*/

#include "JListTID.h"
#include "ListUtils.h"
#include "NestedList.h"
#include "Symbols.h"

/*

1 Implementation of class ~JTupleIdList~

1.1 Constructors and deconstructors

*/


JListTID::JListTID():Attribute()
{}

JListTID::JListTID(bool defined): Attribute(defined), elemlist(0)
{}

JListTID::JListTID(const JListTID& other) :
  Attribute(other.IsDefined()), elemlist(other.GetList())
{}

JListTID::JListTID(const TupleIdentifier& inId) :  Attribute(true), elemlist(0)
{
  elemlist.Append(inId);
}

JListTID::~JListTID()
{}

/*
1.1 Getter and Setter for private Attributes

*/

DbArray<TupleIdentifier> JListTID::GetList() const
{
  return elemlist;
}

void JListTID::SetList(const DbArray<TupleIdentifier> inList)
{
  elemlist.copyFrom(inList);
}

/*
1.3 Override Methods from Attribute

*/

void JListTID::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    JListTID* source = (JListTID*) right;
    elemlist.copyFrom(source->GetList());
  }
}

Attribute::StorageType JListTID::GetStorageType() const
{
  return Default;
}

size_t JListTID::HashValue() const
{
  size_t result = 0;
  if (IsDefined())
  {
    TupleIdentifier t;
    for (int i = 0; i < elemlist.Size(); i++)
    {
      elemlist.Get(i,t);
      result += t.HashValue();
    }
  }
  return result;
}

Attribute* JListTID::Clone() const
{
  return new JListTID(*this);
}


bool JListTID::Adjacent(const Attribute* attrib) const
{
  return false;
}


int JListTID::Compare(const Attribute* rhs) const
{
  JListTID* in = (JListTID*) rhs;
  return Compare(*in);
}

int JListTID::Compare(const JListTID& in) const
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
            int res = 0;
            TupleIdentifier t1, t2;
            for (int i = 0; i < elemlist.Size(); i++)
            {
              elemlist.Get(i,t1);
              in.elemlist.Get(i,t2);
              res = t1.Compare(t2);
              if (res != 0) return res;
            }
            return 0;
          }
        }
      }
    }
  }
}

int JListTID::NumOfFLOBs() const
{
  return 1;
}

Flob* JListTID::GetFLOB(const int n)
{
  if (n == 0) return &elemlist;
  else return 0;
}

void JListTID::Destroy()
{
  elemlist.Destroy();
}

size_t JListTID::Sizeof() const
{
  return sizeof(JListTID);
}

ostream& JListTID::Print(ostream& os) const
{
  os << "List ";
  if (IsDefined())
  {
    TupleIdentifier t;
    for(int i = 0; i < elemlist.Size(); i++)
    {
      os << i+1 << ". " ;
      elemlist.Get(i,t);
      t.Print(os);
      os << endl;
    }
    os << "end of list." << endl;
  }
  else
  {
    os << Symbol::UNDEFINED()<< endl;
  }
  return os;
}

const string JListTID::BasicType()
{
  return "jlisttid";
}

const bool JListTID::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Methods

*/


JListTID& JListTID::operator=(const JListTID& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    elemlist.copyFrom(other.elemlist);
  }
  return *this;
}

bool JListTID::operator==(const JListTID& other) const
{
  if (Compare(other) == 0) return true;
  else return false;
}

/*
1.5 Operators for Secondo Integration

*/

ListExpr JListTID::Out(ListExpr typeInfo, Word value)
{
  JListTID* source = (JListTID*) value.addr;
  if (source->IsDefined())
  {
    if(source->elemlist.Size() == 0) return nl->TheEmptyList();
    else
    {
      NList result(nl->TheEmptyList());
      TupleIdentifier e;
      bool first = true;
      for (int i = 0; i < source->elemlist.Size(); i++)
      {
        source->elemlist.Get(i,e);
        Word wt = SetWord(&e);
        NList tl(TupleIdentifier::Out(nl->TheEmptyList(), wt));
        if (first)
        {
          result = tl.enclose().enclose();
          first = false;
        }
        else
        {
          result.append(tl.enclose());
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


Word JListTID::In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct=true;
    return SetWord(Address(new JListTID(false)));
  }

  ListExpr rest = instance;
  ListExpr first = nl->TheEmptyList();
  correct = true;
  JListTID* in = new JListTID(true);
  while( !nl->IsEmpty( rest ) )
  {
    first = nl->First( rest );
    rest = nl->Rest( rest );
    Word wt = TupleIdentifier::In(nl->TheEmptyList(), nl->First(first),
                                  errorPos, errorInfo, correct);
    if (correct)
    {
      TupleIdentifier* t = (TupleIdentifier*) wt.addr;
      in->Append(*t);
      t->DeleteIfAllowed();
      t = 0;
    }
    else
    {
      cmsg.inFunError("Incorrect " + TupleIdentifier::BasicType()+ " in list.");
      in->DeleteIfAllowed();
      delete in;
      return SetWord(Address(0));
    }
  }
  return SetWord(in);
}

Word JListTID::Create(const ListExpr typeInfo)
{
  return SetWord(new JListTID(true));
}


void JListTID::Delete( const ListExpr typeInfo, Word& w )
{
 JListTID* obj = (JListTID*) w.addr;
 obj->DeleteIfAllowed();
 w.addr = 0;
}


void JListTID::Close( const ListExpr typeInfo, Word& w )
{
  ((JListTID*) w.addr)->DeleteIfAllowed();
  w.addr = 0;
}


Word JListTID::Clone( const ListExpr typeInfo, const Word& w )
{
  return new JListTID(*((JListTID*) w.addr));
}


void* JListTID::Cast( void* addr )
{
  return (new (addr) JListTID);
}


bool JListTID::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}



int JListTID::SizeOf()
{
  return sizeof(JListTID);
}

bool JListTID::Save(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  JListTID* obj = (JListTID*) value.addr;
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


bool JListTID::Open(SmiRecord& valueRecord, size_t& offset,
                    const ListExpr typeInfo, Word& value )
{
  JListTID* obj = new JListTID (true);
  valueRecord.Read( obj, obj->SizeOf(), offset );
  obj->del.refs = 1;
  obj->del.SetDelete();
  offset += obj->SizeOf();
  value = SetWord( obj );
  return true;
}

ListExpr JListTID::Property()
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
      nl->TextAtom("(" + TupleIdentifier::BasicType() + " ... " +
                    TupleIdentifier::BasicType() + "), " +
                    "a list of " + TupleIdentifier::BasicType() + "s, " +
                    "of the tuples of the in-, out-, adjacent, and reverse " +
                    "adjacent sections of a section or a junction."),
      nl->StringAtom("(154 252 357)")));
}

/*
1.6 Helpful Operators

*/

void JListTID::Append (const TupleIdentifier& e)
{
  elemlist.Append(e);
}

int JListTID::GetNoOfComponents() const
{
  return elemlist.Size();
}

void JListTID::Get(const int i, TupleIdentifier& e) const
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Get(i,e);
}

void JListTID::Put(const int i, const TupleIdentifier& e)
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Put(i,e);
}
