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

1. Includes

*/

#include "PQManagement.h"

/*
1 Implementation of class ~PQManagement~

1.1 Constructors and Deconstructors

*/

PQManagement::PQManagement(const int a) :
  visited(0), pq(0), firstFreeVisited(0), firstFreePQ(0)
{}

PQManagement::~PQManagement()
{}

/*
1.1. Insert

*/

void PQManagement::Insert(const JPQEntry& e)
{
  int visitedPos = 0;
  bool visitedLeft = true;
  //Checks if the end junction of the new entry has already been used
  if (FindVisited(e, visitedPos, visitedLeft))
  {
    //Found before. Check if it is still in priority queue
    VisitedJunctionTreeElement actVisitedElem = GetVisited(visitedPos);
    int pqPos = actVisitedElem.GetValue().GetIndexPQ();
    if ( pqPos > -1)
    {
      //still in pq check if the new distance is smaller than the old one.
      JPQEntryTreeElement actPQEntry = GetPQ(pqPos);
      if (actPQEntry.GetValue().GetPriority() > e.GetPriority())
      {
        //Update actPQEntry with values from e
        RemoveFromPQ(actPQEntry, pqPos);
        pqPos = InsertPQ(e);
        actVisitedElem.SetValue(VisitedJunction(e, pqPos));
        visited.Put(visitedPos, actVisitedElem);
      }
    }
    //in all other cases we do nothing
  }
  else
  {
    //New node found insert in pq and visited junctions
    int pqPos = InsertPQ(e);
    InsertVisited(VisitedJunction(e,pqPos), visitedPos, visitedLeft);
  }
}

int PQManagement::InsertPQ(const JPQEntry& entry)
{
  int pos = firstFreePQ;
  pq.Put(firstFreePQ, JPQEntryTreeElement(entry));
  firstFreePQ++;
  if (pos != 0)
  {
    int fatherPos = 0;
    bool fatherLeft = true;
    JPQEntryTreeElement father = GetFather(entry, pos, fatherPos, fatherLeft);
    if (fatherLeft)
      father.SetLeftSon(pos);
    else
      father.SetRightSon(pos);
    pq.Put(fatherPos, father);
  }
  return pos;
}

void PQManagement::InsertVisited(const VisitedJunction& elem,
                                 const int visitedPos,
                                 const bool visitedLeft)
{
  int pos = firstFreeVisited;
  visited.Put(firstFreeVisited, VisitedJunctionTreeElement(elem));
  firstFreeVisited++;
  if (pos != 0)
  {
    VisitedJunctionTreeElement father = GetVisited(visitedPos);
    if (visitedLeft)
      father.SetLeftSon(pos);
    else
      father.SetRightSon(pos);
    visited.Put(visitedPos,father);
  }
}

/*
1.1. GetAndDeleteMin

*/

JPQEntry* PQManagement::GetAndDeleteMin()
{
  if (!IsEmptyPQ())
  {
    int pos = 0;
    JPQEntryTreeElement minElem;
    PQGetMin(pos, minElem);
    RemoveFromPQ(minElem, pos);
    return new JPQEntry(minElem.GetValue());
  }
  else
    return 0;
}

/*
1.1 IsEmpty

*/

bool PQManagement::IsEmptyPQ() const
{
  return (firstFreePQ == 0);
}

bool PQManagement::IsEmptyVisited() const
{
  return (firstFreeVisited == 0);
}

/*
1.1 FindVisited

*/

bool PQManagement::FindVisited(const JPQEntry& e, int& visitedPos,
                               bool& visitedLeft) const
{
  if (IsEmptyVisited())
  {
    visitedPos = -1;
    return false;
  }
  else
  {
    assert(0 <= visitedPos && visitedPos < firstFreeVisited);
    VisitedJunctionTreeElement actElem = GetVisited(visitedPos);
    switch(actElem.GetValue().CompareEndJID(e.GetEndPartJID()))
    {
      case -1:
      {
        if (actElem.GetRightSon() != -1)
        {
          visitedPos = actElem.GetRightSon();
          return FindVisited(e,visitedPos,visitedLeft);
        }
        else
        {
          visitedLeft = false;
          return false;
        }
        break;
      }
      case 1:
      {
        if (actElem.GetLeftSon() != -1)
        {
          visitedPos = actElem.GetLeftSon();
          return FindVisited(e,visitedPos,visitedLeft);
        }
        else
        {
          visitedLeft = true;
          return false;
        }
        break;
      }
      case 0:
      {
        return true;
        break;
      }
      default: // should never been reached
      {
        assert(false);
        visitedPos = -1;
        return false;
        break;
      }
    }
  }
}

/*
1.1 Get

*/

VisitedJunctionTreeElement PQManagement::GetVisited(const int pos) const
{
  assert(0 <= pos && pos < firstFreeVisited);
  VisitedJunctionTreeElement elem;
  visited.Get(pos,elem);
  return elem;
}

JPQEntryTreeElement PQManagement::GetPQ(const int pos) const
{
  assert(0 <= pos && pos < firstFreePQ);
  JPQEntryTreeElement elem;
  pq.Get(pos,elem);
  return elem;
}

/*
1.1 RemoveFromPQ

*/

