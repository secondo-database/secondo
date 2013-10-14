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

 This file contains same classes and operators as
 the file AVL\_Tree.h. Only the struct ~TestStruct~ is new.
 It is added to some methods to check
 how often decisions could be made without using the precise data of type
 mpq\_class.

 To save some space the comments are reduced to a minimum. For more information
 see file AVL\_Tree.h

1 Includes and defines

*/

#ifndef TESTAVLTREE_H_
#define TESTAVLTREE_H_

#include "Line2.h"
#include "Toolbox.h"
//#include "Precise2DAlgebra.h"
//#include "Region2Algebra.h"
#include <queue>
#include <vector>


namespace p2d_test{


class AVLNode;
class AVLSegment;
class Event;

/*
1 Struct TestStruct

*/
struct TestStruct{
mpz_class noCallMightIntersect;
mpz_class noCallIntersect;
mpz_class noCallCmpIntersectionIntervall;
mpz_class noCallCmpWithPreciseData;
mpz_class noCmpGrid;
mpz_class noCmpPrecise;
mpz_class noSegmentsIn;
mpz_class noSegmentsOut;

 void clear(){
  noCallMightIntersect = 0;
  noCallIntersect = 0;
  noCallCmpIntersectionIntervall = 0;
  noCallCmpWithPreciseData = 0;
  noCmpGrid = 0;
  noCmpPrecise = 0;
  noSegmentsIn = 0;
  noSegmentsOut = 0;
 }

 void print(){
  cout << "Input: "<<noSegmentsIn <<" Segmente"<<endl;
  if (noSegmentsOut >0){
    cout << "Output: "<<noSegmentsOut <<" Segmente"<<endl;
  }
  cout << "Anzahl der Aufrufe:"<<endl;
  cout << "mightIntersect: "<<noCallMightIntersect<<endl;
  cout << "intersects:     "<<noCallIntersect<<endl<<endl;
  cout << "Vergleich der Schnittpunkte mit der Sweep-Line"<<endl;
  cout << "ausschliesslich mit den Gitterdaten: "
    <<noCallCmpIntersectionIntervall<<endl;
  cout << "unter Verwendung der praezisen Daten: "
    <<noCallCmpWithPreciseData<<endl<<endl;
  cout << "Vergleich zweier Zahlen insgesamt:"<<endl;
  cout << "ausschliesslich mit den Gitterdaten: "<<noCmpGrid<<endl;
  cout << "unter Verwendung der praezisen Daten: "<<noCmpPrecise<<endl;
  cout << "Anmerkung: Hier wurden nur die Vergleiche gezaehlt, bei denen ein "
       << "Vergleich der Gitterdaten sinnvoll moeglich war. Vergleiche von "
       << "Zahlen, wie z.B. die Steigung zweier Segmente, wurden nicht "
       << "mitgezaehlt, da nur ein Ueberblick gewonnen darueber gewonnen werden"
       << " sollte, wie oft auf den Zugriff der praezisen Daten verzichtet "
       << "werden konnte, wenn eine Zahl als Tupel (int, mpq_class) vorliegt.)"
       <<endl;
 }
};

/*
2 Class TestAVLTree

*/
class AVLTree {

private:

 AVLNode* root;

public:

/*
1.1 Constructor and destructor

*/
 AVLTree();

 ~AVLTree();

/*
1.1 ~insert~

*/
 void insert(AVLSegment* x, AVLSegment*& pred, AVLSegment*& suc,
   TestStruct& t);

/*
1.1 ~remove~

*/
 void removeGetNeighbor(AVLSegment* elem, AVLSegment*& pred, AVLSegment*& suc,
   TestStruct& t);

 void removeGetNeighbor2(AVLSegment* x, int gridXPos, mpq_class& preciseXPos,
   AVLSegment*& pred, AVLSegment*& suc,  TestStruct& t);

