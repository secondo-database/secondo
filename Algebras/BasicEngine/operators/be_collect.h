/*
----
This file is part of SECONDO.

Copyright (C) 2022,
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

//[$][\$]

*/

#ifndef BE_COLLECT_H
#define BE_COLLECT_H

#include "StandardTypes.h"

namespace BasicEngine {

bool EvaluateTypeMappingExpr(std::string expression, std::string &result);

ListExpr be_collect_tm(ListExpr args);

int be_collect_vm(Word* args, Word& result, int message,
        Word& local, Supplier s);

/*
1.1.3 Specification

*/
OperatorSpec be_collect_spec (
   "(text) --> stream(tuple(...))",
   "be_collect(_)",
   "Fetches the data from the used database into SECONDO",
   "query be_collect('select * from cars') collect"
);

/*
1.1.6 Definition of operator ~be\_collect~

*/
Operator be_collect_op (
         "be_collect",              // name
         be_collect_spec.getStr(),  // specification
         be_collect_vm,             // value mapping
         Operator::SimpleSelect,    // trivial selection function
         be_collect_tm             // type mapping
);


} // namespace BasicEngine

#endif