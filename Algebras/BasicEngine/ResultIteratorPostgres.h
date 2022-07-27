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
#ifndef _RESULT_ITERATOR_POSTGRES_H_
#define _RESULT_ITERATOR_POSTGRES_H_

#include "ResultIteratorGeneric.h"

#include <postgres.h>
#include <catalog/pg_type.h>
#include <libpq-fe.h>

namespace BasicEngine {
class ResultIteratorPostgres : public ResultIteratorGeneric {
public:
  ResultIteratorPostgres(PGresult *res, ListExpr &type)
      : ResultIteratorGeneric(type), res(res) {

    if (ready) {
      totalTuples = PQntuples(res);
    } else {
      totalTuples = 0;
    }
  }

  virtual ~ResultIteratorPostgres() {
    if (res != nullptr) {
      PQclear(res);
      res = nullptr;
    }
  }

  virtual bool hasNextTuple();
  virtual Tuple *getNextTuple();

private:
  PGresult *res = nullptr;

  int currentTuple = 0;
  int totalTuples = 0;
};
} // namespace BasicEngine

#endif