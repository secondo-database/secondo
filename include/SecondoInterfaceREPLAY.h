
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

#include <string>
#include "SecondoInterfaceCS.h"
#include "FileSystem.h"

class SecondoInterfaceREPLAY : public SecondoInterfaceCS{
  
  
public:
  SecondoInterfaceREPLAY(bool isServer = false,
                         NestedList* _nl=0);
  
  virtual ~SecondoInterfaceREPLAY();

  /* expansion stage 1 */
  void setReplayFile(const std::string iReplayFile);

  virtual bool Initialize(const std::string& user, 
                          const std::string& pswd,
                          const std::string& host,
                          const std::string& port,
                          const std::string& profile,
                          const std::string& home,
                          std::string& errorMsg,
                          const bool multiUser = false );

  virtual void Terminate();

  /* expansion stage 1+2 */
  virtual void Secondo(const std::string& commandText,
                       const ListExpr commandLE,
                       const int commandType,
                       const bool commandAsText,
                       const bool resultAsText,
                       ListExpr& resultList,
                       int& errorCode,
                       int& errorPos,
                       std::string& errorMessage,
                       const std::string& resultFileName =
                       "SecondoResult",
                       const bool isApplicationLevelCommand = true);

protected:

  /* expansion stage 1 */
  bool getExternalConfig(const std::string &parmFile);

  void showMasterConfig();  
  bool getMasterConfig(const std::string& parmFile, 
                       const std::string &delimPart);
  bool getNodesConfig(const std::string& parmFile, 
                      const std::string &delimPart,
                      const std::string &delimFull);
  void showNodesConfig();
  bool connectNode(const unsigned int nodeNo,
                   const std::string& user,
                   const std::string& pswd);
  bool connectReplayNodes(const std::string& user,
                          const std::string& pswd);
  bool disconnectNode(const unsigned int nodeNo);
  bool disconnectReplayNodes();
  bool sendCommandToNode(const unsigned int nodeNo,
                         const int commandType,
                         const std::string& cmdText);
  bool sendAllCommandsToNode(const unsigned int nodeNo,
                             std::vector<std::string> commandList);

  bool checkReplayCommand(std::string& cmdText);

  /* expansion stage 2 */
  unsigned int getMaxCoresFromNodes();
  unsigned int getSumAllCoresFromNodes();

  bool checkReplayImport(const std::string& cmdText,
                         std::string& replayImpCommand);
  bool sendAllFilesToNode(const unsigned int nodeNo, 
                          const unsigned int startWithFileNo,
                          const unsigned int noSplitFiles, 
                          const std::string basePath, 
                          const std::string filePrefix);
  bool sendAllShapesToNode(const unsigned int nodeNo, 
                           const unsigned int startWithFileNo,
                           const unsigned int noSplitFiles, 
                           const std::string basePath, 
                           const std::string filePrefix);
  bool sendAllDBLPToNode(const unsigned int nodeNo, 
                         const unsigned int startWithFileNo,
                         const unsigned int noSplitFiles, 
                         const std::string basePath);
  bool sendAllImagesToNode(const unsigned int nodeNo,
                           std::vector<std::string> imageList);
  bool sendShareFileToNode(const unsigned int nodeNo,
                           const std::string& localfilename,
                           const std::string& cpDestPath);
  bool sendFileToNode(const unsigned int nodeNo,
                      const std::string& localfilename,
                      const std::string& serverFileName,
                      const bool allowOverwrite);
  bool getReplayImportPrmList(std::vector<std::string>& paramlist, 
                              const std::string& cmdText);
  bool checkReplayImportNoParams(const std::string& replayImpCommand,
                                 std::vector<std::string>& paramlist);

  std::string getNodeLastSendFilePath(const unsigned int nodeNo);

  bool controllerTransferFile(const unsigned int noSplitFiles,
                              const std::string& subFileName);
  bool controllerTransferShapeFile(const unsigned int noSplitFiles,
                                   const std::string& subFileName);
  bool controllerTransferDBLPFile(const unsigned int noSplitFiles,
                                  const std::string& subFileName);
  bool executeReplayOsmImport(std::vector<std::string>& paramlist,
                              const unsigned int noSplitFiles);
  bool executeReplayCSVImport(std::vector<std::string>& paramlist,
                              const unsigned int noSplitFiles);
  bool executeReplaySHPImport(std::vector<std::string>& paramlist,
                              const unsigned int noSplitFiles);
  bool executeReplayDBLPImport(std::vector<std::string>& paramlist,
                               const unsigned int noSplitFiles);
  bool executeReplayIMGImport(std::vector<std::string>& paramlist);
  bool executeReplayShareFile(std::vector<std::string>& paramlist);
   
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

  bool importAllImgOnNode(const unsigned int nodeNo,
			  std::vector<std::string> imageList,
                          std::vector<int> numberer,
                          const std::string relName);
                          
  bool importImgOnNode(const unsigned int nodeNo,
                       const std::string relDesc,
                       const std::string relName);

  std::string importDBLPGetCmdTxt(const std::string currentObject,
                             const unsigned int currentNo,
                             const std::string transferPath);

  /* expansion stage 1 */
  std::string replayFile; // path of external config file

  struct ReplayHost {
    std::string hostname;
    std::string ip;
    std::string port;
    std::string cores;
    Socket* server;
    NestedList* nl;  
    CSProtocol* csp;
    bool initialized; // state of replay node
  };

  ReplayHost master;
  std::vector<ReplayHost> nodes;

  /* expansion stage 2 */
  std::string replayImportMode;
  std::vector<std::string> transferFilePath;
};

#endif
