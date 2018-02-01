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

2017-10-28: Sebastian J. Bronner $<$sebastian@bronner.name$>$

\tableofcontents

1 Preliminary Setup

*/
#include "ConnectionSession.h"

namespace distributed2 {
  extern bool showCommands;
  extern distributed2::CommandLog commandLog;
}

namespace distributed4 {
  using distributed2::ConnectionInfo;
  using distributed2::commandLog;
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
      throw runtime_error{"The ConnectionInfo pointer passed to "
        "ConnectionSession is invalid."};
    try {
      switchDatabase();
    } catch(const runtime_error&) {
      if(d)
        delete ci;
      throw;
    }
  }
/*
2.2 "ConnectionSession(const string, int, const string)"[1]

"host"[1], "port"[1], and "config"[1] describe the secondo instance to which a
connection should be established. That connection is established and the
database changed to the current local database.

*/
  ConnectionSession::ConnectionSession(const string& host, int port, const
      string& config) {
    string config_temp{config};  // because non-const in createConnection
    ci = ConnectionInfo::createConnection(host, port, config_temp);
    if(!ci)
      throw runtime_error{"A connection to " + host + ":" + to_string(port) +
        " could not be established."};
    try {
      switchDatabase();
    } catch(const runtime_error&) {
      delete ci;
      throw;
    }
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
        ci->simpleCommand(*it, err, msg, res, false, rt, true, 
            commandLog);
    }
    if(deleteci)
      delete ci;
  }
/*
4 Informational Member Functions

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

5 Active Member Functions

These member functions perform some action on the connected [secondo] instance.

5.1 "run"[1]

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
    ci->simpleCommand(cmd, err, msg, res, false, rt, showCommands, 
        commandLog);
    if(err)
      throw runtime_error{"Peer " + ci->getHost() + ":" +
        to_string(ci->getPort()) + ": " + cmd + ": " + msg};
    if(!uncmd.empty())
      rollback.push_back(uncmd);
    return NList{res};
  }
/*
5.2 "clearRollback"[1]

After running a sequence of commands, the caller may reach a state where it is
no longer desirable (or necessary) to perform a rollback. That is where
"clearRollback"[1] comes in. "rollback"[1] is emptied of its stored commands so
that any future rollback will only be performed on new commands.

*/
  void ConnectionSession::clearRollback() {
    rollback.clear();
  }
/*
5.3 "switchDatabase"[1]

*/
  void ConnectionSession::switchDatabase(const string& dbname) {
    if(!ci->switchDatabase(dbname, false, showCommands))
      throw runtime_error{"The database \"" + dbname + "\" could not be "
        "opened on " + ci->getHost() + ":" + to_string(ci->getPort()) + "."};
  }
/*
5.4 "beginTransaction"[1]

*/
  void ConnectionSession::beginTransaction() {
    run("begin transaction", "abort transaction");
  }
/*
5.5 "commitTransaction"[1]

*/
  void ConnectionSession::commitTransaction() {
    clearRollback();
    run("commit transaction");
  }
/*
6 Object Manipulation Member Functions

6.1 "updateObject"[1]

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
6.2 "letObject"[1]

Create a new object named "name"[1] of type "type"[1] with the value at
"address"[1]. Specifying the type is optional; if it is not specified, it is
derived from a local object named "name"[1]. It is also possible to specify
that an object is to be created with an empty value (but defined). This is
indicated by passing "nullptr"[1] in "address"[1] (or not specifying it at all
if "type"[1] is also not specified). In this case an attempt is made to create
the object by instantiating it with the nested list "()". This is not possible
for all objects, though, and will abort with an error for those objects. It is
useful, though, for example, to create relations with no tuples in them.

*/
  void ConnectionSession::letObject(const string& name, const Address address,
      NList type)
  {
    SecondoCatalog* ctlg{SecondoSystem::GetCatalog()};
    if(type.isEmpty())
      type = NList{ctlg->GetObjectTypeExpr(name)};
    NList value{address ? ctlg->OutObject(type.listExpr(), Word{address}) :
      NList{}};
    run("let " + name + " = [const " + type.convertToString() + " value " +
        value.convertToString() + "]", "delete " + name);
  }
/*
6.3 "deleteObject"[1]

*/
  void ConnectionSession::deleteObject(const string& name) {
    run("delete " + name);
  }
/*
6.4 "lockObject"[1]

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
6.5 "unlockObject"[1]

*/
  void ConnectionSession::unlockObject(const string& name) {
    run("query unlock(\"" + name + "\")");
  }
