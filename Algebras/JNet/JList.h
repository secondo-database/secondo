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

2012, May Simone Jandt

1 Defines and Includes

*/

#ifndef JLIST_H
#define JLIST_H

#include <ostream>
#include "ListUtils.h"
#include "NestedList.h"
#include "../../Tools/Flob/DbArray.h"
#include "Symbols.h"
#include "Attribute.h"
#include "StandardTypes.h"
#include "JRouteInterval.h"
#include "RouteLocation.h"
#include "NetDistanceGroup.h"


/*
1 class ~List~

Template class that enables us to use a sorted list of an Attribute of
kind DATA as attribute in relations.

*/

template<class ListElem>
class JList: public Attribute
{

/*
1.1 Public deklarations

*/
public:

/*
1.1.1. Constructors and deconstructors

*/

explicit JList(bool defined);
JList(const JList<ListElem>& other);
explicit JList(const ListElem& elem);

~JList();

/*
1.1.1 Getter and Setter for private Attributes

*/

DbArray<ListElem> GetList() const;

void SetList(const DbArray<ListElem> inList);

/*
1.1.1 Overwrite Methods from Attribute

*/

void CopyFrom(const Attribute* right);
StorageType GetStorageType() const;
size_t HashValue() const;
Attribute* Clone() const;
bool Adjacent(const Attribute* attrib) const;
static int Compare(const void* ls, const void* rs);
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
1.1.1 Standard Methods

*/

JList<ListElem>& operator=(const JList<ListElem>& other);

bool operator==(const JList<ListElem>& other) const;
bool operator!=(const JList<ListElem>& other) const;
bool operator<(const JList<ListElem>& other) const;
bool operator<=(const JList<ListElem>& other) const;
bool operator>(const JList<ListElem>& other) const;
bool operator>=(const JList<ListElem>& other) const;

JList<ListElem>& operator+=(const ListElem& e);
JList<ListElem>& operator+=(const JList<ListElem>& other);

JList<ListElem>& operator-=(const ListElem& e);
JList<ListElem>& operator-=(const JList<ListElem>& other);


/*
1.1.1 Operators for Secondo Integration

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
static ListExpr Property();
static string Example();

/*
1.1.1 Helpful Operators

1.1.1.1 ~GetNoOfComponents~

Returns the number of list elements.

*/

int GetNoOfComponents() const;

/*
1.1.1.1 ~Get~

Returns the element at position ~i~ as ~res~.

*/

void Get(const int i, ListElem& res) const;
void Get(const int i, ListElem* res) const;

/*
1.1.1.1 ~isEmpty~

Returns true if the ~elemlist~ is empty or not defined.

*/

bool IsEmpty() const;

/*
1.1.1.1 ~Contains~

Checks if a ~e~ is in the list. If ~e~ is in the list then ~true~ is
returned and the position in the list is given by ~pos~. Elsewhere ~false~
is returned and ~pos~ is -1;

*/

bool Contains(const ListElem& e, int& pos);

/*
1.1.1.1 ~TrimToSize~

Reduces the list size to the number of elements.

*/

void TrimToSize();

/*
1.1.1.1 ~Restrict~

Restricts the list to the given values

*/

JList<ListElem>& Restrict(const ListElem& sub);
JList<ListElem>& Restrict(const JList<ListElem>& sub);

/*
1.1.1.1 Controll Bulkload of lists

StartBulkload enables insertion of elements in random order into an empty list.

*/

void StartBulkload();

/*
EndBulkload ensures that the list elements inserted during bulkload are pretty
sorted.

*/

void EndBulkload();

/*
1.1 Private declarations

*/

private:

/*
1.1.1 Attributes

*/

DbArray<ListElem> elemlist;
bool activBulkload;

/*
1.1.1 Default Constructors

Private because should only be used in Cast-Function.

*/

JList();

/*
1.1.1. Operations

1.1.1.1 ~Put~

Writes the data of ~p~ at position ~i~ of ~elemlist~. Private danger of
conflict with sorted criteria.

*/

bool Put(const int i, const ListElem& p);

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
1 Definition of Listclasses.

*/

typedef JList<CcInt> JListInt;
typedef JList<JRouteInterval> JListRInt;
typedef JList<RouteLocation> JListRLoc;
typedef JList<NetDistanceGroup> JListNDG;


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
    elemlist.copyFrom(other.GetList());
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
1.1 Getter and Setter for private Attributes

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
  RemoveDuplicates();
}

template<class ListElem>
void JList<ListElem>::StartBulkload()
{
  SetDefined(true);
  activBulkload = true;
  elemlist.clean();
  elemlist.TrimToSize();
}

template<class ListElem>
void JList<ListElem>::EndBulkload()
{
  if (!IsDefined())
  {
    Clear();
    SetDefined(false);
  }
  else
  {
    activBulkload = false;
    Sort();
    RemoveDuplicates();
    TrimToSize();
  }
}


