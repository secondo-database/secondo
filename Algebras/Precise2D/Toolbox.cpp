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

#include "Toolbox.h"

namespace p2d {

/*
 ~createCoordinate~

 If the first argument is of type int or real, then the grid-value and the
 precise- value is calculated and the function returns true. If not, then
 the function returns false.

*/
bool createCoordinate(ListExpr& value, int& grid, mpq_class& precise){
 if (nl->AtomType(value)==IntType){
  grid= nl->IntValue(value);
  precise=0;
  return true;
 } else {
  if (nl->AtomType(value)==RealType){
   double d = nl->RealValue(value);
   createValue(d, grid, precise);
   /*
    grid= (int) d;

    if ((d<0) && (d != grid)){
     //d has a decimal
     grid--;
    }
    assert ((0<=(d-grid))&&((d-grid)<1));
    precise = p2d::computeMpqFromDouble((d-grid));
    return true;
    */
   return true;
  }
 }
 return false;
}

void createValue(double value, int& grid, mpq_class& precise){
 grid= (int) value;

 if ((value<0) && (value != grid)){
  //value has a decimal and is less 0, the grid value is the
  //next smallest integer
  grid--;
 }
 assert((0<=(value-grid))&&((value-grid)<1));
 precise = p2d::computeMpqFromDouble((value-grid));
}

void createPoint2(const double x, const double y, Point2** result){
 mpq_class preciseX, preciseY;
 int gX, gY;
 createValue(x, gX, preciseX);
 createValue(y, gY, preciseY);
 (*result) = new Point2(true, gX, gY, preciseX, preciseY);

}

const double Factor = 0.00000001;

bool AlmostEqual(double a, double b) {
 double diff = abs(a - b);
 return (diff < Factor);
}

mpq_class computeMpqFromDouble(double value) {
 int denom = 1;
 int num = 0;
 while (!AlmostEqual(value, 0.0) && denom < 100000000) {

  denom = denom * 10;
  int n = round(value * 10.0);
  num = num * 10 + n;
  value = (value * 10.0) - n;
  if (AlmostEqual(value, 1.0)) {
   num++;
   value = value - 1.0;
  }
 }
 mpq_class result(num, denom);
 result.canonicalize();
 return result;
}

} /* namespace p2d */
