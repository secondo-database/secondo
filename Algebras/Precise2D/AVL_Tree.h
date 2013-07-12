/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2009, University in Hagen, Faculty of Mathematics and
 Computer Science, Database Systems for New Applications.

 SECONDO is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by
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

 //paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]
 //paragraph [10] Footnote: [{\footnote{] [}}]
 //[TOC] [\tableofcontents]

 [TOC]

 0 Overview

 1 Includes and defines

*/

#ifndef AVL_TREE_H_
#define AVL_TREE_H_

//#include "Line2.h"
#include "Point2.h"
#include "Points2.h"
#include "Region2Algebra.h"
#include <queue>
#include <vector>

namespace p2d {

class SegmentData;
class Line2;
class Point2;
class Points2;

class AVLNode;
class AVLSegment;
class Event;

class AVLTree {

private:

 AVLNode* root;

public:

 /*
  Default-constructor

 */
 AVLTree();

 /*
  Destructor

 */
 ~AVLTree();

 /*
  ~insert~

 */
 void insert(AVLSegment* x, AVLSegment*& pred, AVLSegment*& suc);

 /*
  ~removeGetNeighbor~

  This function removes ~x~ from the tree. To find ~x~ in the tree,
  we use the order of the segments at the time in which the sweepline
  is above the right endpoint of ~x~. Additionally ~pred~ will point
  to the predecessor of ~x~, if it exists, and ~suc~ will point to
  the successor. If there are no predecessor or successor ~pred~ resp.
  ~suc~ will be NULL.

 */
 void removeGetNeighbor(AVLSegment* elem, AVLSegment*& pred, AVLSegment*& suc);

 /*
  ~removeGetNeighbor2~

  This function removes ~x~ from the tree. For finding ~x~ in the tree
  we use the order of the segments at the time in which the sweepline
  is in the position ~gridXPos~ + ~preciseXPos~. Additionally ~pred~
  will point to the predecessor of ~x~, if it exists, and ~suc~ will
  point to the successor. If there are no predecessor
  or successor ~pred~ resp. ~suc~ will be NULL.

 */
 void removeGetNeighbor2(AVLSegment* x, int gridXPos, mpq_class& preciseXPos,
   AVLSegment*& pred, AVLSegment*& suc);

 /*
  ~removeInvalidSegment~

  This function removes ~x~ from the tree. For finding ~x~ in the tree
  we use the order of the segments at the time in which the sweepline
  is in the position ~gridXPos~ + ~preciseXPos~.

 */
 void removeInvalidSegment(AVLSegment* x, int gridXPos, mpq_class& preciseXPos);

 /*
  ~invertSegments~
  This method searches the nodes which contains the AVLSegments
  stored in ~v~ and inverts the segments. Additionally ~pred~ will
  point to the predecessor of the AVLSegment stored leftmost in the
  tree, if it exists, and ~suc~ will point to the successor of the
  AVLSegment stored rightmost in the tree. If there are no
  predecessor or successor ~pred~ resp. ~suc~ will point to the same
  objects as before calling ~removeGetNeighbor~. It might be useful
  to initialize ~pred~ and ~suc~ with NULL before calling this method.

 */
 void invertSegments(vector<AVLSegment*>& v, int gridX, mpq_class& pX,
   int gridY, mpq_class& pY, AVLSegment*& pred, size_t predIndex,
   AVLSegment*& suc, size_t sucIndex);

 /*
  ~inorder~

 */
 void inorder();
};

class AVLNode {
private:
 AVLSegment* elem;
 AVLNode* left;
 AVLNode* right;
 int height;

 /*
  ~setHeight~

 */
 void setHeight();

 /*
  ~balance~

 */
 int balance();

 /*
  ~counterClockwiseRotation~ and ~clockwiseRotation~

  Both functions rebalance the avl-tree.

 */
 AVLNode* counterClockwiseRotation();

 AVLNode* clockwiseRotation();

 /*
  ~isLeaf~

 */
 bool isLeaf();

