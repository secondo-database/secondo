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

1 Includes

*/

#include "JRLTree.h"

/*
1 Implementation of class JRLTree

1.1 Constructors and Deconstructors

*/

JRLTree::JRLTree() : tree(0), firstFree(0)
{}

JRLTree::JRLTree(const int a) : tree(a), firstFree(0)
{}

JRLTree::JRLTree(const DbArray<RouteLocation>* inArray) : tree(0), firstFree(0)
{
  if (inArray->Size() > 0)
  {
    RouteLocation actRL(false);
    for (int i = 0; i < inArray->Size(); i++)
    {
      inArray->Get(i, actRL);
      if (actRL.IsDefined())
        Insert(actRL);
    }
  }
}

JRLTree::~JRLTree(){};


/*
1.1 Destroy

Remove harddisk part of DbArray

*/

void JRLTree::Destroy()
{
  tree.Destroy();
}

/*
1.1 Print

*/

ostream& JRLTree::Print(ostream& os) const
{
  os << "JRLTree: ";
  if (firstFree > 0)
  {
    JRLTreeElement elem;
    for (int i = 0; i < firstFree; i++)
    {
      tree.Get(i,elem);
      elem.Print(os);
    }
  }
  os << endl;
  return os;
}

/*
1.1 Insert

*/

void JRLTree::Insert(const RouteLocation rl, int pos /*=0*/)
{
  if (IsEmpty())
  {
    tree.Put(firstFree, JRLTreeElement(rl, -1,-1));
    firstFree++;
  }
  else
  {
    JRLTreeElement testRL;
    tree.Get(pos, testRL);
    switch(rl.Compare(testRL.GetValue()))
    {
      case -1:
      {
        if (testRL.GetLeftSon() > -1)
          Insert(rl, testRL.GetLeftSon());
        else
          Insert(true, pos, testRL, rl);
        break;
      }

      case 1:
      {
        if (testRL.GetRightSon() > -1)
          Insert(rl, testRL.GetRightSon());
        else
          Insert(false, pos, testRL, rl);
        break;
      }

      case 0:
      {
        //rloc already exists do nothing
        break;
      }

      default: //should never been reached
        assert(false);
    }
  }
}


void JRLTree::Insert (const bool left, const int pos,
                      JRLTreeElement& testRL,
                      const RouteLocation& test)
  {
    if (left)
      testRL.SetLeftSon(firstFree);
    else
      testRL.SetRightSon(firstFree);
    tree.Put(pos, testRL);
    tree.Put(firstFree, JRLTreeElement(test,-1,-1));
    firstFree++;
  }

/*
1.1 TreeToDbArray

*/

void JRLTree::TreeToDbArray(DbArray<RouteLocation>* outArray,
                            int fromPos /*= 0*/)
{
  assert(fromPos > -1 && fromPos < firstFree);
  JRLTreeElement test;
  tree.Get(fromPos,test);
  if (test.GetLeftSon() > -1)
    TreeToDbArray(outArray, test.GetLeftSon());
  outArray->Append(test.GetValue());
  if (test.GetRightSon() > -1)
    TreeToDbArray (outArray, test.GetRightSon());
}


/*
1.1. ~IsEmpty~

*/

bool JRLTree::IsEmpty() const
{
  return (firstFree == 0);
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JRLTree& in)
{
  in.Print(os);
  return os;
}

