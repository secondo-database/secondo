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
#include "Operator.h"
#include "../tcPointcloud2.h"

namespace pointcloud2 {

/*
1.1 The Pointcloud2 Bbox Operator

*/
class op_bbox {
    static ListExpr bboxTM(ListExpr args);

    static int bboxVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();
public:
    explicit op_bbox() = default;
    ~op_bbox() = default;

    std::shared_ptr<Operator> getOperator();

};

/*
1.2 The Pointcloud2 Bbox2D Operator

*/
class op_bbox2d {

    static ListExpr bbox2dTM(ListExpr args);

    static int bbox2dVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_bbox2d() = default;
    ~op_bbox2d() = default;

    std::shared_ptr<Operator> getOperator();

};


/*
1.3  The Pointcloud2 maxZ Operator

*/
class op_maxz {

    static ListExpr maxzTM(ListExpr args);

    static int maxzVM(Word* args, Word& result, int message,
                Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_maxz () = default;
    ~op_maxz () = default;

    std::shared_ptr<Operator> getOperator();


};

/*
1.4 The Pointcloud2 minZ Operator

*/
class op_minz {

    static ListExpr minzTM(ListExpr args);

    static int minzVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_minz() = default;
    ~op_minz() = default;

    std::shared_ptr<Operator> getOperator();


};

/*
1.5 The Pointcloud2 Size Operator

*/
class op_size {

    static ListExpr sizeTM(ListExpr args);

    static int sizeVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_size() = default;
    ~op_size() = default;

    std::shared_ptr<Operator> getOperator();


};

} // end of namespace pointcloud2
