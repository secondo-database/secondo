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

//[$][\$]

*/

/*
3.1 ~createConnection~

Creating a specified and saves it in the connections vector.
Additionally add an entry to the importer vector.

*/
#include "WorkerConnection.h"

using namespace distributed2;
using namespace std;

namespace BasicEngine {

bool WorkerConnection::createConnection() {

  if (connection != nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Connection is alrady established";
    return false;
  }

  string config = string(connectionInfo->config);

  ConnectionInfo *ci = ConnectionInfo::createConnection(
      connectionInfo->host, stoi(connectionInfo->port), config, defaultTimeout,
      defaultHeartbeat);

  if (ci == nullptr) {
    BOOST_LOG_TRIVIAL(error)
        << "Couldn't connect to secondo-Worker on host " << connectionInfo->host
        << " with port " << connectionInfo->port << "!";

    return false;
  }

  bool switchResult = ci->switchDatabase(connectionInfo->dbName, true, false,
                                         true, defaultTimeout);

  if (!switchResult) {
    BOOST_LOG_TRIVIAL(error)
        << "Unable to switch to database " << connectionInfo->dbName
        << " on host " << connectionInfo->host << " with port "
        << connectionInfo->port << "!";

    ci->deleteIfAllowed();
    return false;
  }

  connection = ci;

  return true;
}

/*
3.2 ~executeSecondoCommand~

Execute a SECONDO command

*/
bool WorkerConnection::executeSecondoCommand(const string &command,
                                             const bool checkResult) {

  if (connection == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Connection is not established" << this;
    return false;
  }

  string errMsg;
  int err = 0;
  double rt;
  string res;
  CommandLog commandLog;

  connection->simpleCommand(command, err, res, false, rt, false, commandLog,
                            true, defaultTimeout);

  if (err != 0) {
    BOOST_LOG_TRIVIAL(error)
        << "Got ErrCode:" << err << " / command was: " << command;

    return false;
  }

  if (!checkResult) {
    return true;
  }

  if (res != "(bool TRUE)") {
    BOOST_LOG_TRIVIAL(error) << "Error: Got invalid result from remote node "
                             << res << " / command was: " << command;

    return false;
  }

  return true;
}

/*
3.28 ~simpleCommand~

Execute a command or query on the worker.

Returns true if everything is OK and there are no failure.
Displays an error message if something goes wrong.

*/
bool WorkerConnection::performSimpleSecondoCommand(const std::string &command) {

  if (connection == nullptr) {
    BOOST_LOG_TRIVIAL(error) << "Connection is not established" << this;
    return false;
  }

  int err = 0;
  double rt;
  const int defaultTimeout = 0;
  distributed2::CommandLog CommandLog;
  std::string res;

  connection->simpleCommand(command, err, res, false, rt, false, CommandLog,
                            true, defaultTimeout);

  if (err != 0) {
    BOOST_LOG_TRIVIAL(error) << "Got error from server: " << this << " " << err
                             << res << " command was: " << command;
    return false;
  }

  bool resultOk = (res == "(bool TRUE)");

  if (!resultOk) {
    BOOST_LOG_TRIVIAL(error) << "Got unexpected result from server: " << this
                             << " " << res << " command was: " << command;
    return false;
  }

  return true;
}

} // namespace BasicEngine