 /*
  ~deletemin~

 */
 AVLSegment* deletemin(AVLNode* node);

public:

 AVLNode(AVLSegment* elem);

 AVLNode(const AVLNode&);

 ~AVLNode();

 /*

  ~=~

 */
 AVLNode& operator=(const AVLNode& node);

 AVLSegment* getElement() {
  return elem;
 }

 void setElement(AVLSegment** seg);

 /*
  ~insert~

  Inserts ~x~ in the tree with ~this~ as root if it is not yet inside.
  For descending in the tree we need ~pos~ to estimate if ~x~
  belongs in the left or right subtree. The function returns the node
  containing ~x~. Additionally ~pred~ points to the predecessor, if a
  predecessor exists. If not, ~pred~ points to the same object as
  before. It might be useful to initialize ~pred~ with NULL
  before calling ~insert~. The same goes for ~suc~ wih the
  successor.

 */
 AVLNode* insert(AVLSegment* x, AVLSegment*& pred, AVLSegment*& suc);

 /*
  ~removeGetNeighbor~

  This function removes ~x~ from the tree. For finding ~x~ in the tree
  we use the order of the segments at the time in which the sweepline
  is above the right endpoint of ~x~. Additionally ~pred~ will point
  to the predecessor of ~x~, if it exists, and ~suc~ will point to
  the successor. If there are no predecessor or successor ~pred~
  resp. ~suc~ will point to the same objects as before calling
  ~removeGetNeighbor~. It might be useful to initialize ~pred~
  and ~suc~ with NULL before calling this method.

 */
 AVLNode* removeGetNeighbor(AVLSegment* x, AVLSegment*& pred, AVLSegment*& suc);

 /*
  ~removeGetNeighbor2~

  This function removes ~x~ from the tree. For finding ~x~ in the
  tree we use the order of the segments at the time in which the
  sweepline is in the position ~gridXPos~ + ~preciseXPos~.
  Additionally ~pred~ will point to the predecessor of ~x~, if it
  exists, and ~suc~ will point to the successor. If there are no
  predecessor or successor ~pred~ resp. ~suc~ will point to the same
  objects as before calling ~removeGetNeighbor~. It might be useful
  to initialize ~pred~ and ~suc~ with NULL before calling this method.

 */
 AVLNode* removeGetNeighbor2(AVLSegment* x, int gridXPos, mpq_class preciseXPos,
   AVLSegment*& pred, AVLSegment*& suc);

 /*
  ~removeInvalidSegment~

  This function removes the invalid ~x~ from the tree. For finding
  ~x~ in the tree we use the order of the segments at the time in
  which the sweepline is in the position ~gridXPos~ + ~preciseXPos~.
  A segment is invalid, if we insert a segment, which overlaps an
  already contained segment. In this case there are two equal segments
  in the tree, one valid and the second invalid. The invalid segment
  lay either above the valid one or left resp. right below the valid
  segment.

 */
 AVLNode* removeInvalidSegment(AVLSegment* x, int gridXPos,
   mpq_class preciseXPos, bool& found);

 int getHeight() {
  return height;
 }

 /*
  ~memberPlusNeighbor~

  ~result~ will be true if ~key~ is member of the tree with ~this~
  as root. For descending in the tree we need ~pos~ to estimate if
  ~key~ is in the left or right subtree. If ~key~ is found,
  ~memberPlusNeighbor~returns the node containing ~key~. If ~pred~
  is true, neighbor will hold the predecessor of ~key~ if a
  predecessor exists. If ~pred~ is false, neighbor will hold
  the successor of ~key~.
  If ~key~ is not found, ~memberPlusNeighbor~ returns NULL.
  Take a look, that if ~key~ is not found or there is no predecessor
  resp. successor, then ~neighbor~ don't change. It might be useful,
  to initilize ~neighbor~ with NULL, before ~memberPlusNeighbor~
  is called.

 */
 AVLNode* memberPlusNeighbor(AVLSegment* key, bool& result, int gridXPos,
   mpq_class& preciseXPos, int gridYPos, mpq_class& preciseYPos,
   AVLSegment*& neighbor, bool pred);

