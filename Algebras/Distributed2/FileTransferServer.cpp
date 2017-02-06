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

#include <fstream>

#include "FileTransferServer.h"
#include "FileTransferKeywords.h"
#include "SocketIO.h"
#include "StringUtils.h"

using namespace std;

namespace distributed2 {

FileTransferServer::FileTransferServer(int _port) :
        Server(_port) {
}

int FileTransferServer::start() {
    listener = Socket::CreateGlobal("localhost", stringutils::int2str(port));
    if (!listener->IsOk()) {
        return 1;
    }
    server = listener->Accept();
    if (!server->IsOk()) {
        return 2;
    }
    return communicate();
}

int FileTransferServer::communicate() {
    try {
        iostream& io = server->GetSocketStream();
        io << FileTransferKeywords::FileTransferServer() << endl;
        io.flush();
        string line;
        getline(io, line);
        if (line == FileTransferKeywords::Cancel()) {
            return true;
        }
        if (line != FileTransferKeywords::FileTransferClient()) {
            cerr << "Protocol error" << endl;
            return 3;
        }
        getline(io, line);
        if (line == FileTransferKeywords::SendFile()) {
            return sendFile(io);
        } else if (line == FileTransferKeywords::ReceiveFile()) {
            return receiveFile(io);
        } else {
            cerr << "protocol error" << endl;
            return 4;
        }
    } catch (...) {
        cerr << "Exception in server occured during communination" << endl;
        return 5;
    }
}

int FileTransferServer::sendFile(iostream& io) {
    // client ask for a file
    string filename;
    getline(io, filename);
    ifstream in(filename.c_str(), ios::binary);
    if (!in) {
        io << FileTransferKeywords::FileNotFound() << endl;
        io.flush();
        return 6;
    }
    in.seekg(0, in.end);
    size_t length = in.tellg();
    in.seekg(0, in.beg);
    io << FileTransferKeywords::Data() << endl;
    io << stringutils::any2str(length) << endl;
    io.flush();
    size_t bufsize = 1048576;
    char buffer[bufsize];
    while (!in.eof() && in.good()) {
        in.read(buffer, bufsize);
        size_t r = in.gcount();
        io.write(buffer, r);
    }
    in.close();
    io << FileTransferKeywords::EndData() << endl;
    io.flush();
    return 0;
}

bool FileTransferServer::receiveFile(iostream& io) {
    // not implemented yet
    return 7;
}


} /* namespace distributed2 */
