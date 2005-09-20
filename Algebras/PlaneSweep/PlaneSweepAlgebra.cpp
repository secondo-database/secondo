/*
-----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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
----

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]

[1] Implementation of the realmisation for typs of the Spatial Algebra

[TOC]

1 Overview

This implementation uses the classes ~Line~, and ~Region~ used in the
Spatial Algebra. They are used in the realmisation and plane-sweep-
Algorithmen for operations explained in the ROSE-Algebra


2 Defines and Includes

*/
         
using namespace std;

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "SpatialAlgebra.h"
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <list>
#include <set>

#ifdef SECONDO_WIN32
const double M_PI = acos( -1.0 );
#endif

extern NestedList* nl;
extern QueryProcessor* qp;


/*
3 Classes for construktion of a REALM

Several classes are needed to contruct al REALM from a ~line~- or a ~region~-Object.
A REALM  is a set of points and halfsegments which not intersects.
The REALMed objects are build with a plane-sweep-algorithm.

*/

/*
3 Classes for segement-intersection a la [Bentley/Ottmann 1979]
    like Prof. G[ue]ting in November 1999 in Datenstrukturen

*/

//template<class E> class BinTree;
//template<class E> class BinTreeNode;


class Segment
{
public:
   Segment() {};
   ~Segment() {};
   Segment::Segment(bool reg, const CHalfSegment& inchs);
   const Point& GetLP();
   const Point& GetRP();
   void SetLP(const Point& p);
   void SetRP(const Point& p);
   bool GetIn1() const;
   const CHalfSegment& GetCHS();
   void CHSInsert(list<CHalfSegment>& r1, list<CHalfSegment>& r2);


private:
   bool in1;
   CHalfSegment chs;
};

Segment::Segment(bool in, const CHalfSegment& inchs)
{
   in1 = in;
   chs = CHalfSegment(inchs.IsDefined(), true, inchs.GetLP(), inchs.GetRP() );
   chs.attr.insideAbove = inchs.attr.insideAbove;
}

const Point& Segment::GetLP()	{  return chs.GetLP(); }

const Point& Segment::GetRP()	{  return chs.GetRP();}

void Segment::SetLP(const Point& p)   {
   if  ( p != chs.GetRP() && p != chs.GetLP() )   chs.Set(true,p,chs.GetRP());
}

void Segment::SetRP(const Point& p)   {
   if  ( p != chs.GetLP() && p != chs.GetRP() )  chs.Set(true,chs.GetLP(),p);
}

bool Segment::GetIn1() const	{ return in1; }
const CHalfSegment& Segment::GetCHS() { return chs;}

void Segment::CHSInsert(list<CHalfSegment>& r1, list<CHalfSegment>& r2)
{
   if (GetIn1())   r1.push_front(GetCHS());
   else  r2.push_front(GetCHS() );
   //cout << " ausgegebenes Segment " << GetCHS() << endl;
}

ostream& operator<<(ostream &os, Segment& segm)
{
   return (os << "Segment -CHS: " << segm.GetCHS() );
}

enum EventKind {bottomSeg, rightSeg, split, intersection, leftSeg};

/*
7  Class for making entries in the priority queue

*/

class XEvent
{
public:
   XEvent() {};
   ~XEvent() {};
   XEvent(Coord& x, Coord& y, int inseg, double k, double a, EventKind event);
   XEvent(Coord& x, Coord& y, int inseg1, int inseg2, EventKind event);
   XEvent(Coord& x, Coord& y, int inseg, EventKind event);
   XEvent(XEvent* event);
   void Set(const XEvent& event);
   Coord GetX() const;
   Coord GetY() const;
   EventKind GetKind() const;
   int GetFirst() const;
   int GetSecond() const;
   double GetSlope() const;
   double GetA() const;
   int Less (const XEvent ev2, const Coord x) const;
   bool EventEqual (const XEvent ev2) const;

private:
   Coord x;
   Coord y;
   double slope;
   double a;
   EventKind kind;
   int seg1;
   int seg2;

   XEvent& operator= (const XEvent& ev);

};

XEvent::XEvent(XEvent* event)
{
   x = event-> x;
   y = event-> y;
   slope = event->slope;
   a = event -> a;
   kind= event->kind;
   seg1 = event->seg1;
   seg2 = event->seg2;
}

XEvent::XEvent(Coord& inx, Coord& iny, int inseg, EventKind event)
{
   if (event==split)  {
      kind=event;
      x = inx;
      y = iny;
      seg1 = inseg;
      seg2 = -1;
      slope = 0;
      a = 0;
   }
   else if (event == intersection)  {
      cout << "incorrect number of values for split " << endl;
   }
}

XEvent:: XEvent(Coord& inx, Coord& iny, int inseg, double k1, double a1, EventKind event)
{
   if (event==rightSeg || event==bottomSeg || event==leftSeg)  {
      kind=event;
      x = inx;
      y = iny;
      slope = k1;
      a = a1;
      seg1 = inseg;
      seg2 = -1;
   }
   else if (event == intersection)  {
      cout << "incorrect number of values for intersection " << endl;
   }
   else  {
      cout << "incorrect eventkind for konstruktor 1" << event << endl;
   }
 }

XEvent:: XEvent(Coord& inx, Coord& iny, int inseg1, int inseg2,EventKind event)
{
   if (event == intersection) {
      kind = event;
      x = inx;
      y = iny;
      seg1 = inseg1;
      seg2 = inseg2;
      slope = 0;
      a= 0;
   }
   else  {
      cout << "incorrect eventkind for constructor instersection" << event << endl;
   }
}

void XEvent::Set(const XEvent& event) {
   x = event.x;
   y = event.y;
   slope = event.slope;
   a = event.a;
   kind= event.kind;
   seg1 = event.seg1;
   seg2 = event.seg2;
}

XEvent& XEvent::operator= (const XEvent& ev)  { return *this;}
Coord XEvent::GetX() const		{return x;}
Coord XEvent::GetY() const		{return y;}
EventKind XEvent::GetKind() const	{return kind;}
int XEvent::GetFirst() const     	{return seg1;}
int XEvent::GetSecond() const 		{return seg2;}
double XEvent::GetSlope() const		{return slope;}
double XEvent::GetA() const		{return a;}

bool XEvent::EventEqual (const XEvent ev2) const
{
   if (GetKind() == ev2.GetKind() && GetFirst()== ev2.GetFirst() &&
       GetSecond() == ev2.GetSecond() )    return true;
   else return false;
}

int XEvent::Less (const XEvent ev2, const Coord x) const
{
   if ( EventEqual (ev2) )  return 0;
   else if (GetX() < ev2.GetX())  { return -1; }
   else if (GetX() > ev2.GetX()) { return 1; }
   // if same x-coordinates, then compare event-kind
   else if (GetKind() < ev2.GetKind())  	{ return -1;}
   else if (GetKind() > ev2.GetKind())  	{ return 1;}
   else  {// the same x-Coordinates and the same event-kind -> different cases
      if (GetKind() == leftSeg || GetKind() == bottomSeg) {
         if (GetFirst() < ev2.GetFirst())	{ return -1;}
	 else					{ return 1;}
      }
      else if (GetKind() == rightSeg) {
         if (GetY() < ev2.GetY())		{ return -1;}
	 else if (GetY() > ev2.GetY())		{return 1;}
	 else if (GetSlope() > ev2.GetSlope())	{return 1;}
	 else 					{return -1;}
      }
      else if (GetKind() == intersection)  {
         if (GetY() < ev2.GetY())		{return -1;}
	 else					{return 1;}
      }
      else if (GetKind() == split)  {
         if (GetY() < ev2.GetY())		{return -1;}
	 else					{return 1;}
      }
      // should not reached
      else {
         cout << " incorrect eventkind in < " << GetKind() << endl;
         return -1;
     }
   }
}

/*
loeschne

*/

ostream& operator<<(ostream &os, const XEvent& ev)
{
   //return (os << "XEvent: " );
   return (os << "XEvent: x:" << ev.GetX() << "  y: " << ev.GetY() << "  Kind: " << ev.GetKind()<<
   "   First: " << ev.GetFirst() << "  Second:  " << ev.GetSecond() << "  slope: " << ev.GetSlope()
   << "  a: " << ev.GetA()  << " )"  );
}


template<class E> class BinTree ;
template<class E> class BinTreeNode;

enum Status {NEW, BAL, UNBAL, DUP};

template<class E> class BinTreeNode
{
public:

   BinTreeNode<E>() {};
   E GetEntry()				{ return entry;}
   void SetEntry(const E& newentry)	{ entry.Set(newentry); }
   BinTreeNode<E>* GetNext()		{ return Next; }
   BinTreeNode<E>* GetPred()		{ return Pred; }

private:
   friend class BinTree<E>;
   BinTreeNode<E>* Next;
   BinTreeNode<E>* Pred;
   BinTreeNode<E>* Left;
   BinTreeNode<E>* Right;
   BinTreeNode<E>* Parent;
   Status State;
   E entry;
};


template<class E> class BinTree
{
public:
   BinTree<E>();
   ~BinTree<E>();
   BinTreeNode<E>* Insert (const E &entry, const Coord x);
   BinTreeNode<E>* Insert (const E &entry, const Coord x, Segment sgs[]);
   BinTreeNode<E>* Insert (const E &entry, const Coord x, const E &old, BinTreeNode<E>* oldnode);
   void Delete (const E &entry, const Coord x);
   void DeleteNode (BinTreeNode<E>* node) ;
   E BinTree<E>::GetFirstAndDelete () ;
   void Clear();
   BinTreeNode<E>* GetFirst();
   BinTreeNode<E>* GetLast();
   int GetCount();
   bool IsEmpty();
   BinTreeNode<E>* Find (const E &entry, const Coord x);
   BinTreeNode<E>* Find (const E &entry, const Coord x,  Segment sgs[]);
   BinTreeNode<E>* Find (const E &entry, const Coord x,  const E &old, BinTreeNode<E>* oldnode);
   void BinTree<E>::Output() ;

protected:
   int cmp(const E &first, const E &second, const Coord x);
   int cmp(const E &first, const E &second, const Coord x, Segment sgs[]);
   int cmp(const E &first, const E &second, const Coord x, const E &old, BinTreeNode<E>* oldnode);


private:
   BinTreeNode<E>* First;
   BinTreeNode<E>* Last;
   BinTreeNode<E>* Root;
   BinTreeNode<E>* NIL;
   int  mCount;
   void RotateRight(BinTreeNode<E>* node);
   void RotateLeft(BinTreeNode<E>* node);
   void BalanceInsert(BinTreeNode<E>* node);
   void BalanceDelete(BinTreeNode<E>* node);
   void MakeDelete(BinTreeNode<E>* node);
   void MakeInsert(BinTreeNode<E>* node, const Coord x);
   void MakeInsert(BinTreeNode<E>* node, const Coord x, Segment sgs[]);
   void MakeInsert(BinTreeNode<E>* node, const Coord x, const E &old, BinTreeNode<E>* oldnode);
   void DeleteInList(BinTreeNode<E>* node);
   void InsertBeforeInList(BinTreeNode<E>* node1, BinTreeNode<E>* node2);
   void InsertAfterInList(BinTreeNode<E>* node1, BinTreeNode<E>* node2);
   BinTreeNode<E>* TreeNext(BinTreeNode<E>* node);
   BinTreeNode<E>* TreePred(BinTreeNode<E>* node);
   BinTreeNode<E>* GetMin(BinTreeNode<E>* node);
   BinTreeNode<E>* GetMax(BinTreeNode<E>* node);
};



template<class E> BinTree<E>::BinTree()  {
   NIL = new BinTreeNode<E>;
   NIL->Next = 0;
   NIL->Pred = 0;
   NIL->Parent = 0;
   NIL ->Left = NIL;
   NIL->Right = NIL;
   NIL->State = BAL;
   Root = NIL;
   First = 0;
   Last = 0;
   mCount = 0;
}

template<class E> BinTree<E>::~BinTree()  {
   Clear();
   delete NIL;
}


template<class E> BinTreeNode<E>* BinTree<E>::Insert (const E &entry, const Coord x)  {
   BinTreeNode<E>* node = new BinTreeNode<E>();
   // initialize node
   node->Next = 0;
   node->Pred = 0;
   node->Parent = 0;
   node ->Left = NIL;
   node->Right = NIL;
   node->State = UNBAL;
   node->SetEntry(entry);
   // Insert Node in Tree
   MakeInsert(node, x);
   mCount++;
   return node;
}

template<class E> BinTreeNode<E>* BinTree<E>::Insert (const E &entry, const Coord x, Segment sgs[])
{
   BinTreeNode<E>* node = new BinTreeNode<E>();
   // initialize node
   node->Next = 0;
   node->Pred = 0;
   node->Parent = 0;
   node ->Left = NIL;
   node->Right = NIL;
   node->State = UNBAL;
   node->SetEntry(entry);
   // Insert Node in Tree
   MakeInsert(node, x, sgs);
   mCount++;
   return node;
}

template<class E> BinTreeNode<E>* BinTree<E>::Insert(const E &entry, const Coord x, const E &old, BinTreeNode<E>* oldnode) {
   BinTreeNode<E>* node = new BinTreeNode<E>();
   // initialize node
   node->Next = 0;
   node->Pred = 0;
   node->Parent = 0;
   node ->Left = NIL;
   node->Right = NIL;
   node->State = UNBAL;
   node->SetEntry(entry);
   // Insert Node in Tree
   MakeInsert(node, x, old, oldnode);
   mCount++;
   return node;
}

template<class E> void BinTree<E>::Delete (const E &entry, const Coord x) {
   BinTreeNode<E>* node = Find(entry, x);
   assert (node);
   MakeDelete(node);
   mCount --;
   delete node;
}

template<class E> void BinTree<E>::DeleteNode (BinTreeNode<E>* node) {
   assert (node);
   MakeDelete(node);
   mCount --;
   delete node;
}

template<class E> E BinTree<E>::GetFirstAndDelete () {
   BinTreeNode<E>* node = First;
   E en = node->GetEntry();
   assert (node);
   MakeDelete(node);
   mCount --;
   if (! IsEmpty() ) {
      BinTreeNode<E> * next = node->GetNext();
      // remove all dups
      while (next != NIL) {
         if ( en.EventEqual(next->GetEntry()) ) {
            BinTreeNode<E>* del = next;
	    next = next->GetNext() ;
	    assert (del);
	    MakeDelete(del);
	    mCount --;
	    delete del;
         }
         else next = NIL;
      }
   }
   delete node;
   return en;
}

template<class E>  BinTreeNode<E>* BinTree<E>::Find (const E &entry, const Coord x)
{
   BinTreeNode<E>* node = Root;
   int c;
   while ( node  != NIL) {
      c = cmp (entry, node->GetEntry(), x);
      if ( c==0)  return node;
      else if (c<0)  node = node->Left;
      else node = node->Right;
   }
   return 0;  // entry not found
}

template<class E>  BinTreeNode<E>* BinTree<E>::Find (const E &entry, const Coord x, Segment sgs[] )
{
   BinTreeNode<E>* node = Root;
   int c;
   while ( node  != NIL) {
      c = cmp (entry, node->GetEntry(), x, sgs);
      if ( c==0)  return node;
      else if (c<0)  node = node->Left;
      else node = node->Right;
   }
   return 0;  // entry not found
}

template<class E>  BinTreeNode<E>* BinTree<E>::Find (const E &entry, const Coord x, const E &old, BinTreeNode<E>* oldnode)
{
   BinTreeNode<E>* node = Root;
   int c;
   while ( node  != NIL) {
      c = cmp (entry, node->GetEntry(), x, old, oldnode);
      if ( c==0)  return node;
      else if (c<0)  node = node->Left;
      else node = node->Right;
   }
   return 0;  // entry not found
}

template<class E>  void BinTree<E>::Clear() {
   BinTreeNode<E>* node = First;
   BinTreeNode<E>* node2;
   while ( node)  {
      node2 = node;
      node = node->Next;
      delete node2;
   }
   First = 0;
   Last = 0;
   Root = NIL;
   mCount = 0;
}

template<class E> bool BinTree<E>::IsEmpty() { return (mCount==0); }
template<class E> BinTreeNode<E>* BinTree<E>::GetFirst()  { return First; }
template<class E> BinTreeNode<E>* BinTree<E>::GetLast()   { return Last; }
template<class E> int BinTree<E>::GetCount () { return mCount; }

