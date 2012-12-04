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

#include <limits>
#include "PQManagement.h"

using namespace jnetwork;

/*
1 Implementation of class ~PQManagement~

1.1 Constructors and Deconstructors

*/

PQManagement::PQManagement() :
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
    if ( pqPos > 0)
    {
      //still in pq check if the new distance is smaller than the old one.
      JPQEntryTreeElement actPQEntry = GetPQ(pqPos);
      if (actPQEntry.GetValue().GetPriority() > e.GetPriority())
      {
        //Update actPQEntry with values from e
        SetPQ(pqPos, e);
        actVisitedElem.SetValue(VisitedJunction(e, pqPos));
        SetVisited(visitedPos, actVisitedElem);
        actPQEntry = GetPQ(pqPos);
        CorrectPositionUp(actPQEntry, pqPos);
      }
    }
    //in all other cases we do nothing
  }
  else
  {
    //New node found insert in pq and visited junctions
    int pqPos = InsertPQ(e);
    InsertVisited(VisitedJunction(e,pqPos), visitedPos, visitedLeft);
    if (pqPos > 0)
    {
      JPQEntryTreeElement newEntry = GetPQ(pqPos);
      CorrectPositionUp(newEntry, pqPos);
    }
  }
}

/*
1.1. GetAndDeleteMin

*/

JPQEntry* PQManagement::GetAndDeleteMin()
{
  if (!IsEmptyPQ())
  {
    int first = 0;
    JPQEntryTreeElement minElem = GetPQ(first);
    JPQEntry* result = new JPQEntry(minElem.GetValue());
    int last = firstFreePQ-1;
    JPQEntryTreeElement lastElem = GetPQ(last);
    JPQEntry root = lastElem.GetValue();
    minElem.SetValue(root);
    SetPQ(first, minElem);
    SetPQ(last, JPQEntryTreeElement(JPQEntry(Direction(Both),
                                            -1, -1, -1, -1, -1, -1,
                                            numeric_limits<double>::max(),
                                            numeric_limits<double>::max()),
                                   -1,-1));

    if (last > 0)
    {
      bool fatherLeft = true;
      int fatherPos = GetFatherPos(last,fatherLeft);
      JPQEntryTreeElement father = GetPQ(fatherPos);
      if (fatherLeft)
        father.SetLeftSon(-1);
      else
        father.SetRightSon(-1);
      SetPQ(fatherPos, father);
    }
    firstFreePQ--;
    int visitedPos = 0;
    bool visitedLeft = true;
    if (FindVisited(root, visitedPos, visitedLeft))
    {
      VisitedJunctionTreeElement rootAtVisited = GetVisited(visitedPos);
      VisitedJunction rootVisited = rootAtVisited.GetValue();
      rootVisited.SetIndexPQ(first);
      rootAtVisited.SetValue(rootVisited);
      SetVisited(visitedPos, rootAtVisited);
    }
    if (!IsEmptyPQ())
    {
      minElem = GetPQ(first);
      CorrectPositionDown(minElem, first);
    }
    return result;
  }
  else
    return 0;
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
    os << i << ".Elem: " << curElem << endl;
  }
  os << "visited junctions: " << endl;
  VisitedJunctionTreeElement actElem;
  for (int i = 0; i < firstFreeVisited; i++)
  {
    visited.Get(i,actElem);
    os << i << ".Elem: " << actElem << endl;
  }
  return os;
}

/*
1 Implementation of Private Methods

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
        visitedLeft = false;
        if (actElem.GetRightSon() != -1)
        {
          visitedPos = actElem.GetRightSon();
          return FindVisited(e,visitedPos,visitedLeft);
        }
        else
        {
          return false;
        }
        break;
      }
      case 1:
      {
        visitedLeft = true;
        if (actElem.GetLeftSon() != -1)
        {
          visitedPos = actElem.GetLeftSon();
          return FindVisited(e,visitedPos,visitedLeft);
        }
        else
        {
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
1.1 Getter and Setter

*/

VisitedJunctionTreeElement PQManagement::GetVisited(const int pos) const
{
  assert(0 <= pos && pos < firstFreeVisited);
  VisitedJunctionTreeElement elem;
  visited.Get(pos,elem);
  return elem;
}

void PQManagement::SetVisited(const int pos, const VisitedJunctionTreeElement e)
{
  assert(0<= pos && pos < firstFreeVisited);
  visited.Put(pos, e);
}


JPQEntryTreeElement PQManagement::GetPQ(const int pos) const
{
  assert(0 <= pos && pos < firstFreePQ);
  JPQEntryTreeElement elem;
  pq.Get(pos,elem);
  return elem;
}

void PQManagement::SetPQ(const int pos, const JPQEntryTreeElement e)
{
  assert(0<= pos && pos < firstFreePQ);
  pq.Put(pos, e);
}

/*
1.1.1 Insert into PQ

*/

int PQManagement::InsertPQ(const JPQEntry& entry)
{
  int pos = firstFreePQ;
  firstFreePQ++;
  JPQEntryTreeElement newEntry(entry, -1, -1);
  SetPQ(pos, newEntry);
  if (pos > 0)
  {
    bool fatherLeft = true;
    int fatherPos = GetFatherPos(pos, fatherLeft);
    JPQEntryTreeElement father = GetPQ(fatherPos);
    if (fatherLeft)
      father.SetLeftSon(pos);
    else
      father.SetRightSon(pos);
    SetPQ(fatherPos, father);
  }
  return pos;
}

