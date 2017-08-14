/*

1.1 ~OperatorCheckDBServiceStatus~

This operator allows to check the status of the ~DBService~. It checks and
prints the stored metadata and reconnects the ~DBService~ workers if applicable.

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
#ifndef ALGEBRAS_DBSERVICE_OperatorCheckDBServiceStatus_HPP_
#define ALGEBRAS_DBSERVICE_OperatorCheckDBServiceStatus_HPP_

#include "Operator.h"

namespace DBService
{

/*

1.1.1 Operator Specification

*/

struct CheckDBServiceStatusInfo: OperatorInfo
{
    CheckDBServiceStatusInfo()
    {
        name = "checkdbservicestatus";
        signature = "-> bool";
        syntax = "checkdbservicestatus()";
        meaning = "check status of DBService, reconnect worker nodes and "
                "print currently stored metadata to the command line";
        example = "query checkdbservicestatus()";
        remark = "needs to be executed on a DBService system";
        usesArgsInTypeMapping = false;
    }
};

/*

1.1.1 Class Definition

*/

class OperatorCheckDBServiceStatus
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

#endif /* ALGEBRAS_DBSERVICE_OperatorCheckDBServiceStatus_HPP_ */
