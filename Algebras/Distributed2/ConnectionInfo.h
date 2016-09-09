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
#ifndef ALGEBRAS_DISTRIBUTED2_CONNECTIONINFO_H_
#define ALGEBRAS_DISTRIBUTED2_CONNECTIONINFO_H_

#include "semaphore.h"

#include "fsrel.h"
#include "DArray.h"
#include "frel.h"
#include "fobj.h"
#include <iostream>
#include <vector>
#include <list>

#include "SecondoInterface.h"
#include "SecondoInterfaceCS.h"
#include "FileSystem.h"
#include "Algebra.h"
#include "NestedList.h"
#include "StandardTypes.h"
#include "FTextAlgebra.h"
#include "ListUtils.h"
#include "Stream.h"
#include "NList.h"
#include "ArrayAlgebra.h"
#include "SocketIO.h"
#include "StopWatch.h"

#include "LongInt.h"

#include "Bash.h"
#include "DebugWriter.h"

#include "FileRelations.h"
#include "FileAttribute.h"
#include "CommandLog.h"

namespace distributed2
{

/*

0 Class ConnectionInfo

This class represents a connection to a remote Secondo Server.

*/

class ConnectionInfo
{
public:
    ConnectionInfo(const std::string& _host,
                   const int _port,
                   const std::string& _config,
                   SecondoInterfaceCS* _si,
                   NestedList* _mynl);
    virtual ~ConnectionInfo();

    std::string getHost() const;

    int getPort() const;

    std::string getConfig() const;

    bool check(bool showCommands, bool logOn, CommandLog& commandLog);

    void setId(const int i);

    void simpleCommand(std::string command1,
                       int& err,
                       std::string& result,
                       bool rewrite,
                       double& runtime,
                       bool showCommands,
                       bool logOn,
                       CommandLog& commandLog);

    std::string getSecondoHome(bool showCommands,
                               CommandLog& commandLog);

    bool cleanUp(bool showCommands,
                 bool logOn,
                 CommandLog& commandLog);
    bool switchDatabase(const std::string& dbname,
                        bool createifnotexists,
                        bool showCommands);

    void simpleCommand(const std::string& command1,
                       int& error,
                       std::string& errMsg,
                       std::string& resList,
                       const bool rewrite,
                       double& runtime,
                       bool log,
                       bool showCommands,
                       CommandLog& commandLog);

    void simpleCommandFromList(const std::string& command1,
                               int& error,
                               std::string& errMsg,
                               std::string& resList,
                               const bool rewrite,
                               double& runtime,
                               bool showCommands,
                               bool logOn,
                               CommandLog& commandLog);

    void simpleCommand(const std::string& command1,
                       int& error,
                       std::string& errMsg,
                       ListExpr& resList,
                       const bool rewrite,
                       double& runtime,
                       bool showCommands,
                       bool logOn,
                       CommandLog& commandLog);

    int serverPid();

    int sendFile(const std::string& local,
                 const std::string& remote,
                 const bool allowOverwrite);

    int requestFile(const std::string& remote,
                    const std::string& local,
                    const bool allowOverwrite);

    std::string getRequestFolder();

    std::string getSendFolder();

    std::string getSendPath();

    static ConnectionInfo* createConnection(const std::string& host,
                                            const int port,
                                            std::string& config,
                                            const std::string& user ="",
                                            const std::string& passwd="");

    bool createOrUpdateObject(const std::string& name,
                              ListExpr typelist,
                              Word& value,
                              bool showCommands,
                              bool logOn,
                              CommandLog& commandLog);

    bool createOrUpdateRelation(const std::string& name,
                                ListExpr typeList,
                                Word& value,
                                bool showCommands,
                                bool logOn,
                                CommandLog& commandLog);

    bool createOrUpdateRelationFromBinFile(const std::string& name,
                                           const std::string& filename,
                                           bool showCommands,
                                           bool logOn,
                                           CommandLog& commandLog,
                                           const bool allowOverwrite = true);

    bool createOrUpdateAttributeFromBinFile(const std::string& name,
                                            const std::string& filename,
                                            bool showCommands,
                                            bool logOn,
                                            CommandLog& commandLog,
                                            const bool allowOverwrite = true);

    bool saveRelationToFile(ListExpr relType,
                            Word& value,
                            const std::string& filename);
    bool saveAttributeToFile(ListExpr type,
                             Word& value,
                             const std::string& filename);

    bool storeObjectToFile(const std::string& objName,
                           Word& value,
                           ListExpr typeList,
                           const std::string& fileName);

    bool retrieve(const std::string& objName,
                  ListExpr& resType,
                  Word& result,
                  bool checkType,
                  bool showCommands,
                  bool logOn,
                  CommandLog& commandLog);

    bool retrieveRelation(const std::string& objName,
                          ListExpr& resType,
                          Word& result,
                          bool showCommands,
                          bool logOn,
                          CommandLog& commandLog);

    bool retrieveRelationInFile(const std::string& fileName,
                                ListExpr& resType,
                                Word& result,
                                bool showCommands,
                                bool logOn,
                                CommandLog& commandLog);

    bool retrieveRelationFile(const std::string& objName,
                              const std::string& fname1,
                              bool showCommands,
                              bool logOn,
                              CommandLog& commandLog);

    bool retrieveAnyFile(const std::string& remoteName,
                         const std::string& localName,
                         bool showCommands,
                         bool logOn,
                         CommandLog& commandLog);
    Word createRelationFromFile(const std::string& fname, ListExpr& resType);

    std::ostream& print(std::ostream& o) const;


private:
    void retrieveSecondoHome(bool showCommands,
                             CommandLog& commandLog);

    std::string host;
    int port;
    std::string config;
    SecondoInterfaceCS* si;
    NestedList* mynl;
    int serverPID;
    std::string secondoHome;
    std::string requestFolder;
    std::string sendFolder;
    std::string sendPath;
    boost::recursive_mutex simtx; // mutex for synchronizing 
                                  // access to the interface
};

std::ostream& operator<<(std::ostream& o, const ConnectionInfo& sc);

/*
0.16 ~showError~

*/

void showError(const ConnectionInfo* ci, const std::string& command ,
               const int errorCode, const std::string& errorMessage);

void showError(const SecondoInterfaceCS* ci, const std::string& command ,
               const int errorCode, const std::string& errorMessage);

} /* namespace distributed2 */

#endif /* ALGEBRAS_DISTRIBUTED2_CONNECTIONINFO_H_ */