 void removeInvalidSegment(AVLSegment* x, int gridXPos,
   mpq_class& preciseXPos,  TestStruct& t);

/*
1.1 ~invertSegments~

*/
 void invertSegments(vector<AVLSegment*>& v, int gridX, mpq_class& pX,
   int gridY, mpq_class& pY, AVLSegment*& pred, size_t predIndex,
   AVLSegment*& suc, size_t sucIndex,  TestStruct& t);

/*
1.1 ~inorder~

*/
 void inorder();
};

/*
3 Class AVLNode

*/
class AVLNode {
private:
 AVLSegment* elem;
 AVLNode* left;
 AVLNode* right;
 int height;

/*
1.1 ~setHeight~

*/
 void setHeight();

/*
1.1 ~balance~

*/
 int balance();

/*
1.1 ~counterClockwiseRotation~ and ~clockwiseRotation~

*/
 AVLNode* counterClockwiseRotation();

 AVLNode* clockwiseRotation();

/*
1.1 ~isLeaf~

*/
 bool isLeaf();

/*
1.1 ~deletemin~

*/
 AVLSegment* deletemin(AVLNode* node);

public:

/*
1.1 Constructor and destructor

*/
 AVLNode(AVLSegment* elem);

 AVLNode(const AVLNode&);

 ~AVLNode();

/*
1.1 ~=~

*/
 AVLNode& operator=(const AVLNode& node);

 AVLSegment* getElement() {
  return elem;
 }

 void setElement(AVLSegment** seg);

/*
1.1 ~insert~

*/
 AVLNode* insert(AVLSegment* x, AVLSegment*& pred, AVLSegment*& suc,
   TestStruct& t);

/*
1.1 ~remove~

*/
 AVLNode* removeGetNeighbor(AVLSegment* x, AVLSegment*& pred,
   AVLSegment*& suc,  TestStruct& t);

 AVLNode* removeGetNeighbor2(AVLSegment* x, int gridXPos, mpq_class preciseXPos,
   AVLSegment*& pred, AVLSegment*& suc,  TestStruct& t);

 AVLNode* removeInvalidSegment(AVLSegment* x, int gridXPos,
   mpq_class preciseXPos, bool& found,  TestStruct& t);

/*
1.1 ~getHeight~

*/
 int getHeight() {
  return height;
 }

/*
1.1 ~member~

*/
 AVLNode* memberPlusNeighbor(AVLSegment* key, bool& result, int gridXPos,
   mpq_class& preciseXPos, int gridYPos, mpq_class& preciseYPos,
   AVLSegment*& neighbor, bool pred,  TestStruct& t);

 AVLNode* member(AVLSegment* key, bool& result, int gridXPos,
   mpq_class& preciseXPos, int gridYPos, mpq_class& preciseYPos,
   TestStruct& t);

/*
1.1 ~inorder~

*/
 void inorder();
};

/*
1 Slope and Position

*/
enum Slope {
 vertical, horizontal, positivSlope, negativSlope
};
enum Position {
 top, bottom, pLeft, pRight
};

/*
1 ~KindOfEvent~

*/
enum KindOfEvent {
 leftEndpoint, rightEndpoint, intersectionPoint
};

/*
1 ~Owner~

*/
enum Owner {
 first, second, both, none
};

/*
1 SetOperation and RelationshipOperation

*/
enum SetOperation {
 union_op, intersection_op, difference_op
};

enum RelationshipOperation {
 intersects_op, inside_op, overlaps_op
};

/*
1 Struct ~Coordinate~

*/
struct Coordinate {
 int gx;
 int gy;
 mpq_class px;
 mpq_class py;

