/*
----
This file is part of SECONDO.

Copyright (C) 2017,
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

*/
#ifndef ALGEBRAS_DBSERVICE_OPERATORREAD_HPP_
#define ALGEBRAS_DBSERVICE_OPERATORREAD_HPP_

#include "Operator.h"

namespace DBService {

/*

1 \textit{}

\textit{DBService}
TODO

*/

struct ReadInfo: OperatorInfo
{
    ReadInfo()
    {
        name = "read";
        signature = ""; // TODO
        syntax = ""; // TODO
        meaning = "execute a certain query and fall back to "
                  "replica provided by DBService if necessary";
        usesArgsInTypeMapping = true;
    }
};

class OperatorRead
{
public:
    static ListExpr mapType(ListExpr nestedList);
    static int mapValue(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s);
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_OPERATORREAD_HPP_ */
