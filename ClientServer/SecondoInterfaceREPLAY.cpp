/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

1 The Implementation-Module SecondoInterfaceREPLAY

June 2015 Matthias Kunsmann ReplayVersion of the ~SecondoInterfaceCS~

[TOC]

*/

#include <iostream>
#include <fstream>
#include <sstream>

//#define TRACE_ON 1

#undef TRACE_ON

#include "LogMsg.h"
#include "SecondoInterfaceREPLAY.h"
#include "SocketIO.h"
#include "Profiles.h"
#include "CSProtocol.h"
#include "StringUtils.h"

#include <thread>
#include <future>

using namespace std;

SecondoInterfaceREPLAY::SecondoInterfaceREPLAY(bool isServer, /*= false*/
                                               NestedList* _nl /*=0 */):
  SecondoInterfaceCS(isServer, _nl) { }

SecondoInterfaceREPLAY::~SecondoInterfaceREPLAY() {
}

bool
SecondoInterfaceREPLAY::getMasterConfig(const string& parmFile, 
                                        const string& delimPart) {
/*
Read the configuration of the master from the configuration file and save it
in the instance variable for later use.

*/

  string theMaster;
  theMaster = SmiProfile::GetParameter("Replay", "Master", "", parmFile);

  stringutils::StringTokenizer token(theMaster, delimPart);

  master.hostname = token.nextToken();
  master.ip = token.nextToken();
  master.port = token.nextToken();
  master.cores = token.nextToken();

  // check Master config
  if (master.hostname != "" && master.ip != "" && 
      master.port != "" && master.cores != "") {
    return true;
  } else {
    cout << "Wrong configuration of master. Please check!" << endl << endl;    
    return false;
  }
  
}

bool
SecondoInterfaceREPLAY::getNodesConfig(const string& parmFile, 
                       const string &delimPart, const string &delimFull) {
/*
Read the configuration of all nodes from the configuration file and save it
in the instance variable (vector) for later use.

*/

  string allNodes;
  string currentNode;
  ReplayHost elem;
  allNodes = SmiProfile::GetParameter("Replay", "Node", "", parmFile);

  // only add nodes if at least one is configured in configuration file
  if (allNodes != "") {
    stringutils::StringTokenizer token(allNodes, delimFull);
    while (token.hasNextToken()) {
      currentNode = token.nextToken();
      stringutils::StringTokenizer innertoken(currentNode, delimPart);
      elem.hostname = innertoken.nextToken();
      elem.ip = innertoken.nextToken();
      elem.port = innertoken.nextToken();
      elem.cores = innertoken.nextToken();
      elem.initialized = false;
      elem.csp = 0;
      elem.server = 0;
      elem.nl = 0;
      
      // check Nodes config
      if (elem.hostname != "" && elem.ip != "" && 
          elem.port != "" && elem.cores != "") {
        nodes.push_back(elem); 
      } else {
        cout << "Wrong configuration of one node. Please check!" 
             << endl << endl;
      }
    }
  }

  return true;
}

void
SecondoInterfaceREPLAY::showMasterConfig() {
/*
Display the current configuration of the master.

*/

  cout << endl << "Master Configuration" << endl;
  cout << "Hostname: " << master.hostname << endl;
  cout << "IP-Adress: " << master.ip << endl;
  cout << "Port: " << master.port << endl;
  cout << "Cores: " << master.cores << endl << endl;
}

void
SecondoInterfaceREPLAY::showNodesConfig() {
/*
Display the configuration of every node.

*/

  // check if at least one node is available
  if (nodes.size() > 0) {
    cout << "Nodes Configuration" << endl;
  }

  for (unsigned int i=0; i<nodes.size(); ++i) {
      
    cout << "Hostname: " << nodes[i].hostname << endl;
    cout << "IP-Adress: " << nodes[i].ip << endl;
    cout << "Port: " << nodes[i].port << endl;
    cout << "Cores: " << nodes[i].cores << endl << endl;
  }
}

