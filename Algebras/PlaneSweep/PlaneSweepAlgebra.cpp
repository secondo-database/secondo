/*

[1] PlaneSweepAlgebra

September 2005 Annette Seberich-D[ue]ll

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

October 2006, Christian Duentgen (CD): Datatypes Points, Region and Line
have set-based-semantics. This means, the member functions
~void SetDefined(bool)~ and ~bool IsDefined()~ have no functional
implementation (are just dummies). Also, one must use ~empty~ instead of
~undef~ values. All operators have been corrected to implement this convention.

1 Overview

This implementation file contains the implementation of the realmisation
for the classes ~line~ and ~region~ used in the SpatialALgebra.

 They are used in the realmisation and plane-sweep-
Algorithmen for operations explained in the ROSE-Algebra


2 Defines and Includes

*/


#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "Algebras/Spatial/SpatialAlgebra.h"
#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <list>
#include <set>
#include <queue>
#include <algorithm>

using namespace std;

// some compiler versions seem not to have a definition for M_PI
#ifndef M_PI
const double M_PI = acos( -1.0 );
#endif

//#define PS_DEBUG

extern NestedList* nl;
extern QueryProcessor* qp;


namespace planesweep {


enum realmkind { PLANESWEEP, QUADRATIC, OVERLAPPING };
enum State {FIRST, SECOND, BOTH};

struct HalfSegmentCheck {
  bool splitsegment;
  list<Point> pointlist;
  HalfSegmentCheck() : splitsegment(false) {};
};

const bool PSA_DEBUG = false;
const realmkind kindofrealm = OVERLAPPING;

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
   Segment(bool reg, const HalfSegment& inhs);
   Segment(const Segment& s);
   const Point& GetLeftPoint();
   const Point& GetRightPoint();
   void SetLeftPoint(const Point& p);
   void SetRightPoint(const Point& p);
   bool GetIn1() const;
   const HalfSegment& GetCHS();
   void CHSInsert(list<HalfSegment>& r1, list<HalfSegment>& r2);

   inline Segment& operator=( const Segment& s );

private:
   bool in1;
   HalfSegment hs;
};
/*
konstruktor to biuld up a new segment-object

*/
Segment::Segment(bool in, const HalfSegment& inhs)
{
   in1 = in;
   hs = inhs;
   hs.SetLeftDomPoint(true);
}

Segment::Segment(const Segment& s)
{
  in1 = s.in1;
  hs = s.hs;
}

/*
functions to read and set property values of a segment-object

*/
const Point& Segment::GetLeftPoint()   {  return hs.GetLeftPoint(); }
const Point& Segment::GetRightPoint()   {  return hs.GetRightPoint();}
bool Segment::GetIn1() const    { return in1; }

void Segment::SetLeftPoint(const Point& p)   {
   //if  ( p != hs.GetRightPoint() && p != hs.GetLeftPoint() )
   if ((!AlmostEqual(p, hs.GetRightPoint())) &&
                          (!AlmostEqual(p, hs.GetLeftPoint())))
      hs.Set(true,p,hs.GetRightPoint());
}

void Segment::SetRightPoint(const Point& p)   {
   if  ( p != hs.GetLeftPoint() && p != hs.GetRightPoint() )
      hs.Set(true,hs.GetLeftPoint(),p);
}

const HalfSegment& Segment::GetCHS() { return hs;}

/*
Inserts a HalfSegment in right list of HalfSegments

*/
void Segment::CHSInsert(list<HalfSegment>& r1,
   list<HalfSegment>& r2)
{
   if (GetIn1())   r1.push_front(GetCHS());
   else  r2.push_front(GetCHS() );
}