/*
1.1 Override Methods from Attribute

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
    ListElem e(false);
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
int JList<ListElem>::Compare(const void* ls, const void* rs)
{
  JList<ListElem> lhs(*(JList<ListElem>*) ls);
  JList<ListElem> rhs(*(JList<ListElem>*) rs);
  return lhs.Compare(rhs);
}

template<class ListElem>
int JList<ListElem>::Compare(const Attribute* rhs) const
{
  JList<ListElem> in(*(JList<ListElem>*) rhs);
  return Compare(in);
}

template<class ListElem>
int JList<ListElem>::Compare(const JList<ListElem>& in) const
{
  assert(!activBulkload && !in.activBulkload);
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
            ListElem p1(false);
            ListElem p2(false);
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
  if (IsDefined())
  {
    os << "Begin List: " << endl;
    ListElem p(false);
    for (int i = 0; i < elemlist.Size(); i++)
    {
      os << i << ". ";
      elemlist.Get(i,p);
      p.Print(os);
    }
    os << endl << "end of List." << endl;
  }
  else
  {
    os << "List is " <<  Symbol::UNDEFINED() << endl;
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
  return (Compare(other) == 0);
}

template<class ListElem>
bool JList<ListElem>::operator!=(const JList< ListElem >& other) const
{
  return (Compare(other) != 0);
}

template<class ListElem>
bool JList<ListElem>::operator<(const JList< ListElem >& other) const
{
  return (Compare(other) < 0);
}


template<class ListElem>
bool JList<ListElem>::operator<=(const JList<ListElem>& other) const
{
  return (Compare(other)<1);
}

template<class ListElem>
bool JList<ListElem>::operator>(const JList<ListElem>& other) const
{
  return (Compare(other) > 0 );
}

template<class ListElem>
bool JList<ListElem>::operator>=(const JList<ListElem>& other) const
{
  return (Compare(other) > -1);
}

template<class ListElem>
JList<ListElem>& JList<ListElem>::operator+=(const ListElem& e)
{
  if (IsDefined() && e.IsDefined())
  {
    if(activBulkload)
      elemlist.Append(e);
    else
    {
      int pos = 0;
      elemlist.Find(&e, ListElem::Compare, pos);
      ListElem actElem, nextElem;
      elemlist.Get(pos,actElem);
      if (actElem.Compare(e) != 0)
      {
        nextElem = actElem;
        elemlist.Put(pos, e);
        pos++;
        while(pos < elemlist.Size())
        {
          elemlist.Get(pos, actElem);
          elemlist.Put(pos, nextElem);
          nextElem = actElem;
          pos++;
        }
        elemlist.Append(nextElem);
      }
    }
  }
  return *this;
}

template<class ListElem>
JList<ListElem>& JList<ListElem>::operator+=(const JList<ListElem>& l)
{
  if (IsDefined() && l.IsDefined())
  {
    DbArray<ListElem> source = l.GetList();
    if (activBulkload)
    {
      elemlist.Append(source);
    }
    else
    {
      int i = 0;
      int j = 0;
      ListElem actElem, actSource;
      DbArray<ListElem>* result = new DbArray<ListElem>(0);
      while (i < elemlist.Size() && j < source.Size())
      {
        elemlist.Get(i,actElem);
        source.Get(j,actSource);
        switch(actElem.Compare(actSource))
        {
          case -1:
          {
            result->Append(actElem);
            i++;
            break;
          }

          case 0:
          {
            result->Append(actElem);
            i++;
            j++;
            break;
          }

          case 1:
          {
            result->Append(actSource);
            j++;
            break;
          }

          default: //should never happen
          {
            assert(false);
            break;
          }
        }
      }
      while (i < elemlist.Size())
      {
        elemlist.Get(i, actElem);
        result->Append(actElem);
        i++;
      }
      while (j < source.Size())
      {
        elemlist.Get(j, actSource);
        result->Append(actSource);
        j++;
      }
      elemlist.clean();
      elemlist.copyFrom(*result);
      result->Destroy();
      delete result;
    }
  }
  return *this;
}

template<class ListElem>
JList<ListElem>& JList<ListElem>::operator-=(const ListElem& e)
{
  if (IsDefined() && e.IsDefined() && !IsEmpty())
  {
    int pos;
    if (elemlist.Find(&e, ListElem::Compare, pos)){
      ListElem actElem;
      elemlist.Get(pos, actElem);
      if (actElem == e)
      {
        pos++;
        while(pos < elemlist.Size())
        {
          elemlist.Get(pos, actElem);
          elemlist.Put(pos-1, actElem);
          pos++;
        }
        elemlist.resize(elemlist.Size()-1);
      }
    }
  }
  return *this;
}

template<class ListElem>
JList<ListElem>& JList<ListElem>::operator-=(const JList<ListElem>& l)
{
  if (IsDefined() && l.IsDefined() && !IsEmpty() && !l.IsEmpty())
  {
    int i = 0;
    int j = 0;
    ListElem actElem, actSource;
    DbArray<ListElem>* result = new DbArray<ListElem>(0);
    DbArray<ListElem> source = l.GetList();
    while (i < elemlist.Size() && j < source.Size())
    {
      elemlist.Get(i,actElem);
      source.Get(j,actSource);
      switch(actElem.Compare(actSource))
      {
        case -1:
        {
          result->Append(actElem);
          i++;
          break;
        }

        case 0:
        {
          j++;
          i++;
          break;
        }

        case 1:
        {
          j++;
          break;
        }

        default: //should never happen
        {
          assert(false);
          break;
        }
      }
    }
    while(i < elemlist.Size())
    {
      elemlist.Get(i,actElem);
      result->Append(actElem);
      i++;
    }
    elemlist.clean();
    elemlist.copyFrom(*result);
    result->Destroy();
    delete result;
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
      ListElem e(false);
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
    return SetWord(new JList<ListElem>(false));
  }

  ListExpr rest = instance;
  ListExpr first = nl->TheEmptyList();
  correct = true;
  JList<ListElem>* in = new JList<ListElem>(true);
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
1.1 Helpful Operators

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
void JList<ListElem>::Get(const int i, ListElem* res) const
{
  if (!IsDefined() || IsEmpty()) res->SetDefined(false);
  else
  {
    assert (0 <= i && i < elemlist.Size());
    elemlist.Get(i, res);
  }
}

template<class ListElem>
bool JList<ListElem>::Put(const int i, const ListElem& p)
{
  assert (0 <= i && i < elemlist.Size());
  ListElem right(false);
  ListElem left(false);
  if (i > 0)
    Get(i-1,left);
  if (i < elemlist.Size()-1)
    Get(i+1,right);
  if ((left.IsDefined() && right.IsDefined() && left < p && p < right) ||
      (left.IsDefined() && !right.IsDefined() && left < p) ||
      (!left.IsDefined() && right.IsDefined() && p < right))
  {
    elemlist.Put(i,p);
    return true;
  }
  else
    return false;
}

template<class ListElem>
bool JList<ListElem>::Contains(const ListElem& e, int& pos)
{
  if(!IsDefined()) return false;
  else return elemlist.Find(&e,ListElem::Compare, pos);
}

template<class ListElem>
bool JList<ListElem>::IsEmpty() const
{
  if (IsDefined())
    return (elemlist.Size() == 0);
  else
    return true;
}

template<class ListElem>
void JList<ListElem>::Sort()
{
  activBulkload = !elemlist.Sort(ListElem::Compare);
}

template<class ListElem>
void JList<ListElem>::RemoveDuplicates()
{
  if (IsDefined() && GetNoOfComponents() > 1 && !activBulkload)
  {
    ListElem lastElem(false);
    ListElem curElem(false);
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
    help.Destroy();
    TrimToSize();
  }
}


template<class ListElem>
void JList<ListElem>::TrimToSize()
{
  elemlist.TrimToSize();
}

/*
1.1.1 ~Restrict~

*/