 //bool operator>(const Coordinate& c) const {
 bool greaterThan(const Coordinate& c, TestStruct& t)const{
  t.noCmpGrid++;
  if (gx != c.gx) {
   t.noCmpGrid++;
   if (gx > c.gx) {
    return true;
   } else {
    return false;
   }
  }
  t.noCmpPrecise++;
  int cmpX = cmp(px, c.px);
  if (cmpX != 0) {
   return cmpX > 0;
  }
  t.noCmpGrid++;
  if (gy != c.gy) {
   t.noCmpGrid++;
   if (gy > c.gy) {
    return true;
   } else {
    return false;
   }
  }
  t.noCmpPrecise++;
  return cmp(py, c.py) > 0;
 }
};

/*
1 Class AVLSegment

*/
class AVLSegment {

protected:
 int gridXL, gridYL, gridXR, gridYR;

 const Flob* flob;

 const DbArray<unsigned int>* dbarray;

 p2d::SegmentData* originalData1;

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
1.1 ~prepareData~

*/
 void prepareData(int & resultGridX, mpq_class& resultPX, mpq_class& value);

/*
1.1 ~intervalIsVertical~

*/
 bool intervalIsVertical();

/*
1.1 ~computeBeginOfIntersectionInterval~

*/
int computeBeginOfIntersectionInterval(int pos);

/*
1.1 ~computeEndOfIntersectionInterval~

*/
int computeEndOfIntersectionInterval(int pos);


/*
1.1 ~isVertical~

*/
 bool isVertical(TestStruct& t) const;

/*
1.1 ~set~

*/
 void set(mpq_class xl, mpq_class yl, mpq_class xr, mpq_class yr, Owner o,
   TestStruct& t);

 void set(int gxl, int gyl, int gxr, int gyr, mpq_class xl, mpq_class yl,
   mpq_class xr, mpq_class yr, Owner o, TestStruct& t);

public:
 AVLSegment();

 AVLSegment(const Flob* preciseData, p2d::SegmentData* sd, Owner o);

 AVLSegment(const Flob* preciseData, p2d::SegmentData* sd, Owner o,
   int scalefactor);

 AVLSegment(const DbArray<unsigned int>* preciseData, Reg2GridHalfSegment& sd,
   Reg2PrecHalfSegment* ps, Owner o);

 AVLSegment(const DbArray<unsigned int>* preciseData,
   Reg2GridHalfSegment& gs, Reg2PrecHalfSegment& ps, Owner o,
   int scalefactor);

 AVLSegment(const AVLSegment& s);

 AVLSegment(int gxl, int gyl, int gxr, int gyr, mpq_class xLeft,
   mpq_class yLeft, mpq_class xRight, mpq_class yRight, Owner o);

 ~AVLSegment();

/*
1.1 ~getter~

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

 mpq_class getPreciseXL() const;

 mpq_class getPreciseYL() const;

 mpq_class getPreciseXR() const;

 mpq_class getPreciseYR() const;

 int getNumberOfChanges() const;

 void incrementNumberOfChanges();

 bool getFirstInsideAbove() const;

 bool getSecondInsideAbove() const;

 short int getConBelow();

 short int getConAbove();

 void setConBelow(short int i);

 Owner getOwner() const {
  return owner;
 }

/*
1.1 ~setter~

*/
 void setConAbove(short int i);

 void inkrConAbove() {
  conAbove++;
 }

 void inkrConBelow() {
  conBelow++;
 }

 void dekrConAbove() {
  conAbove--;
 }

 void dekrConBelow() {
  conBelow--;
 }

 void setFirstInsideAbove(bool insideAbove);

 void setSecondInsideAbove(bool insideAbove);

 void setNumberOfChanges(int i);

 void changeValidity(bool v);

 void setOwner(Owner o) {
  owner = o;
 }

/*
1.1 ~print~

*/
 void print();

/*
1.1 test-functions

*/
 bool equal(AVLSegment& s, TestStruct& t) const;

 bool isPoint(TestStruct& t) const;

 bool isEndpoint(int gx, int gy, mpq_class px, mpq_class py, TestStruct& t);

 bool startsAtPoint(int gx, int gy, mpq_class px, mpq_class py, TestStruct& t);

