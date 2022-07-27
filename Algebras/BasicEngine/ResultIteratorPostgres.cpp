/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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

#include "ResultIteratorPostgres.h"

namespace BasicEngine {

/*
  1.1 Is a new tuple available in the iterator

*/
bool ResultIteratorPostgres::hasNextTuple() {
  return currentTuple < totalTuples;
}

/*
  1.2 Return the next tuple from the iterator

*/
Tuple *ResultIteratorPostgres::getNextTuple() {

  Tuple *result = basicTuple->Clone();

  for (size_t i = 0; i < attributeInstances.size(); i++) {

    Attribute *attr = attributeInstances[i]->Clone();

    if (PQgetisnull(res, currentTuple, i)) {
      attr->SetDefined(false);
    } else {
      char *value = PQgetvalue(res, currentTuple, i);
      attr->ReadFromString(value);
      attr->SetDefined(true);
    }

    result->PutAttribute(i, attr);
  }

  currentTuple++;

  return result;
}
}; // namespace BasicEngine
