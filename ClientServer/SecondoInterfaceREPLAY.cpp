/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Department of Computer Science,
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

September 2015 Matthias Kunsmann ReplayVersion of the ~SecondoInterfaceCS~

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
#include "FileSystem.h"

#include <thread>
#include <future>

#include <algorithm>
#include <iterator>

#include <ctime>

using namespace std;

/*
Callback function for FileSystem::FileSearch in
SecondoInterfaceREPLAY::executeReplayIMGImport

Checks if filename ends with an image-format extension
for later use in replay import.

*/
bool callBackFileSearch(const string& absolutePath,
                        const string& filename,
                        FileAttributes attribs) {
  bool found = false;

  string checkFilename = filename;
  stringutils::toUpper(checkFilename);

  // JPG
  found = stringutils::endsWith(checkFilename, "JPG");
 
  if (found == false) found = stringutils::endsWith(checkFilename, "GIF");
  if (found == false) found = stringutils::endsWith(checkFilename, "TIF");
  if (found == false) found = stringutils::endsWith(checkFilename, "BMP");
  if (found == false) found = stringutils::endsWith(checkFilename, "BMP");
  if (found == false) found = stringutils::endsWith(checkFilename, "PNG");

  return found;
}

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
                                       const string &delimPart,
                                       const string &delimFull) {
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
                                    const string& user,
                                    const string& pswd) {
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
SecondoInterfaceREPLAY::checkReplayImport(const string& cmdText,
                                          string& replayImpCommand)
{
/*
Check the assigned command if it a command for the replay
import feature

*/

  std::size_t found; 
  found = cmdText.find("replayOSMImport");
  if (found != std::string::npos) {
      replayImpCommand = "replayOSMImport";
      return true;
  }

  found = cmdText.find("replayCSVImport");
  if (found != std::string::npos) {
     replayImpCommand = "replayCSVImport";
     return true;
  }

  found = cmdText.find("replaySHPImport");
  if (found != std::string::npos) {
     replayImpCommand = "replaySHPImport";
     return true;
  }

  found = cmdText.find("replayDBLPImport");
  if (found != std::string::npos) {
     replayImpCommand = "replayDBLPImport";
     return true;
  }

  found = cmdText.find("replayIMGImport");
  if (found != std::string::npos) {
     replayImpCommand = "replayIMGImport";
     return true;
  }

  return false;
}

bool
SecondoInterfaceREPLAY::getReplayImportPrmList(std::vector<string>& paramlist, 
                                               const string& cmdText) 
{
/* 
Split the params of special import commands to an array

*/

  std::size_t foundleft; 
  std::size_t foundright;
  foundleft = cmdText.find("(");
  foundright = cmdText.find(")");
  string params = cmdText.substr(foundleft + 1, foundright - foundleft - 1);

  stringutils::StringTokenizer token(params, ",");
  while (token.hasNextToken()) {
      paramlist.push_back(token.nextToken());
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

unsigned int
SecondoInterfaceREPLAY::getMaxCoresFromNodes() {
/*
Get the maximum number of cores from all nodes

*/

  unsigned int maxCores = 0;
  unsigned int curCores = 0;

  for (unsigned int i=0; i<nodes.size(); ++i) {
    try {
      curCores = stoi(nodes[i].cores);
    } catch (const exception& e) {
      curCores = 0;
    }

    if (maxCores < curCores) {
      maxCores = curCores;
    }
  }
  return maxCores;
}  

unsigned int
SecondoInterfaceREPLAY::getSumAllCoresFromNodes() {
/*
Get the sum of all cores from all nodes

*/

  unsigned int sumCores = 0;
  unsigned int curCores = 0;

  for (unsigned int i=0; i<nodes.size(); ++i) {
    try {
      curCores = stoi(nodes[i].cores);
    } catch (const exception& e) {
      curCores = 0;
    }

    sumCores += curCores;
  }
  return sumCores;
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
SecondoInterfaceREPLAY::Initialize(const string& user,
                                   const string& pswd,
                                   const string& host,
                                   const string& port,
                                   string& parmFile,
                                   string& errorMsg,
                                   const bool multiUser)
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
        
      // Can overwritten with call parameter
      replayImportMode = SmiProfile::GetParameter("Replay", "ReplayImportMode",
                                                   "", parmFile);
      cout << "Default Replay Import mode: " << replayImportMode;
    }
  }

  return (initialized);
}

bool 
SecondoInterfaceREPLAY::sendCommandToNode(const unsigned int nodeNo, 
                                          const int commandType,
                                          const string& cmdText) {
/*
Send a secondo command to the node

*/

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

bool
SecondoInterfaceREPLAY::sendAllCommandsToNode(const unsigned int nodeNo,
                                              std::vector<string> commandList) {
/* 
Send the commands from commandList sequential to the node. At the moment
no parallel execution on the nodes itself of the commands.

*/

  int commandType = 1;

  for (unsigned int i=0; i < commandList.size(); ++i) {
    sendCommandToNode(nodeNo, commandType, commandList[i]);
  }

  return true;
}

bool
SecondoInterfaceREPLAY::sendFileToNode(const unsigned int nodeNo,
                                       const string& localfilename,
                                       const string& serverFileName,
                                       const bool allowOverwrite ){
/*
Send a file from local filesystem of the master to an node

*/

   if(localfilename.empty() || serverFileName.empty()){
        return ERR_INVALID_FILE_NAME;
   }

   iostream& iosocknode = nodes[nodeNo].server->GetSocketStream();

   iosocknode << nodes[nodeNo].csp->startFileTransfer << endl;
   iosocknode << serverFileName << endl;
   string allow = allowOverwrite?"<ALLOW_OVERWRITE>":"<DISALLOW_OVERWRITE>";
   iosocknode << allow << endl;
   iosocknode.flush();

   string line;
   getline(iosocknode,line);

   if(line == "<SecondoError>"){
      int errCode = ERR_FILE_EXISTS;
      iosocknode >> errCode;
      nodes[nodeNo].csp->skipRestOfLine();
      getline(iosocknode,line);

      if(line != "</SecondoError>"){

        return ERR_IN_SECONDO_PROTOCOL;
      }

      return errCode;
   }

   if(line != "<SecondoOK>"){
     return ERR_IN_SECONDO_PROTOCOL;
   }

   nodes[nodeNo].csp->SendFile(localfilename);
   iosocknode << nodes[nodeNo].csp->endFileTransfer << endl;
   ListExpr resultList;
   int errorCode; 
   int errorPos;
   string errorMessage;
   errorCode = nodes[nodeNo].csp->ReadResponse( resultList,
                                  errorCode, errorPos,
                                  errorMessage         );
   // return errorCode;
   return true;
}

bool
SecondoInterfaceREPLAY::sendAllFilesToNode(const unsigned int nodeNo, 
                                           const unsigned int startWithFileNo,
                                           const unsigned int noSplitFiles, 
                                           const string basePath, 
                                           const string filePrefix) {
/*
Send all files of an splitting process to an Node. There is the
possibility to send all files to all nodes (replication) or send
a number of files to every node.

*/

  string transferFileName;
  string destFileName;

  for (unsigned int i = startWithFileNo; 
       i < startWithFileNo + noSplitFiles; ++i) {
    transferFileName = basePath + "/" + filePrefix + stringutils::int2str(i);
    destFileName = filePrefix + stringutils::int2str(i);

    sendFileToNode(nodeNo, transferFileName, destFileName, true);

    // no special tread handling needed, because the threads only fill 
    // a new element into the array
    transferFilePath.push_back(stringutils::int2str(nodeNo) + 
                               ":" + 
                               getNodeLastSendFilePath(nodeNo) + 
                               "/" + 
                               filePrefix + 
                               stringutils::int2str(i)
                              );
  }

  return true;
}

bool 
SecondoInterfaceREPLAY::sendAllShapesToNode(const unsigned int nodeNo, 
                                            const unsigned int startWithFileNo,
                                            const unsigned int noSplitFiles, 
                                            const string basePath, 
                                            const string filePrefix) {
/*
Send all shapre files (shp+dbf) of an splitting process to an Node.
There is the possibility to send all files to all nodes (replication) 
or send a number of files to every node.

*/

  string transferFileName;
  string destFileName;

  for (unsigned int i = startWithFileNo; 
       i < startWithFileNo + noSplitFiles; ++i) {
    transferFileName = basePath + "/" + filePrefix + stringutils::int2str(i);
    destFileName = filePrefix + stringutils::int2str(i);

    sendFileToNode(nodeNo, transferFileName + ".shp", 
                   destFileName + ".shp", true);
    sendFileToNode(nodeNo, transferFileName + ".dbf", 
                   destFileName + ".dbf", true);

    // no special tread handling needed, because the threads only fill 
    // a new element into the array
    transferFilePath.push_back(stringutils::int2str(nodeNo) + 
                               ":" + 
                               getNodeLastSendFilePath(nodeNo) + 
                               "/" + 
                               filePrefix + 
                               stringutils::int2str(i) +
                               ".shp"
                              );
    transferFilePath.push_back(stringutils::int2str(nodeNo) + 
                               ":" + 
                               getNodeLastSendFilePath(nodeNo) + 
                               "/" + 
                               filePrefix + 
                               stringutils::int2str(i) +
                               ".dbf"
                              );

  }

  return true;
}




bool 
SecondoInterfaceREPLAY::sendAllImagesToNode(const unsigned int nodeNo,
                                            std::vector<string> imageList) {
/* 
Send all images from search result to the node

*/
  std::size_t found;
  string serverFileName;

  for (unsigned int i = 0; i < imageList.size(); ++i) {
     found = imageList[i].rfind("/");
     serverFileName = imageList[i].substr(found + 1);
     sendFileToNode(nodeNo, imageList[i], serverFileName, true);

    // no special tread handling needed, because the threads only fill 
    // a new element into the array
    transferFilePath.push_back(stringutils::int2str(nodeNo) + 
                               ":" + 
                               getNodeLastSendFilePath(nodeNo) + 
                               "/" + 
                               serverFileName
                              );

  }
  
  return true;
}

string
SecondoInterfaceREPLAY::getNodeLastSendFilePath(const unsigned int nodeNo) {
/*
Get the last send file path from sending an file to an node

*/
  iostream& iosocknode = nodes[nodeNo].server->GetSocketStream();
  iosocknode << "<SEND_FILE_PATH>" << endl;
  iosocknode.flush();
  string line;
  getline(iosocknode,line);
  return line;
}

bool 
SecondoInterfaceREPLAY::controllerTransferFile(const unsigned int noSplitFiles,
                                               const string& subFileName) {
/*
Controls sending of files to the nodes. The function can differentiate 
between "Replication" - all files on all nodes - and partitioning, send only 
part of the file to an node, so that every node has different files.

*/
  bool futureRes;
  vector<std::future<bool>> futures;

  if (replayImportMode == "Replication") {
    for (unsigned int i=0; i<nodes.size(); ++i) {
      futures.push_back(async(std::launch::async,
                      &SecondoInterfaceREPLAY::sendAllFilesToNode,
                      this, i, 0, noSplitFiles,
                      FileSystem::GetCurrentFolder(), 
                      subFileName + "_"));
    }
  } else {
    int startWithFileNo = 0;
    int partNoOfSplitFiles;
    for (unsigned int i=0; i<nodes.size(); ++i) {
      partNoOfSplitFiles = stoi(nodes[i].cores);       
      futures.push_back(async(std::launch::async,
                        &SecondoInterfaceREPLAY::sendAllFilesToNode,
                        this, i, startWithFileNo, partNoOfSplitFiles,
                        FileSystem::GetCurrentFolder(),
                        subFileName + "_"));
      startWithFileNo += partNoOfSplitFiles;
    }
  }

 // wait until all files send to all nodes
  for (unsigned int i=0; i<futures.size(); ++i) {
    try {
      futureRes = futures[i].get();
      if (futureRes == false) {
        cout << "Error while sending file to node : " 
             << nodes[i].hostname << endl;
      }
    } catch (const exception& e) {
      cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what() 
           << " for sending file to node: " << nodes[i].hostname
           << "with ip:" << nodes[i].ip;
    }
  }

  return true;
}

bool 
SecondoInterfaceREPLAY::controllerTransferShapeFile(
                            const unsigned int noSplitFiles,
                            const string& subFileName) {
/*
Controls sending of files (ShapeFiles) to the nodes. The function can 
differentiate between "Replication" - all files on all nodes - and 
partitioning, send only part of the file to an node, so that every node
 has different files.

*/

  bool futureRes;
  vector<std::future<bool>> futures;

  if (replayImportMode == "Replication") {
    for (unsigned int i=0; i<nodes.size(); ++i) {
      futures.push_back(async(std::launch::async,
                      &SecondoInterfaceREPLAY::sendAllShapesToNode,
                      this, i, 0, noSplitFiles,
                      FileSystem::GetCurrentFolder(), 
                      subFileName + "_"));
    }
  } else {
    int startWithFileNo = 0;
    int partNoOfSplitFiles;
    for (unsigned int i=0; i<nodes.size(); ++i) {
      partNoOfSplitFiles = stoi(nodes[i].cores);       
      futures.push_back(async(std::launch::async,
                        &SecondoInterfaceREPLAY::sendAllShapesToNode,
                        this, i, startWithFileNo, partNoOfSplitFiles,
                        FileSystem::GetCurrentFolder(),
                        subFileName + "_"));
      startWithFileNo += partNoOfSplitFiles;
    }
  }

  // wait until all files send to all nodes
  for (unsigned int i=0; i<futures.size(); ++i) {
    try {
      futureRes = futures[i].get();
      if (futureRes == false) {
        cout << "Error while sending file to node : " 
             << nodes[i].hostname << endl;
      }
    } catch (const exception& e) {
      cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what() 
           << " for importing file on node: " << nodes[i].hostname
           << "with ip:" << nodes[i].ip;
    }
  }

  return true;
}


bool 
SecondoInterfaceREPLAY::executeReplayOsmImport(std::vector<string>& paramlist,
                                             const unsigned int noSplitFiles) {
/*
Execute of ReplayOsmImport.

1) Split an OSM-File
2) Transfer the splitted OSM-Files to the nodes
3) Import these files on the nodes
4) Garbage Collection for temporary objects

*/
  iostream& iosock = server->GetSocketStream();
  string cmdText="";
  ListExpr resultList   = nl->TheEmptyList();
  int errorCode;
  int errorPos;
  string errorMessage;
  string subFileName;
  string prefix;
  int commandType = 1;

  // if filename length lower then 3 characters, use default value
  // for subFileName
  if (paramlist[0].size() < 3) {
    subFileName = "osm";
    prefix = "OSM";
  } else {
    // find position of filename
    std::size_t found = paramlist[0].rfind("/");

    // use first 3 characters of import filename for subFileName
    // and prefix
    if (found!=std::string::npos) {
      subFileName = paramlist[0].substr(found + 1, 3);
    } else {
      subFileName = paramlist[0].substr(0, 3);
    }

    // use lower characters for subFileName
    std::transform(subFileName.begin(), subFileName.end(), 
                   subFileName.begin(), ::tolower);

    // use upper characters for prefix 
    prefix = subFileName;
    std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);

  }

  // Split of full OSM file (on master)
  cout << endl << "Splitting OSM file..." << endl;
  cmdText = "query divide_osm('" + paramlist[0] + "',\"" + 
        subFileName + "\", " + 
        stringutils::int2str(noSplitFiles) + ", \"" + 
        prefix + "\")";
  
  iosock << "<Secondo>" << endl
         << commandType << endl
         << cmdText << endl
         << "</Secondo>" << endl;
 
  // Receive result
  errorCode = csp->ReadResponse( resultList,
                                 errorCode, errorPos,
                                 errorMessage         );

  cout << errorCode << endl;

  // Transfer files to nodes
  cout << "Transfer files to nodes ..." << endl;
  controllerTransferFile(noSplitFiles, subFileName);

  // Full OSM Import on nodes
  cout << "Import files on nodes ..." << endl;
  unsigned int currentNode;
  string currentFilePath;

  // Initialize vector for saving import files per node
  vector< vector<string> > commandsPerNode;

  for (unsigned int i=0; i < nodes.size(); ++i) {
    commandsPerNode.push_back(vector <string>());
  }

  // Now add the commands for the nodes to the array
  for (unsigned int i=0; i<transferFilePath.size(); ++i) {
     stringutils::StringTokenizer token(transferFilePath[i], ":");
     currentNode = stoi(token.nextToken());
     currentFilePath = token.nextToken();
     cmdText = "query fullosmimport('" + currentFilePath + "',\"" 
               + subFileName + stringutils::int2str(i) +"\")";
     commandsPerNode[currentNode].push_back(cmdText);
  }

  // Now send array element with commands to every node
  bool futureRes;
  vector<std::future<bool>> futures;

  for (unsigned int i=0; i < nodes.size(); ++i) {
    futures.push_back(async(std::launch::async,
                    &SecondoInterfaceREPLAY::sendAllCommandsToNode,
                    this, i, commandsPerNode[i]));
  }

  for (unsigned int i=0; i<futures.size(); ++i) {
    try {
      futureRes = futures[i].get();
      if (futureRes == false) {
        cout << "Error while importing file on node : "
             << nodes[i].hostname << endl;
      }
    } catch (const exception& e) {
      cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what()
           << " for sending file to node: " << nodes[i].hostname
           << "with ip:" << nodes[i].ip;
    }
  }

  // Remove filelocations from transferFilePath for current instance
  transferFilePath.clear();

  // cleanup system from temp data (splitted files)
  cout << "Garbage collection from OSM-Import..." << endl;
  string filename;
  for (unsigned int i=0; i<noSplitFiles; ++i) {
    filename = FileSystem::GetCurrentFolder() + "/" 
               + subFileName + "_" + stringutils::int2str(i);
    if (FileSystem::FileOrFolderExists(filename)) {
      FileSystem::DeleteFileOrFolder(filename);
    }
  }

  // added types from divide_osm, must be deleted after splitting
  // delete typename
  // prefixNodes_type, prefixNodeTags_type
  // prefixWays_type, prefixWayTags_type
  // prefixRelations_type, prefixRelationTags_type
  array<string, 6> delTypes = {"Nodes_type", "NodeTags_type", "Ways_type",
                      "WayTags_type", "Relations_type",
                      "RelationTags_type"};

  for (unsigned int i = 0; i < delTypes.size(); ++i) {
    cmdText = "delete " + prefix + delTypes[i];
    cout << cmdText << endl;

    iosock << "<Secondo>" << endl
           << commandType << endl
           << cmdText << endl
           << "</Secondo>" << endl;
  }

  return true;

}

bool
SecondoInterfaceREPLAY::splitCSV(const std::string& filename,
                                 const unsigned int headersize,
                                 const bool multiline,
                                 const std::string& subFileName,
                                 const unsigned int noSplitFiles) {
/*
Split a csv file in noSplitFiles files

*/
  string line;
  ifstream aFile(filename);
  std::size_t lines_count = 0;
  std::size_t lines_per_file = 0;
  std::size_t current_line = 0;
  vector<string> headerlines;

  // count total lines in file without header
  lines_count=std::count(std::istreambuf_iterator<char>(aFile), 
                         std::istreambuf_iterator<char>(), '\n');
  lines_per_file = (lines_count - headersize) / noSplitFiles;

  ofstream outputFile;
  aFile.seekg (0, ios::beg);

  // get header from csv file
  for (unsigned int i = 0; i < headersize; ++i) {
    if (aFile.is_open()) {
      getline(aFile,line);
      headerlines.push_back(line);
    }
  }  

  // split file in noSplitFiles
  for (unsigned int i = 0; i <= noSplitFiles; ++i) {
    outputFile.open(subFileName + "_" + stringutils::int2str(i));
    current_line = 0;
    if (aFile.is_open()) {
      // write header
      for (unsigned int he=0; he < headerlines.size(); he++) {
        if (outputFile.is_open()) {
          outputFile << headerlines[he] << '\n';
        }
      }

      // write datasets in files
      int mode = 0;

      while ( current_line < lines_per_file && getline (aFile,line) ) {
        if (outputFile.is_open()) {
          outputFile << line << '\n';
          
          if (multiline) {
            do {
              for (unsigned int lc=0; lc < line.length(); ++lc) {
                if (line[lc] == '"') {
                  if (mode == 1) {
                    mode = 0;
                  } else {
                    mode = 1;
                  }
                }
               }
               if (mode == 1) {
                 getline(aFile,line);
                 outputFile << line << '\n';
                 ++current_line;
               }
            } while (mode == 1); 
          }
          ++current_line;
        } else {
          cout << "Split CSV: Unable to open output file";
          return false;
        }
      }
    } else {
      cout << "Split CSV: Unable to open file";
      return false;
    }
    outputFile.close(); 
  } 
  aFile.close(); 
  return true;    
}

bool
SecondoInterfaceREPLAY::executeReplayCSVImport(std::vector<string>& paramlist,
                                             const unsigned int noSplitFiles) {
/*
Execute of ReplayCSVImport.

1) Split an CSV-File
2) Transfer the splitted CSV-Files to the nodes
3) Import these files on the nodes
4) Garbage Collection for temporary objects

*/

  string cmdText="";
  string subFileName;

  // if filename length lower then 3 characters, use default value
  // for subFileName
  if (paramlist[0].size() < 3) {
    subFileName = "csv";
  } else {
    // find position of filename
    std::size_t found = paramlist[0].rfind("/");

    // use first 3 characters of import filename for subFileName
    if (found!=std::string::npos) {
      subFileName = paramlist[0].substr(found + 1, 3);
    } else {
      subFileName = paramlist[0].substr(0, 3);
    }

    // use lower characters for subFileName
    std::transform(subFileName.begin(), subFileName.end(), 
                   subFileName.begin(), ::tolower);
  }

  // Split of full CSV file (on master)
  cout << endl << "Splitting CSV file ..." << endl;

  bool b;
  istringstream(paramlist[5]) >> std::boolalpha >> b;

  splitCSV(paramlist[0], stoi(paramlist[1]), b, subFileName, noSplitFiles);

  // Transfer files to nodes
  cout << "Transfer files to nodes ..." << endl;
  controllerTransferFile(noSplitFiles, subFileName);

  // CSV Import on nodes
  // Params: filename, headersize, comment, separator, uses_quotes, multiline
  cout << "Import files on nodes ..." << endl;

  unsigned int currentNode;
  string currentFilePath;

  // Initialize vector for saving import files per node
  vector< vector<string> > commandsPerNode;
  for (unsigned int i=0; i < nodes.size(); ++i) {
    commandsPerNode.push_back(vector <string>());  
  }

  // Now add the commands for the nodes to the array
  for (unsigned int i=0; i<transferFilePath.size(); ++i) {
     stringutils::StringTokenizer token(transferFilePath[i], ":");
     currentNode = stoi(token.nextToken());
     currentFilePath = token.nextToken();
     cmdText = "query csvimport2('" + currentFilePath + "', " + paramlist[1] + 
               ",\"" + paramlist[2] +"\",\"" + paramlist[3] +"\"," + 
               paramlist[4] + "," + paramlist[5] + ") count";

     commandsPerNode[currentNode].push_back(cmdText);
  }  

  // Now send array element with commands to every node
  bool futureRes;
  vector<std::future<bool>> futures;

  for (unsigned int i=0; i < nodes.size(); ++i) {
     futures.push_back(async(std::launch::async,
                     &SecondoInterfaceREPLAY::sendAllCommandsToNode,
                     this, i, commandsPerNode[i]));
  }

  for (unsigned int i=0; i<futures.size(); ++i) {
    try {
      futureRes = futures[i].get();
      if (futureRes == false) {
        cout << "Error while importing file on node : " 
             << nodes[i].hostname << endl;
      }
    } catch (const exception& e) {
      cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what() 
           << " for sending file to node: " << nodes[i].hostname
           << "with ip:" << nodes[i].ip;
    }
  }

  cout << "Garbage collection from CSV-Import..." << endl;

  // Remove filelocations from transferFilePath for current instance
  transferFilePath.clear();

  // cleanup system from temp data (splitted files)
  string filename;
  for (unsigned int i=0; i<noSplitFiles; ++i) {
    filename = FileSystem::GetCurrentFolder() + "/" 
               + subFileName + "_" + stringutils::int2str(i);
    if (FileSystem::FileOrFolderExists(filename)) {
      FileSystem::DeleteFileOrFolder(filename);
    }
  }

  return true;
}

bool 
SecondoInterfaceREPLAY::splitSHP(const std::string& filename,
                                 const std::string& subFileName,
                                 const unsigned int noSplitFiles) {
/*
Split an shape file in noSplitFiles files (shp+dbf-File)

*/

  // Split of the Shape-File (shp)
  const unsigned int shpHeaderSize = 100;

  string errorMessage;
  unsigned int bytesPerFile;

  ofstream outfile;
  fstream inoutfile;

  char* header = new char[shpHeaderSize];
  unsigned int writeBytes;
  uint32_t recnumHeader;
  uint32_t recnum;
  uint32_t contLength;
  uint32_t contLengthBin;
  vector<unsigned int> recordsNode;

  ifstream file(filename, ios::binary|ios::in);

  if(!file.good()){
     errorMessage = "problem in reading shp file";
     cout << errorMessage << endl;
     return false;
  }

  file.seekg(0,ios::end);
  std::size_t flen = file.tellg();
  if(flen < shpHeaderSize){
     errorMessage = "not a valid shape file";
     cout << errorMessage << endl;
     file.close();
     return false;
  }

  bytesPerFile = (flen - shpHeaderSize) / noSplitFiles;

  file.seekg(0,ios::beg);

  // split file in noSplitFiles
  for (unsigned int i = 0; i < noSplitFiles; ++i) {
    writeBytes = 0;
    recnum = 1;
    outfile.open(subFileName + "_" + stringutils::int2str(i) + ".shp",
                  ios::binary|ios::out);
  
    if (i == 0) {
      file.read(header, shpHeaderSize);
    }
    outfile.write(header, shpHeaderSize);

    do {

      file.read(reinterpret_cast<char*>(&recnumHeader), 4);
      file.read(reinterpret_cast<char*>(&contLength), 4);
      contLengthBin = contLength;
      if (!file.eof()) { 
        if (WinUnix::isLittleEndian()){
          contLength = WinUnix::convertEndian(contLength) * 2;
          recnumHeader = WinUnix::convertEndian(recnum);
        } else {
          contLength = contLength * 2;
        }
        // recnum begins with 1 in every splitted file, so recnum
        // has to change
        outfile.write(reinterpret_cast<char*>(&recnumHeader), 4);
        outfile.write(reinterpret_cast<char*>(&contLengthBin), 4);
      }
      writeBytes += 8;

      char* dataset = new char[contLength];
      file.read(dataset, contLength);
      if (!file.eof()) { 
        outfile.write(dataset, contLength);
        ++recnum;
      }
      writeBytes += contLength;

      delete[] dataset;
    } while (!file.eof() and writeBytes <= bytesPerFile);
    outfile.close();
    // save the number of records in splitted file, in dbase
    // file there must be the same order
    recordsNode.push_back(recnum - 1);
  }

  file.close();
  delete[] header;

  // change File Length in all Fileheader of splitted files
  // to the correct size
  uint32_t fileLength;
  for (unsigned int i = 0; i < noSplitFiles; ++i) {
    inoutfile.open(subFileName + "_" + stringutils::int2str(i), 
                   ios::binary|ios::in|ios::out);
    inoutfile.seekg(0,ios::end);
    fileLength = inoutfile.tellg() / 2;
    if (WinUnix::isLittleEndian()){
      fileLength = WinUnix::convertEndian(fileLength);
    }
    inoutfile.seekg(24,ios::beg);    
    inoutfile.write(reinterpret_cast<char*>(&fileLength), 4);
    inoutfile.close();
  }

  // Split of the dBASE-File (dbf)
  uint32_t noRecords_dbf;
  uint16_t headerLength_dbf;
  uint16_t recordSize_dbf;
  uint16_t noAttributes_dbf;

  string dbfFileName = filename.substr(0, filename.rfind(".shp")) + ".dbf";

  ifstream file_dbf(dbfFileName, ios::binary|ios::in); 

  file_dbf.seekg(0,ios::beg);
  unsigned char code;
  file_dbf.read(reinterpret_cast<char*>(&code),1);
 
  if(!file_dbf.good() || ( (code!=3) && (code!=0x83) )) {
     errorMessage = "problem in reading dbf file";
     cout << errorMessage << endl;
     return false;
  }

  file_dbf.seekg(4,ios::beg);
  file_dbf.read(reinterpret_cast<char*>(&noRecords_dbf),4);
  file_dbf.read(reinterpret_cast<char*>(&headerLength_dbf),2);
  file_dbf.read(reinterpret_cast<char*>(&recordSize_dbf),2);

  if(!WinUnix::isLittleEndian()) {
    noRecords_dbf = WinUnix::convertEndian(noRecords_dbf);
    headerLength_dbf = WinUnix::convertEndian(headerLength_dbf);
    recordSize_dbf = WinUnix::convertEndian(recordSize_dbf);
  }

  if ( (headerLength_dbf-1) % 32  != 0) {
    errorMessage = "invalid length for the header (" 
                   + stringutils::int2str(headerLength_dbf) + ")";
    cout << errorMessage << endl;
    file_dbf.close();
    return false;
  }

  noAttributes_dbf = (headerLength_dbf - 32) / 32;
  if (noAttributes_dbf < 1) {
    errorMessage = "numer of attributes invalid(" 
                   + stringutils::int2str(noAttributes_dbf) + ")";
    cout << errorMessage << endl;
    file_dbf.close();
    return false;
  }

  char* header_dbf = new char[headerLength_dbf];
  for (unsigned int i = 0; i < recordsNode.size(); ++i) {
    ofstream outfile_dbf;
    fstream inoutfile_dbf;

    outfile_dbf.open(subFileName + "_" + stringutils::int2str(i) + ".dbf",
                      ios::binary|ios::out);

    // read header and store it in local variable, nedded in every file
    if (i == 0) {
      file_dbf.seekg(0,ios::beg);
      file_dbf.read(header_dbf, headerLength_dbf);
    }
    outfile_dbf.write(header_dbf, headerLength_dbf);

    // get the same records as in correspending shp-File
    for (unsigned int j = 0; j < recordsNode[i]; ++j) {
      char* record_dbf = new char[recordSize_dbf];
      file_dbf.read(record_dbf, recordSize_dbf);
      outfile_dbf.write(record_dbf, recordSize_dbf);
      delete[] record_dbf;
    }

    outfile_dbf.close();
  }

  // change recordSize to the new value in every file
  fstream inoutfile_dbf;
  for (unsigned int i = 0; i < recordsNode.size(); ++i) {

    inoutfile_dbf.open(subFileName + "_" + stringutils::int2str(i) + ".dbf",
                        ios::binary|ios::in|ios::out);
    noRecords_dbf = recordsNode[i];

    if (!WinUnix::isLittleEndian()) {
      noRecords_dbf = WinUnix::convertEndian(noRecords_dbf);
    }
    inoutfile_dbf.seekg(4,ios::beg);    
    inoutfile_dbf.write(reinterpret_cast<char*>(&noRecords_dbf), 4);
    inoutfile_dbf.close();
  } 

  return true;
}

bool
SecondoInterfaceREPLAY::executeReplaySHPImport(std::vector<string>& paramlist,
                                             const unsigned int noSplitFiles) {
/*
Execute of ReplaySHPImport.

1) Split an SHP-File/DBF-File
2) Transfer the splitted SHP/DBF-Files to the nodes
3) Import these files on the nodes
4) Garbage Collection for temporary objects

*/

  string cmdText="";
  string subFileName;

  // if filename length lower then 3 characters, use default value
  // for subFileName
  if (paramlist[0].size() < 3) {
    subFileName = "shp";
  } else {
    // find position of filename
    std::size_t found = paramlist[0].rfind("/");

    // use first 3 characters of import filename for subFileName
    if (found!=std::string::npos) {
      subFileName = paramlist[0].substr(found + 1, 3);
    } else {
      subFileName = paramlist[0].substr(0, 3);
    }

    // use lower characters for subFileName
    std::transform(subFileName.begin(), subFileName.end(), 
                   subFileName.begin(), ::tolower);
  }

  // Split of full SHP file (on master)
  cout << endl << "Splitting SHP file..." << endl;
  splitSHP(paramlist[0], subFileName, noSplitFiles);

  // Transfer files to nodes
  cout << "Transfer files to nodes ..." << endl;
  controllerTransferShapeFile(noSplitFiles, subFileName);

  // Full SHP+DBF Import on nodes
  cout << "Import files on nodes ..." << endl;
  unsigned int currentNode;
  string currentFilePath;
  string currentType;

  // Initialize vector for saving import files per node
  vector< vector<string> > commandsPerNode;

  for (unsigned int i=0; i < nodes.size(); ++i) {
    commandsPerNode.push_back(vector <string>());
  }

  // Now add the commands for the nodes to the array
  for (unsigned int i=0; i<transferFilePath.size(); ++i) {
     stringutils::StringTokenizer token(transferFilePath[i], ":");
     currentNode = stoi(token.nextToken());
     currentFilePath = token.nextToken();
     currentType = currentFilePath.substr(currentFilePath.size() - 3);
     if (currentType == "dbf") {
       cmdText = "query dbimport2('" + currentFilePath + "') count";
     } else {
       cmdText = "query shpimport2('" + currentFilePath + "') count";
     }

     commandsPerNode[currentNode].push_back(cmdText);
  }

  // Now send array element with commands to every node
  bool futureRes;
  vector<std::future<bool>> futures;

  for (unsigned int i=0; i < nodes.size(); ++i) {
    futures.push_back(async(std::launch::async,
                    &SecondoInterfaceREPLAY::sendAllCommandsToNode,
                    this, i, commandsPerNode[i]));
  }

  for (unsigned int i=0; i<futures.size(); ++i) {
    try {
      futureRes = futures[i].get();
      if (futureRes == false) {
        cout << "Error while importing file on node : "
             << nodes[i].hostname << endl;
      }
    } catch (const exception& e) {
      cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what()
           << " for importing file on node: " << nodes[i].hostname
           << "with ip:" << nodes[i].ip;
    }
  }
  
  cout << "Garbage collection from SHP-Import..." << endl;

  // Remove filelocations from transferFilePath for current instance
  transferFilePath.clear();

  // cleanup system from temp data (splitted files)

  string filename;
  for (unsigned int i=0; i<noSplitFiles; ++i) {
    filename = FileSystem::GetCurrentFolder() + "/" + subFileName 
               + "_" + stringutils::int2str(i) + ".shp";
    if (FileSystem::FileOrFolderExists(filename)) {
      FileSystem::DeleteFileOrFolder(filename);
    }
    filename = FileSystem::GetCurrentFolder() + "/" + subFileName + "_" 
               + stringutils::int2str(i) + ".dbf";
    if (FileSystem::FileOrFolderExists(filename)) {
      FileSystem::DeleteFileOrFolder(filename);
    }
  }

  return true;
}

bool
SecondoInterfaceREPLAY::splitDBLP(const std::string& filename, 
                                 const std::string& subFileName,
                                 const unsigned int noSplitFiles) {
/*
Split an dblp file in noSplitFiles files

*/

  string line;
  std::size_t bytes_per_file = 0;
  vector<string> headerlines;

  ofstream outputFile;
  
  ifstream aFile(filename, ios::ate);
  bytes_per_file = aFile.tellg() / noSplitFiles;
  aFile.seekg (0, ios::beg);

  // get header from dblp file
  while (line != "<dblp>") {
    if (aFile.is_open()) {
      getline(aFile,line);
      headerlines.push_back(line);
    }
  }  
  
  // split file in noSplitFiles
  for (unsigned int i = 0; i < noSplitFiles; ++i) {
    outputFile.open(subFileName + "_" + stringutils::int2str(i));

    // write header
    for (unsigned int he=0; he < headerlines.size(); he++) {
      if (outputFile.is_open()) {
        outputFile << headerlines[he] << '\n';
      }
    }

    // write content
    while (aFile.tellg() <= (int) (bytes_per_file * (i + 1))) {
      getline(aFile,line);
      outputFile << line << endl;
    } 

    // write until current tag is closed
    while (line[1] != '/') {
      getline(aFile,line);
      outputFile << line << endl;
    }
 
    // In last file the tag came from original file
    if (i < noSplitFiles - 1) {
      outputFile << "</dblp>" << endl;
    }

    outputFile.close();
  }
  aFile.close(); 

  return true;
}

bool
SecondoInterfaceREPLAY::executeReplayDBLPImport(std::vector<string>& paramlist,
                                            const unsigned int noSplitFiles) {
/*
Execute of ReplayDBLPImport.

1) Split an DBLP-File
2) Transfer the splitted DBLP-Files to the nodes
3) Import these files on the nodes
4) Garbage  Collection for temporary objects

*/

  string cmdText="";
  string subFileName;

  // if filename length lower then 3 characters, use default value
  // for subFileName
  if (paramlist[0].size() < 3) {
    subFileName = "xml";
  } else {
    // find position of filename
    std::size_t found = paramlist[0].rfind("/");

    // use first 3 characters of import filename for subFileName
    if (found!=std::string::npos) {
      subFileName = paramlist[0].substr(found + 1, 3);
    } else {
      subFileName = paramlist[0].substr(0, 3);
    }

    // use lower characters for subFileName
    std::transform(subFileName.begin(), subFileName.end(), 
                   subFileName.begin(), ::tolower);
  }

  // Split of full DBLP file (on master)
  cout << endl << "Splitting DBLP file..." << endl;
  splitDBLP(paramlist[0], subFileName, noSplitFiles);

  // Transfer files to nodes
  // @TODO: Vorher konvertieren in CSV-Format???

  cout << "Transfer files to nodes ..." << endl;
  controllerTransferFile(noSplitFiles, subFileName);

  // Import files on nodes
  // @TODO: Import als CSV-Format??? Konvertieren vor Transfer???
  cout << "Import files on nodes ...TODO..." << endl;

  cout << "Garbage collection from DBLP-Import..." << endl;

  // Remove filelocations from transferFilePath for current instance
  transferFilePath.clear();

  // cleanup system from temp data (splitted files)
  string filename;
  for (unsigned int i=0; i<noSplitFiles; ++i) {
    filename = FileSystem::GetCurrentFolder() + "/" + subFileName + "_" 
               + stringutils::int2str(i);
    if (FileSystem::FileOrFolderExists(filename)) {
      FileSystem::DeleteFileOrFolder(filename);
    }
  }

  // @TODO: Import -> keine Algebra vorhanden, unter tools/converter 
  // gibt es jedoch ein Beispiel auf nicht Algebra Basis. Hier müsste
  //  man prüfen, ob man dies irgendwie verwenden könnte.

  return true;
}

bool
SecondoInterfaceREPLAY::importImgOnNode(const unsigned int nodeNo,
                                        const string relDesc,
                                        const string relName) {
/*
Write the file with the relation object for the node,
transfer it to the node and restore (import) it

*/

  ofstream outputFile;
  string relFileName; 

  relFileName = "relation_" + stringutils::int2str(nodeNo);
  outputFile.open(relFileName);
  outputFile << relDesc << endl;
  outputFile.close();
  sendFileToNode(nodeNo, FileSystem::GetCurrentFolder() + "/" + relFileName,
                  relFileName, true);

  string cmdText = "restore " + relName + " from '" 
                   + getNodeLastSendFilePath(nodeNo) 
                   + "/" + relFileName + "'"; 
 
  sendCommandToNode(nodeNo, 1, cmdText);

  return true;
}

bool
SecondoInterfaceREPLAY::executeReplayIMGImport(std::vector<string>& paramlist) {
/*
Execute of ReplayIMGImport.

1) Get all images of an directory structur with an specified level of recursion
2) Transfer the files of the directory structur to the nodes
3) Import these images on the nodes
4) Garbage Collection for temporary objects

*/
  FilenameList theList;

  unsigned int numFiles;
  unsigned int filesPerNode;
  unsigned int currentFileNo = 0;
  unsigned int startFileNo = 0;
  unsigned int remainder = 0;

  string relObject;

  // Initialize vector for saving files per node
  vector< vector<string> > elementsPerNode;

  // Step 1: Get all images of the directory from the parameter
  cout << endl << "Searching in filepath for images..." << endl;

  FileSystem::FileSearch( paramlist[0],
                          theList,
                          0,
                          stoi(paramlist[1]),
                          false,
                          true,
                          *callBackFileSearch );

  cout << "Transfering images files to the nodes..." << endl;

  // Create a filelist for every node
  if (replayImportMode == "Replication") {
    for (unsigned int i=0; i < nodes.size(); i++) {
      sendAllImagesToNode(i, theList);
    }
  } else {
    numFiles = theList.size();
    filesPerNode = numFiles / nodes.size();
    remainder = numFiles % nodes.size();

    for (unsigned int i=0; i < nodes.size(); ++i) {
      elementsPerNode.push_back(vector <string>());
    }

    for (unsigned int i=0; i < nodes.size(); i++) {
      startFileNo = currentFileNo;
      for (unsigned int j=startFileNo; j < startFileNo + filesPerNode; ++j) {
         elementsPerNode[i].push_back(theList[j]);
         ++currentFileNo;
      }
    }

    // Now send the rest of the files (remainder), one file for every node
    startFileNo = theList.size() - remainder;
    for (unsigned int i=0; i < remainder; ++i) {
         elementsPerNode[i].push_back(theList[startFileNo + i]);
    }

    // Now send array element with files to every node
    bool futureRes;
    vector<std::future<bool>> futures;

    for (unsigned int i=0; i < nodes.size(); ++i) {
       futures.push_back(async(std::launch::async,
                       &SecondoInterfaceREPLAY::sendAllImagesToNode,
                       this, i, elementsPerNode[i]));
    }

    for (unsigned int i=0; i<futures.size(); ++i) {
      try {
        futureRes = futures[i].get();
        if (futureRes == false) {
          cout << "Error while transferring images to node : " 
               << nodes[i].hostname << endl;
        }
      } catch (const exception& e) {
        cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what() 
             << " for sending image to node: " << nodes[i].hostname
             << "with ip:" << nodes[i].ip;
      }
    }
  }

  cout << "Import images on nodes..." << endl;

  unsigned int currentNode;
  string currentFilePath;
  std::size_t found;
  string filename;
  vector<string> relation;

  // create relation file for every node
  for (unsigned int i = 0; i < nodes.size(); i++) {
    relObject = "(OBJECT " + paramlist[2] + "() ";
    relObject = relObject + R"X*X(

      (rel (tuple ( (Filename filepath)(Picture picture ) ))) (
 
    )X*X";

    relation.push_back(relObject);
  }

  // create relation file for every node
  time_t now = time(0);
  tm *ltm = localtime(&now);
  string currentDate = stringutils::int2str(1900 + ltm->tm_year) + "-" + 
                       stringutils::int2str(1 + ltm->tm_mon) + "-" + 
                       stringutils::int2str(ltm->tm_mday);

  for (unsigned int i=0; i<transferFilePath.size(); ++i) {
    stringutils::StringTokenizer token(transferFilePath[i], ":");
    currentNode = stoi(token.nextToken());
    currentFilePath = token.nextToken();
    found = currentFilePath.rfind("/");
    filename = currentFilePath.substr(found + 1);
    relObject = "";
    relObject = relObject + "   ( <text>" + currentFilePath + "</text--->\n";
    relObject = relObject + "     ( \"" + filename + "\" \n";
    relObject = relObject + "       \"" + currentDate + "\" \n";
    relObject = relObject + "       \"unknown\"\n";
    relObject = relObject + "       TRUE \n";
    relObject = relObject + "       <file>" + currentFilePath 
                + "</file--->) \n";
    relObject = relObject + "   ) \n";
    relation[currentNode] = relation[currentNode] + relObject; 
  }

  for (unsigned int i = 0; i < nodes.size(); i++) {
    relObject = relation[i];
    relObject = relObject + R"X*X(
    ))
    )X*X";
    relation[i] = relObject;
  }

  // Now write the file with the relation object for every node,
  // transfer it to the node and restore (import) it
  bool futureRes;
  vector<std::future<bool>> futures;

  for (unsigned int i = 0; i < nodes.size(); i++) {
    futures.push_back(async(std::launch::async,
                    &SecondoInterfaceREPLAY::importImgOnNode,
                    this, i, relation[i], paramlist[2]));
  }

  for (unsigned int i=0; i<futures.size(); ++i) {
    try {
      futureRes = futures[i].get();
      if (futureRes == false) {
        cout << "Error while importing file on node : "
             << nodes[i].hostname << endl;
      }
    } catch (const exception& e) {
      cout << "SYSTEM ERROR FOR STARTING/FINISHING THREAD: " << e.what()
           << " for importing file on node: " << nodes[i].hostname
           << "with ip:" << nodes[i].ip;
    }
  }

  // Remove filelocations from transferFilePath for current instance
  transferFilePath.clear();

  // cleanup system from temp data (relation description for every node)
  // no deletion of images
  string relFileName;
  for (unsigned int i = 0; i < nodes.size(); i++) {
    relFileName = "relation_" + stringutils::int2str(i);
    filename = FileSystem::GetCurrentFolder() + "/" + relFileName;

    if (FileSystem::FileOrFolderExists(filename)) {
      FileSystem::DeleteFileOrFolder(filename);
    }
  }

  return true;
}


