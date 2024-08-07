/*

1.1.1 Class Implementation

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
#include "NestedList.h"
#include "ListUtils.h"
#include "Algebras/Relation-C++/RelationAlgebra.h"
#include "OperatorRELARG.hpp"

#include <loguru.hpp>

using namespace std;

extern boost::recursive_mutex nlparsemtx;

namespace DBService {

ListExpr OperatorRELARG::mapType(ListExpr args)
{

  LOG_F(INFO, "%s", "Acquiring lock for nlparsemtx...");
  boost::lock_guard<boost::recursive_mutex> guard(nlparsemtx);
  LOG_F(INFO, "%s", "Successfully acquired lock for nlparsemtx...");

  if(!nl->HasMinLength(args,1)){
    return listutils::typeError("too less arguments");
  }
  if(!Relation::checkType(nl->First(args))){
    return listutils::typeError("first argument is not a relation");
  }
  return nl->First(args);
}

} /* namespace DBService */
