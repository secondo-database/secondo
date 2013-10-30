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

#ifndef _Line2_H
#define _Line2_H

#include <stdio.h>
#include <gmp.h>
#include <gmpxx.h>
#include "Algebra.h"
#include "RectangleAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "AttrType.h"
//#include "Precise2DAlgebra.h"
//#include "Point2.h"
#include "SpatialAlgebra.h"
#include "Region2Algebra.h"
//#include "Toolbox.h"

namespace p2d {

class Point2;
class Points2;
//class Region2;

/*
2 Class ~SegmentData~

 The class ~SegmentData~ describes the precise representation of a
 segment.
 The absolute coordinates of the 2 segmentpoints are stored in 4 variables of
 type int. The difference between the absolute coordinates and their respective
 representation will be stored in a flob. ~xLeftPos~ is the index where the
 x-coordinate of the left point in this flob starts and ~xLeftNumOfChars~ is
 his length (analog the y-coordinate and the right point)

*/
class SegmentData {

/*
2.1 Private attributes

  * ~xLeft~, ~yLeft~, ~xRight~ and ~yRight~ is the absolute value of the
  given coordinate

  * ~xLeftPos~, ~yLeftPos~, ~xRightPos~, ~yRightPos~ is
  the index position of the first char representing the given
  coordinate in the flob.

  * ~xLeftNumOfChars~, ~yLeftNumOfChars~, ~xRightNumOfChars~,
  ~yRightNumOfChars~ is the number of all chars representing this
  respective coordinate instance in the flob.

*/
 private:
 int xLeft;
 int yLeft;
 int xRight;
 int yRight;

 size_t xLeftPos;
 size_t yLeftPos;
 size_t xRightPos;
 size_t yRightPos;
 size_t xLeftNumOfChars;
 size_t yLeftNumOfChars;
 size_t xRightNumOfChars;
 size_t yRightNumOfChars;

/*
  Indicates whether the left point is the dominating point or the
  secondary point

*/
 bool ldp;

/*
  The face identifier

*/
 int faceno;

/*
  The cycle identifier

*/
 int cycleno;

/*
  The edge (segment) identifier

*/
 int edgeno;

/*
  Used for fast spatial scan of the inside\_pr algorithm

*/
 int coverageno;

/*
  Indicates whether the region's area is above or left of its segment

*/
 bool insideAbove;

/*
  Stores the position of the partner half segment in half segment
  ordered array

*/
 int partnerno;

public:

/*
2.2 Constructors and destructor

*/

 SegmentData() {}

/*
  Constructor that initializes all the grid values.

*/
 SegmentData(int xLeft, int yLeft, int xRight, int yRight, bool ldp,
   int edgeNo = -999999);

/*
   Constructs a new ~SegmentData~-Object with the given endpoints.

*/
 SegmentData(Point2* lp, Point2* rp, bool ldp, int edgeno,
   Flob& preciseCoordinates);

 SegmentData(SegmentData& d);

/*
2.3 Member functions

*/


/*
2.3.1 Read access methods

*/
 int getLeftX(void) const {
  return xLeft;
 }
 int getLeftY(void) const {
  return yLeft;
 }
 int getRightX(void) const {
  return xRight;
 }
 int getRightY(void) const {
  return yRight;
 }

/*
  For the difference between the absolute coordinates and their respective
  representation:
  All these methods fetch the chars representing the given coordinate from
  the flob using the indices given in this instance's private attributes, and
  convert them to the correct instance of type mpq\_class, representing the
  value of the given coordinate.

*/
 mpq_class getPreciseLeftX(const Flob& preciseCoordinates) const;
 mpq_class getPreciseLeftY(const Flob& preciseCoordinates) const;
 mpq_class getPreciseRightX(const Flob& preciseCoordinates) const;
 mpq_class getPreciseRightY(const Flob& preciseCoordinates) const;

/*
  Returns the named value of the dominating point of the half segment.

*/
 const int GetDomGridXCoord() const;
 const int GetDomGridYCoord() const;
 const mpq_class GetDomPreciseXCoord(const Flob& preciseCoordinates) const;
 const mpq_class GetDomPreciseYCoord(const Flob& preciseCoordinates) const;

/*
  Returns the named value of the secondary point of the half segment.

*/
 const int GetSecGridXCoord() const;
 const int GetSecGridYCoord() const;
 const mpq_class GetSecPreciseXCoord(const Flob& preciseCoordinates) const;
 const mpq_class GetSecPreciseYCoord(const Flob& preciseCoordinates) const;

 int GetEdgeno() {
  return edgeno;
 }
 int GetPartnerno() {
  return partnerno;
 }
 bool GetInsideAbove() {
  return insideAbove;
 }
 int GetCoverageno() {
  return coverageno;
 }
 int GetCycleno() {
  return cycleno;
 }
 int GetFaceno() {
  return faceno;
 }
 int GetEdgeNo() {
  return edgeno;
 }

/*
2.3.1 Write access methods

*/
 void SetLeftX(int lx) {
  xLeft = lx;
 }