Segment& Segment::operator=( const Segment& s )
{
  in1 = s.in1;
  hs = s.hs;
  return *this;
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

This class represents a node in the tree ~BinTree~. Each node has an entry,
which values can be change and read. Each node had pointers to all neighors in
the tree and in the list ant ro his father.

*/

template<class E> class BinTreeNode
{
public:

   BinTreeNode<E>() {};
   E GetEntry()                         { return entry;}
   void SetEntry(const E& newentry)     { entry.Set(newentry); }
   BinTreeNode<E>* GetNext()            { return Next; }
   BinTreeNode<E>* GetPred()            { return Pred; }

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

This class implements a binary tree. This tree is used in the realmisation
prozess and in the plane-sweep algorithm of the operations union intersection
and minus. This class can be used with the classes ~XEvent~, ~SSSLine~ und
~StatusLine~ in this algebra.
To use the template the class for an entry must have a function less for
comparisations of two objects.
This tree is a binary tree (a red-black-tree) with a doubled linked list, which
join all entries in the tree. The list is used for searching the predecessor
and the successesor of entrie. There is also a pointer to the first and the
last entrie of the tree (used in the priority queue)

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
   //NIL = 0;
}

/*
2.2.2 functions to insert entries

There are different functions to insert an entry in the tree. One for each
class in this algebra, which uses this template. First used from class ~PQueue~
with entry ~XEvent~. Returns the first entry and delete it from the tree (top
and pop). Second used in class ~StatusLine~ entry ~SSSEntry~ and third used
from class ~SLine~ entry ~SEntry~.

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

There are different functions to delete an entry in the tree. One for each
class in this algebra, which uses this template. First used from class ~PQueue~
with entry ~XEvent~. Returns the first entry and delete it from the tree (top
and pop). Second used in class ~StatusLine~ entry ~SSSEntry~ and third used
from class ~SLine~ entry ~SEntry~.

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
            //del = 0;
         }
         else next = NIL;
      }
   }
   //cout << node->GetEntry().GetX();
   //if (node->Pred) cout << " Pred: " << node->Pred->GetEntry().GetX() << endl;
   //if (node->Next) cout << " Next: " << node->Next->GetEntry().GetX() << endl;
   //cout << endl;
   delete node;
   //node = 0;
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
   //node = 0;
}

// used for class ~SLine~ with entry ~SEntry~
template<class E> void BinTree<E>::DeleteNode (BinTreeNode<E>*
   node) {
   assert (node);
   MakeDelete(node);
   mCount --;
   delete node;
   node = 0;
}

/*
3.2.3 functions to find entries

There are different functions to find an entry in the tree or to find the right
place to insert a entry. One for  each class in this algebra, which uses this
template. First used from class ~PQueue~ with entry ~XEvent~. Second used in
class ~StatusLine~ (entry ~SSSEntry~) and third used from class ~SLine~ (entry
~SEntry~).

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
      //node2 = 0;
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

There are different functions to compare two entries. One for each class in
this algebra, which uses this template. First used from class ~Queue~ with
entry ~XEvent~. Second used in class ~StatusLine~ entry ~SSSEntry~ and third
used from class ~SLine~ entry ~SEntry~.

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

There are different functions to compare two entries. One for each class in
this algebra, which uses this template. First used from class ~PQueue~ with
entry ~XEvent~. Second used in class ~StatusLine~ entry ~SSSEntry~ and third
used from class ~SLine~ entry ~SEntry~.

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
First searhs the right place, then inserts the entry and then calls the
balance-function

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
First searhs the right entry, then delete the entry and then calls the balance-
function

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
   bool balance=false;
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
  if (node->Pred)  {
    node->Pred->Next = node->Next;
  }
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

Several classes are needed to contruct al REALM from a ~line~- or a ~region~-
Object.
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
   XEvent(const XEvent& event);
   void Set(const XEvent& event);
   Coord GetX() const;
   Coord GetY() const;
   EventKind GetKind() const;
   int GetFirst() const;
   int GetSecond() const;
   double GetSlope() const;
   double GetA() const;
   int Less (const XEvent& ev2, const Coord x) const;
   bool Equal (const XEvent& ev2) const;
   const bool operator< (const XEvent& ev2) const;
   const bool operator== (const XEvent& ev2) const;
   XEvent& operator= (const XEvent& ev);

private:
   Coord x;
   Coord y;
   double slope;
   double a;
   EventKind kind;
   int seg1;
   int seg2;
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

XEvent::XEvent(const XEvent& event)
{
   x = event. x;
   y = event. y;
   slope = event.slope;
   a = event.a;
   kind= event.kind;
   seg1 = event.seg1;
   seg2 = event.seg2;
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

Coord XEvent::GetX() const              {return x;}
Coord XEvent::GetY() const              {return y;}
EventKind XEvent::GetKind() const       {return kind;}
int XEvent::GetFirst() const            {return seg1;}
int XEvent::GetSecond() const           {return seg2;}
double XEvent::GetSlope() const         {return slope;}
double XEvent::GetA() const             {return a;}

/*
3.2.3 Operations

*/

XEvent& XEvent::operator= (const XEvent& ev)
{
  Set(ev);
  return *this;
}

bool XEvent::Equal (const XEvent& ev2) const
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

int XEvent::Less (const XEvent& ev2, const Coord x) const
{
   //if ( Equal (ev2) )  { cout << "kommt tats�hlich vor" << endl; return 0;}
   // ordered by x-value
   //else if (GetX() < ev2.GetX())  { return -1; }
   if (GetX() < ev2.GetX())  { return -1; }
   else if (GetX() > ev2.GetX()) { return 1; }
   // then ordered by event-kind
   else if (GetKind() < ev2.GetKind())          { return -1;}
   else if (GetKind() > ev2.GetKind())          { return 1;}
   else  {// groups ordered by different categories
      if (GetKind() == leftSegment || GetKind() == verticalSegment) {
          // add at the end of a group
         if (GetFirst() < ev2.GetFirst())       { return -1;}
         else                                   { return 1;}
      }
      else if (GetKind() == rightSegment) {
         // then ordered by y-value, then slope of the segment
         if (GetY() < ev2.GetY())               { return -1;}
         else if (GetY() > ev2.GetY())          {return 1;}
         else if (GetSlope() > ev2.GetSlope())  {return 1;}
         else                                   {return -1;}
      }
      else if (GetKind() == intersection)  {
         // ordered by y-value
         if (GetY() < ev2.GetY())               {return -1;}
         else                                   {return 1;}
      }
      else if (GetKind() == split)  {
         // ordered by y-value
         if (GetY() < ev2.GetY())               {return -1;}
         else                                   {return 1;}
      }
      // should not reached
      else {
         cout << "eventkind not known < " << GetKind() << endl;
         return -1;
     }
   }
}

const bool XEvent::operator< (const XEvent& ev2) const {

        return !(Less(ev2, Coord()) < 1);
}

const bool XEvent::operator== (const XEvent& ev2) const {

        return (Equal(ev2));
}

ostream& operator<<(ostream &os, const XEvent& ev)
{
   return (os << "XEvent: x:" << ev.GetX() << "  y: "
      << ev.GetY() << "  Kind: " << ev.GetKind()<< "   First: "
      << ev.GetFirst() << "  Second:  " << ev.GetSecond()
      << "  slope: " << ev.GetSlope() << "  a: " << ev.GetA() );
}



/*
3.3 class PQueue

This class is implementing the priority of XEvents used in the realmisation
process and also in the plane-Sweep-algorithm for operations.
The priority queue based on a binary tree (template ~BinTree~) and uses the
class ~XEvent~ as entries.

*/
class PQueue
{
public:
   inline PQueue() {};
   inline ~PQueue() {};
   inline void insert (const XEvent& event);
   inline XEvent getFirstAndDelete();
   inline bool isEmpty();
   inline int size();

private:
   priority_queue<XEvent> qu2;
};

/*
functions for initialising a tree and reading property values for a PQueue-
Object

*/
inline bool PQueue::isEmpty()
{
  return qu2.empty();
}

inline int PQueue::size()
{
  return qu2.size();
}

inline void PQueue::insert (const XEvent& event)
{
  qu2.push(event);
}

inline XEvent PQueue::getFirstAndDelete()
{
   XEvent event = qu2.top();
   qu2.pop();
   return event;
}

ostream& operator<<(ostream &os, const PQueue& pq)
{
  PQueue pq2;
  pq2 = pq;
  while ( !pq2.isEmpty() ) {
    os << pq2.getFirstAndDelete() << endl;
  }
  return (os);
}

/*
3.4 class SSSEntry

This class implements a entry for the Sweepline-Status-Structure (class
~StatusLine~). Each entry has three attributes: the HalfSegment coded as a
integer and two double-values to calculate the y-value for a given x-Coordinate

*/

class SSSEntry
{
public:
   SSSEntry() { hs = -1;};
   ~SSSEntry() {};
   SSSEntry(int inseg, Segment segs[] );
   SSSEntry(int inseg, double ink, double ina);
   SSSEntry(const SSSEntry& s);
   void Set(const SSSEntry& in);

   bool Equal (const SSSEntry& se) const;
   int GetSeg() const;
   const double GetY (Coord x, Segment segs[]) const;
   double Getk() const;
   double Geta() const;
   int Less (const SSSEntry& in2, const Coord x, Segment segs[]) const;

private:
   int hs;
   double k;
   double a;
};

/*
3.4.1 Constructors and Destructors

*/
SSSEntry:: SSSEntry(int inseg, double ink, double ina)
{
   hs = inseg;
   k = ink;
   a = ina;
}

SSSEntry:: SSSEntry(const SSSEntry& s)
{
   hs = s.hs;
   k = s.k;
   a = s.a;
}

/*
creates a new entry-object from a given CHalfsegments and calculates slope and a

*/
SSSEntry:: SSSEntry (int inseg, Segment segs[])
{
   hs = inseg;
   Coord xl, xr, yl, yr;
   xl = segs[inseg].GetCHS().GetLeftPoint().GetX();
   xr = segs[inseg].GetCHS().GetRightPoint().GetX();
   yl = segs[inseg].GetCHS().GetLeftPoint().GetY();
   yr = segs[inseg].GetCHS().GetRightPoint().GetY();
   k = (yr - yl) / (xr - xl) ;
   a = yl - k*xl;
}


/*
3.4.2 Functions for readig and setting values of a SSSEntry-Object

*/
void SSSEntry:: Set(const SSSEntry& in)
{
   hs = in.hs;
   k = in.k;
   a = in.a;
}

double SSSEntry::Getk() const                   {return k; }
double SSSEntry::Geta() const                   {return a; }
int SSSEntry::GetSeg() const                    {return hs; }

/*
3.4.3 Functions for comparisation

*/
bool SSSEntry::Equal(const SSSEntry& se) const
{
   if (GetSeg() == se.GetSeg()) return true;
   else return false;
}

/*
SSSEntry-objects are used as entries in the sweepline-structure (class
~StatusLine~). This class uses a template ~BinTree~ for the implementation.
These two function are need to calculate the y-value and for the comparisation
of two XEvents in the template BinTree to insert the XEvents in the right node.

*/

const double SSSEntry::GetY(Coord x, Segment segs[]) const  {
   Coord res;
   bool end = false;
   if (segs[GetSeg()].GetCHS().GetLeftPoint().GetX() == x) {
      end = true;
      res = segs[GetSeg()].GetCHS().GetLeftPoint().GetY();
   }
   else if (segs[GetSeg()].GetCHS().GetRightPoint().GetX() == x) {
      end = true;
      res = segs[GetSeg()].GetCHS().GetRightPoint().GetY();
   }
   if ( end) {
      double y = res ;
      return y;
   }
   else {
      double xv = x ;
      return ( k*xv + a);
   }
}

int SSSEntry:: Less (const SSSEntry& in2, const Coord x,
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

This class implements the Sweepline-Status-Structure of the realmisation
process. It uses the template ~BinTree~ and the class ~SSSEntry~ as entry-
objects.

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
   bool InsertIntersection(const HalfSegment& hs1,
      const HalfSegment& hs2,int s1,int s2, PQueue& pq );
   void erase(SSSEntry* entry);
   BinTreeNode<SSSEntry>* GetGreater(Coord xvalue, Coord y,
     Segment segs[]);
   BinTreeNode<SSSEntry>* GetNext(BinTreeNode<SSSEntry>* ent);
   BinTreeNode<SSSEntry>* GetPred(BinTreeNode<SSSEntry>* ent);
   bool innerInter( const HalfSegment& hs1, const HalfSegment&
      hs2,  Point& resp, HalfSegment& rhs, bool& first,
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

StatusLine::StatusLine()        { BinTree<SSSEntry> ssl;}
bool StatusLine::IsEmpty()      { return  ssl.IsEmpty() ; }
int StatusLine::Size()          { return ssl.GetCount(); }
BinTreeNode<SSSEntry>* StatusLine::GetNext
   (BinTreeNode<SSSEntry>* ent)  {return ent->GetNext();}
BinTreeNode<SSSEntry>* StatusLine::GetPred
   (BinTreeNode<SSSEntry>* ent)  {return ent->GetPred();}

/*
3.5.2 Functions for insert, delete and find entry-objects

*/

/*
This function is used to find the right position to insert a new entry. It
searchs that node in the statusline, which will be the next in the order. (Used
in the insert-function)

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
This function searches a entry in the Sweepline-Staus-Stucture. It uses the
calculated y-value. (Used in the delete-function.)

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
function to insert an entry in the sweepline, after inserting at the right
position (sweep line is ordered by y-values) tests for intersection are made
with predecessor and successor.
Additionally tests are made with more neighbors, if two segments are parallel.

*/
void StatusLine::Insert(Coord x, SSSEntry& entry, Segment sgs[],
   PQueue& pq)
{
   // insert entry into sweepline
   BinTreeNode<SSSEntry>* en = ssl.Insert(entry, x, sgs);
   BinTreeNode<SSSEntry>* pred = en->GetPred();
   BinTreeNode<SSSEntry>* succ = en->GetNext();
   //bool inter;
   int segnr = entry.GetSeg();
   if (pred != 0 ) { // test for intersection with predecessor
     int psegnr = pred->GetEntry().GetSeg();
     //inter = 
       InsertIntersection (sgs[psegnr].GetCHS(),
       sgs[segnr].GetCHS(), psegnr, segnr, pq);
       /*if (inter) { // pred and entry intersects comm
         BinTreeNode<SSSEntry>* predpred = pred->GetPred();
         if (predpred != 0 ) { // test for intersection with predpred
           cout << "predpred" << endl;
           int ppsegnr = predpred->GetEntry().GetSeg();
           bool inter2=InsertIntersection(sgs[ppsegnr].GetCHS(),
             sgs[segnr].GetCHS(), ppsegnr,segnr,pq);
             if (inter2) { // and so on
               BinTreeNode<SSSEntry>* predpredpred =
                predpred->GetPred();
                if (predpredpred !=0) {
                  cout << "predpredpred" << endl;
                  int pppSegnr = predpredpred->GetEntry().GetSeg();
                  InsertIntersection(sgs[pppSegnr].GetCHS(),
                    sgs[segnr].GetCHS(), pppSegnr,segnr,pq);
               } // predpredpred
            } // (inter2)
         } // predpred !=0
      } // (inter) comm */
   } // pred !=0
   if (succ != 0) {   // test for intersection with successor
    int sSegnr = succ->GetEntry().GetSeg();
      //inter = 
       InsertIntersection (sgs[segnr].GetCHS(),
         sgs[sSegnr].GetCHS(), segnr, sSegnr, pq);
      /*if (inter) { // intersection entry- succ comm
        BinTreeNode<SSSEntry>* succsucc = succ->GetNext();
        if (succsucc != 0 ) { // succsucc exists
          cout << "succsucc" << endl;
          int ssSegnr = succsucc->GetEntry().GetSeg();
          bool inter2= InsertIntersection (sgs[segnr].GetCHS(),
            sgs[ssSegnr].GetCHS(), segnr, ssSegnr,pq);
            if (inter2) { // intersection entry - succsucc
              BinTreeNode<SSSEntry>* succsuccsucc =
                 succsucc->GetNext();
              if (succsuccsucc !=0) {
                cout << "succsuccsucc" << endl;
                int sssSegnr = succsuccsucc->GetEntry().GetSeg();
                InsertIntersection(sgs[segnr].GetCHS(),
                  sgs[sssSegnr].GetCHS(), segnr,sssSegnr,pq);
           }
         } // (inter2)
       } // succsucc!0=
     } // (inter) comm */
   } // succ !=0
}

/*
function for deleting a entry from the sweep line
Atuomatically a test for intersection the new neihbors is added (predecessor
and succsessor of ald entry are neighbors now).

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
         // test for intersection with pred- and succ-HalfSegment
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
as result the single point of intersection or the Segments which overlap are
given as result in resp or rhs
If the intersetion points are very close to the endpoint (Distance is
less than DIST), it is assumed, that the intersection is only a problem of
rounding double values

*/

bool StatusLine::innerInter( const HalfSegment& hs1,
   const HalfSegment& hs2, Point& resp, HalfSegment& rhs,
   bool& first, bool& second ) const
{
   resp.SetDefined(false);
   first = false;               second = false;
   Coord xl,yl,xr,yr ,  Xl,Yl,Xr,Yr;
   double k, a, K, A;
   k= a= K= A=0;

   xl=hs1.GetLeftPoint().GetX();  yl=hs1.GetLeftPoint().GetY();
   xr=hs1.GetRightPoint().GetX();  yr=hs1.GetRightPoint().GetY();
   if (xl!=xr)  // hs1 not vertical
   {     //k=(yr-yl) / (xr-xl);  a=yl - k*yl;
      k=(yr - yl) / (xr - xl);
      a=yl - k*xl;
   }
   Xl=hs2.GetLeftPoint().GetX();  Yl=hs2.GetLeftPoint().GetY();
   Xr=hs2.GetRightPoint().GetX();  Yr=hs2.GetRightPoint().GetY();
   if (Xl!=Xr)  // hs2 not vertical
   {     //K=(Yr-Yl) / (Xr-Xl);  A=Yl - K*Xl;
      K =  (Yr - Yl) / (Xr - Xl);
      A = Yl - K*Xl;
   }
   if ((xl==xr) && (Xl==Xr))  {
      //both hs1 and hs2 are vertical lines
      if (xl!=Xl) return false;
      else  {
         Coord ylow, yup, Ylow, Yup;
         if (yl<yr)   { ylow=yl;  yup=yr;  }
         else         { ylow=yr;  yup=yl;  }
         if (Yl<Yr)   { Ylow=Yl;  Yup=Yr;  }
         else     { Ylow=Yr;  Yup=Yl;  }
         if  (((ylow>Ylow) && (ylow<Yup))||
              ((yup>Ylow) && (yup<Yup)) ||
              ((Ylow>ylow) && (Ylow<yup))||
              ((Yup>ylow) && (Yup<yup))) {
            Point p1, p2;
            if (ylow>Ylow)      p1.Set(xl, ylow);
            else                p1.Set(xl, Ylow);
            if (yup<Yup)        p2.Set(xl, yup);
            else                p2.Set(xl, Yup);
            rhs.Set(true, p1, p2);
            first = true;       second = true;
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
         double y0=k*Xl+a;
         Coord yy=y0;
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
// hier einfgen
      if ( Xl==xl && Yl>yl && Yl<yr )
         {resp.Set(Xl,Yl); first = true; return true;}
      if ( Xr==xl && Yr>yl && Yr<yr )
         {resp.Set(Xr,Yr); first = true; return true;}
      else  {
         double y0=K*xl+A;
         Coord yy=y0;
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
         if (xl>Xl)     p1.Set(xl, yl);
         else           p1.Set(Xl, Yl);
         if (xr<Xr)     p2.Set(xr, yr);
         else           p2.Set(Xr, Yr);
         rhs.Set(true, p1, p2);
         first = true; second = true;
         return true;
      }
     else return false;
   } // else (k==K)
   else  {
      double x0 = (A-a) / (k-K);  // y0=x0*k+a;
      double y0 = x0*k+a;
        Coord xx = x0; Coord yy=y0;
     if (hs1.GetLeftPoint() == hs2.GetLeftPoint() ||
         hs1.GetRightPoint() == hs2.GetRightPoint() ) return false;
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
bool StatusLine::InsertIntersection(const HalfSegment& hs1,const
   HalfSegment& hs2,int s1, int s2, PQueue& pq)
{
   bool result = false;
   Point point;
   Coord xp, yp;
   HalfSegment res;
   bool one, two;
   // test if two HalfSegments intersects
   // if intersection in one point -> point isDefined
   // if they overlap -> HalfSegment res is the result-segment
   bool innerinter = innerInter(hs1, hs2, point, res, one, two);
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
      else {
         // both segments are the same
         if (hs1 == hs2) { return true;}
         // both segments starts with same point
         else if (hs1.GetLeftPoint() == hs2.GetLeftPoint() ) {
            if (hs1.Inside(hs2) ) {
               xp = hs1.GetRightPoint().GetX();
               yp = hs1.GetRightPoint().GetY();
               XEvent ev1( xp, yp, s2, split);
               pq.insert(ev1);
            }
            else {
               xp = hs2.GetRightPoint().GetX();
               yp = hs2.GetRightPoint().GetY();
               XEvent ev1(xp, yp, s1, split);
               pq.insert(ev1);
            }
         }
         // both segments ends in same point
         else if (hs1.GetRightPoint() == hs2.GetRightPoint() )  {
            if (hs1.Inside(hs2) ) {
               xp = hs1.GetLeftPoint().GetX();
               yp = hs1.GetLeftPoint().GetY();
               XEvent ev1(xp, yp, s2, split);
               pq.insert(ev1);
            }
            else {
               xp = hs2.GetLeftPoint().GetX();
               yp = hs2.GetLeftPoint().GetY();
               XEvent ev1(xp, yp, s1, split);
               pq.insert(ev1);
            }
         }
         // one segement inside the other
         else if (hs1.Inside(hs2)) {
            xp = hs1.GetLeftPoint().GetX();
            yp = hs1.GetLeftPoint().GetY();
            XEvent ev1(xp, yp, s2, split);
            pq.insert(ev1);
            xp = hs1.GetRightPoint().GetX();
            yp = hs1.GetRightPoint().GetY();
            XEvent ev2(xp, yp, s2, split);
            pq.insert(ev2);
         }
         else if (hs2.Inside(hs1)) {
            xp = hs2.GetLeftPoint().GetX();
            yp = hs2.GetLeftPoint().GetY();
            XEvent ev1 (xp, yp, s1, split);
            pq.insert(ev1);
            xp = hs2.GetRightPoint().GetX();
            yp = hs2.GetRightPoint().GetY();
            XEvent ev2 (xp, yp, s1, split);
            pq.insert(ev2);
         }
         else  { // overlap without Inside
            if (hs1.GetRightPoint() > hs2.GetRightPoint() )   {
               xp = hs1.GetLeftPoint().GetX();
               yp = hs1.GetLeftPoint().GetY();
               XEvent ev1(xp, yp, s2, split);
               pq.insert(ev1);
               xp = hs2.GetRightPoint().GetX();
               yp = hs2.GetRightPoint().GetY();
               XEvent ev2(xp, yp, s1, split);
               pq.insert(ev2);
            }
            else {
               xp = hs1.GetRightPoint().GetX();
               yp = hs1.GetRightPoint().GetY();
               XEvent ev1(xp, yp, s2, split);
               pq.insert(ev1);
               xp = hs2.GetLeftPoint().GetX();
               yp = hs2.GetLeftPoint().GetY();
               XEvent ev2(xp, yp, s1, split);
               pq.insert(ev2);
            }
         } // overlap without Inside
      }
   } // innerinter
   return result;
}

/*
with this function two entries are exchanged. A test for intersection for new
neighbors is added

*/
void StatusLine::Exchange (BinTreeNode<SSSEntry>* e1,
   BinTreeNode<SSSEntry>* e2, Segment sgs[], PQueue& pq)
{
   if ( (e1 != 0) && (e2 != 0) ) {
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
}

/*
3.6 class VStructure

This class is  activated, if a vertical segment occurs (SetDefined = true). It
is used to store the y-values of end-point of segments at the sweepline, at
wich the vertical segment must be split.
In this class a <list> from the C++-Standards is used.

*/

class VStructure {
public:
   friend class VList;
   VStructure();
   ~VStructure()   {};
   void Insert (Coord y);
   void SetDefined(bool def)    { defined = def;}
   bool IsDefined ()            { return defined; }
   void Clear();
private:
   list<Coord>  vstruct;
   bool defined;
   Coord min, max;
   bool IsEmpty()               { return vstruct.empty(); }
   Coord GetMin()               { return min; }
   Coord GetMax()               { return max; }
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
If a vertical segments occurs the vlist is activated and the segment is
inserted. When the sweep line moves, the vlist is cleared, that means alss
segments in the vlist are tested for overlaps with each other in the list and
the sweepline is scanned from bottom to top for intersection with segments.
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
    int Size();

private:
   list<Segment> vlist;

   void DeleteFirst();
   Segment First();
   list<Segment> eraseOverlaps(Coord sline, VStructure vs);
   void testStatusLine(StatusLine& sl, Segment segs[]);
   void testOverlaps(Coord& sweepline, VStructure& vs,
      StatusLine& sl, Segment segs[]);
   void output();
};

/*
Initalisation, reading properties

*/
VList::VList()             { list<Coord> vlist; }
int VList::Size()          { return vlist.size(); }
Segment VList::First()   { return (vlist.front()); }
bool VList::IsEmpty()      { return vlist.empty(); }

/*
functions for handling the list

*/
void VList::Clear()        { vlist.clear(); }
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
           <<s.GetCHS().GetLeftPoint()<<" "<<s.GetCHS().GetRightPoint()<<endl;
      ++p;
   }
   cout << " in output fertig " << endl;
}

/*
This function is called if the sweep line moves to a new position. The vertical
segments are worked off. This means tests are made to find intersection with
other segments.

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
      HalfSegment hs = first.GetCHS();
      DeleteFirst();
      list<Segment>::iterator p = vlist.begin();
      // test first Segment for overlaps with other HalfSegments
      while ( (p != vlist.end()) &&  (deleted == false)  )  {
         Segment ref = *p;
         HalfSegment test = ref.GetCHS();
         HalfSegment res;
         bool overlap = hs.Intersection(test,res);
         //if (overlap) cout << "overlap is true" << endl;
         //cout << first << endl;
         //cout << ref << endl;
         //cout << res << endl;
         if (overlap  && ( first.GetIn1() != ref.GetIn1()) ) {
            // only different regions are tested
            // first segment inside the other Segment
            if (hs.Inside(test) ) {
               if (hs.GetLeftPoint()==test.GetLeftPoint()&&hs.GetRightPoint()==
                  test.GetRightPoint()) { }
               else if (hs.GetLeftPoint() == test.GetLeftPoint() ) {
                  Segment newseg(ref.GetIn1(),test);
                  newseg.SetRightPoint(res.GetRightPoint());
                  vlist.insert(p,newseg);
                  ref.SetLeftPoint(res.GetRightPoint());
                  vlist.erase(p);  vlist.push_back(ref);
               }
               else if (hs.GetRightPoint() == test.GetRightPoint()) {
                  Segment newseg(ref.GetIn1(),test);
                  newseg.SetRightPoint(res.GetLeftPoint());
                  vlist.insert(p,newseg);
                  ref.SetLeftPoint(res.GetLeftPoint());
                  vlist.erase(p); vlist.push_back(ref);
               }
               else  {
                  Segment newseg1(ref.GetIn1(),test);
                  newseg1.SetRightPoint(res.GetLeftPoint());
                  vlist.insert(p,newseg1);
                  Segment newseg2(ref.GetIn1(), test);
                  newseg2.SetLeftPoint(res.GetLeftPoint());
                  newseg2.SetRightPoint(res.GetRightPoint());
                  vlist.insert(p,newseg2);
                  ref.SetLeftPoint(res.GetRightPoint());
                  vlist.erase(p);
                  vlist.push_back(ref);
               }
            }
            // second segment inside the other
            else if (test.Inside(hs))  {
               if (hs.GetLeftPoint() == test.GetLeftPoint() ) {
                  Segment newseg(first.GetIn1(),hs);
                  newseg.SetRightPoint(res.GetRightPoint());
                  vlist.insert(p,newseg);
                  first.SetLeftPoint(res.GetRightPoint());
               }
               else if (hs.GetRightPoint() == test.GetRightPoint()) {
                  Segment newseg(first.GetIn1(),hs);
                  newseg.SetRightPoint(res.GetLeftPoint());
                  vlist.insert(p,newseg);
                  first.SetLeftPoint(res.GetLeftPoint());
               }
               else  {
                  Segment newseg1(first.GetIn1(),hs);
                  newseg1.SetRightPoint(res.GetLeftPoint());
                  vlist.insert(p,newseg1);
                  Segment newseg2(first.GetIn1(),hs);
                  newseg2.SetLeftPoint(res.GetLeftPoint());
                  newseg2.SetRightPoint(res.GetRightPoint());
                  vlist.insert(p,newseg2);
                  first.SetLeftPoint(res.GetRightPoint());
               }
            }
            // overlap but not inside
            else if (res.GetLeftPoint() > hs.GetLeftPoint())  {
               Segment newseg1(first.GetIn1(),hs);
               newseg1.SetRightPoint(res.GetLeftPoint());
               vlist.insert(p,newseg1);
               first.SetLeftPoint(res.GetLeftPoint());
               Segment newseg2(ref.GetIn1(),test);
               newseg2.SetRightPoint(res.GetRightPoint());
               vlist.insert(p,newseg2);
               ref.SetLeftPoint(res.GetRightPoint());
               vlist.erase(p); vlist.push_back(ref);
            }
            else   {
               Segment newseg1(first.GetIn1(),hs);
               newseg1.SetRightPoint(res.GetRightPoint());
               vlist.insert(p,newseg1);
               first.SetLeftPoint(res.GetRightPoint());
               Segment newseg2(ref.GetIn1(),test);
               newseg2.SetRightPoint(res.GetLeftPoint());
               vlist.insert(p,newseg2);
               ref.SetLeftPoint(res.GetLeftPoint());
               vlist.erase(p); vlist.push_back(ref);
            }
         } // end overlap
         ++p;
      } // end 2nd while and now treat rightEnds of CHAlfSegments
      // tests if there are endpoints of other segments inside verticals
      Coord min = vs.GetMin();
      Coord max =  vs.GetMax();
      hs = first.GetCHS();
      if (!((hs.GetLeftPoint().GetY()<=min &&
             hs.GetRightPoint().GetY()<= min)
         ||(hs.GetLeftPoint().GetY()>=max &&
            hs.GetRightPoint().GetY()>= max))) {
         list<Coord>::iterator ps = vs.vstruct.begin();
         bool ready = false;
         while (ps != vs.vstruct.end() && ready != true) {
            Coord y = *ps;
            if (y >= hs.GetRightPoint().GetY())   ready = true;
            else if (y > hs.GetLeftPoint().GetY()) {
               // split CHalfsegment
               Point newp = Point(true, sweepline,y);
               Segment newseg(first.GetIn1(),first.GetCHS());
               newseg.SetRightPoint(newp);
               newlist.push_back(newseg);
               first.SetLeftPoint(newp);
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
for each vertical segment in the list the sweep line is scanned from the bottom
  of a vertical to the top. If a non-vertical segments occurs, a XEvent split
  is inserted. The vertical segment ist also split and inserted in result.

*/

void  VList::testStatusLine(StatusLine& sline, Segment segs[])
{
   list<Segment> newlist;
//   list<Segment>::iterator p = vlist.begin();
   //cout << "StatusLine" << endl;
   //sline.output(1,segs); cout << endl;
   while ( ! IsEmpty() )  {
      Segment vertref = First();
      HalfSegment vert = vertref.GetCHS();
      DeleteFirst();
      Coord x = vert.GetLeftPoint().GetX();
      Coord y = vert.GetLeftPoint().GetY();
      // searches the next segment to the bottom of vertical segment
      BinTreeNode<SSSEntry>* node = sline.GetGreater(x,y,segs);
      while (node != 0)  {
         //cout << "StatusLine node != 0" << endl;
         SSSEntry  ent = node -> GetEntry();
         Segment testref = segs[ent.GetSeg()];
         //cout << testref << endl;
         HalfSegment test = testref.GetCHS() ;
         Point res;
         // tests if a segments intersects the vertical
         if( vert.Crosses(test) ) { // intersection occurs
            vert.Intersection(test, res);
            //cout << "inter is true" << endl;
            Segment newseg1(vertref.GetIn1(), vert);
            newseg1.SetLeftPoint(res);
            vlist.push_front (newseg1);
            vertref.SetRightPoint(res);
            Segment newseg2(testref.GetIn1(), test);
            newseg2.SetRightPoint(res);
            newlist.push_back(newseg2);
            testref.SetLeftPoint(res);
            //cout << "Testref: " << testref << endl;
            segs[ent.GetSeg()] = testref;
         } // end inter
         node = node->GetNext();
         if (node!=0 && node->GetEntry().GetY(x,segs)
            >vert.GetRightPoint().GetY())  {node = 0;}
      } // all entries in statusline tested
//      p++; // next vertical Segment
      newlist.push_back(vertref);
   }
   vlist = newlist;
}

/*
3.8 class is_greater_than

*/

class is_greater_than
{
public:
   is_greater_than (Point p)
     : value(p)
   {}

   bool operator() (const Point element) const
   {
     if ( (element.GetX() > value.GetX()) or ((element.GetX() == value.GetX())
      and (element.GetY() > value.GetY())) )
       return true;
     return false;
   }

private:
   Point value;
};

/*
3.8 class MakeRealm

In this class the realmisation prozess was worked out. For each possible
combination of line and region-objects a function exists.
the steps were the same, but the results are different. In the first step the
plane-Sweep was prepared, that means for all segments in both objects the
XEvent for left and right end was inserted in the priority queue. In the second
step the sweep line moves from left ro right through the plane and handle all
events.

*/

class MakeRealm {
public:
   MakeRealm() {};
   ~MakeRealm() {};
   void PQueueOutput(PQueue& pq);
   void  REALM (const Region* reg1, const Region* reg2, Region* result1,
      Region* result2);
   void  REALM (const Region* reg1, const Region* reg2, Line* result1,
   Line* result2);
   void  REALM (const Line* reg1, const Region* reg2, Line* result1,
      Region* result2);
   void  REALM (const Line* reg1, const Line* reg2, Line* result1,
      Line* result2);
   template <class T, class U, class V, class W> void REALM2(const T* ,
                      const U*, const bool, const bool, V* , W*);

private:
   void PerformPlaneSweep(PQueue& pq,Segment segs[],
      list<HalfSegment>& res1,list<HalfSegment>& res2,
      const int counter);
   void PrepareCHS (bool No1, const HalfSegment& hs, PQueue& pqu,
      Segment segs[], int counter);
   template<class T> void readhalfsegments(vector<HalfSegment>&,
                                             const T&) const;
   void print(const HalfSegment&) const;
   void printsegvector(const vector<HalfSegment>&) const;
   void printlist(const list<Point>&);
   void print(const HalfSegmentCheck&);
   void printsegcheckvector(const vector<HalfSegmentCheck>&);
   void checksegments(const HalfSegment&, const HalfSegment& hs2, const int,
             const int, vector<HalfSegmentCheck>&, vector<HalfSegmentCheck>&);
   void checksegmentsline(const HalfSegment&, const HalfSegment& hs2, const int,
             const int, vector<HalfSegmentCheck>&);
   bool xoverlaps(const HalfSegment&, const HalfSegment&);
   void dorealm(const vector<HalfSegment>&, const vector<HalfSegment>&,
             vector<HalfSegmentCheck>&, vector<HalfSegmentCheck>&);
   void dorealm2(const vector<HalfSegment>&, const vector<HalfSegment>&,
                 const bool,const bool, vector<HalfSegmentCheck>&,
                 vector<HalfSegmentCheck>&);
   int intersects(const Point&, const Point&, const Point&, const Point&);
   bool onsegment(const Point&, const Point&, const Point&);
   double direction(const Point&, const Point&, const Point&);
   void hsintersection(const HalfSegment&, const HalfSegment&, Point&);
   void insertpoint(list<Point>&, const Point);
   void createrealmedobject(const vector<HalfSegment>&,
                    vector<HalfSegmentCheck>&, vector<HalfSegment>&);
};

void MakeRealm::REALM( const Region* reg1,
                       const Region* reg2,
                       Region* result1,
                       Region* result2      )
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the first region for PlanSweep
   Segment* segs = new Segment[ (reg1->Size() + reg2->Size() )  / 2 ];
   int count = 0;
   // insert all segments of region1 into priority queue
   for (int i=0; i<reg1->Size(); i++)   {
      HalfSegment hs;
      reg1->Get(i,hs);
      if (hs.IsLeftDomPoint() == true) {
         PrepareCHS(true, hs, pqueue, segs, count);
         count ++;
      }
   }
   // Prepare the second region for PlaneSweep
   for (int i=0; i<reg2->Size(); i++)   {
      HalfSegment hs;
      reg2->Get(i,hs);
      if (hs.IsLeftDomPoint() == true) {
         PrepareCHS(false, hs, pqueue, segs, count);
         count ++;
      }
   }
   //cout << pqueue << endl;
   list<HalfSegment> res1, res2;
      //cout << "hier ok1" << endl;
   PerformPlaneSweep(pqueue, segs, res1, res2, count);
   //pqueue.Clear();
   //  reconstruction of first region
   result1->Clear();
   result1->StartBulkLoad();
   int counter = 0;
              //cout << "hier ok2" << endl;
   while (! res1.empty() )  {
      HalfSegment hs = res1.back();
      hs.attr.edgeno = counter;
      hs.SetLeftDomPoint(true);       *result1 += hs;
      hs.SetLeftDomPoint(false);      *result1 += hs;
      counter ++;
      res1.pop_back();
   }
   result1->EndBulkLoad();

   //  reconstruction of second region
   result2->Clear();
   result2->StartBulkLoad();
   counter = 0;
   while (! res2.empty() )    {
      HalfSegment hs = res2.front();
      //cout << hs << endl;
      hs.attr.edgeno = counter;
      hs.SetLeftDomPoint(true);       *result2 += hs;
      hs.SetLeftDomPoint(false);      *result2 += hs;
      res1.erase(res2.begin());
      counter++;
   }
   result2->EndBulkLoad();
   delete[] segs;
   //cout << "REALM1" << endl;
   //cout << *result1 << endl;
   //cout << "REALM2" << endl;
   //cout << *result2 << endl;
}

void MakeRealm::REALM(const Region* reg1, const Region* reg2,
                      Line* result1, Line* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the region for PlanSweep
   Segment* segs = new Segment[ (reg1->Size() + reg2->Size()) / 2 ];
   int count = 0;
   // insert all segments of region1 into priority queue
   for (int i=0; i<reg1->Size(); i++)   {
      HalfSegment hs;
      reg1->Get(i,hs);
      if (hs.IsLeftDomPoint() == true) {
         PrepareCHS(true, hs, pqueue, segs, count);
         count ++;
      }
   }
   // Prepare the line for PlaneSweep
   for (int i=0; i<reg2->Size(); i++)   {
      HalfSegment hs;
      reg2->Get(i,hs);
      if (hs.IsLeftDomPoint() == true) {
         PrepareCHS(false, hs, pqueue, segs, count);
         count ++;
      }
   }
   list<HalfSegment> res1, res2;
   PerformPlaneSweep(pqueue, segs, res1, res2, count);
   //  reconstruction of region
   result1->Clear();
   result1->StartBulkLoad();
   int counter = 0;
   while (! res1.empty() )  {
      HalfSegment hs = res1.back();
      hs.attr.edgeno = counter;
      hs.SetLeftDomPoint(true);       *result1 += hs;
      hs.SetLeftDomPoint(false);      *result1 += hs;
      res1.pop_back();
      counter ++;
   }
   // VTA - I need to come back here
   result1->EndBulkLoad();
   //  reconstruction of line
   result2->Clear();
   result2->StartBulkLoad();
   counter = 0;
   while (! res2.empty() )    {
      HalfSegment hs = res2.front();
      hs.attr.edgeno = counter;
      hs.SetLeftDomPoint(true);       *result2 += hs;
      hs.SetLeftDomPoint(false);      *result2 += hs;
      res1.erase(res2.begin());
      counter++;
   }
   // VTA - I need to come back here
   result2->EndBulkLoad();
   delete[] segs;
}

void MakeRealm::REALM(const Line* line1, const Region* reg2,
                      Line* result1, Region* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the line for PlanSweep
   Segment* segs = new Segment[ (line1->Size() + reg2->Size()) / 2 ];
   int count = 0;
   // insert all segments of region1 into priority queue
   for (int i=0; i<line1->Size(); i++)   {
      HalfSegment hs;
      line1->Get(i,hs);
      if (hs.IsLeftDomPoint() == true) {
         PrepareCHS(true, hs, pqueue, segs, count);
         count ++;
      }
   }
   // Prepare the region for PlaneSweep
   for (int i=0; i<reg2->Size(); i++)   {
      HalfSegment hs;
      reg2->Get(i,hs);
      if (hs.IsLeftDomPoint() == true) {
         PrepareCHS(false, hs, pqueue, segs, count);
         count ++;
      }
   }
   list<HalfSegment> res1, res2;
   PerformPlaneSweep(pqueue, segs, res1, res2, count);
   //  reconstruction of region
   result1->Clear();
   result1->StartBulkLoad();
   int counter = 0;
   while (! res1.empty() )  {
      HalfSegment hs = res1.back();
      hs.attr.edgeno = counter;
      hs.SetLeftDomPoint(true);       *result1 += hs;
      hs.SetLeftDomPoint(false);      *result1 += hs;
      counter++;
      res1.pop_back();
   }
   // VTA - I need to come back here - solved ??
   result1->EndBulkLoad();
   //  reconstruction of line
   result2->Clear();
   result2->StartBulkLoad();
   counter = 0;
   while (! res2.empty() )    {
      HalfSegment hs = res2.front();
      hs.attr.edgeno = counter;
      hs.SetLeftDomPoint(true);       *result2 += hs;
      hs.SetLeftDomPoint(false);      *result2 += hs;
      res1.erase(res2.begin());
      counter++;
   }
   result2->EndBulkLoad();
   delete[] segs;
}

void MakeRealm::REALM(const Line* line1, const Line* line2,
                      Line* result1, Line* result2)
{
   PQueue pqueue ;  // priority Queue for XEvents
   // Prepare the first line for PlanSweep
   Segment* segs = new Segment[ (line1->Size() + line2->Size()) /2 ];
   int count = 0;
   // insert all segments of line into priority queue
   for (int i=0; i<line1->Size(); i++)   {
      HalfSegment hs;
      line1->Get(i,hs);
      if (hs.IsLeftDomPoint() == true) {
         PrepareCHS(true, hs, pqueue, segs, count);
         count ++;
      }
   }

   // Prepare the second line for PlaneSweep
   for (int i=0; i< line2->Size(); i++)   {
      HalfSegment hs;
      line2->Get(i,hs);
      if (hs.IsLeftDomPoint() == true) {
         PrepareCHS(false, hs, pqueue, segs, count);
         count ++;
      }
   }

   list<HalfSegment> res1, res2;
   PerformPlaneSweep(pqueue, segs, res1, res2, count);
   //  reconstruction of first line
   result1->Clear();
   result1->StartBulkLoad();
   int counter=0;
   while (! res1.empty() )  {
      HalfSegment hs = res1.back();
      hs.attr.edgeno = counter;
      hs.SetLeftDomPoint(true);       *result1 += hs;
      hs.SetLeftDomPoint(false);      *result1 += hs;
      counter++;
      res1.pop_back();
   }
   // VTA - I need to come back here
   result1->EndBulkLoad();
   //  reconstruction of second line
   result2->Clear();
   result2->StartBulkLoad();
   counter=0;
   while (! res2.empty() )    {
      HalfSegment hs = res2.back();
      hs.attr.edgeno = counter;
      hs.SetLeftDomPoint(true);       *result2 += hs;
      hs.SetLeftDomPoint(false);      *result2 += hs;
      counter++;
      res2.pop_back();
   }
   // VTA - I need to come back here - solved ??
   result2->EndBulkLoad();
   //cout << result1 << endl;
   //cout << result2 << endl;
   delete[] segs;
}

template<class T> void MakeRealm::readhalfsegments(vector<HalfSegment>& v,
                                                     const T& obj) const
{
  HalfSegment hs;

  for (int i=0; i<obj->Size(); i++)
  {
    obj->Get(i,hs);
    if (hs.IsLeftDomPoint() == true) v.push_back(hs);
  }
}

void MakeRealm::print(const HalfSegment& hs) const
{
  Coord xl, xr, yl, yr;

  xl=hs.GetLeftPoint().GetX();
  yl=hs.GetLeftPoint().GetY();
  xr=hs.GetRightPoint().GetX();
  yr=hs.GetRightPoint().GetY();
  cout << "HalfSegment: ";
  cout << "(" << xl << ", " << yl << ") -> ";
  cout << "(" << xr << ", " << yr << ")";
  cout << endl;
}

void MakeRealm::printlist(const list<Point>& values)
{
  list<Point>::const_iterator it = values.begin();
  cout << "[";
  while ( it != values.end() )
  {
    //print(*it);
    cout << *it;
    it++;
  }
  cout << "]" << endl;
}

void MakeRealm::print(const HalfSegmentCheck& segcheck)
{
  if ( segcheck.splitsegment == true )
    cout << "Split segment = yes; ";
  else cout << "Split segment =  no; ";
  printlist(segcheck.pointlist);
}

void MakeRealm::printsegcheckvector(const vector<HalfSegmentCheck>& scv)
{
  vector<HalfSegmentCheck>::const_iterator it = scv.begin();
  cout << "----- Segment Check Vector" << endl;
  while ( it != scv.end() )
  {
    print(*it);
    it++;
  }
  cout << "-----" << endl;
  cout << endl;
}

void MakeRealm::printsegvector(const vector<HalfSegment>& v) const
{
  vector<HalfSegment>::const_iterator it = v.begin();
  cout << "----- Segment Vector" << endl;
  while ( it != v.end() )
  {
    print(*it);
    it++;
  }
  cout << "-----" << endl;
  cout << endl;
}

bool MakeRealm::onsegment(const Point& pi, const Point& pj,
                const Point& pk)
{
  //cout << "pi.GetX(): " << pi.GetX() << endl;
  //cout << "pj.GetX(): " << pj.GetX() << endl;
  //cout << "pi.Gety(): " << pi.GetY() << endl;
  //cout << "pj.GetY(): " << pj.GetY() << endl;
  //cout << "pk.GetX(): " << pk.GetX() << endl;
  //cout << "pk.GetY(): " << pk.GetY() << endl;


  return ( ((min(pi.GetX(), pj.GetX()) <= pk.GetX()) and
              (pk.GetX() <= max(pi.GetX(), pj.GetX()))) and
              ((min(pi.GetY(), pj.GetY()) <= pk.GetY()) and
              (pk.GetY() <= max(pi.GetY(), pj.GetY()))) );
}

double MakeRealm::direction(const Point& pi, const Point& pj, const Point& pk)
{
  double cross1 = pk.GetX() - pi.GetX();
  double cross2 = pj.GetY() - pi.GetY();
  double cross3 = pj.GetX() - pi.GetX();
  double cross4 = pk.GetY() - pi.GetY();

  if ( AlmostEqual(cross1 * cross2 - cross3 * cross4, 0.0) )
    return 0.0;
  else
    return cross1 * cross2 - cross3 * cross4;
}

int MakeRealm::intersects(const Point& p1, const Point& p2,
                const Point& p3, const Point& p4)
{
  double d1, d2, d3, d4;

  d1 = direction(p3, p4, p1);
  d2 = direction(p3, p4, p2);
  d3 = direction(p1, p2, p3);
  d4 = direction(p1, p2, p4);

  //cout << "d1: " << d1 << endl;
  //cout << "d2: " << d1 << endl;
  //cout << "d3: " << d1 << endl;
  //cout << "d4: " << d1 << endl;

  //if (d1 != 0) cout << "d1 != 0" << endl;
  //else cout << "d1 == 0" << endl;
  //if (!AlmostEqual(d1, 0.0)) cout << "!AlmostEqual(d1, 0.0)" << endl;
  //else cout << "AlmostEqual(d1, 0.0)" << endl;


  if ( (!(((d1 > 0.0 and d2 < 0.0) or (d1 < 0.0 and d2 > 0.0)) and
       ((d3 > 0.0 and d4 < 0.0) or (d3 < 0.0 and d4 > 0.0)))) and
       (!AlmostEqual(d1, 0.0)) and (!AlmostEqual(d2, 0.0)) and
       (!AlmostEqual(d3, 0.0)) and (!AlmostEqual(d4, 0.0)) )
    return 0;

  if ( ((d1 > 0.0 and d2 < 0.0) or (d1 < 0.0 and d2 > 0.0)) and
     ((d3 > 0.0 and d4 < 0.0) or (d3 < 0.0 and d4 > 0.0)) )
    return 1;

  //if ( AlmostEqual(d1, 0.0) and AlmostEqual(d2, 0.0) and
  //AlmostEqual(d3, 0.0) and AlmostEqual(d4, 0.0) )
    //cout << " all d's ate almost equal " << endl;

  if ( AlmostEqual(d1, 0.0) and AlmostEqual(d2, 0.0) and
       AlmostEqual(d3, 0.0) and AlmostEqual(d4, 0.0) )
  {
  //if ( (d1 == 0.0) and (d2 == 0.0) and (d3 == 0.0) and (d4 == 0.0) )
  //{

    //cout << " all d's are 0 " << endl;
    //if ( p1 == p3 ) cout << "p1 == p3" << endl;
    //else cout << "p1 != p3" << endl;
    //if ( AlmostEqual(p1, p3) ) cout << "p1 almost equal p3" << endl;
    //else cout << "p1 not almost equal p3" << endl;

    //if ( onsegment(p1, p2, p4) ) cout << "onsegment(p1, p2, p4)" << endl;
    //else cout << "not onsegment(p1, p2, p4)" << endl;


    if ( (AlmostEqual(p1, p3)) and (AlmostEqual(p2, p4)) )
    {
      //cout << "case(11)" << endl;
      return 14; // case (11)
    }

    if ( AlmostEqual(p2, p3) ) // case (1)
    {
      //cout << "case(1)" << endl;
      return 14;
    }

    if (  (onsegment(p3, p4, p2)) and (onsegment(p1, p2, p3))
          and (p2 != p3) and (p2 != p4)
          and (p1 != p3) and (p1 != p4) ) // case (2)
    {
      //cout << "case(2)" << endl;
      return 2;
    }

    if ( (AlmostEqual(p1, p3)) and (onsegment(p3, p4, p2)) ) // case (3)
    {
      //cout << "case(3)" << endl;
      return 3;
    }

    if (  (onsegment(p3, p4, p1)) and (onsegment(p3, p4, p2))
          and (p2 != p3) and (p2 != p4)
          and (p1 != p3) and (p1 != p4) ) // case (4)
    {
      //cout << "case(4)" << endl;
      return 4;
    }

    if ( (AlmostEqual(p2, p4)) and (onsegment(p3, p4, p1)) ) // case (5)
    {
      //cout << "case(5)" << endl;
      return 5;
    }

    if (  (onsegment(p3, p4, p1)) and (onsegment(p1, p2, p4))
          and (p2 != p3) and (p2 != p4)
          and (p1 != p3) and (p1 != p4) ) // case (6)
    {
      //cout << "case(6)" << endl;
      return 6;
    }

    if ( AlmostEqual(p1, p4) ) // case (7)
    {
      //cout << "case(11)" << endl;
      return 14;
    }

    if ( (AlmostEqual(p1, p3)) and (onsegment(p1, p2, p4)) ) // case (8)
    {
      //cout << "case(8)" << endl;
      return 7;
    }

    if (  (onsegment(p1, p2, p3)) and (onsegment(p1, p2, p4))
          and (p2 != p3) and (p2 != p4)
          and (p1 != p3) and (p1 != p4) ) // case (9)
    {
      //cout << "case(9)" << endl;
      return 8;
    }

    if ( (AlmostEqual(p2, p4)) and (onsegment(p1, p2, p3)) ) // case (10)
    {
      //cout << "case(10)" << endl;
      return 9;
    }
  }

  //cout << "GO HEEEEEEEEEEEERE" << endl;
  if ( ((AlmostEqual(d4, 0.0)) and (onsegment(p1, p2, p4))) and
     (!(AlmostEqual(p4, p1))) and (!(AlmostEqual(p4, p2))) )
    return 10;

  if ( ((AlmostEqual(d3, 0.0)) and (onsegment(p1, p2, p3))) and
    (!(AlmostEqual(p3, p1))) and (!(AlmostEqual(p3, p2))) )
    return 11;

  if ( ((AlmostEqual(d1, 0.0)) and (onsegment(p3, p4, p1))) and
    (!(AlmostEqual(p3, p1))) and (!(AlmostEqual(p4, p1))) )
    return 12;

  if ( ((AlmostEqual(d2, 0.0)) and (onsegment(p3, p4, p2))) and
  (!(AlmostEqual(p3, p2))) and (!(AlmostEqual(p4, p2))) )
    return 13;
  //cout << "other" << endl;
  return 14;
}

void MakeRealm::hsintersection(const HalfSegment& seg1,
                               const HalfSegment& seg2, Point& p)
{
  double m1, m2, a1, a2;

  //Precondition: the two half segments properly intersect

  if ( AlmostEqual(seg1.GetLeftPoint().GetX(), seg1.GetRightPoint().GetX()) )
  {
      m2 = (seg2.GetRightPoint().GetY() - seg2.GetLeftPoint().GetY()) /
           (seg2.GetRightPoint().GetX() - seg2.GetLeftPoint().GetX());
      a2 = seg2.GetLeftPoint().GetY() - (m2 * seg2.GetLeftPoint().GetX());
      p.Set(seg1.GetLeftPoint().GetX(), m2 * seg1.GetLeftPoint().GetX() + a2);
  }
  else if (AlmostEqual(seg2.GetLeftPoint().GetX(), seg2.GetRightPoint().GetX()))
  {
      m1 = (seg1.GetRightPoint().GetY() - seg1.GetLeftPoint().GetY()) /
           (seg1.GetRightPoint().GetX() - seg1.GetLeftPoint().GetX());
      a1 = seg1.GetLeftPoint().GetY() - (m1 * seg1.GetLeftPoint().GetX());
      p.Set(seg2.GetLeftPoint().GetX(), m1 * seg2.GetLeftPoint().GetX() + a1);
  } else {
      m1 = (seg1.GetRightPoint().GetY() - seg1.GetLeftPoint().GetY()) /
           (seg1.GetRightPoint().GetX() - seg1.GetLeftPoint().GetX());
      m2 = (seg2.GetRightPoint().GetY() - seg2.GetLeftPoint().GetY()) /
           (seg2.GetRightPoint().GetX() - seg2.GetLeftPoint().GetX());
      a1 = seg1.GetLeftPoint().GetY() - (m1 * seg1.GetLeftPoint().GetX());
      a2 = seg2.GetLeftPoint().GetY() - (m2 * seg2.GetLeftPoint().GetX());
      p.Set((a2 - a1) / (m1 - m2), ((m1 * a2) - (m2 * a1)) / (m1 - m2));
  }
}

void MakeRealm::insertpoint(list<Point>& values, const Point p)
{
  list<Point>::iterator it, pred;

  if ( values.empty() )
    values.push_front(p);
  else
  {
    it = find_if( values.begin(), values.end(), is_greater_than(p) );

    pred = it;

    if ( pred != values.begin() )
    {
      if ( (*(--pred)) != p )
      {
        values.insert(it, p);
      }
    }
    else
        values.push_front(p);
  }
}

bool MakeRealm::xoverlaps(const HalfSegment& hs1, const HalfSegment& hs2)
{
  if ( ((hs2.GetLeftPoint() > hs1.GetLeftPoint()) &&
        (hs2.GetLeftPoint() < hs1.GetRightPoint())) ||
       ((hs1.GetLeftPoint() > hs2.GetLeftPoint()) &&
        (hs1.GetLeftPoint() < hs2.GetRightPoint())) ||
       ((hs1.GetRightPoint() > hs2.GetLeftPoint()) &&
        (hs1.GetRightPoint() < hs2.GetRightPoint())) ||
       ((hs2.GetRightPoint() > hs1.GetLeftPoint()) &&
        (hs2.GetRightPoint() < hs1.GetRightPoint())) )
      return true;
    return false;
}

void MakeRealm::checksegmentsline(const HalfSegment& hs1,
  const HalfSegment& hs2, const int i, const int j,
  vector<HalfSegmentCheck>& vsc1)
{
  Point ispoint;

  switch ( intersects(hs1.GetLeftPoint(), hs1.GetRightPoint(),
                      hs2.GetLeftPoint(), hs2.GetRightPoint()) )
  {
    case 0: // cout << "Segments do not intersect, do nothing" << endl;
            break;

    case 1: // cout << "Segments intersect properly" << endl;
            hsintersection(hs1, hs2, ispoint);
            vsc1[i].splitsegment = true;
            insertpoint(vsc1[i].pointlist, ispoint);
            vsc1[j].splitsegment = true;
            insertpoint(vsc1[j].pointlist, ispoint);
            break;

    // cases 2 - 9 cover overlapping halfsegments

    case 2: // p1----p3----p2----p4 (2)
            // cout << "Segments overlap case(2)" << endl;
            vsc1[i].splitsegment = true;
            insertpoint(vsc1[i].pointlist, hs2.GetLeftPoint());
            vsc1[j].splitsegment = true;
            insertpoint(vsc1[j].pointlist, hs1.GetRightPoint());
            break;

    case 3: // p1,p3----p2----p4 (3)
            // cout << "Segments overlap case(3)" << endl;
            vsc1[j].splitsegment = true;
            insertpoint(vsc1[j].pointlist, hs1.GetRightPoint());
            break;

        case 4: // p3----p1----p2----p4 (4)
                // cout << "Segments overlap case(4)" << endl;
                vsc1[j].splitsegment = true;
                insertpoint(vsc1[j].pointlist, hs1.GetLeftPoint());
                insertpoint(vsc1[j].pointlist, hs1.GetRightPoint());
                break;

        case 5: // p3----p1----p2,p4 (5)
                // cout << "Segments overlap case(5)" << endl;
                vsc1[j].splitsegment = true;
                insertpoint(vsc1[j].pointlist, hs1.GetLeftPoint());
                break;

        case 6: // p3----p1----p4----p2 (6)
                // cout << "Segments overlap case(6)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, hs2.GetRightPoint());
                vsc1[j].splitsegment = true;
                insertpoint(vsc1[j].pointlist, hs1.GetLeftPoint());
                break;

        case 7: // p1,p3----p4----p2 (8)
                // cout << "Segments overlap case(8)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, hs2.GetRightPoint());
                break;

        case 8: // p1----p3----p4----p2 (9)
                // cout << "Segments overlap case(9)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, hs2.GetLeftPoint());
                insertpoint(vsc1[i].pointlist, hs2.GetRightPoint());
                break;

        case 9: // p1----p3----p2,p4 (10)
                // cout << "Segments overlap case(10)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, hs2.GetLeftPoint());
                break;

        case 10: // p1---p4----p2 (12)
                 //       |
                 //      p3
                 // cout << "Segments overlap case(12)" << endl;
                 vsc1[i].splitsegment = true;
                 insertpoint(vsc1[i].pointlist, hs2.GetRightPoint());
                 break;

        case 11: //      p4 (13)
                 //       |
                 // p1---p3----p2
                 // cout << "Segments overlap case(13)" << endl;
                 vsc1[i].splitsegment = true;
                 insertpoint(vsc1[i].pointlist, hs2.GetLeftPoint());
                 break;

        case 12: //      p2 (14)
                 //       |
                 // p3---p1----p4
                 // cout << "Segments overlap case(14)" << endl;
                 vsc1[j].splitsegment = true;
                 insertpoint(vsc1[j].pointlist, hs1.GetLeftPoint());
                 break;

        case 13: // p3---p2----p4 (15)
                 //       |
                 //      p1
                 // cout << "Segments overlap case(15)" << endl;
                 vsc1[j].splitsegment = true;
                 insertpoint(vsc1[j].pointlist, hs1.GetRightPoint());
                 break;

        // case 14 covers overlapping half segments, where nothing
        // has to be done

        case 14: // p1----p2,p3----p4 (1)
                 // p3----p4,p1----p2 (7)
                 // p1,p3----p2,p4 (11)
                 // cout << "Do nothing case(1, 7, 11)" << endl;
                 break;

        default: cout << "should not happen " << endl;
      }
}


void MakeRealm::checksegments(const HalfSegment& hs1, const HalfSegment& hs2,
             const int i, const int j,
             vector<HalfSegmentCheck>& vsc1, vector<HalfSegmentCheck>& vsc2)
{
  Point ispoint;

  //if ( intersects(hs1.GetLeftPoint(), hs1.GetRightPoint(),
                   //hs2.GetLeftPoint(), hs2.GetRightPoint()) )
                   //cout << " they intersect " << endl;

  //cout << " they intersect" << intersects(hs1.GetLeftPoint(),
  //hs1.GetRightPoint(),
  //hs2.GetLeftPoint(), hs2.GetRightPoint()) << endl;

  switch ( intersects(hs1.GetLeftPoint(), hs1.GetRightPoint(),
                      hs2.GetLeftPoint(), hs2.GetRightPoint()) )
  {
    case 0: //cout << "Segments do not intersect, do nothing" << endl;
            break;

    case 1: //cout << "Segments intersect properly" << endl;
            hsintersection(hs1, hs2, ispoint);
            vsc1[i].splitsegment = true;
            insertpoint(vsc1[i].pointlist, ispoint);
            vsc2[j].splitsegment = true;
            insertpoint(vsc2[j].pointlist, ispoint);
            break;

    // cases 2 - 9 cover overlapping halfsegments

    case 2: // p1----p3----p2----p4 (2)
            //cout << "Segments overlap case(2)" << endl;
            vsc1[i].splitsegment = true;
            insertpoint(vsc1[i].pointlist, hs2.GetLeftPoint());
            vsc2[j].splitsegment = true;
            insertpoint(vsc2[j].pointlist, hs1.GetRightPoint());
            break;

    case 3: // p1,p3----p2----p4 (3)
            //cout << "Segments overlap case(3)" << endl;
            vsc2[j].splitsegment = true;
            insertpoint(vsc2[j].pointlist, hs1.GetRightPoint());
            break;

        case 4: // p3----p1----p2----p4 (4)
                //cout << "Segments overlap case(4)" << endl;
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, hs1.GetLeftPoint());
                insertpoint(vsc2[j].pointlist, hs1.GetRightPoint());
                break;

        case 5: // p3----p1----p2,p4 (5)
                //cout << "Segments overlap case(5)" << endl;
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, hs1.GetLeftPoint());
                break;

        case 6: // p3----p1----p4----p2 (6)
                //cout << "Segments overlap case(6)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, hs2.GetRightPoint());
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, hs1.GetLeftPoint());
                break;

        case 7: // p1,p3----p4----p2 (8)
                //cout << "Segments overlap case(8)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, hs2.GetRightPoint());
                break;

        case 8: // p1----p3----p4----p2 (9)
                //cout << "Segments overlap case(9)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, hs2.GetLeftPoint());
                insertpoint(vsc1[i].pointlist, hs2.GetRightPoint());
                break;

        case 9: // p1----p3----p2,p4 (10)
                //cout << "Segments overlap case(10)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, hs2.GetLeftPoint());
                break;

        case 10: // p1---p4----p2 (12)
                 //       |
                 //      p3
                 //cout << "Segments overlap case(12)" << endl;
                 vsc1[i].splitsegment = true;
                 insertpoint(vsc1[i].pointlist, hs2.GetRightPoint());
                 break;

        case 11: //      p4 (13)
                 //       |
                 // p1---p3----p2
                 //cout << "Segments overlap case(13)" << endl;
                 vsc1[i].splitsegment = true;
                 insertpoint(vsc1[i].pointlist, hs2.GetLeftPoint());
                 break;

        case 12: //      p2 (14)
                 //       |
                 // p3---p1----p4
                 //cout << "Segments overlap case(14)" << endl;
                 vsc2[j].splitsegment = true;
                 insertpoint(vsc2[j].pointlist, hs1.GetLeftPoint());
                 break;

        case 13: // p3---p2----p4 (15)
                 //       |
                 //      p1
                 //cout << "Segments overlap case(15)" << endl;
                 vsc2[j].splitsegment = true;
                 insertpoint(vsc2[j].pointlist, hs1.GetRightPoint());
                 break;

        // case 14 covers overlapping half segments, where nothing
        // has to be done

        case 14: // p1----p2,p3----p4 (1)
                 // p3----p4,p1----p2 (7)
                 // p1,p3----p2,p4 (11)
                 //cout << "Do nothing case(1, 7, 11)" << endl;
                 break;

        default: cout << "should not happen " << endl;
      }
}

void MakeRealm::dorealm2(const vector<HalfSegment>& vs1,
        const vector<HalfSegment>& vs2,const bool isline1, const bool isline2,
        vector<HalfSegmentCheck>& vsc1, vector<HalfSegmentCheck>& vsc2)
{
  unsigned int i = 0, j = 0, k, l, hscurrindex, hscurrindexi, hscurrindexj;
  HalfSegment hs1, hs2, hscurr, hscurri, hscurrj;
  State status;

  //if ( true ) printsegvector(vs1);
  //if ( true ) printsegvector(vs2);
  while ( (i < vs1.size()) || (j < vs2.size()) )
  {
    if (i < vs1.size() && j < vs2.size() )
    {
      //cout << "if1" << endl;
      hs1 = vs1[i];
      hs2 = vs2[j];
      if ( AlmostEqual(hs1.GetLeftPoint(), hs2.GetLeftPoint()) &&
           AlmostEqual(hs1.GetRightPoint(), hs2.GetRightPoint()) )
      {
        hscurrindexi = i;
        i++;
        hscurrindexj = j;
        j++;
        hscurri = hs1;
        hscurrj = hs2;
        status = BOTH;
        //cout << "STATUS IS BOTH" << endl;
      }
      else if ( hs1 < hs2)
      {
        //cout << "hs1 < hs2" << endl;
        hscurrindex = i;
        i++;
        hscurr = hs1;
        status = FIRST;
      }
      else
      {
        //cout << "hs1 > hs2" << endl;
        hscurrindex = j;
        j++;
        hscurr = hs2;
        status = SECOND;
      }
      //cout << hscurr << endl;
      //if (status == FIRST) cout << "FIRST1" << endl;
      //else cout << "SECOND1" << endl;
      if ( status == BOTH )
      {
        k = hscurrindexj + 1;
        if ( isline1 )
        {
          l = hscurrindexi + 1;
          while ( (l < vs1.size()) && (xoverlaps(hscurri, vs1[l])) )
          {
            checksegmentsline(hscurri, vs1[l], hscurrindexi, l, vsc1);
            l++;
          }
        }
        if ( isline2 )
        {
          l = hscurrindexj + 1;
          while ( (l < vs2.size()) && (xoverlaps(hscurrj, vs2[l])) )
          {
            checksegmentsline(hscurrj, vs2[l], hscurrindexj, l, vsc2);
            l++;
          }
        }
        while ( (k < vs2.size()) && (xoverlaps(hscurri, vs2[k])) )
        {
          checksegments(hscurri, vs2[k], hscurrindexi, k, vsc1, vsc2);
          //cout << "index ki = " << k << endl;
          k++;
        }
        k = hscurrindexi + 1;
        while ( (k < vs1.size()) && (xoverlaps(hscurrj, vs1[k])) )
        {
          checksegments(hscurrj, vs1[k], hscurrindexj, k, vsc1, vsc2);
          //cout << "index kj = " << k << endl;
          k++;
        }

        //cout << "TREAT BOTH CASE" << endl;
      }
      else if ( status == FIRST )
      {
        k = j;
        /*while ( (jleft < vs2.size()) && (!(xoverlaps(hscurr, vs2[jleft]))) )
        {
          cout << "move left pointer" << endl;
          jleft++;
        }

        k = jleft;*/


        //realm the lines
        if ( isline1 )
        {
          l = hscurrindex + 1;
          while ( (l < vs1.size()) && (xoverlaps(hscurr, vs1[l])) )
          {
            //cout << "realm the line first" << l << endl;
            checksegmentsline(hscurr, vs1[l], hscurrindex, l, vsc1);
            //printsegcheckvector(vsc1);
            //printsegcheckvector(vsc2);
            l++;
          }
        }

        while ( (k < vs2.size()) && (xoverlaps(hscurr, vs2[k])) )
        {
          //cout << "realm the two segments " << k << endl;
          checksegments(hscurr, vs2[k], hscurrindex, k, vsc1, vsc2);
          //printsegcheckvector(vsc1);
          //printsegcheckvector(vsc2);
          k++;
        }
        //cout << endl;
      }
      else if ( status == SECOND )
      {
        k = i;
        /*while ( (ileft < vs1.size()) && (!(xoverlaps(hscurr, vs1[ileft]))) )
        {
          cout << "move left pointer" << endl;
          ileft++;
        }

        k = ileft;*/

        //realm the lines
        if ( isline2 )
        {
          l = hscurrindex + 1;
          //if ( l < vs2.size() ) cout << "l < vs2.size()" << endl;
          //else cout << "l >= vs2.size()" << endl;
          //if ( xoverlaps(hscurr, vs2[l]) )
          //cout << "xoverlaps(hscurr, vs2[l])" << endl;
          while ( (l < vs2.size()) && (xoverlaps(hscurr, vs2[l])) )
          {
            //cout << "realm the line second" << l << endl;
            checksegmentsline(hscurr, vs2[l], hscurrindex, l, vsc2);
            //printsegcheckvector(vsc1);
            //printsegcheckvector(vsc2);
            l++;
          }
        }

          //if ( k < vs1.size() ) cout << "k < vs1.size()" << endl;
          //else cout << "k >= vs1.size()" << endl;
          //if ( xoverlaps(hscurr, vs1[k]) )
          //cout << "xoverlaps(hscurr, vs1[k])" << endl;
          //else cout << "not xoverlaps(hscurr, vs1[k])" << endl;
        while ( (k < vs1.size()) && (xoverlaps(hscurr, vs1[k])) )
        {
          //cout << "realm the two segments " << k << endl;
          //cout << hscurr << endl;
          //cout << vs1[k] << endl;
          checksegments(vs1[k], hscurr, k, hscurrindex, vsc1, vsc2);
          //checksegments(hscurr, vs1[k], k, hscurrindex, vsc1, vsc2);
          //printsegcheckvector(vsc1);
          //printsegcheckvector(vsc2);
          k++;
        }
        //cout << endl;
      }
    }
    else if ( i < vs1.size() )
    {
      //cout << "if2" << endl;
      hscurr = vs1[i];
      hscurrindex = i;
      i++;
      status = FIRST;
      //cout << hscurr << endl;
      //if (status == FIRST) cout << "FIRST2" << endl << endl;
      //else cout << "SECOND2" << endl << endl;

        //realm the lines
        if ( isline1 )
        {
          l = hscurrindex + 1;
          while ( (l < vs1.size()) && (xoverlaps(hscurr, vs1[l])) )
          {
            //cout << "realm the line outside first " << endl;
            checksegmentsline(hscurr, vs1[l], hscurrindex, l, vsc1);
            //printsegcheckvector(vsc1);
            //printsegcheckvector(vsc2);
            l++;
          }
        }

      k = j;
      while ( (k < vs2.size()) && (xoverlaps(hscurr, vs2[k])) )
      {
        //cout << "realm the two segments " << k << endl;
        checksegments(hscurr, vs2[k], hscurrindex, k, vsc1, vsc2);
        //printsegcheckvector(vsc1);
        //printsegcheckvector(vsc2);
        k++;
      }
      //cout << endl;
    }
    else if ( j < vs2.size() )
    {
      //cout << "if3" << endl;
      hscurr = vs2[j];
      hscurrindex = j;
      j++;
      status = SECOND;
      //cout << hscurr << endl;
      //if (status == FIRST) cout << "FIRST3" << endl << endl;
      //else cout << "SECOND3" << endl << endl;

        //realm the lines
        if ( isline2 )
        {
          l = hscurrindex + 1;
          while ( (l < vs2.size()) && (xoverlaps(hscurr, vs2[l])) )
          {
            //cout << "realm the line outside" << l << endl;
            checksegmentsline(hscurr, vs2[l], hscurrindex, l, vsc2);
            //printsegcheckvector(vsc1);
            //printsegcheckvector(vsc2);
            l++;
          }
        }

      k = i;
      while ( (k < vs1.size()) && (xoverlaps(hscurr, vs1[k])) )
      {
        //cout << "realm the two segments " << k << endl;
        checksegments(vs1[k], hscurr, k, hscurrindex, vsc1, vsc2);
        //printsegcheckvector(vsc1);
        //printsegcheckvector(vsc2);
        k++;
      }
      //cout << endl;
    }
  }
}

void MakeRealm::dorealm(const vector<HalfSegment>& vs1,
                        const vector<HalfSegment>& vs2,
             vector<HalfSegmentCheck>& vsc1, vector<HalfSegmentCheck>& vsc2)
{
  Point ispoint;

  for (unsigned int i = 0; i < vs1.size(); i++)
  {
    for (unsigned int j = 0; j < vs2.size(); j++)
    {
      switch ( intersects(vs1[i].GetLeftPoint(), vs1[i].GetRightPoint(),
                          vs2[j].GetLeftPoint(), vs2[j].GetRightPoint()) )
      {
        case 0: // cout << "Segments do not intersect, do nothing" << endl;
                break;

        case 1: // cout << "Segments intersect properly" << endl;
                hsintersection(vs1[i], vs2[j], ispoint);
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, ispoint);
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, ispoint);
                break;

        // cases 2 - 9 cover overlapping halfsegments

        case 2: // p1----p3----p2----p4 (2)
                // cout << "Segments overlap case(2)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, vs2[j].GetLeftPoint());
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, vs1[i].GetRightPoint());
                break;

        case 3: // p1,p3----p2----p4 (3)
                // cout << "Segments overlap case(3)" << endl;
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, vs1[i].GetRightPoint());
                break;

        case 4: // p3----p1----p2----p4 (4)
                // cout << "Segments overlap case(4)" << endl;
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, vs1[i].GetLeftPoint());
                insertpoint(vsc2[j].pointlist, vs1[i].GetRightPoint());
                break;

        case 5: // p3----p1----p2,p4 (5)
                // cout << "Segments overlap case(5)" << endl;
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, vs1[i].GetLeftPoint());
                break;

        case 6: // p3----p1----p4----p2 (6)
                // cout << "Segments overlap case(6)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, vs2[j].GetRightPoint());
                vsc2[j].splitsegment = true;
                insertpoint(vsc2[j].pointlist, vs1[i].GetLeftPoint());
                break;

        case 7: // p1,p3----p4----p2 (8)
                // cout << "Segments overlap case(8)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, vs2[j].GetRightPoint());
                break;

        case 8: // p1----p3----p4----p2 (9)
                // cout << "Segments overlap case(9)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, vs2[j].GetLeftPoint());
                insertpoint(vsc1[i].pointlist, vs2[j].GetRightPoint());
                break;

        case 9: // p1----p3----p2,p4 (10)
                // cout << "Segments overlap case(10)" << endl;
                vsc1[i].splitsegment = true;
                insertpoint(vsc1[i].pointlist, vs2[j].GetLeftPoint());
                break;

        case 10: // p1---p4----p2 (12)
                 //       |
                 //      p3
                 // cout << "Segments overlap case(12)" << endl;
                 vsc1[i].splitsegment = true;
                 insertpoint(vsc1[i].pointlist, vs2[j].GetRightPoint());
                 break;

        case 11: //      p4 (13)
                 //       |
                 // p1---p3----p2
                 // cout << "Segments overlap case(13)" << endl;
                 vsc1[i].splitsegment = true;
                 insertpoint(vsc1[i].pointlist, vs2[j].GetLeftPoint());
                 break;

        case 12: //      p2 (14)
                 //       |
                 // p3---p1----p4
                 // cout << "Segments overlap case(14)" << endl;
                 vsc2[j].splitsegment = true;
                 insertpoint(vsc2[j].pointlist, vs1[i].GetLeftPoint());
                 break;

        case 13: // p3---p2----p4 (15)
                 //       |
                 //      p1
                 // cout << "Segments overlap case(15)" << endl;
                 vsc2[j].splitsegment = true;
                 insertpoint(vsc2[j].pointlist, vs1[i].GetRightPoint());
                 break;

        // case 14 covers overlapping half segments, where nothing
        // has to be done

        case 14: // p1----p2,p3----p4 (1)
                 // p3----p4,p1----p2 (7)
                 // p1,p3----p2,p4 (11)
                 // cout << "Do nothing case(1, 7, 11)" << endl;
                 break;

        default: cout << "should not happen " << endl;
      }
    }
  }
}

