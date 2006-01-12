/*

[1] PlaneSweepAlgebra

September 2005 Annette Seberich-D[ue]ll

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

1 Overview

This implementation file contains the implementation of the realmisation
for the classes ~line~ and ~region~ used in the SpatialALgebra.

 They are used in the realmisation and plane-sweep-
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
3 Class Segment

This class is used to save the values of each segment of both
arguments.

*/

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
/*
konstruktor to biuld up a new segment-object

*/
Segment::Segment(bool in, const CHalfSegment& inchs)
{
   in1 = in;
   chs = CHalfSegment(inchs.IsDefined(), true, inchs.GetLP(),
         inchs.GetRP() );
   chs.attr.insideAbove = inchs.attr.insideAbove;
}

/*
functions to read and set property values of a segment-object

*/
const Point& Segment::GetLP()	{  return chs.GetLP(); }
const Point& Segment::GetRP()	{  return chs.GetRP();}
bool Segment::GetIn1() const	{ return in1; }

void Segment::SetLP(const Point& p)   {
   if  ( p != chs.GetRP() && p != chs.GetLP() )
      chs.Set(true,p,chs.GetRP());
}

void Segment::SetRP(const Point& p)   {
   if  ( p != chs.GetLP() && p != chs.GetRP() )
      chs.Set(true,chs.GetLP(),p);
}

const CHalfSegment& Segment::GetCHS() { return chs;}

/*
Inserts a CHalfSegment in right list of CHalfSegments

*/
void Segment::CHSInsert(list<CHalfSegment>& r1,
   list<CHalfSegment>& r2)
{
   if (GetIn1())   r1.push_front(GetCHS());
   else  r2.push_front(GetCHS() );
}

ostream& operator<<(ostream &os, Segment& segm)
{
   return (os << "Segment -CHS: " << segm.GetCHS() );
}

/*
2. Template

This template is implementing a binary tree (red-blach-tree).
It is used for the classes ~PQueue~, ~Statusline~ and ~Sline~.
These classes implement the priority queue
(Sweep-Event-Structure) and the Sweepline-Status-Structure
of the Plane-Sweep-Algorithm.

*/

template<class E> class BinTree ;
template<class E> class BinTreeNode;

enum Status {NEW, BAL, UNBAL, DUP};

/*
2.1 Template BinTreeNode

This class represents a node in the tree ~BinTree~. Each node has an entry, which values can be change and read. Each node had pointers to all neighors in the tree and in the list ant ro his father.

*/

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

/*
2.2 Template BinTree

This class implements a binary tree. This tree is used in the realmisation prozess and in the plane-sweep algorithm of the operations union intersection and minus. This class can be used with the classes ~XEvent~, ~SSSLine~ und ~StatusLine~ in this algebra.
To use the template the class for an entry must have a function less for comparisations of two objects.
This tree is a binary tree (a red-black-tree) with a doubled linked list, which join all entries in the tree. The list is used for searching the predecessor and the successesor of entrie. There is also a pointer to the first and the last entrie of the tree (used in the priority queue)

*/
template<class E> class BinTree
{
public:
   BinTree<E>();
   ~BinTree<E>();
   // different function to insert entries in the tree
   BinTreeNode<E>* Insert(const E &entry, const Coord x);
   BinTreeNode<E>* Insert(const E &entry,const Coord x,Segment sgs[]);
   BinTreeNode<E>* Insert(const E &entry, const Coord x,
      const E &old, BinTreeNode<E>* oldnode);
   // different functions to delete entries in the tree
   void Delete (const E &entry, const Coord x);
   void DeleteNode (BinTreeNode<E>* node) ;
   // to use the tree as a priority queue
   E GetFirstAndDelete () ;
   void Clear();
   BinTreeNode<E>* GetFirst();
   BinTreeNode<E>* GetLast();
   int GetCount();
   bool IsEmpty();
   // different funtion to insert entries in a tree
   BinTreeNode<E>* Find (const E &entry, const Coord x);
   BinTreeNode<E>* Find (const E &entry, const Coord x,
      Segment sgs[]);
   BinTreeNode<E>* Find (const E &entry, const Coord x,
      const E &old, BinTreeNode<E>* oldnode);
   void Output() ;

protected:
   // compare - functions to searxh thr right node
   int cmp(const E &first, const E &second, const Coord x);
   int cmp(const E &first, const E &second, const Coord x,
      Segment sgs[]);
   int cmp(const E &first, const E &second, const Coord x,
      const E &old, BinTreeNode<E>* oldnode);


private:
   BinTreeNode<E>* First;
   BinTreeNode<E>* Last;
   BinTreeNode<E>* Root;
   BinTreeNode<E>* NIL;
   int  mCount;
   // balancing functions for the binary tree
   void RotateRight(BinTreeNode<E>* node);
   void RotateLeft(BinTreeNode<E>* node);
   void BalanceInsert(BinTreeNode<E>* node);
   void BalanceDelete(BinTreeNode<E>* node);
   void MakeDelete(BinTreeNode<E>* node);
   void MakeInsert(BinTreeNode<E>* node, const Coord x);
   void MakeInsert(BinTreeNode<E>* node, const Coord x,
      Segment sgs[]);
   void MakeInsert(BinTreeNode<E>* node, const Coord x,
      const E &old, BinTreeNode<E>* oldnode);
   // build up a double-linked list of all entrie
   void DeleteInList(BinTreeNode<E>* node);
   void InsertBeforeInList(BinTreeNode<E>* node1, BinTreeNode<E>*
      node2);
   void InsertAfterInList(BinTreeNode<E>* node1, BinTreeNode<E>*
      node2);
   BinTreeNode<E>* TreeNext(BinTreeNode<E>* node);
   BinTreeNode<E>* TreePred(BinTreeNode<E>* node);
   BinTreeNode<E>* GetMin(BinTreeNode<E>* node);
   BinTreeNode<E>* GetMax(BinTreeNode<E>* node);
};

/*
build up a empty tree, initialise the atrributes

*/

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

/*
2.2.2 functions to insert entries

There are different functions to insert an entry in the tree. One for each class in this algebra, which uses this template. First used from class ~PQueue~ with entry ~XEvent~. Returns the first entry and delete it from the tree (top and pop). Second used in class ~StatusLine~ entry ~SSSEntry~ and third used from class ~SLine~ entry ~SEntry~.

*/