 void SetLeftY(int ly) {
  yLeft = ly;
 }

 void SetRightX(int rx) {
  xRight = rx;
 }

 void SetRightY(int ry) {
  yRight = ry;
 }

 void SetPreciseLeftX(Flob& preciseCoordinates, mpq_class lx);
 void SetPreciseLeftY(Flob& preciseCoordinates, mpq_class ly);
 void SetPreciseRightX(Flob& preciseCoordinates, mpq_class rx);
 void SetPreciseRightY(Flob& preciseCoordinates, mpq_class ry);
 void SetPreciseLeftCoordinates(Point2* lp, Flob& preciseCoordinates);
 void SetPreciseRightCoordinates(Point2* rp, Flob& preciseCoordinates);
 void addPreciseValues(Flob& f, mpq_class pLeftX, mpq_class pLeftY,
   mpq_class pRightX, mpq_class pRightY);

/*
  Returns the boolean flag which indicates whether the dominating point is on
  the left side.

*/
 bool IsLeftDomPoint() const;

 void SetPartnerno(int no);
 void SetInsideAbove(bool b);
 void SetCoverageno(int no);
 void SetCycleno(int no);
 void SetFaceno(int no);
 void SetEdgeNo(int no);


/*
2.3.1 ~=~

*/
  SegmentData& operator=(const SegmentData& l);
/*
2.3.1 ~BoundingBox~

*/
 const Rectangle<2> BoundingBox(const Flob& preciseCoordinates,
   const Geoid* geoid = 0) const;

 bool IsVertical(const Flob& preciseCoordinates) const;

};

/*
3 Class Line2

*/
class Line2: public StandardSpatialAttribute<2> {

private:

/*
  One coordinate consists of an integer as a grid coordinate and a
  rational number as the difference between the real coordinate and
  the grid value. These rational numbers will be stored in the
  Flob ~preciseCoodinates~

*/
 Flob preciseCoordinates;

/*
  segmentData stores the grid values, the attributes of the segments and the
  indices for the precise Coordinates.

*/
 DbArray<SegmentData> segmentData;

 bool ordered;

/*
  The number of components for the line.

*/
 int noComponents;

/*
  The bounding box that fully encloses all half segments of the line.

*/
 Rectangle<2> bbox;

/*
3.1 Private functions used in EndBulkLoad

*/

/*
  ~Sort~

  Sorts the segments using mergesort.

*/
 void Sort();

 void MergeSort(int startIndex, int endIndex);

 void Merge(int startIndex, int divide, int endIndex);

  void SetPartnerNo();

/*
3.2  ~SegmentIsVertical~

  Returns true if a segment with the given x-coordinates is vertical,
  false otherwise.

*/
 bool SegmentIsVertical(int lx, mpq_class plx, int rx, mpq_class prx);

/*
3.3  ~CompareSegment~

  Compares the two given segments of one ~Line2~-Object. Returns
  1	if ~seg1~ $>$ ~seg2~,
  0	if ~seg1~ $=$ ~seg2~ and
  -1	if ~seg1~ $<$ ~seg2~

*/
 int CompareSegment(const SegmentData& seg1, const SegmentData& seg2) const;


/*
3.3   ~CompareSegment2~

   Compares the two given segments of two different ~Line2~-Objects. Returns
   1 if ~seg1~ $>$ ~seg2~,
   0 if ~seg1~ $=$ ~seg2~ and
   -1  if ~seg1~ $<$ ~seg2~

*/
 int CompareSegment2(const SegmentData& seg1, const SegmentData& seg2,
   const Flob& preciseCoordOfSeg2) const;



public:

/*
3.1 Constructors and destructor

*/
Line2();

 Line2(const bool def, bool ldp, int xl, int yl, int xr, int yr, mpq_class pxl,
   mpq_class pyl, mpq_class pxr, mpq_class pyr);

 Line2(const Line2& cp);

 Line2(bool def);

 ~Line2() {};

/*
3.2 Member-functions

*/

/*
3.2.1 Read access methods

  The following functions returns the asked coordinate of the
  ~i~-th segment of the line2-object.

*/
 int getLeftGridX(int i) const;

 int getLeftGridY(int i) const;

 int getRightGridX(int i) const;

 int getRightGridY(int i) const;

 mpq_class getPreciseLeftX(int i) const;

 mpq_class getPreciseLeftY(int i) const;

 mpq_class getPreciseRightX(int i) const;