void MakeRealm::createrealmedobject(const vector<HalfSegment>& s,
                    vector<HalfSegmentCheck>& sc, vector<HalfSegment>& res)
{
  HalfSegment rseg;
  Point lp, rp;
  int faceno, cycleno;
  bool insideAbove;

  for (unsigned int u = 0; u < s.size(); u++)
  {
    faceno = s[u].attr.faceno;
    cycleno = s[u].attr.cycleno;
    insideAbove = s[u].attr.insideAbove;

    if ( sc[u].splitsegment == false )
            res.push_back(s[u]);
    else
    {
      lp = s[u].GetLeftPoint();
      list<Point>::iterator it;
      for (it = sc[u].pointlist.begin(); it != sc[u].pointlist.end(); ++it)
      {
        rp = *it;
        if(!AlmostEqual(lp,rp)){
           rseg.Set(true, lp, rp);
           rseg.attr.faceno = faceno;
           rseg.attr.cycleno = cycleno;
           rseg.attr.insideAbove = insideAbove;
           res.push_back(rseg);
           lp = rp;
        }
      }
      rp = s[u].GetRightPoint();

      if(!AlmostEqual(lp,rp)){
          rseg.Set(true, lp, rp);
          rseg.attr.faceno = faceno;
          rseg.attr.cycleno = cycleno;
          rseg.attr.insideAbove = insideAbove;
          res.push_back(rseg);
      }
    }
  }
}

