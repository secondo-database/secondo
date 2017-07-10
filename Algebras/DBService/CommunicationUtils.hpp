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

*/
#ifndef ALGEBRAS_DBSERVICE_COMMUNICATIONUTILS_HPP_
#define ALGEBRAS_DBSERVICE_COMMUNICATIONUTILS_HPP_

#include <queue>

namespace DBService {

/*

1 \textit{CommunicationUtils}

\textit{DBService}
The \textit{CommunicationUtils} provide functions to send and receive messages
using an \textit{std::iostream}.

*/

class CommunicationUtils
{
public:
/*

1.1 \textit{receivedExpectedLine}

This function checks whether an expected line was received on a specified
stream.

*/
    static bool receivedExpectedLine(std::iostream& io,
            const std::string& expectedLine);

/*

1.1 \textit{receivedExpectedLines}

This function checks whether the expected lines were received on a specified
stream.

*/
    static bool receivedExpectedLines(std::iostream& io,
            std::queue<std::string>& expectedLines);

/*

1.1 \textit{receiveLine}

This function stores a received line in the specified string object.

*/
    static void receiveLine(std::iostream& io,
            std::string& line);

/*

1.1 \textit{receiveLines}

This function stores the specified numbers of received lines in the specified
queue of string objects.

*/
    static void receiveLines(std::iostream& io,
            const size_t count,
            std::queue<std::string>& lines);

/*

1.1 \textit{sendLine}

This function writes the specified message into the stream.

*/
    static void sendLine(std::iostream& io,
            const std::string& line);

/*

1.1 \textit{sendBatch}

This function writes the specified queue of messages into the stream.

*/
    static void sendBatch(std::iostream& io,
            std::queue<std::string>& lines);

/*

1.1 \textit{streamStatusOk}

This function checks whether the stream is in any kind of error state which
would lead to crashes if we use it for sending or receiving messages.

*/
protected:
    static bool streamStatusOk(std::iostream& io);
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_COMMUNICATIONUTILS_HPP_ */
