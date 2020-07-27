/*

1.1 ~OperatorDBIARG~

This operator retrieves the type of the x-th argument in the argumentlist.
If this argument represents a valid Secondo type, just this type is
returned. Otherwise the type is requested from the DBService. To do so,
also the symbol of the first argument, i.e. the relation, is used.


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
#ifndef ALGEBRAS_DBSERVICE_OperatorDBIARG_HPP_
#define ALGEBRAS_DBSERVICE_OperatorDBIARG_HPP_

#include "Operator.h"
#include "Algebras/DBService/OperatorCommon.hpp"
#include "StringUtils.h"

namespace DBService {

/*

1.1.1 Operator Specification

*/
template<int X>
struct DBIARGInfo: OperatorInfo
{
    DBIARGInfo()
    {
        std::string x = stringutils::int2str(X);
        name = "DBIARG" + x;
        signature = "(rel(tuple(X)))) x t_2 x ... x t_"
                    + x +"  ... x t_n  -> t_"+x;
        syntax =  "DBIARG"+x+"(_,_,...)";
        meaning = "determine the type of the argument at position "+x+ 
                  " by either retrieving "
                  "it from the local type or from the DBService";
        example = "only for usage in operator signature";
        remark  = "requires a DBService system";
        usesArgsInTypeMapping = true;
    }
};

/*

1.1.1 Class Definition

*/

template<int X>
class OperatorDBIARG
{
public:

/*

1.1.1.1 Type Mapping Function

*/
    static ListExpr mapType(ListExpr args) {
       bool locallyAvailable;
       return OperatorCommon::getDerivedType(args, X, locallyAvailable);
    }
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_OPERATORDBIARG_HPP */