template <class T, class U, class V, class W>
void MakeRealm::REALM2(const T* obj1, const U* obj2, const bool isline1,
                       const bool isline2, V* result1, W* result2)
{
   vector<HalfSegment> vl1, vl2, vl1res, vl2res;
   int counter;

   if ( !obj1->BoundingBox().IntersectsUD(obj2->BoundingBox()) )
   {
     if ( PSA_DEBUG ) cout << "Bounding boxes do not intersect." << endl;
     *result1 = *obj1;
     *result2 = *obj2;
   }
   else
   {
     if ( PSA_DEBUG ) cout << "Bounding boxes intersect." << endl;
     readhalfsegments(vl1, obj1);
     if ( PSA_DEBUG ) printsegvector(vl1);
     readhalfsegments(vl2, obj2);
     if ( PSA_DEBUG ) printsegvector(vl2);

     HalfSegmentCheck sc;
     vector<HalfSegmentCheck> scl1(vl1.size(), sc), scl2(vl2.size(), sc);
     if ( kindofrealm == OVERLAPPING )
       dorealm2(vl1, vl2, isline1, isline2, scl1, scl2);
     else dorealm(vl1, vl2, scl1, scl2);
     if ( PSA_DEBUG ) printsegcheckvector(scl1);
     if ( PSA_DEBUG ) printsegcheckvector(scl2);

     createrealmedobject(vl1, scl1, vl1res);
     if ( PSA_DEBUG ) printsegvector(vl1res);
     createrealmedobject(vl2, scl2, vl2res);
     if ( PSA_DEBUG ) printsegvector(vl2res);

     // create first region object
     result1->Clear();
     result1->StartBulkLoad();
     counter=0;
     while (! vl1res.empty() )  {
        HalfSegment hs = vl1res.back();
        hs.attr.edgeno = counter;
        hs.SetLeftDomPoint(true);
        *result1 += hs;
        hs.SetLeftDomPoint(false);
        *result1 += hs;
        counter++;
        vl1res.pop_back();
     }
     result1->EndBulkLoad();

     //create second region object
     result2->Clear();
     result2->StartBulkLoad();
     counter=0;
     while (! vl2res.empty() )  {
        HalfSegment hs = vl2res.back();
        hs.attr.edgeno = counter;
        hs.SetLeftDomPoint(true);
        *result2 += hs;
        hs.SetLeftDomPoint(false);
        *result2 += hs;
        counter++;
        vl2res.pop_back();
     }
     result2->EndBulkLoad();
   }
}