 /*
  ~member~

  ~result~ will be true if ~key~ is member of the tree with ~this~
  as root. For descending in the tree we need ~pos~ to estimate if
  ~key~ is in the left or right subtree. If ~key~ is found ~member~
  returns the node containing ~key~ and NULL if not.

 */
 AVLNode* member(AVLSegment* key, bool& result, int gridXPos,
   mpq_class& preciseXPos, int gridYPos, mpq_class& preciseYPos);

 /*
  ~inorder~

 */
 void inorder();
};

/*
 The both following enumeration-types are used in ~mightIntersect~

*/
enum Slope {
 vertical, horizontal, positivSlope, negativSlope
};
enum Position {
 top, bottom, pLeft, pRight
};

/*
 ~KindOfEvent~ is used in ~Event~.

*/
enum KindOfEvent {
 leftEndpoint, rightEndpoint, intersectionPoint
};

/*
 ~Owner~ is used in AVLSegment to mark the owner of the segments i for
 the set-operations

*/
enum Owner {
 first, second, both, none
};

/*
 ~SetOperation~ is used to distinguish between the set operations.

*/
enum SetOperation {
 union_op, intersection_op, difference_op
};

enum RelationshipOperation {
 intersects_op, inside_op, overlaps_op
};

/*
 ~Coordinate~ is used in the ~selectNext~-functions.

*/
struct Coordinate {
 int gx;
 int gy;
 mpq_class px;
 mpq_class py;

 bool operator>(const Coordinate& c) const {
  if (gx != c.gx) {
   if (gx > c.gx) {
    return true;
   } else {
    return false;
   }
  }
  int cmpX = cmp(px, c.px);
  if (cmpX != 0) {
   return cmpX > 0;
  }
  if (gy != c.gy) {
   if (gy > c.gy) {
    return true;
   } else {
    return false;
   }
  }
  return cmp(py, c.py) > 0;
 }
};

class AVLSegment {

protected:
 int gridXL, gridYL, gridXR, gridYR;

 const Flob* flob;

 const DbArray<unsigned int>* dbarray;

 SegmentData* originalData1;

 Reg2PrecHalfSegment* originalData2;

 mpq_class pxl;

 mpq_class pyl;

 mpq_class pxr;

 mpq_class pyr;

 Owner owner;

 bool valid;

 bool isNew;

 int noOfChanges;

 bool firstInsideAbove;

 bool secondInsideAbove;

 short int conAbove;

 short int conBelow;

 AVLSegment(int gridX, mpq_class pX, int gridY, mpq_class pY);

 /*
  ~prepareData~

  Extract the integer from ~value~.

 */
 void prepareData(int & resultGridX, mpq_class& resultPX, mpq_class& value);

 /*

  ~intervalIsVertical~

 */
 bool intervalIsVertical();

 /*
  ~computeBeginOfIntersectionInterval~

  If we compute only with the grid-values there is an interval where this
  segment will intersect the sweepline. This function computes the begin of
  this interval.

 */
int computeBeginOfIntersectionInterval(int pos);

 /*
  ~computeEndOfIntersectionInterval~

  If we compute only with the grid-values there is an interval where this
  segment will intersect the sweepline. This function computes the end of this
  interval.

 */
int computeEndOfIntersectionInterval(int pos);


 /*
  ~isVertical~

 */
 bool isVertical() const;

 /*
  ~set~
  Change ~this~ to an avlsegment with the given values.

 */
 void set(mpq_class xl, mpq_class yl, mpq_class xr, mpq_class yr, Owner o);

 void set(int gxl, int gyl, int gxr, int gyr, mpq_class xl, mpq_class yl,
   mpq_class xr, mpq_class yr, Owner o);

public:
 AVLSegment();

 AVLSegment(const Flob* preciseData, SegmentData* sd, Owner o);

