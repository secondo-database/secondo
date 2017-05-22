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
#include "Algebras/DBService/DebugOutput.hpp"

namespace DBService
{

using namespace std;

void print(string& text)
{
        cout << text << endl;
}

void printFunction(const char* text)
{
        cout << "********************************" << endl;
        cout << text << endl;
}

void print(const string& text)
{
        cout << text << endl;
}

void print(const char* text)
{
        cout << text << endl;
}

void print(ListExpr nestedList)
{
        cout << "length: " << nl->ListLength(nestedList) << endl;
        cout << nl->ToString(nestedList) << endl;
}

void print(int number)
{
        cout << number << endl;
}

void print(const char* text, int number)
{
        cout << text << endl;
        cout << number << endl;
}

void print(const char* text, ListExpr nestedList)
{
        cout << text << endl;
        cout << "length: " << nl->ListLength(nestedList) << endl;
        cout << nl->ToString(nestedList) << endl;
}

void print(const char* text1, string& text2)
{
        cout << text1 << endl;
        cout << text2 << endl;
}

void print(const char* text1, const string& text2)
{
        cout << text1 << endl;
        cout << text2 << endl;
}

void print(const std::string& text1, const char* text2)
{
    cout << text1 << endl;
    cout << text2 << endl;
}

void print(const LocationInfo& locationInfo)
{
    cout << "LocationInfo:" << endl;
    cout << locationInfo.getHost() << endl;
    cout << locationInfo.getPort() << endl;
    cout << locationInfo.getDisk() << endl;
    cout << locationInfo.getCommPort() << endl;
    cout << locationInfo.getTransferPort() << endl;
}

void print(const RelationInfo& relationInfo)
{
    cout << "RelationInfo:" << endl;
    cout << relationInfo.getDatabaseName() << endl;
    cout << relationInfo.getRelationName() << endl;
    cout << relationInfo.getOriginalLocation().getHost() << endl;
    cout << relationInfo.getOriginalLocation().getPort() << endl;
    cout << relationInfo.getOriginalLocation().getDisk() << endl;
}

}