template<class E> int BinTree<E>::cmp(const E &first, const E &second, const Coord x) {
   if (first.Less (second, x) <0 ) return -1;
   else if ( first.Less(second, x) > 0  )  return 1;
   else return 0;
}

template<class E> int BinTree<E>::cmp(const E &first, const E &second, const Coord x, Segment sgs[]) {
   if (first.Less (second, x, sgs) <0 ) return -1;
   else if ( first.Less(second, x, sgs) > 0  )  return 1;
   else return 0;
}

template<class E> int BinTree<E>::cmp(const E &first, const E &second, const Coord x, const E &old, BinTreeNode<E>* oldnode) {
   if (first.Less (second, x, old, oldnode) <0 ) return -1;
   else if ( first.Less(second, x, old, oldnode) > 0  )  return 1;
   else return 0;
}


template<class E> void BinTree<E>::RotateRight(BinTreeNode<E>* node) {
   BinTreeNode<E>* n2 = node->Left;
   node->Left = n2->Right;
   if (n2->Right != NIL) n2->Right->Parent = node;
   if (n2 != NIL) n2->Parent = node->Parent;
   if (node->Parent){
      if ( node == node->Parent->Right) node->Parent->Right = n2;
      else   node->Parent->Left = n2;
   }
   else  Root = n2;
   n2->Right = node;
   if (node != NIL) node->Parent = n2;
}


template<class E> void BinTree<E>::RotateLeft(BinTreeNode<E>* node)  {
   BinTreeNode<E>* n2 = node->Right;
   node->Right = n2->Left;
   if (n2->Left != NIL) n2->Left->Parent = node;
   if (n2 != NIL) n2->Parent = node->Parent;
   if (node->Parent){
      if ( node == node->Parent->Left) node->Parent->Left = n2;
      else   node->Parent->Right = n2;
   }
   else  Root = n2;
   n2->Left = node;
   if (node != NIL) node->Parent = n2;
}

template<class E> void BinTree<E>::BalanceInsert(BinTreeNode<E>* node) {
   while (node != Root && node->Parent->State == UNBAL)  {
      if ( node->Parent == node->Parent->Parent->Left) {
         BinTreeNode<E>* n2 = node->Parent->Parent->Right;
	 if (n2->State == UNBAL) {
	    node->Parent->State = BAL;
	    n2->State = BAL;
	    node->Parent->Parent->State = UNBAL ;
	    node = node->Parent->Parent;
         } // end second if
	 else  {
	     if (node == node->Parent->Right) {
	        node = node->Parent;
		RotateLeft (node);
	     } // end if
	     node->Parent->State = BAL;
	     node->Parent->Parent->State = UNBAL;
	     RotateRight (node->Parent->Parent);
	 } // end else
      } // end first if
      else {
          BinTreeNode<E>* n2 = node->Parent->Parent->Left;
	  if ( n2->State == UNBAL) {
	     node->Parent->State = BAL;
	     n2->State = BAL;
	     node->Parent->Parent->State = UNBAL;
	     node = node->Parent->Parent;
	  } // end if
	  else {
	     if ( node == node->Parent->Left) {
	        node = node->Parent;
		RotateRight(node);
	     } // end if
	     node->Parent->State = BAL;
	     node->Parent->Parent->State = UNBAL;
	     RotateLeft (node->Parent->Parent);
	  } // end else
      } // end else
   } // end while
   Root->State = BAL;
}

template<class E> void BinTree<E>::BalanceDelete(BinTreeNode<E>* node) {
   while (node != Root && node->State == BAL ) {
      if ( node == node->Parent->Left) {
         BinTreeNode<E>* n2 = node->Parent->Right;
	 if (n2->State == UNBAL) {
	    node->Parent->State = UNBAL;
	    n2->State = BAL;
	    RotateLeft(node->Parent);
	    n2 = node->Parent->Right;
         } // end second if
	 if (n2->Left->State == BAL && n2->Right->State == BAL)  {
	    n2->State = UNBAL;
	    node = node->Parent;
	 }
         else {
	    if (n2->Right->State == BAL) {
	       n2->Left->State = BAL;
	       n2->State = UNBAL;
	       RotateRight (n2);
	       n2 = node->Parent->Right;
	     } // end if
	     n2->State = node->Parent->State;
	     node->Parent->State = BAL;
	     n2->Right->State = BAL;
	     RotateLeft (node->Parent);
	     node = Root;
	 } // end else
      } // end first if
      else {
          BinTreeNode<E>* n2 = node->Parent->Left;
	  if ( n2->State == UNBAL) {
	     node->Parent->State = UNBAL;
	     n2->State = BAL;
	     RotateRight(node->Parent);
	     n2 = node->Parent->Left;
	  } // end if
          if ( n2->Right->State == BAL && n2->Left->State == BAL) {
	     n2->State = UNBAL;
	     node = node->Parent;
	  } // end
	  else {
	     if ( n2->Left->State == BAL) {
	        n2->Right->State = BAL;
		n2->State = UNBAL;
		RotateLeft (n2);
	        n2 = node->Parent->Left;
	     } // end if
	     n2->State = node->Parent->State;
	     node->Parent->State = BAL;
	     n2->Left->State = BAL;
	     RotateRight (node->Parent);
	     node = Root;
	  } // end else
      } // end else
   } // end while
   node->State = BAL;
}


template<class E> void BinTree<E>::MakeInsert (BinTreeNode<E>* node, const Coord x)  {
   BinTreeNode<E>* node1;
   BinTreeNode<E>* node2;
   int c;
   node1 = Root;
   node2 = 0;
   while (node1 != NIL )  {
      c = cmp(node->entry, node1->entry, x);
      if ( c==0) { // do nothing
         mCount--; return; }
      node2 = node1;
      node1 = (c<0)? node1->Left: node1->Right ;
   }
   if (node2 ) {
      node -> Parent = node2;
      if (c<0) {  // insert left
         InsertBeforeInList (node2, node);
	 node2-> Left = node;
      }
      else  { // c>0
         node1 = TreeNext (node2);
	 if (node1)  InsertBeforeInList ( node1, node);
	 else      InsertAfterInList(Last, node);
	 node2->Right = node;
      }
   }
   else { // insert first entry
      Root = node;
      node->State = BAL;
      First = node;
      Last = node;
   }
   BalanceInsert(node);
}

template<class E> void BinTree<E>::MakeInsert (BinTreeNode<E>* node, const Coord x, Segment sgs[])  {
   BinTreeNode<E>* node1;
   BinTreeNode<E>* node2;
   int c;
   node1 = Root;
   node2 = 0;
   while (node1 != NIL )  {
      c = cmp(node->entry, node1->entry, x, sgs);
      if ( c==0) { // do nothing
         mCount--; return;  }
      node2 = node1;
      node1 = (c<0)? node1->Left: node1->Right ;
   }
   if (node2 ) {
      node -> Parent = node2;
      if (c<0) {  // insert left
         InsertBeforeInList (node2, node);
	 node2-> Left = node;
      }
      else  { // c>0
         node1 = TreeNext (node2);
	 if (node1)  InsertBeforeInList ( node1, node);
	 else      InsertAfterInList(Last, node);
	 node2->Right = node;
      }
   }
   else { // insert first entry
      Root = node;
      node->State = BAL;
      First = node;
      Last = node;
   }
   BalanceInsert(node);
}

template<class E> void BinTree<E>::MakeInsert (BinTreeNode<E>* node, const Coord x, const E &old, BinTreeNode<E>* oldnode)  {
   BinTreeNode<E>* node1;
   BinTreeNode<E>* node2;
   int c;
   node1 = Root;
   node2 = 0;
   while (node1 != NIL )  {
      c = cmp(node->entry, node1->entry, x, old, oldnode);
      if ( c==0) { // do nothing
         mCount--; return;  }
      node2 = node1;
      node1 = (c<0)? node1->Left: node1->Right ;
   }
   if (node2 ) {
      node -> Parent = node2;
      if (c<0) {  // insert left
         InsertBeforeInList (node2, node);
	 node2-> Left = node;
      }
      else  { // c>0
         node1 = TreeNext (node2);
	 if (node1)  InsertBeforeInList ( node1, node);
	 else      InsertAfterInList(Last, node);
	 node2->Right = node;
      }
   }
   else { // insert first entry
      Root = node;
      node->State = BAL;
      First = node;
      Last = node;
   }
   BalanceInsert(node);
}

template<class E> void BinTree<E>::MakeDelete(BinTreeNode<E>* node) {
   if (node->State == DUP) { // node only in list, but not in tree
      DeleteInList(node);
      return;
   }
   if (node->Next && node->Next->State == DUP ) {
      // replace node with data from node->Next
      node->Next->State = node->State;
      node->Next->Left = node->Left;
      node->Next->Right = node->Right;
      node->Next->Parent = node->Parent;
      if (node->Next->Parent) {
         if (node->Next->Parent->Left == node)  node->Next->Parent->Left = node->Next;
	 else    node->Next->Parent->Right = node->Next;
      }
      else  Root = node->Next;
      node->Next->Left->Parent = node->Next;
      node->Next->Right->Parent = node->Next;
      DeleteInList(node);
      return;
   } // end if
   DeleteInList (node);
   BinTreeNode<E>* n1;
   BinTreeNode<E>* n2;
   bool balance;
   if (node->Left == NIL || node->Right == NIL)  n1 = node;
   else  n1 = GetMin(node->Right);
   if (n1->Left != NIL)   n2 = n1->Left;
   else   n2 = n1->Right;
   n2->Parent = n1->Parent;
   if (n1->Parent) {
      if (n1 == n1->Parent->Left)  n1->Parent->Left = n2;
      else    n1->Parent->Right = n2;
   }
   else Root = n2;
   if (n1->State == BAL)   balance = true;
   if ( n1 != node) {
      n1->State = node->State;
      n1->Left = node->Left;
      n1->Right = node->Right;
      n1->Parent = node->Parent;
      if (n1->Parent) {
         if (n1->Parent->Left ==node)  n1->Parent->Left = n1;
	 else    n1->Parent->Right = n1;
      }
      else  Root = n1;
      n1->Left->Parent = n1;
      n1->Right->Parent = n1;
   }
   if (balance == true) BalanceDelete(n2);
}

template<class E> void BinTree<E>::DeleteInList(BinTreeNode<E>* node){
  if (node->Pred)  node->Pred->Next = node->Next;
   else First = node->Next;
   if ( node->Next) node->Next->Pred = node->Pred;
   else Last = node->Pred;
}

template<class E> void BinTree<E>::InsertBeforeInList(BinTreeNode<E>* n1, BinTreeNode<E>* n2) {
   n2->Pred = n1->Pred;
   if (n2->Pred)  n2->Pred->Next = n2;
   else First = n2;
   n2->Next = n1;
   n1->Pred = n2;
}

template<class E> void BinTree<E>::InsertAfterInList(BinTreeNode<E>* n1, BinTreeNode<E>* n2) {
   n2->Next = n1->Next;
   if (n2->Next)  n2->Next->Pred = n2;
   else Last = n2;
   n2->Pred = n1;
   n1->Next = n2;
}

template<class E> BinTreeNode<E>* BinTree<E>::TreeNext(BinTreeNode<E>* node) {
   if (node->Right != NIL ) return GetMin(node->Right);
   while ( node->Parent) {

      if (node == node->Parent->Left) break;
      node = node->Parent;
   }
   return node->Parent;
}

template<class E> BinTreeNode<E>* BinTree<E>::TreePred(BinTreeNode<E>* node) {
   if (node->Left != NIL ) return GetMax(node->Left);
   while ( node->Parent) {
      if (node == node->Parent->Right) break;
      node = node->Parent;
   }
   return node->Parent;
}

template<class E> BinTreeNode<E>* BinTree<E>::GetMin(BinTreeNode<E>* node)  {
   while (node->Left != NIL)  node = node->Left;
   return node;
}

template<class E> BinTreeNode<E>* BinTree<E>::GetMax(BinTreeNode<E>* node) {
   while ( node->Right != NIL) node = node->Right;
   return node;
}

/*
class PQueue  - SSX

*/

class PQueue
{
public:
   PQueue();
   ~PQueue() { qu.Clear(); }
   void insert (XEvent& event);
   XEvent getFirstAndDelete();
   bool isEmpty();

   void PQueueOutput();

private:
   BinTree<XEvent> qu;
   int size();
   XEvent first();
   int cmp (const XEvent* ev1, const XEvent* ev2) const;
   bool eventEqual  (const XEvent* ev1, const XEvent* ev2) const;

};

PQueue::PQueue()  {BinTree<XEvent> qu; }
bool PQueue::isEmpty()		{ return (qu.IsEmpty() ); }
int PQueue::size()		{ return (qu.GetCount()) ; }

void PQueue::insert (XEvent& event)   {
   Coord y;
   qu.Insert(event, y);
   //PQueueOutput();
}

XEvent PQueue::getFirstAndDelete()  {
   //cout << " PQUeueu: GetFirstAndDelete()" ;
   XEvent event =  qu.GetFirstAndDelete();
   //cout << "   event " << event << endl;
   //PQueueOutput();
   return (event);
}


void PQueue::PQueueOutput()
{
   cout << " in output " << endl;
   BinTreeNode<XEvent>* node = qu.GetFirst();
   // BinTreeNode<XEvent>* ev = First();
   if (size() > 0 ) {
      while ( node != 0 )  {
         XEvent ev = node->GetEntry();
         cout << "XEvent: "<< ev.GetX()<< " "<< ev.GetY()<< " "<< ev.GetKind()<< " " <<                         ev.GetFirst()<<" "<<ev.GetSecond()<<" "<<ev.GetSlope()<< " "<< ev.GetA()<< endl;
         node = node->GetNext();
      }
   }
   cout << " in output fertig " << endl;
}


/*
class SSLEntry

*/

class SSLEntry
{
public:
   SSLEntry() { chs = -1;};
   ~SSLEntry() {};
   SSLEntry(int inseg, Segment segs[] );
   SSLEntry(int inseg, double ink, double ina);
   void Set(const SSLEntry& in);

   bool Equal (const SSLEntry se) const;
   int GetSeg() const;
   const double GetY (Coord x, Segment segs[]) const;
   double Getk() const;
   double Geta() const;
   int SSLEntry:: Less (const SSLEntry in2, const Coord x, Segment segs[]) const;

private:
   int chs;
   double k;
   double a;

   SSLEntry& operator= (const SSLEntry& se);
};

SSLEntry:: SSLEntry(int inseg, double ink, double ina)
{
   chs = inseg;
   k = ink;
   a = ina;
}

SSLEntry:: SSLEntry (int inseg, Segment segs[])
{
   chs = inseg;
   Coord xl, xr, yl, yr;
   xl = segs[inseg].GetCHS().GetLP().GetX();
   xr = segs[inseg].GetCHS().GetRP().GetX();
   yl = segs[inseg].GetCHS().GetLP().GetY();
   yr = segs[inseg].GetCHS().GetRP().GetY();
  #ifdef RATIONAL_COORDINATES
      k= ((yr.IsInteger()? yr.IntValue():yr.Value()) -
                 (yl.IsInteger()? yl.IntValue():yl.Value())) /
                ((xr.IsInteger()? xr.IntValue():xr.Value()) -
                 (xl.IsInteger()? xl.IntValue():xl.Value())) ;
      a = (yl.IsInteger()? yl.IntValue():yl.Value()) -
                  k* (xl.IsInteger()? xl.IntValue():xl.Value());
  #else
      k = (yr - yl) / (xr - xl) ;
      a = yl - k*xl;
  #endif

}

void SSLEntry:: Set(const SSLEntry& in)
{
   chs = in.chs;
   k = in.k;
   a = in.a;
}

SSLEntry& SSLEntry::operator= (const SSLEntry& se) 	{return *this;}
double SSLEntry::Getk() const				{return k; }
double SSLEntry::Geta() const				{return a; }
int SSLEntry::GetSeg() const				{return chs; }

bool SSLEntry::Equal(const SSLEntry se) const
{
   if (GetSeg() == se.GetSeg()) return true;
   else return false;
}