 AVLSegment(const DbArray<unsigned int>* preciseData, Reg2GridHalfSegment& sd,
   Reg2PrecHalfSegment* ps, Owner o);

 AVLSegment(const AVLSegment& s);

 AVLSegment(int gxl, int gyl, int gxr, int gyr, mpq_class xLeft,
   mpq_class yLeft, mpq_class xRight, mpq_class yRight, Owner o);

 ~AVLSegment();

 /*
  ~getGrid\_~

  returns the grid values.

 */
 int getGridXL() const {
  return gridXL;
 }
 int getGridYL() const {
  return gridYL;
 }
 int getGridXR() const {
  return gridXR;
 }
 int getGridYR() const {
  return gridYR;
 }

 /*
  ~getPrecise\_~

  returns the precise values.

 */
 mpq_class getPreciseXL() const;

 mpq_class getPreciseYL() const;

 mpq_class getPreciseXR() const;

 mpq_class getPreciseYR() const;


 /*
  ~getNumberOfChanges~

  If a segment has been changed, ~noOfChanges~ has been modified (incremented).
  It will be used in the plane-sweep-algorithms for detecting invalid events
  saved in the priority\_queue.

 */
 int getNumberOfChanges() const;

 /*
  ~setNumberOfChanges~

  Sets ~noOfChanges~ to the given argument.

 */
 void setNumberOfChanges(int i);

 /*
  ~incrementNumberOfChanges~

 */
 void incrementNumberOfChanges();

 /*
  ~getFirstInsideAbove~

 */
 bool getFirstInsideAbove() const;

 /*
  ~getSecondInsideAbove~

 */
 bool getSecondInsideAbove() const;

 /*
  ~setFirstInsideAbove~

 */
 void setFirstInsideAbove(bool insideAbove);

 /*
  ~setSecondInsideAbove~

 */
 void setSecondInsideAbove(bool insideAbove);

 /*
  ~getConBelow~

 */
 short int getConBelow();

 /*
  ~getConAbove~

 */
 short int getConAbove();

 /*
  ~setConBelow~

 */
 void setConBelow(short int i);

 /*
  ~setConAbove~

 */
 void setConAbove(short int i);

 /*
  ~inkrConAbove~

 */
 void inkrConAbove() {
  conAbove++;
 }

 /*
  ~inkrConBelow~

 */
 void inkrConBelow() {
  conBelow++;
 }

 /*
  ~dekrConAbove~

 */
 void dekrConAbove() {
  conAbove--;
 }

 /*
  ~dekrConBelow~

 */
 void dekrConBelow() {
  conBelow--;
 }

 /*
  ~print~

 */
 void print();

 /*
  ~equal~

 */
 bool equal(AVLSegment& s) const;

 /*
  ~isPoint~

 */
 bool isPoint() const;

 bool isEndpoint(int gx, int gy, mpq_class px, mpq_class py);

 bool startsAtPoint(int gx, int gy, mpq_class px, mpq_class py);

 bool endsInPoint(int gx, int gy, mpq_class px, mpq_class py);

 bool isLeftOf(Event& event);

 bool leftPointIsLeftOf(Event& event);

 bool isLeftPointOf(AVLSegment& s);

 bool isRightPointOf(AVLSegment& s);

 bool hasEqualLeftEndpointAs(AVLSegment& s);

 /*
  ~isValid~

  Returns true, if the segment is valid, false otherwise. A segment will be
  invalid, if there is another segment in the avl-tree, which overlaps ~this~
  completely. This function is used for the plain-sweep-algorithms, to create
  a new event, which will delete this from the avl-tree as soon as possible if
  a segment is marked as invalid.

 */
 bool isValid() const;

 /*
  ~changeValidity~
  Sets ~valid~ to the given value.

 */
 void changeValidity(bool v);

 /*
  ~getOwner~

 */
 Owner getOwner() const {
  return owner;
 }

 /*
  ~setOwner~

 */
 void setOwner(Owner o) {
  owner = o;
 }