void
SecondoInterfaceREPLAY::Secondo(const string& commandText,
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
  bool onlyMasterCommand = true;
  bool isReplayImportCommand = false;
  int noSplitFiles; 
  bool futureRes;
  string replayImpCommand;
  std::vector<string> paramlist;
  bool paramsOK;

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

    // Check for Replay Import Command
    isReplayImportCommand = checkReplayImport(cmdText, replayImpCommand);

    if (isReplayImportCommand == true) {
      // At the moment supported formats for Replay Import:
      // Open Street Map (osm)
      // CSV
      // Shape-Files (shp)
      // DBLP
      // Directory tree with images

      // Commands:
      // If import do replication or partitioning on the nodes is specified
      // in SecondoConfig.ini (ReplayModus)
      // Can overwritten with every replayImportCommand with an optional last
      //  parameter
      // Parameter value: Replication or Partitioning

      // replayOSMImport(filename[,replayImportMode])
      // replayCSVImport(filename, headersize, comment, separator, uses_quotes, 
      //                 multiline[,replayImportMode])
      // replaySHPImport(filename[,replayImportMode])
      // replayDBLPImport(filename[,replayImportMode])
      // replayIMGImport(startDirectory, recursiveLevel, relationName
      //  [,replayImportMode])
       
      // Parse params and save it in param list array
      paramsOK = getReplayImportPrmList(paramlist, cmdText);   

      // Check if user overwriten the default replayImportMode

      if ( (replayImpCommand == "replayOSMImport") ||
           (replayImpCommand == "replaySHPImport") ||
           (replayImpCommand == "replayDBLPImport") 
         ) {
        // Second parameter for replayOSMImport, 
        // replaySHPImport, replayDBLPImport      

        if (paramlist.size() == 2) {
          if ( (paramlist[1] == "Replication") ||
               (paramlist[1] == "Partitioning") ) {

            replayImportMode = paramlist[1];
          }
        }
      } else if (replayImpCommand == "replayIMGImport") {
        // Fourth parameter for replayIMGImport
        if (paramlist.size() == 4) {
          if ( (paramlist[3] == "Replication") ||
               (paramlist[3] == "Partitioning") ) {
            replayImportMode = paramlist[3];
          }
         }
      } else if (replayImpCommand == "replayCSVImport") {
        // Seventh parameter for replayCSVImport
        if (paramlist.size() == 7) {
          if ( (paramlist[6] == "Replication") ||
               (paramlist[6] == "Partitioning") ) {
            replayImportMode = paramlist[6];
          }
        }
      }

      if (replayImportMode == "Replication") {
        // every node gets the number of files of the node
        // with the most cores, so this node can import
        // all files at the same time
        noSplitFiles = getMaxCoresFromNodes();
      } else {
        // every node gets the number of files of his cores
        noSplitFiles = getSumAllCoresFromNodes();
      }

      if (paramsOK) {
        // OSM
        if (replayImpCommand == "replayOSMImport") {
          executeReplayOsmImport(paramlist, noSplitFiles);
        // CSV
        } else if (replayImpCommand == "replayCSVImport") {
          executeReplayCSVImport(paramlist, noSplitFiles);
        // SHP
        } else if (replayImpCommand == "replaySHPImport") {
          executeReplaySHPImport(paramlist, noSplitFiles);
        // DBLP
        } else if (replayImpCommand == "replayDBLPImport") {
          executeReplayDBLPImport(paramlist, noSplitFiles);
        // Images
        } else if (replayImpCommand == "replayIMGImport") {
          executeReplayIMGImport(paramlist);       
        }
      } 
      // After an replyImport no other command will be executed
      cout << "replayImport closed!";
      cmdText = "query \"READY!\"";
   } 

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