bool
SecondoInterfaceREPLAY::connectNode(const unsigned int nodeNo, 
                                  const string& user, const string& pswd) {
/*
Connect client with the node in nodes-vector with the index nodeNo.

*/

   string line = "";
   cout << "Connecting with Secondo server '" << nodes[nodeNo].hostname 
        << "' (node) on port "
       << nodes[nodeNo].port << " ..." << endl;
 
    nodes[nodeNo].server = Socket::Connect( nodes[nodeNo].ip, 
                            nodes[nodeNo].port, Socket::SockGlobalDomain);
 
    if ( nodes[nodeNo].server != 0 && nodes[nodeNo].server->IsOk() )
    {
      iostream& iosock = nodes[nodeNo].server->GetSocketStream();

      if(nodes[nodeNo].csp!=0) {
        delete nodes[nodeNo].csp;
      }
      nodes[nodeNo].nl = new NestedList();
      nodes[nodeNo].csp = new CSProtocol(nodes[nodeNo].nl, iosock);
      getline( iosock, line );
      if ( line == "<SecondoOk/>" ) {
        iosock << "<Connect>" << endl
               << user <<  endl
               << pswd << endl
               << "</Connect>" << endl;
        getline( iosock, line );

        if ( line == "<SecondoIntro>" )
          {
            do
            {
              getline( iosock, line );
              if ( line != "</SecondoIntro>" )
              {
                cout << line << endl;
              }
            }
            while (line != "</SecondoIntro>");
            nodes[nodeNo].initialized = true;
          }
          else if ( line == "<SecondoError>" )
          {
            getline( iosock, line );
            cout << "Server-Error: " << line << endl;
            getline( iosock, line );
          }
          else
          {
            cout << "Unidentifiable response from server: " << line << endl;
          }
        }
    }
    else
    {
      cout << "Failed connect to: " << nodes[nodeNo].hostname 
           << " with ip: " << nodes[nodeNo].ip << endl;
      nodes[nodeNo].server = 0;
      return false;
    }
    return true;
}

bool
SecondoInterfaceREPLAY::connectReplayNodes(const string& user,
                                           const string& pswd) {
/*
Connect all nodes from nodes-vector asynchron with the client. Uses the new 
async-Feature from C++ 11.

*/

  // check if at least one node is available
  if (nodes.size() == 0) {
    cout << "No Nodes configured!" << endl;
    return true;
  }

  bool futureRes;
  vector<std::future<bool>> futures;

  for (unsigned  int i=0; i<nodes.size(); ++i) {
    futures.push_back(async(std::launch::async, 
                    &SecondoInterfaceREPLAY::connectNode, this, i,
                     user, pswd));
  }

  // wait all connects are ready
  for (unsigned int i=0; i<futures.size(); ++i) {
    try {
      futureRes = futures[i].get();
      if (futureRes == false) {
        cout << "Error while connecting to node : " 
             << nodes[i].hostname << endl;
      }
    } catch (const exception& e) {
      cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what() 
           << " for connection of node: " << nodes[i].hostname
           << "with ip:" << nodes[i].ip;

      return false;
    }
  }
  return true;
}

bool
SecondoInterfaceREPLAY::disconnectNode(const unsigned int nodeNo) {
/* 
Disconnect client from the node in nodes-vector with the index nodeNo.

*/

  if (nodes[nodeNo].server != 0) 
  {
    iostream& iosock = nodes[nodeNo].server->GetSocketStream();
    iosock << "<Disconnect/>" << endl;

    nodes[nodeNo].server->Close();
    delete nodes[nodeNo].server;
    nodes[nodeNo].server = 0;
  }
   
  if (nodes[nodeNo].csp != 0){
    delete nodes[nodeNo].csp;
    nodes[nodeNo].csp = 0;
  }

  if (nodes[nodeNo].initialized)
  {
    delete nodes[nodeNo].nl;
    nodes[nodeNo].initialized = false;
  }

  return true;
}