 /*
  ~=~

 */
 AVLSegment& operator=(const AVLSegment& s);

 /*
  ~isParallelTo~

 */
 bool isParallelTo(const AVLSegment& s) const;

 /*
  ~compareIntersectionintervalWithSweepline~

  computes the interval, where ~this~ and ~s~ run through ~gridXPos~ and
  ~gridXPos~+1. Returns
  -1 if ~this~ runs below ~s~
  0 if both intervals intersect and we can't decide, whether ~this~ run
  below ~s~ or vice versa
  1 if ~this~ runs above ~s~

 */
 int compareIntersectionintervalWithSweepline(AVLSegment& s, int gridXPos);

 /*
  ~compareInPosOrRight~
  Compares the y-values in a given x-value ~pos~ in both segments. The result is
  - -1  if the value of this is less than the one in ~seg~
  -  0  if both values are equal
  - +1  if the value of this is greater than the one in ~seg~

  If both segments intersect in the given x-position, we take the given order
  more right.

  Precondition for this function:
  this.xl <= gridPos this.xr and seg.xl <= gridPos <= seg.xr
  If the precondition is not satisfied, an assertion ends with false.

 */
 int compareInPosOrRight(AVLSegment& s, int gridXPos, mpq_class& preciseXPos);

 /*
  ~compareInPosOrLeft~
  Compares the y-values in a given x-value ~pos~ in both segments. The result is
  - -1  if the value of this is less than the one in ~seg~
  -  0  if both values are equal
  - +1  if the value of this is greater than the one in ~seg~

  If both segments intersect in the given x-position, we take the given order
  more left.

  Precondition for this function:
  this.xl <= gridPos this.xr and seg.xl <= gridPos <= seg.xr
  If the precondition is not satisfied, an assertion ends with false.

 */
 int compareInPosOrLeft(AVLSegment& s, int gridXPos, mpq_class& preciseXPos);

 /*
  ~compareInPosForMember~
  Compares the y-values in a given x-value ~pos~ in both segments.
  The result is
  - -1 if the value of this is less than the one in ~seg~
  (then ~this~ is below ~seg~)
  -  0 if both values are equal
  - +1 if the value of this is greater than the one in ~seg~
  (then ~this~ is above ~seg~)

  If both segments intersect in the given x-position, we take the
  given order more left.

  Precondition for this function:
  this.xl <= gridPos this.xr and seg.xl <= gridPos <= seg.xr
  If the precondition is not satisfied, an assertion ends with false.

 */
 int compareInPosForMember(AVLSegment& s, int gridXPos, mpq_class& preciseXPos,
   int gridYPos, mpq_class& preciseYPos);
 /*

  ~mightIntersect~
  Checks if two AVLSegments might intersect or not.

  We take only the grid-values of each AVLSegment to estimate, if the segments
  might intersect or not. So one real segment is bordered by a bounding box,
  prepared by 6 segments.
  P.e. for a segment with the endpoints (xl,yl) and (xr,yr) with a positiv
  slope it are the segments:
  - (xl,yl,xl,yl+1)
  - (xl,yl+1, xr, yr*1)
  - (xr, yr*1, xr+1,yr+1)
  - (xr+1,yr, xr+1,yr+1)
  - (xl+1,yl,xr+1,yr)
  - (xl,yl,xl+1,yl)

 */
 bool mightIntersect(AVLSegment& s);

 /*

  ~intersect~
  returns true if both segments intersect or overlap. If it is so, the
  intersection-point or intersection-interval is saved in ~result~.

 */
 bool intersect(AVLSegment& seg, AVLSegment& result);

};

/*
 SimpleSegment and BoundingSegments are used in mightIntersect.
 An object of type BoundingSegments contains the segments which
 form the envelope of a real segment.

*/
class SimpleSegment {

private:

 int xl, yl, xr, yr;

 Position p;

public:

 SimpleSegment() {
 }
 ;

 SimpleSegment(int xL, int yL, int xR, int yR, Position p);

