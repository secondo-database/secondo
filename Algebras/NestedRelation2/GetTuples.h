/*
 ----
 This file is part of SECONDO.

 Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

 1.1 GetTuples

 Via "gettuples"[2] a collection of specific tuples, identified by their tuple
 id, can be extracted from the given relation. This is useful if using an index.

*/

#ifndef ALGEBRAS_NESTEDRELATION2_OPERATORS_GETTUPLES_H_
#define ALGEBRAS_NESTEDRELATION2_OPERATORS_GETTUPLES_H_

#include "Stream.h"

namespace nr2a {

class GetTuples
{
  public:
    struct Info : OperatorInfo
    {
      Info();
    };
    virtual ~GetTuples();

    static ListExpr MapType(ListExpr args);
    static ValueMapping functions[];
    static int SelectFunction(ListExpr args);
    static int GetTuplesArel(Word* args, Word& result, int message,
        Word& local, Supplier s);
    static int GetTuplesNrel(Word* args, Word& result, int message,
        Word& local, Supplier s);

    static CreateCostEstimation costEstimators[];

  protected:

  private:
    GetTuples(); // Declared, but not defined => Linker error on usage
    static ListExpr BuildResultingType(const ListExpr attributesIndex,
        const ListExpr attributesNrel);
};

} /* namespace nr2a*/

#endif /* ALGEBRAS_NESTEDRELATION2_OPERATORS_GETTUPLES_H_*/