/*
7 Atom Query Member Functions

These member functions handle the common case where a single value atom is
expected as a response to a query.

7.1 "queryAtom"[1]

Determine the response from any query that responds with a simple value. If the
response is more complex than that, "runtime\_error"[1] is thrown. "cmd"[1] may
be given without the keyword "query"[1].

*/
  NList ConnectionSession::queryAtom(string cmd) {
    if(cmd.substr(0, 6) != "query ")
      cmd = "query " + cmd;
    NList resp{run(cmd)};
    if(!(resp.length() == 2 && resp.first().isSymbol()))
      throw runtime_error{"Received a response from " + ci->getHost() + ":" +
        to_string(ci->getPort()) + " that is not an atom: " + cmd + " -> " +
          resp.convertToString()};
    return resp;
  }
/*
7.2 "querySymbol"[1]

*/
  string ConnectionSession::querySymbol(const string& cmd) {
    NList resp{queryAtom(cmd)};
    if(!(resp.first().str() == "symbol" && resp.second().isSymbol()))
      throw runtime_error{"Received a value from " + ci->getHost() + ":" +
        to_string(ci->getPort()) + " that is not a symbol: " + cmd + " -> " +
          resp.convertToString()};
    return resp.second().str();
  }
/*
7.3 "queryString"[1]

*/
  string ConnectionSession::queryString(const string& cmd) {
    NList resp{queryAtom(cmd)};
    if(!(resp.first().str() == CcString::BasicType() &&
          resp.second().isString()))
      throw runtime_error{"Received a value from " + ci->getHost() + ":" +
        to_string(ci->getPort()) + " that is not a string: " + cmd + " -> " +
          resp.convertToString()};
    return resp.second().str();
  }
/*
7.4 "queryText"[1]

*/
  string ConnectionSession::queryText(const string& cmd) {
    NList resp{queryAtom(cmd)};
    if(!(resp.first().str() == FText::BasicType() && resp.second().isText()))
      throw runtime_error{"Received a value from " + ci->getHost() + ":" +
        to_string(ci->getPort()) + " that is not a text: " + cmd + " -> " +
          resp.convertToString()};
    return resp.second().str();
  }
/*
7.5 "queryInt"[1]

*/
  int ConnectionSession::queryInt(const string& cmd) {
    NList resp{queryAtom(cmd)};
    if(!(resp.first().str() == CcInt::BasicType() && resp.second().isInt()))
      throw runtime_error{"Received a value from " + ci->getHost() + ":" +
        to_string(ci->getPort()) + " that is not an integer: " + cmd + " -> " +
          resp.convertToString()};
    return resp.second().intval();
  }
/*
7.6 "queryReal"[1]

*/
  double ConnectionSession::queryReal(const string& cmd) {
    NList resp{queryAtom(cmd)};
    if(!(resp.first().str() == CcReal::BasicType() && resp.second().isReal()))
      throw runtime_error{"Received a value from " + ci->getHost() + ":" +
        to_string(ci->getPort()) + " that is not a real: " + cmd + " -> " +
          resp.convertToString()};
    return resp.second().realval();
  }
/*
7.7 "queryBool"[1]

*/
  bool ConnectionSession::queryBool(const string& cmd) {
    NList resp{queryAtom(cmd)};
    if(!(resp.first().str() == CcBool::BasicType() && resp.second().isBool()))
      throw runtime_error{"Received a value from " + ci->getHost() + ":" +
        to_string(ci->getPort()) + " that is not a bool: " + cmd + " -> " +
          resp.convertToString()};
    return resp.second().boolval();
  }
/*
7.8 "queryNumeric"[1]

*/
  double ConnectionSession::queryNumeric(const string& cmd) {
    try {
      return queryReal(cmd);
    } catch(const runtime_error&) {
      try {
        return static_cast<double>(queryInt(cmd));
      } catch(const runtime_error&) {
        throw runtime_error{"Received a value from " + ci->getHost() + ":" +
          to_string(ci->getPort()) + " that is neither a real nor an integer: "
            + cmd};
      }
    }
  }
/*
7.9 "queryTextual"[1]

*/
  string ConnectionSession::queryTextual(const string& cmd) {
    try {
      return queryString(cmd);
    } catch(const runtime_error&) {
      try {
        return queryText(cmd);
      } catch(const runtime_error&) {
        throw runtime_error{"Received a value from " + ci->getHost() + ":" +
          to_string(ci->getPort()) + " that is neither a string nor a text: "
            + cmd};
      }
    }
  }
}