 int getXL() {
  return xl;
 }

 int getYL() {
  return yl;
 }

 int getXR() {
  return xr;
 }

 int getYR() {
  return yr;
 }

 Position getPosition() {
  return p;
 }

};

class BoundingSegments {
private:

 int numSeg;

 SimpleSegment* segments;

 void createBoundingSegments(Slope s, int gxl, int gyl, int gxr, int gyr);

 bool isVertical(int i);

 bool isHorizontal(int i);

 bool isLeft(int i);

 bool isRight(int i);

 bool isTop(int i);

 bool isBottom(int i);

public:
 BoundingSegments(int gxl, int gyl, int gxr, int gyr);

 ~BoundingSegments();

 bool intersect(BoundingSegments& bs);

 int getNumOfSegments() {
  return numSeg;
 }

 int getXL(int i) {
  return segments[i].getXL();
 }

 int getYL(int i) {
  return segments[i].getYL();
 }

 int getXR(int i) {
  return segments[i].getXR();
 }

 int getYR(int i) {
  return segments[i].getYR();
 }
};

class Event {

private:
 KindOfEvent kind;
 int gridX;
 int gridY;
 mpq_class preciseX;
 mpq_class preciseY;
 AVLSegment* seg;
 AVLSegment* lSeg;
 AVLSegment* gSeg;
 int noOfChangesSeg;

public:

 Event(KindOfEvent k, AVLSegment* s);

 Event(KindOfEvent k, AVLSegment* s1, AVLSegment* s2);

 Event(KindOfEvent k, AVLSegment* s1, AVLSegment* s2, AVLSegment* s3);

 Event(const Event& e);

 Event() {
 }
 ;

 ~Event();

 /*
  An Event need to be processed, when the sweep-line reaches the
  eventpoint (x,y). The eventpoint (x,y) is divided in 4 parts:
  gridX is the grid-value of the real x-value and preciseX is the precise
  part. The real x value is getGridX()+getPreciseX(). The same applies to y.

 */
 /*
  ~getGridX~

 */
 int getGridX() const;

 /*
  ~getPreciseX~

 */
 mpq_class getPreciseX() const;

 /*
  ~getGridY~

 */
 int getGridY() const;

 /*
  ~getPreciseY~

 */
 mpq_class getPreciseY() const;

 KindOfEvent getEvent() {
  return kind;
 }

 /*
  ~getSegment~
  Returns the associated segment, if ~this~ is an right- or leftendpoint-event,
  NULL otherwise;

 */
 AVLSegment* getSegment() const;

 /*
  ~getLeftSegment~
  Returns the associated lower segment, if ~this~ is an intersection-event,
  NULL otherwise;

 */
 AVLSegment* getLeftSegment() const;

 /*
  ~getRightSegment~
  Returns the associated greater segment, if ~this~ is an intersection-event,
  NULL otherwise;

 */
 AVLSegment* getRightSegment() const;

 /*
  ~getNoOfChanges~

  Between the time an event has beed created and the time, the event need to be
  processed the associated segment(s) might be changed and the event is invalid.
  To detect this, an event holds the number of changes of the associated
  segment(s).

 */
 int getNoOfChanges() const;

 Event& operator=(const Event& e);

 /*
  ~isValid~
  An Event becomes invalid, if the associated segment(s) have been changed
  between the time where the eventis put in the priority\_queue and the time
  where it has to be precessed.

 */
 bool isValid() const;

 /*
  ~isLeftEndpointEvent~

 */
 bool isLeftEndpointEvent() const;

 /*
  ~isRightEndpointEvent~

 */
 bool isRightEndpointEvent() const;

 /*
  ~isIntersectionEvent~

 */
 bool isIntersectionEvent() const;

 bool operator>(const Event& r) const;

