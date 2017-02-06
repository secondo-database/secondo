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

#include <iosfwd>
#include <fstream>

#include "FileTransferClient.h"
#include "FileTransferKeywords.h"
#include "SocketIO.h"
#include "StringUtils.h"

using namespace std;

namespace distributed2 {

FileTransferClient::FileTransferClient(string& _server, int _port,
        bool _receive, string& _localName, string& _remoteName) :
        Client(_server, _port, 0),
        receive(_receive), localName(_localName), remoteName(_remoteName) {
}

int FileTransferClient::start() {
    socket = Socket::Connect(server, stringutils::int2str(port),
            Socket::SockGlobalDomain, 3, 1);
    if (!socket) {
        return 8;
    }
    if (!socket->IsOk()) {
        return 9;
    }
    if (receive) {
        return receiveFile();
    } else {
        return sendFile();
    }
}

int FileTransferClient::sendFile() {
    return 10; // not implemented yet
}

int FileTransferClient::receiveFile() {
    iostream& io = socket->GetSocketStream();
    string line;
    getline(io, line);
    if (line != FileTransferKeywords::FileTransferServer()) {
        return 11;
    }
    io << FileTransferKeywords::FileTransferClient() << endl;
    io << FileTransferKeywords::SendFile() << endl;
    io << remoteName << endl;
    io.flush();
    getline(io, line);
    if (line == FileTransferKeywords::FileNotFound()) {
        return 12;
    }
    if (line != FileTransferKeywords::Data()) {
        return 13;
    }
    getline(io, line);
    bool ok;
    size_t t = stringutils::str2int<size_t>(line, ok);

    if (!ok || t < 1) {
        return 14;
    }

    size_t bufsize = 4096;
    char buffer[bufsize];

    // read in data
    ofstream out(localName.c_str(), ios::binary | ios::trunc);
    while (t > 0) {
        size_t s = min(t, bufsize);
        if (!io.read(buffer, s)) {
            return 15;
        }
        t -= s;
        out.write(buffer, s);
    }
    out.close();
    getline(io, line);
    if (line != FileTransferKeywords::EndData()) {
        return 16;
    }
    return 0;
}

}
