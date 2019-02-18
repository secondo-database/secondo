/*
----
This file is part of SECONDO.

Copyright (C) 2019,
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


//[<] [\ensuremath{<}]
//[>] [\ensuremath{>}]

\setcounter{tocdepth}{3}
\tableofcontents



1 Import Operators Header

*/
#pragma once
#include <memory>

#include "AlgebraTypes.h"
#include "Operator.h"

namespace pointcloud2 {
/*
1.1 importxyz Operator a.k.a. CVS-import

*/
class op_importxyz{
    static ListExpr importxyzTM(ListExpr args);

    static int importxyzVMT( Word* args, Word& result, int message,
            Word& local, Supplier s );

    std::string getOperatorSpec();

public:
    explicit op_importxyz() = default;
    ~op_importxyz() = default;

    std::shared_ptr<Operator> getOperator();
};

/*
1.2 importPc2FromLas Operator
importPc2FromLas: {string, text} -> pointcloud2(X)

*/

class op_importPc2FromLas{
    static ListExpr tupleTypeOfLasPointFormat(const int format);

    static ListExpr importPc2FromLasTM(ListExpr args);

    static int importPc2FromLasVMT( Word* args, Word& result, int message,
            Word& local, Supplier s );

    std::string getOperatorSpec();

public:
    explicit op_importPc2FromLas() = default;
    ~op_importPc2FromLas() = default;

    std::shared_ptr<Operator> getOperator();
};

/*
1.3 importPc2FromStl Operator

*/
class op_importPc2FromStl {
    static ListExpr importPc2FromStlTM(ListExpr args);

    static int importPc2FromStlVM( Word* args, Word& result, int message,
            Word& local, Supplier s );

    std::string getOperatorSpec();

public:
    explicit op_importPc2FromStl() = default;
    ~op_importPc2FromStl() = default;

    std::shared_ptr<Operator> getOperator();

};
} //end of namespace pointcloud2
