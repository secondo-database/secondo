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

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

#include "ConnectionGeneric.h"

using namespace std;

namespace BasicEngine {

string ConnectionGeneric::limitSQLQuery(const std::string &query) {

    string resultQuery = string(query);

    string sqlQueryUpper = boost::to_upper_copy<std::string>(query);

    // Only select queries can be restrircted
    if(! boost::algorithm::starts_with(sqlQueryUpper, "SELECT")) {
        return resultQuery;    
    }

    // Is query already restricted?
    if(sqlQueryUpper.find("LIMIT") == std::string::npos) {
        
        // Remove existing ";" at the query end, if exists
        if(boost::algorithm::ends_with(resultQuery, ";")) {
            resultQuery.pop_back();
        }

        // Limit query to 1 result tuple
        resultQuery.append(" LIMIT 1;");
    }
    
    return resultQuery;
}

}