/*

1.1 ~OperatorDBRARG~

This operator takes a relation name and checks whether there is a relation with
this name in the local SECONDO system. Otherwise, it connects to the DBService
and retrieves the type of the relation from there in case the relation exists.

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
#ifndef ALGEBRAS_DBSERVICE_OperatorDBRARG_HPP_
#define ALGEBRAS_DBSERVICE_OperatorDBRARG_HPP_

#include "Operator.h"

namespace DBService {

/*

1.1.1 Operator Specification

*/

struct DBRARGInfo: OperatorInfo
{
    DBRARGInfo()
    {
        name = "DBRARG";
        signature = "(rel(tuple(X)))) -> (stream(tuple(X)))";
        syntax = "DBRARG(_)";
        meaning = "determine the type of a relation by either retrieving "
                "it from the local relation or from the DBService";
        example = "only for usage in operator signature";
        remark = "requires a DBService system";
        usesArgsInTypeMapping = false;
    }
};

/*

1.1.1 Class Definition

*/

class OperatorDBRARG
{
public:

/*

1.1.1.1 Type Mapping Function

*/
    static ListExpr mapType(ListExpr nestedList);
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_OPERATORREAD_HPP_ */