const double SSLEntry::GetY(Coord x, Segment segs[]) const  {
   Coord res;
   bool end = false;
   if (segs[GetSeg()].GetCHS().GetLP().GetX() == x) {
      end = true;
      res = segs[GetSeg()].GetCHS().GetLP().GetY();
   }
   else if (segs[GetSeg()].GetCHS().GetRP().GetX() == x) {
      end = true;
      res = segs[GetSeg()].GetCHS().GetRP().GetY();
   }
   if ( end) {
      #ifdef RATIONAL_COORDINATES
      double y = (res.IsInteger()? res.IntValue(): res.Value()) ;
      #else
      double y = res ;
      #endif
      return y;
   }
   else {
      #ifdef RATIONAL_COORDINATES
      double xv = (x.IsInteger()? x.IntValue(): x.Value()) ;
      #else
      double xv = x ;
      #endif
      return ( k*xv + a);
   }
}

int SSLEntry:: Less (const SSLEntry in2, const Coord x, Segment segs[]) const
{
   //cout << " in less" << endl;
   if (Equal(in2)) return 0;
   else if (GetY(x, segs) < in2.GetY(x, segs)) return -1;
   if (GetY(x, segs) > in2.GetY(x, segs)) return 1;
   //cout << " y-Wertw gleich  " << (GetY(x,segs))<< "    " << (in2.GetY(x,segs)) << endl;
   //if ( segs[GetSeg()].GetCHS().GetLP() == segs[in2.GetSeg()].GetCHS().GetLP() ) {
      if (Getk() < in2.Getk()) return -1;
      else if (Getk() > in2.Getk() ) return 1;
      else if (GetSeg() < in2.GetSeg() ) return -1;
      else if (GetSeg() > in2.GetSeg() ) return 1;
      else return 0;
   //}
   //if (GetSeg() < in2.GetSeg() ) return -1;
   //if (GetSeg() > in2.GetSeg() ) return 1;
   return 0;
}

ostream& operator<<(ostream &os, const SSLEntry& en)
{
  return (os   <<" CHS("<< en.GetSeg()
               <<") (k: "<< en.Getk() << " a:" << en.Geta() <<") ");
}

/*
class   - SSL
 Point point (true,event.GetX(), event.GetY() );

*/

class StatusLine
{
public:
   friend class VList;
   StatusLine();
   ~StatusLine() {};
   void Insert(Coord x, SSLEntry& entry, Segment sgs[], PQueue& pq);
   void Delete(Coord x, Coord oldx, SSLEntry& entry, Segment sgs[],PQueue& pq );
   void Exchange (BinTreeNode<SSLEntry>* e1, BinTreeNode<SSLEntry>* e2, Segment sgs[], PQueue& pq);
   BinTreeNode<SSLEntry>* Find(const Coord x, const Coord oldx, const SSLEntry& entry,Segment segs[]);

   void output(Coord x, Segment segs[]);

private:
   BinTree<SSLEntry> ssl;
   bool IsEmpty();
   int Size();
   bool InsertIntersection(const CHalfSegment& chs1,const CHalfSegment& chs2,int s1,int s2,
   	PQueue& pq );
   void erase(SSLEntry* entry);
   BinTreeNode<SSLEntry>* GetGreater(Coord xvalue, Coord y, Segment segs[]);
   BinTreeNode<SSLEntry>* GetNext(BinTreeNode<SSLEntry>* ent);
   BinTreeNode<SSLEntry>* GetPred(BinTreeNode<SSLEntry>* ent);

};

void StatusLine::output(double x, Segment segs[]) {
   //cout << "Statuslinie: bei x  size:" << ssl.GetCount() << "    BEI X:" << x << endl;;
   BinTreeNode<SSLEntry>* p = ssl.GetFirst();
   if  (! ssl.IsEmpty() ) {
      while ( p != 0 ) {
         SSLEntry en = p->GetEntry();
         cout << "y:"<< en.GetY(x, segs) << "  seg:" << en.GetSeg() << " k:" << en.Getk()
         << "  a:" << en.Geta() << endl;
         p = p->GetNext();
      }
   }
}


StatusLine::StatusLine()	{ BinTree<SSLEntry> ssl;}
bool StatusLine::IsEmpty() 	{ return  ssl.IsEmpty() ; }
int StatusLine::Size()		{ return ssl.GetCount(); }
//BinTree<SSLEntry* StatusLine::GetMin()  { return ssl.GetFirst(); }
BinTreeNode<SSLEntry>* StatusLine::GetNext(BinTreeNode<SSLEntry>* ent)  {return ent->GetNext();}
BinTreeNode<SSLEntry>* StatusLine::GetPred(BinTreeNode<SSLEntry>* ent)  {return ent->GetPred();}


BinTreeNode<SSLEntry>* StatusLine::GetGreater(Coord x, Coord y, Segment segs[])
{
   BinTreeNode<SSLEntry>* node = ssl.GetFirst();
   while ( node != 0 && (node->GetEntry().GetY(x, segs) <= y) )
       node = node -> GetNext();
   return node;
}

BinTreeNode<SSLEntry>* StatusLine::Find(const Coord x, const Coord oldx, const SSLEntry& entry,
		 Segment segs[])
{
   BinTreeNode<SSLEntry>* node = ssl.Find(entry, x, segs);
   if (node == 0) {
      node = ssl.Find(entry, oldx, segs );
      if (node == 0) {
         node = ssl.GetFirst();
	 while (node != 0 && node->GetEntry().GetSeg() != entry.GetSeg()  )
	    node = node->GetNext() ;
       }
   }
   return node;
}

void StatusLine::Insert(Coord x, SSLEntry& entry, Segment sgs[], PQueue& pq)
{
   BinTreeNode<SSLEntry>* en = ssl.Insert(entry, x, sgs);
   // test for intersection with pred- and succ-CHalfSegment
   BinTreeNode<SSLEntry>* pred = en->GetPred();
   BinTreeNode<SSLEntry>* succ = en->GetNext();
   int segnr = entry.GetSeg();
   if (pred != 0 ) { // pred exists
      int psegnr = pred->GetEntry().GetSeg();
      bool inter = InsertIntersection (sgs[psegnr].GetCHS(),sgs[segnr].GetCHS(),
	              psegnr, segnr, pq);
         if (inter) {
	    BinTreeNode<SSLEntry>* predpred = pred->GetPred();
	    if (predpred != 0 ) { // predpred exists
               int ppsegnr = predpred->GetEntry().GetSeg();
	       bool inter2 = InsertIntersection(sgs[ppsegnr].GetCHS(),
	                     sgs[segnr].GetCHS(), ppsegnr,segnr,pq);
	       if (inter2) {
                  BinTreeNode<SSLEntry>* predpredpred = predpred->GetPred();
	          if (predpredpred !=0) {
	             int pppSegnr = predpredpred->GetEntry().GetSeg();
                     InsertIntersection(sgs[pppSegnr].GetCHS(),sgs[segnr].GetCHS(),
		     pppSegnr,segnr,pq);
		  }
	       }
	    }
         }
   }
   if (succ != 0) {   // succ exist
      int sSegnr = succ->GetEntry().GetSeg();
      bool inter = InsertIntersection (sgs[segnr].GetCHS(), sgs[sSegnr].GetCHS(),
	              segnr, sSegnr, pq);
      if (inter) {
	BinTreeNode<SSLEntry>* succsucc = succ->GetNext();
	if (succsucc != 0 ) { // succsucc exists
            int ssSegnr = succsucc->GetEntry().GetSeg();
	    bool inter2= InsertIntersection (sgs[segnr].GetCHS(),
		         sgs[ssSegnr].GetCHS(), segnr, ssSegnr,pq);
	    if (inter2) {
	       BinTreeNode<SSLEntry>* succsuccsucc = succsucc->GetNext();
	       if (succsuccsucc !=0) {
	          int sssSegnr = succsuccsucc->GetEntry().GetSeg();
		  InsertIntersection(sgs[segnr].GetCHS(),sgs[sssSegnr].GetCHS(),
		  segnr,sssSegnr,pq);
	       }
	    }
         }
      }
   }
}

void StatusLine::Delete(Coord x, Coord oldx, SSLEntry& entry, Segment sgs[], PQueue& pq)
{
   BinTreeNode<SSLEntry>* en = Find(x, oldx, entry, sgs);
   if (en != 0) {
      BinTreeNode<SSLEntry>* pred = en->GetPred();
      BinTreeNode<SSLEntry>* succ = en->GetNext();
      if ( pred != 0 && succ != 0)
      {
         int s1 = pred->GetEntry().GetSeg();
	 int s2 = succ->GetEntry().GetSeg();
         // test for intersection with pred- and succ-CHalfSegment
         if (sgs[s1].GetIn1() != sgs[s2].GetIn1() )
            InsertIntersection (sgs[s1].GetCHS(), sgs[s2].GetCHS(), s1, s2, pq);
      }
      ssl.DeleteNode(en);
   }
}

bool StatusLine::InsertIntersection(const CHalfSegment& chs1,const CHalfSegment& chs2,int s1,
	int s2, PQueue& pq)
{
   bool result = false;
   Point point;
   Coord xp, yp;
   CHalfSegment res;
   bool one, two;
   bool innerinter = chs1.innerInter(chs2, point, res, one, two);
   if (innerinter)  {
      result = true;
      if (point.IsDefined() ) { // intersection in one point
	 xp = point.GetX();  yp = point.GetY();
	 if ( one && two) {XEvent ev1(xp, yp, s1, s2, intersection); pq.insert(ev1);}
	 else if (one)    {XEvent ev1(xp, yp, s1, split); pq.insert(ev1); }
	 else if (two)    {XEvent ev1(xp, yp, s2, split); pq.insert(ev1); }
      }
      else if (res.IsDefined() ) {  // overlap-intersection
	 if (chs1 == chs2) { return true;}
	 else if (chs1.GetLP() == chs2.GetLP() ) {
	    if (chs1.Inside(chs2) ) {
	       xp = chs1.GetRP().GetX();  yp = chs1.GetRP().GetY();
	       XEvent ev1( xp, yp, s2, split);  pq.insert(ev1);
     	    }
	    else {
	       xp = chs2.GetRP().GetX();  yp = chs2.GetRP().GetY();
	       XEvent ev1(xp, yp, s1, split);   pq.insert(ev1);
	    }
	 }
	 else if (chs1.GetRP() == chs2.GetRP() )  {
	    if (chs1.Inside(chs2) ) {
	       xp = chs1.GetLP().GetX();  yp = chs1.GetLP().GetY();
	       XEvent ev1(xp, yp, s2, split);   pq.insert(ev1);
	    }
	    else {
	       xp = chs2.GetLP().GetX();  yp = chs2.GetLP().GetY();
	       XEvent ev1(xp, yp, s1, split);   pq.insert(ev1);
	    }
	 }
	 else if (chs1.Inside(chs2)) {
	    xp = chs1.GetLP().GetX();   yp = chs1.GetLP().GetY();
	    XEvent ev1(xp, yp, s2, split);      pq.insert(ev1);
	    xp = chs1.GetRP().GetX();   yp = chs1.GetRP().GetY();
	    XEvent ev2(xp, yp, s2, split);      pq.insert(ev2);
	 }
	 else if (chs2.Inside(chs1)) {
	    xp = chs2.GetLP().GetX();  yp = chs2.GetLP().GetY();
	    XEvent ev1 (xp, yp, s1, split);     pq.insert(ev1);
	    xp = chs2.GetRP().GetX();   yp = chs2.GetRP().GetY();
	    XEvent ev2 (xp, yp, s1, split);     pq.insert(ev2);
	 }
	 else  { // overlap without Inside
	    if (chs1.GetRP() > chs2.GetRP() )   {
	       xp = chs1.GetLP().GetX();  yp = chs1.GetLP().GetY();
	       XEvent ev1(xp, yp, s2, split);   pq.insert(ev1);
	       xp = chs2.GetRP().GetX();  yp = chs2.GetRP().GetY();
	       XEvent ev2(xp, yp, s1, split);   pq.insert(ev2);
	    }
	    else {
	       xp = chs1.GetRP().GetX();   yp = chs1.GetRP().GetY();
	       XEvent ev1(xp, yp, s2, split);    pq.insert(ev1);
	       xp = chs2.GetLP().GetX();   yp = chs2.GetLP().GetY();
	       XEvent ev2(xp, yp, s1, split);    pq.insert(ev2);
	    }
	 } // overlap without Inside
      }  // res.Defined()
   } // innerinter
   return result;
}

void StatusLine::Exchange (BinTreeNode<SSLEntry>* e1, BinTreeNode<SSLEntry>* e2,
      Segment sgs[], PQueue& pq)
{
   BinTreeNode<SSLEntry>* pred = e1->GetPred();
   BinTreeNode<SSLEntry>* succ = e2->GetNext();
   SSLEntry entry1 ( e1->GetEntry() );
   SSLEntry entry2 ( e2->GetEntry() );
   e2->SetEntry(entry1);
   e1->SetEntry(entry2);
   if (pred != 0 ) {
      int e = e1->GetEntry().GetSeg();
      int s1 = pred->GetEntry().GetSeg();
      if (sgs[s1].GetIn1() != sgs[e].GetIn1() )
         InsertIntersection (sgs[s1].GetCHS(),sgs[e].GetCHS(), s1, e, pq );
   }
   if (succ != 0 ) {
      int es = e2->GetEntry().GetSeg();
      int s2 = succ->GetEntry().GetSeg();
      if (sgs[es].GetIn1() != sgs[s2].GetIn1() )
         InsertIntersection (sgs[es].GetCHS(), sgs[s2].GetCHS(), es, s2, pq );
   }
}

/*
class VStructure

*/

class VStructure {
public:
   friend class VList;
   VStructure();
   ~VStructure()   {}
   void Insert (Coord y);
   void SetDefined(bool def)   	{ defined = def;}
   bool IsDefined () 		{ return defined; }
   void Clear();
private:
   list<Coord>  vstruct;
   bool defined;
   Coord min, max;
   bool IsEmpty()   		{ return vstruct.empty(); }
   Coord GetMin()   		{ return min; }
   Coord GetMax()   		{ return max; }
   void Output();
};

void VStructure::Output()
{
   cout << " in output Vstructure " << endl;
   list<Coord>::iterator p = vstruct.begin();
   while (p != vstruct.end())
   {
      Coord y = *p;
      cout << "   y = " << y << endl;
      ++p;
   }
   if (IsDefined() ) cout << " in output fertig min " << min << "   max" << max << endl;
}

VStructure::VStructure()
{
   list<Coord> vstruct;
   defined = false;
}

void VStructure::Clear()
{
   vstruct.clear();
   SetDefined(false);
}

void VStructure::Insert (Coord y)
{
   if (IsEmpty()) {
      vstruct.insert(vstruct.begin(), y);
      min = y;
      max = y;
   }
   list<Coord>::iterator p = vstruct.begin();
   while ( (*p < y) && (p != vstruct.end()) )   { ++p; }
   if ( (*p) != y ) {
      vstruct.insert(p,y);
      if (y < min)   { min = y;}
      else if (y > max)  {max = y;}
   }
}

/*
class vlist

*/

class VList {
public:
   VList();
   ~VList() {};
   void Insert (Segment& seg);
   list<Segment> makeClear(Coord& sweepline, VStructure& vs, StatusLine& sl, Segment segs[]);
    void Clear();
    bool IsEmpty();

private:
   list<Segment> vlist;

   void DeleteFirst();
   Segment First();
   int Size();
   list<Segment> eraseOverlaps(Coord sline, VStructure vs);
   void testStatusLine(StatusLine& sl, Segment segs[]);
   void testOverlaps(Coord& sweepline, VStructure& vs, StatusLine& sl, Segment segs[]);
   void output();
};

void VList::Clear()  	   { vlist.clear(); }
VList::VList()     	   { list<Coord> vlist; }
int VList::Size()    	   { return vlist.size(); }
void VList::Insert (Segment& seg)   {vlist.push_back(seg);}
Segment VList::First()   { return (vlist.front()); }
void VList::DeleteFirst()  { vlist.erase(vlist.begin()); }
bool VList::IsEmpty()	   { return vlist.empty(); }


void VList::output()
{
   cout << " in output VList " << endl;
   list<Segment>::iterator p = vlist.begin();
   while (p != vlist.end())
   {
      Segment s = *p;
      cout << "   Segment = " << s.GetIn1() << " " << s.GetCHS().GetLP() << " " << s.GetCHS().GetRP() << endl;
      ++p;
   }
   cout << " in output fertig " << endl;
}


list<Segment> VList::makeClear(Coord& sweep, VStructure& vstruct, StatusLine& ssl, Segment segments[])
{
   if ( ! IsEmpty() )  {
      testOverlaps( sweep ,vstruct, ssl, segments);
      testStatusLine (ssl, segments);
   }
   return vlist;
}