/*
function gets a HalfSegment, creates two XEvents ( both endpoints) and inserts
them into priority queue. An ~-Segment~-objects was created and inserted in the
array with all segments.

*/

void MakeRealm::PrepareCHS ( bool No1, const HalfSegment& hs,
   PQueue& pqu, Segment segs[], int counter)
{
   Coord xl, xr, yl, yr;
   xl=hs.GetLeftPoint().GetX();
   yl=hs.GetLeftPoint().GetY();
   xr=hs.GetRightPoint().GetX();
   yr=hs.GetRightPoint().GetY();
   // only left HalfSegments are used
   // build up array with all HalfSegments of Region
   Segment se (No1,hs);
   segs[counter] = se;
   // insert XEvents in PQueue
   // insert a vertical Segment
   if ( xl==xr)
   {
      // insert bottom of vertical hs
      XEvent ev1(xl, yl ,counter, 0.0, 0.0, verticalSegment);
      pqu.insert(ev1);
   }
   // insert a non-vertical hs
   else
   { // calculate k and a
      double k = (yr - yl) / (xr - xl) ;
      double a = yl - k*xl;
      // insert left end of hs
      XEvent ev2(xl, yl, counter, k, a, leftSegment);
      //cout << "left" << ev2 << endl;
      pqu.insert(ev2);
       // insert right end of hs
      XEvent ev3(xr, yr, counter, k, a, rightSegment);
      //cout << "right" << ev3 << endl;
      pqu.insert(ev3);
      //cout << pqu << endl;
      //while(!pqu.isEmpty()) {
       //cout << pqu.getFirstAndDelete().GetX() << endl;
      //}
   }
}