bool
SecondoInterfaceREPLAY::disconnectReplayNodes()
{
/*
Disconnect all nodes from nodes-vector asynchron with the client. Uses the new 
async-Feature from C++ 11.

*/

  bool futureRes;

  vector<std::future<bool>> futures;
  for (unsigned int i=0; i<nodes.size(); ++i) 
  {
    futures.push_back(async(std::launch::async, 
                       &SecondoInterfaceREPLAY::disconnectNode, this, i));
  }

  // wait until all nodes are disconnected
  for (unsigned int i=0; i<futures.size(); ++i) {
    try {
      futureRes = futures[i].get();
      if (futureRes == false) {
        cout << "Error while disconnecting from node : " 
             << nodes[i].hostname << endl;
      }
    } catch (const exception& e) {
      cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what() 
           << " for disconnection of node: " << nodes[i].hostname
           << "with ip:" << nodes[i].ip;
    }
  }
  return true;
}

bool
SecondoInterfaceREPLAY::checkReplayCommand(string& cmdText)
{
/*
Check the assigned command if it a command only for master or for master 
and all nodes.

1) Check if comand is one of the new s-commands, which only execute on 
   the master.

2) Data Retrieval language command (query) only execute on the master.


3) Inquieries only execute on the master.

*/

  std::size_t found; 
  bool onlyMasterCommand = false;

  // Data Manipulation Language commands
  // New s-command sends only to the master
  // Remove s before sending to master
  found = cmdText.find("stype");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
    cmdText = cmdText.replace(found,5,"type");
  }
  found = cmdText.find("sdelete type");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
    cmdText = cmdText.replace(found,12,"delete type");
  }
  found = cmdText.find("screate");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
    cmdText = cmdText.replace(found,7,"create");
  }
  found = cmdText.find("supdate");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
    cmdText = cmdText.replace(found,7,"update");
  }
  found = cmdText.find("slet");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
    cmdText = cmdText.replace(found,4,"let");
  }
  found = cmdText.find("sderive");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
    cmdText = cmdText.replace(found,7,"derive");
  }
  found = cmdText.find("sdelete");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
    cmdText = cmdText.replace(found,7,"delete");
  }
  found = cmdText.find("skill");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
    cmdText = cmdText.replace(found,5,"kill");
  }
    
  // Data Retrieval Language command only send to master
  found = cmdText.find("query");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
  }
 
  // Inquiries only send to master
  found = cmdText.find("list");
  if (found != std::string::npos) {
    onlyMasterCommand = true;
  }

  return onlyMasterCommand;
}

  
void
SecondoInterfaceREPLAY::Terminate()
{
  // Disconnect all nodes, then the master
  disconnectReplayNodes();

  if ( server != 0 )
  {
    iostream& iosock = server->GetSocketStream();
    iosock << "<Disconnect/>" << endl;
    server->Close();
    delete server;
    server = 0;

  }

  if (csp != 0){
     delete csp;
     csp = 0;
  }

  if (initialized)
  {
    if(!externalNL){
      delete nl;
      SmiEnvironment::DeleteTmpEnvironment();
    }
    al = 0;
    initialized = false;
  }
}