 bool endsInPoint(int gx, int gy, mpq_class px, mpq_class py, TestStruct& t);

 bool isLeftOf(Event& event, TestStruct& t);

 bool leftPointIsLeftOf(Event& event, TestStruct& t);

 bool isLeftPointOf(AVLSegment& s, TestStruct& t);

 bool isRightPointOf(AVLSegment& s, TestStruct& t);

 bool hasEqualLeftEndpointAs(AVLSegment& s, TestStruct& t);

 bool isValid() const;

 bool isParallelTo(const AVLSegment& s) const;



/*
1.1 ~=~

*/
 AVLSegment& operator=(const AVLSegment& s);

/*
1.1 ~compareIntersectionintervalWithSweepline~

*/
 int compareIntersectionintervalWithSweepline(AVLSegment& s, int gridXPos,
   TestStruct& t);

/*
1.1 ~compareInPosOrRight~

*/
 int compareInPosOrRight(AVLSegment& s, int gridXPos, mpq_class& preciseXPos,
   TestStruct& t);

/*
1.1 ~compareInPosOrLeft~

*/
 int compareInPosOrLeft(AVLSegment& s, int gridXPos, mpq_class& preciseXPos,
   TestStruct& t);

/*
1.1 ~compareInPosForMember~

*/
 int compareInPosForMember(AVLSegment& s, int gridXPos, mpq_class& preciseXPos,
   int gridYPos, mpq_class& preciseYPos, TestStruct& t);
/*
1.1 ~mightIntersect~

*/
 bool mightIntersect(AVLSegment& s, TestStruct& t);

/*
1.1 ~intersect~

*/
 bool intersect(AVLSegment& seg, AVLSegment& result, TestStruct& t);

};

/*
1 Classes SimpleSegment and BoundingSegments

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

/*
1 Class Event

*/
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

/*
1.1 Constructors and destructor

*/
 Event(KindOfEvent k, AVLSegment* s);

 Event(KindOfEvent k, AVLSegment* s1, AVLSegment* s2);

 Event(KindOfEvent k, AVLSegment* s1, AVLSegment* s2, AVLSegment* s3);

 Event(const Event& e);

 Event() {};

 ~Event();

/*
1.1 ~getter~

*/
 int getGridX() const;

 mpq_class getPreciseX() const;

 int getGridY() const;

 mpq_class getPreciseY() const;

 KindOfEvent getEvent() {
  return kind;
 }

 AVLSegment* getSegment() const;

 AVLSegment* getLeftSegment() const;

 AVLSegment* getRightSegment() const;

 int getNoOfChanges() const;

/*
1.1 ~=~

*/
 Event& operator=(const Event& e);

/*
1.1 ~isValid~

*/
 bool isValid() const;

/*
1.1 ~isLeftEndpointEvent~

*/
 bool isLeftEndpointEvent() const;

/*
1.1 ~isRightEndpointEvent~

*/
 bool isRightEndpointEvent() const;

/*
1.1 ~isIntersectionEvent~

*/
 bool isIntersectionEvent() const;

/*
1.1 ~$>$~

*/
 bool operator>(const Event& r) const;

/*
1.1 ~isIntersectionEvent~

*/
 void print() const;

};



/*
1 ~selectNext~

*/
template<class C1, class C2>
Owner selectNext(const C1& l, int& pos1, const C2& r, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  TestStruct& t);

template<class C1, class C2>
Owner selectNext(const C1& l, int& pos1, const C2& r, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  int scalefactor, TestStruct& t);

Owner selectNext(const p2d::Line2& l, int& pos1, const p2d::Line2& r, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  TestStruct& t);

Owner selectNext(const p2d::Line2& l, int& pos1, const p2d::Line2& r, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  int scalefactor, TestStruct& t);

Owner selectNext(/*const*/ Region2& r1, int& pos1,
 /*const*/ Region2& r2, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  TestStruct& t);