template<class ListElem>
JList<ListElem>& JList<ListElem>::Restrict(const ListElem& sub)
{
  if (IsDefined() && sub.IsDefined() && !IsEmpty())
  {
    int pos = 0;
    if (Contains(sub,pos))
    {
      elemlist.clean();
      elemlist.Append(sub);
    }
    else
      elemlist.clean();
    TrimToSize();
  }
  return *this;
}

template<class ListElem>
JList<ListElem>& JList<ListElem>::Restrict(const JList<ListElem>& sub)
{
  if (IsDefined() && sub.IsDefined() && !IsEmpty())
  {
    int i = 0;
    int j = 0;
    ListElem actElem, actSource;
    DbArray<ListElem>* result = new DbArray<ListElem>(0);
    DbArray<ListElem> source = sub.GetList();
    while (i < elemlist.Size() && j < source.Size())
    {
      elemlist.Get(i,actElem);
      source.Get(j,actSource);
      switch(actElem.Compare(actSource))
      {
        case -1:
        {
          i++;
          break;
        }

        case 0:
        {
          result->Append(actElem);
          j++;
          i++;
          break;
        }

        case 1:
        {
          j++;
          break;
        }

        default: //should never happen
        {
          assert(false);
          break;
        }
      }
    }
    elemlist.clean();
    elemlist.copyFrom(*result);
    result->Destroy();
    delete result;
  }
  return *this;
}

/*
1 Overwrite output operator

*/

template<class ListElem>
ostream& operator<<(ostream& os, const JList<ListElem>& l)
{
  l.Print(os);
  return os;
}

#endif // JLIST_H