void PQManagement::RemoveFromPQ(JPQEntryTreeElement& elem, int pos)
{
  if (pos != 0)
  {
    int fatherPos = 0;
    bool fatherLeft = true;
    JPQEntryTreeElement father = GetFather(elem.GetValue(), pos, fatherPos,
                                           fatherLeft);
    if (elem.GetLeftSon() == -1)
    {
      if (elem.GetRightSon() == -1)
      {
        if (fatherLeft)
          father.SetLeftSon(-1);
        else
          father.SetRightSon(-1);
      }
      else
      {
        if (fatherLeft)
          father.SetLeftSon(elem.GetRightSon());
        else
          father.SetRightSon(elem.GetRightSon());
      }
      pq.Put(fatherPos,father);
    }
    else
    {
      if (elem.GetRightSon() == -1)
      {
        if (fatherLeft)
          father.SetLeftSon(elem.GetLeftSon());
        else
          father.SetRightSon(elem.GetLeftSon());
        pq.Put(fatherPos,father);
      }
      else
      {
        int minPos = elem.GetRightSon();
        JPQEntryTreeElement minElem;
        PQGetMin(minPos,minElem);
        SwapPQ(elem, pos, minElem, minPos);
        RemoveFromPQ(elem,minPos);
      }
    }
  }
  else
  {
    if (elem.GetLeftSon() == -1)
    {
      if (elem.GetRightSon() == -1)
        firstFreePQ = 0;
      else
      {
        int minPos = elem.GetRightSon();
        JPQEntryTreeElement minElem;
        PQGetMin(minPos, minElem);
        SwapPQ(elem, pos, minElem, minPos);
        RemoveFromPQ(elem, minPos);
      }
    }
    else
    {
      if (elem.GetRightSon() == -1)
      {
        int maxPos = elem.GetLeftSon();
        JPQEntryTreeElement maxElem;
        PQGetMax(maxPos, maxElem);
        SwapPQ(elem, pos, maxElem, maxPos);
        RemoveFromPQ(elem,maxPos);
      }
      else
      {
        int minPos = elem.GetRightSon();
        JPQEntryTreeElement minElem;
        PQGetMin(minPos, minElem);
        SwapPQ(elem, pos, minElem, minPos);
        RemoveFromPQ(elem,minPos);
      }
    }
  }
}

/*
1.1 GetFather

*/

JPQEntryTreeElement PQManagement::GetFather(const JPQEntry& elem, const int pos,
                                            int& fatherPos,
                                            bool& fatherLeft) const
{
  assert(0 <= fatherPos && fatherPos < firstFreePQ &&
         0 < pos && pos < firstFreePQ);

  JPQEntryTreeElement actElem = GetPQ(fatherPos);
  switch(actElem.GetValue().Compare(elem))
  {
    case -1:
    {
      if (actElem.GetRightSon() != -1)
      {
        if (actElem.GetRightSon() != pos)
        {
          fatherPos = actElem.GetRightSon();
          return GetFather(elem, pos, fatherPos, fatherLeft);
        }
      }
      fatherLeft = false;
      return actElem;
      break;
    }
    case 1:
    {
      if (actElem.GetLeftSon() != -1)
      {
        if (actElem.GetLeftSon() != pos)
        {
          fatherPos = actElem.GetLeftSon();
          return GetFather(elem, pos, fatherPos, fatherLeft);
        }
      }
      fatherLeft = true;
      return actElem;
      break;
    }
    default:
    {
      //should never been reached
      assert(false);
      break;
    }
  }
}

/*
1.1 PQGet

*/

void PQManagement::PQGetMin(int& pos, JPQEntryTreeElement& elem) const
{
  assert(0 <= pos && pos < firstFreePQ);
  elem = GetPQ(pos);
  while(elem.GetLeftSon() != -1)
  {
    pos = elem.GetLeftSon();
    elem = GetPQ(pos);
  }
}

void PQManagement::PQGetMax(int& pos, JPQEntryTreeElement& elem) const
{
  assert(0 <= pos && pos < firstFreePQ);
  elem = GetPQ(pos);
  while(elem.GetRightSon() != -1)
  {
    pos = elem.GetRightSon();
    elem = GetPQ(pos);
  }
}

/*
1.1 SwapPQ

*/

void PQManagement::SwapPQ(JPQEntryTreeElement& elem, const int pos,
                          JPQEntryTreeElement& minElem, const int minPos)
{
  JPQEntry help = elem.GetValue();
  elem.SetValue(minElem.GetValue());
  minElem.SetValue(help);
  pq.Put(pos, elem);
  pq.Put(minPos, minElem);
}

/*
1.1 IsEmpty

*/

bool PQManagement::IsEmpty() const
{
  return IsEmptyPQ();
}

/*
1.1 Destroy

*/

void PQManagement::Destroy()
{
  pq.Destroy();
  visited.Destroy();
}


/*
1.1 Print

*/

ostream& PQManagement::Print(ostream& os) const
{
  os << "priority queue: " << endl;
  JPQEntryTreeElement curElem;
  for (int i = 0; i < firstFreePQ; i++)
  {
    pq.Get(i,curElem);
    curElem.Print(os);
  }
  os << "visited junctions: " << endl;
  VisitedJunctionTreeElement actElem;
  for (int i = 0; i < firstFreeVisited; i++)
  {
    visited.Get(i,actElem);
    actElem.Print(os);
  }
  return os;
}

/*
1 Overload output operator

*/

ostream& operator<<(ostream& os, const PQManagement& e)
{
  e.Print(os);
  return os;
}
