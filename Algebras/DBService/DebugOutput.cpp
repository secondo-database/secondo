/*

1.1.1 Function Implementations

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
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/TraceSettings.hpp"

namespace DBService
{

using namespace std;

void print(string& text)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << text << endl;
    }
}

void printFunction(const char* text)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << "********************************" << endl;
        cout << text << endl;
    }
}

void printFunction(boost::thread::id tid, const char* text)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << "********************************" << endl;
        cout << "[Thread " << tid << "] " << text << endl;
    }
}

void print(const string& text)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << text << endl;
    }
}

void print(const char* text)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << text << endl;
    }
}

void print(ListExpr nestedList)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << "length: " << nl->ListLength(nestedList) << endl;
        cout << nl->ToString(nestedList) << endl;
    }
}

void print(int number)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << number << endl;
    }
}

void print(const char* text, int number)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << text << endl;
        cout << number << endl;
    }
}

void print(const char* text, ListExpr nestedList)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << text << endl;
        cout << "length: " << nl->ListLength(nestedList) << endl;
        cout << nl->ToString(nestedList) << endl;
    }
}

void print(const char* text1, string& text2)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << text1 <<  ": " << text2 << endl;
    }
}

void print(const char* text1, const string& text2)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << text1 <<  ": " << text2 << endl;
    }
}

void print(boost::thread::id tid, const char* text1, const string& text2)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << "[Thread " << tid << "] " << text1 << ": " << text2 << endl;
    }
}

void print(const string& text1, const char* text2)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << text1 << endl;
        cout << text2 << endl;
    }
}

void print(const LocationInfo& locationInfo)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << "LocationInfo:" << endl;
        printLocationInfo(locationInfo);
    }
}

void printLocationInfo(const LocationInfo& locationInfo)
{
    cout << "Host:\t\t" << locationInfo.getHost() << endl;
    cout << "Port:\t\t" << locationInfo.getPort() << endl;
    cout << "Disk:\t\t" << locationInfo.getDisk() << endl;
    cout << "CommPort:\t" << locationInfo.getCommPort() << endl;
    cout << "TransferPort:\t" << locationInfo.getTransferPort() << endl;
}

void print(const RelationInfo& relationInfo)
{
    if(TraceSettings::getInstance()->isDebugTraceOn())
    {
        cout << "RelationInfo:" << endl;
        printRelationInfo(relationInfo);
    }
}

void printRelationInfo(const RelationInfo& relationInfo)
{
    cout << "DatabaseName:\t" << relationInfo.getDatabaseName() << endl;
    cout << "RelationName:\t" << relationInfo.getRelationName() << endl;
    cout << "Host:\t\t" << relationInfo.getOriginalLocation().getHost() << endl;
    cout << "Port:\t\t" << relationInfo.getOriginalLocation().getPort() << endl;
    cout << "Disk:\t\t" << relationInfo.getOriginalLocation().getDisk() << endl;
    for(ReplicaLocations::const_iterator it
            = relationInfo.nodesBegin(); it != relationInfo.nodesEnd(); it++)
    {
        cout << "Node:\t\t" << it->first << " (Replicated: " <<
                (it->second ? "TRUE" : "FALSE") << ")"
                << endl;
    }
}

}
