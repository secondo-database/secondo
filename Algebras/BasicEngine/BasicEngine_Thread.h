/*
----
This file is part of SECONDO.

Copyright (C) 2021,
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

@author
c. Behrndt

@note
Checked - 2020

@history
Version 1.0 - Created - C.Behrndt - 2020

*/
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/algorithm/string.hpp>
#include "Algebras/Distributed2/ConnectionInfo.h"
#include "ConnectionGeneric.h"

#ifndef _BasicEngine_Thread_H_
#define _BasicEngine_Thread_H_

namespace BasicEngine {

/*
4 Class ~BasicEngine\_Thread~

This class represents the controling from the system.

*/
class BasicEngine_Thread{

public:

/*
4.1 Public Methods

4.1.1 Constructor

*/
BasicEngine_Thread(distributed2::ConnectionInfo* _ci){ 
  ci = _ci; 
};

/*
4.1.2 Destructor

*/
virtual ~BasicEngine_Thread() {};

bool getResult();

void startImport(std::string _tab,
                std::string _remoteCreateName,std::string _remoteName,
                const bool importSchema);

void startExport(std::string _tab, std::string _path, std::string _nr,
              std::string _remoteCreateName, std::string _remoteName);

void startBEQuery(std::string _tab, std::string _query);

void startBECommand(std::string command);

void startSecondoCommand(std::string command);

bool simpleCommand(const std::string &command);

private:

/*
4.2 Members

4.2.1 ~runner~

The runner for the thread.

*/
boost::thread runner;

/*
4.2.2 ~mtx~

The variable for the mtex.

*/
boost::mutex mtx;

/*
4.2.3 ~tab~

 A variable for the tab name.

*/
std::string tab;

/*
4.2.4 ~remoteCreateName~

A variable for the name of the file,
which stores a Create-Statement.

*/
std::string remoteCreateName;

/*
4.2.5 ~importSchema~

Perform schema import

*/
bool importSchema;

/*
4.2.5 ~remoteName~

A variable for the name of the target table.

*/
std::string remoteName;

/*
4.2.6 ~query~

A variable for the query.

*/
std::string query;

/*
4.2.7 ~path~

A variable for the path.

*/
std::string path;

/*
4.2.8 ~nr~

A variable for a number of this worker.

*/
std::string nr;

/*
4.2.9 ~ci~

This is the connection to this worker.

*/
distributed2::ConnectionInfo* ci;

/*
4.2.10 ~started~

This boolean variable shows the status of the thread.
If one thread is running then its TRUE, else its FALSE.

*/
bool started = false;

/*
4.2.11 ~val~

This boolean variable shows if there was anywhere a failure.

*/
bool val = true;

/*
4.3 Private Methods

*/
void runImport();
void runExport();
void runBEQuery();
void runBECommand();
void runSecondoCommand();


}; // Class BasicEngine_Thread

}; /* namespace BasicEngine */

#endif //_BasicEngine_Thread_H_
