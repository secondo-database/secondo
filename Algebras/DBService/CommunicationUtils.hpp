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
#ifndef ALGEBRAS_DBSERVICE_COMMUNICATIONUTILS_HPP_
#define ALGEBRAS_DBSERVICE_COMMUNICATIONUTILS_HPP_

#include <queue>

namespace DBService {

class CommunicationUtils
{
public:
    static bool receivedExpectedLine(std::iostream& io,
            const std::string& expectedLine);
    static bool receivedExpectedLines(std::iostream& io,
            std::queue<std::string>& expectedLines);
    static void receiveLine(std::iostream& io,
            std::string& line);
    static void receiveLines(std::iostream& io,
            const size_t count,
            std::queue<std::string>& lines);
    static void sendLine(std::iostream& io,
            const std::string& line);
    static void sendBatch(std::iostream& io,
            std::queue<std::string>& lines);
protected:
    static bool streamStatusOk(std::iostream& io);
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_COMMUNICATIONUTILS_HPP_ */
