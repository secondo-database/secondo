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

#ifndef BASIC_ENGINE_POSTGRES_SQL_DIALECT
#define BASIC_ENGINE_POSTGRES_SQL_DIALECT

#include "SQLDialect.h"

namespace BasicEngine {

class SQLDialectPostgres : public SQLDialect {

public:
  std::string getDropIndexSQL(const std::string &table,
                              const std::string &column) {

    return "DROP INDEX IF EXISTS " + table + "_idx;";
  }

  std::string getExportDataForPartitionSQL(const std::string &table,
                                           const std::string &exportFile,
                                           size_t partition) {

    return "COPY (SELECT * FROM " + table + " WHERE (" + be_partition_slot +
           " = " + std::to_string(partition) +
           ") TO "
           "'" +
           exportFile + "' BINARY;";
  }
};

} // namespace BasicEngine

#endif