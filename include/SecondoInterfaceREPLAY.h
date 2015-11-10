
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
#include "FileSystem.h"

class SecondoInterfaceREPLAY : public SecondoInterfaceCS{
  
  
public:
  SecondoInterfaceREPLAY(bool isServer = false,
                         NestedList* _nl=0);
  
  virtual ~SecondoInterfaceREPLAY();

  /* expansion stage 1 */
  void setReplayFile(const string iReplayFile);

  virtual bool Initialize(const string& user, 
                          const string& pswd,
                          const string& host,
                          const string& port,
                          string& profile,
                          string& errorMsg,
                          const bool multiUser = false );

  virtual void Terminate();

  /* expansion stage 1+2 */
  virtual void Secondo(const string& commandText,
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

  /* expansion stage 1 */
  bool getExternalConfig(const string &parmFile);

  void showMasterConfig();  
  bool getMasterConfig(const string& parmFile, 
                       const string &delimPart);
  bool getNodesConfig(const string& parmFile, 
                      const string &delimPart,
                      const string &delimFull);
  void showNodesConfig();
  bool connectNode(const unsigned int nodeNo,
                   const string& user,
                   const string& pswd);
  bool connectReplayNodes(const string& user,
                          const string& pswd);
  bool disconnectNode(const unsigned int nodeNo);
  bool disconnectReplayNodes();
  bool sendCommandToNode(const unsigned int nodeNo,
                         const int commandType,
                         const string& cmdText);
  bool sendAllCommandsToNode(const unsigned int nodeNo,
                             std::vector<string> commandList);

  bool checkReplayCommand(string& cmdText);

  /* expansion stage 2 */
  unsigned int getMaxCoresFromNodes();
  unsigned int getSumAllCoresFromNodes();

  bool checkReplayImport(const string& cmdText,
                         string& replayImpCommand);
  bool sendAllFilesToNode(const unsigned int nodeNo, 
                          const unsigned int startWithFileNo,
                          const unsigned int noSplitFiles, 
                          const string basePath, 
                          const string filePrefix);
  bool sendAllShapesToNode(const unsigned int nodeNo, 
                           const unsigned int startWithFileNo,
                           const unsigned int noSplitFiles, 
                           const string basePath, 
                           const string filePrefix);
  bool sendAllDBLPToNode(const unsigned int nodeNo, 
                         const unsigned int startWithFileNo,
                         const unsigned int noSplitFiles, 
                         const string basePath);
  bool sendAllImagesToNode(const unsigned int nodeNo,
                           std::vector<string> imageList);
  bool sendShareFileToNode(const unsigned int nodeNo,
                           const string& localfilename,
                           const string& cpDestPath);
  bool sendFileToNode(const unsigned int nodeNo,
                      const string& localfilename,
                      const string& serverFileName,
                      const bool allowOverwrite);
  bool getReplayImportPrmList(std::vector<string>& paramlist, 
                              const string& cmdText);
  bool checkReplayImportNoParams(const string& replayImpCommand,
                                 std::vector<string>& paramlist);

  string getNodeLastSendFilePath(const unsigned int nodeNo);

  bool controllerTransferFile(const unsigned int noSplitFiles,
                              const string& subFileName);
  bool controllerTransferShapeFile(const unsigned int noSplitFiles,
                                   const string& subFileName);
  bool controllerTransferDBLPFile(const unsigned int noSplitFiles,
                                  const string& subFileName);
  bool executeReplayOsmImport(std::vector<string>& paramlist,
                              const unsigned int noSplitFiles);
  bool executeReplayCSVImport(std::vector<string>& paramlist,
                              const unsigned int noSplitFiles);
  bool executeReplaySHPImport(std::vector<string>& paramlist,
                              const unsigned int noSplitFiles);
  bool executeReplayDBLPImport(std::vector<string>& paramlist,
                               const unsigned int noSplitFiles);
  bool executeReplayIMGImport(std::vector<string>& paramlist);
  bool executeReplayShareFile(std::vector<string>& paramlist);
   
  bool splitCSV(const std::string& filename, 
                const unsigned int headersize,
                const bool multiline,
                const std::string& subFileName,
                const unsigned int noSplitFiles);

  bool splitSHP(const std::string& filename,
                const std::string& subFileName,
                const unsigned int noSplitFiles);

  bool splitDBLP(const std::string& filename, 
                 const std::string& subFileName,
                 const unsigned int noSplitFiles);

  bool DBLPtoSECONDO(const std::string& subFileName,
                     const unsigned int noSplitFiles);

  bool importImgOnNode(const unsigned int nodeNo,
                       const string relDesc,
                       const string relName);

  string importDBLPGetCmdTxt(const string currentObject,
                             const unsigned int currentNo,
                             const string transferPath);

  /* expansion stage 1 */
  string replayFile; // path of external config file

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

  /* expansion stage 2 */
  string replayImportMode;
  std::vector<string> transferFilePath;
};

#endif
