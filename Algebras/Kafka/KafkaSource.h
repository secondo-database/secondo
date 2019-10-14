/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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


#ifndef KAFKA_KAFKASOURCE_H
#define KAFKA_KAFKASOURCE_H

#include "Attribute.h" // implementation of attribute types
#include "Algebra.h" // definition of the algebra
#include "NestedList.h" // required at many places
#include "QueryProcessor.h" // needed for implementing value mappings
#include "AlgebraManager.h" // e.g., check for a certain kind
#include "Operator.h" // for operator creation
#include "StandardTypes.h" // priovides int, real, string, bool type
#include "Algebras/FText/FTextAlgebra.h"
#include "Symbols.h" // predefined strings
#include "ListUtils.h" // useful functions for nested lists
#include "Stream.h" // wrapper for secondo streams

#include "GenericTC.h" // use of generic type constructors

#include "LogMsg.h" // send error messages

#include "Tools/Flob/DbArray.h" // use of DbArrays

#include "Algebras/Relation-C++/RelationAlgebra.h" // use of tuples

#include "FileSystem.h" // deletion of files

#include <math.h> // required for some operators
#include <stack>
#include <limits>

namespace kafka {

    extern Operator kafkaSourceOp;

    std::string readTypeString(std::string broker, std::string topic);

    ListExpr validateTopicArg(ListExpr arg);

    ListExpr validateBrokerArg(ListExpr arg);

    ListExpr validateBooleanArg(ListExpr arg);

}


#endif //KAFKA_KAFKASOURCE_H