 mpq_class getPreciseRightY(int i) const;

/*
3.2.2 ~get~

  Returns the ~i~-th SegmentData-object of the line2-object.

*/
 void get(int i, SegmentData&) const;

/*
3.2.3 ~getPreciseCoordinates~

  Returns a pointer to he flob, which stores the precise coordinates.

*/
 const Flob* getPreciseCoordinates() const {
  return &preciseCoordinates;
 }

/*
3.2.4 ~addSegment~

  Adds a segment to the line2-object.

*/
 void addSegment(bool ldp, int leftX, int leftY, int rightX, int rightY,
   mpq_class pLeftX, mpq_class pLeftY, mpq_class pRightX, mpq_class pRightY,
   int edgeNo);

 void addSegment(bool ldp, Point2* lp, Point2* rp, int edgeno);

/*
3.2.4 ~More functions~

*/
 void Clear();

 bool IsOrdered() {
  return ordered;
 }

 Line2& operator=(const Line2& l);

 bool IsLeftDomPoint(int i) const;

/*
  ~Size~

  Returns the number of segments in this line2-object.

*/
 int Size() const;

/*
  ~Destroy~

  Deletes the segments

*/
 void Destroy();

/*
 Helper-functions for EndBulkLoad

*/
 void collectFace(int faceno, int startPos, DbArray<bool>& used);

 int getUnusedExtension(int startPos, const DbArray<bool>& used) const;

 void computeComponents();

/*
  ~StartBulkLoad~

*/
 void StartBulkLoad();

/*
  ~EndBulkLoad~

  Sorts the segments, realminize them and updates the attributes.

*/
 void EndBulkLoad(bool sort = true, bool realminize = true, bool trim = true);

/*
3.2 Set Operators

*/

/*
  Union computes the union of two line2-objects

*/
 void unionOP(Line2& l2, Line2& res, const Geoid* geoid = 0);
 void unionWithScaling(Line2& l2, Line2& res, const Geoid* geoid = 0);

/*
  Intersection computes the intersection of two line2-objects

*/
 void intersection(Line2& l2, Line2& res, const Geoid* geoid = 0);
 void intersectionWithScaling(Line2& l2, Line2& res, const Geoid* geoid = 0);

/*
  Minus returns all segments and parts of segments of ~this~ which are not
  in the ~l2~. The result will be stored in ~res~

*/
 void minus(Line2& l2, Line2& res, const Geoid* geoid = 0);
 void minusWithScaling(Line2& l2, Line2& res, const Geoid* geoid = 0);

/*
3.2 ~intersect~

  Returns true, if ~this~ intersect ~l2~, false otherwise.

*/
 bool intersects(Line2& l2, const Geoid* geoid = 0);
 bool intersectsWithScaling(Line2& l2, const Geoid* geoid = 0);
/*
3.2 ~crossings~

  ~result~ will contain all crossing-points of ~this~ and ~l2~.

*/
 void crossings(Line2& l2, Points2& result, const Geoid* geoid = 0);
 void crossingsWithScaling(Line2& l2, Points2& result, const Geoid* geoid = 0);
/*
3.2 Functions required by Secondo and virtual functions
of the StandardSpatialAttribute

*/

 static const string BasicType() {
  return "line2";
 }

 static const bool checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
 }

 size_t Sizeof() const;

 size_t HashValue() const;

 void CopyFrom(const Attribute* right);

 int Compare(const Attribute *arg) const;

 bool Adjacent(const Attribute *arg) const;

 Line2* Clone() const;

 ostream& Print(ostream &os) const {
  os << *this;
  return os;
 }

 int NumOfFLOBs(void) const {
  return 2;
 }

 Flob *GetFLOB(const int i) {
  assert(0 <= i && i <= 1);
  if (i == 0) {
   return &preciseCoordinates;
  } else {
   return &segmentData;
  }
 }

 static Word CloneLine2(const ListExpr typeInfo, const Word& w);

 static void* CastLine2(void* addr);

 static int SizeOfLine2();

 static ListExpr OutLine2(ListExpr typeInfo, Word value);

 static Word InLine2(const ListExpr typeInfo, const ListExpr instance,
   const int errorPos, ListExpr& errorInfo, bool& correct);

 static Word CreateLine2(const ListExpr typeInfo);

 static void DeleteLine2(const ListExpr typeInfo, Word& w);

 static void CloseLine2(const ListExpr typeInfo, Word& w);

 static ListExpr Line2Property();

 static bool CheckLine2(ListExpr type, ListExpr& errorInfo);

 virtual const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;

 virtual double Distance(const Rectangle<2>& rect,
   const Geoid* geoid = 0) const;

 virtual bool IsEmpty() const;

 virtual bool Intersects(const Rectangle<2>& rect,
   const Geoid* geoid = 0) const;
};

/*
4 ~convertLineIntoLine2~

 This function converts a line-object in a line2-object.

*/
void convertLineToLine2(Line& l, Line2& result);

} // end of namespace p2d

#endif/* _Line2_H*/
