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

2011, October Simone Jandt

*/

#ifndef JLIST_H
#define JLIST_H

#include <ostream>
#include "ListUtils.h"
#include "NestedList.h"
#include "../../Tools/Flob/DbArray.h"
#include "Symbols.h"
#include "Attribute.h"
#include "../TupleIdentifier/TupleIdentifier.h"
#include "PairTID.h"
#include "NetDistanceGroup.h"

/*
1. class ~List~
Enables us to use a list of an Attributetype as attribute in relations.

*/

template<class ListElem>
class JList: public Attribute
{

public:

/*
1.1. Constructors and deconstructors

The default constructor should only be used in the cast-Function.

*/

JList();
JList(bool defined);
JList(const JList<ListElem>& other);
JList(const ListElem& ptidrloc);

~JList();

/*
1.2 Getter and Setter for private Attributes

*/

DbArray<ListElem> GetList() const;

void SetList(const DbArray<ListElem> inList);

/*
1.3 Override Methods from Attribute

*/

void CopyFrom(const Attribute* right);
StorageType GetStorageType() const;
size_t HashValue() const;
Attribute* Clone() const;
bool Adjacent(const Attribute* attrib) const;
int Compare(const Attribute* rhs) const;
int Compare(const JList<ListElem>& in) const;
int NumOfFLOBs () const;
Flob* GetFLOB(const int n);
void Destroy();
size_t Sizeof() const;
ostream& Print(ostream& os) const;
static const string BasicType();
static const bool checkType(const ListExpr type);

/*
1.4 Standard Methods

*/

JList<ListElem>& operator=(const JList<ListElem>& other);
bool operator==(const JList<ListElem>& other) const;

/*
1.5 Operators for Secondo Integration

*/

static ListExpr Out(ListExpr typeInfo, Word value);
static Word In(const ListExpr typeInfo, const ListExpr instance,
               const int errorPos, ListExpr& errorInfo, bool& correct);
static Word Create(const ListExpr typeInfo);
static void Delete( const ListExpr typeInfo, Word& w );
static void Close( const ListExpr typeInfo, Word& w );
static Word Clone( const ListExpr typeInfo, const Word& w );
static void* Cast( void* addr );
static bool KindCheck( ListExpr type, ListExpr& errorInfo );
static int SizeOf();
static bool Save(SmiRecord& valueRecord, size_t& offset,
                 const ListExpr typeInfo, Word& value );
static bool Open(SmiRecord& valueRecord, size_t& offset,
                   const ListExpr typeInfo, Word& value );
static ListExpr Property();
static string Example();

/*
1.6 Helpful Operators

*/

void Append (const ListElem& e);
int GetNoOfComponents() const;
void Get(const int i, ListElem& res) const;
void Put(const int i, const ListElem& p);

private:

DbArray<ListElem> elemlist;
};

typedef JList<PairTIDRLoc> ListPairTIDRLoc;
typedef JList<PairTIDRInt> ListPairTIDRInt;
typedef JList<NetDistanceGroup> ListNetDistGrp;
typedef JList<TupleIdentifier> JListTID;

/*
1 Implementation of class ~JPairTIDRLocList~

1.1. Constructors and deconstructors

*/

template<class ListElem>
JList<ListElem>::JList():Attribute()
{}

template<class ListElem>
JList<ListElem>::JList(bool defined): Attribute(defined), elemlist(0)
{}

template<class ListElem>
JList<ListElem>::JList(const JList<ListElem>& other) :
  Attribute(other.IsDefined()), elemlist(other.GetList().Size())
{
  if (other.IsDefined()) elemlist.copyFrom(other.GetList());
}

template<class ListElem>
JList<ListElem>::JList(const ListElem& inId) :
  Attribute(true), elemlist(0)
{
  elemlist.Append(inId);
}

template<class ListElem>
JList<ListElem>::~JList()
{}

/*
1.2 Getter and Setter for private Attributes

*/

template<class ListElem>
DbArray<ListElem> JList<ListElem>::GetList() const
{
  return elemlist;
}

template<class ListElem>
void JList<ListElem>::SetList(const DbArray<ListElem> inList)
{
  elemlist.copyFrom(inList);
}

/*
1.3 Override Methods from Attribute

*/

template<class ListElem>
void JList<ListElem>::CopyFrom(const Attribute* right)
{
  SetDefined(right->IsDefined());
  if (right->IsDefined())
  {
    JList<ListElem>* source = (JList<ListElem>*) right;
    elemlist.copyFrom(source->GetList());
  }
}

template<class ListElem>
Attribute::StorageType JList<ListElem>::GetStorageType() const
{
  return Default;
}

template<class ListElem>
size_t JList<ListElem>::HashValue() const
{
  size_t result = 0;
  if (IsDefined())
  {
    ListElem e;
    for(int i = 0; i < elemlist.Size(); i++)
    {
      elemlist.Get(i, e);
      result += e.HashValue();
    }
  }
  return result;
}

template<class ListElem>
Attribute* JList<ListElem>::Clone() const
{
  return new JList<ListElem>(*this);
}

template<class ListElem>
bool JList<ListElem>::Adjacent(const Attribute* attrib) const
{
  return false;
}

template<class ListElem>
int JList<ListElem>::Compare(const Attribute* rhs) const
{
  JList<ListElem>* in = (JList<ListElem>*) rhs;
  return Compare(*in);
}

template<class ListElem>
int JList<ListElem>::Compare(const JList<ListElem>& in) const
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
            ListElem p1,p2;
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

template<class ListElem>
int JList<ListElem>::NumOfFLOBs() const
{
  return 1;
}

