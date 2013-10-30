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


0 Overview

1 Includes and defines

*/

#ifndef TOOLBOX_H_
#define TOOLBOX_H_

#include <stdio.h>
#include <gmp.h>
#include <gmpxx.h>
#include "NestedList.h"
#include "Line2.h"
//#include "Point2.h"
//#include "Points2.h"
#include "Region2Algebra.h"

namespace p2d {

class Point2;
//class Line2;

//class Point2;
//class Line2;
//class Region2;

/*
1 ~createCoordinate~

*/
bool createCoordinate(ListExpr& value, int& grid, mpq_class& precise);

/*
1 ~createValue~

*/
void createValue(double value, int& gx, mpq_class& py);

/*
1 ~createPoint2~

*/
void createPoint2(const double x, const double y, Point2** result);

/*
1 ~createAlmostEqual~

*/
bool AlmostEqual(double a, double b);

/*
1 ~computeMpqFromDouble~

*/
mpq_class computeMpqFromDouble(double value);

/*
1 ~ceil\_mpq~

*/
mpz_class ceil_mpq(mpq_class& value);

/*
1 ~floor\_mpq~

*/
mpz_class floor_mpq(mpq_class& value);

/*
1 ~prepareData~
computes the grid value ~resultGridX~ = floor(~value~) and
the difference (~value~-~resultGridX~)=~resultPX~.

*/
void prepareData(int& resultGridX, mpq_class& resultPX,
  mpq_class value);

/*
1 ~computeScalefactor~

 Some operators scale the region-objects before the plane-sweep starts.
 The scalefactor is a power of 10, so that the greatest value has max. 5 digits.

*/
mpq_class computeScalefactor(Region2& reg1, Region2& reg2);

mpq_class computeScalefactor(Region2& reg1);

mpq_class computeScalefactor(const Line2& line1, const Line2& line2);
} /* namespace p2d */
#endif /* TOOLBOX_H_ */