void VList::testOverlaps(Coord& sweepline, VStructure& vs, StatusLine& sl, Segment segs[])
{
   list<Segment> newlist;
   while ( !IsEmpty() )  {
      bool deleted = false;
      Segment first = First();
      CHalfSegment chs = first.GetCHS();
      DeleteFirst();
      list<Segment>::iterator p = vlist.begin();
      // test first Segment for overlaps with other CHalfSegments in vlist
      while ( (p != vlist.end()) &&  (deleted == false)  )  {
         Segment ref = *p;
	 CHalfSegment test = ref.GetCHS();
	 CHalfSegment res;
	 bool overlap = chs.overlapintersect(test,res);
	 if (overlap  && ( first.GetIn1() != ref.GetIn1()) ) {
	    // different regions
	    // one inside the other CHalfSegment
	    if (chs.Inside(test) ) {
	       if (chs.GetLP() == test.GetLP() && chs.GetRP() == test.GetRP() ) { }
	       else if (chs.GetLP() == test.GetLP() ) {
		  Segment newseg(ref.GetIn1(),test); newseg.SetRP(res.GetRP());
		  vlist.insert(p,newseg);
		  ref.SetLP(res.GetRP());
		  vlist.erase(p);  vlist.push_back(ref);
	       }
	       else if (chs.GetRP() == test.GetRP()) {
	          Segment newseg(ref.GetIn1(),test);  	newseg.SetRP(res.GetLP());
		  vlist.insert(p,newseg);
		  ref.SetLP(res.GetLP());
		  vlist.erase(p); vlist.push_back(ref);
	       }
	       else  {
	          Segment newseg1(ref.GetIn1(),test); newseg1.SetRP(res.GetLP());
		  vlist.insert(p,newseg1);
		  Segment newseg2(ref.GetIn1(), test);
		  newseg2.SetLP(res.GetLP()); newseg2.SetRP(res.GetRP());
		  vlist.insert(p,newseg2);
		  ref.SetLP(res.GetRP());
		  vlist.erase(p); vlist.push_back(ref);
	       }
	    }
	    else if (test.Inside(chs))  {
	       if (chs.GetLP() == test.GetLP() ) {
		  Segment newseg(first.GetIn1(),chs); newseg.SetRP(res.GetRP());
		  vlist.insert(p,newseg);
		  first.SetLP(res.GetRP());
               }
	       else if (chs.GetRP() == test.GetRP()) {
		  Segment newseg(first.GetIn1(),chs); newseg.SetRP(res.GetLP());
		  vlist.insert(p,newseg);
		  first.SetLP(res.GetLP());
	       }
	       else  {
		  Segment newseg1(first.GetIn1(),chs); newseg1.SetRP(res.GetLP());
		  vlist.insert(p,newseg1);
		  Segment newseg2(first.GetIn1(),chs);
		  newseg2.SetLP(res.GetLP()); newseg2.SetRP(res.GetRP());
		  vlist.insert(p,newseg2);
		  first.SetLP(res.GetRP());
	       }
	    }
	    // overlap but not inside
	    else if (res.GetLP() > chs.GetLP())  {
	       Segment newseg1(first.GetIn1(),chs); newseg1.SetRP(res.GetLP());
	       vlist.insert(p,newseg1);
	       first.SetLP(res.GetLP());
	       Segment newseg2(ref.GetIn1(),test); newseg2.SetRP(res.GetRP());
	       vlist.insert(p,newseg2);
	       ref.SetLP(res.GetRP());
	       vlist.erase(p); vlist.push_back(ref);
	    }
	    else   {
	       Segment newseg1(first.GetIn1(),chs); newseg1.SetRP(res.GetRP());
	       vlist.insert(p,newseg1);
	       first.SetLP(res.GetRP());
	       Segment newseg2(ref.GetIn1(),test); newseg2.SetRP(res.GetLP());
	       vlist.insert(p,newseg2);
	       ref.SetLP(res.GetLP());
	       vlist.erase(p); vlist.push_back(ref);
	    }
	 } // end overlap
         ++p;
      } // end 2nd while and now treat rightEnds of CHAlfSegments
      Coord min = vs.GetMin();
      Coord max =  vs.GetMax();
      chs = first.GetCHS();
      if (!((chs.GetLP().GetY()<=min && chs.GetRP().GetY()<= min)
	 ||    (chs.GetLP().GetY()>=max && chs.GetRP().GetY()>= max))) {
  	 list<Coord>::iterator ps = vs.vstruct.begin();
	 bool ready = false;
	 while (ps != vs.vstruct.end() and ready != true) {
	    Coord y = *ps;
            if (y >= chs.GetRP().GetY())   ready = true;
	    else if (y > chs.GetLP().GetY()) {// split CHalfsegment
	       Point newp = Point(true, sweepline,y);
               Segment newseg(first.GetIn1(),first.GetCHS()); newseg.SetRP(newp);
	       newlist.push_back(newseg);
	       first.SetLP(newp);
	    }
	    ps++;
	 }
      }
      newlist.push_back (first);
   } // end 1st while
   vs.Clear();       // clean the V-Structure
   vlist = newlist;
}

void  VList::testStatusLine(StatusLine& sline, Segment segs[])
{
   list<Segment> newlist;
   list<Segment>::iterator p = vlist.begin();
   while ( ! IsEmpty() )  {
      Segment vertref = First();
      CHalfSegment vert = vertref.GetCHS();
      DeleteFirst();
      Coord x = vert.GetLP().GetX();
      Coord y = vert.GetLP().GetY();
      BinTreeNode<SSLEntry>* node = sline.GetGreater(x,y,segs);
      while (node != 0)  {
         SSLEntry  ent = node -> GetEntry();
         Segment testref = segs[ent.GetSeg()];
         CHalfSegment test = testref.GetCHS() ;
         Point res;
         bool inter = vert.spintersect (test, res);
	 if (inter) {
	    Segment newseg1(vertref.GetIn1(), vert);
	    newseg1.SetLP(res);
	    vlist.push_front (newseg1);
	    vertref.SetRP(res);
	    Segment newseg2(testref.GetIn1(), test);
	    newseg2.SetRP(res);
	    newlist.push_back(newseg2);
	    testref.SetLP(res);
            segs[ent.GetSeg()] = testref;
         } // end inter
	 node = node->GetNext();
	 if ( node != 0 && node->GetEntry().GetY(x, segs) > vert.GetRP().GetY()) {node = 0;}
      } // all entries in statusline tested
      p++; // next vertical Segment
      newlist.push_back(vertref);
   }
   vlist = newlist;
}

/*
class MakeRealm

*/

class MakeRealm {
public:
   MakeRealm() {};
   ~MakeRealm() {};
   void PQueueOutput(PQueue& pq);
   void  REALM (CRegion* reg1, CRegion* reg2, CRegion* result1, CRegion* result2);
 void  REALM (CRegion* reg1, CRegion* reg2, CLine* result1, CLine* result2);
   void  REALM (CLine* reg1, CRegion* reg2, CLine* result1, CRegion* result2);
   void  REALM (CLine* reg1, CLine* reg2, CLine* result1, CLine* result2);

private:
   void PerformPlaneSweep(PQueue& pq, Segment segs[], list<CHalfSegment>& res1,
      list<CHalfSegment>& res2, const int counter);
   void PrepareCHS (bool No1, CHalfSegment& chs, PQueue& pqu, Segment segs[],
      int counter);
};

void MakeRealm::REALM(CRegion* reg1, CRegion* reg2, CRegion* result1, CRegion* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the first region for PlanSweep
   Segment segs[ (reg1->Size() + reg2->Size() )  / 2 ];
   int count = 0;
   // insert all segments of region1 into priority queue
   for (int i=0; i<reg1->Size(); i++)   {
      CHalfSegment chs;
      reg1->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(true, chs, pqueue, segs, count);
         count ++;
      }
   }
   // Prepare the second region for PlaneSweep
   for (int i=0; i<reg2->Size(); i++)   {
      CHalfSegment chs;
      reg2->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(false, chs, pqueue, segs, count);
         count ++;
      }
   }
   list<CHalfSegment> res1, res2;
   PerformPlaneSweep(pqueue, segs, res1, res2, count);
   //  reconstruction of first region
   result1->Clear();
   result1->StartBulkLoad();
   int counter = 0;
   //cout << " region 1 neu berechnet" << endl;
   while (! res1.empty() )  {
      CHalfSegment chs = res1.back();
      chs.attr.partnerno = counter;
      chs.SetLDP(true);       result1->InsertHs(chs);
      chs.SetLDP(false);      result1->InsertHs(chs);
      res1.pop_back();
      counter ++;
      //cout << " chs ausgabe 1 " << chs << endl;
   }
   result1 -> EndBulkLoad(true);
   result1->SetPartnerNo();
   result1->ComputeRegion();
   result2->Clear();
   result2->StartBulkLoad();
   counter = 0;
   // cout << " region 2 neu berechnet" << endl;
   while (! res2.empty() )    {
      CHalfSegment chs = res2.front();
      chs.attr.partnerno = counter;
      chs.SetLDP(true);       result2->InsertHs(chs);
      chs.SetLDP(false);      result2->InsertHs(chs);
      res1.erase(res2.begin());
      counter++;
      //cout << " chs ausgabe 2 " << chs << endl;
   }
   result2 -> EndBulkLoad(true);
   result2->SetPartnerNo();
   result2->ComputeRegion();
}

void MakeRealm::REALM(CRegion* reg1, CRegion* reg2, CLine* result1, CLine* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the first region for PlanSweep
   Segment segs[ (reg1->Size() + reg2->Size()) / 2 ];
   int count = 0;
   // insert all segments of region1 into priority queue
   for (int i=0; i<reg1->Size(); i++)   {
      CHalfSegment chs;
      reg1->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(true, chs, pqueue, segs, count);
         count ++;
      }
   }
   // Prepare the second region for PlaneSweep
   for (int i=0; i<reg2->Size(); i++)   {
      CHalfSegment chs;
      reg2->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(false, chs, pqueue, segs, count);
         count ++;
      }
   }
   list<CHalfSegment> res1, res2;
   PerformPlaneSweep(pqueue, segs, res1, res2, count);
   //  reconstruction of first region
   result1->Clear();
   result1->StartBulkLoad();
   int counter = 0;
   //cout << " region 1 neu berechnet" << endl;
   while (! res1.empty() )  {
      CHalfSegment chs = res1.back();
      chs.attr.partnerno = counter;
      chs.SetLDP(true);       result1->InsertHs(chs);
      chs.SetLDP(false);      result1->InsertHs(chs);
      res1.pop_back();
      counter ++;
      //cout << " chs ausgabe 1 " << chs << endl;
   }
   result1 -> EndBulkLoad(true);
   result2->Clear();
   result2->StartBulkLoad();
   counter = 0;
   // cout << " region 2 neu berechnet" << endl;
   while (! res2.empty() )    {
      CHalfSegment chs = res2.front();
      chs.attr.partnerno = counter;
      chs.SetLDP(true);       result2->InsertHs(chs);
      chs.SetLDP(false);      result2->InsertHs(chs);
      res1.erase(res2.begin());
      counter++;
      //cout << " chs ausgabe 2 " << chs << endl;
   }
   result2 -> EndBulkLoad(true);
}

void MakeRealm::REALM(CLine* line1, CRegion* reg2, CLine* result1, CRegion* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the first region for PlanSweep
   Segment segs[ (line1->Size() + reg2->Size()) / 2 ];
   int count = 0;
   // insert all segments of region1 into priority queue
   for (int i=0; i<line1->Size(); i++)   {
      CHalfSegment chs;
      line1->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(true, chs, pqueue, segs, count);
         count ++;
      }
   }
   // Prepare the second region for PlaneSweep
   for (int i=0; i<reg2->Size(); i++)   {
      CHalfSegment chs;
      reg2->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(false, chs, pqueue, segs, count);
         count ++;
      }
   }
   list<CHalfSegment> res1, res2;
   PerformPlaneSweep(pqueue, segs, res1, res2, count);
   //  reconstruction of first region
   result1->Clear();
   result1->StartBulkLoad();
   while (! res1.empty() )  {
      CHalfSegment chs = res1.back();
      chs.SetLDP(true);       result1->InsertHs(chs);
      chs.SetLDP(false);      result1->InsertHs(chs);
      res1.pop_back();
   }
   result1 -> EndBulkLoad(true);
   result2->Clear();
   result2->StartBulkLoad();
   int counter = 0;
   while (! res2.empty() )    {
      CHalfSegment chs = res2.front();
      chs.attr.partnerno = counter;
      chs.SetLDP(true);       result2->InsertHs(chs);
      chs.SetLDP(false);      result2->InsertHs(chs);
      res1.erase(res2.begin());
      counter++;
   }
   result2 -> EndBulkLoad(true);
   result2->SetPartnerNo();
   result2->ComputeRegion();
}

void MakeRealm::REALM(CLine* line1, CLine* line2, CLine* result1, CLine* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the first region for PlanSweep
   Segment segs[ (line1->Size() + line2->Size()) /2 ];
   int count = 0;
   // insert all segments of region1 into priority queue
   for (int i=0; i<line1->Size(); i++)   {
      CHalfSegment chs;
      line1->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(true, chs, pqueue, segs, count);
         count ++;
      }
   }
   // Prepare the second region for PlaneSweep
   for (int i=0; i< line2->Size(); i++)   {
      CHalfSegment chs;
      line2->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(false, chs, pqueue, segs, count);
         count ++;
      }
   }
   list<CHalfSegment> res1, res2;
   PerformPlaneSweep(pqueue, segs, res1, res2, count);
   //  reconstruction of first region
   result1->Clear();
   result1->StartBulkLoad();
   while (! res1.empty() )  {
      CHalfSegment chs = res1.back();
      chs.SetLDP(true);       result1->InsertHs(chs);
      chs.SetLDP(false);      result1->InsertHs(chs);
      res1.pop_back();
   }
   result1 -> EndBulkLoad(true);
   result2->Clear();
   result2->StartBulkLoad();
   while (! res2.empty() )    {
      CHalfSegment chs = res2.back();
      chs.SetLDP(true);       result2->InsertHs(chs);
      chs.SetLDP(false);      result2->InsertHs(chs);
      res2.pop_back();
   }
   result2 -> EndBulkLoad(true);
}


void MakeRealm::PrepareCHS ( bool No1, CHalfSegment& chs, PQueue& pqu,
   Segment segs[], int counter)
{
   Coord xl, xr, yl, yr;
   xl=chs.GetLP().GetX();
   yl=chs.GetLP().GetY();
   xr=chs.GetRP().GetX();
   yr=chs.GetRP().GetY();
   // only CHalfsegments with a left point as dominating point are used
   // build up array with all CHalfSegments of Region
   Segment se (No1,chs);
   segs[counter] = se;
   // insert XEvents in PQueue
   // insert a vertical Segment
   if ( xl==xr)
   {
      // insert bottom of vertical chs
      XEvent ev1(xl, yl ,counter, 0.0, 0.0, bottomSeg);
      pqu.insert(ev1);
   }
   // insert a not vertical chs
   else
   { // calculate k and a
#ifdef RATIONAL_COORDINATES
      double k= ((yr.IsInteger()? yr.IntValue():yr.Value()) -
                 (yl.IsInteger()? yl.IntValue():yl.Value())) /
                ((xr.IsInteger()? xr.IntValue():xr.Value()) -
                 (xl.IsInteger()? xl.IntValue():xl.Value())) ;
      double a = (yl.IsInteger()? yl.IntValue():yl.Value()) -
                  k* (xl.IsInteger()? xl.IntValue():xl.Value());
#else
      double k = (yr - yl) / (xr - xl) ;
      double a = yl - k*xl;
#endif
      // insert left end of chs
      XEvent ev2(xl, yl, counter, k, a, leftSeg);
      pqu.insert(ev2);
       // insert right end of chs
      XEvent ev3(xr, yr, counter, k, a, rightSeg);
      pqu.insert(ev3);
   }
}