Owner selectNext(/*const*/Region2& r1, int& pos1,
/*const*/Region2& r2, int& pos2,
  priority_queue<Event, vector<Event>, greater<Event> >& q, Event& event,
  int scalefactor, TestStruct& t);

/*
1 ~splitNeighbors~

*/
void splitNeighbors(AVLSegment* current, AVLSegment* neighbor,
  AVLSegment* overlappingSegment, AVLSegment* right, TestStruct& t);


/*
1 ~intersectionForSetOp~

*/
bool intersectionTestForSetOp(AVLSegment* s1, AVLSegment* s2, Event& event,
  priority_queue<Event, vector<Event>, greater<Event> >& q, bool leftIsSmaller,
  TestStruct& t);

/*
1 ~collectSegmentsForInverting~

*/
void collectSegmentsForInverting(vector<AVLSegment*>& segmentVector,
  Event& event, priority_queue<Event, vector<Event>, greater<Event> >& q,
  size_t& predIndex, size_t& sucIndex, bool& inversionnecessary, TestStruct& t);

/*
1 ~createNewSegments~

*/
void createNewSegments(AVLSegment& s, p2d::Line2& result, int& edgeNo,
  SetOperation op);

void createNewSegments(AVLSegment& s, p2d::Line2& result, int& edgeNo,
  SetOperation op, int scalefactor);

/*
1 ~createNewSegments~

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  p2d::Line2& result, int& edgeNo, SetOperation op, TestStruct& t);

void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  p2d::Line2& result, int& edgeNo, SetOperation op,
  int scalefactor, TestStruct& t);

/*
1 ~createNewSegments~

*/
void createNewSegments(AVLSegment& s, Region2& result, int& edgeno,
  SetOperation op);

void createNewSegments(AVLSegment& s, Region2& result, int& edgeno,
  SetOperation op, int scalefactor);
/*
1 ~createNewSegments~

*/
void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, Region2& result, int& edgeno, SetOperation op,
  TestStruct& t);

void createNewSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, Region2& result, int& edgeno, SetOperation op,
  int scalefactor, TestStruct& t);

/*
1 ~checkSegments~

*/
void checkSegments(vector<AVLSegment*>& segmentVector, Event& event,
  AVLSegment* successor, bool& result, RelationshipOperation op, TestStruct& t);

/*
1 ~checkSegment~

*/
void checkSegment(AVLSegment& seg, bool& result, RelationshipOperation op);


/*
1 ~SetOp~

*/
void SetOp(const p2d::Line2& line1, const p2d::Line2& line2, p2d::Line2& result,
  SetOperation op, TestStruct& t, const Geoid* geoid = 0);

void SetOpWithScaling(const p2d::Line2& line1, const p2d::Line2& line2,
  p2d::Line2& result, SetOperation op, TestStruct& t, const Geoid* geoid = 0);

void SetOp(/*const*/ Region2& region1,/*const*/ Region2& region2,
  Region2& result, SetOperation op, TestStruct& t, const Geoid* geoid = 0);

void SetOpWithScaling(/*const*/ Region2& region1,/*const*/ Region2& region2,
  Region2& result, SetOperation op, TestStruct& t, const Geoid* geoid = 0);
/*
1 ~intersects~

*/
bool intersects(const p2d::Line2& line1, const p2d::Line2& line2,
  TestStruct& t, const Geoid* geoid = 0);

bool intersectsWithScaling(const p2d::Line2& line1, const p2d::Line2& line2,
  TestStruct& t, const Geoid* geoid = 0);

/*
1 ~intersects~

*/
bool intersects(/*const*/ Region2& region1,/*const*/ Region2& region2,
  TestStruct& t, const Geoid* geoid = 0);

bool intersectsWithScaling(/*const*/ Region2& region1,
  /*const*/ Region2& region2, TestStruct& t, const Geoid* geoid = 0);


} //end of p2d
#endif/* AVLTREE_H_*/
