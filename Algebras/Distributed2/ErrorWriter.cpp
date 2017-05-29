/*
----
This file is part of SECONDO.

Copyright (C) 2015,
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

#include "ErrorWriter.h"
#include "FileSystem.h"
#include "SecondoSystem.h"
#include "WinUnix.h"

using namespace std;

namespace distributed2 {

ErrorWriter::ErrorWriter(): dbname(""), pid(""), mtx(){
    pid = stringutils::int2str(WinUnix::getpid());
}

ErrorWriter::~ErrorWriter(){
    if(dbname.size()>0){
        out.close();
    }
}

void ErrorWriter::writeLog( const string& host, const int port,
        const int server_pid, const string& message){
    boost::lock_guard<boost::mutex> guard(mtx);
    try{
        string dbname = SecondoSystem::GetInstance()->GetDatabaseName();
        bool open = this->dbname.size()==0;
        if(this->dbname.size()>0 && this->dbname!=dbname){
            out.close();
            this->dbname = dbname;
        }
        if(this->dbname.size()==0){
            return; // log only for
        }
        string fn;

        if(open){
            FileSystem::CreateFolder("dist2Messages");
            fn = "dist2Messages/Errors_" + dbname
                    + "_"+pid+".txt";
            out.open(fn.c_str(), ios::out|ios::app);
        }
        if(!out.good()){
            cerr << "Cannot write to file " << fn << endl;
            return;
        }
        out << " ----- " << endl;
        out << "Host " << host << "@" << port << endl;
        out << "Server-PID " << server_pid << endl;
        out << "message" << endl << message << endl << endl;
    } catch(...){
        cerr << "error in writing log file" << endl;
    }
}

} /* namespace distributed2 */