bool
SecondoInterfaceREPLAY::Initialize( const string& user, const string& pswd,
                                    const string& host, const string& port,
                                    string& parmFile, string& errorMsg,
                                    const bool multiUser )
{
  // variables for replay-version
  bool checkMaster;

  string delimFull = ",";
  string delimPart = ":";
  // ---

  string secHost = host;
  string secPort = port;

  string line = "";

  if ( !initialized )
  {
    cout << "Initializing the Secondo system ..." << endl << endl;

    // configuration of master and all nodes of replay configuration
    cout << "Secondo Replay Configuration:" << endl;
    checkMaster = getMasterConfig(parmFile, delimPart);

    // if master not correctly configured, use 
    // param host, port -> if available
    // if not available use params from SecondoConfig.ini SecondoHost 
    // and SecondoPort
    if (!checkMaster) {

      if (secHost.length() == 0 || secPort.length() == 0) {
        if ( parmFile.length() != 0 ) {
  
          secHost = SmiProfile::GetParameter( "Environment",
                                              "SecondoHost",
                                              "", parmFile );

          secPort = SmiProfile::GetParameter( "Environment",
                                              "SecondoPort",
                                              "", parmFile );
        }
      } 
    } else {
      secHost = master.ip;
      secPort = master.port;
      showMasterConfig();
    
      getNodesConfig(parmFile, delimPart, delimFull);
      showNodesConfig();

    }

    // initialize runtime flags
    InitRTFlags(parmFile);

    bool ok=false;
    if(externalNL){
      cout << "use already existing nested list storage" << endl;
    } else {
      cout << "Setting up temporary Berkeley-DB envinronment" << endl;
      if ( SmiEnvironment::SetHomeDir(parmFile) ) {
           SmiEnvironment::SetUser( user ); // TODO: Check for valid user/pswd
           ok = true;
      } else {
           string errMsg;
           SmiEnvironment::GetLastErrorCode( errMsg );
            cout  << "Error: " << errMsg << endl;
            errorMsg += errMsg +"\n";
            ok=false;
       }

       if (ok)
          ok = (SmiEnvironment::CreateTmpEnvironment( cerr ) == 0);

       if (!ok) {
           cerr << "Error: No Berkeley-DB environment! "
                << "Persistent nested lists not available!"
               << endl;
       }
    }

    if ( secHost.length() > 0 && secPort.length() > 0 )
    {
      cout << "Connecting with Secondo server '" << secHost << "' on port "
           << secPort << " ..." << endl;
       
      server = Socket::Connect( secHost, secPort, Socket::SockGlobalDomain );

      if ( server != 0 && server->IsOk() )
      {
        iostream& iosock = server->GetSocketStream();
        
        if(csp!=0){
          delete csp;
        }
        csp = new CSProtocol(nl, iosock);
        getline( iosock, line );
        if ( line == "<SecondoOk/>" )
        {
          iosock << "<Connect>" << endl
                 << user <<  endl
                 << pswd  << endl
                 << "</Connect>" << endl;
          getline( iosock, line );
          if ( line == "<SecondoIntro>" )
          {
            do
            {
              getline( iosock, line );
              if ( line != "</SecondoIntro>" )
              {
                cout << line << endl;
              }
            }
            while (line != "</SecondoIntro>");
            initialized = true;
          }
          else if ( line == "<SecondoError>" )
          {
            getline( iosock, line );
            cout << "Server-Error: " << line << endl;
            getline( iosock, line );
          }
          else
          {
            cout << "Unidentifiable response from server: " << line << endl;
          }
        }
        else if ( line == "<SecondoError>" )
        {
          getline( iosock, line );
          cout << "Server-Error: " << line << endl;
          getline( iosock, line );
        }
        else
        {
          cout << "Unidentifiable response from server: " << line << endl;
        }

      }
      else
      {
        cout << "failed." << endl;
      }
      if ( !initialized && server != 0)
      {
        server->Close();
        delete server;
        server = 0;
      }
    }
    else
    {
      cout << "Invalid or missing host (" << secHost << ") and/or port ("
           << secPort << ")." << endl;
    }

    // After connect of master connect all nodes
    // If connect to master failed, don't connect to the nodes
    if (checkMaster && initialized) {
      connectReplayNodes(user, pswd);
    }
  }

  return (initialized);
}

