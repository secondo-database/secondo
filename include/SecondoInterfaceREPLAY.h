
/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, 
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

#ifndef SECONDO_INTERFACE_REPLAY_H
#define SECONDO_INTERFACE_REPLAY_H

#include "SecondoInterfaceCS.h"

class SecondoInterfaceREPLAY : public SecondoInterfaceCS{
  
  
public:
  SecondoInterfaceREPLAY(bool isServer = false, NestedList* _nl=0);
  
  virtual ~SecondoInterfaceREPLAY();

  virtual bool Initialize( const string& user, const string& pswd,
                   const string& host, const string& port,
                   string& profile,
                   string& errorMsg,
                   const bool multiUser = false );

  virtual void Terminate();

  virtual void Secondo( const string& commandText,
                const ListExpr commandLE,
                const int commandType,
                const bool commandAsText,
                const bool resultAsText,
                ListExpr& resultList,
                int& errorCode,
                int& errorPos,
                string& errorMessage,
                const string& resultFileName =
                                "SecondoResult",
                const bool isApplicationLevelCommand = true);

protected:
  void showMasterConfig();  
  bool getMasterConfig(const string& parmFile, const string &delimPart);
  bool getNodesConfig(const string& parmFile, const string &delimPart, 
                      const string &delimFull);
  void showNodesConfig();
  bool connectNode(const unsigned int nodeNo, const string& user, 
                   const string& pswd);
  bool connectReplayNodes(const string& user, const string& pswd);
  bool disconnectNode(const unsigned int nodeNo);
  bool disconnectReplayNodes();
  bool sendCommandToNode(const unsigned int nodeNo, const int commandType,
                         const string& cmdText);

  bool checkReplayCommand(string& cmdText);

  struct ReplayHost {
    string hostname;
    string ip;
    string port;
    string cores;
    Socket* server;
    NestedList* nl;  
    CSProtocol* csp;
    bool initialized; // state of replay node
  };

  ReplayHost master;
  std::vector<ReplayHost> nodes;

};

#endif

