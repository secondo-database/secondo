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
#include <ctime>
#include <sstream>

#include "Algebras/DBService/SecondoUtilsLocal.hpp"
#include "Algebras/DBService/TraceWriter.hpp"
#include "Algebras/DBService/DebugOutput.hpp"

using namespace std;

namespace DBService {

TraceWriter::TraceWriter(string& context)
{
    std::time_t currentTime = std::time(0);
    stringstream fileName;

    string host;
    SecondoUtilsLocal::readFromConfigFile(
            host,
            "Environment",
            "SecondoHost",
            "");

    string port;
    SecondoUtilsLocal::readFromConfigFile(
            port,
            "Environment",
            "SecondoPort",
            "");

    fileName << context << "_"
             << host << "_"
             << port << "_"
             << currentTime << ".trc";

    traceFile= unique_ptr<ofstream>
    (new ofstream(fileName.str().c_str(),std::ios::binary));
}

TraceWriter::~TraceWriter()
{
    if(*traceFile)
    {
        traceFile->close();
    }
}

void TraceWriter::write(const string& text)
{
    print(text);
    *traceFile << text << endl;
}

void TraceWriter::write(const char* text)
{
    print(text);
    *traceFile << text << endl;
}

void TraceWriter::write(const size_t text)
{
    print(text);
    *traceFile << text << endl;
}

void TraceWriter::write(const LocationInfo& location)
{
    print(location);
    *traceFile << location.getHost() << endl;
    *traceFile << location.getPort() << endl;
    *traceFile << location.getDisk() << endl;
    *traceFile << location.getCommPort() << endl;
    *traceFile << location.getTransferPort() << endl;
}

void TraceWriter::write(const RelationInfo& relationInfo)
{
    *traceFile << "RelationInfo:" << endl;
    *traceFile << relationInfo.getDatabaseName() << endl;
    *traceFile << relationInfo.getRelationName() << endl;
    *traceFile << relationInfo.getOriginalLocation().getHost() << endl;
    *traceFile << relationInfo.getOriginalLocation().getPort() << endl;
    *traceFile << relationInfo.getOriginalLocation().getDisk() << endl;
}

void TraceWriter::write(const char* description, const string& text)
{
    print(description, text);
    *traceFile << description << ": " << text << endl;
}

void TraceWriter::write(const char* description, int number)
{
    print(description, number);
    *traceFile << description << ": " << number << endl;
}

void TraceWriter::writeFunction(const char* text)
{
    printFunction(text);
    *traceFile << "********************************" << endl;
    *traceFile << text << endl;
}

} /* namespace DBService */