bool 
SecondoInterfaceREPLAY::sendCommandToNode(const unsigned int nodeNo, 
                                const int commandType, const string& cmdText) {
  ListExpr nodeResultList;
  int nodeErrorCode;
  int nodeErrorPos;
  string nodeErrorMessage;

  if (nodes[nodeNo].server != 0 && nodes[nodeNo].server->IsOk())
  {
    nodeErrorMessage = "";
    nodeErrorCode    = 0;
    nodeResultList   = nodes[nodeNo].nl->TheEmptyList();

    iostream& iosocknode = nodes[nodeNo].server->GetSocketStream();
    ios_base::iostate s = iosocknode.exceptions();
    iosocknode.exceptions(ios_base::failbit|ios_base::badbit|ios_base::eofbit);

    if ( iosocknode.fail() )
    {
      nodeErrorCode = ERR_CONNECTION_TO_SERVER_LOST;
    }
    if ( nodeErrorCode != 0 )
    {
      cout << "Received an error from Node " << nodes[nodeNo].hostname << ": ";
      cout << nodeErrorCode << endl;

      return false;
    }

    iosocknode << "<Secondo>" << endl
               << commandType << endl
               << cmdText << endl
               << "</Secondo>" << endl;

     // Receive result
     nodeErrorCode = nodes[nodeNo].csp->ReadResponse( nodeResultList,
                                       nodeErrorCode, nodeErrorPos,
                                        nodeErrorMessage         );

     iosocknode.exceptions(s);

     if (nodeErrorCode != 0) 
     { 
       cout << "Received an error from Node " << nodes[nodeNo].hostname << ": ";
       cout << nodeErrorMessage << endl;
     }
  }
  return true; 
}

