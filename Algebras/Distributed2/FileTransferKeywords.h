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
#ifndef ALGEBRAS_DISTRIBUTED2_FILETRANSFERKEYWORDS_H_
#define ALGEBRAS_DISTRIBUTED2_FILETRANSFERKEYWORDS_H_

#include <string>

/*
4. File Transfer between workers

The following code implements the file transfer between different workers
without using the master. For that purpose, an operator is provided which
creates a server and waits for the request of a client. If a client is
connected, the client may either send a file to this server or request a
file from the server. The client itself is also created by an operator.

In the first implementation, the server allows only a single client to
connect. After transferring a single file, the connection is terminated.

If the overhead for creating a server is big, this will be changed in the
future.

*/

namespace distributed2 {

class FileTransferKeywords {
public:
    static std::string FileTransferServer();
    static std::string FileTransferClient();
    static std::string SendFile();
    static std::string ReceiveFile();
    static std::string Data();
    static std::string EndData();
    static std::string FileNotFound();
    static std::string Cancel();
    static std::string OK();

    static bool isBool(std::string s);
    static bool getBool(std::string s);
};

}

#endif /* ALGEBRAS_DISTRIBUTED2_FILETRANSFERKEYWORDS_H_ */