 void print() const;

};

/*
 ~selectNext~

 This function selects the next event in dependence of the given x-coordinates.
 If the next event comes from one of the given ~l~-arguments, an leftendpoint-event
 is created from the given segment.
 If there are no segments in the ~l~-arguments and the queue is empty, the function returns
 ~none~ to show, that there is no event to process.

*/
template<class C1, class C2>
Owner selectNext(const C1& l, int& pos1, const C2& r, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event);

Owner selectNext(const Line2& l, int& pos1, const Line2& r, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event);

Owner selectNext(/*const*/ Region2& r1, int& pos1,
  /*const*/ Region2& r2, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event);

template<class C>
Owner selectNext(const C& l, int& pos,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event);


/*
 ~splitNeighbours~

 ~Current~ and ~neighbor~ intersect in  ~overlappingSegment~, which has to be a point.
 This function splits ~current~ and ~neighbor~ in 4 parts:
 ~current~ stores the left part of the original ~current~, ~rightC~ the right part,
 neighbor analog.

*/
void splitNeighbors(AVLSegment* current, AVLSegment* neighbor,
  AVLSegment* overlappingSegment, AVLSegment* rightC, AVLSegment* rightN);

/*
 ~mergeNeighbors~
 This function merges overlapping segments to one segment, stored in ~neighbor~.

*/
bool mergeNeighbors(AVLSegment* current, AVLSegment* neighbor);

/*
 ~splitNeighbors~

 The segments overlap, starting in the left endpoint of
 overlappingSegments. This function splits overlapping segments in
 max. 3 segments:
 If ~neighbor~ and ~current~ have the same leftendpoints, ~neighbor~
 contains the overlapping part and ~current~ is set to invalid.
 If ~neighbor~ lay more left than ~current~, ~neighbor~ gets the
 left part and ~current~ the overlapping part.
 ~right~ contains the part right of the overlapping part. It might
 be only the right endpoint.

*/
void splitNeighbors(AVLSegment* current, AVLSegment* neighbor,
  AVLSegment* overlappingSegment, AVLSegment* right);

/*
 ~intersectionTestForRealminize~

 This function checks whether the given AVLSegments ~left~ and ~right~
 intersect or not. If they intersect in exact one point, an intersection-event
 will be created. If there is an overlapping part, both segments will be
 merged to one segment, stored in ~right~ and ~left~ will be set to invalid.

*/
void intersectionTestForRealminize(AVLSegment* left, AVLSegment* right,
  Event* event, priority_queue<Event, vector<Event>, greater<Event> >& q,
  bool leftIsSmaller);

/*
 ~intersectionTestForSetOp~

 This function checks whether the given AVLSegments ~s1~ and ~s2~
 intersect or not. If they intersect in exact one point, an intersection-event
 will be created. If there is an overlapping part, both segments will be
 split in 3 parts: If the overlapping part starts at the same point as ~s2~,
 ~s2~ gets the overlapping part and ~s1~ is set to invalid. Otherwise
 ~s2~ contains the left part and ~s1~ the overlapping part. If there exists a
 segment right of the overlapping part, an leftentpoint-event with this
 segment is created.

*/
bool intersectionTestForSetOp(AVLSegment* s1, AVLSegment* s2, Event& event,
  priority_queue<Event, vector<Event>, greater<Event> >& q, bool leftIsSmaller);

/*
 ~collectSegmentsForInverting~

 collect all intersection-events with the event-point of ~event~ and stores
 the respective segments in ascending order in ~segmentVector~. If there are
 parallel segments in the vector, the left one has to be the left one
 after the inversion too. So they are stored in the (actual) false order in
 the ~segmentVector~. After the inversion we have the right order again.
 Because of this, the second segment of the ~segmentVector~ might be the most
 right one after the inversion and the segment before the last one in the
 ~segmentVector~might be the most left one after the inversion. The index of
 the real most left/right segment after the inversion will be stored in
 ~sucIndex~ and ~predIndex~. If there are only two segments wich are parallel,
 an inversion is not necessary and ~inversionNecessary~ will be set to false;

*/
void collectSegmentsForInverting(vector<AVLSegment*>& segmentVector,
  Event& event, priority_queue<Event, vector<Event>, greater<Event> >& q,
  size_t& predIndex, size_t& sucIndex, bool& inversionnecessary);

/*
 ~createNewSegments~

 Creates a new Halfsegment and stores it in ~result~ if it satify the conditions
 for the given SetOperation ~op~.

*/
void createNewSegments(AVLSegment& s, Line2& result, int& edgeNo,
  SetOperation op);

/*
 ~createNewSegments~

 For each AVLSegment in the ~segmentVector~ which satisfy the conditions for the
 given Setoperation ~op~ and don't end in the event-point of ~event~, a new
 Halfsegment will be created and stored in ~result~.
 The AVLSegments of the ~segmentVector~ which don't end in the event-point of
 ~event~ get the event-point as the leftendpoint.

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  Line2& result, int& edgeNo, SetOperation op);

/*
 ~createNewSegments~

 Creates a new Halfsegment and stores it in ~result~ if it satify the conditions
 for the given SetOperation ~op~.

*/
void createNewSegments(AVLSegment& s, Region2& result, int& edgeno,
  SetOperation op);

/*
 ~createNewSegments~

 For each AVLSegment in the ~segmentVector~ which satisfy the conditions for the
 given Setoperation ~op~ and don't end in the event-point of ~event~, a new
 Halfsegment will be created and stored in ~result~.
 The AVLSegments of the ~segmentVector~ which don't end in the event-point of
 ~event~ get the event-point as the leftendpoint and for some AVLSegments the
 predicates are changed.

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, Region2& result, int& edgeno, SetOperation op);

/*
 ~checkSegments~

 Checks if the given AVLSegments of the ~segmentVector~ satisfy the conditions
 of the given ~op~ and adjust the values of the predicates of the segments
 if need to be.

*/
void checkSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, bool& result, RelationshipOperation op);