void MakeRealm::PerformPlaneSweep(PQueue& pq, Segment segs[], list<CHalfSegment>& list1, list<CHalfSegment>& list2, const int counter)
{
   set<int> mi;
   Point oldP;
   VList vlist;
   VStructure vs;
   StatusLine sl;
   if ( pq.isEmpty())  return;
   Coord sweepline, oldsweep;
   while (! pq.isEmpty())  {
      XEvent event = pq.getFirstAndDelete();
     if (sweepline != event.GetX())  { // new sweepline
         oldsweep = sweepline;
      	 // handle all vertical CHalfSegments at old sweepline
         if ( !vlist.IsEmpty() ) {
	    list<Segment> newlist = vlist.makeClear(sweepline, vs, sl, segs);
	    list<Segment>::iterator iter = newlist.begin();
            while (iter != newlist.end()) {
               Segment segment = *iter;
	       segment.CHSInsert(list1,list2);
               ++iter;
            }
	    vlist.Clear();   vs.Clear();
	 }
         sweepline = event.GetX();
      }
      Segment seg = segs[event.GetFirst()];
      // XEvent for left end of CHalfSegment
      SSLEntry entry (event.GetFirst(),event.GetSlope(), event.GetA());
      if (event.GetKind() == leftSeg)   {
         sl.Insert(event.GetX(),entry, segs, pq);
         if ( vs.IsDefined())   vs.Insert(event.GetY());
      }
      // XEvent for right end of CHalfSegment
      else if (event.GetKind() == rightSeg) {
         sl.Delete(event.GetX(), oldsweep, entry, segs, pq);
	 if ( vs.IsDefined())   vs.Insert(event.GetY());
	 segs[event.GetFirst()].CHSInsert(list1,list2);
      }
      else if (event.GetKind() == split) {
         Point point (true, event.GetX(), event.GetY() );
         Segment new1 ( seg.GetIn1(), seg.GetCHS());
	 if ( (point != new1.GetLP()) && (point != new1.GetRP() ) ) {
	    new1.SetRP(point);     new1.CHSInsert(list1,list2);
	    seg.SetLP(point);      segs[event.GetFirst()] = seg;
	 }
      }
      // XEvent intersection
      else if (event.GetKind() == intersection)  {
         Point p (true, event.GetX(), event.GetY() ) ;
	 if (oldP.IsDefined() && p.GetX()== oldP.GetX() && p.GetY() == oldP.GetY() ){
	    // multiple intersections in one point
	    Segment seg2 = segs[event.GetSecond()];
	    if (mi.find(event.GetFirst()) == mi.end()) {  // split first Segment
	       SSLEntry entry1 (event.GetFirst(), segs);
	       sl.Delete(event.GetX(), oldsweep, entry1, segs, pq);
	       Segment new1 ( seg.GetIn1(), seg.GetCHS());  new1.SetRP(p);
	       new1.CHSInsert(list1,list2);
	       seg.SetLP(p);	segs[event.GetFirst()] = seg;
	       mi.insert (event.GetFirst() );
	       SSLEntry entry2(event.GetFirst(), segs);
               sl.Insert(event.GetX(), entry2, segs, pq);
	    }
	    if ( mi.find(event.GetSecond()) == mi.end()  ){ // split second segment
	       SSLEntry entry1 (event.GetSecond(), segs);
	       sl.Delete(event.GetX(), oldsweep, entry1, segs, pq);
	       Segment new2 (seg2.GetIn1(),seg2.GetCHS());  new2.SetRP(p);
	       new2.CHSInsert(list1,list2);
	       seg2.SetLP(p);	segs[event.GetSecond()] = seg2;
	       mi.insert (event.GetSecond());
	       SSLEntry entry2(event.GetSecond(),segs);
               sl.Insert(event.GetX(),entry2, segs, pq);
	    }
	 }
	 else {  // first intersection-event in this point
            Segment seg2 = segs[event.GetSecond()];
	    Segment new1 ( seg.GetIn1(), seg.GetCHS());  new1.SetRP(p);
	    Segment new2 (seg2.GetIn1(),seg2.GetCHS());  new2.SetRP(p);
	    new1.CHSInsert(list1,list2);	new2.CHSInsert(list1,list2);
	    CHalfSegment chstest(seg.GetCHS());
	    seg.SetLP(p);	segs[event.GetFirst()] = seg;
	    seg2.SetLP(p);	segs[event.GetSecond()] = seg2;
	    // first Intersection in this event-point
	    oldP = p;
	    mi.clear();
	    mi.insert(event.GetFirst());	  mi.insert(event.GetSecond() );
	    SSLEntry entry1(event.GetFirst(), segs);
	    BinTreeNode<SSLEntry>* node1 = sl.Find(sweepline, oldsweep, entry1, segs );
	    BinTreeNode<SSLEntry>* node2 = node1->GetNext();
	    if (node2 !=0 && node2->GetEntry().GetSeg() == event.GetSecond() ) {}
	    else node2 = node1->GetPred();
	    sl.Exchange(node1, node2, segs, pq);
	 }
      }
      // XEvent - bottom of a vertical Segment
      else if (event.GetKind() == bottomSeg)  {
        vs.SetDefined(true);                     // build up VStructure
	vlist.Insert(segs[event.GetFirst()]);    // insert Segment in VList
      }
      else  {   cout << " wrong eventkind !!!!!!!!!!!!!!!!!!!!"; }
   }
}

class SEntry {
public:
   SEntry() {};
   ~SEntry() {};
   SEntry(CHalfSegment& inch);
   SEntry(SEntry* in);
   void Set(const SEntry& in);
   const double GetY(Coord x) const;
   int GetU() const;
   int GetO() const;
   CHalfSegment GetCHS() const;
   void SetU(int newU);
   void SetO(int newO);
   int Less (const SEntry ev2, const Coord x, const SEntry oldev,BinTreeNode<SEntry>* oldnode ) const;
   bool Equal (const SEntry in2) const;
   SEntry& operator= (const SEntry& in);

public:  // eigentlich private
   double GetSlope() const;
   double GetA() const;

private:
   CHalfSegment ch;
   double slope;
   double a;
   int u;
   int o;



};

ostream& operator<<(ostream &os, const SEntry& en)
{
  return (os   <<" CHS("<<en.GetCHS().GetLDP()
               <<") ("<< en.GetCHS().GetLP() << " "<< en.GetCHS().GetRP() <<")  u=" << en.GetU() << "  o=" <<en.GetO() );
}


SEntry::SEntry(SEntry* in)
{
   ch = in-> ch;
   slope = in->slope;
   a = in->a;
   o = in -> o;
   u = in ->u;
}

SEntry::SEntry(CHalfSegment& inch)
{
   ch = inch;
   Coord xl, xr, yl, yr;
   xl = ch.GetLP().GetX();
   xr = ch.GetRP().GetX();
   yl = ch.GetLP().GetY();
   yr = ch.GetRP().GetY();
#ifdef RATIONAL_COORDINATES
   double ink= ((yr.IsInteger()? yr.IntValue():yr.Value()) -
                (yl.IsInteger()? yl.IntValue():yl.Value())) /
               ((xr.IsInteger()? xr.IntValue():xr.Value()) -
               (xl.IsInteger()? xl.IntValue():xl.Value())) ;
   double ina = (yl.IsInteger()? yl.IntValue():yl.Value()) -
               ink* (xl.IsInteger()? xl.IntValue():xl.Value());
#else
   double ink = (yr - yl) / (xr - xl) ;
   double ina = yl - ink*xl;
#endif
   slope = ink ;
   a = ina;
   u=0;
   o=0;
}

void SEntry::Set(const SEntry& in) {
   ch = in.ch;
   slope = in.slope;
   a = in.a;
   o = in.o;
   u = in.u;
}

SEntry& SEntry::operator= (const SEntry& in)  { return *this;}
CHalfSegment SEntry::GetCHS() const	{return ch;}
int SEntry::GetU() const	     	{return u;}
int SEntry::GetO() const	     	{return o;}
double SEntry::GetSlope() const		{return slope;}
double SEntry::GetA() const		{return a;}
void SEntry::SetU (int newU)		{ u = newU; }
void SEntry::SetO (int newO) 		{ o = newO; }


const double SEntry::GetY(Coord x) const  {
 /*  Coord res;
   bool end = false;
   if (ch.GetLP().GetX() == x) { end = true; res = ch.GetLP().GetY(); }
   else if (ch.GetRP().GetX() == x) { end = true; res = ch.GetRP().GetY(); }
   if ( end) {
     #ifdef RATIONAL_COORDINATES
      double y = (res.IsInteger()? res.IntValue(): res.Value()) ;
     #else
      double y = res ;
     #endif
      return y;
   }
     else {
 */   #ifdef RATIONAL_COORDINATES
      double xv = (x.IsInteger()? x.IntValue(): x.Value()) ;
      #else
      double xv = x ;
      #endif
      return ( slope*xv + a);
  // }
}

bool SEntry::Equal (const SEntry in2) const
{
   if (GetCHS() == in2.GetCHS() )  return true;
   else { return false;}
}


int SEntry::Less (const SEntry in2, const Coord x, const SEntry oldev, BinTreeNode<SEntry>* oldnode) const
{
   //cout << " 1. Segment " << GetCHS() << " 2. Segment" << in2.GetCHS() << endl;
   if ( Equal(in2) )     return 0;
   if (GetCHS().GetLP().GetX() == x && in2.GetCHS().GetLP().GetX() == x) {
      //cout << "  two halfsegments starts at same x-coordinate " ;
      if (GetCHS().GetLP().GetY() == in2.GetCHS().GetLP().GetY() ) {
         //cout << " same left point ";
         if (GetSlope() < in2.GetSlope())  { //cout << " winkel kleiner " << endl;
	    return -1; }
         else if (GetSlope() > in2.GetSlope() ) { //cout << " winkel groesser" << endl;
	    return 1; }
         else {  //cout << "gleicher startpunkt, gleicher winkel ";
            if (GetA() < in2.GetA())  { //cout << " Achsenabschnitt kleiner " << endl;
	       return -1; }
            else if (GetA() > in2.GetA() ) { //cout << " achsenabschnitt groesser" << endl;
	       return 1; }
	    else { //cout << " eigentlich identisch ??? " << endl;
	    return 1; }
	 }
      }
      else {
         //cout << " y-Koordinate nicht gleich   ";
         double test = GetCHS().GetLP().GetY() - in2.GetCHS().GetLP().GetY();
         if ( test > 0.000001 || test < -0.000001) {
	    //cout << " test: ergebnisse nciht beieandnder " << endl;;
            if (test < 0)           return -1;
            else if ( test > 0 )    return 1;
         }
	 else {  //  |test| < 0.000001
	    //cout << " test: ergebnisse nah beienander ";
	    //cout << " Unterschiede:  y " << (GetCHS().GetLP().GetY()-in2.GetY(x) );
	    //cout << "  unterschiede berechnet " <<(GetY(x) - in2.GetY(x) );
	    //cout << "     slope: " << (GetSlope() - in2.GetSlope());
	    //cout << "    a:     " << (GetA() -in2.GetA() );
	    double  diffY1 = (GetCHS().GetLP().GetY()-in2.GetY(x) );
	    double  diffY2 = (GetY(x) - in2.GetY(x) );
	    if (diffY1 == diffY2 &&
	       GetCHS().attr.faceno == oldev.GetCHS().attr.faceno &&
	       GetCHS().attr.cycleno == oldev.GetCHS().attr.cycleno &&
	     ((GetCHS().attr.edgeno - oldev.GetCHS().attr.edgeno) == 1 ||
	      (GetCHS().attr.edgeno - oldev.GetCHS().attr.edgeno) == -1) ) {
	       if  (oldnode != 0 &&  oldnode->GetEntry().GetCHS() == in2.GetCHS()) {
	          //cout << " avor altem knoten " << endl;
	           return -1;
	       }
	       else if (oldnode != 0 && oldnode->GetPred() != 0 &&
	          oldnode->GetPred()->GetEntry().GetCHS() == in2.GetCHS() ) {
	          //cout << " aus altem Knoten 2 " << endl;
	          return 1;
	       }
	    }
            if  (diffY1 > 0 && diffY2 >0 ) { //cout << "beide größer " << endl;
               double test2 = (GetSlope() - in2.GetSlope()) * diffY1 + (GetA() -in2.GetA() );
	       //cout << " wie groß ist test???" << test2 << endl;
	       //cout << " difff1 == diff2 & beide größer  wegen SW  " << endl;
	       if (GetSlope() < in2.GetSlope()) { //cout << " hier????  winkel kleiner" << endl;
	          return -1; }
	       else { //cout << " hier ???? winkel größer " << endl;
	         return 1;}
            }
	    else if ( diffY1 < 0 && diffY2 <0) { //cout << " beide kleiner" << endl;
	       double test2 = (GetSlope() - in2.GetSlope()) * diffY1 + (GetA() -in2.GetA() );
	       //cout << " wie groß ist test???" << test2 << endl;
	       //cout << " benötiht für SW slope=0 a=Unterschied , test<0/ slope>0, a<0, test<0" << endl;
               return -1; }
	    else  if (diffY1 == diffY2) { //cout << "  hier?????  differenzen gleich" << endl;
	       Coord x0(0.1);
	       if (GetY(x+x0) < in2.GetY(x+x0) ) { //cout << " hier ????? rechts kleriner" << endl;
	          return -1; }
	       else  if (GetY(x+x0) > in2.GetY(x+x0) ) { //cout << " hier???? rechts größer" << endl;
	          return 1; }
	       else { //cout << " hier ????? gleich was tun????" << endl;
	          return 1;  }
	    }
	    else if (diffY1 < 0) { //cout << " hier??? unterschied größer" << endl;
                return -1;   }
            else if (diffY1 > 0) { //cout << " unterschied größer nötig für SW" << endl;
	       if (GetSlope() < in2.GetSlope() ) { //cout << " winkel kleiner -SW" << endl;
	          return -1; }
	       else { //cout << " hier ?????winkel größer" << endl;
                 return 1;   }
	    }
	    else { //cout << " hier ???? was soll ich nur tun " << endl;
	       return 1;}
         } // else test < 0.00001
       } // else kommt vor
   } // if starts same koordinate
   else if (GetCHS().GetLP().GetX() == x) {
      //cout << "linkes segment einfügen ";
      double test = GetCHS().GetLP().GetY() - in2.GetY(x);
      if ( test > 0.000001 || test < -0.000001) {
         if (test < 0) 	   	 return -1;
         else if ( test > 0 )	 return 1;
      }
      else {  //  |test| < 0.000001
	 //cout << " test: ergebnisse nah beienander ";
	 //cout << " Unterschiede:  y " << (GetCHS().GetLP().GetY()-in2.GetY(x) );
	 //cout << "  unterschiede berechnet " <<(GetY(x) - in2.GetY(x) );
	 //cout << "     slope: " << (GetSlope() - in2.GetSlope());
	 //cout << "    a:     " << (GetA() -in2.GetA() );
	 double  diffY1 = (GetCHS().GetLP().GetY()-in2.GetY(x) );
	 double  diffY2 = (GetY(x) - in2.GetY(x) );
	 //cout << " differenzen vergleich " << (diffY1 - diffY2) << endl;
	 if (diffY1 == diffY2 &&
	    GetCHS().attr.faceno == oldev.GetCHS().attr.faceno &&
	    GetCHS().attr.cycleno == oldev.GetCHS().attr.cycleno &&
	  ((GetCHS().attr.edgeno - oldev.GetCHS().attr.edgeno) == 1 ||
	   (GetCHS().attr.edgeno - oldev.GetCHS().attr.edgeno) == -1) ) {
	    if  (oldnode != 0 &&  oldnode->GetEntry().GetCHS() == in2.GetCHS()) {
	        //cout << " avor altem knoten " << endl;
	        return -1;
	    }
	    else if (oldnode != 0 && oldnode->GetPred() != 0 &&
	       oldnode->GetPred()->GetEntry().GetCHS() == in2.GetCHS() ) {
	       //cout << " aus altem Knoten 2 " << endl;
	       return 1;
	    }
	    else if (oldnode == 0) {
	        //cout << " wegen SWWWW  am ende einfügen, weil letzter entfernt " << endl;
	       return 1; }
	 }
         if  (diffY1 > 0 ) { //cout << " diff größer 0 " << endl;
	    double test2 = (GetSlope() - in2.GetSlope()) * diffY1 + (GetA() -in2.GetA() );
	    //cout << " wie groß ist test???" << test2 << endl;
	    if (diffY2 < 0) { //cout << " wegen SW" << endl;
	       return 1; }
	    else if (diffY2 >0)  { //cout << " wegen PA" << endl;
	       Coord x0(0.01); Coord xnext = x+x0;
	       double ytest1 = GetY(xnext);
	       double ytest2 = in2.GetY(xnext);
	       //cout << "y-Wert weiter rechts" << (ytest1-ytest2) << endl;
	       if ((ytest1-ytest2) == 0) { //cout << " wegen PA" << endl;
	          return -1; }
	       else { //cout << "wegen SW" << endl;
	          return 1;}
	       }
	    else { //cout << " hier????  gebraucht" << endl;
	       return 1; }
	 }
	 else if ( diffY1 < 0) { //cout << " diff kleiner" << endl;
	    double test2 = (GetSlope() - in2.GetSlope()) * diffY1 + (GetA() -in2.GetA() );
	    //cout << " wie groß ist test???" << test << endl;
	    if ( (GetSlope() - in2.GetSlope())< 0) { //cout << " winkel kleiner " << endl;
	       return -1; }
	    else  {  //cout << " slope größer, a kleiner test kleiner wegen EBE " << endl;
               return 1; }
	 }
	 else { //cout << " hier ?????  was soll ich nur tun " << endl;
	       return 1;}
      } // |test|  < 0.0001
   }  // else linkes segment einfügen
   else if ( GetCHS().GetRP().GetX() == x &&
      in2.GetCHS().GetRP().GetX() ==x ){
      if (GetY(x) < in2.GetY(x))      return -1;
      else if (GetY(x) > in2.GetY(x))  return 1;
      else  {
         double y1, y2;
         y1 =     GetY(x - 0.1);
         y2 = in2.GetY(x - 0.1);
         if ( y1 < y2)       return -1;
         else if (y1 > y2)  return 1;
      }
   }
   else if (GetY(x) < in2.GetY(x))    return -1;
   else if (GetY(x) > in2.GetY(x))    return 1;
   else  if (GetY(x) == in2.GetY(x) ) {
      //cout << " same y-value, becasue of rounding the Coord ????? " << endl;
      double y1 =     GetY(x + 1.0);
      double y2 = in2.GetY(x + 1.0);
      if (y1 < y2)      return -1;
      else if (y1 > y2)  return 1;
      else {
         if (GetSlope() < in2.GetSlope() )   return -1;
         else if (GetSlope() > in2.GetSlope() )   return 1;
	 else      return 1;
      }
   }
   return 0;
}

