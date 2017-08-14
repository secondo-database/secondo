/*

1 Operators

1.1 ~OperatorAddNode~

The operator ~addnode~ allows adding a worker node to the ~DBService~ system.

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
#ifndef ALGEBRAS_DBSERVICE_OPERATORADDNODE_HPP_
#define ALGEBRAS_DBSERVICE_OPERATORADDNODE_HPP_

#include "Operator.h"

namespace DBService
{

/*

1.1.1 Operator Specification

*/

struct AddNodeInfo: OperatorInfo
{
    AddNodeInfo()
    {
        name = "addnode";
        signature = "string x int x string -> bool";
        syntax = "addnode(string, int, string)";
        meaning = "add a worker node to the DBService system";
        example = "query addnode('132.176.69.181', 9989, '/secondo/config')";
        remark = "requres a remote server";
        usesArgsInTypeMapping = false;
    }
};

/*

1.1.1 Class Definition

*/

class OperatorAddNode
{
public:

/*

1.1.1.1 Type Mapping Function

*/
    static ListExpr mapType(ListExpr nestedList);

/*

1.1.1.1 Value Mapping Function

*/
    static int mapValue(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s);
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_OPERATORADDNODE_HPP_ */
