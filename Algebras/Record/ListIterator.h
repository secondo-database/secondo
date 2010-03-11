/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software{} you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation{} either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY{} without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO{} if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/


#ifndef __LIST_ITERATOR
#define __LIST_ITERATOR


#include "NestedList.h"
#include "NList.h"


// An iterator for 'listExpr' that allows the programmer
// to traverse the list
class ListIterator
{
  public:

    //Constructors
    ListIterator(const NList& list);
    ListIterator(const ListExpr& list);
    ListIterator(const ListIterator& iter);

    //Assign operators
    ListIterator& operator=(const NList& list);
    ListIterator& operator=(const ListExpr& list);
    ListIterator& operator=(const ListIterator& iter);

    // Returns 'true' if the iteration has more elements.
    bool HasNext() const;

    // Returns the next element in the iteration as 'ListExpr'.
    ListExpr NextListExpr();

    // Returns the next element in the iteration as 'NList'.
    NList NextNList();

  private:
    ListExpr listExpr;
};


inline
ListIterator::ListIterator(const NList& list)
  : listExpr(list.listExpr())
{}


inline
ListIterator::ListIterator(const ListExpr& list)
  : listExpr(list)
{}


inline
ListIterator::ListIterator(const ListIterator& iter)
  : listExpr(iter.listExpr)
{}


inline ListIterator&
ListIterator::operator=(const NList& list)
{
  this->listExpr = list.listExpr();
  return *this;
}


inline ListIterator&
ListIterator::operator=(const ListExpr& list)
{
  this->listExpr = list;
  return *this;
}


inline ListIterator&
ListIterator::operator=(const ListIterator& iter)
{
  this->listExpr = iter.listExpr;
  return *this;
}


inline bool
ListIterator::HasNext() const
{
  return (nl->IsEmpty(this->listExpr) == false);
}


inline ListExpr
ListIterator::NextListExpr()
{
  ListExpr next;

  if (nl->IsEmpty(this->listExpr)) {
    next = this->listExpr;
  } else {
    next = nl->First(this->listExpr);
    this->listExpr = nl->Rest(this->listExpr);
  }

  return next;
}


inline NList
ListIterator::NextNList()
{
  return NList(this->NextListExpr());
}


#endif // __LIST_ITERATOR