template<class ListElem>
Flob* JList<ListElem>::GetFLOB(const int n)
{
  if (n == 0) return &elemlist;
  else return 0;
}

template<class ListElem>
void JList<ListElem>::Destroy()
{
  elemlist.Destroy();
}

template<class ListElem>
size_t JList<ListElem>::Sizeof() const
{
  return sizeof(JList<ListElem>);
}

template<class ListElem>
ostream& JList<ListElem>::Print(ostream& os) const
{
  os << "List: ";
  if (IsDefined())
  {
    ListElem p;
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

template<class ListElem>
const std::string JList<ListElem>::BasicType()
{
  return "list" + ListElem::BasicType();
}

template<class ListElem>
const bool JList<ListElem>::checkType(const ListExpr type)
{
  return listutils::isSymbol(type, BasicType());
}

/*
1.4 Standard Methods

*/

template<class ListElem>
JList<ListElem>& JList<ListElem>::operator=(const JList<ListElem>& other)
{
  SetDefined(other.IsDefined());
  if (other.IsDefined())
  {
    elemlist.copyFrom(other.GetList());
  }
    return *this;
}

template<class ListElem>
bool JList<ListElem>::operator==(const JList<ListElem>& other) const
{
  if (Compare(other) == 0) return true;
  else return false;
}

/*
1.5 Operators for Secondo Integration

*/

template<class ListElem>
ListExpr JList<ListElem>::Out(ListExpr typeInfo, Word value)
{
  JList<ListElem>* source = (JList<ListElem>*) value.addr;
  if (source->IsDefined())
  {
    if(source->elemlist.Size() == 0) return nl->TheEmptyList();
    else
    {
      NList result(nl->TheEmptyList());
      ListElem e;
      bool first = true;
      for (int i = 0; i < source->elemlist.Size(); i++)
      {
        source->elemlist.Get(i,e);
        Word w = SetWord(&e);
        NList elem(ListElem::Out(nl->TheEmptyList(), w));
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

template<class ListElem>
Word JList<ListElem>::In(const ListExpr typeInfo, const ListExpr instance,
                        const int errorPos, ListExpr& errorInfo, bool& correct)
{
  if(nl->IsEqual(instance,Symbol::UNDEFINED()))
  {
    correct=true;
    return SetWord(Address(new JList<ListElem>(false)));
  }

  ListExpr rest = instance;
  ListExpr first = nl->TheEmptyList();
  correct = true;
  JList<ListElem>* in = new JList<ListElem>(true);
  while( !nl->IsEmpty( rest ) )
  {
    first = nl->First( rest );
    rest = nl->Rest( rest );
    Word w = ListElem::In(nl->TheEmptyList(), first, errorPos,
                          errorInfo, correct);
    if (correct)
    {
      ListElem* e = (ListElem*) w.addr;
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

template<class ListElem>
Word JList<ListElem>::Create(const ListExpr typeInfo)
{
  return SetWord(new JList<ListElem>(false));
}

template<class ListElem>
void JList<ListElem>::Delete( const ListExpr typeInfo, Word& w )
{
  ((JList<ListElem>*) w.addr)->Destroy();
  delete ((JList<ListElem>*) w.addr);
  w.addr = 0;
}

template<class ListElem>
void JList<ListElem>::Close( const ListExpr typeInfo, Word& w )
{
  delete ((JList<ListElem>*) w.addr);
  w.addr = 0;
}

template<class ListElem>
Word JList<ListElem>::Clone( const ListExpr typeInfo, const Word& w )
{
  return new JList<ListElem>(*((JList<ListElem>*) w.addr));
}

template<class ListElem>
void* JList<ListElem>::Cast( void* addr )
{
  return (new (addr) JList<ListElem>);
}

template<class ListElem>
bool JList<ListElem>::KindCheck( ListExpr type, ListExpr& errorInfo )
{
  return checkType(type);
}

template<class ListElem>
int JList<ListElem>::SizeOf()
{
  return sizeof(JList<ListElem>);
}

template<class ListElem>
bool JList<ListElem>::Save(SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value )
{
  JList<ListElem>* obj = (JList<ListElem>*) value.addr;
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

template<class ListElem>
bool JList<ListElem>::Open(SmiRecord& valueRecord, size_t& offset,
                          const ListExpr typeInfo, Word& value )
{
  JList<ListElem>* obj = new JList<ListElem> (true);
  valueRecord.Read( obj, obj->SizeOf(), offset );
  obj->del.refs = 1;
  obj->del.SetDelete();
  offset += obj->SizeOf();
  value = SetWord( obj );
  return true;
}

template<class ListElem>
ListExpr JList<ListElem>::Property()
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
      nl->TextAtom("("+ ListElem::BasicType() + " ... " +
                     ListElem::BasicType() + "), list of " +
                     ListElem::BasicType()+"."),
      nl->TextAtom("("+ Example() + ")")));
}

template<class ListElem>
string JList<ListElem>::Example(){
  return ListElem::Example();
}

/*
1.6 Helpful Operators

*/

template<class ListElem>
void JList<ListElem>::Append (const ListElem& e)
{
  elemlist.Append(e);
}

template<class ListElem>
int JList<ListElem>::GetNoOfComponents()const
{
  return elemlist.Size();
}

template<class ListElem>
void JList<ListElem>::Get(const int i, ListElem& res) const
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Get(i, res);
}

template<class ListElem>
void JList<ListElem>::Put(const int i, const ListElem& p)
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Put(i,p);
}

#endif // JLIST_H
