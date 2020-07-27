/*
----
This file is part of SECONDO.

Copyright (C) 2020,
Faculty of Mathematics and Computer Science,
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


//[$][\$]
//[_][\_]

*/
#ifndef TSDTWOP_H
#define TSDTWOP_H

#include "Stream.h"
#include "../Relation-C++/RelationAlgebra.h"
#include "../Algebras/OrderedRelation/OrderedRelationAlgebra.h"
#include "../ExtRelation-2/TupleBuffer2.h"
#include "tsop.h"
#include "timeseriesdistanceopli.h"

using std::vector;

struct WarpingPathPoint
{
  int _x;
  int _y;

  WarpingPathPoint(int x, int y)
  {
      cout << "[" << std::to_string(x) << ":"
           << std::to_string(y) << "]" << endl;
      this->_x = x;
      this->_y = y;
  }

  std::string toString()
  {
      return "["+ std::to_string(this->_x) + "," +
              std::to_string(this->_y) +"]";
  }
};

class TsDTWOp  : public TsOp
{
public:
    TsDTWOp();
    static ListExpr tsDTWTypeMap(ListExpr);
    static int tsDTWValueMap(Word*, Word&, int, Word&, Supplier);
    static int tsDDTWValueMap(Word*, Word&, int, Word&, Supplier);
    static int tsDTWSCValueMap(Word*, Word&, int, Word&, Supplier);
    static ListExpr tsDTWSCTypeMap(ListExpr);


private:
    static double euklidianDistance(Tuple*, Tuple*);
    static ListExpr checkAttributes(ListExpr);
    static ListExpr createResultType(ListExpr, ListExpr);
    static int countValues(ListExpr);
    static void optimalWarpingPath(vector<vector<double>>&,
                                   vector<WarpingPathPoint>&);
    static void optimalWarpingPath(vector<vector<double>>&,
                                   vector<WarpingPathPoint>&, int band_limit);
    static double computeDerivative(double qiminus1,
                                    double qi, double qiplus1);
    static double derivativeDistance(double derivative_ts1,
                                     double derivative_ts2);
};

#endif // TSDTWOP_H