/*
This function performs plane-sweep of realmisation process. The priority queue
(~PQueue~) was handle until all XEvents are passed.

*/
void MakeRealm::PerformPlaneSweep(PQueue& pq, Segment segs[],
   list<HalfSegment>& list1, list<HalfSegment>& list2,
   const int counter)
{
   // initalisations
   set<int> mi;
   Point oldP(true,0,0);
   VList vlist;
   VStructure vs;
   StatusLine sl;
   XEvent event;
   bool insertedCurrentSegment=false;
   
#ifdef PS_DEBUG
   int i=0;
   int j=0;
#endif

   if ( pq.isEmpty())  return;
   Coord sweepline=0, oldsweep=0;
   // work out the ~PQueue~-object
   //cout << pq << endl;
   while ( (! pq.isEmpty()) || (vlist.Size() >= 1) )  {
     // take the first XEvent
      if ( !pq.isEmpty() ) {
        event = pq.getFirstAndDelete();
        insertedCurrentSegment = false;
      }

#ifdef PS_DEBUG
      i += 1;
      cout << "Loop1: " << i << endl;
      cout << "Size: " << vlist.Size();
      if (pq.isEmpty()) cout << " PQEmpty ";
      else cout << " PQNOTEmpty ";
      else cout <<  "insertedCurrentSegment is false " << endl;
#endif

      if ( ((sweepline != event.GetX()) || (vlist.Size() > 1)) ||
                     ( ( (pq.isEmpty()) && (vlist.Size() == 1) )
                          && insertedCurrentSegment ) )  { // new sweepline
         oldsweep = sweepline;
         // handle all vertical HalfSegments at old sweepline
         if ( !vlist.IsEmpty() ) {
                 //cout << "Size VList: " <<  vlist.Size() << endl;
            list<Segment> newlist = vlist.processAll(sweepline,vs,
               sl,segs);
            list<Segment>::iterator iter = newlist.begin();
            while (iter != newlist.end()) {
               Segment segment = *iter;
               //cout << "Segment: " << segment << endl;
               segment.CHSInsert(list1,list2);
               ++iter;
            }
            vlist.Clear();
            vs.Clear();
         }
         if (pq.isEmpty() && insertedCurrentSegment) return;
         sweepline = event.GetX();
      }
      Segment seg = segs[event.GetFirst()]; // get segment - entry
      // XEvent for left end of HalfSegment
      SSSEntry entry (event.GetFirst(),event.GetSlope(), event.GetA());
      if (event.GetKind() == leftSegment)   {
         sl.Insert(event.GetX(),entry, segs, pq);
         if ( vs.IsDefined())   vs.Insert(event.GetY());
      }
      // XEvent for right end of HalfSegment
      else if (event.GetKind() == rightSegment) {
         sl.Delete(event.GetX(), oldsweep, entry, segs, pq);
         if ( vs.IsDefined())   vs.Insert(event.GetY());
         segs[event.GetFirst()].CHSInsert(list1,list2);
         
#ifdef PS_DEBUG
         list<HalfSegment>::iterator it;
         it = list1.begin();
         while(it != list1.end()) cout << *it++ << endl;
         cout << endl;
         it = list2.begin();
         while(it != list2.end()) cout << *it++ << endl;
         cout << endl;
#endif

         insertedCurrentSegment = true;
      }
      // XEvent for split one HalfSegment
      else if (event.GetKind() == split) {
         Point point (true, event.GetX(), event.GetY() );
         Segment new1 ( seg.GetIn1(), seg.GetCHS());
         if ( (point != new1.GetLeftPoint()) &&
              (point != new1.GetRightPoint() ) ) {
            new1.SetRightPoint(point);
            new1.CHSInsert(list1,list2);
            seg.SetLeftPoint(point);
            segs[event.GetFirst()] = seg;
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
               new1.SetRightPoint(p);
               new1.CHSInsert(list1,list2);
               seg.SetLeftPoint(p);    segs[event.GetFirst()] = seg;
               mi.insert (event.GetFirst() );
               SSSEntry entry2(event.GetFirst(), segs);
               sl.Insert(event.GetX(), entry2, segs, pq);
            }
            if ( mi.find(event.GetSecond()) == mi.end()  ){
               // split second segment
               SSSEntry entry1 (event.GetSecond(), segs);
               sl.Delete(event.GetX(), oldsweep, entry1, segs, pq);
               Segment new2 (seg2.GetIn1(),seg2.GetCHS());
               new2.SetRightPoint(p);
               new2.CHSInsert(list1,list2);
               seg2.SetLeftPoint(p);   segs[event.GetSecond()] = seg2;
               mi.insert (event.GetSecond());
               SSSEntry entry2(event.GetSecond(),segs);
               sl.Insert(event.GetX(),entry2, segs, pq);
            }
         }
         else {  // first intersection-event in this point
            Segment seg2 = segs[event.GetSecond()];
            Segment new1 ( seg.GetIn1(), seg.GetCHS());
            if(AlmostEqual(new1.GetLeftPoint(), p))
              continue;
            new1.SetRightPoint(p);
            Segment new2 (seg2.GetIn1(),seg2.GetCHS());
            if(AlmostEqual(new2.GetLeftPoint(), p))
              continue;
            new2.SetRightPoint(p);
            new1.CHSInsert(list1,list2);
            new2.CHSInsert(list1,list2);
            HalfSegment hstest(seg.GetCHS());
            seg.SetLeftPoint(p);       segs[event.GetFirst()] = seg;
            seg2.SetLeftPoint(p);      segs[event.GetSecond()] = seg2;
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
#ifdef PS_DEBUG
        j += 1;
        cout << "Loop2: " << j << endl;
#endif
        vs.SetDefined(true);      // build up VStructure
        vlist.Insert(segs[event.GetFirst()]);
        insertedCurrentSegment = true;
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
   SEntry(HalfSegment& inch);
   SEntry(SEntry* in);
   SEntry(const SEntry& s);
   void Set(const SEntry& in);
   const double GetY(Coord x) const;
   int GetU() const;
   int GetO() const;
   HalfSegment GetCHS() const;
   void SetU(int newU);
   void SetO(int newO);
   int Less (const SEntry &ev2, const Coord x, const SEntry& oldev,
       BinTreeNode<SEntry>* oldnode ) const;
   bool Equal (const SEntry& in2) const;
   SEntry& operator= (const SEntry& in);

public:  // eigentlich private
   double GetSlope() const;
   double GetA() const;

private:
   HalfSegment ch;
   double slope;
   double a;
   int u;
   int o;



};

ostream& operator<<(ostream &os, const SEntry& en)
{
  return (os   <<" CHS("<<en.GetCHS().IsLeftDomPoint()
               <<") ("<< en.GetCHS().GetLeftPoint() << " "
               << en.GetCHS().GetRightPoint() <<")  u=" << en.GetU()
               << "  o=" <<en.GetO() );
}

/*
4.1.1 Constructors

*/
SEntry::SEntry(SEntry* in)
{
   ch = in-> ch;
   slope = in->slope;
   a = in->a;
   o = in -> o;
   u = in ->u;
}

SEntry::SEntry(const SEntry& in)
{
   ch = in.ch;
   slope = in.slope;
   a = in.a;
   o = in.o;
   u = in.u;
}

SEntry::SEntry(HalfSegment& inch)
{
   ch = inch;
   Coord xl, xr, yl, yr;
   xl = ch.GetLeftPoint().GetX();
   xr = ch.GetRightPoint().GetX();
   yl = ch.GetLeftPoint().GetY();
   yr = ch.GetRightPoint().GetY();
   double ink = (yr - yl) / (xr - xl) ;
   double ina = yl - ink*xl;
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

void SEntry::SetU (int newU)            { u = newU; }
void SEntry::SetO (int newO)            { o = newO; }

/*
4.2.3 Functions for reading property values

*/
HalfSegment SEntry::GetCHS() const     {return ch;}
int SEntry::GetU() const                {return u;}
int SEntry::GetO() const                {return o;}
double SEntry::GetSlope() const         {return slope;}
double SEntry::GetA() const             {return a;}

/*
4.2.4 Functions to compare two entries

*/
/*
calculate the y-value at the sweep  line, to insert at the right order

*/
const double SEntry::GetY(Coord x) const  {
   Coord res;
   bool end = false;
   if (ch.GetLeftPoint().GetX() == x)
      { end = true; res = ch.GetLeftPoint().GetY(); }
   else if (ch.GetRightPoint().GetX() == x)
      { end = true; res = ch.GetRightPoint().GetY(); }
   if ( end) {
      double y = res ;
      return y;
   }
     else {
      double xv = x ;
      return ( slope*xv + a);
   }
}

bool SEntry::Equal (const SEntry& in2) const
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
int SEntry::Less (const SEntry& in2, const Coord x, const
   SEntry& oldev, BinTreeNode<SEntry>* oldnode) const
{
   //cout << " test Sentry in Less   1. CHS:" << endl;
 //       << GetCHS() << "     2.CHS: " << in2.GetCHS() << endl; ;
   if ( Equal(in2) )     return 0;
   if (GetCHS().GetLeftPoint().GetX() == x &&
      in2.GetCHS().GetLeftPoint().GetX() == x) {
   //   cout << " beide starten am gleichen punkt " << endl;
      double y1 = GetCHS().GetLeftPoint().GetY();
      double y2 = in2.GetCHS().GetLeftPoint().GetY();
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
//             cout << "very close" << endl;
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
   else if (GetCHS().GetLeftPoint().GetX() == x) {
 //     cout << " einzufuegendes startet an x-Koordinate " ;
      double test = GetCHS().GetLeftPoint().GetY() - in2.GetY(x);
      if ( test > 0.000001 || test < -0.000001) {
   //      cout << " testet y-Vergleich" << endl;
         if (test < 0)           return -1;
         else if ( test > 0 )    return 1;
      }
      else {  //  |test| < 0.000001 very close
//          cout << " very close" << endl; ;
         if (GetCHS().attr.faceno == oldev.GetCHS().attr.faceno &&
         GetCHS().attr.cycleno == oldev.GetCHS().attr.cycleno &&
         ((GetCHS().attr.edgeno-oldev.GetCHS().attr.edgeno)==1 ||
         (GetCHS().attr.edgeno-oldev.GetCHS().attr.edgeno)==-1)) {
//          cout << " an alten segmenten anhaengen " << endl;
//          if (oldnode !=0 && oldnode->GetEntry().GetCHS() !=0)
//             cout <<" alter Knoten "<< oldnode->GetEntry().GetCHS() ;
//          if (oldnode != 0 && oldnode->GetPred() != 0 )
//             cout << "  vorg�ger von oldnode " <<
//             oldnode->GetPred()->GetEntry().GetCHS();
//          cout << endl;
            if  (oldnode!=0 &&  oldnode->GetEntry().GetCHS()
                ==in2.GetCHS())   return 1;
            else if (oldnode != 0 && oldnode->GetPred() != 0 &&
               oldnode->GetPred()->GetEntry().GetCHS() == in2.GetCHS())
               return 1;
            else if (oldnode == 0)  return 1;
         }
//       cout << " keine alten vorhanden" << endl;
         if (GetSlope() < in2.GetSlope())  {//cout << " slope < " <<     endl;
         return -1;}
         else  return 1;
      } // |test|  < 0.0001
   }  // else
   else if ( GetCHS().GetRightPoint().GetX()==x &&
      in2.GetCHS().GetRightPoint().GetX() ==x ){
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

SEntry& SEntry::operator=( const SEntry& e )
{
  Set(e);
  return *this;
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
SLine::SLine()                          {BinTree<SEntry> qu;}
bool SLine::IsEmpty()                   { return (qu.IsEmpty() ); }
Coord SLine::GetX()                     { return x; }
int SLine::GetSize()                    {return qu.GetCount(); }
void SLine::SetX(const Coord newx)      { x = newx; }

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
      //cout << "Node # 0" << endl;
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
         HalfSegment hs = ev.GetCHS();
         double abstand =
             ((hs.GetLeftPoint().GetX() - hs.GetRightPoint().GetX())*
              (hs.GetLeftPoint().GetX() - hs.GetRightPoint().GetX()) ) +
             ((hs.GetLeftPoint().GetY() - hs.GetRightPoint().GetY()) *
              (hs.GetLeftPoint().GetY() - hs.GetRightPoint().GetY())) ;
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

The algorithm computes segment classifications like described in ROSE-Algebra. Then for each operator different criteria are used to find the right segments to build up the result.
For both arguments the first segment was selected. If this segment is a left segment, it was inserted in sweep line. The segment classification was computed. If the segment belongs to a line, the segment was deleted at once. If the selected segment was a right segment of a region, it is deleted from the sweep line. If the segment classification has the searched vlaues it is added into result.

*/

class MakeOp
{
public:
   MakeOp() {};
   ~MakeOp() {};
   void Intersection(const Region* reg1, const Region* reg2,Region* result);
   void Intersection(const Region* reg, const Line* line,Line* result);
   void Intersection(const Line* reg1, const Line* reg2i,Line* result);
   void Union(const Region* reg1, const Region* reg2,Region* result);
   void Union(const Line* line1, const Line* line2,Line* result);
   void Minus(const Region* reg1, const Region* reg2,Region* result);
   void Minus(const Line* line, const Region* reg,Line* result);
   void Minus(const Line* line1, const Line* line2i,Line* result);
   bool Intersects(const Region* reg1, const Region* reg2);
   bool Intersects(const Line* line1, const Line* line2);
   bool Intersects (const Region* reg, const Line* line);
   bool P_Intersects(const Region* reg1, const Region* reg2);
   bool P_Intersects(const Region* reg, const Line* line);
   bool P_Intersects(const Line* line1, const Line* line2);
};

/*
4.3.1 Operator intersection-new

If the segment classifikation contains the number 2, the segment is added to result.

*/
/*
intersection-operator for two region-objects

*/
void MakeOp::Intersection(const Region* reg1, const Region* reg2,Region* result)
{
  // first Realmisation of both regions
   Region* res1  = new Region(0);
   Region* res2 = new Region(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( reg1, reg2, res1 , res2 );
                         break;
      case QUADRATIC:
      case OVERLAPPING:
                       mr.REALM2( reg1, reg2, false, false, res1 , res2 );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   SLine sweepline;
   int i = 0; int j = 0; int counter = 0;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1 = 0;
   BinTreeNode<SEntry>* oldnode2 = 0;
   HalfSegment hsAkt;
   HalfSegment hs1;
   HalfSegment hs2;
   result->Clear();
   result->StartBulkLoad();
   State status;
   Coord aktSweep = 0;
   Coord oldSweep = 0;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size() ) {
     // select the first segment
      res1->Get(i,hs1);
      res2->Get(j,hs2);
      if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
          hs1.GetRightPoint()==hs2.GetRightPoint()){
         i ++;   j ++;   hsAkt = hs1;    status = BOTH;
      }
      else if ( hs1 < hs2) {i ++; hsAkt = hs1; status = FIRST;}
      else {j ++; hsAkt = hs2; status = SECOND;}
      aktSweep = hsAkt.GetDomPoint().GetX();
      if (hsAkt.IsLeftDomPoint() == false) {  // right end of segment
         hsAkt.SetLeftDomPoint(true);
         SEntry ent (hsAkt);
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
//  cout << " Ausgabe nr. " << counter << " hs " << hsAkt;
            hsAkt.attr.edgeno = counter;
            hsAkt.SetLeftDomPoint(false);     (*result) += hsAkt;
            hsAkt.SetLeftDomPoint(true);        (*result) += hsAkt;
            counter++;
         }
      }
      else { // left end of segment
         SEntry ent (hsAkt);
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
            if (hs1.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
         }
         if (status == SECOND || status == BOTH) {
            if (hs2.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
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
}

/*
~Intersection~ operation.

*/


 void MakeOp::Intersection(const Region* reg, const Line* line, Line* result)
 {
  // first Realmisation of both arguments
   Line* resline  = new Line(0);
   Region* resregion = new Region(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP:  mr.REALM( line, reg, resline , resregion );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2(line, reg, false, true, resline , resregion);
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   HalfSegment hsAkt;
   HalfSegment hs1, hs2;
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
      resline->Get(i,hs1);
      resregion->Get(j,hs2);
      if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
          hs1.GetRightPoint()==hs2.GetRightPoint()){
         i ++;   j ++;   hsAkt = hs2;    status = BOTH; }
      else if ( hs1 < hs2) {i ++; hsAkt = hs1; status = FIRST;}
      else {j ++; hsAkt = hs2; status = SECOND;}
      aktSweep = hsAkt.GetDomPoint().GetX();
      if (hsAkt.IsLeftDomPoint() == false) {  // right end of segment
         if (status == SECOND || status == BOTH) {
            hsAkt.SetLeftDomPoint(true);
            SEntry ent (hsAkt);
            // delete segment from sweep line
            SEntry en = sweepline.FindAndDelete(ent,aktSweep,
               oldSweep, oldEntry1, oldnode1);
         }
      }
      else { // left end of segment
         SEntry ent (hsAkt);
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
            if (hs2.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
            ent.SetU(ms);
            ent.SetO(ns);
            node->SetEntry(ent);
         }
         if (status == FIRST || status == BOTH) {
            if (pred != 0 && pred->GetEntry().GetO() > 0)
               (*result) += hsAkt;
            // segments from line are deleted at once
            if (status == FIRST) sweepline.Delete(node);
         }
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   resline->Destroy(); delete resline;
   resregion->Destroy(); delete resregion;


   unsigned int size = result->Size();
   result->Resize(size*2);
   /*
     At this point, for each segment, a single halfsegment is inserted
     into the halfsegment array of the result. The egde numbers are
     taken from the original objects and may be outside the valid range
     (0..noSegments(result).
     The following loop corrects this state by adding the missing
     halfsegments and correcting the edge numbers.
   */

    HalfSegment hs_old;
    for( unsigned int i=0; i< size; i++){ // scan all old halfsegments
         result->Get(i,hs_old);
         hs_old.SetLeftDomPoint(true);
         hs_old.attr.edgeno = i;
         result->Put(i,hs_old);
         HalfSegment hsNew(hs_old);
         hsNew.SetLeftDomPoint(false);
         *result += hsNew;
    }

   // VTA - I need to come back here , should be solved

   result->EndBulkLoad();
  // cout << " ===========================result fertig ========" << endl;
}

/*
~Intersection~ operation

*/
void MakeOp::Intersection(const Line* line1, const Line* line2, Line* result)
{
  // first Realmisation of both lines
   Line* res1 = new Line(0);
   Line* res2 = new Line(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( line1, line2, res1 , res2  );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( line1, line2, true, true, res1 , res2  );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   int i = 0;
   int j = 0;
   HalfSegment hs1, hs2;
   result ->Clear();
   result->StartBulkLoad();
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,hs1);
      res2->Get(j,hs2);
      // segments, which are the same in both arguments are added
      // in result
      if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
          hs1.GetRightPoint()==hs2.GetRightPoint())
         {  i ++;   j ++;   (*result) += hs1; }
      else if ( hs1 < hs2)  i ++;
      else if ( hs1 > hs2)  j ++;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   // VTA - I need to come back here -- solved ??
   unsigned int size = result->Size();
   result->Resize(size*2);
    HalfSegment hs_old;
    for( unsigned int i=0; i< size; i++){ // scan all old halfsegments
         result->Get(i,hs_old);
         hs_old.SetLeftDomPoint(true);
         hs_old.attr.edgeno = i;
         result->Put(i,hs_old);
         HalfSegment hsNew(hs_old);
         hsNew.SetLeftDomPoint(false);
         *result += hsNew;
    }
   result->EndBulkLoad();
}

/*
~Intersects~ predicate

*/
bool MakeOp::P_Intersects(const Region* reg1, const Region* reg2)
{
  // first Realmisation of both regions
   Region* res1  = new Region(0);
   Region* res2 = new Region(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( reg1, reg2, res1 , res2  );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( reg1, reg2, false, false, res1 , res2  );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   HalfSegment hsAkt;
   HalfSegment hs1, hs2;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,hs1);
      res2->Get(j,hs2);
      if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
          hs1.GetRightPoint()==hs2.GetRightPoint()){
         i ++;   j ++;   hsAkt = hs1;    status = BOTH;      }
      else if ( hs1 < hs2) {i ++; hsAkt = hs1; status = FIRST;}
      else {j ++; hsAkt = hs2; status = SECOND;}
      aktSweep = hsAkt.GetDomPoint().GetX();
      if (hsAkt.IsLeftDomPoint() == false) {  // right end of segment
         hsAkt.SetLeftDomPoint(true);
         SEntry ent (hsAkt);
         // delete segment from sweep line
         if (status == FIRST)
            SEntry en = sweepline.FindAndDelete(ent,aktSweep,oldSweep,
               oldEntry1, oldnode1);
          else SEntry en = sweepline.FindAndDelete(ent,aktSweep,
             oldSweep, oldEntry2, oldnode2);
      }
      else { // left end of segment
         SEntry ent (hsAkt);
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
            if (hs1.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
         }
         if (status == SECOND || status == BOTH) {
            if (hs2.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
         }
         // if one intersection is found, break and return true
         if (ms == 2 || ns == 2) {
            res1->Destroy(); delete res1;
            res2->Destroy(); delete res2;
            return true;
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
   return false;
}

/*
~Intersects~ predicate

*/
bool MakeOp::P_Intersects(const Region* reg, const Line* line)
{
  // first Realmisation of both arguments
   Line* resline  = new Line(0);
   Region* resregion = new Region(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( line, reg, resline , resregion  );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2(line, reg, false, true, resline , resregion);
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   HalfSegment hsAkt;
   HalfSegment hs1, hs2;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   // execute until one argument is empty
   while ( i < resline->Size() && j < resregion->Size() ) {
     // select_ first
      resline->Get(i,hs1);
      resregion->Get(j,hs2);
      if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
          hs1.GetRightPoint()==hs2.GetRightPoint()){
         i ++; j ++; hsAkt = hs2; status = BOTH;
      }
      else if ( hs1 < hs2) {i ++; hsAkt = hs1; status = FIRST;}
      else {j ++; hsAkt = hs2; status = SECOND;}
      aktSweep = hsAkt.GetDomPoint().GetX();
      if (hsAkt.IsLeftDomPoint() == false) {  // right end of segment
         if (status == SECOND || status == BOTH) {
            hsAkt.SetLeftDomPoint(true);
            SEntry ent (hsAkt);
            // delete segment from sweep line
            SEntry en = sweepline.FindAndDelete(ent,aktSweep,
               oldSweep, oldEntry1, oldnode1);
         }
      }
      else { // left end of segment
         SEntry ent (hsAkt);
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
            if (hs2.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
            ent.SetU(ms);
            ent.SetO(ns);
            node->SetEntry(ent);
         }
         if (status == FIRST || status == BOTH) {
            if ( pred != 0  && pred->GetEntry().GetO() > 0 )
               {
                  resline->Destroy(); delete resline;
                  resregion->Destroy(); delete resregion;
                  return true;
               }
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


/*
~Intersects~ predicate

*/
bool MakeOp::P_Intersects(const Line* line1, const Line* line2)
{
     // first Realmisation of both lines
   Line* res1 = new Line(0);
   Line* res2 = new Line(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( line1, line2, res1 , res2  );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( line1, line2, true, true, res1 , res2 );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   int i = 0;
   int j = 0;
   HalfSegment hs1, hs2;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,hs1);
      res2->Get(j,hs2);
      // if two segments are the same -> return true
      if (hs1.GetLeftPoint()== hs2.GetLeftPoint() &&
          hs1.GetRightPoint()== hs2.GetRightPoint() )
         { return true; }
      else if ( hs1 < hs2)  i ++;
      else if ( hs1 > hs2)   j ++;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   return false;
}

/*
~Intersects~ predicate

*/
bool MakeOp::Intersects(const Region* reg1, const Region* reg2)
{
  // first Realmisation of both regions
   Region* res1  = new Region(0);
   Region* res2 = new Region(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( reg1, reg2, res1 , res2 );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( reg1, reg2, false, false, res1 , res2 );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   HalfSegment hsAkt;
   HalfSegment hs1, hs2;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,hs1);
      res2->Get(j,hs2);
      if (hs1.GetLeftPoint()==hs2.GetLeftPoint()||
          hs1.GetLeftPoint()==hs2.GetRightPoint()
       || hs1.GetRightPoint()==hs2.GetLeftPoint()||
          hs1.GetRightPoint()==hs2.GetRightPoint() ){
         res1->Destroy(); delete res1;
         res2->Destroy(); delete res2;
         return true;
      }
      else if ( hs1 < hs2) {i ++; hsAkt = hs1; status = FIRST;}
      else {j ++; hsAkt = hs2;  status = SECOND;}
      aktSweep = hsAkt.GetDomPoint().GetX();
      if (hsAkt.IsLeftDomPoint() == false) {  // right end of segment
         hsAkt.SetLeftDomPoint(true);
         SEntry ent (hsAkt);
         // delete segment from sweep line
         if (status == FIRST)
            SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
               oldEntry1, oldnode1);
         else SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
               oldEntry2, oldnode2);
      }
      else { // left end of segment
         SEntry ent (hsAkt);
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
         if (hsAkt.attr.insideAbove == true)   ns = ns+1;
         else                                   ns = ns-1;
         // set segmentclasses in SEntry
         if (ms == 2 || ns == 2) {
           res1->Destroy(); delete res1;
           res2->Destroy(); delete res2;
           return true;
         }
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
/*
~Intersects~ predicate

*/

bool MakeOp::Intersects (const Region* reg, const Line* line) {
  // first Realmisation of both arguments
   Region* res1  = new Region(0);
   Line* res2 = new Line(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( line, reg, res2 , res1 );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( line, reg, false, true, res2 , res1 );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   HalfSegment hsAkt;
   HalfSegment hs1, hs2;
   State status;
   Coord aktSweep;
   Coord oldSweep;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size() ) {
     // select_ first
      res1->Get(i,hs1);
      res2->Get(j,hs2);
      if (hs1.GetLeftPoint()==hs2.GetLeftPoint()||
          hs1.GetLeftPoint()==hs2.GetRightPoint()||
          hs1.GetRightPoint()==hs2.GetLeftPoint()||
          hs1.GetRightPoint()==hs2.GetRightPoint() ){
          res1->Destroy(); delete res1;
          res2->Destroy(); delete res2;
          return true;
      }
      else if ( hs1 < hs2) {i ++; hsAkt = hs1; status = FIRST;}
      else {j ++; hsAkt = hs2; status = SECOND;}
      aktSweep = hsAkt.GetDomPoint().GetX();
      if ( hsAkt.IsLeftDomPoint() == false) {
         if (status == FIRST) {  // right end of segment of region
            hsAkt.SetLeftDomPoint(true);
            SEntry ent (hsAkt);
            // delete segment from sweep line
            SEntry en = sweepline.FindAndDelete(ent,aktSweep,
               oldSweep, oldEntry1, oldnode1);
         }
      }
      else { // left end of segment
         SEntry ent (hsAkt);
         // add segment into sweepline-Status-Structure
         BinTreeNode<SEntry>* node = sweepline.Insert(ent,
            aktSweep, oldEntry1, oldnode1);
         BinTreeNode<SEntry>* pred = node->GetPred() ;
         if (status == FIRST ) {
            // insert HalfSegment from region into sweepline
            //int np = 0;
            //int ms, ns;
            if (pred != 0) pred->GetEntry().GetO();
            // calculate new segmentclass of new ChalfSegment
            //ms = np;
            //ns = np;
            if (hs1.attr.insideAbove == true)
               { ent.SetU(0); ent.SetO(1); }
            else
               { ent.SetU(1); ent.SetO(0); }
            node->SetEntry(ent);
         } //  end else
         else if (status == SECOND) {
            if (pred != 0)
               { if (pred->GetEntry().GetO() == 1) {
                   res1->Destroy(); delete res1;
                   res2->Destroy(); delete res2;
                   return true;
                 }
            }
            else sweepline.Delete (node);
         }
      }
      oldSweep = aktSweep ;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   return false;
}

/*
~Intersects~ predicate

*/
bool MakeOp::Intersects (const Line* line1, const Line* line2)
{
  // first Realmisation of both lines
   Line* res1 = new Line(0);
   Line* res2 = new Line(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( line1, line2, res1 , res2 );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( line1, line2, true, true, res1 , res2 );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   int i = 0;
   int j = 0;
   HalfSegment hs1, hs2;
   // execute until one argument is empty
   while ( i < res1->Size() && j < res2->Size()) {
     // select_ first
      res1->Get(i,hs1);
      res2->Get(j,hs2);
      if (hs1.GetLeftPoint()==hs2.GetLeftPoint() ||
          hs1.GetLeftPoint()==hs2.GetRightPoint() ||
          hs1.GetRightPoint()==hs2.GetLeftPoint() ||
          hs1.GetRightPoint() == hs2.GetRightPoint() ) {
           res1->Destroy(); delete res1;
           res2->Destroy(); delete res2;
           return true;
      }
      if ( hs1 < hs2)       i ++;
      else if ( hs1 > hs2)  j ++;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   return false;
}

/*
~Union~ operator

*/

void MakeOp::Union(const Region* reg1, const Region* reg2,
                      Region* result)
{
   SLine sweepline;
   int i = 0;
   int j = 0;
   int counter = 0;
   HalfSegment hsAkt;
   HalfSegment hs1, hs2;
   State status;
   Coord aktSweep, oldSweep ;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   Region* res1, *res2;
   MakeRealm mr;

   // first Realmisation of both regions
   res1 = new Region(0);
   res2 = new Region(0);

   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( reg1, reg2, res1 , res2 );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( reg1, reg2, false, false, res1 , res2 );
                         break;
      default: cout << "should not happen" << endl;
   }
   //cout << "realm ok" << *res1 << endl;
   //cout << "realm ok" << *res2 << endl;

   // initialisations
   result->Clear();
   result->StartBulkLoad();
   // while there are segments in the realm-based arguments
   //cout << "hier noch union ok" << endl;
   //cout << *res1 << endl;
   while ( i < res1->Size() || j < res2->Size()) {
      // select_first
      //cout << "while start" << endl;
      if (i < res1->Size() && j < res2->Size() ) {
         //cout << "if1" << endl;
         res1->Get(i,hs1);
         res2->Get(j,hs2);
         if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
             hs1.GetRightPoint()==hs2.GetRightPoint() ){
            i ++;   j ++;   hsAkt = hs1;    status = BOTH; }
         else if ( hs1 < hs2) {i ++; hsAkt = hs1; status = FIRST;}
         else {j ++; hsAkt = hs2; status = SECOND;}
      }
      else if (i < res1->Size() ) {
         //cout << "if2" << endl;
         res1->Get(i,hs1);  i ++;   hsAkt = hs1;  status = FIRST;
      }
      else {
         //cout << "if3" << endl;
         res2->Get(j,hs2);  j ++;   hsAkt = hs2;  status = SECOND;
      }
      aktSweep = hsAkt.GetDomPoint().GetX();
      if (hsAkt.IsLeftDomPoint() == false) { // right sement choosen
         //cout << "if4" << endl;
         hsAkt.SetLeftDomPoint(true);
         SEntry ent (hsAkt);
         SEntry en;
         // delete segment from sweep line
         if (status == FIRST) {
            //cout << "if5" << endl;
            SEntry in = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
            oldEntry1, oldnode1);
            en.Set(in);
         }
         else {
            //cout << "if6" << endl;
            SEntry in = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
            oldEntry2, oldnode2);
            en.Set(in);
         }
         if (en.GetO() == 0 || en.GetU() == 0) {  // HalfSegment in result
               //cout << "if7" << endl;
               hsAkt.attr.edgeno = counter;
               hsAkt.SetLeftDomPoint(false);    (*result) += hsAkt;
               hsAkt.SetLeftDomPoint(true);     (*result) += hsAkt;
               counter++;
         }
         if (status == BOTH) {oldEntry1=oldEntry2; oldnode1=oldnode2;}
         //cout << "if8" << endl;}
         //cout << "right";
         //sweepline.SLineOutput(aktSweep);
      }
      else { // left end of segment
         //cout << "if9" << endl;
         SEntry ent (hsAkt);
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
            //cout << "if10" << endl;
            if (hs1.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
         }
         if (status == SECOND || status == BOTH) {
        //cout << "if11" << endl;
            if (hs2.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
         }
         // set new segmentclasses in SEntry
         ent.SetU(ms);
         ent.SetO(ns);
         node->SetEntry(ent);
         //cout << "left";
         //sweepline.SLineOutput(aktSweep);
      } //  end else
      oldSweep = aktSweep ;
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   result->EndBulkLoad(true,true,true,true);
}

/*
~Union~ operator

*/

void MakeOp::Union(const Line* line1, const Line* line2,Line* result)
{
  // first Realmisation of both lines
   Line* res1 = new Line(0);
   Line* res2 = new Line(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( line1, line2, res1 , res2 );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( line1, line2, true, true, res1 , res2 );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   int i = 0;
   int j = 0;
   HalfSegment hs1, hs2;
   result ->Clear();
   result->StartBulkLoad();
   // while there are segments in the realm-based arguments
   while ( i < res1->Size() || j < res2->Size()) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,hs1);
         res2->Get(j,hs2);
         // add same segments only once
         if (hs1.GetLeftPoint()== hs2.GetLeftPoint() &&
             hs1.GetRightPoint()== hs2.GetRightPoint() )
            {  i ++;   j ++;   (*result) += hs1; }
         else if ( hs1 < hs2) { i ++;  (*result) += hs1; }
         else if ( hs1 > hs2)  { j ++;  (*result) += hs2; }
      }
      else if (i<res1->Size() )
         { res1->Get(i,hs1);  i ++;  (*result) += hs1; }
      else if ( j < res2->Size() )
         { res2->Get(j,hs2);  j ++;  (*result) += hs2; }
   } // end while
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   // VTA - I need to come back here
   unsigned int size = result->Size();
   if(size>0) {result->Resize(size*2);}
    HalfSegment hs_old;
    for( unsigned int i=0; i< size; i++){ // scan all old halfsegments
         result->Get(i,hs_old);
         hs_old.SetLeftDomPoint(true);
         hs_old.attr.edgeno = i;
         result->Put(i,hs_old);
         HalfSegment hsNew(hs_old);
         hsNew.SetLeftDomPoint(false);
         *result += hsNew;
    }

   result->EndBulkLoad();
}


/*
~Minus~ operator

*/
void MakeOp::Minus(const Region* reg1, const Region* reg2, Region* result)
{
  // first Realmisation of both regions
   Region* res1 = new Region(0);
   Region* res2 = new Region(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( reg1, reg2, res1 , res2 );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( reg1, reg2, false, false, res1 , res2 );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   SLine sweepline;
   int i = 0;   int j = 0;  int counter = 0;
   HalfSegment hsAkt;
   HalfSegment hs1, hs2;
   result ->Clear();
   result->StartBulkLoad();
   State status = State::BOTH;
   Coord aktSweep, oldSweep ;
   SEntry oldEntry1, oldEntry2;
   BinTreeNode<SEntry>* oldnode1;
   BinTreeNode<SEntry>* oldnode2;
   // while there are segments in the first arguments
   while ( i < res1->Size() ) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,hs1);
         res2->Get(j,hs2);
         if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
             hs1.GetRightPoint()==hs2.GetRightPoint() )
            {i ++; j ++; hsAkt = hs1;  status = BOTH;    }
         else if ( hs1 < hs2) { i ++; hsAkt = hs1; status = FIRST; }
         else { j ++; hsAkt = hs2; status = SECOND; }
      }
      else if (i < res1->Size() ) {
         res1->Get(i,hs1);  i ++;   hsAkt = hs1;  status = FIRST;
      }
      else if ( j < res2->Size() ) {
         res2->Get(j,hs2);  j ++;   hsAkt = hs2;  status = SECOND; }
      aktSweep = hsAkt.GetDomPoint().GetX();
      if (hsAkt.IsLeftDomPoint() == false) {  // right end of segment
         hsAkt.SetLeftDomPoint(true);
         SEntry ent (hsAkt);
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
            hsAkt.attr.edgeno = counter;
            hsAkt.SetLeftDomPoint(false);       (*result) += hsAkt;
            hsAkt.SetLeftDomPoint(true);        (*result) += hsAkt;
            counter++;
         }
         else if ( (status == SECOND && en.GetU()==1 && en.GetO()==2)||
         (status == SECOND && en.GetU() == 2 && en.GetO() == 1) ) {
            // attr insideABove must be changed for segments from 2nd
            hsAkt.attr.insideAbove = ! hsAkt.attr.insideAbove ;
            hsAkt.attr.edgeno = counter;
            hsAkt.SetLeftDomPoint(false);       (*result) += hsAkt;
            hsAkt.SetLeftDomPoint(true);        (*result) += hsAkt;
            counter++;
         }
      }
      else { // left end of segment
         SEntry ent (hsAkt);
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
         ms = np;        ns = np;
         if (status == FIRST || status == BOTH) {
            if (hs1.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
         }
         if (status == SECOND || status == BOTH) {
            if (hs2.attr.insideAbove == true) ns = ns+1;
            else                                ns = ns-1;
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
}

/*
~Minus~ operator

*/
void MakeOp::Minus(const Line* line, const Region* reg,Line* result)
{   // first Realmisation of both arguments
   Line* resLine = new Line(0);
   Region* resReg = new Region(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( line, reg, resLine , resReg );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( line, reg, true, false, resLine , resReg );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   SLine sweepline;
   int i = 0;
   int j = 0;
   HalfSegment hsAkt;
   HalfSegment hs1, hs2;
   result ->Clear();
   result->StartBulkLoad();
   State status = State::BOTH;
   Coord aktSweep, oldSweep ;
   SEntry oldEntry1;
   BinTreeNode<SEntry>* oldnode1;
    // while there are segments in the first arguments
   int counter = 0;
   while ( i < resLine -> Size() ) {
     // select_ first
      if (i < resLine -> Size() && j < resReg -> Size() ) {
         resLine -> Get(i,hs1);
         resReg -> Get(j,hs2);
         if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
             hs1.GetRightPoint()==hs2.GetRightPoint()){
            i ++;   j ++;   hsAkt = hs2;    status = BOTH;  }
         else if ( hs1 < hs2) { i ++; hsAkt = hs1; status = FIRST;}
         else { j ++; hsAkt = hs2; status = SECOND;}
      }
      else if (i < resLine ->Size() ) {
         resLine -> Get(i,hs1); i ++; hsAkt = hs1; status = FIRST;}
      aktSweep = hsAkt.GetDomPoint().GetX();
      if (hsAkt.IsLeftDomPoint() == false) {  // delete right end
         if ( status == SECOND || status == BOTH ) {
            hsAkt.SetLeftDomPoint(true);
            SEntry ent (hsAkt);
            // delete segment from sweep line
            SEntry en = sweepline.FindAndDelete(ent,aktSweep, oldSweep,
               oldEntry1, oldnode1);
         }
      }
      else { // hsAkt.IsLeftDomPoint() == true
         SEntry ent (hsAkt);
         BinTreeNode<SEntry>* en = sweepline.Insert(ent,aktSweep,
            oldEntry1, oldnode1);
         BinTreeNode<SEntry>* pred = en->GetPred() ;
         if (status == FIRST ) {
            if (pred == 0 || (pred!=0 && pred->GetEntry().GetO()==0) )
            {
         HalfSegment auxHs1( hs1 );
               auxHs1.attr.edgeno = counter;
               (*result) += auxHs1;
               auxHs1.SetLeftDomPoint(false);
               (*result) += auxHs1;
               counter++;
            }
            sweepline.Delete (en) ;
         }
         else {  // status == SECOND or BOTH
            if (hsAkt.attr.insideAbove == true)
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
   // VTA - I need to come back here
   result->EndBulkLoad();
}

/*
~Minus~ operator

*/

void MakeOp::Minus(const Line* line1, const Line* line2,Line* result)
{
  // first Realmisation of both lines
   Line* res1 = new Line(0);
   Line* res2 = new Line(0);
   MakeRealm mr;
   switch ( kindofrealm )
   {
      case PLANESWEEP: mr.REALM( line1, line2, res1 , res2 );
                         break;
      case QUADRATIC:
      case OVERLAPPING: mr.REALM2( line1, line2, true, true, res1 , res2 );
                         break;
      default: cout << "should not happen" << endl;
   }
   // initialisations
   int i = 0;
   int j = 0;
   HalfSegment hs1, hs2;
   result ->Clear();
   result->StartBulkLoad();
    // while there are segments in the first arguments
   while ( i < res1->Size() ) {
     // select_ first
      if (i < res1->Size() && j < res2->Size() ) {
         res1->Get(i,hs1);
         res2->Get(j,hs2);
         if (hs1.GetLeftPoint()==hs2.GetLeftPoint() &&
             hs1.GetRightPoint()==hs2.GetRightPoint() )
            { i ++;  j ++; }
         else if ( hs1 < hs2) { i ++;  (*result) += hs1;  }
         else if ( hs1 > hs2)  j ++;
      }
      else if (i < res1->Size() ) {
         res1->Get(i,hs1);
         i ++;
         (*result) += hs1;
      }
   }
   res1->Destroy(); delete res1;
   res2->Destroy(); delete res2;
   // VTA - I need to come back here
   unsigned int size = result->Size();
   result->Resize(size*2);
    HalfSegment hs_old;
    for( unsigned int i=0; i< size; i++){ // scan all old halfsegments
         result->Get(i,hs_old);
         hs_old.SetLeftDomPoint(true);
         hs_old.attr.edgeno = i;
         result->Put(i,hs_old);
         HalfSegment hsNew(hs_old);
         hsNew.SetLeftDomPoint(false);
         *result += hsNew;
    }
   result->EndBulkLoad();
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
    if ( s == Point::BasicType()  ) return (stpoint);
    if ( s == Points::BasicType() ) return (stpoints);
    if ( s == Line::BasicType()   ) return (stline);
    if ( s == Region::BasicType() ) return (stregion);
    if ( s == Rectangle<2>::BasicType()   ) return (stbox);
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
         return (nl->SymbolAtom( Line::BasicType() ));
      else if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( Line::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
         NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( Region::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
         NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( Region::BasicType() ));
   }
   return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

const string RealmSpec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
  "( <text> (region x region -> region) or (line x line -> line)"
  " or (region x line -> region) or (line x region -> line)</text--->"
  "<text>realm(_,_)</text--->"
  "<text>Returns the first object as REALMed object.</text--->"
  "<text>query realm ( line1, region2 )</text--->"
    ") )";

/*
realmisation of a line and a region-object

*/
static int realm_lr( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   Line *l1 = ((Line*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined())
   {
      Line* test1 = static_cast<Line*>(result.addr);
      Region* test2 = new Region(0);
      MakeRealm mr;
      switch ( kindofrealm )
      {
        case PLANESWEEP: mr.REALM( l1, r2, test1 , test2 );
                         break;
        case QUADRATIC:
        case OVERLAPPING: mr.REALM2( l1, r2, true, false, test1 , test2 );
                         break;
        default: cout << "should not happen" << endl;
      }

      //mr.REALM2( l1, r2, test1 , test2 );
      delete test2;
      return(0);
   }
   else  {
       ((Line *)result.addr)->Clear();
       ((Line *)result.addr)->SetDefined( false );
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
   Line *l1 = ((Line*)args[1].addr);
   Region *r2 = ((Region*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined())
   {
      Line* test1 = new Line(0);
      Region* test2 = static_cast<Region*>(result.addr);
      MakeRealm mr;
      switch ( kindofrealm )
      {
        case PLANESWEEP: mr.REALM( l1, r2, test1 , test2 );
                         break;
        case QUADRATIC :
        case OVERLAPPING: mr.REALM2( l1, r2, false, true, test1 , test2 );
                         break;
        default: cout << "should not happen" << endl;
      }
      delete test1;
      return(0);
   }
   else  {
       ((Region *)result.addr)->Clear();
       ((Region *)result.addr)->SetDefined( false );
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
   Line *l1 = ((Line*)args[0].addr);
   Line *l2 = ((Line*)args[1].addr);
   if (l1->IsDefined() && l2->IsDefined())
   {
      Line* test1 = static_cast<Line*>(result.addr);
      Line* test2 = new Line(0);
      MakeRealm mr;
      switch ( kindofrealm )
      {
        case PLANESWEEP: mr.REALM( l1, l2, test1 , test2 );
                         break;
        case QUADRATIC:
        case OVERLAPPING: mr.REALM2( l1, l2, true, true, test1 , test2 );
                         break;
        default: cout << "should not happen" << endl;
      }
      delete test2;
      return(0);
   }
   else
   {
       ((Line *)result.addr)->Clear();
       ((Line *)result.addr)->SetDefined( false );
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
   Region *r1 = ((Region*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined())
   {
      Region* test1 = static_cast<Region*>(result.addr);
      Region* test2 = new Region(0);
      MakeRealm mr;
      switch ( kindofrealm )
      {
        case PLANESWEEP: mr.REALM( r1, r2, test1 , test2 );
                         break;
        case QUADRATIC:
        case OVERLAPPING: mr.REALM2( r1, r2, false, false, test1 , test2 );
                         break;
        default: cout << "should not happen" << endl;
      }
      delete test2;
      return(0);
   }
   else
   {
       ((Region *)result.addr)->Clear(); // added by CD
       // ((Region *)result.addr)->SetDefined( false ); // commented out by CD
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
         return (nl->SymbolAtom( Line::BasicType() ));
      else if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( Line::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( Line::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( Region::BasicType() ));
    }
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/*
operator intersection for objects line -line

*/
static int Inter_ll( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   Line *line1 = ((Line*)args[0].addr);
   Line *line2 = ((Line*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() ) {
      if (line1->IsEmpty() || line2->IsEmpty() ) {
          ((Line *)result.addr)->SetDefined( false );
         return (0);
      }
      else if (line1->BoundingBox().IsDefined() &&
         line2->BoundingBox().IsDefined() ) {
         if (line1->BoundingBox().Intersects(line2->BoundingBox())) {
            Line* res = 0;
            MakeOp mo;
            mo.Intersection( line1, line2, static_cast<Line*>(result.addr));
            delete res;
            return(0);
         }
         else   {
            ((Line *)result.addr)->Clear();
            return (0);
         }
      }
      else  {
         Line* res = 0;
         MakeOp mo;
         mo.Intersection( line1, line2,static_cast<Line*>(result.addr));
         delete res;
         return(0);
      }
   }
   else  {
     ((Line *)result.addr)->Clear();
     ((Line *)result.addr)->SetDefined( false );

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
   Line *l1 = ((Line*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
      if (l1->IsEmpty() || r2->IsEmpty() ) {
         ((Line *)result.addr)->SetDefined( false );
         return (0);
      }
      if (l1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox())) {
            MakeOp mo;
            mo.Intersection( r2, l1,static_cast<Line*>(result.addr));
            return(0);
         }
         else   {
           ((Line *)result.addr)->Clear();
            return (0);
         }
      }
      else  {
         MakeOp mo;
         mo.Intersection( r2, l1,static_cast<Line*>(result.addr));
         return(0);
      }
   }
   else  {
       ((Line *)result.addr)->Clear();
       ((Line *)result.addr)->SetDefined( false );
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
   Line *l1 = ((Line*)args[1].addr);
   Region *r2 = ((Region*)args[0].addr);
   if (l1->IsDefined() && r2->IsDefined() ) {
      if (l1->IsEmpty() || r2->IsEmpty() ) {
         ((Line *)result.addr)->Clear();
         return (0);
      }
      if (l1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if ( l1->BoundingBox().Intersects( r2->BoundingBox() ) )  {
            MakeOp mo;
            mo.Intersection( r2, l1,static_cast<Line*>(result.addr));
            return(0);
         }
         else   {
            ((Line *)result.addr)->Clear();
            return (0);
         }
      }
      else  {
         MakeOp mo;
         mo.Intersection( r2, l1, static_cast<Line*>(result.addr));
         return(0);
      }
   }
   else  {
       ((Line *)result.addr)->Clear();
       ((Line *)result.addr)->SetDefined( false );
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
   Region *r1 = ((Region*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
      if (r1->IsEmpty() || r2->IsEmpty() ) {
          ((Region *)result.addr)->Clear();
          return (0);
      }
      if (r1->BoundingBox().IsDefined() &&
         r2->BoundingBox().IsDefined() ) {
         if ( r1->BoundingBox().Intersects( r2->BoundingBox())) {
            MakeOp mo;
            mo.Intersection( r1, r2,static_cast<Region*>(result.addr) );
            return(0);
         }
         else   { // no intersection possible
            ((Region *)result.addr)->Clear();
            return (0);
         }
      }
      else  {
         MakeOp mo;
         mo.Intersection( r1, r2,static_cast<Region*>(result.addr));
         return(0);
      }
   }
   else  {
       ((Region *)result.addr)->Clear();
       ((Region *)result.addr)->SetDefined( false );
       return (0);
   }
}

const string InterSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   "(<text>(line x line -> line) or "
   "(line x region -> line) or "
   "(region x line -> line) or "
   "(region x region -> region)</text--->"
   "<text>intersection_new(_,_)</text--->"
   "<text>Returns the intersection of 2 lines/regions.</text--->"
   "<text>query intersection_new ( region1, region2 )</text--->"
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
         return (nl->SymbolAtom( CcBool::BasicType() ));
      else if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
         NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( CcBool::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( CcBool::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( CcBool::BasicType() ));
    }
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/*
operator intersects for objects line -line

*/
static int Intersects_ll( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   Line *line1 = ((Line*)args[0].addr);
   Line *line2 = ((Line*)args[1].addr);
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
   Line *l1 = ((Line*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
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
   Line *l1 = ((Line*)args[1].addr);
   Region *r2 = ((Region*)args[0].addr);
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
   Region *r1 = ((Region*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
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
   "( <text>(line || region) x (line || region) -> bool</text--->"
   "<text>_ intersects_new _</text--->"
   "<text>tests if two lines/regions intersects.</text--->"
   "<text>query region1 intersects_new region2</text--->"
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
         return (nl->SymbolAtom( Line::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( Region::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( Region::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( Region::BasicType() ));
    }
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/*
operator union for objects region - region

*/
static int Union_rr( Word* args, Word& result, int message,
Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   Region *r1 = ((Region*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() )    {
      if ( r1->IsEmpty() && r2 -> IsEmpty() ) {
         ((Region *)result.addr)->Clear();
         return (0);
      }
      else if (r1->IsEmpty() ) {
         (static_cast<Region*>(result.addr))->CopyFrom(r2);
         return(0);
      }
      else if (r2->IsEmpty() ) {
         (static_cast<Region*>(result.addr))->CopyFrom(r1);
         return(0);
      }
      else {
         MakeOp mo;
         mo.Union( r1, r2,static_cast<Region*>(result.addr) );
         return(0);
      }
   }
   else  {
       ((Region *)result.addr)->Clear();
       ((Region *)result.addr)->SetDefined( false );
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
   Line *line = ((Line*)args[0].addr);
   Region *reg = ((Region*)args[1].addr);
   if ( reg->IsDefined()&&!reg->IsEmpty()&&line->IsDefined() ) {
      (static_cast<Region*>(result.addr))->CopyFrom(reg);
      return(0);
   }
   else  {
      ((Region *)result.addr)->Clear();
      ((Region *)result.addr)->SetDefined( false );
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
   Line *line = ((Line*)args[1].addr);
   Region *reg = ((Region*)args[0].addr);
   if ( ! reg -> IsEmpty() && line->IsDefined() &&
   reg->IsDefined() )    {
      (static_cast<Region*>(result.addr))->CopyFrom(reg);
      return(0);
   }
   else  {
      ((Region *)result.addr)->Clear();
      ((Region *)result.addr)->SetDefined( false );
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
   Line *line1 = ((Line*)args[0].addr);
   Line *line2 = ((Line*)args[1].addr);
   if (line1->IsDefined() && line2->IsDefined() )    {
      if ( line1->IsEmpty() ) {
         ((Line*)result.addr)->CopyFrom(line2);
         return(0);
      }
      else if (line2->IsEmpty() ) {
         ((Line*)result.addr)->CopyFrom(line1);
         return(0);
      }
      else {
         MakeOp mo;
         mo.Union( line1, line2,static_cast<Line*>(result.addr));
         return(0);
      }
   }
   else  {
       ((Line *)result.addr)->Clear();
       ((Line *)result.addr)->SetDefined( false );
       return (0);
   }
}

const string UnionSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   "(<text>(line x line -> line) or "
   "(line x region -> region) or "
   "(region x line -> region) or "
   "(region x region -> region)</text--->"
   "<text>union_new(_,_)</text--->"
   "<text>Returns the union of 2 lines/regions .</text--->"
   "<text>query union_new ( region1, region2 )</text--->"
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
         return (nl->SymbolAtom( Line::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stline &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( Line::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stline )
         return (nl->SymbolAtom( Region::BasicType() ));
      else  if ( NewSpatialTypeOfSymbol( arg1 ) == stregion &&
          NewSpatialTypeOfSymbol( arg2 ) == stregion )
         return (nl->SymbolAtom( Region::BasicType() ));
    }
    return (nl->SymbolAtom( Symbol::TYPEERROR() ));
}

/*
operator minus for objects region - region

*/
static int Minus_rr( Word* args, Word& result, int message,
   Word& local, Supplier s )
{
   result = qp->ResultStorage( s );
   Region *r1 = ((Region*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
   if (r1->IsDefined() && r2->IsDefined() ) {
      if ( r1 -> IsEmpty() ) {
         ((Region *)result.addr)->Clear();
         return(0);
      }
      else if ( r2->IsEmpty() ) {
         (static_cast<Region*>(result.addr))->CopyFrom(r1);
         return(0);
      }
      else if (r1->BoundingBox().IsDefined() &&
      r2->BoundingBox().IsDefined() ) {
         if ((r1->BoundingBox().Intersects(r2->BoundingBox()))) {
            MakeOp mo;
            mo.Minus( r1, r2,static_cast<Region*>(result.addr));
            return(0);
         }
         else   {
            (static_cast<Region*>(result.addr))->CopyFrom(r1);
            return (0);
         }
      }
      else    {
         MakeOp mo;
         mo.Minus( r1, r2, static_cast<Region*>(result.addr));
         return(0);
      }
   }
   else   {
       ((Region *)result.addr)->Clear();
       ((Region *)result.addr)->SetDefined( false );
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
   Line *line = ((Line*)args[0].addr);
   Region *reg = ((Region*)args[1].addr);
   if (line->IsDefined() && reg->IsDefined() )    {
      if ( line-> IsEmpty()|| reg->IsEmpty() ) {
         ((Line *)result.addr)->Clear();
         return(0);
      }
      else if (line->BoundingBox().IsDefined() &&
      reg->BoundingBox().IsDefined() ) {
         if (line->BoundingBox().Intersects(reg->BoundingBox())) {
            MakeOp mo;
            mo.Minus( line,reg,static_cast<Line*>(result.addr) );
            return(0);
         }
         else   {
            (static_cast<Line*>(result.addr))->CopyFrom(line);
            return (0);
         }
      }
      else    {
         MakeOp mo;
         mo.Minus( line, reg,static_cast<Line*>(result.addr) );
         return(0);
      }
   } // if IsDefined()
   else   {
      ((Line *)result.addr)->Clear();
      ((Line *)result.addr)->SetDefined( false );
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
   Line *line = ((Line*)args[1].addr);
   Region *reg = ((Region*)args[0].addr);
   if ( ! line->IsEmpty() && line->IsDefined() && reg->IsDefined() )    {
      (static_cast<Region*>(result.addr))->CopyFrom(reg) ;
      return(0);
   }
   else  {
      ((Region *)result.addr)->Clear(); // added by CD
      //((Region *)result.addr)->SetDefined( false );// removed by CD
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
  Line *line1 = ((Line*)args[0].addr);
  Line *line2 = ((Line*)args[1].addr);
  if (line1->IsDefined() && line2->IsDefined() )    {
    if (line1->IsEmpty() || line2->IsEmpty()) {
       (static_cast<Line*>(result.addr))->CopyFrom(line1);
       return (0);
    }
    else if (line1->BoundingBox().IsDefined() &&
             line2->BoundingBox().IsDefined() ) {
      if ((line1->BoundingBox().Intersects
           (line2->BoundingBox() ) ) ) {
        MakeOp mo;
        mo.Minus( line1, line2,static_cast<Line*>(result.addr) );
        return(0);
      }
      else {
        (static_cast<Line*>(result.addr))->CopyFrom(line1);
            return(0);
      }
    }
    else {
      MakeOp mo;
      mo.Minus( line1, line2,static_cast<Line*>(result.addr) );
      return(0);
    }
  }
  else  {
    ((Line *) result.addr)->Clear();
    ((Line *) result.addr)->SetDefined(false);
    return (0);
  }
}


const string MinusSpec  =
   "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" )"
   "( <text>(line x line -> line) or "
   "(line x region -> line) or "
   "(region x line -> line) or "
   "(region x region -> region)</text--->"
   "<text>minus_new(_,_)</text--->"
   "<text>Returns the differenz of 2 lines/regions.</text--->"
   "<text>query minus_new ( region1, region2 )</text--->"
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
   Line *line1 = ((Line*)args[0].addr);
   Line *line2 = ((Line*)args[1].addr);
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
   Line *l1 = ((Line*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
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
   Line *l1 = ((Line*)args[1].addr);
   Region *r2 = ((Region*)args[0].addr);
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
   Region *r1 = ((Region*)args[0].addr);
   Region *r2 = ((Region*)args[1].addr);
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
   "( <text>(line || region) x (line || region) -> bool </text--->"
   "<text>_ p_intersects _</text--->"
   "<text>Returns if two lines/regions intersects interior.</text--->"
   "<text>query region1 p_intersects region2</text--->"
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

} // end of namespace planesweep


extern "C"
Algebra*
InitializePlaneSweepAlgebra( NestedList* nlRef, QueryProcessor* qpRef )
{
  nl = nlRef;
  qp = qpRef;
  return (new planesweep::PlaneSweepAlgebra());
}
