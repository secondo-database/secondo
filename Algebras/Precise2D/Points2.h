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

#ifndef POINTS2_H_
#define POINTS2_H_

#include <stdio.h>
#include <gmp.h>
#include <gmpxx.h>
#include "Algebra.h"
#include "../Rectangle/RectangleAlgebra.h"
#include "../../Tools/Flob/DbArray.h"
#include "NestedList.h"
#include "ListUtils.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "AttrType.h"
#include "Point2.h"

namespace p2d {

//class Point2;
//class PointData;

/*
2 Class Points2

*/
class Points2: public StandardSpatialAttribute<2> {

private:

/*
  One coordinate contains of an integer as a grid coordinate and a
  rational number as the difference between the real coordinate and
  the grid value. These rational numbers will be stored in the
  flob ~preciseCoodinates~

*/
 Flob preciseCoordinates;

/*
  pointsData stores the gridvalues, the attributes of the segments and the
  indices for the precise Coordinates.

*/
 DbArray<PointData> pointsData;

 bool ordered;

/*
  The bounding box that fully encloses all half segments of the line.

*/
 Rectangle<2> bbox;

 bool IsOrdered() {
  return ordered;
 }

/*
2.1 Functions for EndBulkLoad

*/
 void Sort();

 void MergeSort(int startIndex, int endIndex);

 void Merge(int startIndex, int divide, int endIndex);

 void RemoveDuplicates();

 void SetPartnerNo();

/*
2.2 ~ComparePoints~

 Compares two points of one ~Point2~-object

*/
 int ComparePoints(const PointData* seg1, const PointData* seg2) const;

/*
2.2 ~ComparePoints2~

 Compares two points of two different ~Point2~-objects

*/
 int ComparePoints2(const PointData& p1, const PointData& p2,
   const Flob* preciseCoordOfP2) const;



public:

/*
2.1 Constructors and destructor

*/

 inline Points2() {};

 Points2(const bool def, Point2* p);

 Points2(const Points2& cp);

 Points2(bool def);

 ~Points2() {};

/*
2.2 Memeber-functions

*/

/*
2.2.1 ~=~

*/
Points2& operator=(const Points2& p);

/*
2.2.1 getter

 For more information see class Point2

*/
 int getGridX(int i) const;

 int getGridY(int i) const;

 mpq_class getPreciseX(int i) const;

 mpq_class getPreciseY(int i) const;

 char* getPreciseXAsString(int i) const;

 char* getPreciseYAsString(int i) const;

 PointData getPoint(int i) const;

/*
2.2.2  ~getPreciseCoordinates~

  Returns a pointer to he flob, which stores the precise coordinates.

*/
 const Flob* getPreciseCoordinates() const {
  return &preciseCoordinates;
 }

/*
2.2.2 ~addPoint~

  Adds a point to the Set of points

*/
 void addPoint(Point2* p);
 void addPoint(Point2& p);
/*
2.2.2 ~Clear~

*/
 void Clear();

/*
2.2.2 ~Size~

  Return the number of points stored in the points2-object.

*/
 int Size() const;

/*
2.2.2 ~StartBulkLoad~

  Marks the points2-object as unsorted.

*/
 void StartBulkLoad();

/*
2.2.2 ~EndBulkLoad~

  Sorts the points2-object and removes duplikates.

*/
 void EndBulkLoad(bool sort = true, bool remDup = true, bool trim = true);

/*
2.3 functions required by Secondo

*/
 static const string BasicType() {
  return "points2";
 }

 static const bool checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
 }

 inline size_t Sizeof() const;

 inline size_t HashValue() const;

 inline void CopyFrom(const Attribute* right);

 inline int Compare(const Attribute *arg) const;

 inline bool Adjacent(const Attribute *arg) const {
  return false;
 }

 inline Points2* Clone() const {
  return new Points2(*this);
 }

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
   return &pointsData;
  }
 }

 static Word ClonePoints2(const ListExpr typeInfo, const Word& w);

 static void* CastPoints2(void* addr);

 static int SizeOfPoints2();

 static ListExpr OutPoints2(ListExpr typeInfo, Word value);

 static Word InPoints2(const ListExpr typeInfo, const ListExpr instance,
   const int errorPos, ListExpr& errorInfo, bool& correct);

 static Word CreatePoints2(const ListExpr typeInfo);

 static void DeletePoints2(const ListExpr typeInfo, Word& w);

 static void ClosePoints2(const ListExpr typeInfo, Word& w);

 static ListExpr Points2Property();

 static bool CheckPoints2(ListExpr type, ListExpr& errorInfo);

 virtual const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;

 virtual double Distance(const Rectangle<2>& rect,
   const Geoid* geoid = 0) const;

 virtual bool IsEmpty() const;

 virtual bool Intersects(const Rectangle<2>& rect,
   const Geoid* geoid = 0) const;
};

} // end of namespace p2d

#endif/* POINTS2_H_*/