/*
 ~checkSegment~

 Checks if the given AVLSegments of the ~segmentVector~ satisfy the conditions
 of the given ~op~.

*/
void checkSegment(AVLSegment& seg, bool& result, RelationshipOperation op);

/*
 ~Realminize~

*/
void Realminize(const Line2& src, Line2& result, const bool forceThrow);


/*
 ~SetOp~

 for ~line2~

*/
void SetOp(const Line2& line1, const Line2& line2, Line2& result,
  SetOperation op, const Geoid* geoid = 0);

/*
 ~SetOp~

 for ~region2~

*/
void SetOp(/*const*/ Region2& region1, /*const*/ Region2& region2,
  Region2& result, SetOperation op, const Geoid* geoid = 0);

/*
 ~intersects~

 ~line2~ x ~line2~ $->$ bool


 returns true, if there both line2-objects intersect,
 false otherwise.

*/
bool intersects(const Line2& line1, const Line2& line2, const Geoid* geoid = 0);

/*
 ~intersects~

 ~region2~ x ~region2~ $->$ ~bool~

 returns true, if both region2-objects intersect,
 false otherwise.

*/
bool intersects(/*const*/ Region2& region1, /*const*/ Region2& region2,
  const Geoid* geoid = 0);

/*
 ~overlaps~

 ~region2~ x ~region2~ $->$ ~bool~

 returns true, if both region2-objects overlap,
 false otherwise.

*/
bool overlaps(/*const*/ Region2& region1, /*const*/ Region2& region2,
  const Geoid* geoid = 0);

/*
 ~crossings~

 ~region2~ x ~region2~ $->$ ~points~

 returns the points, where the first argument crosses the second argument.

*/
void crossings(const Line2& line1, const Line2& line2, Points2& result,
  const Geoid* geoid = 0);

/*
 ~inside~

 ~region2~ x ~region2~ $->$ ~bool~

 returns true, if there the region of the first argument is completely
 inside the second argument, false otherwise.

*/
bool inside(/*const*/ Region2& region1, /*const*/ Region2& region2,
  const Geoid* geoid = 0);

} //end of p2d

#endif /* AVL_TREE_H_*/

