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

#include <iostream>
#include <string>

#include "CommunicationUtils.hpp"

using namespace std;

namespace DBService
{

bool CommunicationUtils::streamStatusOk(iostream& io)
{
    if(!io.good())
    {
        std::cout << " eof()=" << io.eof();
        std::cout << " fail()=" << io.fail();
        std::cout << " bad()=" << io.bad();
        return false;
    }
    return true;
}


bool CommunicationUtils::receivedExpectedLine(iostream& io,
                                              const string& expectedLine)
{
    if(!streamStatusOk(io))
    {
        return false;
    }
    string line;
    getline(io, line);
    if (line != expectedLine)
    {
        return false;
    }
    return true;
}

bool CommunicationUtils::receivedExpectedLines(iostream& io,
                                               queue<string>& expectedLines)
{
    if(!streamStatusOk(io))
    {
        return false;
    }
    string line;
    while(!expectedLines.empty())
    {
        getline(io, line);
        if (line != expectedLines.front())
        {
            return false;
        }
        expectedLines.pop();
    }
    return true;
}

void CommunicationUtils::receiveLine(iostream& io,
        string& line)
{
    if(streamStatusOk(io))
    {
        getline(io, line);
    }
}

void CommunicationUtils::receiveLines(iostream& io,
        const size_t count,
        queue<string>& lines)
{
    if(streamStatusOk(io))
    {
        string line;
        for(size_t i = 0; i < count; i++)
        {
            getline(io, line);
            lines.push(line);
        }
    }
}

void CommunicationUtils::sendLine(iostream& io,
                                  const std::string& line)
{
    if(streamStatusOk(io))
    {
        io << line << endl;
        io.flush();
    }
}

void CommunicationUtils::sendBatch(iostream& io,
                                   queue<string>& lines)
{
    if(streamStatusOk(io))
    {
        while(!lines.empty())
        {
            io << lines.front() << endl;
            lines.pop();
        }
        io.flush();
    }
}

} /* namespace DBService */
