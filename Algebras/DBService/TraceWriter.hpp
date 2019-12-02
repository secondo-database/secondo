/*

1.1 ~TraceWriter~

The ~TraceWriter~ allows writing information into a trace file.

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
#ifndef ALGEBRAS_DBSERVICE_TRACEWRITER_HPP_
#define ALGEBRAS_DBSERVICE_TRACEWRITER_HPP_

#include <fstream>
#include <memory>
#include <string>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "NestedList.h"

#include "Algebras/DBService/LocationInfo.hpp"
#include "Algebras/DBService/RelationInfo.hpp"
#include "Algebras/DBService/DebugOutput.hpp"
#include "Algebras/DBService/TraceSettings.hpp"

namespace DBService {

/*

1.1.1 Class Definition

*/

class TraceWriter {
public:

/*

1.1.1.1 Constructor

*/
    TraceWriter(std::string& context, int port, std::ostream& out);

/*

1.1.1.1 Destructor

*/
    ~TraceWriter();

/*

1.1.1.1 Write Functions

These functions allow writing information into the tracefile. They are available
for all necessary data types.

*/
    void write(const std::string& text);
    void write(const char* text);
    void write(const boost::thread::id tid, const char* text);
    void write(const size_t text);
    void write(const LocationInfo& location);
    void write(const RelationInfo& relationInfo);
    void write(const char* description, const std::string& text);
    void write(
            const boost::thread::id tid,
            const char* description,
            const std::string& text);
    void write(const char* description, int number);
    void writeFunction(const char* text);
    void writeFunction(const boost::thread::id tid, const char* text);
    void write(
            const boost::thread::id tid,
            const char* text,
            ListExpr nestedList);
    void write(
            const boost::thread::id tid,
            const char* text,
            bool value);

    template<class T>
    void write(const std::string& text,
               const std::vector<T>& value){
         
         print(text, *out);
         for(auto& v : value){
           print(v, *out);
         }
         if(TraceSettings::getInstance()->isFileTraceOn())
         {
            *traceFile << text << endl;
            for(auto& v : value){
              *traceFile << v << endl;
            }
         }
    }


/*

1.1.1.1 ~fileName~

Stores the name of the trace file

*/
private:
    std::string fileName;
    std::ostream* out;

/*

1.1.1.1 ~traceFile~

Stores a pointer to the trace file.

*/
    std::unique_ptr<std::ofstream> traceFile;
/*

1.1.1.1 ~traceWriterMutex~

Mutex used to coordinate multi-threaded access by different servers.

*/
    boost::mutex traceWriterMutex;
};

} /* namespace DBService */

#endif /* ALGEBRAS_DBSERVICE_TRACEWRITER_HPP_ */
