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



1 The Pointcloud2 projectUTM Operator

projectUTM: pointcloud2(R,tuple(Z1...Zn))
-> pointcloud2(R,tuple(Z1...Zn,ObjID:int,CatID:int))

*/

#pragma once
#include <memory>

#include "Operator.h"


namespace pointcloud2{

class op_projectUTM{

    static ListExpr projectUTMTM(ListExpr args);

    static int projectUTMVM( Word* args, Word& result, int message,
            Word& local, Supplier s );

    std::string getOperatorSpec();

public:
    explicit op_projectUTM() = default;
    ~op_projectUTM() = default;

    std::shared_ptr<Operator> getOperator();
};

class op_projectWGS84{

    static ListExpr projectWGS84TM(ListExpr args);

    static int projectWGS84VM( Word* args, Word& result, int message,
            Word& local, Supplier s );

    std::string getOperatorSpec();

public:
    explicit op_projectWGS84() = default;
    ~op_projectWGS84() = default;

    std::shared_ptr<Operator> getOperator();
};

class op_UTMZone{

    static ListExpr UTMZoneTM(ListExpr args);

    static int UTMZoneVM( Word* args, Word& result, int message,
            Word& local, Supplier s );

    std::string getOperatorSpec();

public:
    explicit op_UTMZone() = default;
    ~op_UTMZone() = default;

    std::shared_ptr<Operator> getOperator();
};

class op_UTMSouth{

    static ListExpr UTMSouthTM(ListExpr args);

    static int UTMSouthVM( Word* args, Word& result, int message,
            Word& local, Supplier s );

    std::string getOperatorSpec();

public:
    explicit op_UTMSouth() = default;
    ~op_UTMSouth() = default;

    std::shared_ptr<Operator> getOperator();
};

} //end of namespace
