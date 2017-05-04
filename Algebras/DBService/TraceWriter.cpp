/*
----
This file is part of SECONDO.

Copyright (C) 2016,
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
#include <ctime>
#include <sstream>

#include "TraceWriter.hpp"

using namespace std;

namespace DBService {

TraceWriter::TraceWriter(string& context)
{
    std::time_t currentTime = std::time(0);
    stringstream fileName;
    fileName << context << "_" << currentTime << ".trc";
    traceFile= auto_ptr<ofstream>
    (new ofstream(fileName.str().c_str(),std::ios::binary));
}

void TraceWriter::write(const string& text)
{
    *traceFile << text << endl;
}

void TraceWriter::write(const char* text)
{
    *traceFile << text << endl;
}

void TraceWriter::write(const size_t text)
{
    *traceFile << text << endl;
}

void TraceWriter::write(const LocationInfo& location)
{
    *traceFile << location.getHost() << endl;
    *traceFile << location.getPort() << endl;
    *traceFile << location.getDisk() << endl;
    *traceFile << location.getCommPort() << endl;
    *traceFile << location.getTransferPort() << endl;
}

TraceWriter::~TraceWriter()
{
if(*traceFile)
{
    traceFile->close();
}

}


} /* namespace DBService */
