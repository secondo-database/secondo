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



1 Pc2RasterTest Operators Header

*/

#pragma once

#include "tcPointcloud2.h"

namespace pointcloud2 {

/*
1.1 Pc2RasterTest

*/

class op_Pc2RasterTest{
    size_t _rasterSize = 0;
    bool _even = false;

    static ListExpr Pc2RasterTestTM(ListExpr args);

    static int Pc2RasterTestVMT( Word* args, Word& result, int message,
      Word& local, Supplier s );

    std::string getOperatorSpec();

    void evenRaster(Pointcloud2 *res) const;

    void randomRaster(Pointcloud2 *res) const;

public:
    explicit op_Pc2RasterTest() = default;

    op_Pc2RasterTest(size_t rasterSize, bool even) :
        _rasterSize(rasterSize), _even(even){}

    ~op_Pc2RasterTest() = default;

    std::shared_ptr<Operator> getOperator();
};

} //end of namespace pointcloud2