/*
class SLine -

*/

class SLine
{
public:
   SLine();
   ~SLine() { qu.Clear(); }
   BinTreeNode<SEntry>* Insert (SEntry& in, const Coord x, SEntry& oldEntry, BinTreeNode<SEntry>* &oldnode);
   bool IsEmpty();
   Coord GetX();
   void SetX(const Coord newx);
   void Delete(SEntry& in, const Coord sx);
   void Delete (BinTreeNode<SEntry>* node);
   SEntry FindAndDelete(SEntry& en, const Coord x, const Coord oldx, SEntry& oldEntry, BinTreeNode<SEntry>* &oldnode);
   void SLineOutput(const Coord x);
   int GetSize();

private:
   BinTree<SEntry> qu;
   Coord x;
   SEntry First();
   int cmp (const SEntry* in1, const SEntry* in2) const;
   bool EventEqual  (const SEntry* ev1, const SEntry* in2) const;

};

SLine::SLine()  			{BinTree<SEntry> qu;}
bool SLine::IsEmpty()			{ return (qu.IsEmpty() ); }
Coord SLine::GetX()			{ return x; }
void SLine::SetX(const Coord newx)	{ x = newx; }
int SLine::GetSize()			{return qu.GetCount(); }

BinTreeNode<SEntry>* SLine::Insert (SEntry& in, const Coord x,  SEntry& oldEntry, BinTreeNode<SEntry>* &oldnode)   {
   BinTreeNode<SEntry>* node = qu.Insert(in, x, oldEntry, oldnode);
   oldEntry.Set(in);
   oldnode = node ->GetNext();
   //cout << " in Insert " << endl;
   //cout << " entry " << in << endl;
   //cout << " oldEntry " << oldEntry << endl;
   //cout << " oldnode ";
   //if (oldnode != 0) cout << oldnode->GetEntry() << endl;
   return node;
}

void SLine::Delete (BinTreeNode<SEntry>* node)   {
   qu.DeleteNode (node);
}

SEntry SLine::FindAndDelete(SEntry& en, const Coord x, const Coord
   oldx, SEntry& oldEntry, BinTreeNode<SEntry>* &oldnode) {
   //cout << " find and delete " << endl;
   BinTreeNode<SEntry>* node = qu.Find(en,x, oldEntry, oldnode);
   //cout << " nach qu.find";
   if ( node == 0) {
      //cout << " in node == 0 " << endl;
       BinTreeNode<SEntry>* node2 = qu.Find(en, oldx, oldEntry, oldnode );
       node = node2;
       if (node == 0) {
          node2 = qu.GetFirst();
	  while (node2 != 0 && node2->GetEntry().GetCHS() != en.GetCHS()  )
	     node2 = node2->GetNext() ;
	  node = node2;
       }
   }
   if (node != 0) {
      //cout << " in node !0 " << endl;
      SEntry entry = node->GetEntry();
      oldEntry.Set( entry);
      oldnode = node ->GetNext();
      qu.DeleteNode(node);
      //cout << " oldnode: " << oldnode << "   entry ";
      //if (oldnode !=0) cout << oldnode->GetEntry() << endl;
      //else cout << " oldnode == lee" << endl;
      return entry;
   }
   //cout << " ende findanddelete " << endl;
   return 0;
}

void SLine::SLineOutput(const Coord x)
{
   cout << " in SLine output   size:" << GetSize() << "   bei x:" << x<< endl;
   BinTreeNode<SEntry>* node = qu.GetFirst();
   // BinTreeNode<XEvent>* ev = First();
   if ( !qu.IsEmpty() ) {
      while (node != 0 )  {
         SEntry ev = node->GetEntry();
         cout <<" y:" << ev.GetY(x)  << "  y rechts " << ev.GetY(x+0.01) << "  SEntry: "  << ev.GetCHS() << "  U:" << ev.GetU() << "   o:" << ev.GetO() << "  slope:" << ev.GetSlope()<< " " << " "<< ev.GetA()<< endl;
         node = node->GetNext();
      }
   }
   cout << " in output fertig " << endl;
}


enum State {FIRST, SECOND, BOTH};
class MakeOp
{
public:
   MakeOp() {};
   ~MakeOp() {};
   CRegion* Intersection(CRegion* reg1, CRegion* reg2);
   CLine* Intersection(CRegion* reg, CLine* line);
   CLine* Intersection(CLine* reg1, CLine* reg2);
   CRegion* Union(CRegion* reg1, CRegion* reg2);
   CLine* Union(CLine* line1, CLine* line2);
   CRegion* Minus(CRegion* reg1, CRegion* reg2);
   CLine* Minus(CLine* line, CRegion* reg);
   CLine* Minus(CLine* line1, CLine* line2);
   bool Intersects(CRegion* reg1, CRegion* reg2);
   bool Intersects(CLine* line1, CLine* line2);
   bool Intersects (CRegion* reg, CLine* line);
   bool P_Intersects(CRegion* reg1, CRegion* reg2);
   bool P_Intersects(CRegion* reg, CLine* line);
   bool P_Intersects(CLine* line1, CLine* line2);
};

CRegion* MakeOp::Intersection(CRegion* reg1, CRegion* reg2)
{
  // first Realmisation of both regions
   CRegion* res1  = new CRegion(0);
   CRegion* res2 = new CRegion(0);
   MakeRealm mr;
   mr.REALM( reg1, reg2, res1 , res2 );
   SLine sweepline;
   int i = 0;   int j = 0;    int counter = 0;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   CHalfSegment chs1, chs2, chsAkt;
   CRegion* result = new CRegion(0);
   result ->Clear();
   result->StartBulkLoad();
   State status;
   Coord aktSweep;
   Coord oldSweep;
   //sweepline.SLineOutput(aktSweep);
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ){
         i ++;   j ++;   chsAkt = chs1;    status = BOTH;
      }
      else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST; }
      else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
      //cout << " status: " << status << endl;
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 if (status == FIRST)  {
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
	    //cout << " oldnode1 ";
	    if (en.GetO() == 2 || en.GetU() == 2) {  // CHalfSegment in result
	       chsAkt.attr.partnerno = counter;
               chsAkt.SetLDP(false);	(*result) += chsAkt;
	       chsAkt.SetLDP(true);	(*result) += chsAkt;
	      counter++;
	   }
	 }
	 else {
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry2, oldnode2);
	    if (en.GetO() == 2 || en.GetU() == 2) {  // CHalfSegment in result
	       chsAkt.attr.partnerno = counter;
               chsAkt.SetLDP(false);	(*result) += chsAkt;
	       chsAkt.SetLDP(true);	(*result) += chsAkt;
	       counter++;
	    }
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en;
	 if (status == FIRST)  en = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 else      en = sweepline.Insert(ent,aktSweep, oldEntry2, oldnode2);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 int np = 0;
	 int ms, ns;
	 if (pred != 0) np = pred->GetEntry().GetO();
	 // calculate new segmentclass of new ChalfSegment
	 ms = np;
	 ns = np;
	 if (status == FIRST || status == BOTH) {
	    if (chs1.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
	 if (status == SECOND || status == BOTH) {
	    if (chs2.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
	 // set new segmentclasses in SEntry
	 if (ms < 0 || ns<0) cout << "Probleme ???????????" << endl;
	 ent.SetU(ms);
	 ent.SetO(ns);
	 en->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
      //sweepline.SLineOutput(aktSweep);
   } // end while
   result->EndBulkLoad();
   result->SetPartnerNo();
   result->ComputeRegion();
   return result;
}

 CLine* MakeOp::Intersection(CRegion* reg, CLine* line)
 {
  // first Realmisation of both regions
   CLine* resline  = new CLine(0);
   CRegion* resregion = new CRegion(0);
   MakeRealm mr;
   mr.REALM( line, reg, resline , resregion );
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   CLine* result = new CLine(0);
   result ->Clear();
   result->StartBulkLoad();
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1 ;
   BinTreeNode<SEntry>* oldnode1;
   //sweepline.SLineOutput(aktSweep);
   while ( i < resline->Size() && j < resregion->Size() ) {
     // select_ first
      resline->Get(i,chs1);
      resregion->Get(j,chs2);
      if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ){
         i ++;   j ++;   chsAkt = chs2;    status = BOTH;
      }
      else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST; }
      else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         if (status == SECOND || status == BOTH) {
            chsAkt.SetLDP(true);
            SEntry ent (chsAkt);
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 int np = 0;
	 int ms, ns;
	 if (pred != 0) np = pred->GetEntry().GetO();
	 // calculate new segmentclass of new ChalfSegment
	 ms = np;
	 ns = np;
	 if (status == SECOND || status == BOTH) {
	    if (chs2.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	    ent.SetU(ms);
	    ent.SetO(ns);
	    en->SetEntry(ent);
	 }
	 if (status == FIRST || status == BOTH) {
	    if (pred != 0 ) { if (pred->GetEntry().GetO() > 0) (*result) += chsAkt; }
	    if (status == FIRST) sweepline.Delete(en);
	 }
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   result->EndBulkLoad();
   return result;
}


CLine* MakeOp::Intersection(CLine* line1, CLine* line2)
{
  // first Realmisation of both regions
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   CLine* result = new CLine(0);
   result ->Clear();
   result->StartBulkLoad();
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() )
	 {  i ++;   j ++;   (*result) += chs1; }
      else if ( chs1 < chs2)  i ++;
      else if (chs1 > chs2)   j ++;
   } // end while
   result->EndBulkLoad();
   return result;
}

bool MakeOp::P_Intersects(CRegion* reg1, CRegion* reg2)
{
  // first Realmisation of both regions
   CRegion* res1  = new CRegion(0);
   CRegion* res2 = new CRegion(0);
   MakeRealm mr;
   mr.REALM( reg1, reg2, res1 , res2 );
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ){
         i ++;   j ++;   chsAkt = chs1;    status = BOTH;
      }
      else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST; }
      else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 int np = 0;
	 int ms, ns;
	 if (pred != 0) np = pred->GetEntry().GetO();
	 // calculate new segmentclass of new ChalfSegment
	 ms = np;
	 ns = np;
	 if (status == FIRST || status == BOTH) {
	    if (chs1.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
	 if (status == SECOND || status == BOTH) {
	    if (chs2.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
         if (ms == 2 || ns == 2) { return true; }
	 // set new segmentclasses in SEntry
	 ent.SetU(ms);
	 ent.SetO(ns);
	 en->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   return false;
}

bool MakeOp::P_Intersects(CRegion* reg, CLine* line)
{
  // first Realmisation of both regions
   CLine* resline  = new CLine(0);
   CRegion* resregion = new CRegion(0);
   MakeRealm mr;
   mr.REALM( line, reg, resline , resregion );
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   //sweepline.SLineOutput(aktSweep);
   while ( i < resline->Size() && j < resregion->Size() ) {
     // select_ first
      resline->Get(i,chs1);
      resregion->Get(j,chs2);
      if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ){
         i ++;   j ++;   chsAkt = chs2;    status = BOTH;
      }
      else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST; }
      else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         if (status == SECOND || status == BOTH) {
            chsAkt.SetLDP(true);
            SEntry ent (chsAkt);
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 int np = 0;
	 int ms, ns;
	 if (pred != 0) np = pred->GetEntry().GetO();
	 // calculate new segmentclass of new ChalfSegment
	 ms = np;
	 ns = np;
	 if (status == SECOND || status == BOTH) {
	    if (chs2.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	    ent.SetU(ms);
	    ent.SetO(ns);
	    en->SetEntry(ent);
	 }
	 if (status == FIRST || status == BOTH) {
	    if ( pred != 0  && pred->GetEntry().GetO() > 0 ) { return true; }
	    if (status == FIRST) sweepline.Delete(en);
	 }
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   return false;
}

bool MakeOp::P_Intersects(CLine* line1, CLine* line2)
{
     // first Realmisation of both regions
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ) { return true; }
      else if ( chs1 < chs2)  i ++;
      else if (chs1 > chs2)   j ++;
   } // end while
   return false;
}


bool MakeOp::Intersects(CRegion* reg1, CRegion* reg2)
{
  // first Realmisation of both regions
   CRegion* res1  = new CRegion(0);
   CRegion* res2 = new CRegion(0);
   MakeRealm mr;
   mr.REALM( reg1, reg2, res1 , res2 );
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   State status;
   Coord aktSweep;
   Coord oldSweep;
    SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP() == chs2.GetLP() || chs1.GetLP() == chs2.GetRP() ||
          chs1.GetRP() == chs2.GetLP() || chs1.GetRP() == chs2.GetRP() ) return true;
      else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST; }
      else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 int np = 0;
	 int ms, ns;
	 if (pred != 0) np = pred->GetEntry().GetO();
	 // calculate new segmentclass of new ChalfSegment
	 ms = np;
	 ns = np;
	 if (chsAkt.attr.insideAbove == true)	ns = ns+1;
	 else 					ns = ns-1;
	 // set segmentclasses in SEntry
	 if (ms == 2 || ns == 2) return true;
	 ent.SetU(ms);
	 ent.SetO(ns);
	 en->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   return false;
}

bool MakeOp::Intersects (CRegion* reg, CLine* line) {
  // first Realmisation of both regions
   CRegion* res1  = new CRegion(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line, reg, res2 , res1 );
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP() == chs2.GetLP() || chs1.GetLP() == chs2.GetRP() ||
          chs1.GetRP() == chs2.GetLP() || chs1.GetRP() == chs2.GetRP() ) return true;
      else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST; }
      else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      aktSweep = chsAkt.GetDPoint().GetX();
      if (status == FIRST && chsAkt.GetLDP() == false) {  // right end of segment of region
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 if (status == FIRST ) { // insert CHalfSegment from region into sweepline
	    int np = 0;
	    int ms, ns;
	    if (pred != 0) np = pred->GetEntry().GetO();
	    // calculate new segmentclass of new ChalfSegment
	    ms = np;
	    ns = np;
	    if (chs1.attr.insideAbove == true)	{ ent.SetU(0); ent.SetO(1); }
	    else 				{ ent.SetU(1); ent.SetO(0); }
	    en->SetEntry(ent);
         } //  end else
	 if (status == SECOND) {
	    if (pred != 0)
	       { if (pred->GetEntry().GetO() == 1) return true; }
	    else sweepline.Delete (en);
	 }
      }
      oldSweep = aktSweep ;
   } // end while
   return false;
}