void
SecondoInterfaceREPLAY::Secondo( const string& commandText,
                             const ListExpr commandLE,
                             const int commandType,
                             const bool commandAsText,
                             const bool resultAsText,
                             ListExpr& resultList,
                             int& errorCode,
                             int& errorPos,
                             string& errorMessage,
                             const string& resultFileName /*="SecondoResult"*/,
                             const bool isApplicationLevelCommand /* = true */ )
{
/*
~Secondo~ reads a command and executes it; it possibly returns a result.
The command is one of the set of SECONDO commands.

For an explanation of the error codes refer to SecondoInterface.h

*/

  string cmdText="";
  ListExpr list;
  string filename="", dbName="", objName="", typeName="";
  errorMessage = "";
  errorCode    = 0;
  resultList   = nl->TheEmptyList();

  // variables for replay-version
  bool onlyMasterCommand = false;
  bool futureRes;
  // ---

  if ( server == 0 )
  {
    errorCode = ERR_IN_SECONDO_PROTOCOL;
  }
  else
  {
    switch (commandType)
    {
      case 0:  // list form
      {
        if ( commandAsText )
        {
          cmdText = commandText;
        }
        else
        {
          if ( !nl->WriteToString( cmdText, commandLE ) )
          {
            // syntax error in command/expression
            errorCode = ERR_SYNTAX_ERROR;
          }
        }
        break;
      }
      case 1:  // text form
      {
        cmdText = commandText;
        break;
      }
      default:
      {
       // Command type not implemented
        errorCode = ERR_CMD_LEVEL_NOT_YET_IMPL;
      }
    } // switch
  }

  string line;
  iostream& iosock = server->GetSocketStream();

  ios_base::iostate s = iosock.exceptions();
  iosock.exceptions(ios_base::failbit|ios_base::badbit|ios_base::eofbit);

  if ( iosock.fail() )
  {
    errorCode = ERR_CONNECTION_TO_SERVER_LOST;
  }
  if ( errorCode != 0 )
  {
    return;
  }

  string::size_type posDatabase = cmdText.find( "database " );
  string::size_type posSave     = cmdText.find( "save " );
  string::size_type posRestore  = cmdText.find( "restore " );
  string::size_type posTo       = cmdText.find( "to " );
  string::size_type posFrom     = cmdText.find( "from " );

  if ( posDatabase != string::npos &&
       posSave     != string::npos &&
       posTo       != string::npos &&
       posSave < posDatabase && posDatabase < posTo )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    if ( nl->ReadFromString( cmdText, list ) )
    {
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "save" ) &&
           nl->IsEqual( nl->Second( list ), "database" ) &&
           nl->IsEqual( nl->Third( list ), "to" ) &&
           nl->IsAtom( nl->Fourth( list )) &&
           ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
            (nl->AtomType( nl->Fourth( list )) == SymbolType) ||
            (nl->AtomType( nl->Fourth( list )) == SymbolType) ))
      {
        switch(nl->AtomType(nl->Fourth(list))){
          case SymbolType : filename = nl->SymbolValue(nl->Fourth(list));
                             break;
          case StringType : filename = nl->StringValue(nl->Fourth(list));
                             break;
          case TextType : nl->Text2String(nl->Fourth(list),filename);
                             break;
        }
        // request for save database
        iosock << "<DbSave/>" << endl;

        errorCode = csp->ReadResponse( resultList,
                                       errorCode, errorPos,
                                       errorMessage         );

        if (errorCode == ERR_NO_ERROR) {
          nl->WriteToFile( filename.c_str(), resultList );
          resultList=nl->TheEmptyList();
        }
      }
      else
      {
        // Not a valid 'save database' command
        errorCode = 1;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }
  else if ( posSave != string::npos && // save object to filename
            posTo   != string::npos &&
            posDatabase == string::npos &&
            posSave < posTo )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    if ( nl->ReadFromString( cmdText, list ) )
    {
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "save" ) &&
           nl->IsAtom( nl->Second( list )) &&
          (nl->AtomType( nl->Fourth( list )) == SymbolType) &&
           nl->IsEqual( nl->Third( list ), "to" ) &&
           nl->IsAtom( nl->Fourth( list )) &&
          ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fourth( list )) == StringType) ||
           (nl->AtomType( nl->Fourth( list )) == TextType)))
      {
        switch(nl->AtomType(nl->Fourth(list))){
         case SymbolType : filename = nl->SymbolValue(nl->Fourth(list));
                           break;
         case StringType : filename = nl->StringValue(nl->Fourth(list));
                           break;
         case TextType : nl->Text2String(nl->Fourth(list),filename);
                           break;
        }

        objName = nl->SymbolValue( nl->Second( list ) );

        // request list representation for object
        iosock << "<ObjectSave>" << endl
               << objName << endl
               << "</ObjectSave>" << endl;

        errorCode = csp->ReadResponse( resultList,
                                       errorCode, errorPos,
                                       errorMessage         );

        if (errorCode == ERR_NO_ERROR) {
          cout << "writing file " << filename << endl;
          nl->WriteToFile( filename.c_str(), resultList );
          resultList=nl->TheEmptyList();
        }
      }
      else
      {
        // Not a valid 'save database' command
        errorCode = 1;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }

  else if ( posRestore  != string::npos &&
            posDatabase == string::npos &&
            posFrom     != string::npos &&
            posRestore < posFrom )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    if ( nl->ReadFromString( cmdText, list ) )
    {
      if ( nl->ListLength( list ) == 4 &&
           nl->IsEqual( nl->First( list ), "restore" ) &&
           nl->IsAtom( nl->Second( list )) &&
          (nl->AtomType( nl->Second( list )) == SymbolType) &&
           nl->IsEqual( nl->Third( list ), "from" ) &&
           nl->IsAtom( nl->Fourth( list )) &&
          ((nl->AtomType( nl->Fourth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fourth( list )) == StringType) ||
           (nl->AtomType( nl->Fourth( list )) == TextType))   )
      {
        // send object symbol
        string filename ="";
        switch(nl->AtomType(nl->Fourth(list))){
           case SymbolType : filename = nl->SymbolValue(nl->Fourth(list));
                             break;
           case StringType : filename = nl->StringValue(nl->Fourth(list));
                             break;
           case TextType :  nl->Text2String(nl->Fourth(list),filename);
                             break;
           default : assert(false);
        }
        iosock << csp->startObjectRestore << endl
               << nl->SymbolValue( nl->Second( list ) ) << endl;

        // send file data
        csp->SendFile(filename);

        // send end tag
        iosock << csp->endObjectRestore << endl;

        errorCode = csp->ReadResponse( resultList,
                                       errorCode, errorPos,
                                       errorMessage         );
      }
      else
      {
        // Not a valid 'restore object' command
        errorCode = ERR_CMD_NOT_RECOGNIZED;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }

  else if ( posDatabase != string::npos &&
            posRestore  != string::npos &&
            posFrom     != string::npos &&
            posRestore < posDatabase && posDatabase < posFrom )
  {
    if ( commandType == 1 )
    {
      cmdText = string( "(" ) + commandText + ")";
    }
    if ( nl->ReadFromString( cmdText, list ) )
    {
      if ( nl->ListLength( list ) == 5 &&
           nl->IsEqual( nl->First( list ), "restore" ) &&
           nl->IsEqual( nl->Second( list ), "database" ) &&
           nl->IsAtom( nl->Third( list )) &&
          (nl->AtomType( nl->Third( list )) == SymbolType) &&
           nl->IsEqual( nl->Fourth( list ), "from" ) &&
           nl->IsAtom( nl->Fifth( list )) &&
          ((nl->AtomType( nl->Fifth( list )) == SymbolType) ||
           (nl->AtomType( nl->Fifth( list )) == StringType) ||
           (nl->AtomType( nl->Fifth( list )) == TextType))   )
      {
        // send object symbol
        iosock << csp->startDbRestore << endl
               << nl->SymbolValue( nl->Third( list ) ) << endl;

        // send file data
        filename =  "";
        switch(nl->AtomType(nl->Fifth(list))){
          case SymbolType : filename = nl->SymbolValue(nl->Fifth(list));
                            break;
          case StringType : filename = nl->StringValue(nl->Fifth(list));
                            break;
          case TextType : nl->Text2String(nl->Fifth(list),filename);
                            break;
          default : assert(false);
        }
        csp->SendFile(filename);

        // send end tag
        iosock << csp->endDbRestore << endl;

        errorCode = csp->ReadResponse( resultList,
                                       errorCode, errorPos,
                                       errorMessage         );
      }
      else
      {
        // Not a valid 'restore database' command
        errorCode = ERR_CMD_NOT_RECOGNIZED;
      }
    }
    else
    {
      // Syntax error in list
      errorCode = ERR_SYNTAX_ERROR;
    }
  }
  else
  {
    // Send Secondo command

    // check command if execute only on master or on master and note
    // remove prefix s from s-commands
    onlyMasterCommand = checkReplayCommand(cmdText);

    iosock << "<Secondo>" << endl
           << commandType << endl
           << cmdText << endl
           << "</Secondo>" << endl;
 
    // Receive result
    errorCode = csp->ReadResponse( resultList,
                                   errorCode, errorPos,
                                   errorMessage         );
  }

  iosock.exceptions(s);
  if ( resultAsText )
  {
    nl->WriteToFile( resultFileName, resultList );
  }

  // Send commands to nodes
  if (!onlyMasterCommand) {
    vector<std::future<bool>> futures;
    for (unsigned int i=0; i<nodes.size(); ++i) 
    {
      futures.push_back(async(std::launch::async, 
                      &SecondoInterfaceREPLAY::sendCommandToNode, this, 
                      i, commandType, cmdText));
    }
    // wait commands sent to all nodes
    for (unsigned int i=0; i<futures.size(); ++i) {
      try {
        futureRes = futures[i].get();
        if (futureRes == false) {
          cout << "Error while sending command to node : " 
               << nodes[i].hostname << endl;
        }
      } catch (const exception& e) {
        cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what() 
             << " for sending command to node: " << nodes[i].hostname
             << "with ip:" << nodes[i].ip;
      }
    }
  }
}


