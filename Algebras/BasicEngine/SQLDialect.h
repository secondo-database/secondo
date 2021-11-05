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

#ifndef BASIC_ENGINE_SQL_DIALECT
#define BASIC_ENGINE_SQL_DIALECT

namespace BasicEngine {

class SQLDialect {
    public:

    virtual ~SQLDialect() {}

    virtual std::string getDropTableSQL(const std::string &table) {
        return "DROP TABLE IF EXISTS " + table + ";";
    }

    virtual std::string getCreateTableFromPredicateSQL(const std::string &table,
                                                const std::string &query) {

        return "CREATE TABLE " + table + " AS (" + query + ")";
    }

    virtual std::string getCopySchemaSQL(const std::string &table) {
        return "SELECT * FROM " + table + " LIMIT 0";
    }

    virtual std::string getRenameTableSQL(const std::string &source,
                                    const std::string &destination) {

        return "ALTER TABLE " + source + " RENAME TO " + destination + ";";
    }

    virtual std::string getBeginTransactionSQL() {
        return "START TRANSACTION;";
    }

    virtual std::string getAbortTransactionSQL() {
        return "ROLLBACK;";
    }

    virtual std::string getCommitTransactionSQL() {
        return "COMMIT;";
    }

    virtual std::string getRemoveColumnFromTableSQL(const std::string &table,
                    const std::string &name) {
        return "ALTER TABLE " + table + " DROP COLUMN " + name;
    }

    // Abstract SQL queries
    virtual std::string getDropIndexSQL(const std::string &table,
                                const std::string &column) = 0;

};

}

#endif