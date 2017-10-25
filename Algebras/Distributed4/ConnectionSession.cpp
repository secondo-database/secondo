/*
----
This file is part of SECONDO.

Copyright (C) 2017, Faculty of Mathematics and Computer Science, Database
Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

SECONDO is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
SECONDO; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA  02111-1307  USA
----

//paragraph [10] title: [{\Large \bf] [}]
//characters [1] tt: [\texttt{] [}]
//[secondo] [{\sc Secondo}]

[10] Implementation of Class ConnectionSession

2017-08-14: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "ConnectionSession.h"

namespace distributed2 {
  extern bool showCommands;
  extern bool logOn;
  extern distributed2::CommandLog commandLog;
}

namespace distributed4 {
  using distributed2::ConnectionInfo;
  using distributed2::commandLog;
  using distributed2::logOn;
  using distributed2::showCommands;
  using std::cerr;
  using std::runtime_error;
  using std::string;
  using std::to_string;
/*
2 Constructors

2.1 "ConnectionSession(const ConnectionInfo[*], bool)"[1]

"ci"[1] is a pointer to a "ConnectionInfo" object. The database on the secondo
instance connected by "ci"[1] is changed to the current local database. As a
side-effect that ensures that "ci"[1] is functional. "d"[1] specifies whether
this object is responsible for deleting "ci"[1], or not (the default when it is
passed in, in that case the caller is implicitly held responsible for its
pointer).

*/
  ConnectionSession::ConnectionSession(ConnectionInfo* ci, bool d): ci{ci},
    deleteci{d} {
    if(!ci)
      throw runtime_error{"The passed connection is invalid. No identifying "
        "information is available."};
    switchDatabase();
  }
/*
2.2 "ConnectionSession(const string, int, const string)"[1]

"host"[1], "port"[1], and "config"[1] describe the secondo instance to which a
connection should be established. That connection is established and the
database changed to the current local database.

*/
  ConnectionSession::ConnectionSession(const string& host, int port, string&
      config): ci{ConnectionInfo::createConnection(host, port, config)} {
    if(!ci)
      throw runtime_error{"A connection to " + host + ":" + to_string(port) +
        " could not be established."};
    switchDatabase();
  }
/*
3 Destructor

"ci"[1] is handed in by the caller or passed back by
"ConnectionInfo::createConnection"[1]. Depending on the situation,
responsibility for deleting it may lie with the caller or it may lie here.
Which is the case is tracked in "deleteci"[1].

*/
  ConnectionSession::~ConnectionSession() {
    if(!rollback.empty()) {
      cerr << "ConnectionSession for " << ci->getHost() << ":" << ci->getPort()
        << " was destroyed with " << rollback.size() << " entries in the "
        "rollback list. Performing rollback now." << endl;
      int err;
      string msg;
      ListExpr res;
      double rt;
      for(auto it{rollback.rbegin()}; it != rollback.rend(); ++it)
        ci->simpleCommand(*it, err, msg, res, false, rt, true, logOn,
            commandLog);
    }
    if(deleteci)
      delete ci;
  }
/*
4 Member Functions

4.1 "getHost"[1]

*/
  string ConnectionSession::getHost() const {
    return ci->getHost();
  }
/*
4.2 "getPort"[1]

*/
  int ConnectionSession::getPort() const {
    return ci->getPort();
  }
/*
4.3 "getConfig"[1]

*/
  string ConnectionSession::getConfig() const {
    return ci->getConfig();
  }
/*
4.4 "run"[1]

The command passed in "cmd"[1] will be executed via "ci"[1]. If "uncmd"[1] is
specified (is not the empty string), it is added to "rollback"[1] after
"cmd"[1] was executed successfully on each connection. In case of an error, a
"runtime\_error"[1] is thrown with an error message. If the caller doesn't
handle the exception, the commands in "rollback"[1] will be executed as a
consequence of leaving scope.

*/
  NList ConnectionSession::run(const string& cmd, const string& uncmd) {
    int err;
    string msg;
    ListExpr res;
    double rt;
    ci->simpleCommand(cmd, err, msg, res, false, rt, showCommands, logOn,
        commandLog);
    if(err)
      throw runtime_error{"Peer " + ci->getHost() + ":" +
        to_string(ci->getPort()) + ": " + cmd + ": " + msg};
    if(!uncmd.empty())
      rollback.push_back(uncmd);
    return NList{res};
  }
/*
4.5 "clearRollback"[1]

After running a sequence of commands, the caller may reach a state where it is
no longer desirable (or necessary) to perform a rollback. That is where
"clearRollback"[1] comes in. "rollback"[1] is emptied of its stored commands so
that any future rollback will only be performed on new commands.

*/
  void ConnectionSession::clearRollback() {
    rollback.clear();
  }
/*
4.6 "updateObject"[1]

*/
  void ConnectionSession::updateObject(const string& name, const Address
      address) {
    SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
    NList type{ctlg->GetObjectTypeExpr(name)};
    NList value{ctlg->OutObject(type.listExpr(), Word{address})};
    run("update " + name + " := [const " + type.convertToString() + " value " +
        value.convertToString() + "]");
  }
/*
4.7 "lockObject"[1]

*/
  void ConnectionSession::lockObject(const string& name, bool exclusive) {
    try {
      run("query trylock(\"" + name + "\", " + (exclusive ? "TRUE" : "FALSE") +
          ")");
    } catch(const runtime_error& e) {
      cmsg.info() << e.what() << " Waiting for " << (exclusive ? "exclusive" :
          "sharable") << " ownership." << endl;
      cmsg.send();
      run("query lock(\"" + name + "\", " + (exclusive ? "TRUE" : "FALSE") +
          ")", "query unlock(\"" + name + "\")");
    }
  }
/*
4.8 "unlockObject"[1]

*/
  void ConnectionSession::unlockObject(const string& name) {
    run("query unlock(\"" + name + "\")");
  }
/*
4.9 "beginTransaction"[1]

*/
  void ConnectionSession::beginTransaction() {
    run("begin transaction", "abort transaction");
  }
/*
4.10 "commitTransaction"[1]

*/
  void ConnectionSession::commitTransaction() {
    run("commit transaction");
  }
/*
4.11 "switchDatabase"[1]

*/
  void ConnectionSession::switchDatabase(const string& dbname) {
    if(!ci->switchDatabase(dbname, false, showCommands))
      throw runtime_error{"The database \"" + dbname + "\" could not be "
        "opened on " + ci->getHost() + ":" + to_string(ci->getPort()) + "."};
  }
/*
4.12 "createEmpty"[1]

*/
  void ConnectionSession::createEmpty(const std::string& name, const NList&
      type) {
    run("let " + name + " = [const " + type.convertToString() + " value ()]");
    rollback.push_back("delete " + name);
  }
}