bool MakeOp::Intersects (CLine* line1, CLine* line2)
{
  // first Realmisation of both regions
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP() == chs2.GetLP() || chs1.GetLP() == chs2.GetRP() ||
          chs1.GetRP() == chs2.GetLP() || chs1.GetRP() == chs2.GetRP() ) return true;
      if ( chs1 < chs2)       i ++;
      else if (chs1 > chs2)   j ++;

   } // end while
   return false;
}


CRegion* MakeOp::Union(CRegion* reg1, CRegion* reg2)
{
   // first Realmisation of both regions
   CRegion* res1 = new CRegion(0);
   CRegion* res2 = new CRegion(0);
   MakeRealm mr;
   mr.REALM( reg1, reg2, res1 , res2 );
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   CRegion* result = new CRegion(0);
   result ->Clear();
   result->StartBulkLoad();
   int counter = 0;
   State status;
   Coord aktSweep, oldSweep ;
    SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   while ( i < res1->Size() || j < res2->Size()) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,chs1);
         res2->Get(j,chs2);
         if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ){
            i ++;   j ++;   chsAkt = chs1;    status = BOTH; }
	 else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST; }
         else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
      }
      else if (i < res1->Size() ) {
         res1->Get(i,chs1);  i ++;   chsAkt = chs1;  status = FIRST;
      }
      else if ( j < res2->Size() ) {
         res2->Get(j,chs2);  j ++;   chsAkt = chs2;  status = SECOND; }
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      //cout << " aktCHS " << chsAkt << endl;
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
	 if (en.GetO() == 0 || en.GetU() == 0) {  // CHalfSegment in result
	    chsAkt.attr.partnerno = counter;
            chsAkt.SetLDP(false);	(*result) += chsAkt;
	    chsAkt.SetLDP(true);	(*result) += chsAkt;
	    counter++;
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 int np = 0;
	 int ms, ns;
	 if (pred != 0) np = pred->GetEntry().GetO();
	 // calculate new segmentclass of new ChalfSegment
	 ms = np;
	 ns = np;
	 if (status == FIRST || status == BOTH) {
	    if (chs1.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
	 if (status == SECOND || status == BOTH) {
	    if (chs2.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
	 // set new segmentclasses in SEntry
	 ent.SetU(ms);
	 ent.SetO(ns);
	 en->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
      //sweepline.SLineOutput(aktSweep);
   } // end while

   result->EndBulkLoad();
   result->SetPartnerNo();
   for (int i=0; i < result->Size(); i++) {
      CHalfSegment chx ;
      result -> Get(i, chx);
   }
   result->ComputeRegion();
   return result;
}


CLine* MakeOp::Union(CLine* line1, CLine* line2)
{
  // first Realmisation of both regions
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   CLine* result = new CLine(0);
   result ->Clear();
   result->StartBulkLoad();
   while ( i < res1->Size() || j < res2->Size()) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,chs1);
         res2->Get(j,chs2);
         if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() )
	    {  i ++;   j ++;   (*result) += chs1; }
	 else if ( chs1 < chs2) { i ++;  (*result) += chs1; }
         else if (chs1 > chs2)  { j ++;  (*result) += chs2; }
      }
      else if (i < res1->Size() )  { res1->Get(i,chs1);  i ++;  (*result) += chs1; }
      else if ( j < res2->Size() ) { res2->Get(j,chs2);  j ++;  (*result) += chs2; }
   } // end while
   result->EndBulkLoad();
   return result;
}



CRegion* MakeOp::Minus(CRegion* reg1, CRegion* reg2)
{
  // first Realmisation of both regions
   CRegion* res1 = new CRegion(0);
   CRegion* res2 = new CRegion(0);
   MakeRealm mr;
   mr.REALM( reg1, reg2, res1 , res2 );
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   CRegion* result = new CRegion(0);
   result ->Clear();
   result->StartBulkLoad();
   int counter = 0;
   State status;
   Coord aktSweep, oldSweep ;
    SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   while ( i < res1->Size() ) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,chs1);
         res2->Get(j,chs2);
         if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ){
          		  i ++;   j ++;   chsAkt = chs1;    status = BOTH;    }
         else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST;  }
         else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
      }
      else if (i < res1->Size() ) {
         res1->Get(i,chs1);  i ++;   chsAkt = chs1;  status = FIRST;
      }
      else if ( j < res2->Size() ) {
         res2->Get(j,chs2);  j ++;   chsAkt = chs2;  status = SECOND; }
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
	 // CHalfSegment in result ?
	 if ( (status == FIRST && en.GetU() == 0 && en.GetO() == 1) ||
	 (status == FIRST && en.GetU() == 1 && en.GetO() == 0) ||
	 (status == BOTH && en.GetU() == 1 && en.GetO() == 1)) {
	    chsAkt.attr.partnerno = counter;
            chsAkt.SetLDP(false);	(*result) += chsAkt;
	    chsAkt.SetLDP(true);	(*result) += chsAkt;
	    counter++;
	 }
	 else if ( (status == SECOND && en.GetU() == 1 && en.GetO() == 2) ||
	 (status == SECOND && en.GetU() == 2 && en.GetO() == 1) ) {
	    chsAkt.attr.insideAbove = ! chsAkt.attr.insideAbove ;
	    chsAkt.attr.partnerno = counter;
            chsAkt.SetLDP(false);	(*result) += chsAkt;
	    chsAkt.SetLDP(true);	(*result) += chsAkt;
	    counter++;
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 int np = 0;
	 int ms, ns;
	 if (pred != 0) np = pred->GetEntry().GetO();
	 // calculate new segmentclass of new ChalfSegment
	 ms = np; 	 ns = np;
	 if (status == FIRST || status == BOTH) {
	    if (chs1.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
	 if (status == SECOND || status == BOTH) {
	    if (chs2.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
	 // set new segmentclasses in SEntry
	 ent.SetU(ms);
	 ent.SetO(ns);
	 en->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
      //sweepline.SLineOutput(aktSweep);
   } // end while

   result->EndBulkLoad();
   result->SetPartnerNo();
   /*cout  <<" ergebnmis vor compute cycle" << endl;
   for (int i=0; i < result->Size(); i++) {
      CHalfSegment chx ;
      result -> Get(i, chx);
      if (chx.GetLDP()) cout << " CHS: " << chx << endl;
   } */
   result->ComputeRegion();
   return result;
}

CLine* MakeOp::Minus(CLine* line, CRegion* reg)
{
     // first Realmisation of both regions
   CLine* resLine = new CLine(0);
   CRegion* resReg = new CRegion(0);
   MakeRealm mr;
   mr.REALM( line, reg, resLine , resReg );
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   CLine* result = new CLine(0);
   result ->Clear();
   result->StartBulkLoad();
   State status;
   Coord aktSweep, oldSweep ;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   while ( i < resLine -> Size() ) {
     // select_ first
      if (i < resLine -> Size() && j < resReg -> Size() ) {
         resLine -> Get(i,chs1);
         resReg -> Get(j,chs2);
         if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ){
            i ++;   j ++;   chsAkt = chs2;    status = BOTH;  }
         else if ( chs1 < chs2) { i ++;   chsAkt = chs1;   status = FIRST;  }
         else if (chs1 > chs2) { j ++;   chsAkt = chs2;   status = SECOND; }
      }
      else if (i < resLine ->Size() ) {
         resLine -> Get(i,chs1);  i ++;   chsAkt = chs1; status = FIRST;
      }
       //  CHalfSegment chsAkt = selectFirst(chs1, chs2, status, i, j)
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // delete right end of region-segment
	 if ( status == SECOND || status == BOTH ) {
            chsAkt.SetLDP(true);
            SEntry ent (chsAkt);
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep, oldEntry1, oldnode1);
	 }
      }
      else { // chsAkt.GetLDP() == true
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep,oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 if (status == FIRST ) {
	    if (pred == 0  || (pred != 0 && pred -> GetEntry().GetO() == 0) )   {
	       (*result) += chs1;
	       chs1.SetLDP(false);
	       (*result) += chs1;
	    }
            sweepline.Delete (en) ;
	 }
	 else {  // status == SECOND or BOTH
	    if (chsAkt.attr.insideAbove == true)  { ent.SetU(0); ent.SetO(1); }
	    else 				  { ent.SetU(1); ent.SetO(0); }
	    en->SetEntry(ent);
	 }
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   result->EndBulkLoad();
   return result;

}

CLine* MakeOp::Minus(CLine* line1, CLine* line2)
{
  // first Realmisation of both regions
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   CLine* result = new CLine(0);
   result ->Clear();
   result->StartBulkLoad();
   while ( i < res1->Size() ) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,chs1);
         res2->Get(j,chs2);
         if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() ){ i ++;  j ++; }
         else if ( chs1 < chs2) { i ++;  (*result) += chs1;  }
         else if (chs1 > chs2)  j ++;
      }
      else if (i < res1->Size() ) {
         res1->Get(i,chs1);
	 i ++;
	 (*result) += chs1;
      }
   }
   result -> EndBulkLoad() ;
   return result;
}


enum NewSpatialType { stpoint, stpoints, stline, stregion, stbox, sterror };

NewSpatialType
NewSpatialTypeOfSymbol( ListExpr symbol )
{
  if ( nl->AtomType( symbol ) == SymbolType )
  {
    string s = nl->SymbolValue( symbol );
    if ( s == "point"  ) return (stpoint);
    if ( s == "points" ) return (stpoints);
    if ( s == "line"   ) return (stline);
    if ( s == "region" ) return (stregion);
    if ( s == "rect"   ) return (stbox);
  }
  return (sterror);
}

Word
NewSpatialNoModelMapping( ArgVector arg, Supplier opTreeNode )
{
  return (SetWord( Address( 0 ) ));
}

ModelMapping newSpatialnomodelmap[] = { NewSpatialNoModelMapping,
               NewSpatialNoModelMapping,
               NewSpatialNoModelMapping,
               NewSpatialNoModelMapping,
               NewSpatialNoModelMapping,
               NewSpatialNoModelMapping,
               NewSpatialNoModelMapping };


int NewSimpleSelect( ListExpr args )  {  return (0); }

/*
Realm-Test-Operator

*/

static ListExpr realmMap( ListExpr args )
{
   ListExpr arg1, arg2;
   if ( nl->ListLength( args ) == 2 )
   {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "line" ));
      else if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "line" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "region" ));

      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "region" ));
     /*
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "line" ));
   */
    }
    return (nl->SymbolAtom( "typeerror" ));
}

const string RealmSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text> (line region) x (line region) -> line or region </text--->"
        "<text>REALM(_,_)</text--->"
        "<text>Returns the first object as a REALMed object.</text--->"
        "<text>query REALM ( line1, region2 )</text--->"
        ") )";


// operator  2
static int
realm_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined())
   {
      CLine* test1 = new CLine(0);
      CRegion* test2 = new CRegion(0);
      MakeRealm mr;
      mr.REALM( l1, r2, test1 , test2 );
      ((CLine *)result.addr) = test1->Clone();
      return(0);
   }
   else
   {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

// operator 3
static int
realm_rl( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[1].addr);
   CRegion *r2 = ((CRegion*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined())
   {
      CLine* test1 = new CLine(0);
      CRegion* test2 = new CRegion(0);
      MakeRealm mr;
      mr.REALM( l1, r2, test1 , test2 );
      ((CRegion *)result.addr) = test2->Clone();
      return(0);
   }
   else
   {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}


// operator 1
static int
realm_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[0].addr);
   CLine *l2 = ((CLine*)args[1].addr);
   if (l1->IsDefined() && l2->IsDefined())
   {
      CLine* test1 = new CLine(0);
      CLine* test2 = new CLine(0);
      MakeRealm mr;
      mr.REALM( l1, l2, test1 , test2 );
      ((CLine *)result.addr) = test1->Clone();
      return(0);
   }
   else
   {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}


// operator 4 REALM  regions and result is first region as a REALM

static int
realm_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined())
   {
      CRegion* test1 = new CRegion(0);
      CRegion* test2 = new CRegion(0);
      MakeRealm mr;
      mr.REALM( r1, r2, test1 , test2 );
      ((CRegion *)result.addr) = test1->Clone();
      return(0);
   }
   else
   {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}


/*
// operator 4 REALM  regions and result is first region as a REALM

static int
realm_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined())
   {
      CLine* test1 = new CLine(0);
      CLine* test2 = new CLine(0);
      MakeRealm mr;
      mr.REALM( r1, r2, test1 , test2 );
      ((CLine *)result.addr) = test1->Clone();
      return(0);
   }
   else
   {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}
*/

ValueMapping RealmMap[] = { realm_ll, realm_lr, realm_rl, realm_rr };

int realmSelectCompute ( ListExpr args) {
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )     return (0);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )    return (1);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )       return (2);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )      return (3);
   else return (-1);

}

Operator realm
        ( "realm", RealmSpec, 4, RealmMap, newSpatialnomodelmap,
          realmSelectCompute, realmMap);


// operator intersection line,region x line,region
static ListExpr InterMap( ListExpr args )
{
   ListExpr arg1, arg2;
   if ( nl->ListLength( args ) == 2 )
   {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "line" ));
      else if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "line" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "line" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "region" ));

    }
    return (nl->SymbolAtom( "typeerror" ));
}


static int
Inter_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() ) {
      if (line1->IsEmpty() || line2->IsEmpty() ) {
          ((CLine *)result.addr)->SetDefined( false );
         // CLine* res = new CLine(0);
	 //((CLine*) result.addr) = res -> Clone() ;
         return (0);
      }
      else if (line1->BoundingBox().IsDefined() && line2->BoundingBox().IsDefined() ) {
         if ( line1->BoundingBox().Intersects( line2->BoundingBox() ) )  {
            CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Intersection( line1, line2 );
	    if ( res->IsEmpty() )  ((CLine *)result.addr)->SetDefined( false );
            else                   ((CLine *)result.addr) = res ->Clone();
            return(0);
         }
         else   {
	    ((CLine *)result.addr)->SetDefined( false );
            // CLine* res = new CLine(0);
            // ((CLine *)result.addr) = res ->Clone() ;
            return (0);
         }
      }
      else  {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Intersection( line1, line2 );
	 if ( res->IsEmpty() )  ((CLine *)result.addr)->SetDefined( false );
         else                   ((CLine *)result.addr) = res ->Clone();
         return(0);
      }
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

static int Inter_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
      if (l1->IsEmpty() || r2->IsEmpty() ) {
         ((CLine *)result.addr)->SetDefined( false );
          //CLine* res = new CLine(0);
	 //((CLine*) result.addr) = res -> Clone();
         return (0);
      }
      if (l1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Intersection( r2, l1 );
	    if (res->IsEmpty() ) ((CLine *)result.addr)->SetDefined( false );
            else                  ((CLine *)result.addr) = res ->Clone();
            return(0);
         }
         else   {
	    ((CLine *)result.addr)->SetDefined( false );
            // CLine* res = new CLine(0);
            // ((CLine *)result.addr) = res ->Clone() ;
            return (0);
         }
      }
      else  {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Intersection( r2, l1  );
	 if (res->IsEmpty() ) ((CLine *)result.addr)->SetDefined( false );
         else                 ((CLine *)result.addr) = res ->Clone();
         return(0);
      }
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

static int Inter_rl ( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[1].addr);
   CRegion *r2 = ((CRegion*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
      if (l1->IsEmpty() || r2->IsEmpty() ) {
         ((CLine *)result.addr)->SetDefined( false );
          //CLine* res = new CLine(0);
	 //((CLine*) result.addr) = res -> Clone();
         return (0);
      }
      if (l1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Intersection( r2, l1 );
	    if (res->IsEmpty() ) ((CLine *)result.addr)->SetDefined( false );
            else                  ((CLine *)result.addr) = res ->Clone();
            return(0);
         }
         else   {
	    ((CLine *)result.addr)->SetDefined( false );
            // CLine* res = new CLine(0);
            // ((CLine *)result.addr) = res ->Clone() ;
            return (0);
         }
      }
      else  {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Intersection( r2, l1  );
	 if (res->IsEmpty() ) ((CLine *)result.addr)->SetDefined( false );
         else                 ((CLine *)result.addr) = res ->Clone();
         return(0);
      }
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}




