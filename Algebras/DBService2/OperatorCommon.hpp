/*

1.1 ~OperatorCommon~

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
#ifndef ALGEBRAS_DBSERVICE_OperatorCommon_HPP_
#define ALGEBRAS_DBSERVICE_OperatorCommon_HPP_

#include "Operator.h"

namespace DBService {

/*

1.1.1 Class Definition

*/

class OperatorCommon
{
public:

/*

1.1.1.1 ~getStreamType~

Returns the stream type  if applying the feed operator
to the relation specified in nextedList. The nestedList may
contains either a full description of the relation type 
or just a symbol specifying the relation's name. In the second
case, the type is retrieved from the DBService.

*/
    static ListExpr getStreamType(ListExpr nestedList, bool& locallyAvailable);
    
/*
1.1.1.1 ~getRelType~

If nestedList is a correct decription of a relation type, this type is 
returned. Otherwise nestedList has to be a symbol contains the relation's
name. In this case, the type is requested from the DBService.

*/
    static ListExpr getRelType(ListExpr nestedList, bool& locallyAvailable);


/*
1.1.1.1 ~getDerivedType~

This operator returns the type of the X-th argument stored in the ~args~ 
nested list. X must be greater than one. The first element in args
must be the relation from that the argument is derived. ~args~ must be 
formatted as a list of arguments each having format (type expr) as
created for type mappings if UsesArgsInTypeMapping is activated.

*/
    static ListExpr getDerivedType(ListExpr args, int X, bool & locallyAvailable);

/*
1.1.1.1 ~allExists~

This Operator checks whether the relation and all derived objects are
managed by the DBService at a single node.

*/
   static bool allExists( const std::string& dbName,
                          const std::string& relName,
                          const std::vector<std::string>& derivates);


};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_OPERATORREAD_HPP_ */