int PQManagement::GetFatherPos(const int pos, bool& isLeftSon)
{
  int result = -1;
  if (pos > 0)
  {
    if ((pos % 2) == 0)
    {
      isLeftSon = false;
      result = pos - 2;
    }
    else
    {
      result = pos - 1;
      isLeftSon = true;
    }
    if (pos > 2)
      result  = result / 2;
  }
  return result;
}

void PQManagement::InsertVisited(const VisitedJunction& elem,
                                 const int fatherPos,
                                 const bool fatherLeft)
{
  int pos = firstFreeVisited;
  firstFreeVisited++;
  SetVisited(pos, VisitedJunctionTreeElement(elem));
  if (pos > 0)
  {
    VisitedJunctionTreeElement father = GetVisited(fatherPos);
    if (fatherLeft)
      father.SetLeftSon(pos);
    else
      father.SetRightSon(pos);
    SetVisited(fatherPos,father);
  }
}

/*
1.1 SwapPQ

*/

void PQManagement::SwapPQ(JPQEntryTreeElement& elem1, const int pos1,
                          JPQEntryTreeElement& elem2, const int pos2)
{
  JPQEntry entry1 = elem1.GetValue();
  int visitedPos1 = 0;
  bool visitedLeft1 = true;
  bool ok1 = FindVisited(entry1, visitedPos1, visitedLeft1);
  JPQEntry entry2 = elem2.GetValue();
  int visitedPos2 = 0;
  bool visitedLeft2 = true;
  bool ok2 = FindVisited(entry2, visitedPos2, visitedLeft2);
  elem1.SetValue(entry2);
  elem2.SetValue(entry1);
  SetPQ(pos1, elem1);
  SetPQ(pos2, elem2);
  assert(ok1);
  VisitedJunctionTreeElement actVisitedElem = GetVisited(visitedPos1);
  VisitedJunction actVisited = actVisitedElem.GetValue();
  actVisited.SetIndexPQ(pos2);
  actVisitedElem.SetValue(actVisited);
  SetVisited(visitedPos1, actVisitedElem);
  assert(ok2);
  actVisitedElem = GetVisited(visitedPos2);
  actVisited = actVisitedElem.GetValue();
  actVisited.SetIndexPQ(pos1);
  actVisitedElem.SetValue(actVisited);
  SetVisited(visitedPos2, actVisitedElem);
}

/*
1.1.1 CorrectPosition

*/

void PQManagement::CorrectPositionUp(JPQEntryTreeElement& newElem, int& pos)
{
  assert (0 <= pos && pos < firstFreePQ );
  if (pos > 0)
  {
    bool fatherLeft = true;
    int fatherPos = GetFatherPos(pos, fatherLeft);
    JPQEntryTreeElement father = GetPQ(fatherPos);
    if (father.GetValue().GetPriority() > newElem.GetValue().GetPriority())
    {
      SwapPQ(newElem, pos, father, fatherPos);
      CorrectPositionUp(father, fatherPos);
    }
  }
}

void PQManagement::CorrectPositionDown(JPQEntryTreeElement& newElem, int& pos)
{
  if (firstFreePQ > 0)
  {
    assert (0 <= pos && pos < firstFreePQ );
    int posLeftSon = newElem.GetLeftSon();
    int posRightSon = newElem.GetRightSon();
    if (posLeftSon != -1)
    {
      JPQEntryTreeElement leftSon = GetPQ(posLeftSon);
      if (posRightSon != -1)
      {
        JPQEntryTreeElement rightSon = GetPQ(posRightSon);
        if (newElem.GetValue().GetPriority() > leftSon.GetValue().GetPriority()
         ||newElem.GetValue().GetPriority() > rightSon.GetValue().GetPriority())
        {
          if (leftSon.GetValue().GetPriority() <
               rightSon.GetValue().GetPriority())
          {
            SwapPQ(newElem, pos, leftSon, posLeftSon);
            CorrectPositionDown(leftSon, posLeftSon);
          }
          else
          {
            SwapPQ(newElem, pos, rightSon, posRightSon);
            CorrectPositionDown(rightSon, posRightSon);
          }
        }
      }
      else
      {
        if (newElem.GetValue().GetPriority() > leftSon.GetValue().GetPriority())
        {
          SwapPQ(newElem, pos, leftSon, posLeftSon);
          CorrectPositionDown(leftSon, posLeftSon);
        }
      }
    }
    else
    {
      if (posRightSon != -1)
      {
        JPQEntryTreeElement rightSon = GetPQ(posRightSon);
        if(newElem.GetValue().GetPriority() > rightSon.GetValue().GetPriority())
        {
          SwapPQ(newElem, pos, rightSon, posRightSon);
          CorrectPositionDown(rightSon, posRightSon);
        }
      }
    }
  }
}

/*
1 Overload output operator

*/

ostream& operator<<(ostream& os, const PQManagement& e)
{
  e.Print(os);
  return os;
}
