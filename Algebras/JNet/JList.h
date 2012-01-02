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
#include "StandardTypes.h"

/*
1. class ~List~

Enables us to use a sorted list of an Attributetype from kind DATA as attribute
in relations.

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


void StartBulkload();
void EndBulkload();

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
void Clear();
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
bool operator!=(const JList<ListElem>& other) const;
bool operator<(const JList<ListElem>& other) const;
bool operator<=(const JList<ListElem>& other) const;
bool operator>(const JList<ListElem>& other) const;
bool operator>=(const JList<ListElem>& other) const;

/*
Add Elements to list. May only be used during Bulkload.

*/
JList<ListElem>& operator+=(const ListElem& e);
JList<ListElem>& operator+=(const JList<ListElem>& other);

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

1.1.1 ~GetNoOfComponents~

Returns the number of list elements.

*/

int GetNoOfComponents() const;

/*
1.1.1 ~Get~

Returns the element at position ~i~ as ~res~.

*/

void Get(const int i, ListElem& res) const;

/*
1.1.1 ~isEmpty~

Returns true if the ~elemlist~ is empty or not defined.

*/

bool IsEmpty() const;

/*
1.1.1 ~Contains~

Checks if a ~e~ is in the list. If ~e~ is in the list then ~true~ is
returned and the position in the list is given by ~pos~. Elsewhere ~false~
is returned and ~pos~ is -1;

*/

bool Contains(const ListElem& e, int& pos);

/*
1.1.1 ~TrimToSize~

Reduces the list size to the number of elements.

*/

void TrimToSize();

/*
2 Private members and methods

*/

private:

DbArray<ListElem> elemlist;
bool activBulkload;

/*
1.1.1 ~Put~

Writes the data of ~p~ at position ~i~ of ~elemlist~. Private danger of
conflict with sorted criteria.

*/

void Put(const int i, const ListElem& p);

/*
1.1.1 ~RemoveDuplicates~

Duplicates will be removed from list.

*/

void RemoveDuplicates();

/*
1.1.1 ~Sort~

Sorts the elementlist and removes duplicates.

*/

void Sort();

};

/*
3 Definition of Listclasses.

*/

typedef JList<PairTIDRLoc> ListPairTIDRLoc;
typedef JList<PairTIDRInt> ListPairTIDRInt;
typedef JList<NetDistanceGroup> ListNetDistGrp;
typedef JList<TupleIdentifier> JListTID;
typedef JList<CcInt> JListInt;

/*
Internal helper function

*/

template<class ListElem>
int ListElemCompare (const void* a, const void* b)
{
  const ListElem* iA = (const ListElem*) a;
  const ListElem* iB = (const ListElem*) b;
  return iA->Compare(*iB);
}

/*
1 Implementation of class ~JList~

1.1. Constructors and deconstructors

*/

template<class ListElem>
JList<ListElem>::JList():Attribute()
{}

template<class ListElem>
JList<ListElem>::JList(bool defined): Attribute(defined), elemlist(0),
  activBulkload(false)
{}

template<class ListElem>
JList<ListElem>::JList(const JList<ListElem>& other) :
  Attribute(other.IsDefined()), elemlist(other.GetList().Size()),
  activBulkload(false)
{
  if (other.IsDefined())
  {
    elemlist.copyFrom(other.GetList());
  }
}

template<class ListElem>
JList<ListElem>::JList(const ListElem& inId) :
  Attribute(true), elemlist(0), activBulkload(false)
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
  assert(IsDefined());
  return elemlist;
}

template<class ListElem>
void JList<ListElem>::SetList(const DbArray<ListElem> inList)
{
  elemlist.copyFrom(inList);
  Sort();
}

template<class ListElem>
void JList<ListElem>::StartBulkload()
{
  activBulkload = true;
}

