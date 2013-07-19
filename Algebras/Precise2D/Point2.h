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

#ifndef _POINT2_H
#define _POINT2_H

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
#include "Precise2DAlgebra.h"

namespace p2d {

class Point2;

/*
2 Class ~PointData~

 The coordinates of a Point2-object will be stored in a Flob. Every Point-object
 stores the index and the length of the x- resp. y-coordinate in a
 PointData-object.

*/

class PointData {
private:
 // Gridvalue of the x- and y-coodinates
 int x, y;

 // Startposition and length of the precise x- and y- coordinates
 // in the Flob.
 size_t xStartPos, yStartPos;
 size_t xNumOfChars, yNumOfChars;

public:

/*
2.1 Constructor and Deconstructor

*/

/*
  PointData will be initialized with the grid-values. The indexes and
  the length for the precise coordinates will be set to 0.

*/
 PointData(const int xGrid, const int yGrid);

 PointData(const PointData& pd);

 inline PointData() {
 }
 ;

 ~PointData() {
 }
 ;

/*
2.2 Member functions

*/

/*
2.2.1 ~getGridX~ and ~getGridY~

  Returns the grid-value of the coordinate.

*/
 int getGridX() const {
  return x;
 }
 int getGridY() const {
  return y;
 }

/*
2.2.2 ~getPreciseX~ and ~getPreciseY~

  These methods fetch the chars representing the given coordinate from
  the Flob using the indices given in this instance's private attributes,
  and convert them to the correct instance of type mpq\_class, representing
  the value of the given coordinate.

*/
 void getPreciseX(const Flob* preciseCoordinates, mpq_class& result) const;
 void getPreciseY(const Flob* preciseCoordinates, mpq_class& result) const;

/*
2.2.3 ~getPreciseXAsString~ and ~getPreciseYAsString~

  These methods returns the chars representing the given coordinate from
  the Flob using the indices given in this instance's private attributes.

*/
 char* getPreciseXAsString(const Flob* preciseCoordinates) const;
 char* getPreciseYAsString(const Flob* preciseCoordinates) const;

/*
2.2.4 ~setGridX~ and ~setGridY~

  These methods stores the grid-value of the coordinates.

*/
 void SetGridX(int xp);
 void SetGridY(int yp);

/*
2.2.5 ~setPreciseX~ and ~setPreciseY~

  These methods writes the precise coordinate as a set of chars in the flob.

*/
 void SetPreciseX(Flob* preciseCoordinates, mpq_class x);
 void SetPreciseY(Flob* preciseCoordinates, mpq_class y);

/*
2.2.6 ~CopyPreciseCoordinates~

  Copies the precise coordinates from the given point2-object in the flob.

*/
 void CopyPreciseCoordinates(Point2* p, Flob* preciseCoordinates);

 /*
 2.2.6 ~Intersects~

   Copies the precise coordinates from the given point2-object in the flob.

 */
 bool Intersects(const Rectangle<2>& rect,
   const Geoid* geoid/*=0*/, const Flob* preciseCoordinates) const;
};

/*
3 Class ~Point2~

*/
class Point2: public StandardSpatialAttribute<2> {

private:
 Flob preciseCoordinates;
 PointData pointData;

public:

/*
3.1 Constructors and Deconstructor

*/
 inline Point2() {
 }
 ;

 Point2(const bool def, const int xCoord, int yCoord, mpq_class preciseX,
   mpq_class preciseY);

 Point2(const Point2& cp);

 Point2(bool def);

 ~Point2() {
 }
 ;

/*
3.2 read access methods

*/
 int getGridX() const;

 int getGridY() const;

 mpq_class& getPreciseX() const;

 mpq_class& getPreciseY() const;

 char* getPreciseXAsString() const;

 char* getPreciseYAsString() const;

/*
3.3 comparison-operators

*/
 bool operator==(const Point2& p) const;
 bool operator<=(const Point2& p) const;
 bool operator<(const Point2& p) const;
 bool operator>=(const Point2& p) const;
 bool operator>(const Point2& p) const;

/*
3.4 functions required by Secondo

*/
 static const string BasicType() {
  return "point2";
 }

 static const bool checkType(const ListExpr type) {
  return listutils::isSymbol(type, BasicType());
 }

 inline size_t Sizeof() const;

 inline size_t HashValue() const {
  if (!IsEmpty())
   return 0;
  return (size_t)(5 * pointData.getGridX() + pointData.getGridY());
 }

 inline void CopyFrom(const Attribute* right);

 inline int Compare(const Attribute *arg) const;

 inline bool Adjacent(const Attribute *arg) const {
  return false;
 }

 inline Point2* Clone() const {
  return new Point2(*this);
 }

 ostream& Print(ostream &os) const {
  os << *this;
  return os;
 }

 int NumOfFLOBs(void) const {
  return 1;
 }

 Flob *GetFLOB(const int i) {
  return &preciseCoordinates;
 }

 static Word ClonePoint2(const ListExpr typeInfo, const Word& w);

 static void* CastPoint2(void* addr);

 static int SizeOfPoint2();

 static ListExpr OutPoint2(ListExpr typeInfo, Word value);

 static Word InPoint2(const ListExpr typeInfo, const ListExpr instance,
   const int errorPos, ListExpr& errorInfo, bool& correct);

 static Word CreatePoint2(const ListExpr typeInfo);

 static void DeletePoint2(const ListExpr typeInfo, Word& w);

 static void ClosePoint2(const ListExpr typeInfo, Word& w);

 static ListExpr Point2Property();

 static bool CheckPoint2(ListExpr type, ListExpr& errorInfo);

 virtual const Rectangle<2> BoundingBox(const Geoid* geoid = 0) const;

 virtual double Distance(const Rectangle<2>& rect,
   const Geoid* geoid = 0) const;
 virtual bool IsEmpty() const;

 virtual bool Intersects(const Rectangle<2>& rect,
   const Geoid* geoid = 0) const;
};

} // end of namespace p2d

#endif/* _POINT2_H*/
