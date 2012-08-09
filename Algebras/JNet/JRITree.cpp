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

1 Includes

*/

#include "JRITree.h"

/*
1 Implementation of class JRITree

1.1 Constructors and Deconstructors

*/

JRITree::JRITree() : tree(0), firstFree(0)
{}

JRITree::JRITree(const int a) : tree(a), firstFree(0)
{}

JRITree::JRITree(const DbArray<JRouteInterval>* inArray) : tree(0), firstFree(0)
{
  if (inArray->Size() > 0)
  {
    JRouteInterval actRI(false);
    for (int i = 0; i < inArray->Size(); i++)
    {
      inArray->Get(i, actRI);
      if (actRI.IsDefined())
        Insert(actRI);
    }
  }
}

JRITree::~JRITree(){};


/*
1.1 Destroy

Remove harddisk part of DbArray

*/

void JRITree::Destroy()
{
  tree.Destroy();
}

/*
1.1 Print

*/

ostream& JRITree::Print(ostream& os) const
{
  os << "JRITree: ";
  if (firstFree > 0)
  {
    JRITreeElement elem;
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

void JRITree::Insert(const JRouteInterval ri, int pos /*=0*/)
{
  if (IsEmpty())
  {
    tree.Put(firstFree, JRITreeElement(ri, -1,-1));
    firstFree++;
  }
  else
  {
    JRITreeElement testRI;
    tree.Get(pos, testRI);
    double test;
    if (ri.Overlaps(testRI.GetRouteInterval()))
    {
      if (ri.GetFirstPosition() < testRI.GetRouteInterval().GetFirstPosition())
      {
        testRI.SetRouteIntervalStart(ri.GetFirstPosition());
        tree.Put(pos, testRI);
        if (testRI.GetLeftSon() > -1)
        {
          test = CheckTree(testRI, pos, testRI, testRI.GetLeftSon(), true);
          testRI.SetRouteIntervalStart(test);
          tree.Put(pos, testRI);
        }
      }
      if (testRI.GetRouteInterval().GetLastPosition() < ri.GetLastPosition())
      {
        testRI.SetRouteIntervalEnd(ri.GetLastPosition());
        tree.Put(pos,testRI);
        if (testRI.GetRightSon() > -1)
        {
          test = CheckTree(testRI, pos, testRI, testRI.GetRightSon(), false);
          testRI.SetRouteIntervalEnd(test);
          tree.Put(pos, testRI);
        }
      }
    }
    else
    {
      switch(ri.Compare(testRI.GetRouteInterval()))
      {
        case -1:
        {
          if (testRI.GetLeftSon() > -1)
            Insert(ri, testRI.GetLeftSon());
          else
            Insert(true, pos, testRI, ri);
          break;
        }

        case 1:
        {
          if (testRI.GetRightSon() > -1)
            Insert(ri, testRI.GetRightSon());
          else
            Insert(false, pos, testRI, ri);
          break;
        }

        default: //should never been reached
          assert(false);
      }
    }
  }
}

void JRITree::Insert (const bool left, const int pos,
                      JRITreeElement& testRI,
                      const JRouteInterval& test)
  {
    if (left)
      testRI.SetLeftSon(firstFree);
    else
      testRI.SetRightSon(firstFree);
    tree.Put(pos, testRI);
    tree.Put(firstFree, JRITreeElement(test,-1,-1));
    firstFree++;
  }

/*
1.1 TreeToDbArray

*/

void JRITree::TreeToDbArray(DbArray<JRouteInterval>* outArray,
                            int fromPos /*= 0*/)
{
  assert(fromPos > -1 && fromPos < firstFree);
  JRITreeElement test;
  tree.Get(fromPos,test);
  if (test.GetLeftSon() > -1)
    TreeToDbArray(outArray, test.GetLeftSon());
  outArray->Append(test.GetRouteInterval());
  if (test.GetRightSon() > -1)
    TreeToDbArray (outArray, test.GetRightSon());
}


/*
1.1 ~CheckTree~

*/

double JRITree::CheckTree(JRITreeElement& father, const int posfather,
                          JRITreeElement& testRI, const int postest,
                          const bool bleft)
{
  assert(postest > -1 && postest < firstFree &&
         posfather > -1 && posfather < firstFree);
  JRITreeElement test;
  tree.Get(postest, test);
  if (test.GetRouteInterval().Overlaps(testRI.GetRouteInterval()))
  {
    if (bleft)
    {
      if (test.GetRouteInterval().GetFirstPosition() <
            testRI.GetRouteInterval().GetFirstPosition())
        testRI.SetRouteIntervalStart(
                                    test.GetRouteInterval().GetFirstPosition());
      if (father.GetLeftSon() == postest)
      {
        father.SetLeftSon(test.GetLeftSon());
        tree.Put(posfather, father);
        test.SetLeftSon(-1);
        tree.Put(postest, test);
      }
      else
      {
        father.SetRightSon(test.GetLeftSon());
        tree.Put(posfather,father);
        test.SetLeftSon(-1);
        tree.Put(postest,test);
      }

      if (father.GetLeftSon() > -1)
        return CheckTree(father, posfather, testRI, father.GetLeftSon(),
                         bleft);
      else
        return testRI.GetRouteInterval().GetFirstPosition();
    }
    else
    {
      if (test.GetRouteInterval().GetLastPosition() >
            testRI.GetRouteInterval().GetLastPosition())
        testRI.SetRouteIntervalEnd(test.GetRouteInterval().GetLastPosition());
      if (father.GetLeftSon() == postest)
      {
        father.SetLeftSon(test.GetRightSon());
        tree.Put(posfather,father);
        test.SetRightSon(-1);
        tree.Put(postest,test);
      }
      else
      {
        father.SetRightSon(test.GetRightSon());
        tree.Put(posfather,father);
        test.SetRightSon(-1);
        tree.Put(postest,test);
      }
      if (father.GetRightSon() > -1 )
        return CheckTree(father, posfather, testRI, father.GetRightSon(),
                         bleft);
      else
        return testRI.GetRouteInterval().GetLastPosition();
    }
  }
  else
  {
    switch(testRI.GetRouteInterval().Compare(test.GetRouteInterval()))
    {
      case -1:
      {
        if (test.GetLeftSon() > -1)
          return CheckTree(test, postest, testRI, test.GetLeftSon(), bleft);
        else
          return testRI.GetValue(bleft);
        break;
      }

      case 1:
      {
        if (test.GetRightSon() > -1)
          return CheckTree(test, postest, testRI, test.GetRightSon(), bleft);
        else
          return testRI.GetValue(bleft);
        break;
      }

      default: //should never been reached
        {
          assert(false);
          return testRI.GetValue(bleft);
          break;
        }
    }
  }
}


/*
1.1. ~IsEmpty~

*/

bool JRITree::IsEmpty() const
{
  return (firstFree == 0);
}

/*
1 Overwrite output operator

*/

ostream& operator<<(ostream& os, const JRITree& in)
{
  in.Print(os);
  return os;
}