static int Inter_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
      if (r1->IsEmpty() || r2->IsEmpty() ) { // no intersection possible
         ((CRegion *)result.addr)->SetDefined( false );
          return (0);
      }
      if (r1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( r1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            CRegion* res = new CRegion(0);
            MakeOp mo;
            res = mo.Intersection( r1, r2 );
	    if ( res->IsEmpty() )
	       ((CRegion *)result.addr)->SetDefined( false );
            else
	       ((CRegion *)result.addr) = res ->Clone();
            return(0);
         }
         else   { // no intersection possible
	    ((CRegion *)result.addr)->SetDefined( false );
            return (0);
         }
      }
      else  {
         CRegion* res = new CRegion(0);
         MakeOp mo;
         res = mo.Intersection( r1, r2 );
	 if ( res->IsEmpty() )
	    ((CRegion *)result.addr)->SetDefined( false );
         else
            ((CRegion *)result.addr) = res ->Clone();
         return(0);
      }
   }
   else  {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}



const string InterSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text> (line region) x (line region) -> (line region) </text--->"
        "<text>intersection_new(_,_)</text--->"
        "<text>Returns the intersecion of 2 regions.</text--->"
        "<text>query intersection_new ( region1,region2 )</text--->"
        ") )";

ValueMapping InterValueMap[] = { Inter_ll, Inter_lr, Inter_rl, Inter_rr };

int InterSelectCompute ( ListExpr args) {
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )     return (0);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )    return (1);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )       return (2);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )      return (3);
   else return (-1);

}

Operator Intersectionrr
        ( "intersection_new", InterSpec, 4, InterValueMap, newSpatialnomodelmap,
          InterSelectCompute, InterMap );


// operator intersects  line,region x line,region
static ListExpr IntersectsMap( ListExpr args )
{
   ListExpr arg1, arg2;
   if ( nl->ListLength( args ) == 2 )
   {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "bool" ));
      else if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "bool" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "bool" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "bool" ));

    }
    return (nl->SymbolAtom( "typeerror" ));
}


static int Intersects_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{

   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() ) {
       if (line1->IsEmpty() || line2->IsEmpty() ) {
         ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (line1->BoundingBox().IsDefined() && line2->BoundingBox().IsDefined() ) {
         if ( line1->BoundingBox().Intersects( line2->BoundingBox() ) )  {
            MakeOp mo;
            bool res = mo.Intersects( line1, line2 );
            ((CcBool *)result.addr) ->Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr) ->Set(true,false) ;
            return (0);
         }
      }
      else  {
         MakeOp mo;
         bool res = mo.Intersects( line1, line2 );
         ((CcBool *)result.addr) ->Set(true,res);
         return(0);
      }
   }
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}

static int Intersects_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
       if (l1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set( true, false);
         return (0);
      }
      if (l1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            MakeOp mo;
            bool res = mo.Intersects ( r2, l1 );
            ((CcBool *)result.addr)-> Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr) -> Set(true,false) ;
            return (0);
         }
      }
      else  {
         MakeOp mo;
         bool res = mo.Intersects( r2, l1 );
         ((CcBool *)result.addr) -> Set(true,res);
         return(0);
      }
   }
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}

static int Intersects_rl( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[1].addr);
   CRegion *r2 = ((CRegion*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
       if (l1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (l1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            MakeOp mo;
            bool res = mo.Intersects ( r2, l1 );
            ((CcBool *)result.addr)-> Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr) -> Set(true,false) ;
            return (0);
         }
      }
      else  {
         MakeOp mo;
         bool res = mo.Intersects( r2, l1 );
         ((CcBool *)result.addr) -> Set(true,res);
         return(0);
      }
   }
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}

static int Intersects_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
      if (r1->IsEmpty() || r2->IsEmpty() ) {
         ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (r1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( r1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            MakeOp mo;
            bool res = mo.Intersects ( r1, r2 );
            ((CcBool *)result.addr)-> Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr) -> Set(true,false) ;
            return (0);
         }
      }
      else  {
         MakeOp mo;
         bool res = mo.Intersects( r1, r2 );
         ((CcBool *)result.addr) -> Set(true,res);
         return(0);
      }
   }
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}

const string IntersectsSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text> (line region) x (line region) -> (line region) </text--->"
        "<text>intersects_new(_,_)</text--->"
        "<text>tests if two lines/regions intersects.</text--->"
        "<text>query intersects_new ( region1,region2 )</text--->"
        ") )";

ValueMapping IntersectsValueMap[] = { Intersects_ll, Intersects_lr, Intersects_rl,
 				      Intersects_rr };

int IntersectsSelectCompute ( ListExpr args) {
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )     return (0);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )    return (1);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )       return (2);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )      return (3);
   else return (-1);

}

Operator Intersectsrr
        ( "intersects_new", IntersectsSpec, 4, IntersectsValueMap, newSpatialnomodelmap,
          IntersectsSelectCompute, IntersectsMap );


// operator 6 Neues Union region - region
static ListExpr unionMap( ListExpr args )
{
   ListExpr arg1, arg2;
   if ( nl->ListLength( args ) == 2 )
   {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "line" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "region" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "region" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "region" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}

static int Union_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() )    {
      if ( r1->IsEmpty() && r2 -> IsEmpty() ) {
         ((CRegion *)result.addr)->SetDefined( false );
         return (0);
      }
      else if (r1->IsEmpty() ) {
         ((CRegion *)result.addr) = r2 ->Clone();
         return(0);
      }
      else if (r2->IsEmpty() ) {
         ((CRegion *)result.addr) = r1 ->Clone();
         return(0);
      }
      else {
         CRegion* res = new CRegion(0);
         MakeOp mo;
         res = mo.Union( r1, r2 );
         ((CRegion *)result.addr) = res ->Clone();
         return(0);
      }
   }
   else  {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}

static int Union_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{

   result = qp->ResultStorage( s );
   CLine *line = ((CLine*)args[0].addr);
   CRegion *reg = ((CRegion*)args[1].addr);
   if ( !reg->IsEmpty() && line->IsDefined() && reg->IsDefined() )    {
      ((CRegion *)result.addr) = reg ->Clone();
      return(0);
   }
   else  {
      ((CRegion *)result.addr)->SetDefined( false );
      return (0);
   }
}

static int Union_rl( Word* args, Word& result, int message, Word& local, Supplier s )
{

   result = qp->ResultStorage( s );
   CLine *line = ((CLine*)args[1].addr);
   CRegion *reg = ((CRegion*)args[0].addr);
   if ( ! reg -> IsEmpty() && line->IsDefined() && reg->IsDefined() )    {
      ((CRegion *)result.addr) = reg ->Clone();
      return(0);
   }
   else  {
      ((CRegion *)result.addr)->SetDefined( false );
      return (0);
   }
}

static int Union_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() )    {
      if (line1->IsEmpty() ) {
         ((CLine *)result.addr)->SetDefined( false );
         // ((CLine *)result.addr) = line2 ->Clone();
         return(0);
      }
      else if (line2->IsEmpty() ) {
         ((CLine *)result.addr) = line1 ->Clone();
         return(0);
      }
      else {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Union( line1, line2 );
         ((CLine *)result.addr) = res ->Clone();
         return(0);
      }
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

const string UnionSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text> (line region) x (line region) -> (line region) </text--->"
        "<text>union_new(_)</text--->"
        "<text>Returns the union of 2 lines/regions .</text--->"
        "<text>query union_new ( region1,region2 )</text--->"
        ") )";

ValueMapping UnionValueMap[] = { Union_ll, Union_lr, Union_rl, Union_rr };

int unionSelectCompute ( ListExpr args) {
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )     return (0);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )    return (1);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )       return (2);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )      return (3);
   else return (-1);

}

Operator Unionrr
        ( "union_new", UnionSpec, 4, UnionValueMap, newSpatialnomodelmap,
          unionSelectCompute, unionMap );


// operator 7 Neues Union region - region

static ListExpr minusMap( ListExpr args )
{
   ListExpr arg1, arg2;
   if ( nl->ListLength( args ) == 2 )
   {
      arg1 = nl->First( args );
      arg2 = nl->Second( args );
      if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "line" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "line" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( "region" ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( "region" ));
    }
    return (nl->SymbolAtom( "typeerror" ));
}


static int Minus_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
      if ( r1 -> IsEmpty() ) {
	 ((CRegion *)result.addr)->SetDefined( false );
         return(0);
      }
      else if ( r2->IsEmpty() ) {
         ((CRegion *)result.addr) = r1 ->Clone();
         return(0);
      }
      else if (r1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if (( r1->BoundingBox().Intersects( r2->BoundingBox() ) ) )  {
            CRegion* res = new CRegion(0);
            MakeOp mo;
            res = mo.Minus( r1, r2 );
	    if (res->IsEmpty() )   ((CRegion *)result.addr)->SetDefined( false );
            else                   ((CRegion *)result.addr) = res ->Clone();
            return(0);
         }
         else   {
            ((CRegion *)result.addr) = r1; // res ->Clone() ;
            return (0);
         }
      }
      else    {
         CRegion* res = new CRegion(0);
         MakeOp mo;
         res = mo.Minus( r1, r2 );
	 if (res->IsEmpty() )   ((CRegion *)result.addr)->SetDefined( false );
         else                   ((CRegion *)result.addr) = res ->Clone();
         // ((CRegion *)result.addr) = res ->Clone();
         return(0);
      }
   }
   else
   {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}

static int Minus_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line = ((CLine*)args[0].addr);
   CRegion *reg = ((CRegion*)args[1].addr);
   if (line->IsDefined() && reg->IsDefined() )    {
      if ( line-> IsEmpty()|| reg->IsEmpty() ) {
         ((CLine *)result.addr)->SetDefined( false );
         //((CLine *)result.addr) = line -> Clone();
         return(0);
      }
      else if (line->BoundingBox().IsDefined() && reg->BoundingBox().IsDefined() ) {
         if ( line->BoundingBox().Intersects( reg->BoundingBox() ) )  {
            CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Minus( line,reg );
	    if ( res -> IsEmpty() ) ((CLine *)result.addr)->SetDefined( false );
            else                    ((CLine *)result.addr) = res ->Clone();
            return(0);
         }
         else   {
            ((CLine *)result.addr) = line->Clone();
            return (0);
         }
      }
      else    {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Minus( line, reg );
         if ( res -> IsEmpty() ) ((CLine *)result.addr)->SetDefined( false );
         else                    ((CLine *)result.addr) = res ->Clone();
         // ((CLine *)result.addr) = res ->Clone();
         return(0);
      }
   } // if IsDefined()
   else   {
      ((CLine *)result.addr)->SetDefined( false );
      return (0);
   }
}

static int Minus_rl( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line = ((CLine*)args[1].addr);
   CRegion *reg = ((CRegion*)args[0].addr);
   if ( ! line->IsEmpty() && line->IsDefined() && reg->IsDefined() )    {
      ((CRegion *)result.addr) = reg ->Clone();
      return(0);
   }
   else  {
      ((CRegion *)result.addr)->SetDefined( false );
      return (0);
   }
}

static int Minus_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() )    {
      if (line1->IsEmpty() ) {
         ((CLine *) result.addr) -> SetDefined(false);
	 return (0);
      }
      else if (line2->IsEmpty() ) {
         ((CLine *)result.addr) = line1 -> Clone();
         return (0);
      }
      else if (line1->BoundingBox().IsDefined() && line2->BoundingBox().IsDefined() ) {
         if ((line1->BoundingBox().Intersects(line2->BoundingBox() ) ) ) {
	    CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Minus( line1, line2 );
	    if (res->IsEmpty() )   ((CLine *)result.addr)->SetDefined( false );
            else                   ((CLine *)result.addr) = res ->Clone();
            return(0);
	 }
	 else {
	    ((CLine *) result.addr) = line1;
	    return(0);
	 }
      }
      else {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Minus( line1, line2 );
	 if (res ->IsEmpty() )  ((CLine *)result.addr)->SetDefined( false );
         else                   ((CLine *)result.addr) = res ->Clone();
         return(0);
      }
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

const string MinusSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text> (line region) x (line region) -> (line region) </text--->"
        "<text>minus_new(_,_)</text--->"
        "<text>Returns the differenz of 2 lines/regions.</text--->"
        "<text>query minus_new ( region1,region2 )</text--->"
        ") )";

ValueMapping MinusValueMap[] = { Minus_ll, Minus_lr, Minus_rl, Minus_rr };

int minusSelectCompute ( ListExpr args) {
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )     return (0);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )    return (1);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )       return (2);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )      return (3);
   else return (-1);

}

Operator Minusrr
        ( "minus_new", MinusSpec, 4, MinusValueMap, newSpatialnomodelmap,
          minusSelectCompute, minusMap );


// operator 8 p_intersects region - region
static int p_inter_ll( Word* args, Word& result, int message, Word& local, Supplier s )
{

   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() ) {
       if (line1->IsEmpty() || line2->IsEmpty() ) {
         ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (line1->BoundingBox().IsDefined() && line2->BoundingBox().IsDefined() ) {
         if ( line1->BoundingBox().Intersects( line2->BoundingBox() ) )  {
            MakeOp mo;
            bool res = mo.P_Intersects( line1, line2 );
            ((CcBool *)result.addr) ->Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr) ->Set(true,false) ;
            return (0);
         }
      }
      else  {
         MakeOp mo;
         bool res = mo.P_Intersects( line1, line2 );
         ((CcBool *)result.addr) ->Set(true,res);
         return(0);
      }
   }
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}

static int p_inter_lr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
       if (l1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set( true, false);
         return (0);
      }
      if (l1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            MakeOp mo;
            bool res = mo.P_Intersects ( r2, l1 );
            ((CcBool *)result.addr)-> Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr) -> Set(true,false) ;
            return (0);
         }
      }
      else  {
         MakeOp mo;
         bool res = mo.P_Intersects( r2, l1 );
         ((CcBool *)result.addr) -> Set(true,res);
         return(0);
      }
   }
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}
static int p_inter_rl( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[1].addr);
   CRegion *r2 = ((CRegion*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
       if (l1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (l1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            MakeOp mo;
            bool res = mo.P_Intersects ( r2, l1 );
            ((CcBool *)result.addr)-> Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr) -> Set(true,false) ;
            return (0);
         }
      }
      else  {
         MakeOp mo;
         bool res = mo.P_Intersects( r2, l1 );
         ((CcBool *)result.addr) -> Set(true,res);
         return(0);
      }
   }
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}


static int p_inter_rr( Word* args, Word& result, int message, Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
       if (r1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (r1->BoundingBox().IsDefined() && r2->BoundingBox().IsDefined() ) {
         if ( r1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            MakeOp mo;
            bool res = mo.P_Intersects( r1, r2 );
            ((CcBool *)result.addr)->Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr)->Set(true, false) ;
            return (0);
         }
      }
      else  {
         MakeOp mo;
         bool res = mo.P_Intersects( r1, r2 );
         ((CcBool *)result.addr) -> Set(true, res);
         return(0);
      }
   }
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}

const string p_IntersectsSpec  =
        "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
        "( <text> (line region) x (line region) -> bool </text--->"
        "<text>p_intersects(_,_)</text--->"
        "<text>Returns if two lines or regions intersects interior.</text--->"
        "<text>query p_intersects ( region1,region2 )</text--->"
        ") )";

ValueMapping p_IntersectsValueMap[] = {p_inter_ll, p_inter_lr, p_inter_rl, p_inter_rr };

Operator pIntersects
          ( "p_inter", p_IntersectsSpec, 4, p_IntersectsValueMap, newSpatialnomodelmap,
	     IntersectsSelectCompute, IntersectsMap);


/*
11 Creating the Algebra

*/

class PlaneSweepAlgebra : public Algebra
{
 public:
  PlaneSweepAlgebra() : Algebra()
  {
    AddOperator (&realm);
    AddOperator (&Intersectionrr);
    AddOperator (&Unionrr);
    AddOperator (&Minusrr);
    AddOperator (&pIntersects);
    AddOperator (&Intersectsrr);
  }
  ~PlaneSweepAlgebra() {};
};

PlaneSweepAlgebra planeSweepAlgebra;

/*
12 Initialization

Each algebra module needs an initialization function. The algebra manager
has a reference to this function if this algebra is included in the list
of required algebras, thus forcing the linker to include this module.

The algebra manager invokes this function to get a reference to the instance
of the algebra class and to provide references to the global nested list
container (used to store constructor, type, operator and object information)
and to the query processor.

The function has a C interface to make it possible to load the algebra
dynamically at runtime.

*/

extern "C"
Algebra*
InitializePlaneSweepAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&planeSweepAlgebra);
}


