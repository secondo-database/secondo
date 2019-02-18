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



1 Basic Operators Header File

*/
#pragma once
#include "../tcPointcloud2.h"
#include "../utility/BitArray.h"

namespace pointcloud2{

/*
1.1 The Pointcloud2 restrict Operator

*/
class op_restrict {
    static ListExpr restrictTM(ListExpr args);

    static int restrictVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_restrict() = default;
    ~op_restrict() = default;

    std::shared_ptr<Operator> getOperator();

};


/*
1.2 The Pointcloud2 restrictXY Operator

*/
class op_restrictXY {
    static ListExpr restrictXYTM(ListExpr args);

    static int restrictXYVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_restrictXY() = default;
    ~op_restrictXY() = default;

    std::shared_ptr<Operator> getOperator();

};


/*
1.3 The Pointcloud2 restrictZ Operator

*/
class op_restrictZ {
    static ListExpr restrictZTM(ListExpr args);

    static int restrictZVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_restrictZ() = default;
    ~op_restrictZ() = default;

    std::shared_ptr<Operator> getOperator();

};

/*
1.4 restrictAttr Operator

*/
class op_restrictAttr {
        static ListExpr restrictAttrTM( ListExpr args );
        static int restrictAttrVM( Word* args, Word& result, int message,
                        Word& local, Supplier s );

        std::string getOperatorSpec();

public:
        explicit op_restrictAttr() = default;
        ~op_restrictAttr() = default;

        std::shared_ptr<Operator> getOperator();
};

/*
1.3 The Pointcloud2 restrictRnd Operator

*/
class op_restrictRnd {
    static ListExpr restrictRndTM(ListExpr args);

    static int restrictRndVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_restrictRnd() = default;

    ~op_restrictRnd() = default;

    std::shared_ptr<Operator> getOperator();

};

} // end of namespace pointcloud2
