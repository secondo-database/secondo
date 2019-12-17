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
#include "../MThreadedAlgebra.h"

namespace mthreaded  {

/*
1.1 The MThreaded MaxCore Operator

*/
class op_maxcore {
    static ListExpr maxcoreTM(ListExpr args);

    static int maxcoreVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();
public:
    explicit op_maxcore() = default;
    ~op_maxcore() = default;

    std::shared_ptr<Operator> getOperator();

};

/*
1.2 The MThreaded SetCore  Operator

*/
class op_setcore {

    static ListExpr setcoreTM(ListExpr args);

    static int setcoreVM(Word* args, Word& result, int message,
            Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_setcore() = default;
    ~op_setcore() = default;

    std::shared_ptr<Operator> getOperator();

};


/*
1.3  The MThreaded GetCore Operator

*/
class op_getcore {

    static ListExpr getcoreTM(ListExpr args);

    static int getcoreVM(Word* args, Word& result, int message,
                Word& local, Supplier s);

    std::string getOperatorSpec();

public:
    explicit op_getcore () = default;
    ~op_getcore () = default;

    std::shared_ptr<Operator> getOperator();


};


} // end of namespace mthreaded