// used for class ~PQUeue~ with entry ~XEvent~
template<class E> BinTreeNode<E>* BinTree<E>::Insert (const E &entry,
   const Coord x)  {
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

// used for class ~StatusLine~ with entry ~SSSEntry~
template<class E> BinTreeNode<E>* BinTree<E>::Insert (const E &entry,
   const Coord x, Segment sgs[])
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

// used for class ~SLine~ with entry ~SEntry~
template<class E> BinTreeNode<E>* BinTree<E>::Insert(const E &entry,
   const Coord x, const E &old, BinTreeNode<E>* oldnode) {
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

/*
2.2.2 functions to delete entries

There are different functions to delete an entry in the tree. One for each class in this algebra, which uses this template. First used from class ~PQueue~ with entry ~XEvent~. Returns the first entry and delete it from the tree (top and pop). Second used in class ~StatusLine~ entry ~SSSEntry~ and third used from class ~SLine~ entry ~SEntry~.

*/

// used for class ~PQUeue~ with entry ~XEvent~
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
         if ( en.Equal(next->GetEntry()) ) {
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

// used for class ~StatusLine~ with entry ~SSSEntry~
template<class E> void BinTree<E>::Delete (const E &entry,
   const Coord x){
   BinTreeNode<E>* node = Find(entry, x);
   assert (node);
   MakeDelete(node);
   mCount --;
   delete node;
}

// used for class ~SLine~ with entry ~SEntry~
template<class E> void BinTree<E>::DeleteNode (BinTreeNode<E>*
   node) {
   assert (node);
   MakeDelete(node);
   mCount --;
   delete node;
}

/*
3.2.3 functions to find entries

There are different functions to find an entry in the tree or to find the right place to insert a entry. One for  each class in this algebra, which uses this template. First used from class ~PQueue~ with entry ~XEvent~. Second used in class ~StatusLine~ (entry ~SSSEntry~) and third used from class ~SLine~ (entry ~SEntry~).

*/

template<class E>  BinTreeNode<E>* BinTree<E>::Find (const E &entry,
   const Coord x)
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

template<class E>  BinTreeNode<E>* BinTree<E>::Find (const E &entry,
   const Coord x, Segment sgs[] )
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

template<class E>  BinTreeNode<E>* BinTree<E>::Find (const E &entry,
   const Coord x, const E &old, BinTreeNode<E>* oldnode)
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

/*
2.2.4 functions reading property values or delete all entries froom a tree

*/

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
template<class E> BinTreeNode<E>* BinTree<E>::GetFirst()
   { return First; }
template<class E> BinTreeNode<E>* BinTree<E>::GetLast()
   { return Last; }
template<class E> int BinTree<E>::GetCount () { return mCount; }


/*
2.2.5 functions to compare entries

There are different functions to compare two entries. One for each class in this algebra, which uses this template. First used from class ~PQueue~ with entry ~XEvent~. Second used in class ~StatusLine~ entry ~SSSEntry~ and third used from class ~SLine~ entry ~SEntry~.

*/

/*
used for class ~PQueue~ with entry ~XEvent~

*/
template<class E> int BinTree<E>::cmp(const E &first,
   const E &second, const Coord x) {
   if (first.Less (second, x) <0 ) return -1;
   else if ( first.Less(second, x) > 0  )  return 1;
   else return 0;
}

/*
 used for class ~StatusLine~ with entry ~SSSEntry~

*/
template<class E> int BinTree<E>::cmp(const E &first,
   const E &second, const Coord x, Segment sgs[]) {
   if (first.Less (second, x, sgs) <0 ) return -1;
   else if ( first.Less(second, x, sgs) > 0  )  return 1;
   else return 0;
}

/*
used for class ~SLine~ with entry ~SEntry~

*/
template<class E> int BinTree<E>::cmp(const E &first, const E
   &second, const Coord x, const E &old, BinTreeNode<E>* oldnode) {
   if (first.Less (second, x, old, oldnode) <0 ) return -1;
   else if ( first.Less(second, x, old, oldnode) > 0  )  return 1;
   else return 0;
}

/*
2.2.4 functions to balance binary tree

There are different functions to compare two entries. One for each class in this algebra, which uses this template. First used from class ~PQueue~ with entry ~XEvent~. Second used in class ~StatusLine~ entry ~SSSEntry~ and third used from class ~SLine~ entry ~SEntry~.

*/
template<class E> void BinTree<E>::RotateRight(BinTreeNode<E>* node)
{
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


template<class E> void BinTree<E>::RotateLeft(BinTreeNode<E>* node)
{
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

/*
function to balance tree after a inserting an entry

*/
template<class E> void BinTree<E>::BalanceInsert(BinTreeNode<E>*
   node)
{
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

/*
function to balance tree after a deleting an entry

*/
template<class E> void BinTree<E>::BalanceDelete(BinTreeNode<E>*
node)
{
   while (node != Root && node->State == BAL ) {
      if ( node == node->Parent->Left) {
         BinTreeNode<E>* n2 = node->Parent->Right;
	 if (n2->State == UNBAL) {
	    node->Parent->State = UNBAL;
	    n2->State = BAL;
	    RotateLeft(node->Parent);
	    n2 = node->Parent->Right;
         } // end second if
	 if (n2->Left->State == BAL && n2->Right->State == BAL)
	 {
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
          if ( n2->Right->State == BAL && n2->Left->State == BAL)
	  {
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

/*
2.2.7 Functions to execute the inserting an new entry
First searchs the right place, then inserts the entry and then calls the balance-function

*/
template<class E> void BinTree<E>::MakeInsert (BinTreeNode<E>* node,
   const Coord x)  {
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

template<class E> void BinTree<E>::MakeInsert (BinTreeNode<E>* node,
   const Coord x, Segment sgs[])  {
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

template<class E> void BinTree<E>::MakeInsert (BinTreeNode<E>* node,
   const Coord x, const E &old, BinTreeNode<E>* oldnode)  {
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

/*
2.2.7 Functions to execute the deleting an entry
First searchs the right entry, then delete the entry and then calls the balance-function

*/
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
         if (node->Next->Parent->Left == node)
	    node->Next->Parent->Left = node->Next;
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

/*
2.2.9 Functions to handle the list

These functions insert or delete entries in the double-linked list.

*/
template<class E> void BinTree<E>::DeleteInList(BinTreeNode<E>*
   node)
{
  if (node->Pred)  node->Pred->Next = node->Next;
   else First = node->Next;
   if ( node->Next) node->Next->Pred = node->Pred;
   else Last = node->Pred;
}

template<class E> void BinTree<E>::InsertBeforeInList(BinTreeNode<E>*
  n1, BinTreeNode<E>* n2)
{
   n2->Pred = n1->Pred;
   if (n2->Pred)  n2->Pred->Next = n2;
   else First = n2;
   n2->Next = n1;
   n1->Pred = n2;
}

template<class E> void BinTree<E>::InsertAfterInList(BinTreeNode<E>*
   n1, BinTreeNode<E>* n2)
{
   n2->Next = n1->Next;
   if (n2->Next)  n2->Next->Pred = n2;
   else Last = n2;
   n2->Pred = n1;
   n1->Next = n2;
}

/*
2.2.8 Functions to get patrs of the tree, minimum or maximum

*/
template<class E> BinTreeNode<E>* BinTree<E>::TreeNext
   (BinTreeNode<E>* node)
{
   if (node->Right != NIL ) return GetMin(node->Right);
   while ( node->Parent) {

      if (node == node->Parent->Left) break;
      node = node->Parent;
   }
   return node->Parent;
}

template<class E> BinTreeNode<E>* BinTree<E>::TreePred
   (BinTreeNode<E>* node)
{
   if (node->Left != NIL ) return GetMax(node->Left);
   while ( node->Parent) {
      if (node == node->Parent->Right) break;
      node = node->Parent;
   }
   return node->Parent;
}

template<class E> BinTreeNode<E>* BinTree<E>::GetMin
(BinTreeNode<E>* node)
{
   while (node->Left != NIL)  node = node->Left;
   return node;
}

template<class E> BinTreeNode<E>* BinTree<E>::GetMax
(BinTreeNode<E>* node)
{
   while ( node->Right != NIL) node = node->Right;
   return node;
}


/*
3 Classes for construktion of a REALM

Several classes are needed to contruct al REALM from a ~line~- or a ~region~-Object.
A REALM  is a set of points and halfsegments which not intersects.
The REALMed objects are build with a plane-sweep-algorithm.

Classes for segement-intersection a la [Bentley/Ottmann 1979]
    like Prof. G[ue]ting in November 1999 in Datenstrukturen

*/

/*
3.2 Class XEvent

Class for making entries in the priority queue

*/

/*
This declaration contains the five typs, which are the possible
events, that can occur in the realmisation

*/

enum EventKind
  {verticalSegment, rightSegment, split, intersection, leftSegment};

class XEvent
{
public:
   XEvent() {};
   ~XEvent() {};
   XEvent(Coord& x, Coord& y, int inseg, double k, double a,
      EventKind event);
   XEvent(Coord& x, Coord& y, int inseg1, int inseg2,
      EventKind event);
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
   bool Equal (const XEvent ev2) const;

private:
   Coord x;
   Coord y;
   double slope;
   double a;
   EventKind kind;
   int seg1;
   int seg2;

   //XEvent& operator= (const XEvent& ev);

};

/*
3.2.1 Construktors and Destructor

*/

/*
construktor for copy an existing antry

*/
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

/*
construktors for event-kind split

*/

XEvent::XEvent(Coord& inx, Coord& iny, int inseg,
   EventKind event)
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
   else  { }
}

/*
construktors for event-kind rightSegment, leftSegment and vrtical Segment

*/
XEvent:: XEvent(Coord& inx, Coord& iny, int inseg, double k1,
   double a1, EventKind event)
{
   if (event==rightSegment || event==verticalSegment ||
      event==leftSegment)  {
      kind=event;
      x = inx;
      y = iny;
      slope = k1;
      a = a1;
      seg1 = inseg;
      seg2 = -1;
   }
   else { }
 }

/*
konstruktors for event-kind intersection

*/
XEvent:: XEvent(Coord& inx, Coord& iny, int inseg1,
   int inseg2,EventKind event)
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
   else  {  }
}

/*
3.2.2 Member functions

sets new values for this-object

*/
void XEvent::Set(const XEvent& event) {
   x = event.x;
   y = event.y;
   slope = event.slope;
   a = event.a;
   kind= event.kind;
   seg1 = event.seg1;
   seg2 = event.seg2;
}

Coord XEvent::GetX() const		{return x;}
Coord XEvent::GetY() const		{return y;}
EventKind XEvent::GetKind() const	{return kind;}
int XEvent::GetFirst() const     	{return seg1;}
int XEvent::GetSecond() const 		{return seg2;}
double XEvent::GetSlope() const		{return slope;}
double XEvent::GetA() const		{return a;}

/*
3.2.3 Operations

*/

//XEvent& XEvent::operator= (const XEvent& ev)  { return *this;}

bool XEvent::Equal (const XEvent ev2) const
{
   if (GetKind() == ev2.GetKind() && GetFirst()== ev2.GetFirst() &&
       GetSecond() == ev2.GetSecond() )    return true;
   else return false;
}

/*
XEvents are used as entries in the priority queue (class ~PQueue~).
The priority queue uses a template ~BinTree~ for the implementation.
For
This function is need for the comparisation of two XEvents in the
template BinTree to insert the XEvents in the right node.

*/

int XEvent::Less (const XEvent ev2, const Coord x) const
{
   if ( Equal (ev2) )  return 0;
   // ordered by x-value
   else if (GetX() < ev2.GetX())  { return -1; }
   else if (GetX() > ev2.GetX()) { return 1; }
   // then ordered by event-kind
   else if (GetKind() < ev2.GetKind())  	{ return -1;}
   else if (GetKind() > ev2.GetKind())  	{ return 1;}
   else  {// groups ordered by different categories
      if (GetKind() == leftSegment || GetKind() == verticalSegment) {
          // add at the end of a group
         if (GetFirst() < ev2.GetFirst())	{ return -1;}
	 else					{ return 1;}
      }
      else if (GetKind() == rightSegment) {
         // then ordered by y-value, then slope of the segment
         if (GetY() < ev2.GetY())		{ return -1;}
	 else if (GetY() > ev2.GetY())		{return 1;}
	 else if (GetSlope() > ev2.GetSlope())	{return 1;}
	 else 					{return -1;}
      }
      else if (GetKind() == intersection)  {
         // ordered by y-value
         if (GetY() < ev2.GetY())		{return -1;}
	 else					{return 1;}
      }
      else if (GetKind() == split)  {
         // ordered by y-value
         if (GetY() < ev2.GetY())		{return -1;}
	 else					{return 1;}
      }
      // should not reached
      else {
         cout << "eventkind not known < " << GetKind() << endl;
         return -1;
     }
   }
}

ostream& operator<<(ostream &os, const XEvent& ev)
{
   return (os << "XEvent: x:" << ev.GetX() << "  y: "
      << ev.GetY() << "  Kind: " << ev.GetKind()<< "   First: "
      << ev.GetFirst() << "  Second:  " << ev.GetSecond()
      << "  slope: " << ev.GetSlope() << "  a: " << ev.GetA()
      << " )"  );
}



/*
3.3 class PQueue

This class is implementing the priority of XEvents used in the realmisation prozess and also in the plane-Sweep-algorithm for operations.
The priority queue based on a binary tree (template ~BinTree~) and uses the class ~XEvent~ as entries.

*/

class PQueue
{
public:
   PQueue();
   void Clear() { qu.Clear(); };
   ~PQueue() { qu.Clear(); };
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

/*
functions for initialising a tree and reading property values for a PQueue-Object

*/
PQueue::PQueue()  		{BinTree<XEvent> qu; }
bool PQueue::isEmpty()		{ return (qu.IsEmpty() ); }
int PQueue::size()		{ return (qu.GetCount()) ; }

/*
implements the push-function

*/
void PQueue::insert (XEvent& event)   {
   Coord y;
   qu.Insert(event, y);
}

/*
implements the functions top and pop of a priority queue in one function

*/
XEvent PQueue::getFirstAndDelete()  {
   XEvent event =  qu.GetFirstAndDelete();
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
         cout << "XEvent: "<< ev.GetX()<< " "<< ev.GetY()<< " "
	 << ev.GetKind()<< " " << ev.GetFirst()<<" "<<ev.GetSecond()
	 <<" "<<ev.GetSlope()<< " "<< ev.GetA()<< endl;
         node = node->GetNext();
      }
   }
   cout << " in output fertig " << endl;
}


/*
3.4 class SSSEntry

This class implements a entry for the Sweepline-Status-Structure (class ~StatusLine~). Each entry has three attributes: the CHalfSegment coded as a integer and two double-values to calculate the y-value for a given x-Coordinate

*/

class SSSEntry
{
public:
   SSSEntry() { chs = -1;};
   ~SSSEntry() {};
   SSSEntry(int inseg, Segment segs[] );
   SSSEntry(int inseg, double ink, double ina);
   void Set(const SSSEntry& in);

   bool Equal (const SSSEntry se) const;
   int GetSeg() const;
   const double GetY (Coord x, Segment segs[]) const;
   double Getk() const;
   double Geta() const;
   int SSSEntry:: Less (const SSSEntry in2, const Coord x,
      Segment segs[]) const;

private:
   int chs;
   double k;
   double a;
};

/*
3.4.1 Constructors and Destructors

*/
SSSEntry:: SSSEntry(int inseg, double ink, double ina)
{
   chs = inseg;
   k = ink;
   a = ina;
}

/*
creates a new entry-object from a given CHalfsegments and calculates slope and a

*/
SSSEntry:: SSSEntry (int inseg, Segment segs[])
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


/*
3.4.2 Functions for readig and setting values of a SSSEntry-Object

*/
void SSSEntry:: Set(const SSSEntry& in)
{
   chs = in.chs;
   k = in.k;
   a = in.a;
}

double SSSEntry::Getk() const			{return k; }
double SSSEntry::Geta() const			{return a; }
int SSSEntry::GetSeg() const			{return chs; }

/*
3.4.3 Functions for comparisation

*/
bool SSSEntry::Equal(const SSSEntry se) const
{
   if (GetSeg() == se.GetSeg()) return true;
   else return false;
}

/*
SSSEntry-objects are used as entries in the sweepline-structure (class ~StatusLine~). This class uses a template ~BinTree~ for the implementation.
These two function are need to calculate the y-value and for the comparisation of two XEvents in the template BinTree to insert the XEvents in the right node.

*/

const double SSSEntry::GetY(Coord x, Segment segs[]) const  {
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

int SSSEntry:: Less (const SSSEntry in2, const Coord x,
   Segment segs[]) const
{
   if (Equal(in2)) return 0;
   else if (GetY(x, segs) < in2.GetY(x, segs)) return -1;
   if (GetY(x, segs) > in2.GetY(x, segs)) return 1;
   // same y-value -> compare slope
      if (Getk() < in2.Getk()) return -1;
      else if (Getk() > in2.Getk() ) return 1;
      // same y-value, same slope -> at the end of the list
      else if (GetSeg() < in2.GetSeg() ) return -1;
      else if (GetSeg() > in2.GetSeg() ) return 1;
      else return 0;
   return 0;
}

ostream& operator<<(ostream &os, const SSSEntry& en)
{
  return (os   <<" CHS("<< en.GetSeg() <<") (k: "
               << en.Getk() << " a:" << en.Geta() <<") ");
}

/*
3.5 class  StatusLine

This class implements the Sweepline-Status-Structure of the realmisation prozess. It uses the template ~BinTree~ and the class ~SSSEntry~ as entry-objects.

*/

class StatusLine
{
public:
   friend class VList;
   StatusLine();
   void Clear() { ssl.Clear(); };
   ~StatusLine() { ssl.Clear(); };
   void Insert(Coord x, SSSEntry& entry, Segment sgs[], PQueue& pq);
   void Delete(Coord x, Coord oldx, SSSEntry& entry, Segment sgs[],
      PQueue& pq );
   void Exchange (BinTreeNode<SSSEntry>* e1, BinTreeNode<SSSEntry>*
      e2, Segment sgs[], PQueue& pq);
   BinTreeNode<SSSEntry>* Find(const Coord x, const Coord oldx,
      const SSSEntry& entry,Segment segs[]);

   void output(Coord x, Segment segs[]);

private:
   BinTree<SSSEntry> ssl;

   bool IsEmpty();
   int Size();
   bool InsertIntersection(const CHalfSegment& chs1,
      const CHalfSegment& chs2,int s1,int s2, PQueue& pq );
   void erase(SSSEntry* entry);
   BinTreeNode<SSSEntry>* GetGreater(Coord xvalue, Coord y,
     Segment segs[]);
   BinTreeNode<SSSEntry>* GetNext(BinTreeNode<SSSEntry>* ent);
   BinTreeNode<SSSEntry>* GetPred(BinTreeNode<SSSEntry>* ent);
   bool innerInter( const CHalfSegment& chs1, const CHalfSegment&
      chs2,  Point& resp, CHalfSegment& rchs, bool& first,
      bool& second ) const;
};

void StatusLine::output(double x, Segment segs[])
{
   BinTreeNode<SSSEntry>* p = ssl.GetFirst();
   if  (! ssl.IsEmpty() ) {
      while ( p != 0 ) {
         SSSEntry en = p->GetEntry();
         cout << "y:"<< en.GetY(x, segs) << "  seg:"
	      << en.GetSeg() << " k:" << en.Getk()
              << "  a:" << en.Geta() << endl;
         p = p->GetNext();
      }
   }
}

/*
3.5.1 Constructors and functions for getting and setting property values

*/

StatusLine::StatusLine()	{ BinTree<SSSEntry> ssl;}
bool StatusLine::IsEmpty() 	{ return  ssl.IsEmpty() ; }
int StatusLine::Size()		{ return ssl.GetCount(); }
BinTreeNode<SSSEntry>* StatusLine::GetNext
   (BinTreeNode<SSSEntry>* ent)  {return ent->GetNext();}
BinTreeNode<SSSEntry>* StatusLine::GetPred
   (BinTreeNode<SSSEntry>* ent)  {return ent->GetPred();}

/*
3.5.2 Functions for insert, delete and find entry-objects

*/

/*
This function is used to find the right position to insert a new entry. It searchs that node in the statusline, which will be the next in the order. (Used in the insert-function)

*/
BinTreeNode<SSSEntry>* StatusLine::GetGreater(Coord x, Coord y,
   Segment segs[])
{
   BinTreeNode<SSSEntry>* node = ssl.GetFirst();
   while ( node != 0 && (node->GetEntry().GetY(x, segs) <= y) )
       node = node -> GetNext();
   return node;
}
/*
This function searches a entry in the Sweepline-Staus-Stucture. It uses the calculated y-value. (Used in the delete-function.)

*/
BinTreeNode<SSSEntry>* StatusLine::Find(const Coord x,
   const Coord oldx, const SSSEntry& entry, Segment segs[])
{
   BinTreeNode<SSSEntry>* node = ssl.Find(entry, x, segs);
   if (node == 0) {
      node = ssl.Find(entry, oldx, segs );
      if (node == 0) {
         node = ssl.GetFirst();
	 while (node!=0 && node->GetEntry().GetSeg()!=
	    entry.GetSeg() )  node = node->GetNext() ;
       }
   }
   return node;
}

/*
function to insert an entry in the sweepline, after inserting at the right position (sweep line is ordered by y-values) tests for intersection are made with predecessor and successor.
Additionally tests are made with more neighbors, if two segments are parallel.

*/
void StatusLine::Insert(Coord x, SSSEntry& entry, Segment sgs[],
   PQueue& pq)
{
   // insert entry into sweepline
   BinTreeNode<SSSEntry>* en = ssl.Insert(entry, x, sgs);
   BinTreeNode<SSSEntry>* pred = en->GetPred();
   BinTreeNode<SSSEntry>* succ = en->GetNext();
   int segnr = entry.GetSeg();
   if (pred != 0 ) { // test for intersection with predecessor
     int psegnr = pred->GetEntry().GetSeg();
     bool inter = InsertIntersection (sgs[psegnr].GetCHS(),
       sgs[segnr].GetCHS(), psegnr, segnr, pq);
       if (inter) { // pred and entry intersects
	 BinTreeNode<SSSEntry>* predpred = pred->GetPred();
	 if (predpred != 0 ) { // test for intersection with predpred
           int ppsegnr = predpred->GetEntry().GetSeg();
	   bool inter2=InsertIntersection(sgs[ppsegnr].GetCHS(),
	     sgs[segnr].GetCHS(), ppsegnr,segnr,pq);
	     if (inter2) { // and so on
               BinTreeNode<SSSEntry>* predpredpred =
		predpred->GetPred();
	        if (predpredpred !=0) {
	          int pppSegnr = predpredpred->GetEntry().GetSeg();
                  InsertIntersection(sgs[pppSegnr].GetCHS(),
		    sgs[segnr].GetCHS(), pppSegnr,segnr,pq);
	       } // predpredpred
            } // (inter2)
         } // predpred !=0
      } // (inter)
   } // pred !=0
   if (succ != 0) {   // test for intersection with successor
    int sSegnr = succ->GetEntry().GetSeg();
      bool inter = InsertIntersection (sgs[segnr].GetCHS(),
         sgs[sSegnr].GetCHS(), segnr, sSegnr, pq);
      if (inter) { // intersection entry- succ
	BinTreeNode<SSSEntry>* succsucc = succ->GetNext();
	if (succsucc != 0 ) { // succsucc exists
          int ssSegnr = succsucc->GetEntry().GetSeg();
	  bool inter2= InsertIntersection (sgs[segnr].GetCHS(),
	    sgs[ssSegnr].GetCHS(), segnr, ssSegnr,pq);
	    if (inter2) { // intersection entry - succsucc
	      BinTreeNode<SSSEntry>* succsuccsucc =
	         succsucc->GetNext();
	      if (succsuccsucc !=0) {
	        int sssSegnr = succsuccsucc->GetEntry().GetSeg();
		InsertIntersection(sgs[segnr].GetCHS(),
		  sgs[sssSegnr].GetCHS(), segnr,sssSegnr,pq);
	   }
	 } // (inter2)
       } // succsucc!0=
     } // (inter)
   } // succ !=0
}

/*
function for deleting a entry from the sweep line
Atuomatically a test for intersection the new neihbors is added (predecessor and succsessor of ald entry are neighbors now).

*/
void StatusLine::Delete(Coord x, Coord oldx, SSSEntry& entry,
   Segment sgs[], PQueue& pq)
{
   BinTreeNode<SSSEntry>* en = Find(x, oldx, entry, sgs);
   if (en != 0) {
      BinTreeNode<SSSEntry>* pred = en->GetPred();
      BinTreeNode<SSSEntry>* succ = en->GetNext();
      if ( pred != 0 && succ != 0)
      {
         int s1 = pred->GetEntry().GetSeg();
	 int s2 = succ->GetEntry().GetSeg();
         // test for intersection with pred- and succ-CHalfSegment
         if (sgs[s1].GetIn1() != sgs[s2].GetIn1() )
            InsertIntersection (sgs[s1].GetCHS(), sgs[s2].GetCHS(),
	    s1, s2, pq);
      }
      ssl.DeleteNode(en);
   }
}

const double DIST = 1* exp(-20.0);

/*
This new Functions is needed for testing, if two CHalfsegments
intersects with inner points and if this is true,
as result the single point of intersection or the Segments which overlap are given as result in resp or rchs
If the intersetion points are very close to the endpoint (Distance is
less than DIST), it is assumed, that the intersection is only a problem of rounding double values

*/

bool StatusLine::innerInter( const CHalfSegment& chs1,
   const CHalfSegment& chs2, Point& resp, CHalfSegment& rchs,
   bool& first, bool& second ) const
{
   resp.SetDefined(false);   	rchs.SetDefined(false);
   first = false;		second = false;
   Coord xl,yl,xr,yr ,  Xl,Yl,Xr,Yr;
   double k, a, K, A;
   xl=chs1.GetLP().GetX();  yl=chs1.GetLP().GetY();
   xr=chs1.GetRP().GetX();  yr=chs1.GetRP().GetY();
   if (xl!=xr)  // chs1 not vertical
   {     //k=(yr-yl) / (xr-xl);  a=yl - k*yl;
    #ifdef RATIONAL_COORDINATES
      k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
         (yl.IsInteger()? yl.IntValue():yl.Value())) /
        ((xr.IsInteger()? xr.IntValue():xr.Value()) -
         (xl.IsInteger()? xl.IntValue():xl.Value()));
      a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
         k*(xl.IsInteger()? xl.IntValue():xl.Value());
    #else
      k=(yr - yl) / (xr - xl);
      a=yl - k*xl;
    #endif
   }
   Xl=chs2.GetLP().GetX();  Yl=chs2.GetLP().GetY();
   Xr=chs2.GetRP().GetX();  Yr=chs2.GetRP().GetY();
   if (Xl!=Xr)  // chs2 not vertical
   {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
    #ifdef RATIONAL_COORDINATES
      K=  ((Yr.IsInteger()? Yr.IntValue():Yr.Value()) -
          (Yl.IsInteger()? Yl.IntValue():Yl.Value())) /
         ((Xr.IsInteger()? Xr.IntValue():Xr.Value()) -
          (Xl.IsInteger()? Xl.IntValue():Xl.Value()));
      A = (Yl.IsInteger()? Yl.IntValue():Yl.Value()) -
         K*(Xl.IsInteger()? Xl.IntValue():Xl.Value());
    #else
      K=  (Yr - Yl) / (Xr - Xl);
      A = Yl - K*Xl;
    #endif
   }
   if ((xl==xr) && (Xl==Xr))  {
      //both chs1 and chs2 are vertical lines
      if (xl!=Xl) return false;
      else  {
         Coord ylow, yup, Ylow, Yup;
         if (yl<yr)   { ylow=yl;  yup=yr;  }
         else         { ylow=yr;  yup=yl;  }
         if (Yl<Yr)   { Ylow=Yl;  Yup=Yr;  }
         else	  { Ylow=Yr;  Yup=Yl;  }
         if  (((ylow>Ylow) && (ylow<Yup))||
	      ((yup>Ylow) && (yup<Yup)) ||
              ((Ylow>ylow) && (Ylow<yup))||
	      ((Yup>ylow) && (Yup<yup))) {
            Point p1, p2;
            if (ylow>Ylow)	p1.Set(xl, ylow);
            else 		p1.Set(xl, Ylow);
            if (yup<Yup) 	p2.Set(xl, yup);
            else 		p2.Set(xl, Yup);
            rchs.Set(true, p1, p2);
	    first = true; 	second = true;
            return true;
         }
         else return false;
      }
   } // (xl==xr) && (Xl==Xr)
   else if (Xl==Xr) {    //only L is vertical
      if ( xl==Xl && yl>Yl && yl<Yr )
         {resp.Set(xl,yl); second = true; return true;}
      if ( xr==Xl && yr>Yl && yr<Yr )
         {resp.Set(xr,yr); second = true; return true;}
      else  {
        #ifdef RATIONAL_COORDINATES
         double y0=k*(Xl.IsInteger()? Xl.IntValue():Xl.Value())+a;
         Coord yy(y0);
       #else
         double y0=k*Xl+a;
         Coord yy=y0;
      #endif
         //(Xl, y0) is the intersection of l and L
         if ((Xl>xl) &&( Xl<xr))  {
	    if ( (yy>=Yl) && (yy <= Yr) ) {
	       resp.Set (Xl,yy);
	       // test if rounding problem, then no intersection
	       if ( (abs(xl-Xl) < DIST && abs(yl-yy) < DIST) ||
	            (abs(xr-Xl) < DIST && abs(yr-yy) < DIST) ||
		     abs(Yl-yy) < DIST || abs(Yr-yy) < DIST)
	          return false;
	       first = true;
               if ( (yy>Yl) && (yy<Yr) ) second = true;
	       return true;
	    }
	    else return false;
         }
      }
   }  // else if (Xl==Xr)
   else if (xl==xr) {    //only l is vertical
// hier einfügen
      if ( Xl==xl && Yl>yl && Yl<yr )
         {resp.Set(Xl,Yl); first = true; return true;}
      if ( Xr==xl && Yr>yl && Yr<yr )
         {resp.Set(Xr,Yr); first = true; return true;}
      else  {
        #ifdef RATIONAL_COORDINATES
         double y0=K*(xl.IsInteger()? xl.IntValue():xl.Value())+A;
         Coord yy(y0);
       #else
         double y0=K*xl+A;
         Coord yy=y0;
      #endif
         //(Xl, y0) is the intersection of l and L
         if ((xl>Xl) && (xl<Xr))  {
	    if ( (yy>=yl) && (yy <= yr) ) {
	       resp.Set (xl,yy);
	       // test if rounding problem, then no intersection
	       if ( (abs(Xl-xl) < DIST && abs(Yl-yy) < DIST) ||
	            (abs(Xr-xl) < DIST && abs(Yr-yy) < DIST) ||
		     abs(yl-yy) < DIST || abs(yr-yy) < DIST)
	          return false;
	       second = true;
               if ( (yy>yl) && (yy<yr) ) first = true;
	       return true;
	    }
	    else return false;
         }
      }
   }
   //otherwise: both *this and *arg are non-vertical lines
   if (k==K)   { // both lines are parallel or the same
      if (a != A) return false;  // parallel lines
      if  (((xl>Xl) && (xl<Xr)) || ((xr>Xl) && (xr<Xr)) ||
           ((Xl>xl) && (Xl<xr)) || ((Xr>xl) && (Xr<xr)))  {
         Point p1, p2;
         if (xl>Xl) 	p1.Set(xl, yl);
         else  		p1.Set(Xl, Yl);
         if (xr<Xr)	p2.Set(xr, yr);
         else  		p2.Set(Xr, Yr);
         rchs.Set(true, p1, p2);
	 first = true; second = true;
         return true;
      }
     else return false;
   } // else (k==K)
   else  {
      double x0 = (A-a) / (k-K);  // y0=x0*k+a;
      double y0 = x0*k+a;
     #ifdef RATIONAL_COORDINATES
        Coord xx(x0);   Coord yy(y0);
     #else
        Coord xx = x0; Coord yy=y0;
     #endif
     if (chs1.GetLP() == chs2.GetLP() ||
         chs1.GetRP() == chs2.GetRP() ) return false;
     // test if rounding problem, then no intersection
     resp.Set(xx,yy);
     if ( (abs(Xl-xx) < DIST && abs(Yl-yy) < DIST) ||
	  (abs(Xr-xx) < DIST && abs(Yr-yy) < DIST) ||
	  (abs(xl-xx) < DIST && abs(yl-yy) < DIST) ||
	  (abs(xr-xx) < DIST && abs(yr-yy) < DIST) )
        return false;
     if ((xx == xl || xx == xr) && xx > Xl && xx < Xr )
        {second = true; return true; }
     if ( (xx == Xl || xx == Xr) && xx > xl && xx < xr )
        {first = true; return true; }
     if ((xx>xl) && (xx<xr) && (xx>Xl) && (xx <Xr))
        {first = true; second= true; return true; }
     else  return false;
   }
}


/*
With this function the tests if two segment intersects are made.
First: test, if two segments intersects (innerInter in SpatialAlgebra).
Second:
- they intersect in one point, a XEvent intersection or split is created.
- they overlap, XEvent split is inserted in the ~PQueue~

*/
bool StatusLine::InsertIntersection(const CHalfSegment& chs1,const
   CHalfSegment& chs2,int s1, int s2, PQueue& pq)
{
   bool result = false;
   Point point;
   Coord xp, yp;
   CHalfSegment res;
   bool one, two;
   // test if two CHalfSegments intersects
   // if intersection in one point -> point isDefined
   // if they overlap -> CHalfSegment res is the result-segment
   bool innerinter = innerInter(chs1, chs2, point, res, one, two);
   if (innerinter)  {
      result = true;
      if (point.IsDefined() ) { // intersection in one point
	 xp = point.GetX();  yp = point.GetY();
	 // both segments intersect with their inner point
	 if ( one && two) {XEvent ev1(xp, yp, s1, s2, intersection);
	                  pq.insert(ev1);}
	// only one segment must be split
	 else if (one)
	    {XEvent ev1(xp, yp, s1, split); pq.insert(ev1);}
	 else if (two)
	     {XEvent ev1(xp, yp, s2, split); pq.insert(ev1);}
      }
      else if (res.IsDefined() ) {  // overlap-intersection
         // both segments are the same
	 if (chs1 == chs2) { return true;}
	 // both segments starts with same point
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
	 // both segments ends in same point
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
	 // one segement inside the other
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

/*
with this function two entries are exchanged. A test for intersection for new neighbors is added

*/
void StatusLine::Exchange (BinTreeNode<SSSEntry>* e1,
   BinTreeNode<SSSEntry>* e2, Segment sgs[], PQueue& pq)
{
   BinTreeNode<SSSEntry>* pred = e1->GetPred();
   BinTreeNode<SSSEntry>* succ = e2->GetNext();
   SSSEntry entry1 ( e1->GetEntry() );
   SSSEntry entry2 ( e2->GetEntry() );
   e2->SetEntry(entry1);
   e1->SetEntry(entry2);
   if (pred != 0 ) {
      int e = e1->GetEntry().GetSeg();
      int s1 = pred->GetEntry().GetSeg();
      if (sgs[s1].GetIn1() != sgs[e].GetIn1() )
         InsertIntersection(sgs[s1].GetCHS(),sgs[e].GetCHS(),
	 s1,e,pq);
   }
   if (succ != 0 ) {
      int es = e2->GetEntry().GetSeg();
      int s2 = succ->GetEntry().GetSeg();
      if (sgs[es].GetIn1() != sgs[s2].GetIn1() )
         InsertIntersection(sgs[es].GetCHS(),sgs[s2].GetCHS(),es,
	 s2,pq);
   }
}

/*
3.6 class VStructure

This class is  activated, if a vertical segment occurs (SetDefined = true). It is used to store the y-values of end-point of segments at the sweepline, at wich the vertical segment must be split.
In this class a <list> from the C++-Standards is used.

*/

class VStructure {
public:
   friend class VList;
   VStructure();
   ~VStructure()   {};
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
   if (IsDefined() ) cout << " in output fertig min " << min
      << "   max" << max << endl;
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

/*
a new value of an y-coordinate is inserted

*/
void VStructure::Insert (Coord y)
{
   if (IsEmpty()) {
      vstruct.insert(vstruct.begin(), y);
      min = y;
      max = y;
   }
   // insert a new coordinate ordered by his value
   list<Coord>::iterator p = vstruct.begin();
   while ( (*p < y) && (p != vstruct.end()) )   { ++p; }
   if ( (*p) != y ) {
      vstruct.insert(p,y);
      if (y < min)   { min = y;}
      else if (y > max)  {max = y;}
   }
}

/*
3.7 class vlist

This class is used to handle vertical segments.
If a vertical segments occurs the vlist is activated and the segment is inserted. When the sweep line moves, the vlist is cleared, that means alss segments in the vlist are tested for overlaps with each other in the list and the sweepline is scanned from bottom to top for intersection with segments.
Implementation uses the <list> from C++-Standards

*/

class VList {
public:
   VList();
   ~VList() {};
   void Insert (Segment& seg);
   list<Segment> processAll(Coord& sweepline, VStructure& vs,
      StatusLine& sl, Segment segs[]);
    void Clear();
    bool IsEmpty();

private:
   list<Segment> vlist;

   void DeleteFirst();
   Segment First();
   int Size();
   list<Segment> eraseOverlaps(Coord sline, VStructure vs);
   void testStatusLine(StatusLine& sl, Segment segs[]);
   void testOverlaps(Coord& sweepline, VStructure& vs,
      StatusLine& sl, Segment segs[]);
   void output();
};

/*
Initalisation, reading properties

*/
VList::VList()     	   { list<Coord> vlist; }
int VList::Size()    	   { return vlist.size(); }
Segment VList::First()   { return (vlist.front()); }
bool VList::IsEmpty()	   { return vlist.empty(); }

/*
functions for handling the list

*/
void VList::Clear()  	   { vlist.clear(); }
void VList::Insert (Segment& seg)   {vlist.push_back(seg);}
void VList::DeleteFirst()  { vlist.erase(vlist.begin()); }



void VList::output()
{
   cout << " in output VList " << endl;
   list<Segment>::iterator p = vlist.begin();
   while (p != vlist.end())
   {
      Segment s = *p;
      cout << "   Segment = " << s.GetIn1() << " "
           <<s.GetCHS().GetLP()<<" "<<s.GetCHS().GetRP()<<endl;
      ++p;
   }
   cout << " in output fertig " << endl;
}

/*
This function is called if the sweep line moves to a new position. The vertical segments are worked off. This means tests are made to find intersection with other segments.

*/
list<Segment> VList::processAll(Coord& sweep, VStructure& vstruct,
   StatusLine& ssl, Segment segments[])
{
   if ( ! IsEmpty() )  {
      testOverlaps( sweep ,vstruct, ssl, segments);
      testStatusLine (ssl, segments);
   }
   return vlist;
}

/*
test for overlaps all  vertical segments at the same x-coordinate

*/

void VList::testOverlaps(Coord& sweepline, VStructure& vs,
   StatusLine& sl, Segment segs[])
{
   list<Segment> newlist;
   while ( !IsEmpty() )  {
      bool deleted = false;
      Segment first = First();
      CHalfSegment chs = first.GetCHS();
      DeleteFirst();
      list<Segment>::iterator p = vlist.begin();
      // test first Segment for overlaps with other CHalfSegments
      while ( (p != vlist.end()) &&  (deleted == false)  )  {
         Segment ref = *p;
	 CHalfSegment test = ref.GetCHS();
	 CHalfSegment res;
	 bool overlap = chs.overlapintersect(test,res);
	 if (overlap  && ( first.GetIn1() != ref.GetIn1()) ) {
	    // only different regions are tested
            // first segment inside the other Segment
	    if (chs.Inside(test) ) {
	       if (chs.GetLP()==test.GetLP()&&chs.GetRP()==
	          test.GetRP()) { }
	       else if (chs.GetLP() == test.GetLP() ) {
		  Segment newseg(ref.GetIn1(),test);
		  newseg.SetRP(res.GetRP());
		  vlist.insert(p,newseg);
		  ref.SetLP(res.GetRP());
		  vlist.erase(p);  vlist.push_back(ref);
	       }
	       else if (chs.GetRP() == test.GetRP()) {
	          Segment newseg(ref.GetIn1(),test);
		  newseg.SetRP(res.GetLP());
		  vlist.insert(p,newseg);
		  ref.SetLP(res.GetLP());
		  vlist.erase(p); vlist.push_back(ref);
	       }
	       else  {
	          Segment newseg1(ref.GetIn1(),test);
		  newseg1.SetRP(res.GetLP());
		  vlist.insert(p,newseg1);
		  Segment newseg2(ref.GetIn1(), test);
		  newseg2.SetLP(res.GetLP());
		  newseg2.SetRP(res.GetRP());
		  vlist.insert(p,newseg2);
		  ref.SetLP(res.GetRP());
		  vlist.erase(p);
		  vlist.push_back(ref);
	       }
	    }
	    // second segment inside the other
	    else if (test.Inside(chs))  {
	       if (chs.GetLP() == test.GetLP() ) {
		  Segment newseg(first.GetIn1(),chs);
		  newseg.SetRP(res.GetRP());
		  vlist.insert(p,newseg);
		  first.SetLP(res.GetRP());
               }
	       else if (chs.GetRP() == test.GetRP()) {
		  Segment newseg(first.GetIn1(),chs);
		  newseg.SetRP(res.GetLP());
		  vlist.insert(p,newseg);
		  first.SetLP(res.GetLP());
	       }
	       else  {
		  Segment newseg1(first.GetIn1(),chs);
		  newseg1.SetRP(res.GetLP());
		  vlist.insert(p,newseg1);
		  Segment newseg2(first.GetIn1(),chs);
		  newseg2.SetLP(res.GetLP());
		  newseg2.SetRP(res.GetRP());
		  vlist.insert(p,newseg2);
		  first.SetLP(res.GetRP());
	       }
	    }
	    // overlap but not inside
	    else if (res.GetLP() > chs.GetLP())  {
	       Segment newseg1(first.GetIn1(),chs);
	       newseg1.SetRP(res.GetLP());
	       vlist.insert(p,newseg1);
	       first.SetLP(res.GetLP());
	       Segment newseg2(ref.GetIn1(),test);
	       newseg2.SetRP(res.GetRP());
	       vlist.insert(p,newseg2);
	       ref.SetLP(res.GetRP());
	       vlist.erase(p); vlist.push_back(ref);
	    }
	    else   {
	       Segment newseg1(first.GetIn1(),chs);
	       newseg1.SetRP(res.GetRP());
	       vlist.insert(p,newseg1);
	       first.SetLP(res.GetRP());
	       Segment newseg2(ref.GetIn1(),test);
	       newseg2.SetRP(res.GetLP());
	       vlist.insert(p,newseg2);
	       ref.SetLP(res.GetLP());
	       vlist.erase(p); vlist.push_back(ref);
	    }
	 } // end overlap
         ++p;
      } // end 2nd while and now treat rightEnds of CHAlfSegments
      // tests if there are endpoints of other segments inside verticals
      Coord min = vs.GetMin();
      Coord max =  vs.GetMax();
      chs = first.GetCHS();
      if (!((chs.GetLP().GetY()<=min && chs.GetRP().GetY()<= min)
	 ||(chs.GetLP().GetY()>=max && chs.GetRP().GetY()>= max))) {
  	 list<Coord>::iterator ps = vs.vstruct.begin();
	 bool ready = false;
	 while (ps != vs.vstruct.end() and ready != true) {
	    Coord y = *ps;
            if (y >= chs.GetRP().GetY())   ready = true;
	    else if (y > chs.GetLP().GetY()) {// split CHalfsegment
	       Point newp = Point(true, sweepline,y);
               Segment newseg(first.GetIn1(),first.GetCHS());
	       newseg.SetRP(newp);
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

/*
for each vertical segment in the list the sweep line is scanned from the bottom of a vertical to the top. If a non-vertical segments occurs, a XEvent split is inserted. The vertical segment ist also split and inserted in result.

*/

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
      // searches the next segment to the bottom of vertical segment
      BinTreeNode<SSSEntry>* node = sline.GetGreater(x,y,segs);
      while (node != 0)  {
         SSSEntry  ent = node -> GetEntry();
         Segment testref = segs[ent.GetSeg()];
         CHalfSegment test = testref.GetCHS() ;
         Point res;
	 // tests if a segments intersects the vertical
         bool inter = vert.spintersect (test, res);
	 if (inter) { // intersection occurs
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
	 if (node!=0 && node->GetEntry().GetY(x,segs)
	    >vert.GetRP().GetY())  {node = 0;}
      } // all entries in statusline tested
      p++; // next vertical Segment
      newlist.push_back(vertref);
   }
   vlist = newlist;
}

/*
3.8 class MakeRealm

In this class the realmisation prozess was worked out. For each possible  combination of line and region-objects a function exists.
the steps were the same, but the results are different. In the first step the plane-Sweep was prepared, that means for all segments in both objects the XEvent for left and right end was inserted in the priority queue. In the second step the sweep line moves from left ro right through the plane and handle all events.

*/

class MakeRealm {
public:
   MakeRealm() {};
   ~MakeRealm() {};
   void PQueueOutput(PQueue& pq);
   void  REALM (CRegion* reg1, CRegion* reg2, CRegion* result1,
      CRegion* result2);
   void  REALM (CRegion* reg1, CRegion* reg2, CLine* result1,
   CLine* result2);
   void  REALM (CLine* reg1, CRegion* reg2, CLine* result1,
      CRegion* result2);
   void  REALM (CLine* reg1, CLine* reg2, CLine* result1,
      CLine* result2);

private:
   void PerformPlaneSweep(PQueue& pq,Segment segs[],
      list<CHalfSegment>& res1,list<CHalfSegment>& res2,
      const int counter);
   void PrepareCHS (bool No1, CHalfSegment& chs, PQueue& pqu,
      Segment segs[], int counter);
};

void MakeRealm::REALM(CRegion* reg1, CRegion* reg2, CRegion* result1,
   CRegion* result2)
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
   pqueue.Clear();
   //  reconstruction of first region
   result1->Clear();
   result1->StartBulkLoad();
   int counter = 0;
   while (! res1.empty() )  {
      CHalfSegment chs = res1.back();
      chs.attr.partnerno = counter;
      chs.SetLDP(true);       result1->InsertHs(chs);
      chs.SetLDP(false);      result1->InsertHs(chs);
      res1.pop_back();
      counter ++;
   }
   result1 -> EndBulkLoad(true);
   result1->SetPartnerNo();
   result1->ComputeRegion();
   //  reconstruction of second region
   result2->Clear();
   result2->StartBulkLoad();
   counter = 0;
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

void MakeRealm::REALM(CRegion* reg1, CRegion* reg2, CLine* result1,
   CLine* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the region for PlanSweep
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
   // Prepare the line for PlaneSweep
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
   //  reconstruction of region
   result1->Clear();
   result1->StartBulkLoad();
   int counter = 0;
   while (! res1.empty() )  {
      CHalfSegment chs = res1.back();
      chs.attr.partnerno = counter;
      chs.SetLDP(true);       result1->InsertHs(chs);
      chs.SetLDP(false);      result1->InsertHs(chs);
      res1.pop_back();
      counter ++;
   }
   result1 -> EndBulkLoad(true);
   //  reconstruction of line
   result2->Clear();
   result2->StartBulkLoad();
   counter = 0;
   while (! res2.empty() )    {
      CHalfSegment chs = res2.front();
      chs.attr.partnerno = counter;
      chs.SetLDP(true);       result2->InsertHs(chs);
      chs.SetLDP(false);      result2->InsertHs(chs);
      res1.erase(res2.begin());
      counter++;
   }
   result2 -> EndBulkLoad(true);
}

void MakeRealm::REALM(CLine* line1, CRegion* reg2, CLine* result1,
   CRegion* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the line for PlanSweep
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
   // Prepare the region for PlaneSweep
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
   //  reconstruction of region
   result1->Clear();
   result1->StartBulkLoad();
   while (! res1.empty() )  {
      CHalfSegment chs = res1.back();
      chs.SetLDP(true);       result1->InsertHs(chs);
      chs.SetLDP(false);      result1->InsertHs(chs);
      res1.pop_back();
   }
   result1 -> EndBulkLoad(true);
   //  reconstruction of line
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

void MakeRealm::REALM(CLine* line1, CLine* line2, CLine* result1,
   CLine* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the first line for PlanSweep
   Segment segs[ (line1->Size() + line2->Size()) /2 ];
   int count = 0;
   // insert all segments of line into priority queue
   for (int i=0; i<line1->Size(); i++)   {
      CHalfSegment chs;
      line1->Get(i,chs);
      if (chs.GetLDP() == true and chs.IsDefined() ) {
         PrepareCHS(true, chs, pqueue, segs, count);
         count ++;
      }
   }
   // Prepare the second line for PlaneSweep
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
   //  reconstruction of first line
   result1->Clear();
   result1->StartBulkLoad();
   while (! res1.empty() )  {
      CHalfSegment chs = res1.back();
      chs.SetLDP(true);       result1->InsertHs(chs);
      chs.SetLDP(false);      result1->InsertHs(chs);
      res1.pop_back();
   }
   result1 -> EndBulkLoad(true);
   //  reconstruction of second line
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

/*
function gets a CHalfSegment, creates two XEvents ( both endpoints) and inserts them into priority queue. An ~-Segment~-objects was created and inserted in the array with all segments.

*/

void MakeRealm::PrepareCHS ( bool No1, CHalfSegment& chs,
   PQueue& pqu, Segment segs[], int counter)
{
   Coord xl, xr, yl, yr;
   xl=chs.GetLP().GetX();
   yl=chs.GetLP().GetY();
   xr=chs.GetRP().GetX();
   yr=chs.GetRP().GetY();
   // only left CHalfSegments are used
   // build up array with all CHalfSegments of Region
   Segment se (No1,chs);
   segs[counter] = se;
   // insert XEvents in PQueue
   // insert a vertical Segment
   if ( xl==xr)
   {
      // insert bottom of vertical chs
      XEvent ev1(xl, yl ,counter, 0.0, 0.0, verticalSegment);
      pqu.insert(ev1);
   }
   // insert a non-vertical chs
   else
   { // calculate k and a
#ifdef RATIONAL_COORDINATES
      double k=((yr.IsInteger()? yr.IntValue():yr.Value()) -
                (yl.IsInteger()? yl.IntValue():yl.Value())) /
               ((xr.IsInteger()? xr.IntValue():xr.Value()) -
                (xl.IsInteger()? xl.IntValue():xl.Value())) ;
      double a=(yl.IsInteger()? yl.IntValue():yl.Value()) -
                k* (xl.IsInteger()? xl.IntValue():xl.Value());
#else
      double k = (yr - yl) / (xr - xl) ;
      double a = yl - k*xl;
#endif
      // insert left end of chs
      XEvent ev2(xl, yl, counter, k, a, leftSegment);
      pqu.insert(ev2);
       // insert right end of chs
      XEvent ev3(xr, yr, counter, k, a, rightSegment);
      pqu.insert(ev3);
   }
}

/*
This function performs plane-sweep of realmisation process. The priority queue (~PQueue~) was handle until all XEvents are passed.

*/
void MakeRealm::PerformPlaneSweep(PQueue& pq, Segment segs[],
   list<CHalfSegment>& list1, list<CHalfSegment>& list2,
   const int counter)
{
   // initalisations
   set<int> mi;
   Point oldP;
   VList vlist;
   VStructure vs;
   StatusLine sl;
   if ( pq.isEmpty())  return;
   Coord sweepline, oldsweep;
   // work out the ~PQueue~-object
   while (! pq.isEmpty())  {
      // take the first XEvent
      XEvent event = pq.getFirstAndDelete();
      if (sweepline != event.GetX())  { // new sweepline
         oldsweep = sweepline;
      	 // handle all vertical CHalfSegments at old sweepline
         if ( !vlist.IsEmpty() ) {
	    list<Segment> newlist = vlist.processAll(sweepline,vs,
	       sl,segs);
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
      Segment seg = segs[event.GetFirst()]; // get segment - entry
      // XEvent for left end of CHalfSegment
      SSSEntry entry (event.GetFirst(),event.GetSlope(), event.GetA());
      if (event.GetKind() == leftSegment)   {
         sl.Insert(event.GetX(),entry, segs, pq);
         if ( vs.IsDefined())   vs.Insert(event.GetY());
      }
      // XEvent for right end of CHalfSegment
      else if (event.GetKind() == rightSegment) {
         sl.Delete(event.GetX(), oldsweep, entry, segs, pq);
	 if ( vs.IsDefined())   vs.Insert(event.GetY());
	 segs[event.GetFirst()].CHSInsert(list1,list2);
      }
      // XEvent for split one CHalfSegment
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
	 if (oldP.IsDefined() && p.GetX()== oldP.GetX() &&
	    p.GetY() == oldP.GetY() ){
	    // multiple intersections in one point
	    Segment seg2 = segs[event.GetSecond()];
	    if (mi.find(event.GetFirst()) == mi.end()) {
	       // split first Segment
	       SSSEntry entry1 (event.GetFirst(), segs);
	       sl.Delete(event.GetX(), oldsweep, entry1, segs, pq);
	       Segment new1 ( seg.GetIn1(), seg.GetCHS());
	       new1.SetRP(p);
	       new1.CHSInsert(list1,list2);
	       seg.SetLP(p);	segs[event.GetFirst()] = seg;
	       mi.insert (event.GetFirst() );
	       SSSEntry entry2(event.GetFirst(), segs);
               sl.Insert(event.GetX(), entry2, segs, pq);
	    }
	    if ( mi.find(event.GetSecond()) == mi.end()  ){
	       // split second segment
	       SSSEntry entry1 (event.GetSecond(), segs);
	       sl.Delete(event.GetX(), oldsweep, entry1, segs, pq);
	       Segment new2 (seg2.GetIn1(),seg2.GetCHS());
	       new2.SetRP(p);
	       new2.CHSInsert(list1,list2);
	       seg2.SetLP(p);	segs[event.GetSecond()] = seg2;
	       mi.insert (event.GetSecond());
	       SSSEntry entry2(event.GetSecond(),segs);
               sl.Insert(event.GetX(),entry2, segs, pq);
	    }
	 }
	 else {  // first intersection-event in this point
            Segment seg2 = segs[event.GetSecond()];
	    Segment new1 ( seg.GetIn1(), seg.GetCHS());
	    new1.SetRP(p);
	    Segment new2 (seg2.GetIn1(),seg2.GetCHS());
	    new2.SetRP(p);
	    new1.CHSInsert(list1,list2);
	    new2.CHSInsert(list1,list2);
	    CHalfSegment chstest(seg.GetCHS());
	    seg.SetLP(p);	segs[event.GetFirst()] = seg;
	    seg2.SetLP(p);	segs[event.GetSecond()] = seg2;
	    // first Intersection in this event-point
	    oldP = p;
	    mi.clear();
	    mi.insert(event.GetFirst());
	    mi.insert(event.GetSecond() );
	    SSSEntry entry1(event.GetFirst(), segs);
	    BinTreeNode<SSSEntry>* node1 =
	       sl.Find(sweepline, oldsweep, entry1, segs );
	    BinTreeNode<SSSEntry>* node2 = node1->GetNext();
	    if (node2 !=0 &&
	       node2->GetEntry().GetSeg() == event.GetSecond() ) {}
	    else node2 = node1->GetPred();
	    sl.Exchange(node1, node2, segs, pq);
	 }
      }
      // XEvent - bottom of a vertical Segment
      else if (event.GetKind() == verticalSegment)  {
        vs.SetDefined(true);      // build up VStructure
	vlist.Insert(segs[event.GetFirst()]);
      }
      else  {   cout << " wrong eventkind !!!!!!!!!!!!!!!!!!!!"; }
   }
   sl.Clear();
}
/*
4 Plane-Sweep-Algorithmen for Operators

In the plane-sweep-algorithm for operators the operation was implemented as described in the ROSE-Algebra.With the realm-based results a Plane-sweep-algorithm computs the new sets for operator  ~union-new~, ~minus-new~ and ~intersection-new~. Predikats ~intersects-new~ and ~p-inter~ are new implemented with same semantic like operators in the SpatialALgebra.
Results for different typs of arguments for each operator was created as described at "A Foundation for representing and querying moving objects" G[ue]ting et. al (2000). Only function was implemented, if a new result was received

*/

/*
4.1 Class SEntry

class for representing entries in the Sweepline-Status-Struktur. Each entry store the CHalfsegment and the segment classsifikation for this segment.

*/
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
   int Less (const SEntry ev2, const Coord x, const SEntry oldev,
       BinTreeNode<SEntry>* oldnode ) const;
   bool Equal (const SEntry in2) const;
   //SEntry& operator= (const SEntry& in);

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
               <<") ("<< en.GetCHS().GetLP() << " "
	       << en.GetCHS().GetRP() <<")  u=" << en.GetU()
	       << "  o=" <<en.GetO() );
}

/*
4.1.1 Construktors

*/
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

//SEntry& SEntry::operator= (const SEntry& in)  { return *this;}

/*
4.2.2 Functions for setting property values

*/
void SEntry::Set(const SEntry& in) {
   ch = in.ch;
   slope = in.slope;
   a = in.a;
   o = in.o;
   u = in.u;
}

void SEntry::SetU (int newU)		{ u = newU; }
void SEntry::SetO (int newO) 		{ o = newO; }

/*
4.2.3 Functions for reading property values

*/
CHalfSegment SEntry::GetCHS() const	{return ch;}
int SEntry::GetU() const	     	{return u;}
int SEntry::GetO() const	     	{return o;}
double SEntry::GetSlope() const		{return slope;}
double SEntry::GetA() const		{return a;}

/*
4.2.4 Functions to compare two entries

*/
/*
calculate the y-value at the sweep  line, to insert at the right order

*/
const double SEntry::GetY(Coord x) const  {
   Coord res;
   bool end = false;
   if (ch.GetLP().GetX() == x)
      { end = true; res = ch.GetLP().GetY(); }
   else if (ch.GetRP().GetX() == x)
      { end = true; res = ch.GetRP().GetY(); }
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
      return ( slope*xv + a);
   }
}

bool SEntry::Equal (const SEntry in2) const
{
   if (GetCHS() == in2.GetCHS() )  return true;
   else { return false;}
}

/*
Function used from template ~BinTree~.
Input: SEntry, which is to be inserted od to be deleted
result: -1 means that the new SEntry < SEntry in tree
         0 means that both SEntry are the same
	+1 means that the new SEntry > SEntry in tree

*/
int SEntry::Less (const SEntry in2, const Coord x, const
   SEntry oldev, BinTreeNode<SEntry>* oldnode) const
{
 //  cout << " test Sentry in Less   1. CHS:" << GetCHS() << "     2.CHS: " << in2.GetCHS() << endl; ;
   if ( Equal(in2) )     return 0;
   if (GetCHS().GetLP().GetX() == x &&
      in2.GetCHS().GetLP().GetX() == x) {
   //   cout << " beide starten am gleichen punkt " << endl;
      double y1 = GetCHS().GetLP().GetY();
      double y2 = in2.GetCHS().GetLP().GetY();
      // left point of both segments start at same x-value
      if ( y1 == y2 ) { // they start at same point
         if (GetSlope() < in2.GetSlope())        return -1;
         else if (GetSlope() > in2.GetSlope() )  return 1;
         else    return 1;
      }
      else { // they start at different point
         if ( (y1-y2) > 0.000001 || (y1-y2) < -0.000001) {
            if ((y1-y2) < 0)           return -1;
            else if ( (y1-y2) > 0 )    return 1;
         }
	 else {  //the point are very close
	    // tests if new CHS is in same cycle as CHS
	    // from same object inserted before
	    if (GetCHS().attr.faceno==oldev.GetCHS().attr.faceno &&
	    GetCHS().attr.cycleno==oldev.GetCHS().attr.cycleno &&
	    ((GetCHS().attr.edgeno-oldev.GetCHS().attr.edgeno)==1 ||
	    (GetCHS().attr.edgeno-oldev.GetCHS().attr.edgeno)==-1))
	    {
	       if  (oldnode!=0 &&  oldnode->GetEntry().GetCHS()==
	          in2.GetCHS())    return 1;
	       else if (oldnode!=0 && oldnode->GetPred()!=0 &&
	       oldnode->GetPred()->GetEntry().GetCHS()==in2.GetCHS())
	          return 1;
	    }
	    if (GetSlope() < in2.GetSlope())    return -1;
	    else  return 1;
         } // else test < 0.00001
       } // else kommt vor
   } // if starts same koordinate
   else if (GetCHS().GetLP().GetX() == x) {
 //     cout << " einzufuegendes startet an x-Koordinate " ;
      double test = GetCHS().GetLP().GetY() - in2.GetY(x);
      if ( test > 0.000001 || test < -0.000001) {
   //      cout << " testet y-Vergleich" << endl;
         if (test < 0) 	   	 return -1;
         else if ( test > 0 )	 return 1;
      }
      else {  //  |test| < 0.000001 very close
 //        cout << " very close" ;
	 if (GetCHS().attr.faceno == oldev.GetCHS().attr.faceno &&
	 GetCHS().attr.cycleno == oldev.GetCHS().attr.cycleno &&
	 ((GetCHS().attr.edgeno-oldev.GetCHS().attr.edgeno)==1 ||
	 (GetCHS().attr.edgeno-oldev.GetCHS().attr.edgeno)==-1)) {
//	    cout << " an alten segmenten anhaengen " << endl;
//	    if (oldnode !=0 && oldnode->GetEntry().GetCHS() !=0)
//	       cout <<" alter Knoten "<< oldnode->GetEntry().GetCHS() ;
//	    if (oldnode != 0 && oldnode->GetPred() != 0 )
//	       cout << "  vorgänger von oldnode " <<
//	       oldnode->GetPred()->GetEntry().GetCHS();
//	    cout << endl;
	    if  (oldnode!=0 &&  oldnode->GetEntry().GetCHS()
	        ==in2.GetCHS())   return 1;
	    else if (oldnode != 0 && oldnode->GetPred() != 0 &&
	       oldnode->GetPred()->GetEntry().GetCHS() == in2.GetCHS())
	       return 1;
	    else if (oldnode == 0)  return 1;
	 }
//	 cout << " keine alten vorhanden" << endl;
	 if (GetSlope() < in2.GetSlope())  {//cout << " slope < " <<     endl;
	 return -1;}
	 else  return 1;
      } // |test|  < 0.0001
   }  // else
   else if ( GetCHS().GetRP().GetX()==x &&
      in2.GetCHS().GetRP().GetX() ==x ){
//      cout << " beide enden an x-koordinate " << endl;
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
//      cout << " beide y-werte gleich " << endl;
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
4.2 class SLine -

Class implements Status-Line-Status-Structure. It uses template ~BinTree~

*/
class SLine
{
public:
   SLine();
   ~SLine() { qu.Clear(); }
   BinTreeNode<SEntry>* Insert (SEntry& in, const Coord x,
      SEntry& oldEntry, BinTreeNode<SEntry>* &oldnode);
   bool IsEmpty();
   Coord GetX();
   void SetX(const Coord newx);
   void Delete(SEntry& in, const Coord sx);
   void Delete (BinTreeNode<SEntry>* node);
   SEntry FindAndDelete(SEntry& en, const Coord& x, const Coord&
      oldx, SEntry& oldEntry, BinTreeNode<SEntry>* &oldnode);
   void SLineOutput(const Coord x);
   int GetSize();

private:
   BinTree<SEntry> qu;
   Coord x;
   SEntry First();
   int cmp (const SEntry* in1, const SEntry* in2) const;
   bool Equal  (const SEntry* ev1, const SEntry* in2) const;

};

/*
4.2.1 Initialisation and reading and setting property values

*/
SLine::SLine()  			{BinTree<SEntry> qu;}
bool SLine::IsEmpty()			{ return (qu.IsEmpty() ); }
Coord SLine::GetX()			{ return x; }
int SLine::GetSize()			{return qu.GetCount(); }
void SLine::SetX(const Coord newx)	{ x = newx; }

BinTreeNode<SEntry>* SLine::Insert (SEntry& in, const Coord x,
   SEntry& oldEntry, BinTreeNode<SEntry>* &oldnode)   {
   BinTreeNode<SEntry>* node = qu.Insert(in, x, oldEntry, oldnode);
   oldEntry.Set(in);
   oldnode = node;
   return node;
}

/*
4.2.2 Insert and Delete entries from sweep line

*/
void SLine::Delete (BinTreeNode<SEntry>* node)   {
   qu.DeleteNode (node);
}

SEntry SLine::FindAndDelete(SEntry& en, const Coord& x, const
   Coord& oldx, SEntry& oldEntry, BinTreeNode<SEntry>* &oldnode) {
   BinTreeNode<SEntry>* node = qu.Find(en,x, oldEntry, oldnode);
   if ( node == 0) {
       BinTreeNode<SEntry>* node2 = qu.Find(en, oldx, oldEntry,
          oldnode );
       node = node2;
       if (node == 0) { // nothing found because of rounding
          node2 = qu.GetFirst();
	  while (node2 != 0 && node2->GetEntry().GetCHS()
	     != en.GetCHS()  )
	     node2 = node2->GetNext() ;
	  node = node2;
       }
   }
   if (node != 0) {
      SEntry entry = node->GetEntry();
      oldEntry.Set(entry);
      oldnode = 0;
      qu.DeleteNode(node);
      return entry;
   }
   return 0;
}

void SLine::SLineOutput(const Coord x)
{
   cout << " in SLine size:" << GetSize()<< "   bei x:"<< x<< endl;
   BinTreeNode<SEntry>* node = qu.GetFirst();
   if ( !qu.IsEmpty() ) {
      while (node != 0 )  {
         SEntry ev = node->GetEntry();
	 CHalfSegment chs = ev.GetCHS();
	 double abstand = ((chs.GetLP().GetX() - chs.GetRP().GetX())*
             (chs.GetLP().GetX() - chs.GetRP().GetX()) ) +
	     ((chs.GetLP().GetY() - chs.GetRP().GetY()) *
	     (chs.GetLP().GetY() - chs.GetRP().GetY())) ;
         cout <<" y:" << ev.GetY(x)
	 << "  SEntry: " << ev.GetCHS()
	      << "  U:" << ev.GetU() << "   o:" << ev.GetO()
	      << "  slope:" << ev.GetSlope()
	      << " Abstand LP-RP: " << abstand
	      << endl;
         node = node->GetNext();
      }
   }
   cout << " in output fertig " << endl;
}

/*
4.3 class MakeOp

This class execute the plane-sweep algorithm. For each operator three functions are implemented. One for each combination of arguments line - region.

The algorithmen computes segment classifikations like described in ROSE-Algebra. Then for each operator different criteria are used to find the right segments to build up the result.
For both arguments the first segment was selected. If this segment is a left segment, it was inserted in sweep line. The segment classification was computed. If the segment belongs to a line, the segment was deleted at once. If the selected segment was a right segment of a region, it is deleted from the sweep line. If the segment classification has the searched vlaues it is added into result.

*/
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

/*
4.3.1 Operator intersection-new

If the segment classifikation contains the number 2, the segment is added to result.

*/
/*
intersection-operator for two region-objects

*/
CRegion* MakeOp::Intersection(CRegion* reg1, CRegion* reg2)
{
  // first Realmisation of both regions
   CRegion* res1  = new CRegion(0);
   CRegion* res2 = new CRegion(0);
   MakeRealm mr;
   mr.REALM( reg1, reg2, res1 , res2 );
   // initialisations
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
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size() ) {
     // select the first segment
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()==chs2.GetLP() && chs1.GetRP()==chs2.GetRP()){
         i ++;   j ++;   chsAkt = chs1;    status = BOTH;
      }
      else if ( chs1 < chs2) {i ++; chsAkt = chs1; status = FIRST;}
      else if (chs1 > chs2) {j ++; chsAkt = chs2; status = SECOND;}
 if (chsAkt.GetDPoint().GetX() != aktSweep)
// cout << " ======== neue Sweep ====== " << endl;
// cout << " status = " << status << endl;
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 SEntry en;
	 // delete segment from sweep line
	 if (status == FIRST)  {
	    SEntry in = sweepline.FindAndDelete(ent,aktSweep,
	    oldSweep, oldEntry1, oldnode1);
	    en.Set(in);
	 }
	 else {
	    SEntry in = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
	    oldEntry2, oldnode2);
	    en.Set(in);
	 }
	 if (en.GetO() == 2 || en.GetU() == 2) {
	    // add segment to result?
//  cout << " Ausgabe nr. " << counter << " chs " << chsAkt;
	    chsAkt.attr.partnerno = counter;
            chsAkt.SetLDP(false);	(*result) += chsAkt;
	    chsAkt.SetLDP(true);	(*result) += chsAkt;
	    counter++;
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* node;
	 // add segment into sweepline-Status-Structure
	 if (status == FIRST)
	  node = sweepline.Insert(ent,aktSweep,oldEntry1,oldnode1);
	 else
	  node = sweepline.Insert(ent,aktSweep,oldEntry2,oldnode2);
	 // get predecessor of inserted entry
	 BinTreeNode<SEntry>* pred = node->GetPred() ;
	 int np = 0;
	 int ms, ns;
	 if (pred != 0) np = pred->GetEntry().GetO();
	 // calculate new segmentclass of new ChalfSegment
	 ms = np;    ns = np;
	 if (status == FIRST || status == BOTH) {
	    if (chs1.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }
	 if (status == SECOND || status == BOTH) {
	    if (chs2.attr.insideAbove == true)	ns = ns+1;
	    else 				ns = ns-1;
	 }  // set new segmentclasses in SEntry
	 ent.SetU(ms);
	 ent.SetO(ns);
	 node->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
// sweepline.SLineOutput(aktSweep);
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   result->EndBulkLoad();
   result->SetPartnerNo();
   result->ComputeRegion();
   return result;
}

 CLine* MakeOp::Intersection(CRegion* reg, CLine* line)
 {
  // first Realmisation of both arguments
   CLine* resline  = new CLine(0);
   CRegion* resregion = new CRegion(0);
   MakeRealm mr;
   mr.REALM( line, reg, resline , resregion );
   // initialisations
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
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   // execute until one argument is empty
   while ( i < resline->Size() && j < resregion->Size() ) {
      // select the first segment
      resline->Get(i,chs1);
      resregion->Get(j,chs2);
      if (chs1.GetLP()==chs2.GetLP() && chs1.GetRP()==chs2.GetRP()){
         i ++;   j ++;   chsAkt = chs2;    status = BOTH; }
      else if ( chs1 < chs2) {i ++; chsAkt = chs1; status = FIRST;}
      else if (chs1 > chs2) {j ++; chsAkt = chs2; status = SECOND;}
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         if (status == SECOND || status == BOTH) {
            chsAkt.SetLDP(true);
            SEntry ent (chsAkt);
	    // delete segment from sweep line
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep,
	       oldSweep, oldEntry1, oldnode1);
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 // add segment into sweepline-Status-Structure
	 BinTreeNode<SEntry>* node = sweepline.Insert(ent,
	    aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = node->GetPred() ;
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
	    node->SetEntry(ent);
	 }
	 if (status == FIRST || status == BOTH) {
	    if (pred != 0 && pred->GetEntry().GetO() > 0)
	       (*result) += chsAkt;
	    // segments from line are deleted at once
	    if (status == FIRST) sweepline.Delete(node);
	 }
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   resline->Destroy(); delete resline;
   resregion->Destroy(); delete resregion;
   result->EndBulkLoad();
  // cout << " ===========================result fertig ========" << endl;
   return result;
}


CLine* MakeOp::Intersection(CLine* line1, CLine* line2)
{
  // first Realmisation of both lines
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   // initialisations
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   CLine* result = new CLine(0);
   result ->Clear();
   result->StartBulkLoad();
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      // segments, which are the same in both arguments are added
      // in result
      if (chs1.GetLP()==chs2.GetLP() && chs1.GetRP()==chs2.GetRP())
	 {  i ++;   j ++;   (*result) += chs1; }
      else if ( chs1 < chs2)  i ++;
      else if (chs1 > chs2)   j ++;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
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
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()==chs2.GetLP() && chs1.GetRP()==chs2.GetRP()){
         i ++;   j ++;   chsAkt = chs1;    status = BOTH;      }
      else if ( chs1 < chs2) {i ++; chsAkt = chs1; status = FIRST;}
      else if (chs1 > chs2) {j ++; chsAkt = chs2; status = SECOND;}
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 // delete segment from sweep line
	 if (status == FIRST)
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep,oldSweep,
	       oldEntry1, oldnode1);
	  else SEntry en = sweepline.FindAndDelete(ent,aktSweep,
	     oldSweep, oldEntry2, oldnode2);
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* node;
	 // add segment into sweepline-Status-Structure
	 if (status == FIRST)
	    node=sweepline.Insert(ent,aktSweep,oldEntry1,oldnode1);
	 else
	    node=sweepline.Insert(ent,aktSweep,oldEntry2,oldnode2);
	 BinTreeNode<SEntry>* pred = node->GetPred() ;
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
	 // if one intersection is found, break and return true
         if (ms == 2 || ns == 2) { return true; }
	 // set new segmentclasses in SEntry
	 ent.SetU(ms);
	 ent.SetO(ns);
	 node->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   return false;
}

bool MakeOp::P_Intersects(CRegion* reg, CLine* line)
{
  // first Realmisation of both arguments
   CLine* resline  = new CLine(0);
   CRegion* resregion = new CRegion(0);
   MakeRealm mr;
   mr.REALM( line, reg, resline , resregion );
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   // execute until one argument is empty
   while ( i < resline->Size() && j < resregion->Size() ) {
     // select_ first
      resline->Get(i,chs1);
      resregion->Get(j,chs2);
      if (chs1.GetLP()==chs2.GetLP()&&chs1.GetRP()==chs2.GetRP()){
         i ++; j ++; chsAkt = chs2; status = BOTH;
      }
      else if ( chs1 < chs2) {i ++; chsAkt = chs1; status = FIRST;}
      else if (chs1 > chs2) {j ++; chsAkt = chs2; status = SECOND;}
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         if (status == SECOND || status == BOTH) {
            chsAkt.SetLDP(true);
            SEntry ent (chsAkt);
	    // delete segment from sweep line
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep,
	       oldSweep, oldEntry1, oldnode1);
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 // add segment into sweepline-Status-Structure
	 BinTreeNode<SEntry>* node = sweepline.Insert(ent,aktSweep,
	    oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = node->GetPred() ;
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
	    node->SetEntry(ent);
	 }
	 if (status == FIRST || status == BOTH) {
	    if ( pred != 0  && pred->GetEntry().GetO() > 0 )
	       { return true; }
	    // segments from line are deleted at once
	    if (status == FIRST) sweepline.Delete(node);
	 }
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   resline->Destroy(); delete resline;
   resregion->Destroy(); delete resregion;
   return false;
}

bool MakeOp::P_Intersects(CLine* line1, CLine* line2)
{
     // first Realmisation of both lines
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   // initialisations
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      // if two segments are the same -> return true
      if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() )
         { return true; }
      else if ( chs1 < chs2)  i ++;
      else if (chs1 > chs2)   j ++;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   return false;
}


bool MakeOp::Intersects(CRegion* reg1, CRegion* reg2)
{
  // first Realmisation of both regions
   CRegion* res1  = new CRegion(0);
   CRegion* res2 = new CRegion(0);
   MakeRealm mr;
   mr.REALM( reg1, reg2, res1 , res2 );
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()==chs2.GetLP()||chs1.GetLP()==chs2.GetRP()
       || chs1.GetRP()==chs2.GetLP()||chs1.GetRP()==chs2.GetRP() )
         return true;
      else if ( chs1 < chs2) {i ++; chsAkt = chs1; status = FIRST;}
      else if (chs1 > chs2) {j ++; chsAkt = chs2;  status = SECOND;}
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 // delete segment from sweep line
	 if (status == FIRST)
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
	       oldEntry1, oldnode1);
	 else SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
	       oldEntry2, oldnode2);
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* node;
	 // add segment into sweepline-Status-Structure
	 if (status == FIRST)
	    node = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 else
	    node = sweepline.Insert(ent,aktSweep, oldEntry2, oldnode2);
	 BinTreeNode<SEntry>* pred = node->GetPred() ;
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
	 node->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   return false;
}

bool MakeOp::Intersects (CRegion* reg, CLine* line) {
  // first Realmisation of both arguments
   CRegion* res1  = new CRegion(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line, reg, res2 , res1 );
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2, chsAkt;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()==chs2.GetLP()||chs1.GetLP()==chs2.GetRP()||
          chs1.GetRP()==chs2.GetLP()||chs1.GetRP()==chs2.GetRP() )
	  return true;
      else if ( chs1 < chs2) {i ++; chsAkt = chs1; status = FIRST;}
      else if (chs1 > chs2) {j ++; chsAkt = chs2; status = SECOND;}
      aktSweep = chsAkt.GetDPoint().GetX();
      if ( chsAkt.GetLDP() == false) {
         if (status == FIRST) {  // right end of segment of region
            chsAkt.SetLDP(true);
            SEntry ent (chsAkt);
	    // delete segment from sweep line
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep,
	       oldSweep, oldEntry1, oldnode1);
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 // add segment into sweepline-Status-Structure
	 BinTreeNode<SEntry>* node = sweepline.Insert(ent,
	    aktSweep, oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = node->GetPred() ;
	 if (status == FIRST ) {
	    // insert CHalfSegment from region into sweepline
	    int np = 0;
	    int ms, ns;
	    if (pred != 0) np = pred->GetEntry().GetO();
	    // calculate new segmentclass of new ChalfSegment
	    ms = np;
	    ns = np;
	    if (chs1.attr.insideAbove == true)
	       { ent.SetU(0); ent.SetO(1); }
	    else
	       { ent.SetU(1); ent.SetO(0); }
	    node->SetEntry(ent);
         } //  end else
	 else if (status == SECOND) {
	    if (pred != 0)
	       { if (pred->GetEntry().GetO() == 1) return true; }
	    else sweepline.Delete (node);
	 }
      }
      oldSweep = aktSweep ;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   return false;
}

bool MakeOp::Intersects (CLine* line1, CLine* line2)
{
  // first Realmisation of both lines
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   // initialisations
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,chs1);
      res2->Get(j,chs2);
      if (chs1.GetLP()==chs2.GetLP() ||
          chs1.GetLP()==chs2.GetRP() ||
          chs1.GetRP()==chs2.GetLP() ||
	  chs1.GetRP() == chs2.GetRP() ) return true;
      if ( chs1 < chs2)       i ++;
      else if (chs1 > chs2)   j ++;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   return false;
}

CRegion* MakeOp::Union(CRegion* reg1, CRegion* reg2)
{
   // first Realmisation of both regions
   CRegion* res1 = new CRegion(0);
   CRegion* res2 = new CRegion(0);
   MakeRealm mr;
   mr.REALM( reg1, reg2, res1 , res2 );
   // initialisations
   SLine sweepline;
   int i = 0;   int j = 0; int counter = 0;
   CHalfSegment chs1, chs2, chsAkt;
   CRegion* result = new CRegion(0);
   result ->Clear();
   result->StartBulkLoad();
   State status;
   Coord aktSweep, oldSweep ;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   // while there are segments in the realm-based arguments
   while ( i < res1->Size() || j < res2->Size()) {
      // select_first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,chs1);
         res2->Get(j,chs2);
         if (chs1.GetLP()==chs2.GetLP() && chs1.GetRP()==chs2.GetRP() ){
            i ++;   j ++;   chsAkt = chs1;    status = BOTH; }
	 else if ( chs1 < chs2) {i ++; chsAkt = chs1; status = FIRST;}
         else if (chs1 > chs2) {j ++; chsAkt = chs2; status = SECOND;}
      }
      else if (i < res1->Size() ) {
         res1->Get(i,chs1);  i ++;   chsAkt = chs1;  status = FIRST;}
      else if ( j < res2->Size() ) {
         res2->Get(j,chs2);  j ++;   chsAkt = chs2;  status = SECOND;}
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) { // right sement choosen
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 SEntry en;
	 // delete segment from sweep line
	 if (status == FIRST) {
	    SEntry in = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
	    oldEntry1, oldnode1);
	    en.Set(in);
	 }
	 else {
	    SEntry in = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
	    oldEntry2, oldnode2);
	    en.Set(in);
	 }
	 if (en.GetO() == 0 || en.GetU() == 0) {  // CHalfSegment in result
	       chsAkt.attr.partnerno = counter;
               chsAkt.SetLDP(false);	(*result) += chsAkt;
	       chsAkt.SetLDP(true);	(*result) += chsAkt;
	       counter++;
	 }
	 if (status == BOTH) {oldEntry1=oldEntry2; oldnode1=oldnode2;}
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* node;
	 // add segment into sweepline-Status-Structure
	 if (status == FIRST)  node = sweepline.Insert(ent,aktSweep,
	    oldEntry1, oldnode1);
	 else   node = sweepline.Insert(ent,aktSweep, oldEntry2, oldnode2);
	 BinTreeNode<SEntry>* pred = node->GetPred() ;
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
	 node->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
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
  // first Realmisation of both lines
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   // initialisations
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   CLine* result = new CLine(0);
   result ->Clear();
   result->StartBulkLoad();
   // while there are segments in the realm-based arguments
   while ( i < res1->Size() || j < res2->Size()) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,chs1);
         res2->Get(j,chs2);
	 // add same segments only once
         if (chs1.GetLP()== chs2.GetLP() && chs1.GetRP()== chs2.GetRP() )
	    {  i ++;   j ++;   (*result) += chs1; }
	 else if ( chs1 < chs2) { i ++;  (*result) += chs1; }
         else if (chs1 > chs2)  { j ++;  (*result) += chs2; }
      }
      else if (i<res1->Size() )
         { res1->Get(i,chs1);  i ++;  (*result) += chs1; }
      else if ( j < res2->Size() )
         { res2->Get(j,chs2);  j ++;  (*result) += chs2; }
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
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
   // initialisations
   SLine sweepline;
   int i = 0;   int j = 0;  int counter = 0;
   CHalfSegment chs1, chs2, chsAkt;
   CRegion* result = new CRegion(0);
   result ->Clear();
   result->StartBulkLoad();
   State status;
   Coord aktSweep, oldSweep ;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   // while there are segments in the first arguments
   while ( i < res1->Size() ) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,chs1);
         res2->Get(j,chs2);
         if (chs1.GetLP()==chs2.GetLP() && chs1.GetRP()==chs2.GetRP() )
	    {i ++; j ++; chsAkt = chs1;  status = BOTH;    }
         else if ( chs1 < chs2) { i ++; chsAkt = chs1; status = FIRST; }
         else if (chs1 > chs2) { j ++; chsAkt = chs2; status = SECOND; }
      }
      else if (i < res1->Size() ) {
         res1->Get(i,chs1);  i ++;   chsAkt = chs1;  status = FIRST;
      }
      else if ( j < res2->Size() ) {
         res2->Get(j,chs2);  j ++;   chsAkt = chs2;  status = SECOND; }
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // right end of segment
         chsAkt.SetLDP(true);
         SEntry ent (chsAkt);
	 SEntry en;
	 // delete segment from sweep line
	 if (status == FIRST) {
	    SEntry in = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
	       oldEntry1, oldnode1);
	    en.Set(in);
	 }
	 else {
	    SEntry in = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
	       oldEntry2, oldnode2);
	    en.Set(in);
	 }
	 if (status == BOTH)
	    {oldEntry1 = oldEntry2, oldnode1 = oldnode2;}
         if ( (status == FIRST && en.GetU() == 0 && en.GetO() == 1) ||
	    (status == FIRST && en.GetU() == 1 && en.GetO() == 0) ||
	    (status == BOTH && en.GetU() == 1 && en.GetO() == 1)) {
	    chsAkt.attr.partnerno = counter;
            chsAkt.SetLDP(false);	(*result) += chsAkt;
	    chsAkt.SetLDP(true);	(*result) += chsAkt;
	    counter++;
	 }
	 else if ( (status == SECOND && en.GetU()==1 && en.GetO()==2)||
	 (status == SECOND && en.GetU() == 2 && en.GetO() == 1) ) {
	    // attr insideABove must be changed for segments from 2nd
	    chsAkt.attr.insideAbove = ! chsAkt.attr.insideAbove ;
	    chsAkt.attr.partnerno = counter;
            chsAkt.SetLDP(false);	(*result) += chsAkt;
	    chsAkt.SetLDP(true);	(*result) += chsAkt;
	    counter++;
	 }
      }
      else { // left end of segment
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* node;
	 // add segment into sweepline-Status-Structure
	 if (status == FIRST)
	    node = sweepline.Insert(ent,aktSweep, oldEntry1, oldnode1);
	 else
	    node = sweepline.Insert(ent,aktSweep, oldEntry2, oldnode2);
	 if (status == BOTH) {oldEntry1 = oldEntry2, oldnode1 = oldnode2;}
	 BinTreeNode<SEntry>* pred = node->GetPred() ;
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
	 node->SetEntry(ent);
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   result->EndBulkLoad();
   result->SetPartnerNo();
   result->ComputeRegion();
   return result;
}

CLine* MakeOp::Minus(CLine* line, CRegion* reg)
{   // first Realmisation of both arguments
   CLine* resLine = new CLine(0);
   CRegion* resReg = new CRegion(0);
   MakeRealm mr;
   mr.REALM( line, reg, resLine , resReg );
   // initialisations
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
    // while there are segments in the first arguments
   while ( i < resLine -> Size() ) {
     // select_ first
      if (i < resLine -> Size() && j < resReg -> Size() ) {
         resLine -> Get(i,chs1);
         resReg -> Get(j,chs2);
         if (chs1.GetLP()==chs2.GetLP() && chs1.GetRP()==chs2.GetRP()){
            i ++;   j ++;   chsAkt = chs2;    status = BOTH;  }
         else if ( chs1 < chs2) { i ++; chsAkt = chs1; status = FIRST;}
         else if (chs1 > chs2) { j ++; chsAkt = chs2; status = SECOND;}
      }
      else if (i < resLine ->Size() ) {
         resLine -> Get(i,chs1); i ++; chsAkt = chs1; status = FIRST;}
      aktSweep = chsAkt.GetDPoint().GetX();
      if (chsAkt.GetLDP() == false) {  // delete right end
	 if ( status == SECOND || status == BOTH ) {
            chsAkt.SetLDP(true);
            SEntry ent (chsAkt);
	    // delete segment from sweep line
	    SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
	       oldEntry1, oldnode1);
	 }
      }
      else { // chsAkt.GetLDP() == true
         SEntry ent (chsAkt);
	 BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep,
	    oldEntry1, oldnode1);
	 BinTreeNode<SEntry>* pred = en->GetPred() ;
	 if (status == FIRST ) {
	    if (pred == 0 || (pred!=0 && pred->GetEntry().GetO()==0) )
	    {
	       (*result) += chs1;    chs1.SetLDP(false);
	       (*result) += chs1;
	    }
            sweepline.Delete (en) ;
	 }
	 else {  // status == SECOND or BOTH
	    if (chsAkt.attr.insideAbove == true)
	       { ent.SetU(0); ent.SetO(1); }
	    else
	       { ent.SetU(1); ent.SetO(0); }
	    en->SetEntry(ent);
	 }
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   resLine->Destroy(); delete resLine;
   resReg->Destroy(); delete resReg;
   result->EndBulkLoad();
   return result;

}

CLine* MakeOp::Minus(CLine* line1, CLine* line2)
{
  // first Realmisation of both lines
   CLine* res1 = new CLine(0);
   CLine* res2 = new CLine(0);
   MakeRealm mr;
   mr.REALM( line1, line2, res1 , res2 );
   // initialisations
   int i = 0;
   int j = 0;
   CHalfSegment chs1, chs2;
   CLine* result = new CLine(0);
   result ->Clear();
   result->StartBulkLoad();
    // while there are segments in the first arguments
   while ( i < res1->Size() ) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,chs1);
         res2->Get(j,chs2);
         if (chs1.GetLP()==chs2.GetLP() && chs1.GetRP()==chs2.GetRP() )
	    { i ++;  j ++; }
         else if ( chs1 < chs2) { i ++;  (*result) += chs1;  }
         else if (chs1 > chs2)  j ++;
      }
      else if (i < res1->Size() ) {
         res1->Get(i,chs1);
	 i ++;
	 (*result) += chs1;
      }
   }
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   result -> EndBulkLoad() ;
   return result;
}

/*
5 Operators

For each operator two arguments was first tested, if their boundingBox intersects each other. If they intersects the realmisation process was invoked and result was computes with class MakeOp. If they do not intersects, the result was computed without class MakeOp.

*/
enum NewSpatialType {stpoint,stpoints,stline,stregion,stbox,sterror};

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

/*
5.1 Realm-Test-Operator

only for testings, operator gets two arguments and creates the first argument as realm-based object as result

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
   }
   return (nl->SymbolAtom( "typeerror" ));
}

const string RealmSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text> (line region)x(line region)->line/region </text--->"
  "<text>REALM(_,_)</text--->"
  "<text>Returns the first object as REALMed object.</text--->"
  "<text>query REALM ( line1, region2 )</text--->"
    ") )";

/*
realmisation of a line and a region-object

*/
static int realm_lr( Word* args, Word& result, int message,
   Word& local, Supplier s )
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
      result.addr = test1;
      return(0);
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
realmisation a region and a line - object

*/

static int realm_rl( Word* args, Word& result, int message,
   Word& local, Supplier s )
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
      result.addr = test2;
      return(0);
   }
   else  {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
realmisation two line-objects

*/
static int realm_ll( Word* args, Word& result, int message,
   Word& local, Supplier s )
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
      result.addr = test1;
      return(0);
   }
   else
   {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
realmisation two region-objects

*/
static int realm_rr( Word* args, Word& result, int message,
   Word& local, Supplier s )
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
      result.addr = test1;
      return(0);
   }
   else
   {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}

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
        ( "realm", RealmSpec, 4, RealmMap, 
          realmSelectCompute, realmMap);

/*
5.2 Operator Intersection

*/

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

/*
operator intersection for objects line -line

*/
static int Inter_ll( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() ) {
      if (line1->IsEmpty() || line2->IsEmpty() ) {
          ((CLine *)result.addr)->SetDefined( false );
         return (0);
      }
      else if (line1->BoundingBox().IsDefined() &&
         line2->BoundingBox().IsDefined() ) {
         if (line1->BoundingBox().Intersects(line2->BoundingBox())) {
            CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Intersection( line1, line2 );
	    if ( res->IsEmpty() )
	       ((CLine *)result.addr)->SetDefined( false );
            else
	       result.addr = res;
            return(0);
         }
         else   {
	    ((CLine *)result.addr)->SetDefined( false );
            return (0);
         }
      }
      else  {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Intersection( line1, line2 );
	 if ( res->IsEmpty() )
	    ((CLine *)result.addr)->SetDefined( false );
         else
	    result.addr = res;
         return(0);
      }
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
operator intersection for objects line -region

*/
static int Inter_lr( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
      if (l1->IsEmpty() || r2->IsEmpty() ) {
         ((CLine *)result.addr)->SetDefined( false );
         return (0);
      }
      if (l1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox())) {
            CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Intersection( r2, l1 );
	    if (res->IsEmpty() )
	      ((CLine *)result.addr)->SetDefined( false );
            else
	      result.addr = res ;
            return(0);
         }
         else   {
	    ((CLine *)result.addr)->SetDefined( false );
            return (0);
         }
      }
      else  {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Intersection( r2, l1  );
	 if (res->IsEmpty() )
	    ((CLine *)result.addr)->SetDefined( false );
         else
	    result.addr = res;
         return(0);
      }
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
operator intersection for objects region -line

*/
static int Inter_rl ( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[1].addr);
   CRegion *r2 = ((CRegion*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
      if (l1->IsEmpty() || r2->IsEmpty() ) {
         ((CLine *)result.addr)->SetDefined( false );
         return (0);
      }
      if (l1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Intersection( r2, l1 );
	    if (res->IsEmpty() )
	       ((CLine *)result.addr)->SetDefined( false );
            else
	       result.addr = res;
            return(0);
         }
         else   {
	    ((CLine *)result.addr)->SetDefined( false );
            return (0);
         }
      }
      else  {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Intersection( r2, l1  );
	 if (res->IsEmpty() )
	    ((CLine *)result.addr)->SetDefined( false );
         else
	    result.addr = res;
         return(0);
      }
   }
   else  {
       ((CLine *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
operator intersection for objects region - region

*/
static int Inter_rr( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
      if (r1->IsEmpty() || r2->IsEmpty() ) {
         ((CRegion *)result.addr)->SetDefined( false );
          return (0);
      }
      if (r1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if ( r1->BoundingBox().Intersects( r2->BoundingBox())) {
            CRegion* res = new CRegion(0);
            MakeOp mo;
            res = mo.Intersection( r1, r2 );
	    if ( res->IsEmpty() )
	       ((CRegion *)result.addr)->SetDefined( false );
            else
	       result.addr = res;
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
            result.addr = res;
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
   "(<text>(line x line->line) or "
   "(line x region->line) or "
   "(region x line->line) or "
   "(region x region->region)</text--->"
   "<text>intersection_new(_,_)</text--->"
   "<text>Returns the intersection of 2 lines/regions .</text--->"
   "<text>query intersection_new ( region1,region2 )</text--->"
   ") )";


ValueMapping InterValueMap[]={Inter_ll,Inter_lr,Inter_rl,Inter_rr };

int InterSelectCompute ( ListExpr args) {
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )  return (0);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )  return (1);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )  return (2);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )  return (3);
   else return (-1);

}    

Operator Intersectionrr
   ( "intersection_new", InterSpec, 4, InterValueMap,
     InterSelectCompute, InterMap );

/*
5.3 Operator intersects

*/
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

/*
operator intersects for objects line -line

*/
static int Intersects_ll( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() ) {
       if (line1->IsEmpty() || line2->IsEmpty() ) {
         ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (line1->BoundingBox().IsDefined() &&
          line2->BoundingBox().IsDefined() ) {
         if (line1->BoundingBox().Intersects(line2->BoundingBox())) {
            MakeOp mo;
            bool res = mo.Intersects( line1, line2 );
            ((CcBool *)result.addr) ->Set(true,res);
            return(0);
         }
         else   {
            ((CcBool *)result.addr) ->Set(true,false) ;
            return (0);
         }
      } // if BoundingBox.IsDefiend
      else  { // no BoundingBox defined
         MakeOp mo;
         bool res = mo.Intersects( line1, line2 );
         ((CcBool *)result.addr) ->Set(true,res);
         return(0);
      }
   }  // if both IsDefined
   else  {
       ((CcBool *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
operator intersects for objects line -region

*/
static int Intersects_lr( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
       if (l1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set( true, false);
         return (0);
      }
      if (l1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if (l1->BoundingBox().Intersects(r2->BoundingBox())) {
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

/*
operator intersects for objects region -line

*/
static int Intersects_rl( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[1].addr);
   CRegion *r2 = ((CRegion*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
       if (l1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (l1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if (l1->BoundingBox().Intersects(r2->BoundingBox())){
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

/*
operator intersects for objects region - region

*/
static int Intersects_rr( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
      if (r1->IsEmpty() || r2->IsEmpty() ) {
         ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (r1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if (r1->BoundingBox().Intersects(r2->BoundingBox())) {
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
   "( <text>(line region)x(line region)->(line region)</text--->"
   "<text>intersects_new(_,_)</text--->"
   "<text>tests if two lines/regions intersects.</text--->"
   "<text>query intersects_new ( region1,region2 )</text--->"
   ") )";


ValueMapping IntersectsValueMap[] = {Intersects_ll,Intersects_lr,
   Intersects_rl, Intersects_rr };


int IntersectsSelectCompute ( ListExpr args) {
   ListExpr arg1 = nl->First(args);
   ListExpr arg2 = nl->Second(args);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )  return (0);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )  return (1);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stline )  return (2);
   if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
      NewSpatialTypeOfSymbol( arg2 ) == stregion )  return (3);
   else return (-1);
}

Operator Intersectsrr
   ( "intersects_new", IntersectsSpec, 4, IntersectsValueMap,
     IntersectsSelectCompute, IntersectsMap );

/*
5.4 Operator union

*/

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

/*
operator union for objects region - region

*/
static int Union_rr( Word* args, Word& result, int message,
Word& local, Supplier s )
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
         result.addr = r2;
         return(0);
      }
      else if (r2->IsEmpty() ) {
         result.addr = r1;
         return(0);
      }
      else {
         CRegion* res = new CRegion(0);
         MakeOp mo;
         res = mo.Union( r1, r2 );
         result.addr = res;
         return(0);
      }
   }
   else  {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
operator union for objects line -region

*/
static int Union_lr( Word* args, Word& result, int message,
   Word& local, Supplier s )
{

   result = qp->ResultStorage( s );
   CLine *line = ((CLine*)args[0].addr);
   CRegion *reg = ((CRegion*)args[1].addr);
   if ( !reg->IsEmpty()&&line->IsDefined()&&reg->IsDefined()) {
      result.addr = reg;
      return(0);
   }
   else  {
      ((CRegion *)result.addr)->SetDefined( false );
      return (0);
   }
}

/*
operator union for objects region -line

*/
static int Union_rl( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line = ((CLine*)args[1].addr);
   CRegion *reg = ((CRegion*)args[0].addr);
   if ( ! reg -> IsEmpty() && line->IsDefined() &&
   reg->IsDefined() )    {
      result.addr = reg;
      return(0);
   }
   else  {
      ((CRegion *)result.addr)->SetDefined( false );
      return (0);
   }
}

/*
operator union for objects line -line

*/
static int Union_ll( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() )    {
      if (line1->IsEmpty() ) {
         ((CLine *)result.addr)->SetDefined( false );
         return(0);
      }
      else if (line2->IsEmpty() ) {
         result.addr = line1;
         return(0);
      }
      else {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Union( line1, line2 );
         result.addr = res;
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
   "(<text>(line x line->line) or "
   "(line x region->region) or "
   "(region x line->region) or "
   "(region x region->region)</text--->"
   "<text>union_new(_,_)</text--->"
   "<text>Returns the union of 2 lines/regions .</text--->"
   "<text>query union_new ( region1,region2 )</text--->"
   ") )";

ValueMapping UnionValueMap[] =
   { Union_ll, Union_lr, Union_rl, Union_rr };

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
   ( "union_new", UnionSpec, 4, UnionValueMap,
   unionSelectCompute, unionMap );


/*
5.5 Operator minus

*/
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

/*
operator minus for objects region - region

*/
static int Minus_rr( Word* args, Word& result, int message,
   Word& local, Supplier s )
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
         result.addr = r1;
         return(0);
      }
      else if (r1->BoundingBox().IsDefined() &&
      r2->BoundingBox().IsDefined() ) {
         if ((r1->BoundingBox().Intersects(r2->BoundingBox()))) {
            CRegion* res = new CRegion(0);
            MakeOp mo;
            res = mo.Minus( r1, r2 );
	    if (res->IsEmpty() )
	       ((CRegion *)result.addr)->SetDefined( false );
            else
	       result.addr = res;
            return(0);
         }
         else   {
            result.addr = r1;
            return (0);
         }
      }
      else    {
         CRegion* res = new CRegion(0);
         MakeOp mo;
         res = mo.Minus( r1, r2 );
	 if (res->IsEmpty() )
	    ((CRegion *)result.addr)->SetDefined( false );
         else
	    result.addr = res;
         return(0);
      }
   }
   else   {
       ((CRegion *)result.addr)->SetDefined( false );
       return (0);
   }
}

/*
operator minus for objects line -region

*/
static int Minus_lr( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line = ((CLine*)args[0].addr);
   CRegion *reg = ((CRegion*)args[1].addr);
   if (line->IsDefined() && reg->IsDefined() )    {
      if ( line-> IsEmpty()|| reg->IsEmpty() ) {
         ((CLine *)result.addr)->SetDefined( false );
         return(0);
      }
      else if (line->BoundingBox().IsDefined() &&
      reg->BoundingBox().IsDefined() ) {
         if (line->BoundingBox().Intersects(reg->BoundingBox())) {
            CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Minus( line,reg );
	    if ( res -> IsEmpty() )
	       ((CLine *)result.addr)->SetDefined( false );
            else
	       result.addr = res;
            return(0);
         }
         else   {
            result.addr = line;
            return (0);
         }
      }
      else    {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Minus( line, reg );
         if ( res -> IsEmpty() )
	    ((CLine *)result.addr)->SetDefined( false );
         else
	    result.addr = res;
         return(0);
      }
   } // if IsDefined()
   else   {
      ((CLine *)result.addr)->SetDefined( false );
      return (0);
   }
}

/*
operator minus for objects region -line

*/
static int Minus_rl( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line = ((CLine*)args[1].addr);
   CRegion *reg = ((CRegion*)args[0].addr);
   if ( ! line->IsEmpty() && line->IsDefined() &&
   reg->IsDefined() )    {
      result.addr = reg ;
      return(0);
   }
   else  {
      ((CRegion *)result.addr)->SetDefined( false );
      return (0);
   }
 }

/*
operator minus for objects line -line

*/
static int Minus_ll( Word* args, Word& result, int message,
Word& local, Supplier s )
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
         result.addr = line1;
         return (0);
      }
      else if (line1->BoundingBox().IsDefined() &&
      line2->BoundingBox().IsDefined() ) {
         if ((line1->BoundingBox().Intersects
	 (line2->BoundingBox() ) ) ) {
	    CLine* res = new CLine(0);
            MakeOp mo;
            res = mo.Minus( line1, line2 );
	    if (res->IsEmpty() )
	       ((CLine *)result.addr)->SetDefined( false );
            else
	       result.addr = res;
            return(0);
	 }
	 else {
	    result.addr = line1;
	    return(0);
	 }
      }
      else {
         CLine* res = new CLine(0);
         MakeOp mo;
         res = mo.Minus( line1, line2 );
	 if (res ->IsEmpty() )
	    ((CLine *)result.addr)->SetDefined( false );
         else
	    result.addr = res;
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
   "( <text>(line x line->line) or "
   "(line x region->line) or "
   "(region x line->line) or "
   "(region x region->region)</text--->"
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
   ( "minus_new", MinusSpec, 4, MinusValueMap, 
   minusSelectCompute, minusMap );


/*
5.6 Operator p-inter

*/
/*
operator p-inter for objects line -line

*/
static int p_inter_ll( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *line1 = ((CLine*)args[0].addr);
   CLine *line2 = ((CLine*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() ) {
       if (line1->IsEmpty() || line2->IsEmpty() ) {
         ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (line1->BoundingBox().IsDefined() &&
      line2->BoundingBox().IsDefined() ) {
         if (line1->BoundingBox().Intersects(line2->BoundingBox())) {
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


/*
operator p-inter for objects line -region

*/
static int p_inter_lr( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
       if (l1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set( true, false);
         return (0);
      }
      if (l1->BoundingBox().IsDefined() &&
      r2->BoundingBox().IsDefined() ) {
         if (l1->BoundingBox().Intersects(r2->BoundingBox())) {
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

/*
operator p-inter for objects region -line

*/
static int p_inter_rl( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CLine *l1 = ((CLine*)args[1].addr);
   CRegion *r2 = ((CRegion*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
       if (l1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (l1->BoundingBox().IsDefined() &&
      r2->BoundingBox().IsDefined() ) {
         if (l1->BoundingBox().Intersects(r2->BoundingBox())) {
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

/*
operator p-inter for objects region - region

*/
static int p_inter_rr( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   CRegion *r1 = ((CRegion*)args[0].addr);
   CRegion *r2 = ((CRegion*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
       if (r1->IsEmpty() || r2->IsEmpty() ) {
          ((CcBool*) result.addr) ->Set ( true, false);
         return (0);
      }
      if (r1->BoundingBox().IsDefined() &&
      r2->BoundingBox().IsDefined() ) {
         if (r1->BoundingBox().Intersects(r2->BoundingBox())) {
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
   "<text>Returns if two lines/regions intersects interior.</text--->"
   "<text>query p_intersects ( region1,region2 )</text--->"
   ") )";

ValueMapping p_IntersectsValueMap[] =
   {p_inter_ll, p_inter_lr, p_inter_rl, p_inter_rr };

Operator pIntersects
   ( "p_intersects", p_IntersectsSpec, 4, p_IntersectsValueMap,
     IntersectsSelectCompute, IntersectsMap);


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

extern "C"
Algebra*
InitializePlaneSweepAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (&planeSweepAlgebra);
}