template<class ListElem>
void JList<ListElem>::EndBulkload()
{
  if (!IsDefined())
  {
    elemlist.clean();
    activBulkload = false;
  }
  else
  {
    Sort();
    RemoveDuplicates();
    TrimToSize();
  }
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
    activBulkload = source->activBulkload;
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
            assert(!activBulkload && !in.activBulkload);
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
void JList<ListElem>::Clear()
{
  elemlist.clean();
  activBulkload = false;
  SetDefined(true);
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
  os << "List: " << endl;
  if (IsDefined())
  {
    ListElem p;
    for (int i = 0; i < elemlist.Size(); i++)
    {
      os << i << ". ";
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
    activBulkload = other.activBulkload;
  }
  return *this;
}

template<class ListElem>
bool JList<ListElem>::operator==(const JList<ListElem>& other) const
{
  if (Compare(other) == 0) return true;
  else return false;
}

template<class ListElem>
bool JList<ListElem>::operator!=(const JList< ListElem >& other) const
{
  if (Compare(other) != 0) return true;
  else return false;
}

template<class ListElem>
bool JList<ListElem>::operator<(const JList< ListElem >& other) const
{
  if (Compare(other) < 0) return true;
  else return false;
}


template<class ListElem>
bool JList<ListElem>::operator<=(const JList<ListElem>& other) const
{
  if (Compare(other)<1) return true;
  else return false;
}

template<class ListElem>
bool JList<ListElem>::operator>(const JList<ListElem>& other) const
{
  if (Compare(other) > 0 ) return true;
  else return false;
}

template<class ListElem>
bool JList<ListElem>::operator>=(const JList<ListElem>& other) const
{
  if (Compare(other) > -1) return true;
  else return false;
}

template<class ListElem>
JList<ListElem>& JList<ListElem>::operator+=(const ListElem& e)
{
  assert (activBulkload); //May only be used while bulkload is activated!
  if (IsDefined() && e.IsDefined())
  {
    elemlist.Append(e);
  }
  else
  {
    if (!IsDefined() && e.IsDefined())
    {
      SetDefined(true);
      elemlist.clean();
      elemlist.Append(e);
    }
  }
  return *this;
}

template<class ListElem>
JList<ListElem>& JList<ListElem>::operator+=(const JList<ListElem>& l)
{
  assert(activBulkload);//May only be used while bulkload is activated!
  if(IsDefined() && l.IsDefined())
  {
    ListElem curElem;
    for (int i = 0; i < l.GetNoOfComponents(); i++)
    {
      l.Get(i,curElem);
      elemlist.Append(curElem);
    }
  }
  else
  {
    if (!IsDefined() && l.IsDefined()) *this = l;
  }
  return *this;
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
  bool firstElem = true;
  ListElem lastElem;
  in->StartBulkload();
  while( !nl->IsEmpty( rest ) )
  {
    first = nl->First( rest );
    rest = nl->Rest( rest );
    Word w = ListElem::In(nl->TheEmptyList(), first, errorPos,
                          errorInfo, correct);
    if (correct)
    {
      ListElem* e = (ListElem*) w.addr;
      if (firstElem) lastElem = *e;
      in->operator+=(*e);
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
  in->EndBulkload();
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
int JList<ListElem>::GetNoOfComponents()const
{
  if (IsDefined()) return elemlist.Size();
  else return -1;
}

template<class ListElem>
void JList<ListElem>::Get(const int i, ListElem& res) const
{
  if (!IsDefined() || IsEmpty()) res.SetDefined(false);
  else
  {
    assert (0 <= i && i < elemlist.Size());
    elemlist.Get(i, res);
  }
}

template<class ListElem>
void JList<ListElem>::Put(const int i, const ListElem& p)
{
  assert (0 <= i && i < elemlist.Size());
  elemlist.Put(i,p);
}

template<class ListElem>
bool JList<ListElem>::Contains(const ListElem& e, int& pos)
{
  if(!IsDefined()) return false;
  else return elemlist.Find(e, Compare, pos);
}

template<class ListElem>
bool JList<ListElem>::IsEmpty() const
{
  if (IsDefined()) return elemlist.Size() == 0;
  else return true;
}

template<class ListElem>
void JList<ListElem>::Sort()
{
  activBulkload = !elemlist.Sort( ListElemCompare<ListElem> );
}

template<class ListElem>
void JList<ListElem>::RemoveDuplicates()
{
  if (IsDefined() && GetNoOfComponents()>1 && !activBulkload)
  {
    ListElem lastElem, curElem;
    DbArray<ListElem> help(elemlist.Size());
    help.copyFrom(elemlist);
    elemlist.clean();
    help.Get(0,lastElem);
    if (lastElem.IsDefined()) elemlist.Append(lastElem);
    for (int i = 1; i < help.Size(); i++)
    {
      help.Get(i,curElem);
      if (curElem.IsDefined() && lastElem.Compare(curElem) != 0)
      {
        elemlist.Append(curElem);
        lastElem = curElem;
      }
    }
    elemlist.TrimToSize();
    help.Destroy();
  }
}


template<class ListElem>
void JList<ListElem>::TrimToSize()
{
  elemlist.TrimToSize();
}


#endif // JLIST_H
