/*
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


//[$][\$]
//[_][\_]

*/
#ifndef ALGEBRAS_DBSERVICE_TRACEWRITER_HPP_
#define ALGEBRAS_DBSERVICE_TRACEWRITER_HPP_

#include <fstream>
#include <memory>
#include <string>

#include "Algebras/DBService/LocationInfo.hpp"
#include "Algebras/DBService/RelationInfo.hpp"

namespace DBService {

/*

1 \textit{}

\textit{DBService}
TODO

*/

class TraceWriter {
public:
    TraceWriter(std::string& context);
    ~TraceWriter();
    void write(const std::string& text);
    void write(const char* text);
    void write(const size_t text);
    void write(const LocationInfo& location);
    void write(const RelationInfo& relationInfo);
    void write(const char* description, const std::string& text);
    void write(const char* description, int number);
    void writeFunction(const char* text);
private:
    std::string fileName;
    std::unique_ptr<std::ofstream> traceFile;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_TRACEWRITER_HPP_ */
